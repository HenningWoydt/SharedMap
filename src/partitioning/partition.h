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

#ifndef SHAREDMAP_PARTITION_H
#define SHAREDMAP_PARTITION_H

#include <vector>

#include "src/datastructures/graph.h"
#include "src/utility/algorithm_configuration.h"
#include "src/utility/definitions.h"

namespace SharedMap {
    /**
     * Uses Kaffpa to partition a graph into k partitions.
     *
     * @param g The graph.
     * @param k The number of partitions.
     * @param imbalance The allowed imbalance.
     * @param partition The resulting partition.
     * @param kaffpa_config Configuration for kaffpa.
     * @param seed The random seed.
     */
    void kaffpa_partition(const Graph &g,
                          u64 k,
                          f64 imbalance,
                          std::vector<u64> &partition,
                          u64 kaffpa_config,
                          u64 seed);

    /**
     * Uses MtKaHyPar to partition a graph into k partitions.
     *
     * @param g The graph.
     * @param k The number of partitions.
     * @param imbalance The allowed imbalance.
     * @param partition The resulting partition.
     * @param mt_kahypar_config Configuration for MtKaHyPar.
     * @param n_threads Number of cores to use.
     * @param seed The random seed.
     */
    void mt_kahypar_partition(const Graph &g,
                              u64 k,
                              f64 imbalance,
                              std::vector<u64> &partition,
                              u64 mt_kahypar_config,
                              u64 n_threads,
                              u64 seed);
}

#endif //SHAREDMAP_PARTITION_H
