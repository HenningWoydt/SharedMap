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

#ifndef SHAREDMAP_LIBSHAREDMAPTYPES_H
#define SHAREDMAP_LIBSHAREDMAPTYPES_H

#include <string>

typedef enum {
    NAIVE,
    LAYER,
    QUEUE,
    NB_LAYER
} shared_map_strategy_type_t;

/**
 * Converts the given enum into a string.
 *
 * @param strategy The strategy.
 * @return String representation.
 */
std::string strategy_to_string(shared_map_strategy_type_t strategy);

typedef enum {
    KAFFPA_FAST,
    KAFFPA_ECO,
    KAFFPA_STRONG,
    MTKAHYPAR_DEFAULT,
    MTKAHYPAR_QUALITY,
    MTKAHYPAR_HIGHEST_QUALITY
} shared_map_algorithm_type_t;

/**
 * Converts the given enum into a string.
 *
 * @param algorithm The algorithm.
 * @return String representation.
 */
std::string algorithm_to_string(shared_map_algorithm_type_t algorithm);

#endif //SHAREDMAP_LIBSHAREDMAPTYPES_H
