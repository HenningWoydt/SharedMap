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
                                                       shared_map_partitioning_type_t parallel_alg,
                                                       shared_map_partitioning_type_t serial_alg,
                                                       int &comm_cost,
                                                       int *partition,
                                                       bool verbose){
    std::string prefix = "---SharedMap: ";
    if(verbose) {
        std::cout << prefix << "Asserting graph!" << std::endl;
        std::cout << prefix << "Graph has " << n << " vertices" << std::endl;
        std::cout << prefix << "Printing each vertex and its neighborhood" << std::endl;
        for(int i = 0; i < n; ++i){
            std::cout << prefix << "Vertex " << i << " : weight = " << v_weights[i] << " neighborhood (neighbor, edge_weight):";
            for(int j = adj_ptrs[i]; j < adj_ptrs[i+1]; ++j){
                std::cout << "(" << adj[j] << ", " << adj_weights[j] << ") ";
            }
            std::cout << std::endl;
        }

        std::cout << prefix << "Asserting other parameters!" << std::endl;
        std::cout << prefix << "Hierarchy: ";
        for(int i = 0; i < l-1; ++i){
            std::cout << hierarchy[i] << ":";
        }
        std::cout << hierarchy[l-1] << std::endl;
        std::cout << prefix << "Distance: ";
        for(int i = 0; i < l-1; ++i){
            std::cout << distance[i] << ":";
        }
        std::cout << distance[l-1] << std::endl;

        std::cout << prefix << "Imbalance: " << imbalance << std::endl;
        std::cout << prefix << "#Threads : " << n_threads << std::endl;
        std::cout << prefix << "Seed     : " << seed << std::endl;

        std::cout << prefix << "Distribution : " << distribution << std::endl;
        std::cout << prefix << "Parallel Alg.: " << parallel_alg << std::endl;
        std::cout << prefix << "Serial Alg.  : " << serial_alg << std::endl;
    }

    return false;
}
