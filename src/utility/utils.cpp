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

#include "utils.h"

namespace SharedMap {
    std::vector<std::string> split(const std::string &str,
                                   const char c) {
        std::vector<std::string> splits;

        std::istringstream iss(str);
        std::string        token;

        while (std::getline(iss, token, c)) {
            splits.push_back(token);
        }

        return splits;
    }

    bool file_exists(const std::string &path) {
        std::ifstream f(path.c_str());
        return f.good();
    }

    void str_to_ints(const std::string &str,
                     std::vector<u64> &ints) {
        ints.resize(str.size());

        u64 idx         = 0;
        u64 curr_number = 0;

        for (const char c: str) {
            if (c == ' ') {
                ints[idx]   = curr_number;
                idx += curr_number != 0;
                curr_number = 0;
            } else {
                curr_number = curr_number * 10 + (c - '0');
            }
        }

        ints[idx] = curr_number;
        idx += curr_number != 0;
        ints.resize(idx);
    }
}
