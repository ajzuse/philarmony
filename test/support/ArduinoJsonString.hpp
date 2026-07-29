#pragma once

#include <ArduinoJson.h>

namespace ArduinoJson {

template <>
struct Converter<String> {
    static void toJson(const String& src, JsonVariant dst) { dst.set(src.c_str()); }

    static String fromJson(JsonVariantConst src) { return String(src.as<const char*>()); }

    static bool checkJson(JsonVariantConst src) { return src.is<const char*>(); }
};

}  // namespace ArduinoJson
