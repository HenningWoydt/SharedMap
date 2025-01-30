#ifndef SHAREDMAP_LIBSHAREDMAP_H
#define SHAREDMAP_LIBSHAREDMAP_H

#include "libsharedmaptypes.h"

/**
 * TODO:
 *
 * @param graph
 * @param hierarchy
 * @param distance
 * @param imbalance
 * @param threads
 * @param seed
 * @param distribution
 * @param parallel
 * @param serial
 * @param partition
 */
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
                                          bool verbose);


/**
 * TODO:
 *
 * @param graph
 * @param hierarchy
 * @param distance
 * @param imbalance
 * @param threads
 * @param seed
 * @param distribution
 * @param parallel
 * @param serial
 * @param partition
 */
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
                                                       bool verbose);

#endif //SHAREDMAP_LIBSHAREDMAP_H
