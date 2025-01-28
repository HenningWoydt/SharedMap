#ifndef SHAREDMAP_QUEUE_H
#define SHAREDMAP_QUEUE_H

#include <atomic>
#include <queue>
#include <thread>
#include <vector>

#include "src/datastructures/graph.h"
#include "src/datastructures/item.h"
#include "src/datastructures/translation_table.h"
#include "src/partitioning/partition_util.h"
#include "src/profiling/stat_collector.h"
#include "src/utility/algorithm_configuration.h"
#include "src/utility/definitions.h"

namespace SharedMap {
    /**
     * Use the queue approach.
     *
     * @param original_g The original graph.
     * @param config The algorithm configuration.
     * @param stat_collector The statistic collector.
     * @return The partition.
     */
    std::vector<u64> solve_queue(const Graph &original_g,
                                 const AlgorithmConfiguration &config,
                                 StatCollector &stat_collector);
}

#endif //SHAREDMAP_QUEUE_H
