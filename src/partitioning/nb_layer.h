#ifndef SHAREDMAP_NB_LAYER_H
#define SHAREDMAP_NB_LAYER_H

#include <atomic>
#include <thread>
#include <vector>

#include "src/datastructures/graph.h"
#include "src/datastructures/item.h"
#include "src/datastructures/item.h"
#include "src/datastructures/translation_table.h"
#include "src/partitioning/partition_util.h"
#include "src/profiling/stat_collector.h"
#include "src/utility/algorithm_configuration.h"
#include "src/utility/definitions.h"

namespace SharedMap {
    /**
     * Uses the non-blocking layer approach.
     *
     * @param original_g The original graph.
     * @param config The algorithm configuration.
     * @param stat_collector The statistic collector.
     * @return The partition.
     */
    std::vector<u64> solve_nb_layer(const Graph &original_g,
                                    const AlgorithmConfiguration &config,
                                    StatCollector &stat_collector);
}

#endif //SHAREDMAP_NB_LAYER_H
