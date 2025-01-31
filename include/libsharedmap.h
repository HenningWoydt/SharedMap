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

#ifndef SHAREDMAP_LIBSHAREDMAP_H
#define SHAREDMAP_LIBSHAREDMAP_H

#include "libsharedmaptypes.h"

/**
 * Executes parallel shared-memory hierarchical multisection.
 *
 * The graph is needed in CSR format.
 * The graphs vertices should be labeled [0, ..., n-1].
 * The vertex weights must be >= 1.
 * An undirected edge {u, v} must be present two times, once in the neighborhood of u and once in the neighborhood of v.
 * Edge weights must be >= 1.
 *
 * To check whether correct input is supplied, you can use the function shared_map_hierarchical_multisection_assert_input .
 *
 * @param n Number of vertices of the graph.
 * @param v_weights Array of n integers, each >= 1.
 * @param adj_ptrs Array of n+1 integer, with adj_ptrs[i] and with adj_ptrs[i+1] pointing to the inclusive start and exclusive end of each neighborhood.
 * @param adj_weights Array of m integers each holding the weight of the edge.
 * @param adj Array of m integers, each holding the neighbor vertex.
 * @param hierarchy Array of l integers, describing the hierarchy.
 * @param distance Array of l integers, describing the distance.
 * @param l Length of the hierarchy and distance.
 * @param imbalance The allowed imbalance, for example 0.03 for a maximum allowed imbalance of 3%.
 * @param n_threads The number of threads to use.
 * @param seed The random seed for diversifying partitioning results.
 * @param strategy Which thread distribution strategy to use. Allowed are {NAIVE, LAYER, QUEUE, NB_LAYER}.
 * @param parallel_alg Which parallel algorithm to use. Recommended are {MTKAHYPAR_DEFAULT, MTKAHYPAR_QUALITY, MTKAHYPAR_HIGHEST_QUALITY}.
 * @param serial_alg Which serial algorithm to use. Recommended are {KAFFPA_FAST, KAFFPA_ECO, KAFFPA_STRONG,}.
 * @param comm_cost The resulting communication cost.
 * @param partition Array of n integers, will hold the resulting partition.
 * @param verbose Whether to print statistics.
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
                                          shared_map_strategy_type_t strategy,
                                          shared_map_algorithm_type_t parallel_alg,
                                          shared_map_algorithm_type_t serial_alg,
                                          int &comm_cost,
                                          int *partition,
                                          bool verbose);


/**
 * Checks if the given input can be read and is consistent. Use this function to
 * assert that the given input to shared_map_hierarchical_multisection is
 * correct in case any errors occur or your program crashed.
 *
 * If the input is correct then the function will return true, otherwise it will return false.
 *
 * @param n Number of vertices of the graph.
 * @param v_weights Array of n integers, each >= 1.
 * @param adj_ptrs Array of n+1 integer, with adj_ptrs[i] and with adj_ptrs[i+1] pointing to the inclusive start and exclusive end of each neighborhood.
 * @param adj_weights Array of m integers each holding the weight of the edge.
 * @param adj Array of m integers, each holding the neighbor vertex.
 * @param hierarchy Array of l integers, describing the hierarchy.
 * @param distance Array of l integers, describing the distance.
 * @param l Length of the hierarchy and distance.
 * @param imbalance The allowed imbalance, for example 0.03 for a maximum allowed imbalance of 3%.
 * @param n_threads The number of threads to use.
 * @param seed The random seed for diversifying partitioning results.
 * @param strategy Which thread distribution strategy to use. Allowed are {NAIVE, LAYER, QUEUE, NB_LAYER}.
 * @param parallel_alg Which parallel algorithm to use. Recommended are {MTKAHYPAR_DEFAULT, MTKAHYPAR_QUALITY, MTKAHYPAR_HIGHEST_QUALITY}.
 * @param serial_alg Which serial algorithm to use. Recommended are {KAFFPA_FAST, KAFFPA_ECO, KAFFPA_STRONG,}.
 * @param comm_cost The resulting communication cost.
 * @param partition Array of n integers, will hold the resulting partition.
 * @param verbose Whether to print statistics.
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
                                                       shared_map_strategy_type_t distribution,
                                                       shared_map_algorithm_type_t parallel_alg,
                                                       shared_map_algorithm_type_t serial_alg,
                                                       bool verbose);

#endif //SHAREDMAP_LIBSHAREDMAP_H
