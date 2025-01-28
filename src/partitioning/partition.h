#ifndef SHAREDMAP_PARTITION_H
#define SHAREDMAP_PARTITION_H

#include <vector>

#include "src/datastructures/graph.h"
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
     */
    void kaffpa_partition(const Graph &g,
                          u64 k,
                          f64 imbalance,
                          std::vector<u64> &partition,
                          u64 kaffpa_config,
                          u64 seed);

    /**
     * Uses MtKaHyPar to partition a graph into k partitions.
     *
     * @param g The graph.
     * @param k The number of partitions.
     * @param imbalance The allowed imbalance.
     * @param partition The resulting partition.
     * @param mt_kahypar_config Configuration for MtKaHyPar.
     * @param n_threads Number of cores to use.
     */
    void mt_kahypar_partition(const Graph &g,
                              u64 k,
                              f64 imbalance,
                              std::vector<u64> &partition,
                              u64 mt_kahypar_config,
                              u64 n_threads,
                              u64 seed);
}

#endif //SHAREDMAP_PARTITION_H
