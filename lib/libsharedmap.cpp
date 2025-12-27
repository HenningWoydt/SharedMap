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

#include <unordered_map>
#include <unordered_set>
#include <limits>

#include "include/libsharedmap.h"
#include "include/libsharedmaptypes.h"

#include "src/utility/algorithm_configuration.h"
#include "src/datastructures/solver.h"

std::string strategy_to_string(shared_map_strategy_type_t distribution) {
    switch (distribution) {
        case NAIVE:
            return "NAIVE";
        case LAYER:
            return "LAYER";
        case QUEUE:
            return "QUEUE";
        case NB_LAYER:
            return "NB_LAYER";
        default:
            return "UNKNOWN";
    }
}

std::string algorithm_to_string(shared_map_algorithm_type_t algorithm) {
    switch (algorithm) {
        case KAFFPA_FAST:
            return "KAFFPA_FAST";
        case KAFFPA_ECO:
            return "KAFFPA_ECO";
        case KAFFPA_STRONG:
            return "KAFFPA_STRONG";
        case MTKAHYPAR_DEFAULT:
            return "MTKAHYPAR_DEFAULT";
        case MTKAHYPAR_QUALITY:
            return "MTKAHYPAR_QUALITY";
        case MTKAHYPAR_HIGHEST_QUALITY:
            return "MTKAHYPAR_HIGHEST_QUALITY";
        default:
            return "UNKNOWN";
    }
}

void shared_map_hierarchical_multisection(int n,
                                          int *v_weights,
                                          int *adj_ptrs,
                                          int *adj_weights,
                                          int *adj,
                                          int *hierarchy,
                                          int *distance,
                                          int l,
                                          float imbalance,
                                          int n_threads,
                                          int seed,
                                          shared_map_strategy_type_t strategy,
                                          shared_map_algorithm_type_t parallel_alg,
                                          shared_map_algorithm_type_t serial_alg,
                                          int &comm_cost,
                                          int *partition,
                                          bool verbose) {
    ASSERT(n > 0);
    ASSERT(l >= 0);
    ASSERT(imbalance >= 0.0f);
    ASSERT(v_weights != nullptr);
    ASSERT(adj_ptrs != nullptr);
    ASSERT(adj != nullptr);
    ASSERT(adj_weights != nullptr);
    ASSERT(partition != nullptr);
    ASSERT(adj_ptrs[0] == 0);

    // m = number of directed adjacency entries in CSR
    const int m = adj_ptrs[n];
    ASSERT(m >= 0);

    // create CSR graph and fill directly (O(n+m))
    SharedMap::CSRGraph g;
    g.n = (SharedMap::u64) n;
    g.m = (SharedMap::u64) m;

    g.weights.resize(g.n);
    g.g_weight = 0;

    for (int i = 0; i < n; ++i) {
        ASSERT(v_weights[i] > 0);
        g.weights[(SharedMap::u64) i] = (SharedMap::u64) v_weights[i];
        g.g_weight += (SharedMap::u64) v_weights[i];
    }

    g.neighborhoods.resize((SharedMap::u64) n + 1);
    for (int i = 0; i <= n; ++i) {
        ASSERT(adj_ptrs[i] >= 0);
        g.neighborhoods[(SharedMap::u64) i] = (SharedMap::u64) adj_ptrs[i];
    }

    g.edges_v.resize(g.m);
    g.edges_w.resize(g.m);

    for (int e = 0; e < m; ++e) {
        const int v = adj[e];
        const int w = adj_weights[e];

        ASSERT(0 <= v && v < n);
        ASSERT(w > 0);

        g.edges_v[(SharedMap::u64) e] = (SharedMap::u64) v;
        g.edges_w[(SharedMap::u64) e] = (SharedMap::u64) w;
    }

    // create the hierarchy and distance
    std::vector<SharedMap::u64> hierarchy_vec(hierarchy, hierarchy + l);
    std::vector<SharedMap::u64> distance_vec(distance, distance + l);

    // create the algorithm configuration
    SharedMap::AlgorithmConfiguration ac(hierarchy_vec,
                                         distance_vec,
                                         imbalance,
                                         parallel_alg,
                                         serial_alg,
                                         (SharedMap::u64) n_threads,
                                         strategy,
                                         (SharedMap::u64) seed);

    // initialize the solver
    SharedMap::Solver solver(ac);

    // solve
    solver.solve(g, partition, comm_cost, verbose);
}

namespace {
    struct PairHash {
        std::size_t operator()(const std::pair<int, int> &p) const noexcept {
            return (std::hash<int>{}(p.first) ^ (std::hash<int>{}(p.second) << 1));
        }
    };
}

