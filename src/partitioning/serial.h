#ifndef SHAREDMAP_SERIAL_H
#define SHAREDMAP_SERIAL_H

#include <vector>

#include "src/datastructures/graph.h"
#include "src/datastructures/item.h"
#include "src/partitioning/partition_util.h"
#include "src/profiling/stat_collector.h"
#include "src/utility/algorithm_configuration.h"
#include "src/utility/definitions.h"

namespace SharedMap {
    /**
     * Uses only one thread.
     *
     * @param original_g The original graph.
     * @param config The algorithm configuration.
     * @param stat_collector The statistic collector.
     * @return The partition.
     */
    std::vector<u64> solve_serial(const Graph &original_g,
                                  const AlgorithmConfiguration &config,
                                  StatCollector &stat_collector);
}

#endif //SHAREDMAP_SERIAL_H
