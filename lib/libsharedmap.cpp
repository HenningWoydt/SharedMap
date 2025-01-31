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

#include "include/libsharedmap.h"
#include "include/libsharedmaptypes.h"

#include "src/datastructures/graph.h"
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
    // create the graph
    SharedMap::Graph g(n);

    // fill in vertex weights
    for (int i = 0; i < n; ++i) { g.set_vertex_weight(i, v_weights[i]); }

    // add edges
    for (int i = 0; i < n; ++i) {
        for (int j = adj_ptrs[i]; j < adj_ptrs[i + 1]; ++j) {
            g.add_edge_if_not_exist(i, adj[j], adj_weights[j]);
        }
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
                                         n_threads,
                                         strategy,
                                         seed);

    // initialize the solver
    SharedMap::Solver solver(ac);

    // solve
    solver.solve(g, partition, comm_cost, verbose);
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

    SharedMap::Graph g(n);

    if (verbose) std::cout << prefix << "Asserting graph!" << std::endl;
    if (verbose) std::cout << prefix << "Graph has " << n << " vertices" << std::endl;
    if (verbose) std::cout << prefix << "Printing each vertex and its neighborhood" << std::endl;
    for (int i = 0; i < n; ++i) {
        if (verbose) std::cout << prefix << "Vertex " << i << " : weight = " << v_weights[i] << " neighborhood (neighbor, edge_weight):";

        if (v_weights[i] <= 0) {
            std::cout << prefix << "Vertex " << i << " has weight " << v_weights[i] << " <= 0, which is not allowed!" << std::endl;
            return false;
        }

        g.set_vertex_weight(i, v_weights[i]);
        for (int j = adj_ptrs[i]; j < adj_ptrs[i + 1]; ++j) {
            if (verbose) std::cout << "(" << adj[j] << ", " << adj_weights[j] << ") ";

            if (adj[j] == i) {
                std::cout << prefix << "Vertex " << i << " has itself as neighbor , which is not allowed!" << std::endl;
                return false;
            }

            if (adj[j] >= n) {
                std::cout << prefix << "Vertex " << i << " has neighbor " << adj[j] << " >= n (" << n << "), which is not allowed!" << std::endl;
                return false;
            }

            if (adj[j] < 0) {
                std::cout << prefix << "Vertex " << i << " has neighbor " << adj[j] << " < 0, which is not allowed!" << std::endl;
                return false;
            }

            if (adj_weights[j] <= 0) {
                std::cout << prefix << "Vertex " << i << " has neighbor " << adj[j] << " and the edge weight is " << adj_weights[j] << " <= 0, which is not allowed!" << std::endl;
                return false;
            }

            g.add_edge_if_not_exist(i, adj[j], adj_weights[j]);
        }
        if (verbose) std::cout << std::endl;
    }

    // check edges exist from both sides
    for (int n1 = 0; n1 < n; ++n1) {
        for (int j = adj_ptrs[n1]; j < adj_ptrs[n1 + 1]; ++j) {
            int  n2    = adj[j];
            int  w2    = adj_weights[j];
            bool found = false;

            for (int k = adj_ptrs[n2]; k < adj_ptrs[n2 + 1]; ++k) {
                int n3 = adj[k];
                int w3 = adj_weights[k];

                if (n1 == n3) {
                    if (w2 != w3) {
                        std::cout << prefix << "Vertex " << n1 << " is adjacent to vertex " << n2 << " with an edge weight of " << w2 << ", however vertex " << n2 << " is adjacent to vertex " << n1 << " with an edge weight of " << w3 << ", which is not allowed!" << std::endl;
                        return false;
                    }
                    found = true;
                    break;
                }

            }

            if (!found) {
                std::cout << prefix << "Vertex " << n1 << " has (neighbor, edge_weight) (" << n2 << ", " << w2 << "), however that edge is missing in " << n2 << "'s neighborhood, which is not allowed!" << std::endl;
                return false;
            }

        }
    }

    if (verbose) std::cout << prefix << "Asserting other parameters!" << std::endl;
    if (verbose) std::cout << prefix << "Hierarchy: ";
    for (int i = 0; i < l - 1; ++i) {
        if (verbose) std::cout << hierarchy[i] << ":";

        if(hierarchy[i] <= 0){
            std::cout << prefix << "Hierarchy position " << i << ", which is " << hierarchy[i] << " can not be <= 0" << std::endl;
            return false;
        }
    }
    if (verbose) std::cout << hierarchy[l - 1] << std::endl;
    if(hierarchy[l-1] <= 0){
        std::cout << prefix << "Hierarchy position " << l-1 << ", which is " << hierarchy[l-1] << " can not be <= 0" << std::endl;
        return false;
    }

    if (verbose) std::cout << prefix << "Distance : ";
    for (int i = 0; i < l - 1; ++i) {
        if (verbose) std::cout << distance[i] << ":";

        if(distance[i] < 0){
            std::cout << prefix << "Distance position " << i << ", which is " << distance[i] << " can not be < 0" << std::endl;
            return false;
        }

        if(distance[i] == 0){
            std::cout << prefix << "Warning: Distance position " << i << ", is 0, which might not be realistic! However, this triggers no error!" << std::endl;
        }

    }
    if (verbose) std::cout << distance[l - 1] << std::endl;
    if(distance[l - 1] < 0){
        std::cout << prefix << "Distance position " << l - 1 << ", which is " << distance[l - 1] << " can not be < 0" << std::endl;
        return false;
    }
    if(distance[l - 1] == 0){
        std::cout << prefix << "Warning: Distance position " << l - 1 << " is 0, which might not be realistic! However, this triggers no error!" << std::endl;
    }

    if (verbose) std::cout << prefix << "Imbalance: " << imbalance << std::endl;
    if(imbalance < 0){
        std::cout << prefix << "Imbalance " << imbalance << " < 0, which is not allowed!" << std::endl;
        return false;
    }
    if(imbalance == 0){
        std::cout << prefix << "Warning: imbalance is 0.0, which might not be practical!  However, this triggers no error!" << std::endl;
    }

    if (verbose) std::cout << prefix << "#Threads : " << n_threads << std::endl;
    if(n_threads <= 0){
        std::cout << prefix << "#Threads " << n_threads << " <= 0, which is not allowed!" << std::endl;
        return false;
    }

    if (verbose) std::cout << prefix << "Seed     : " << seed << std::endl;

    if (verbose) std::cout << prefix << "Strategy     : " << strategy << " (" << strategy_to_string(strategy) << ")" << std::endl;
    if(strategy_to_string(strategy) == "UNKNOWN"){
        std::cout << prefix << "Strategy " << strategy << " is not known! Please choose a valid strategy." << std::endl;
        return false;
    }

    if (verbose) std::cout << prefix << "Parallel Alg.: " << parallel_alg << " (" << algorithm_to_string(parallel_alg) << ")" << std::endl;
    if(algorithm_to_string(parallel_alg) == "UNKNOWN"){
        std::cout << prefix << "Algorithm " << parallel_alg << " is not known! Please choose a valid algorithm." << std::endl;
        return false;
    }

    if (verbose) std::cout << prefix << "Serial Alg.  : " << serial_alg << " (" << algorithm_to_string(serial_alg) << ")" << std::endl;
    if(algorithm_to_string(serial_alg) == "UNKNOWN"){
        std::cout << prefix << "Algorithm " << serial_alg << " is not known! Please choose a valid algorithm." << std::endl;
        return false;
    }

    return true;
}
