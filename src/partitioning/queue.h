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

#ifndef SHAREDMAP_QUEUE_H
#define SHAREDMAP_QUEUE_H

#include <atomic>
#include <queue>
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
    inline void queue_thread_work(Item item,
                                  std::priority_queue<Item> &queue,
                                  std::mutex &queue_lock,
                                  std::atomic<u64> &queue_size,
                                  std::atomic<u64> &n_available_threads,
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

        // load item to process
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

            // free item
            item.free();

            // make threads available
            n_available_threads += n_assigned_threads;
        } else {
            // create the subgraphs and place them in the next stack
            std::vector<Item> temp_stack;
            create_sub_graphs(g, tt, local_k, partition, identifier, temp_stack, depth, n_threads, stat_collector);

            // free item
            item.free();

            // push graphs into queue
            queue_lock.lock();
            for (size_t i = 0; i < local_k; ++i) {
                queue.emplace(temp_stack[i]);
            }
            n_available_threads += n_assigned_threads;
            queue_size += temp_stack.size();
            queue_lock.unlock();
        }
    }

    /**
     * Use the queue approach.
     *
     * @param original_g The original graph.
     * @param config The algorithm configuration.
     * @param stat_collector The statistic collector.
     * @return The partition.
     */
    inline std::vector<u64> solve_queue(const Graph &original_g,
                                 const AlgorithmConfiguration &config,
                                 StatCollector &stat_collector) {
        std::vector<u64> solution(original_g.get_n());    // end partition
        TranslationTable original_tt(original_g.get_n()); // default translation table

        // references for better code readability
        const u64 n_threads = config.n_threads;

        // initialize the queue
        std::priority_queue<Item> queue;
        queue.emplace(new std::vector<u64>(), const_cast<Graph *>(&original_g), &original_tt, false);
        std::mutex queue_lock;
        std::atomic<u64> queue_size = 1;
        std::atomic<u64> n_available_threads = n_threads;

        while (true) {
            while ((n_available_threads == 0 || queue_size == 0) && !(n_available_threads == n_threads && queue_size == 0)) {
                /* wait */
            }

            // get item
            queue_lock.lock();

            // all threads available and queue empty, so finished
            if (n_available_threads == n_threads && queue_size == 0) {
                queue_lock.unlock();
                break;
            }

            u64 n_assigned_threads = (n_available_threads + queue_size - 1) / queue_size;
            Item item = queue.top();
            queue.pop();
            n_available_threads -= n_assigned_threads;
            queue_size -= 1;
            queue_lock.unlock();

            // start thread
            std::thread thread(queue_thread_work,
                               item,
                               std::ref(queue),
                               std::ref(queue_lock),
                               std::ref(queue_size),
                               std::ref(n_available_threads),
                               n_assigned_threads,
                               std::ref(solution),
                               std::ref(original_g),
                               std::ref(config),
                               std::ref(stat_collector));
            thread.detach();
        }

        return solution;
    }
}

#endif //SHAREDMAP_QUEUE_H
