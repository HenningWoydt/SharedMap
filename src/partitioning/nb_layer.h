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

#ifndef SHAREDMAP_NB_LAYER_H
#define SHAREDMAP_NB_LAYER_H

#include <atomic>
#include <thread>
#include <vector>

#include "src/datastructures/graph.h"
#include "src/datastructures/item.h"
#include "src/datastructures/item.h"
#include "src/datastructures/translation_table.h"
#include "src/partitioning/partition_util.h"
#include "src/profiling/stat_collector.h"
#include "src/utility/algorithm_configuration.h"
#include "src/utility/definitions.h"

namespace SharedMap {
    inline void nb_layer_work(u64 local_idx,
                              std::shared_ptr<std::atomic<u64> > layer_idx,
                              std::shared_ptr<std::vector<Item> > layer,
                              const u64 layer_size,
                              u64 n_assigned_threads,
                              std::atomic<u64> &n_inactive_threads,
                              std::atomic<u64> &n_completed_graphs,
                              std::vector<u64> &solution,
                              const Graph &original_g,
                              const AlgorithmConfiguration &config,
                              StatCollector &stat_collector) {
        // collect all items
        std::shared_ptr<std::vector<Item> > next_layer = std::make_shared<std::vector<Item> >();

        while (local_idx < layer_size) {
            // get more threads if available
            n_assigned_threads += n_inactive_threads.exchange(0);

            // references for better code readability
            const std::vector<u64> &hierarchy = config.hierarchy;
            const size_t l = hierarchy.size();
            const std::vector<u64> &index_vec = config.index_vec;
            const std::vector<u64> &k_rem_vec = config.k_rem_vec;
            const f64 global_imbalance = config.imbalance;
            const u64 global_g_weight = original_g.get_weight();
            const u64 global_k = config.k;

            // load item to process
            Item item = layer->at(local_idx);
            const Graph &g = (*item.g);
            const TranslationTable &tt = (*item.tt);
            const std::vector<u64> &identifier = (*item.identifier);

            // get depth info
            const u64 depth = l - 1 - identifier.size();
            const u64 local_k = hierarchy[depth];
            const u64 local_k_rem = k_rem_vec[depth];
            const f64 local_imbalance = determine_adaptive_imbalance(global_imbalance, global_g_weight, global_k, g.get_weight(), local_k_rem, depth + 1);

            // partition the subgraph
            std::vector<u64> partition(g.get_n());
            partition_graph(g, local_k, local_imbalance, partition, n_assigned_threads, depth, config.serial_alg_id, config.parallel_alg_id, config.seed, stat_collector);

            if (depth == 0) {
                // insert solution
                u64 offset = 0;

                for (u64 i = 0; i < identifier.size(); ++i) { offset += identifier[i] * index_vec[index_vec.size() - 1 - i]; }
                for (u64 u = 0; u < g.get_n(); ++u) { solution[tt.get_o(u)] = offset + partition[u]; }

                n_completed_graphs += local_k;
            } else {
                // create the subgraphs and place them in the next stack
                std::vector<Item> temp_stack;
                create_sub_graphs(g, tt, local_k, partition, identifier, temp_stack, depth, n_assigned_threads, stat_collector);

                // push back items into the next layer for this thread
                for (size_t i = 0; i < local_k; ++i) {
                    next_layer->push_back(temp_stack[i]);
                }
            }

            // free item
            item.free();

            // fetch new item
            local_idx = layer_idx->fetch_add(1);
        }

        if (next_layer->empty()) {
            // no more work, so release the threads
            n_inactive_threads += n_assigned_threads;
            return;
        }

        // spawn threads
        u64 n_items = next_layer->size();
        std::shared_ptr<std::atomic<u64> > next_global_idx = std::make_shared<std::atomic<u64> >(std::min(n_assigned_threads, n_items));
        for (u64 t = 0; t < std::min(n_assigned_threads, n_items); ++t) {
            u64 n_local_assigned_threads = std::max((u64) 1, n_assigned_threads / n_items) + (n_assigned_threads > n_items) * (t < n_assigned_threads % n_items);

            std::thread thread(nb_layer_work,
                               t,
                               next_global_idx,
                               next_layer,
                               n_items,
                               n_local_assigned_threads,
                               std::ref(n_inactive_threads),
                               std::ref(n_completed_graphs),
                               std::ref(solution),
                               std::ref(original_g),
                               std::ref(config),
                               std::ref(stat_collector));
            thread.detach();
        }
    }

    /**
     * Uses the non-blocking layer approach.
     *
     * @param original_g The original graph.
     * @param config The algorithm configuration.
     * @param stat_collector The statistic collector.
     * @return The partition.
     */
    std::vector<u64> solve_nb_layer(const Graph &original_g,
                                    const AlgorithmConfiguration &config,
                                    StatCollector &stat_collector) {
        std::vector<u64> solution(original_g.get_n());    // end partition
        TranslationTable original_tt(original_g.get_n()); // default translation table

        // references for better code readability
        const u64 n_threads = config.n_threads;

        // initialize layer;
        std::shared_ptr<std::vector<Item> > layer = std::make_shared<std::vector<Item> >();
        layer->push_back({new std::vector<uint64_t>(), const_cast<Graph *>(&original_g), &original_tt, false});
        std::shared_ptr<std::atomic<u64> > layer_idx = std::make_shared<std::atomic<u64> >(1);
        std::atomic<u64> n_inactive_threads = 0;
        std::atomic<u64> n_completed_subgraphs = 0;

        // process first item
        nb_layer_work(0,
                      std::ref(layer_idx),
                      std::ref(layer),
                      1,
                      n_threads,
                      std::ref(n_inactive_threads),
                      std::ref(n_completed_subgraphs),
                      std::ref(solution),
                      std::ref(original_g),
                      std::ref(config),
                      std::ref(stat_collector));

        while (n_completed_subgraphs != config.k) {
        }

        return solution;
    }
}

#endif //SHAREDMAP_NB_LAYER_H
