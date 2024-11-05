#include "partition_util.h"

#include "src/utility/algorithm_configuration.h"

namespace SharedMap {
    f64 determine_adaptive_imbalance(f64 global_imbalance,
                                     u64 global_g_weight,
                                     u64 global_k,
                                     u64 local_g_weight,
                                     u64 local_k_rem,
                                     u64 depth) {
        f64 local_imbalance = (1.0 + global_imbalance) * ((f64)(local_k_rem * global_g_weight) / (f64)(global_k * local_g_weight));
        local_imbalance     = std::pow(local_imbalance, (f64)1 / (f64)depth) - 1.0;
        return local_imbalance;
    }

    inline void create_sub_graphs_serial(const Graph& g,
                                         const TranslationTable& g_tt,
                                         const u64 k,
                                         const std::vector<u64>& partition,
                                         const std::vector<u64>& identifier,
                                         std::vector<Item>& temp_stack) {
        ASSERT(partition.size() == g.get_n());
        g.assert_graph();

        temp_stack.resize(k);

        for (u64 i = 0; i < k; ++i) {
            temp_stack[i].identifier = new std::vector<u64>(identifier);
            temp_stack[i].identifier->push_back(i);
            temp_stack[i].to_delete = true;
            temp_stack[i].tt = new TranslationTable();
        }

        std::vector<u64> g_sizes(k, 0);

        for (u64 u = 0; u < g.get_n(); ++u) {
            u64 p_id = partition[u];
            temp_stack[p_id].tt->add(g_tt.get_o(u), g_sizes[p_id]);
            g_sizes[p_id] += 1;
        }

        for (u64 i = 0; i < k; ++i) {
            temp_stack[i].g = new Graph(g_sizes[i]);
        }

        for (u64 u = 0; u < g.get_n(); ++u) {
            u64 p_id = partition[u];

            u64 sub_u = temp_stack[p_id].tt->get_n(g_tt.get_o(u));

            temp_stack[p_id].g->set_vertex_weight(sub_u, g.get_vertex_weight(u));

            for (const Edge& e : g[u]) {
                if (partition[e.v] == p_id) {
                    u64 sub_v = temp_stack[p_id].tt->get_n(g_tt.get_o(e.v));
                    temp_stack[p_id].g->add_edge(sub_u, sub_v, e.w);
                }
            }
        }
    }

    void create_sub_graphs(const Graph& g,
                           const TranslationTable& g_tt,
                           const u64 k,
                           const std::vector<u64>& partition,
                           const std::vector<u64>& identifier,
                           std::vector<Item>& temp_stack,
                           const u64 depth,
                           const u64 n_threads,
                           StatCollector& stat_collector) {
        ASSERT(partition.size() == g.get_n());
        g.assert_graph();

        auto sp = std::chrono::high_resolution_clock::now();
        create_sub_graphs_serial(g, g_tt, k, partition, identifier, temp_stack);
        auto ep  = std::chrono::high_resolution_clock::now();

        stat_collector.log_subgraph_creation(depth,
                                             g.get_n(),
                                             n_threads,
                                             k,
                                             sp,
                                             ep);
    }

    void partition_graph(const Graph& g,
                         const u64 k,
                         const f64 imbalance,
                         std::vector<u64>& partition,
                         const u64 n_threads,
                         const u64 depth,
                         const std::vector<u64>& serial_alg_id,
                         const std::vector<u64>& parallel_alg_id,
                         StatCollector& stat_collector) {
        auto sp = std::chrono::high_resolution_clock::now();

        u64 alg = parallel_alg_id[depth];
        if (n_threads == 1) {
            alg = serial_alg_id[depth];
        }

        switch (alg) {
            case GREEDY:
                greedy_partition(g, k, imbalance, partition);
                break;
            case KAFFPA_STRONG:
                kaffpa_partition(g, k, imbalance, partition, KAFFPA_STRONG);
                break;
            case KAFFPA_ECO:
                kaffpa_partition(g, k, imbalance, partition, KAFFPA_ECO);
                break;
            case KAFFPA_FAST:
                kaffpa_partition(g, k, imbalance, partition, KAFFPA_FAST);
                break;
            case MTKAHYPAR_DEFAULT:
                mt_kahypar_partition(g, k, imbalance, partition, MTKAHYPAR_DEFAULT, n_threads);
                break;
            case MTKAHYPAR_QUALITY:
                mt_kahypar_partition(g, k, imbalance, partition, MTKAHYPAR_QUALITY, n_threads);
                break;
            case MTKAHYPAR_HIGHEST_QUALITY:
                mt_kahypar_partition(g, k, imbalance, partition, MTKAHYPAR_HIGHEST_QUALITY, n_threads);
                break;
            default:
                std::cerr << "Algorithm ID " << alg << " not recognized" << std::endl;
                abort();
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
