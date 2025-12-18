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

#ifndef SHAREDMAP_UTILS_H
#define SHAREDMAP_UTILS_H

#include <algorithm>
#include <cmath>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include "src/utility/definitions.h"

namespace SharedMap {
    /**
     * Splits a string into multiple sub-strings. The specified character will
     * serve as the delimiter and will not be present in any string.
     *
     * @param str The string.
     * @param c The character.
     * @return Vector of sub-strings.
     */
    inline std::vector<std::string> split(const std::string &str,
                                          char c) {
        std::vector<std::string> splits;

        std::istringstream iss(str);
        std::string token;

        while (std::getline(iss, token, c)) {
            splits.push_back(token);
        }

        return splits;
    }

    /**
     * Converts a string into the specified datatype. Conversion is done via
     * string stream and ">>" operator.
     *
     * @tparam T The desired datatype.
     * @param str The string.
     * @return The converted string.
     */
    template<typename T>
    T convert_to(const std::string &str) {
        T result;
        std::istringstream iss(str);
        iss >> result;
        return result;
    }

    /**
     * Converts the vector of strings into a vector of T's.
     *
     * @tparam T Type of conversion.
     * @param vec The vector.
     * @return Vector of transformed T's.
     */
    template<typename T>
    std::vector<T> convert(const std::vector<std::string> &&vec) {
        std::vector<T> v;

        for (auto &s: vec) {
            v.push_back(convert_to<T>(s));
        }

        return v;
    }

    /**
     * Converts a string that holds integers to a vector of integers.
     *
     * @param str The string.
     * @param ints The vector that will hold the solution.
     */
    inline void str_to_ints(const std::string &str,
                     std::vector<u64> &ints) {
        ints.resize(str.size());

        u64 idx = 0;
        u64 curr_number = 0;

        for (const char c: str) {
            if (c == ' ') {
                ints[idx] = curr_number;
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

    /**
     * Multiplies all elements in the vector.
     *
     * @tparam T Vector type.
     * @param vec The vector.
     * @return The product.
     */
    template<typename T>
    T product(const std::vector<T> &vec) {
        T p = (T) 1;
        for (auto &x: vec) { p *= x; }
        return p;
    }

    /**
     * Checks if a file exists at the specified path.
     *
     * @param path The file path.
     * @return True if the file exists, false else.
     */
    inline bool file_exists(const std::string &path) {
        std::ifstream f(path.c_str());
        return f.good();
    }

    // Suggested shape of your helper
    struct MMap {
        char *data = nullptr;
        size_t size = 0;
        int fd = -1; // keep fd so you can close it later
    };

    inline MMap mmap_file_ro(const std::string &path) {
        MMap mm;

        int fd = ::open(path.c_str(), O_RDONLY | O_CLOEXEC);
        if (fd < 0) {
            perror("open");
            std::exit(EXIT_FAILURE);
        }

        struct stat st{};
        if (fstat(fd, &st) != 0) {
            perror("fstat");
            std::exit(EXIT_FAILURE);
        }
        size_t size = static_cast<size_t>(st.st_size);

        #ifdef __linux__
        // 1) Tell the kernel we’ll read sequentially (before mmap)
        (void) posix_fadvise(fd, 0, 0, POSIX_FADV_SEQUENTIAL);
        #endif

        void *addr = ::mmap(nullptr, size, PROT_READ, MAP_PRIVATE, fd, 0);
        if (addr == MAP_FAILED) {
            perror("mmap");
            std::exit(EXIT_FAILURE);
        }

        #ifdef __linux__
        // 2) Hint that we’ll need these pages, sequentially (right after mmap)
        (void) madvise(addr, size, MADV_SEQUENTIAL | MADV_WILLNEED);
        #endif

        mm.data = static_cast<char *>(addr);
        mm.size = size;
        mm.fd = fd; // store; close in your munmap_file(...)
        return mm;
    }

    inline void munmap_file(const MMap &mm) {
        if (mm.data && mm.size) ::munmap(mm.data, mm.size);
        if (mm.fd >= 0) ::close(mm.fd);
    }
}

#endif //SHAREDMAP_UTILS_H
