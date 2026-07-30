/*
 * Philarmony Filament Dryer — Desktop Installer / Shared Core
 * Copyright (C) 2026 Philarmony Contributors
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

import 'dart:convert';
import 'dart:typed_data';

/// ESP-IDF NVS partition writer for string entries (Arduino Preferences-compatible).
///
/// Layout matches IDF `nvs_page.hpp`:
/// - page header 32 B (CRC over bytes 4–27)
/// - entry-state bitmap 32 B (2 bits/entry)
/// - entries from offset 64 (32 B each, CRC over ns/type/span/chunk + key + data)
class NvsBinaryWriter {
  static const pageSize = 4096;
  static const entrySize = 32;
  static const pageHeaderSize = 32;
  static const entryStateBitmapSize = 32;
  static const firstEntryOffset = pageHeaderSize + entryStateBitmapSize; // 64
  static const entriesPerPage =
      (pageSize - firstEntryOffset) ~/ entrySize; // 126
  static const nvsVersion = 0xfe;
  static const stateActive = 0xfffffffe;

  static const typeNamespace = 0x00;
  static const typeString = 0x21;

  /// Entry state nibbles (2 bits each): Written = 2, Empty = 3 (0b11).
  static const entryStateWritten = 0x2;
  static const entryStateEmpty = 0x3;

  /// [entries] maps key → UTF-8 string value under [namespace].
  Uint8List build({
    required String namespace,
    required Map<String, String> entries,
    int sizeBytes = 0x5000,
  }) {
    if (sizeBytes % pageSize != 0) {
      throw ArgumentError('sizeBytes must be a multiple of $pageSize');
    }
    final pageCount = sizeBytes ~/ pageSize;
    final pages = List.generate(pageCount, (_) {
      final p = Uint8List(pageSize);
      p.fillRange(0, pageSize, 0xff);
      return p;
    });

    var pageIndex = 0;
    var entryIndex = 0;
    var seqNo = 0;
    var nextNs = 1;
    var headerWritten = false;

    void writePageHeader(int pi, int seq) {
      final page = pages[pi];
      page.fillRange(0, pageSize, 0xff);
      final bd = ByteData.sublistView(page);
      bd.setUint32(0, stateActive, Endian.little);
      bd.setUint32(4, seq, Endian.little);
      page[8] = nvsVersion;
      // CRC over seqNo..unused (bytes 4–27), per ESP-IDF nvs_page.cpp
      final crc = _crc32(page.sublist(4, 28));
      bd.setUint32(28, crc, Endian.little);
    }

    void markEntryWritten(int pi, int ei) {
      // 2 bits per entry in bitmap starting at offset 32
      final bitIndex = ei * 2;
      final byteIndex = pageHeaderSize + (bitIndex ~/ 8);
      final shift = bitIndex % 8;
      final page = pages[pi];
      var b = page[byteIndex];
      b = (b & ~(0x3 << shift)) | (entryStateWritten << shift);
      page[byteIndex] = b;
    }

    int allocEntries(int span) {
      if (!headerWritten) {
        writePageHeader(pageIndex, seqNo++);
        entryIndex = 0;
        headerWritten = true;
      }
      if (entryIndex + span > entriesPerPage) {
        pageIndex++;
        if (pageIndex >= pageCount) {
          throw StateError('NVS image full ($sizeBytes bytes)');
        }
        writePageHeader(pageIndex, seqNo++);
        entryIndex = 0;
      }
      final start = entryIndex;
      entryIndex += span;
      return start;
    }

    void putEntry(int pi, int ei, Uint8List entry32) {
      final offset = firstEntryOffset + ei * entrySize;
      pages[pi].setRange(offset, offset + entrySize, entry32);
      markEntryWritten(pi, ei);
    }

    Uint8List makeEntry({
      required int ns,
      required int type,
      required int span,
      required String key,
      required void Function(ByteData bd) writeData,
    }) {
      final buf = Uint8List(entrySize);
      buf.fillRange(0, entrySize, 0xff);
      final bd = ByteData.sublistView(buf);
      buf[0] = ns;
      buf[1] = type;
      buf[2] = span;
      buf[3] = 0xff; // chunkIndex
      final keyBytes = utf8.encode(key);
      if (keyBytes.length > 15) {
        throw ArgumentError('NVS key too long: $key');
      }
      for (var i = 0; i < 16; i++) {
        buf[8 + i] = i < keyBytes.length ? keyBytes[i] : 0;
      }
      writeData(bd);
      // CRC over ns/type/span/chunk + key[16] + data[8]
      final crc = _crc32([...buf.sublist(0, 4), ...buf.sublist(8, 32)]);
      bd.setUint32(4, crc, Endian.little);
      return buf;
    }

    final nsIndex = nextNs++;
    final nsStart = allocEntries(1);
    putEntry(
      pageIndex,
      nsStart,
      makeEntry(
        ns: 0,
        type: typeNamespace,
        span: 1,
        key: namespace,
        writeData: (bd) => bd.setUint8(24, nsIndex),
      ),
    );

    for (final e in entries.entries) {
      final data = Uint8List.fromList([...utf8.encode(e.value), 0]);
      final dataSpan = (data.length + entrySize - 1) ~/ entrySize;
      final span = 1 + dataSpan;
      final start = allocEntries(span);
      final firstPi = pageIndex;
      putEntry(
        firstPi,
        start,
        makeEntry(
          ns: nsIndex,
          type: typeString,
          span: span,
          key: e.key,
          writeData: (bd) {
            bd.setUint16(24, data.length, Endian.little);
            bd.setUint16(26, 0xffff, Endian.little);
            bd.setUint32(28, _crc32(data), Endian.little);
          },
        ),
      );
      var offset = 0;
      for (var s = 1; s < span; s++) {
        final chunk = Uint8List(entrySize);
        chunk.fillRange(0, entrySize, 0xff);
        final remain = data.length - offset;
        final n = remain > entrySize ? entrySize : remain;
        chunk.setRange(0, n, data, offset);
        offset += n;
        putEntry(firstPi, start + s, chunk);
      }
    }

    final out = Uint8List(sizeBytes);
    for (var i = 0; i < pageCount; i++) {
      out.setRange(i * pageSize, (i + 1) * pageSize, pages[i]);
    }
    return out;
  }

  /// CRC-32 (ISO-HDLC / zlib), matching ESP-IDF NVS.
  static int _crc32(List<int> data) {
    var crc = 0xffffffff;
    for (final b in data) {
      crc ^= b;
      for (var i = 0; i < 8; i++) {
        crc = (crc & 1) != 0 ? (crc >> 1) ^ 0xedb88320 : crc >> 1;
      }
    }
    return (crc ^ 0xffffffff) & 0xffffffff;
  }
}
