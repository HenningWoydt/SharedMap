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

#include "partition.h"

#include <libmtkahypar.h>
#include <libmtkahypartypes.h>
#include <kaHIP_interface.h>

namespace SharedMap {
    void kaffpa_partition(const Graph &g,
                          const u64 k,
                          const f64 imbalance,
                          std::vector<u64> &partition,
                          const u64 kaffpa_config,
                          const u64 seed) {
        ASSERT(imbalance >= 0.0);
        ASSERT(k > 0);

        // enough space
        partition.resize(g.get_n());

        // number of vertices and edges
        int n = (int) g.get_n();
        int m = (int) g.get_m();

        // vertex weights
        int      *v_weights = (int *) malloc(n * sizeof(int));
        for (int i          = 0; i < n; ++i) { v_weights[i] = (int) g.get_vertex_weight(i); }

        // pointer to adjacency lists
        int *adj_ptr   = (int *) malloc((n + 1) * sizeof(int));
        int *adj       = (int *) malloc(2 * m * sizeof(int));
        int *e_weights = (int *) malloc(2 * m * sizeof(int));

        int *insert_idx = (int *) malloc((n + 1) * sizeof(int));

        // set adj_ptr
        adj_ptr[0]    = 0;
        insert_idx[0] = 0;
        for (int u = 0; u < n; ++u) {
            adj_ptr[u + 1]    = adj_ptr[u] + (int) g.get_vertex_n_edge(u);
            insert_idx[u + 1] = insert_idx[u] + (int) g.get_vertex_n_edge(u);
        }

        // adjacency and edge weights
        for (int u = 0; u < n; ++u) {
            // process each vertex
            for (auto &e: g[u]) {
                adj[insert_idx[u]]       = (int) e.v;
                e_weights[insert_idx[u]] = (int) e.w;
                insert_idx[u]++;

                adj[insert_idx[e.v]]       = u;
                e_weights[insert_idx[e.v]] = (int) e.w;
                insert_idx[e.v]++;
            }
        }
        free(insert_idx);

        // number of partitions
        int n_partitions = (int) k;

        // imbalance
        double kaffpa_imbalance = imbalance;

        // edge cut
        int edge_cut;

        // partition result
        int *kaffpa_partition = (int *) malloc(n * sizeof(int));

        // mode
        int mode;
        switch (kaffpa_config) {
            case KAFFPA_STRONG:
                mode = STRONG;
                break;
            case KAFFPA_ECO:
                mode = ECO;
                break;
            case KAFFPA_FAST:
                mode = FAST;
                break;
            default:
                std::cerr << "Kaffpa Config " << kaffpa_config << " not known!" << std::endl;
                abort();
        }

        // execute kaffpa
        kaffpa(&n, v_weights, adj_ptr, e_weights, adj, &n_partitions, &kaffpa_imbalance, true, (int) seed, mode, &edge_cut, kaffpa_partition);

        // get the result
        for (int i = 0; i < n; ++i) {
            partition[i] = kaffpa_partition[i];
        }

        free(v_weights);
        free(adj_ptr);
        free(e_weights);
        free(adj);
        free(kaffpa_partition);
    }

    void mt_kahypar_partition(const Graph &g,
                              const u64 k,
                              const f64 imbalance,
                              std::vector<u64> &partition,
                              const u64 mt_kahypar_config,
                              const u64 n_threads,
                              const u64 seed) {
        ASSERT(imbalance >= 0.0);
        ASSERT(k > 0);

        // enough space
        partition.resize(g.get_n());

        // Initialize thread pool
        mt_kahypar_initialize_thread_pool(n_threads,
                                          false); // activate interleaved NUMA allocation policy

        // Setup partitioning context
        mt_kahypar_context_t     *context = mt_kahypar_context_new();
        mt_kahypar_preset_type_t preset;
        switch (mt_kahypar_config) {
            case MTKAHYPAR_DEFAULT:
                preset = DEFAULT;
                break;
            case MTKAHYPAR_QUALITY:
                preset = QUALITY;
                break;
            case MTKAHYPAR_HIGHEST_QUALITY:
                preset = HIGHEST_QUALITY;
                break;
            default:
                std::cerr << "Mt-KaHyPar Config " << mt_kahypar_config << " not known!" << std::endl;
                abort();
        }
        mt_kahypar_load_preset(context, preset);
        // In the following, we partition a hypergraph into two blocks
        // with an allowed imbalance of 3% and optimize the connective metric (KM1)
        mt_kahypar_set_partitioning_parameters(context,
                                               (int) k /* number of blocks */,
                                               imbalance /* imbalance parameter */,
                                               CUT /* objective function */);

        // set context
        mt_kahypar_set_context_parameter(context, VERBOSE, "0");
        mt_kahypar_set_seed(seed);

        // number of vertices and edges
        auto n = (mt_kahypar_hypernode_id_t) g.get_n();
        auto m = (mt_kahypar_hyperedge_id_t) g.get_m();

        // vertex weights
        auto     *v_weights = (mt_kahypar_hypernode_weight_t *) malloc(n * sizeof(mt_kahypar_hypernode_weight_t));
        for (u64 i          = 0; i < n; ++i) {
            v_weights[i] = (mt_kahypar_hypernode_weight_t) g.get_vertex_weight(i);
        }

        // edges
        u64      idx        = 0;
        auto     *edges     = (mt_kahypar_hypernode_id_t *) malloc(2 * m * sizeof(mt_kahypar_hypernode_id_t));
        auto     *e_weights = (mt_kahypar_hyperedge_weight_t *) malloc(m * sizeof(mt_kahypar_hyperedge_weight_t));
        for (u64 u          = 0; u < n; ++u) {
            for (auto &e: g[u]) {
                edges[2 * idx]     = (mt_kahypar_hypernode_id_t) u;
                edges[2 * idx + 1] = (mt_kahypar_hypernode_id_t) e.v;
                e_weights[idx]     = (mt_kahypar_hyperedge_weight_t) e.w;
                idx += 1;
                ASSERT(idx <= m);
            }
        }

        // Create graph
        mt_kahypar_hypergraph_t graph = mt_kahypar_create_graph(preset, n, m, edges, e_weights, v_weights);

        // Partition graph
        mt_kahypar_partitioned_hypergraph_t partitioned_hg = mt_kahypar_partition(graph, context);

        // Extract Partition
        auto mt_kahypar_partition = std::make_unique<mt_kahypar_partition_id_t[]>(mt_kahypar_num_hypernodes(graph));
        mt_kahypar_get_partition(partitioned_hg, mt_kahypar_partition.get());

        // move partition
        for (u64 i = 0; i < n; ++i) {
            partition[i] = mt_kahypar_partition[i];
            ASSERT(partition[i] < k);
        }

        free(v_weights);
        free(edges);
        free(e_weights);
        mt_kahypar_free_context(context);
        mt_kahypar_free_hypergraph(graph);
        mt_kahypar_free_partitioned_hypergraph(partitioned_hg);
    }
}
