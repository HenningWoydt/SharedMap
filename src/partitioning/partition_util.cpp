#include "partition_util.h"

namespace SharedMap {
    f64 determine_adaptive_imbalance(const f64 global_imbalance,
                                     const u64 global_g_weight,
                                     const u64 global_k,
                                     const u64 local_g_weight,
                                     const u64 local_k_rem,
                                     const u64 depth) {
        f64 local_imbalance = (1.0 + global_imbalance) * ((f64) (local_k_rem * global_g_weight) / (f64) (global_k * local_g_weight));
        local_imbalance = std::pow(local_imbalance, (f64) 1 / (f64) depth) - 1.0;
        return local_imbalance;
    }

    inline void create_sub_graphs_serial(const Graph &g,
                                         const TranslationTable &g_tt,
                                         const u64 k,
                                         const std::vector<u64> &partition,
                                         const std::vector<u64> &identifier,
                                         std::vector<Item> &temp_stack) {
        ASSERT(partition.size() == g.get_n());

        temp_stack.resize(k);

        for (u64 i = 0; i < k; ++i) {
            temp_stack[i].identifier = new std::vector<u64>(identifier);
            temp_stack[i].identifier->push_back(i);
            temp_stack[i].to_delete = true;
            temp_stack[i].tt        = new TranslationTable();
        }

        std::vector<u64> g_sizes(k, 0);

        for (u64 u = 0; u < g.get_n(); ++u) {
            u64 p_id = partition[u];
            temp_stack[p_id].tt->add(g_tt.get_o(u), g_sizes[p_id]);
            g_sizes[p_id] += 1;
        }

        for (u64 i = 0; i < k; ++i) {
            temp_stack[i].g = new Graph(g_sizes[i]);
            temp_stack[i].tt->finalize();
        }

        for (u64 u = 0; u < g.get_n(); ++u) {
            u64 p_id = partition[u];

            u64 sub_u = temp_stack[p_id].tt->get_n(g_tt.get_o(u));

            temp_stack[p_id].g->set_vertex_weight(sub_u, g.get_vertex_weight(u));

            for (const Edge &e: g[u]) {
                if (partition[e.v] == p_id) {
                    u64 sub_v = temp_stack[p_id].tt->get_n(g_tt.get_o(e.v));
                    temp_stack[p_id].g->add_edge(sub_u, sub_v, e.w);
                }
            }
        }
    }

    inline void thread_create_sub_graphs_work(u64 local_idx,
                                              std::atomic<u64> &global_idx,
                                              const Graph &g,
                                              const TranslationTable &g_tt,
                                              const u64 k,
                                              const std::vector<u64> &partition,
                                              const std::vector<u64> &identifier,
                                              std::vector<Item> &temp_stack) {
        while (local_idx < k) {
            // initialize the item
            temp_stack[local_idx].identifier = new std::vector<u64>(identifier);
            temp_stack[local_idx].identifier->push_back(local_idx);
            temp_stack[local_idx].to_delete = true;
            temp_stack[local_idx].tt        = new TranslationTable();

            // initialize translation table and determine graph size
            u64      g_size = 0;
            for (u64 u      = 0; u < g.get_n(); ++u) {
                if (local_idx == partition[u]) {
                    temp_stack[local_idx].tt->add(g_tt.get_o(u), g_size);
                    g_size += 1;
                }
            }

            // create graph, finalize translation table
            temp_stack[local_idx].g = new Graph(g_size);
            temp_stack[local_idx].tt->finalize();

            // create graph
            for (u64 u = 0; u < g.get_n(); ++u) {
                if (local_idx == partition[u]) {
                    u64 sub_u = temp_stack[local_idx].tt->get_n(g_tt.get_o(u));

                    temp_stack[local_idx].g->set_vertex_weight(sub_u, g.get_vertex_weight(u));

                    for (const Edge &e: g[u]) {
                        if (partition[e.v] == local_idx) {
                            u64 sub_v = temp_stack[local_idx].tt->get_n(g_tt.get_o(e.v));
                            temp_stack[local_idx].g->add_edge(sub_u, sub_v, e.w);
                        }
                    }
                }
            }

            // look for the next graph
            local_idx = global_idx.fetch_add(1);
        }
    }

