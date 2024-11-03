#include "JSON_utils.h"

namespace SharedMap {
    std::string to_JSON_value(const u8 x) { return std::to_string(x); }

    std::string to_JSON_value(const u16 x) { return std::to_string(x); }

    std::string to_JSON_value(const u32 x) { return std::to_string(x); }

    std::string to_JSON_value(const u64 x) { return std::to_string(x); }

    std::string to_JSON_value(const s8 x) { return std::to_string(x); }

    std::string to_JSON_value(const s16 x) { return std::to_string(x); }

    std::string to_JSON_value(const s32 x) { return std::to_string(x); }

    std::string to_JSON_value(const s64 x) { return std::to_string(x); }

    std::string to_JSON_value(const f32 x) { return std::to_string(x); }

    std::string to_JSON_value(const f64 x) { return std::to_string(x); }

    std::string to_JSON_value(const std::string& s) { return "\"" + s + "\""; }
}
