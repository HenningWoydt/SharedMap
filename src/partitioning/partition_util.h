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

#ifndef SHAREDMAP_PARTITION_UTIL_H
#define SHAREDMAP_PARTITION_UTIL_H

#include <atomic>
#include <cmath>
#include <thread>

#include "src/datastructures/item.h"
#include "src/datastructures/translation_table.h"
#include "src/partitioning/partition.h"
#include "src/profiling/stat_collector.h"
#include "src/utility/definitions.h"

namespace SharedMap {
    inline void create_sub_graphs_serial(const CSRGraph &g,
                                         const TransTable &g_tt,
                                         const u64 k,
                                         const std::vector<u64> &partition,
                                         const std::vector<u64> &identifier,
                                         std::vector<Item> &temp_stack) {
        ASSERT(partition.size() == g.n);

        temp_stack.resize(k);

        for (u64 i = 0; i < k; ++i) {
            temp_stack[i].identifier = new std::vector<u64>(identifier);
            temp_stack[i].identifier->push_back(i);
            temp_stack[i].to_delete = true;
            temp_stack[i].tt = new TransTable(g.n, g.n / k);
        }

        std::vector<u64> n_s(k, 0);
        std::vector<u64> m_s(k, 0);
        std::vector<u64> w_s(k, 0);

        for (u64 u = 0; u < g.n; ++u) {
            u64 u_w = g.weights[u];
            u64 u_id = partition[u];

            temp_stack[u_id].tt->add(g_tt.get_o(u), n_s[u_id]);

            n_s[u_id] += 1;
            w_s[u_id] += u_w;
            for (u64 j = g.neighborhoods[u]; j < g.neighborhoods[u + 1]; ++j) {
                u64 v = g.edges_v[j];
                u64 v_id = partition[v];
                if (u_id == v_id) {
                    m_s[u_id] += 1;
                }
            }
        }

        for (u64 i = 0; i < k; ++i) {
            temp_stack[i].g = new CSRGraph(n_s[i], m_s[i], w_s[i]);
            temp_stack[i].tt->finalize();
        }

        std::vector<u64> idxs(k, 0);
        for (u64 u = 0; u < g.n; ++u) {
            u64 u_id = partition[u];

            u64 sub_u = temp_stack[u_id].tt->get_n(g_tt.get_o(u));

            temp_stack[u_id].g->weights[sub_u] = g.weights[u];

            for (u64 j = g.neighborhoods[u]; j < g.neighborhoods[u + 1]; ++j) {
                u64 v = g.edges_v[j];
                u64 w = g.edges_w[j];
                if (partition[v] == u_id) {
                    u64 idx = idxs[u_id];
                    u64 sub_v = temp_stack[u_id].tt->get_n(g_tt.get_o(v));
                    temp_stack[u_id].g->edges_v[idx] = sub_v;
                    temp_stack[u_id].g->edges_w[idx] = w;
                    idxs[u_id] += 1;
                }
            }
            temp_stack[u_id].g->neighborhoods[sub_u + 1] = idxs[u_id];
        }
    }

    inline void thread_create_sub_graphs_work(u64 local_idx,
                                              std::atomic<u64> &global_idx,
                                              const CSRGraph &g,
                                              const TransTable &g_tt,
                                              const u64 k,
                                              const std::vector<u64> &partition,
                                              const std::vector<u64> &identifier,
                                              std::vector<Item> &temp_stack) {
        while (local_idx < k) {
            // initialize the item
            temp_stack[local_idx].identifier = new std::vector<u64>(identifier);
            temp_stack[local_idx].identifier->push_back(local_idx);
            temp_stack[local_idx].to_delete = true;
            temp_stack[local_idx].tt = new TransTable(g.n, g.n / k);

            // initialize translation table and determine graph size
            u64 n = 0;
            u64 m = 0;
            u64 weight = 0;
            for (u64 u = 0; u < g.n; ++u) {
                if (local_idx == partition[u]) {
                    weight += g.weights[u];
                    temp_stack[local_idx].tt->add(g_tt.get_o(u), n);
                    for (u64 j = g.neighborhoods[u]; j < g.neighborhoods[u + 1]; ++j) {
                        u64 v = g.edges_v[j];
                        if (partition[u] == partition[v]) {
                            m += 1;
                        }
                    }
                    n += 1;
                }
            }

            // create graph, finalize translation table
            temp_stack[local_idx].g = new CSRGraph(n, m, weight);
            temp_stack[local_idx].tt->finalize();

            // create graph
            u64 idx = 0;
            for (u64 u = 0; u < g.n; ++u) {
                if (local_idx == partition[u]) {
                    u64 sub_u = temp_stack[local_idx].tt->get_n(g_tt.get_o(u));

                    temp_stack[local_idx].g->weights[sub_u] = g.weights[u];

                    for (u64 j = g.neighborhoods[u]; j < g.neighborhoods[u + 1]; ++j) {
                        u64 v = g.edges_v[j];
                        u64 w = g.edges_w[j];

                        if (partition[u] == partition[v]) {
                            u64 sub_v = temp_stack[local_idx].tt->get_n(g_tt.get_o(v));
                            temp_stack[local_idx].g->edges_v[idx] = sub_v;
                            temp_stack[local_idx].g->edges_w[idx] = w;
                            idx += 1;
                        }
                    }
                    temp_stack[local_idx].g->neighborhoods[sub_u + 1] = idx;
                }
            }

            // look for the next graph
            local_idx = global_idx.fetch_add(1);
        }
    }