    inline void create_sub_graphs_parallel(const Graph &g,
                                           const TranslationTable &g_tt,
                                           const u64 n_threads,
                                           const u64 k,
                                           const std::vector<u64> &partition,
                                           const std::vector<u64> &identifier,
                                           std::vector<Item> &temp_stack) {
        ASSERT(partition.size() == g.get_n());

        temp_stack.resize(k);
        std::atomic<u64> global_idx = std::min(n_threads, k);

        // spawn threads
        std::vector<std::thread> threads;
        for (u64                 t = 0; t < std::min(n_threads, k); ++t) {
            threads.emplace_back(thread_create_sub_graphs_work,
                                 t,
                                 std::ref(global_idx),
                                 std::ref(g),
                                 std::ref(g_tt),
                                 k,
                                 std::ref(partition),
                                 std::ref(identifier),
                                 std::ref(temp_stack));
        }

        // Wait for all threads to finish
        for (auto &thread: threads) { if (thread.joinable()) { thread.join(); }}
    }

    void create_sub_graphs(const Graph &g,
                           const TranslationTable &g_tt,
                           const u64 k,
                           const std::vector<u64> &partition,
                           const std::vector<u64> &identifier,
                           std::vector<Item> &temp_stack,
                           const u64 depth,
                           const u64 n_threads,
                           StatCollector &stat_collector) {
        ASSERT(partition.size() == g.get_n());

        auto sp = std::chrono::high_resolution_clock::now();
        if (n_threads == 1) {
            create_sub_graphs_serial(g, g_tt, k, partition, identifier, temp_stack);
        } else {
            create_sub_graphs_parallel(g, g_tt, n_threads, k, partition, identifier, temp_stack);
        }
        auto ep = std::chrono::high_resolution_clock::now();

        stat_collector.log_subgraph_creation(depth,
                                             g.get_n(),
                                             n_threads,
                                             k,
                                             sp,
                                             ep);
    }

    void partition_graph(const Graph &g,
                         const u64 k,
                         const f64 imbalance,
                         std::vector<u64> &partition,
                         const u64 n_threads,
                         const u64 depth,
                         const std::vector<u64> &serial_alg_id,
                         const std::vector<u64> &parallel_alg_id,
                         const u64 seed,
                         StatCollector &stat_collector) {
        auto sp = std::chrono::high_resolution_clock::now();

        u64 alg = parallel_alg_id[depth];
        if (n_threads == 1) {
            alg = serial_alg_id[depth];
        }

        if (k == 1) {
            partition.resize(g.get_n());
            std::fill(partition.begin(), partition.end(), 0);
        } else {
            switch (alg) {
                case KAFFPA_STRONG:
                    kaffpa_partition(g, k, imbalance, partition, KAFFPA_STRONG, seed);
                    break;
                case KAFFPA_ECO:
                    kaffpa_partition(g, k, imbalance, partition, KAFFPA_ECO, seed);
                    break;
                case KAFFPA_FAST:
                    kaffpa_partition(g, k, imbalance, partition, KAFFPA_FAST, seed);
                    break;
                case MTKAHYPAR_DEFAULT:
                    mt_kahypar_partition(g, k, imbalance, partition, MTKAHYPAR_DEFAULT, n_threads, seed);
                    break;
                case MTKAHYPAR_QUALITY:
                    mt_kahypar_partition(g, k, imbalance, partition, MTKAHYPAR_QUALITY, n_threads, seed);
                    break;
                case MTKAHYPAR_HIGHEST_QUALITY:
                    mt_kahypar_partition(g, k, imbalance, partition, MTKAHYPAR_HIGHEST_QUALITY, n_threads, seed);
                    break;
                default:
                    std::cerr << "Algorithm ID " << alg << " not recognized" << std::endl;
                    abort();
            }
        }

        auto ep = std::chrono::high_resolution_clock::now();

        stat_collector.log_partition(depth,
                                     g.get_n(),
                                     alg,
                                     n_threads,
                                     imbalance,
                                     k,
                                     sp,
                                     ep);
    }
}
