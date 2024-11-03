#ifndef SHAREDMAP_LAYER_H
#define SHAREDMAP_LAYER_H

#include <vector>

#include "src/datastructures/graph.h"
#include "src/profiling/stat_collector.h"
#include "src/utility/algorithm_configuration.h"
#include "src/utility/definitions.h"

namespace SharedMap {
    std::vector<u64> solve_layer(const Graph& original_g, const AlgorithmConfiguration& config, StatCollector& stat_collector);
}

#endif //SHAREDMAP_LAYER_H
