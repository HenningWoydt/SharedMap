#include "include/libsharedmap.h"
#include "include/libsharedmaptypes.h"

#include "src/datastructures/graph.h"
#include "src/utility/algorithm_configuration.h"
#include "src/profiling/stat_collector.h"
#include "src/datastructures/solver.h"

std::string distribution_to_string(shared_map_distribution_type_t distribution) {
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
                                          shared_map_distribution_type_t distribution,
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
                                         distribution,
                                         seed);

    // initialize the solver
    SharedMap::Solver solver(ac);

    // solve
    solver.solve(g, partition, verbose);
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
                                                       shared_map_distribution_type_t distribution,
                                                       shared_map_algorithm_type_t parallel_alg,
                                                       shared_map_algorithm_type_t serial_alg,
                                                       int &comm_cost,
                                                       int *partition,
                                                       bool verbose) {
    std::string prefix = "---SharedMap: ";

    SharedMap::Graph g(n);

    if (verbose) std::cout << prefix << "Asserting graph!" << std::endl;
    if (verbose) std::cout << prefix << "Graph has " << n << " vertices" << std::endl;
    if (verbose) std::cout << prefix << "Printing each vertex and its neighborhood" << std::endl;
    for (int i = 0; i < n; ++i) {
        if (verbose) std::cout << prefix << "Vertex " << i << " : weight = " << v_weights[i] << " neighborhood (neighbor, edge_weight):";

        if (v_weights[i] <= 0) {
            std::cout << prefix << "Vertex " << i << " has weight " << v_weights[i] << " <= 0, which is not allowed!" << std::endl;
            return true;
        }

        g.set_vertex_weight(i, v_weights[i]);
        for (int j = adj_ptrs[i]; j < adj_ptrs[i + 1]; ++j) {
            if (verbose) std::cout << "(" << adj[j] << ", " << adj_weights[j] << ") ";

            if (adj[j] == i) {
                std::cout << prefix << "Vertex " << i << " has itself as neighbor , which is not allowed!" << std::endl;
                return true;
            }

            if (adj[j] >= n) {
                std::cout << prefix << "Vertex " << i << " has neighbor " << adj[j] << " >= n (" << n << "), which is not allowed!" << std::endl;
                return true;
            }

            if (adj[j] < 0) {
                std::cout << prefix << "Vertex " << i << " has neighbor " << adj[j] << " < 0, which is not allowed!" << std::endl;
                return true;
            }

            if (adj_weights[j] <= 0) {
                std::cout << prefix << "Vertex " << i << " has neighbor " << adj[j] << " and the edge weight is " << adj_weights[j] << " <= 0, which is not allowed!" << std::endl;
                return true;
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

            for (int k = adj_ptrs[n2]; k < adj_ptrs[n2 + 1] && !false; ++k) {
                int n3 = adj[k];
                int w3 = adj_weights[k];

                if (n1 == n3) {
                    found = true;
                    if (w2 != w3) {
                        std::cout << prefix << "Vertex " << n1 << " is adjacent to vertex " << n2 << " with an edge weight of " << w2 << ", however vertex " << n2 << " is adjacent to vertex " << n1 << " with an edge weight of " << w3 << ", which is not allowed!" << std::endl;
                        return false;
                    }
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
    }
    if (verbose) std::cout << hierarchy[l - 1] << std::endl;
    if (verbose) std::cout << prefix << "Distance : ";
    for (int i = 0; i < l - 1; ++i) {
        if (verbose) std::cout << distance[i] << ":";
    }
    if (verbose) std::cout << distance[l - 1] << std::endl;

    if (verbose) std::cout << prefix << "Imbalance: " << imbalance << std::endl;
    if (verbose) std::cout << prefix << "#Threads : " << n_threads << std::endl;
    if (verbose) std::cout << prefix << "Seed     : " << seed << std::endl;

    if (verbose) std::cout << prefix << "Distribution : " << distribution << " (" << distribution_to_string(distribution) << ")" << std::endl;
    if (verbose) std::cout << prefix << "Parallel Alg.: " << parallel_alg << " (" << algorithm_to_string(parallel_alg) << ")" << std::endl;
    if (verbose) std::cout << prefix << "Serial Alg.  : " << serial_alg << " (" << algorithm_to_string(serial_alg) << ")" << std::endl;

    return false;
}
