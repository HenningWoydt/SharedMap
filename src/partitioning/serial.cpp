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

#include "serial.h"

namespace SharedMap {
    std::vector<u64> solve_serial(const Graph &original_g,
                                  const AlgorithmConfiguration &config,
                                  StatCollector &stat_collector) {
        std::vector<u64> solution(original_g.get_n()); // end partition
        TranslationTable original_tt(original_g.get_n()); // default translation table

        // references for better code readability
        const std::vector<u64> &hierarchy       = config.hierarchy;
        const size_t           l                = hierarchy.size();
        const std::vector<u64> &index_vec       = config.index_vec;
        const std::vector<u64> &k_rem_vec       = config.k_rem_vec;
        const f64              global_imbalance = config.imbalance;
        const u64              global_g_weight  = original_g.get_weight();
        const u64              global_k         = config.k;

        // initialize stack;
        std::vector<Item> stack = {{new std::vector<u64>(), const_cast<Graph *>(&original_g), &original_tt, false}};
        std::vector<Item> temp_stack;
        std::vector<u64>  partition;

        while (!stack.empty()) {
            Item item = stack.back(); // process first item
            stack.pop_back(); // remove top item

            // load item to process
            const Graph            &g          = (*item.g);
            const TranslationTable &tt         = (*item.tt);
            const std::vector<u64> &identifier = (*item.identifier);

            // get depth info
            const u64 depth           = l - 1 - identifier.size();
            const u64 local_k         = hierarchy[depth];
            const u64 local_k_rem     = k_rem_vec[depth];
            const f64 local_imbalance = determine_adaptive_imbalance(global_imbalance, global_g_weight, global_k, g.get_weight(), local_k_rem, depth + 1);

            // partition the subgraph
            partition.resize(g.get_n());
            partition_graph(g, local_k, local_imbalance, partition, 1, depth, config.serial_alg_id, config.parallel_alg_id, config.seed, stat_collector);

            if (depth == 0) {
                // insert solution
                u64      offset = 0;
                for (u64 i      = 0; i < identifier.size(); ++i) { offset += identifier[i] * index_vec[index_vec.size() - 1 - i]; }
                for (u64 u      = 0; u < g.get_n(); ++u) { solution[tt.get_o(u)] = offset + partition[u]; }
            } else {
                // create the subgraphs and place them in the next stack
                temp_stack.clear();
                create_sub_graphs(g, tt, local_k, partition, identifier, temp_stack, depth, 1, stat_collector);

                // push graphs into next work
                for (auto &i: temp_stack) {
                    stack.emplace_back(i);
                }
            }

            // release memory of the subgraph
            item.free();
        }

        return solution;
    }
}
