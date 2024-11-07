#include "nb_layer.h"

namespace SharedMap {
    void nb_layer_thread_work_oversubscribed(u64 local_idx,
                                             std::atomic<u64>& global_idx,
                                             const std::vector<Item>& layer,
                                             const u64 layer_size,
                                             u64 n_assigned_threads,
                                             std::atomic<u64>& n_inactive_threads,
                                             std::vector<u64>& solution,
                                             const Graph& original_g,
                                             const AlgorithmConfiguration& config,
                                             StatCollector& stat_collector) {
        // collect all items
        std::vector<Item> next_layer;

        while (local_idx < layer_size) {
            // get more threads if available
            n_assigned_threads += n_inactive_threads.exchange(0);

            // references for better code readability
            const std::vector<u64>& hierarchy = config.hierarchy;
            const size_t l                    = hierarchy.size();
            const std::vector<u64>& index_vec = config.index_vec;
            const std::vector<u64>& k_rem_vec = config.k_rem_vec;
            const f64 global_imbalance        = config.imbalance;
            const u64 global_g_weight         = original_g.get_weight();
            const u64 global_k                = config.k;

            // load item to process
            Item item                          = layer[local_idx];
            const Graph& g                     = (*item.g);
            const TranslationTable& tt         = (*item.tt);
            const std::vector<u64>& identifier = (*item.identifier);

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
                std::vector<Item> temp_stack;
                create_sub_graphs(g, tt, local_k, partition, identifier, temp_stack, depth, n_assigned_threads, stat_collector);

                // push back items into the next layer for this thread
                for (size_t i = 0; i < local_k; ++i) {
                    next_layer.push_back(temp_stack[i]);
                }
            }

            // free item
            item.free();

            // fetch new item
            local_idx = global_idx.fetch_add(1);
        }

        if (next_layer.empty()) {
            // no more work, so release the threads
            n_inactive_threads += n_assigned_threads;
            return;
        }

        u64 n_items = next_layer.size();
        if (n_assigned_threads >= n_items) {
            // every item can be started with at least one thread

            // spawn threads
            std::vector<std::thread> threads;
            for (u64 t = 0; t < n_items; ++t) {
                u64 n_local_assigned_threads = std::max((u64)1, n_assigned_threads / n_items) + (n_assigned_threads > n_items) * (t < n_assigned_threads % n_items);

                threads.emplace_back(nb_layer_thread_work,
                                     next_layer[t],
                                     n_local_assigned_threads,
                                     std::ref(n_inactive_threads),
                                     std::ref(solution),
                                     std::ref(original_g),
                                     std::ref(config),
                                     std::ref(stat_collector));
            }

            // Wait for all threads to finish
            for (auto& thread : threads) { if (thread.joinable()) { thread.join(); } }
        } else {
            // too many threads than tasks

            // determine how many threads to start
            std::atomic<u64> new_global_idx = n_assigned_threads;

            // spawn threads
            std::vector<std::thread> threads;
            for (u64 t = 0; t < n_assigned_threads; ++t) {
                u64 new_local_idx = t;

                threads.emplace_back(nb_layer_thread_work_oversubscribed,
                                     new_local_idx,
                                     std::ref(new_global_idx),
                                     std::ref(next_layer),
                                     n_items,
                                     n_assigned_threads,
                                     std::ref(n_inactive_threads),
                                     std::ref(solution),
                                     std::ref(original_g),
                                     std::ref(config),
                                     std::ref(stat_collector));
            }

            // Wait for all threads to finish
            for (auto& thread : threads) { if (thread.joinable()) { thread.join(); } }
        }
    }

    void nb_layer_thread_work(Item item,
                              u64 n_assigned_threads,
                              std::atomic<u64>& n_inactive_threads,
                              std::vector<u64>& solution,
                              const Graph& original_g,
                              const AlgorithmConfiguration& config,
                              StatCollector& stat_collector) {
        // get more threads if available
        n_assigned_threads += n_inactive_threads.exchange(0);

        // references for better code readability
        const std::vector<u64>& hierarchy = config.hierarchy;
        const size_t l                    = hierarchy.size();
        const std::vector<u64>& index_vec = config.index_vec;
        const std::vector<u64>& k_rem_vec = config.k_rem_vec;
        const f64 global_imbalance        = config.imbalance;
        const u64 global_g_weight         = original_g.get_weight();
        const u64 global_k                = config.k;

        // load item to process
        const Graph& g                     = (*item.g);
        const TranslationTable& tt         = (*item.tt);
        const std::vector<u64>& identifier = (*item.identifier);

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

            // free item
            item.free();

            // make threads available
            n_inactive_threads += n_assigned_threads;
        } else {
            // create the subgraphs and place them in the next stack
            std::vector<Item> next_layer;
            create_sub_graphs(g, tt, local_k, partition, identifier, next_layer, depth, n_assigned_threads, stat_collector);

            // free item
            item.free();

            if (n_assigned_threads >= local_k) {
                // every item can be started with at least one thread

                // spawn threads
                std::vector<std::thread> threads;
                for (u64 t = 0; t < local_k; ++t) {
                    u64 n_local_assigned_threads = std::max((u64)1, n_assigned_threads / local_k) + (n_assigned_threads > local_k) * (t < n_assigned_threads % local_k);

                    threads.emplace_back(nb_layer_thread_work,
                                         next_layer[t],
                                         n_local_assigned_threads,
                                         std::ref(n_inactive_threads),
                                         std::ref(solution),
                                         std::ref(original_g),
                                         std::ref(config),
                                         std::ref(stat_collector));
                }

                // Wait for all threads to finish
                for (auto& thread : threads) { if (thread.joinable()) { thread.join(); } }
            } else {
                // too many threads than tasks

                // determine how many threads to start
                std::atomic<u64> global_idx = n_assigned_threads;

                // spawn threads
                std::vector<std::thread> threads;
                for (u64 t = 0; t < n_assigned_threads; ++t) {
                    u64 local_idx = t;

                    threads.emplace_back(nb_layer_thread_work_oversubscribed,
                                         local_idx,
                                         std::ref(global_idx),
                                         std::ref(next_layer),
                                         local_k,
                                         n_assigned_threads,
                                         std::ref(n_inactive_threads),
                                         std::ref(solution),
                                         std::ref(original_g),
                                         std::ref(config),
                                         std::ref(stat_collector));
                }

                // Wait for all threads to finish
                for (auto& thread : threads) { if (thread.joinable()) { thread.join(); } }
            }
        }
    }

    std::vector<u64> solve_nb_layer(const Graph& original_g,
                                    const AlgorithmConfiguration& config,
                                    StatCollector& stat_collector) {
        std::vector<u64> solution(original_g.get_n()); // end partition
        TranslationTable original_tt(original_g.get_n()); // default translation table

        // references for better code readability
        const u64 n_threads = config.n_threads;

        // initialize stack;
        Item item                           = {new std::vector<u64>(), const_cast<Graph*>(&original_g), &original_tt, false};
        std::atomic<u64> n_inactive_threads = 0;

        // process first item
        nb_layer_thread_work(item,
                             n_threads,
                             std::ref(n_inactive_threads),
                             std::ref(solution),
                             std::ref(original_g),
                             std::ref(config),
                             std::ref(stat_collector));

        return solution;
    }
}
