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

#ifndef SHAREDMAP_PARTITION_H
#define SHAREDMAP_PARTITION_H

#include <vector>
#include <iostream>

#include <mtkahypar.h>
#include <mtkahypartypes.h>
#include <kaHIP_interface.h>

#include "src/utility/algorithm_configuration.h"
#include "src/utility/definitions.h"

namespace SharedMap {
    /**
     * Uses Kaffpa to partition a graph into k partitions.
     *
     * @param g The graph.
     * @param k The number of partitions.
     * @param imbalance The allowed imbalance.
     * @param partition The resulting partition.
     * @param kaffpa_config Configuration for kaffpa.
     * @param seed The random seed.
     */
    inline void kaffpa_partition(const CSRGraph &g,
                                 u64 k,
                                 f64 imbalance,
                                 std::vector<u64> &partition,
                                 u64 kaffpa_config,
                                 u64 seed) {
        if (imbalance < 0.0) {
            // if this happens then a previous partition was imbalanced and an imbalanced solution is guaranteed
            // simply continue, but try to keep imbalance low
            imbalance = 0.001; // this is 0.1%
        }
        ASSERT(k > 0);

        // enough space
        partition.resize(g.n);

        // number of vertices and edges
        int n = (int) g.n;
        int m = (int) g.m;

        // vertex weights
        int *v_weights = (int *) malloc(n * sizeof(int));
        for (int i = 0; i < n; ++i) { v_weights[i] = (int) g.weights[i]; }

        // pointer to adjacency lists
        int *adj_ptr = (int *) malloc((n + 1) * sizeof(int));
        adj_ptr[0] = 0;
        int *adj = (int *) malloc(m * sizeof(int));
        int *e_weights = (int *) malloc(m * sizeof(int));

        // adjacency and edge weights
        u64 idx = 0;
        for (int u = 0; u < n; ++u) {
            // process each vertex
            for (u64 j = g.neighborhoods[u]; j < g.neighborhoods[u + 1]; ++j) {
                u64 v = g.edges_v[j];
                u64 w = g.edges_w[j];

                adj[idx] = (int) v;
                e_weights[idx] = (int) w;
                idx += 1;
            }
            adj_ptr[u + 1] = idx;
        }

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

    /**
     * Uses MtKaHyPar to partition a graph into k partitions.
     *
     * @param g The graph.
     * @param k The number of partitions.
     * @param imbalance The allowed imbalance.
     * @param partition The resulting partition.
     * @param mt_kahypar_config Configuration for MtKaHyPar.
     * @param n_threads Number of cores to use.
     * @param seed The random seed.
     */
    inline void mt_kahypar_partition(const CSRGraph &g,
                                     u64 k,
                                     f64 imbalance,
                                     std::vector<u64> &partition,
                                     u64 mt_kahypar_config,
                                     u64 n_threads,
                                     u64 seed) {
        if (imbalance < 0.0) {
            // if this happens then a previous partition was imbalanced and an imbalanced solution is guaranteed
            // simply continue, but try to keep imbalance low
            imbalance = 0.001; // this is 0.1%
        }
        ASSERT(k > 0);

        partition.resize(g.n);

        mt_kahypar_error_t error{};
        error.status = SUCCESS;
        error.msg = nullptr;
        error.msg_len = 0;

        // Init library (thread pool, etc.)
        mt_kahypar_initialize((size_t) n_threads,
                              false /* interleaved NUMA allocation policy */);

        // Preset -> context
        mt_kahypar_preset_type_t preset;
        switch (mt_kahypar_config) {
            case MTKAHYPAR_DEFAULT: preset = DEFAULT;
                break;
            case MTKAHYPAR_QUALITY: preset = QUALITY;
                break;
            case MTKAHYPAR_HIGHEST_QUALITY: preset = HIGHEST_QUALITY;
                break;
            default:
                std::cerr << "Mt-KaHyPar Config " << mt_kahypar_config << " not known!\n";
                std::abort();
        }

        mt_kahypar_context_t *context = mt_kahypar_context_from_preset(preset);
        if (!context) {
            std::cerr << "mt_kahypar_context_from_preset returned null\n";
            std::abort();
        }

        mt_kahypar_set_partitioning_parameters(context,
                                               (mt_kahypar_partition_id_t) k,
                                               (double) imbalance,
                                               CUT /* objective for graphs */);

        mt_kahypar_set_seed((uint64_t) seed);

        // Optional: silence logging
        mt_kahypar_set_context_parameter(context, VERBOSE, "0", &error);
        if (error.status != SUCCESS) {
            std::cerr << "mt_kahypar_set_context_parameter failed: " << error.msg << "\n";
            std::abort();
        }

        const auto n = (mt_kahypar_hypernode_id_t) g.n;

        // Vertex weights
        std::vector<mt_kahypar_hypernode_weight_t> v_weights((size_t) n);
        for (u64 i = 0; i < (u64) n; ++i) {
            v_weights[i] = (mt_kahypar_hypernode_weight_t) g.weights[i];
        }

        // Build undirected edge list: each undirected edge becomes a 2-pin "hyperedge"
        // We don't know ahead of time how many u<v edges there are, so reserve generously.
        // Worst-case (if input stores each undirected edge once already): idx can be up to g.m.
        std::vector<mt_kahypar_hypernode_id_t> edges;
        std::vector<mt_kahypar_hyperedge_weight_t> e_weights;
        edges.reserve((size_t) 2 * (size_t) g.m);
        e_weights.reserve((size_t) g.m);

        for (u64 u = 0; u < (u64) n; ++u) {
            for (u64 j = g.neighborhoods[u]; j < g.neighborhoods[u + 1]; ++j) {
                const u64 v = g.edges_v[j];
                const u64 w = g.edges_w[j];
                if (u >= v) continue;

                edges.push_back((mt_kahypar_hypernode_id_t) u);
                edges.push_back((mt_kahypar_hypernode_id_t) v);
                e_weights.push_back((mt_kahypar_hyperedge_weight_t) w);
            }
        }

        const auto num_edges = (mt_kahypar_hyperedge_id_t) e_weights.size();
        if (num_edges == 0) {
            // Degenerate graph: all isolated / no u<v edges
            std::fill(partition.begin(), partition.end(), 0);
            mt_kahypar_free_context(context);
            return;
        }

        mt_kahypar_hypergraph_t hg = mt_kahypar_create_graph(context,
                                                             n,
                                                             num_edges,
                                                             edges.data(),
                                                             e_weights.data(),
                                                             v_weights.data(),
                                                             &error);


        if (error.status != SUCCESS) {
            std::cerr << "mt_kahypar_create_graph failed: " << error.msg << "\n";
            std::abort();
        }

        mt_kahypar_partitioned_hypergraph_t phg =
                mt_kahypar_partition(hg, context, &error);

        if (error.status != SUCCESS) {
            std::cerr << "mt_kahypar_partition failed: " << error.msg << "\n";
            std::abort();
        }

        auto part = std::make_unique<mt_kahypar_partition_id_t[]>(mt_kahypar_num_hypernodes(hg));
        mt_kahypar_get_partition(phg, part.get());

        for (u64 i = 0; i < (u64) n; ++i) {
            partition[i] = (u64) part[i];
            ASSERT(partition[i] < k);
        }

        mt_kahypar_free_partitioned_hypergraph(phg);
        mt_kahypar_free_hypergraph(hg);
        mt_kahypar_free_context(context);
    }
}

#endif //SHAREDMAP_PARTITION_H
