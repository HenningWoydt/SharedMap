#ifndef SHAREDMAP_JSON_UTILS_H
#define SHAREDMAP_JSON_UTILS_H

#include "src/utility/definitions.h"
#include "src/utility/utils.h"

namespace SharedMap {
#define to_JSON_MACRO(x) (std::string("\"") + (#x) + "\" : " + to_JSON_value(x) + ",\n")

    std::string to_JSON_value(u8 x);

    std::string to_JSON_value(u16 x);

    std::string to_JSON_value(u32 x);

    std::string to_JSON_value(u64 x);

    std::string to_JSON_value(s8 x);

    std::string to_JSON_value(s16 x);

    std::string to_JSON_value(s32 x);

    std::string to_JSON_value(s64 x);

    std::string to_JSON_value(f32 x);

    std::string to_JSON_value(f64 x);

    std::string to_JSON_value(const std::string &s);

    template<typename T>
    std::string to_JSON_value(const std::vector<T> &vec) {
        if (vec.empty()) {
            return "[]";
        }
        if (vec.size() == 1) {
            return "[" + to_JSON_value(vec[0]) + "]";
        }
        std::string s = "[";
        for (size_t i = 0; i < vec.size() - 1; ++i) {
            s += to_JSON_value(vec[i]) + ", ";
        }
        s += to_JSON_value(vec.back()) + "]";

        return s;
    }
}

#endif //SHAREDMAP_JSON_UTILS_H
