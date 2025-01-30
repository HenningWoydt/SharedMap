#include "include/libsharedmap.h"
#include "include/libsharedmaptypes.h"

#include "src/datastructures/graph.h"
#include "src/utility/algorithm_configuration.h"
#include "src/profiling/stat_collector.h"
#include "src/datastructures/solver.h"

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
                                          shared_map_partitioning_type_t parallel_alg,
                                          shared_map_partitioning_type_t serial_alg,
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


void shared_map_hierarchical_multisection_assert_input(int n,
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
                                                       shared_map_partitioning_type_t parallel_alg,
                                                       shared_map_partitioning_type_t serial_alg,
                                                       int &comm_cost,
                                                       int *partition,
                                                       bool verbose);