bool shared_map_hierarchical_multisection_assert_input(int n,
                                                       int *v_weights,
                                                       int *adj_ptrs,
                                                       int *adj_weights,
                                                       int *adj,
                                                       int *hierarchy,
                                                       int *distance,
                                                       int l,
                                                       float imbalance,
                                                       int n_threads,
                                                       int seed,
                                                       shared_map_strategy_type_t strategy,
                                                       shared_map_algorithm_type_t parallel_alg,
                                                       shared_map_algorithm_type_t serial_alg,
                                                       bool verbose) {
    std::string prefix = "---SharedMap--- ";

    // ---- basic null/size checks ----
    if (n <= 0) {
        std::cout << prefix << "n <= 0 is not allowed!\n";
        return false;
    }
    if (!v_weights || !adj_ptrs || !adj_weights || !adj || !hierarchy || !distance) {
        std::cout << prefix << "One or more input pointers are null!\n";
        return false;
    }
    if (l <= 0) {
        std::cout << prefix << "l <= 0 is not allowed!\n";
        return false;
    }

    // ---- CSR pointer validity ----
    if (adj_ptrs[0] != 0) {
        std::cout << prefix << "adj_ptrs[0] != 0 (is " << adj_ptrs[0] << "), invalid CSR!\n";
        return false;
    }
    for (int i = 0; i < n; ++i) {
        if (adj_ptrs[i] < 0 || adj_ptrs[i + 1] < 0) {
            std::cout << prefix << "adj_ptrs contains negative values, invalid CSR!\n";
            return false;
        }
        if (adj_ptrs[i] > adj_ptrs[i + 1]) {
            std::cout << prefix << "adj_ptrs not nondecreasing at i=" << i
                    << " (" << adj_ptrs[i] << " > " << adj_ptrs[i + 1] << ")\n";
            return false;
        }
    }
    const int m = adj_ptrs[n];
    if (m < 0) {
        std::cout << prefix << "adj_ptrs[n] < 0, invalid CSR!\n";
        return false;
    }

    if (verbose) {
        std::cout << prefix << "Asserting graph!\n";
        std::cout << prefix << "Graph has " << n << " vertices\n";
        std::cout << prefix << "Graph has " << m << " directed edges (CSR entries)\n";
        std::cout << prefix << "Printing each vertex and its neighborhood\n";
    }

    // ---- per-vertex checks + optional duplicate check ----
    // For duplicate detection per row:
    // - Use an unordered_set per row only if you really want it (can be expensive).
    // Here we do a lightweight check only when verbose (you can change this).
    for (int i = 0; i < n; ++i) {
        if (v_weights[i] <= 0) {
            std::cout << prefix << "Vertex " << i << " has weight " << v_weights[i]
                    << " <= 0, which is not allowed!\n";
            return false;
        }

        if (verbose) {
            std::cout << prefix << "Vertex " << i << " : weight = " << v_weights[i]
                    << " neighborhood (neighbor, edge_weight): ";
        }

        std::unordered_set<int> seen; // only used if verbose; remove if you don't want dup checks
        const int beg = adj_ptrs[i];
        const int end = adj_ptrs[i + 1];

        if (beg < 0 || end < 0 || beg > end || end > m) {
            std::cout << prefix << "CSR row bounds invalid for vertex " << i
                    << " : [" << beg << "," << end << ") with m=" << m << "\n";
            return false;
        }

        for (int j = beg; j < end; ++j) {
            const int nb = adj[j];
            const int ew = adj_weights[j];

            if (verbose) std::cout << "(" << nb << ", " << ew << ") ";

            if (nb == i) {
                std::cout << prefix << "Vertex " << i << " has itself as neighbor, which is not allowed!\n";
                return false;
            }
            if (nb < 0 || nb >= n) {
                std::cout << prefix << "Vertex " << i << " has neighbor " << nb
                        << " out of range [0," << n - 1 << "], not allowed!\n";
                return false;
            }
            if (ew <= 0) {
                std::cout << prefix << "Vertex " << i << " has neighbor " << nb
                        << " with edge weight " << ew << " <= 0, not allowed!\n";
                return false;
            }

            // duplicate neighbor in same row?
            if (verbose) {
                if (!seen.insert(nb).second) {
                    std::cout << "\n" << prefix << "Vertex " << i
                            << " has duplicate neighbor " << nb
                            << " in its CSR row (parallel edges). If this is intended, ignore; "
                            << "otherwise fix input.\n";
                    return false;
                }
            }
        }

        if (verbose) std::cout << "\n";
    }

    // ---- fast symmetry + weight consistency check (expected O(m)) ----
    // store canonical undirected edge (min(u,v), max(u,v)) -> weight
    std::unordered_map<std::pair<int, int>, int, PairHash> edge_weight;
    edge_weight.reserve((size_t) m);

    for (int u = 0; u < n; ++u) {
        for (int j = adj_ptrs[u]; j < adj_ptrs[u + 1]; ++j) {
            const int v = adj[j];
            const int w = adj_weights[j];

            const int a = (u < v) ? u : v;
            const int b = (u < v) ? v : u;
            auto key = std::make_pair(a, b);

            auto it = edge_weight.find(key);
            if (it == edge_weight.end()) {
                edge_weight.emplace(key, w);
            } else {
                // same undirected edge seen again: must match weight
                if (it->second != w) {
                    std::cout << prefix << "Edge (" << a << "," << b << ") appears with inconsistent weights: "
                            << it->second << " vs " << w << "\n";
                    return false;
                }
            }
        }
    }

    // Now ensure every undirected edge appears twice in CSR (u->v and v->u)
    // We do this by counting directed occurrences per undirected key.
    std::unordered_map<std::pair<int, int>, int, PairHash> dir_count;
    dir_count.reserve(edge_weight.size());

    for (int u = 0; u < n; ++u) {
        for (int j = adj_ptrs[u]; j < adj_ptrs[u + 1]; ++j) {
            const int v = adj[j];
            const int a = (u < v) ? u : v;
            const int b = (u < v) ? v : u;
            dir_count[std::make_pair(a, b)] += 1;
        }
    }

    for (const auto &kv: dir_count) {
        const auto &key = kv.first;
        const int count = kv.second;

        // For a simple undirected graph in CSR, each undirected edge should appear exactly twice.
        if (count != 2) {
            std::cout << prefix << "Undirected edge (" << key.first << "," << key.second
                    << ") appears " << count << " times in CSR, expected 2 (u->v and v->u)\n";
            return false;
        }
    }

    // ---- other parameters (your checks, with one small safety fix) ----
    if (verbose) std::cout << prefix << "Asserting other parameters!\n";

    if (verbose) std::cout << prefix << "Hierarchy: ";
    for (int i = 0; i < l; ++i) {
        if (verbose) std::cout << hierarchy[i] << (i + 1 < l ? ":" : "\n");
        if (hierarchy[i] <= 0) {
            std::cout << prefix << "Hierarchy position " << i << " is " << hierarchy[i] << " <= 0, not allowed!\n";
            return false;
        }
    }

    if (verbose) std::cout << prefix << "Distance : ";
    for (int i = 0; i < l; ++i) {
        if (verbose) std::cout << distance[i] << (i + 1 < l ? ":" : "\n");
        if (distance[i] < 0) {
            std::cout << prefix << "Distance position " << i << " is " << distance[i] << " < 0, not allowed!\n";
            return false;
        }
        if (distance[i] == 0) {
            std::cout << prefix << "Warning: Distance position " << i
                    << " is 0, might not be realistic (no error).\n";
        }
    }

    if (verbose) std::cout << prefix << "Imbalance: " << imbalance << "\n";
    if (imbalance < 0) {
        std::cout << prefix << "Imbalance " << imbalance << " < 0, not allowed!\n";
        return false;
    }
    if (imbalance == 0) {
        std::cout << prefix << "Warning: imbalance is 0.0 (no error).\n";
    }

    if (verbose) std::cout << prefix << "#Threads : " << n_threads << "\n";
    if (n_threads <= 0) {
        std::cout << prefix << "#Threads " << n_threads << " <= 0, not allowed!\n";
        return false;
    }

    if (verbose) std::cout << prefix << "Seed     : " << seed << "\n";

    if (verbose)
        std::cout << prefix << "Strategy     : " << strategy
                << " (" << strategy_to_string(strategy) << ")\n";
    if (strategy_to_string(strategy) == "UNKNOWN") {
        std::cout << prefix << "Strategy " << strategy << " is not known!\n";
        return false;
    }

    if (verbose)
        std::cout << prefix << "Parallel Alg.: " << parallel_alg
                << " (" << algorithm_to_string(parallel_alg) << ")\n";
    if (algorithm_to_string(parallel_alg) == "UNKNOWN") {
        std::cout << prefix << "Algorithm " << parallel_alg << " is not known!\n";
        return false;
    }

    if (verbose)
        std::cout << prefix << "Serial Alg.  : " << serial_alg
                << " (" << algorithm_to_string(serial_alg) << ")\n";
    if (algorithm_to_string(serial_alg) == "UNKNOWN") {
        std::cout << prefix << "Algorithm " << serial_alg << " is not known!\n";
        return false;
    }

    return true;
}
