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

#ifndef SHAREDMAP_PARTITION_UTIL_H
#define SHAREDMAP_PARTITION_UTIL_H

#include <atomic>
#include <cmath>
#include <thread>

#include "src/datastructures/graph.h"
#include "src/datastructures/item.h"
#include "src/datastructures/translation_table.h"
#include "src/partitioning/partition.h"
#include "src/profiling/stat_collector.h"
#include "src/utility/algorithm_configuration.h"
#include "src/utility/definitions.h"

namespace SharedMap {
    /**
     * Determine the adaptive imbalance.
     *
     * @param global_imbalance Global desired imbalance.
     * @param global_g_weight Original graph weight.
     * @param global_k Number of final partitions.
     * @param local_g_weight Local graph weight.
     * @param local_k_rem Local number of partitions.
     * @param depth Depth of the partitioning.
     * @return The adaptive imbalance.
     */
    f64 determine_adaptive_imbalance(f64 global_imbalance,
                                     u64 global_g_weight,
                                     u64 global_k,
                                     u64 local_g_weight,
                                     u64 local_k_rem,
                                     u64 depth);

    /**
     * Partitions the graph.
     *
     * @param g The graph.
     * @param k The number of partitions.
     * @param imbalance The imbalance.
     * @param partition Space for the partition.
     * @param n_threads Number of threads to use.
     * @param depth Depth of the partitioning.
     * @param serial_alg_id Vector of serial algorithm ids.
     * @param parallel_alg_id Vector of parallel algorithm ids.
     * @param stat_collector The statistic collector.
     */
    void partition_graph(const Graph &g,
                         u64 k,
                         f64 imbalance,
                         std::vector<u64> &partition,
                         u64 n_threads,
                         u64 depth,
                         const std::vector<u64> &serial_alg_id,
                         const std::vector<u64> &parallel_alg_id,
                         u64 seed,
                         StatCollector &stat_collector);

    /**
     * Creates subgraph of a graph and its partition.
     *
     * @param g The graph.
     * @param g_tt The translation table of the graph.
     * @param k The number of partitions.
     * @param partition The partition.
     * @param identifier The identifier of the graph.
     * @param temp_stack Space to put the partitioning tasks.
     * @param depth The depth of the creation.
     * @param n_threads The number of threads.
     * @param stat_collector The statistic collector.
     */
    void create_sub_graphs(const Graph &g,
                           const TranslationTable &g_tt,
                           u64 k,
                           const std::vector<u64> &partition,
                           const std::vector<u64> &identifier,
                           std::vector<Item> &temp_stack,
                           u64 depth,
                           u64 n_threads,
                           StatCollector &stat_collector);
}

#endif // SHAREDMAP_PARTITION_UTIL_H
