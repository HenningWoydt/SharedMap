#include <atomic>

#include "nb_layer.h"

#include "src/datastructures/translation_table.h"
#include "src/datastructures/item.h"
#include "src/partitioning/partition_util.h"

namespace SharedMap {
    void layer_thread_work(Item item,
                           u64 n_assigned_threads,
                           std::atomic<u64> &n_inactive_threads,
                           std::vector<u64> &solution,
                           const Graph &original_g,
                           const AlgorithmConfiguration &config,
                           StatCollector &stat_collector) {
        // references for better code readability
        const std::vector<u64> &hierarchy       = config.hierarchy;
        const size_t           l                = hierarchy.size();
        const std::vector<u64> &index_vec       = config.index_vec;
        const std::vector<u64> &k_rem_vec       = config.k_rem_vec;
        const f64              global_imbalance = config.imbalance;
        const u64              global_g_weight  = original_g.get_weight();
        const u64              global_k         = config.k;
        const u64              n_threads        = config.n_threads;

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
        std::vector<u64> partition(g.get_n());
        partition_graph(g, local_k, local_imbalance, partition, n_assigned_threads, depth, config.serial_alg_id, config.parallel_alg_id, stat_collector);

        if (depth == 0) {
            // insert solution
            u64 offset = 0;

            for (u64 i = 0; i < identifier.size(); ++i) { offset += identifier[i] * index_vec[index_vec.size() - 1 - i]; }
            for (u64 u = 0; u < g.get_n(); ++u) { solution[tt.get_o(u)] = offset + partition[u]; }
        } else {
            // create the subgraphs and place them in the next stack
            std::vector<Item> next_layer;
            create_sub_graphs(g, tt, local_k, partition, identifier, next_layer, depth, n_threads, stat_collector);

        }

        // free item
        item.free();

    }

    std::vector<u64> solve_nb_layer(const Graph &original_g, const AlgorithmConfiguration &config, StatCollector &stat_collector) {
        std::vector<u64> solution(original_g.get_n()); // end partition
        TranslationTable original_tt(original_g.get_n()); // default translation table

        // references for better code readability
        const std::vector<u64> &hierarchy = config.hierarchy;
        const size_t           l          = hierarchy.size();
        const u64              n_threads  = config.n_threads;

        // initialize stack;
        Item             item               = {new std::vector<u64>(), const_cast<Graph *>(&original_g), &original_tt, false};
        std::atomic<u64> n_inactive_threads = 0;

        // process first item
        layer_thread_work(item,
                          n_threads,
                          std::ref(n_inactive_threads),
                          std::ref(solution),
                          std::ref(original_g),
                          std::ref(config),
                          std::ref(stat_collector));

        return solution;
    }
}
