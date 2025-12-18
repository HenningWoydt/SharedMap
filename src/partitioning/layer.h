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

#ifndef SHAREDMAP_LAYER_H
#define SHAREDMAP_LAYER_H

#include <atomic>
#include <thread>
#include <vector>

#include "src/datastructures/graph.h"
#include "src/datastructures/item.h"
#include "src/datastructures/translation_table.h"
#include "src/partitioning/partition_util.h"
#include "src/profiling/stat_collector.h"
#include "src/utility/algorithm_configuration.h"
#include "src/utility/definitions.h"

namespace SharedMap {
    void layer_thread_work(u64 local_idx,
                           std::atomic<u64> &global_idx,
                           std::vector<Item> &layer,
                           u64 layer_size,
                           std::vector<Item> &next_layer,
                           u64 n_assigned_threads,
                           std::vector<u64> &solution,
                           const Graph &original_g,
                           const AlgorithmConfiguration &config,
                           StatCollector &stat_collector) {
        // references for better code readability
        const std::vector<u64> &hierarchy = config.hierarchy;
        const size_t l = hierarchy.size();
        const std::vector<u64> &index_vec = config.index_vec;
        const std::vector<u64> &k_rem_vec = config.k_rem_vec;
        const f64 global_imbalance = config.imbalance;
        const u64 global_g_weight = original_g.get_weight();
        const u64 global_k = config.k;
        const u64 n_threads = config.n_threads;

        std::vector<Item> temp_stack;
        std::vector<u64> partition;

        while (local_idx < layer_size) {
            // load item to process
            Item &item = layer[local_idx];
            const Graph &g = (*item.g);
            const TranslationTable &tt = (*item.tt);
            const std::vector<u64> &identifier = (*item.identifier);

            // get depth info
            const u64 depth = l - 1 - identifier.size();
            const u64 local_k = hierarchy[depth];
            const u64 local_k_rem = k_rem_vec[depth];
            const f64 local_imbalance = determine_adaptive_imbalance(global_imbalance, global_g_weight, global_k, g.get_weight(), local_k_rem, depth + 1);

            // partition the subgraph
            partition.resize(g.get_n());
            partition_graph(g, local_k, local_imbalance, partition, n_assigned_threads, depth, config.serial_alg_id, config.parallel_alg_id, config.seed, stat_collector);

            if (depth == 0) {
                // insert solution
                u64 offset = 0;
                for (u64 i = 0; i < identifier.size(); ++i) { offset += identifier[i] * index_vec[index_vec.size() - 1 - i]; }
                for (u64 u = 0; u < g.get_n(); ++u) { solution[tt.get_o(u)] = offset + partition[u]; }
            } else {
                // create the subgraphs and place them in the next stack
                temp_stack.clear();
                create_sub_graphs(g, tt, local_k, partition, identifier, temp_stack, depth, n_threads, stat_collector);

                // push graphs into next work
                for (size_t i = 0; i < local_k; ++i) {
                    next_layer[local_idx * local_k + i] = temp_stack[i];
                }
            }

            // free item
            item.free();

            // look for more work
            local_idx = global_idx.fetch_add(1);
        }
    }

    /**
     * Uses the layer approach.
     *
     * @param original_g The original graph.
     * @param config The algorithm configuration.
     * @param stat_collector The statistic collector.
     * @return The partition.
     */
    inline std::vector<u64> solve_layer(const Graph &original_g,
                                        const AlgorithmConfiguration &config,
                                        StatCollector &stat_collector) {
        std::vector<u64> solution(original_g.get_n());    // end partition
        TranslationTable original_tt(original_g.get_n()); // default translation table

        // references for better code readability
        const std::vector<u64> &hierarchy = config.hierarchy;
        const size_t l = hierarchy.size();
        const u64 n_threads = config.n_threads;

        // initialize stack;
        std::vector<Item> layer = {{new std::vector<u64>(), const_cast<Graph *>(&original_g), &original_tt, false}};
        std::vector<Item> next_layer;

        // do l iterations
        for (size_t i = 0; i < l; ++i) {
            // get depth info
            const u64 depth = l - 1 - i;
            const u64 local_k = hierarchy[depth];

            // allocate enough space for the next layer
            if (depth != 0) { next_layer.resize(local_k * layer.size()); }

            // determine how many threads to start
            u64 threads_to_start = std::min(layer.size(), n_threads);
            std::atomic<u64> global_idx = threads_to_start;

            // spawn threads
            std::vector<std::thread> threads;
            for (u64 t = 0; t < threads_to_start; ++t) {
                u64 local_idx = t;
                u64 n_assigned_threads = std::max((u64) 1, (n_threads / layer.size())) + (n_threads > layer.size()) * (t < (n_threads % layer.size()));

                threads.emplace_back(layer_thread_work,
                                     local_idx,
                                     std::ref(global_idx),
                                     std::ref(layer),
                                     layer.size(),
                                     std::ref(next_layer),
                                     n_assigned_threads,
                                     std::ref(solution),
                                     std::ref(original_g),
                                     std::ref(config),
                                     std::ref(stat_collector));
            }

            // Wait for all threads to finish
            for (auto &thread: threads) { if (thread.joinable()) { thread.join(); } }

            // ready next layer
            layer.swap(next_layer);
            next_layer.clear();
        }

        return solution;
    }
}

#endif //SHAREDMAP_LAYER_H
