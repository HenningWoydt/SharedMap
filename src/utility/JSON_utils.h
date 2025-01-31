/*******************************************************************************
 * MIT License
 *
 * This file is part of SharedMap.
 *
 * Copyright (C) 2025 Henning Woydt <henning.woydt@informatik.uni-heidelberg.de>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 ******************************************************************************/

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
