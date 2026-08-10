# Contract: History Export (CSV + PDF)

**Feature**: `003-filament-dryer-control-app` | **Date**: 2026-08-10  
**Clarification**: MVP requires **both** formats (Session 2026-08-10)

## Scope

From History list or cycle detail, user can export a completed `DryingCycle` (optionally with samples) as:

1. **CSV** — all scalar cycle fields + optional sample rows  
2. **PDF** — human-readable summary **and** charts (temp / humidity / heater vs time)

Both MUST succeed for the same cycle in acceptance tests.

## CSV

- Encoding: UTF-8  
- Header row with stable column names (snake_case matching data-model fields)  
- If samples included: either second section or companion `*_samples.csv` — pick one approach and document in exporter README comment; prefer single file with a `section` column or blank-line separated blocks  
- Locale: numeric values use `.` decimal; timestamps ISO-8601 UTC

## PDF

- Page size: A4 or Letter  
- Sections: title (device nickname + material), params (targets, duration), stop reason, stats (avg/max temp, humidity), **charts** rendered from `CycleSample` (or downsampled buffer)  
- Languages: respect app locale (PT-BR / EN-US strings)  
- Must open in common readers (Preview, Adobe, Chrome)

## Non-goals

- Batch export of entire history DB as one PDF (may offer later)  
- Cloud upload of exports  
- Digitally signed PDFs

## Implementation hints (non-normative)

- Shared chart data from Drift samples — do not re-query live WS  
- Generate PDF off UI isolate if large  
- File share via platform share sheet / save dialog
