#ifndef MT_RECPROMAP_PARTITION_H
#define MT_RECPROMAP_PARTITION_H

#include <memory>
#include <vector>
#include <iostream>
#include <thread>
#include <filesystem>

#include "src/utility/definitions.h"
#include "src/utility/macros.h"
#include "graph.h"
#include "translation_table.h"
#include "src/utility/algorthm_configuration.h"

#include "interface/kaHIP_interface.h"
#include <libmtkahypar.h>

namespace RecProMap {

    /**
     * Partitions a graph into k partitions in a greedy like fashion. This
     * function is mainly used for testing purposes.
     *
     * @param g The graph.
     * @param k The number of partitions.
     * @param imbalance The allowed imbalance.
     * @param partition The resulting partition.
     */
    void greedy_partition(const Graph &g,
                          u64 k,
                          f64 imbalance,
                          std::vector<u64> &partition);

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
                          u64 kaffpa_config);

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
                              u64 n_threads);

}

#endif //MT_RECPROMAP_PARTITION_H
