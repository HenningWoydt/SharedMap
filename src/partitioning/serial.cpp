#include "serial.h"

#include "partition_util.h"

namespace SharedMap {
    std::vector<u64> solve_serial(const Graph &g, const AlgorithmConfiguration &config, StatCollector &stat_collector) {
        std::vector<u64> partition(g.get_n()); // end partition
        TranslationTable tt(g.get_n()); // default translation table

        // references for better code readability
        const std::vector<u64> &hierarchy = config.hierarchy;
        const size_t l = hierarchy.size();
        const std::vector<u64> &index_vec = config.index_vec;
        const std::vector<u64> &k_rem_vec = config.k_rem_vec;
        const f64 global_imbalance = config.imbalance;
        const u64 global_g_weight = g.get_weight();
        const u64 global_k = config.k;

        // initialize stack;
        std::vector<Item> stack = {{new std::vector<u64>(), const_cast<Graph *>(&g), &tt, false}};

        std::vector<Item> temp_stack;

        while (!stack.empty()) {
            Item item = stack.back(); // process first item
            stack.pop_back(); // remove top item

            // first process the item given, load item to process
            const Graph &g = (*item.g);
            const TranslationTable &tt = (*item.tt);
            const std::vector<u64> &identifier = (*item.identifier);

            // get depth info
            u64 depth = l - 1 - identifier.size();
            u64 local_k = hierarchy[depth];
            u64 local_k_rem = k_rem_vec[depth];

            f64 local_imbalance = determine_adaptive_imbalance(global_imbalance, global_g_weight, global_k, g.get_weight(), local_k_rem, depth + 1);

            // determine the partition for this graph
            partition_graph(g, local_k, local_imbalance, partition, 1, depth, config.serial_alg_id, config.parallel_alg_id);

            if (depth == 0) {
                // calculate offset
                u64 offset = 0;
                for (u64 i = 0; i < identifier.size(); ++i) { offset += identifier[i] * index_vec[index_vec.size() - 1 - i]; }

                // insert solution
                for (u64 u = 0; u < g.get_n(); ++u) { partition[tt.get_o(u)] = offset + partition[u]; }
            } else {
                // create the subgraphs and place them in the next stack
                temp_stack.clear();
                create_sub_graphs(g, tt, local_k, partition, identifier, temp_stack, depth, 1, stat_collector);

                // push graphs into next work, this has to be thread safe
                for (auto & i : temp_stack) {
                    stack.emplace_back(i);
                }
            }

            item.free();
        }

        return partition;


    }
}
