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

    std::string to_JSON_value(const std::string &s) { return "\"" + s + "\""; }
}
