#ifndef SHAREDMAP_NAIVE_H
#define SHAREDMAP_NAIVE_H

#include <vector>

#include "src/datastructures/graph.h"
#include "src/profiling/stat_collector.h"
#include "src/utility/algorithm_configuration.h"
#include "src/utility/definitions.h"

namespace SharedMap {
    std::vector<u64> solve_naive(const Graph &original_g, const AlgorithmConfiguration &config, StatCollector &stat_collector);
}

#endif //SHAREDMAP_NAIVE_H
