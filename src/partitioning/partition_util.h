#ifndef SHAREDMAP_PARTITION_UTIL_H
#define SHAREDMAP_PARTITION_UTIL_H

#include <cmath>

#include "src/datastructures/graph.h"
#include "src/datastructures/item.h"
#include "src/datastructures/translation_table.h"
#include "src/partitioning/partition.h"
#include "src/profiling/stat_collector.h"
#include "src/utility/definitions.h"

namespace SharedMap {
    f64 determine_adaptive_imbalance(f64 global_imbalance,
                                     u64 global_g_weight,
                                     u64 global_k,
                                     u64 local_g_weight,
                                     u64 local_k_rem,
                                     u64 depth);

    void partition_graph(const Graph& g,
                         const u64 k,
                         const f64 imbalance,
                         std::vector<u64>& partition,
                         const u64 n_threads,
                         const u64 depth,
                         const std::vector<u64>& serial_alg_id,
                         const std::vector<u64>& parallel_alg_id);

    void create_sub_graphs(const Graph& g,
                           const TranslationTable& g_tt,
                           const u64 k,
                           const std::vector<u64>& partition,
                           const std::vector<u64>& identifier,
                           std::vector<Item>& temp_stack,
                           const u64 depth,
                           const u64 n_threads,
                           StatCollector& stat_collector);
} // namespace SharedMap

#endif // SHAREDMAP_PARTITION_UTIL_H