    inline void create_sub_graphs_parallel(const CSRGraph &g,
                                           const TransTable &g_tt,
                                           const u64 n_threads,
                                           const u64 k,
                                           const std::vector<u64> &partition,
                                           const std::vector<u64> &identifier,
                                           std::vector<Item> &temp_stack) {
        ASSERT(partition.size() == g.n);

        temp_stack.resize(k);
        std::atomic<u64> global_idx = std::min(n_threads, k);

        // spawn threads
        std::vector<std::thread> threads;
        for (u64 t = 0; t < std::min(n_threads, k); ++t) {
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
        for (auto &thread: threads) { if (thread.joinable()) { thread.join(); } }
    }

    /**
     * Determine the adaptive imbalance.
     *
     * @param global_imbalance Global desired imbalance.
     * @param global_g_weight Original graph weight.
     * @param global_k Number of final partitions.
     * @param local_g_weight Local graph weight.
     * @param local_k_rem Local number of partitions.
     * @param depth Depth of the partitioning.
     * @return The adaptive imbalance.
     */
    inline f64 determine_adaptive_imbalance(f64 global_imbalance,
                                            u64 global_g_weight,
                                            u64 global_k,
                                            u64 local_g_weight,
                                            u64 local_k_rem,
                                            u64 depth) {
        f64 local_imbalance = (1.0 + global_imbalance) * ((f64) (local_k_rem * global_g_weight) / (f64) (global_k * local_g_weight));
        local_imbalance = std::pow(local_imbalance, (f64) 1 / (f64) depth) - 1.0;
        return local_imbalance;
    }

    /**
     * Partitions the graph.
     *
     * @param g The graph.
     * @param k The number of partitions.
     * @param imbalance The imbalance.
     * @param partition Space for the partition.
     * @param n_threads Number of threads to use.
     * @param depth Depth of the partitioning.
     * @param serial_alg_id Vector of serial algorithm ids.
     * @param parallel_alg_id Vector of parallel algorithm ids.
     * @param seed Seed for diversification.
     * @param stat_collector The statistic collector.
     */
    inline void partition_graph(const CSRGraph &g,
                                u64 k,
                                f64 imbalance,
                                std::vector<u64> &partition,
                                u64 n_threads,
                                u64 depth,
                                const std::vector<u64> &serial_alg_id,
                                const std::vector<u64> &parallel_alg_id,
                                u64 seed,
                                StatCollector &stat_collector) {
        auto sp = std::chrono::high_resolution_clock::now();

        u64 alg = parallel_alg_id[depth];
        if (n_threads == 1) {
            alg = serial_alg_id[depth];
        }

        if (k == 1) {
            // if only one partition
            partition.resize(g.n);
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
                                     g.n,
                                     alg,
                                     n_threads,
                                     imbalance,
                                     k,
                                     sp,
                                     ep);
    }

    /**
     * Creates subgraph of a graph and its partition.
     *
     * @param g The graph.
     * @param g_tt The translation table of the graph.
     * @param k The number of partitions.
     * @param partition The partition.
     * @param identifier The identifier of the graph.
     * @param temp_stack Space to put the partitioning tasks.
     * @param depth The depth of the creation.
     * @param n_threads The number of threads.
     * @param stat_collector The statistic collector.
     */
    inline void create_sub_graphs(const CSRGraph &g,
                                  const TransTable &g_tt,
                                  u64 k,
                                  const std::vector<u64> &partition,
                                  const std::vector<u64> &identifier,
                                  std::vector<Item> &temp_stack,
                                  u64 depth,
                                  u64 n_threads,
                                  StatCollector &stat_collector) {
        ASSERT(partition.size() == g.n);

        auto sp = std::chrono::high_resolution_clock::now();
        if (n_threads == 1) {
            create_sub_graphs_serial(g, g_tt, k, partition, identifier, temp_stack);
        } else {
            create_sub_graphs_parallel(g, g_tt, n_threads, k, partition, identifier, temp_stack);
        }
        auto ep = std::chrono::high_resolution_clock::now();

        stat_collector.log_subgraph_creation(depth,
                                             g.n,
                                             n_threads,
                                             k,
                                             sp,
                                             ep);
    }
}

#endif // SHAREDMAP_PARTITION_UTIL_H
