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
    void nb_layer_thread_work_oversubscribed(u64 local_idx,
                                             std::atomic<u64>& global_idx,
                                             const std::vector<Item>& layer,
                                             u64 layer_size,
                                             u64 n_assigned_threads,
                                             std::atomic<u64>& n_inactive_threads,
                                             std::vector<u64>& solution,
                                             const Graph& original_g,
                                             const AlgorithmConfiguration& config,
                                             StatCollector& stat_collector);

    void nb_layer_thread_work(Item item,
                              u64 n_assigned_threads,
                              std::atomic<u64>& n_inactive_threads,
                              std::vector<u64>& solution,
                              const Graph& original_g,
                              const AlgorithmConfiguration& config,
                              StatCollector& stat_collector);

    /**
     * Uses the non-blocking layer approach.
     *
     * @param original_g The original graph.
     * @param config The algorithm configuration.
     * @param stat_collector The statistic collector.
     * @return The partition.
     */
    std::vector<u64> solve_nb_layer(const Graph& original_g,
                                    const AlgorithmConfiguration& config,
                                    StatCollector& stat_collector);
}

#endif //SHAREDMAP_NB_LAYER_H
