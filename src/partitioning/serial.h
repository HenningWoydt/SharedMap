#ifndef SHAREDMAP_SERIAL_H
#define SHAREDMAP_SERIAL_H

#include <vector>

#include "src/datastructures/graph.h"
#include "src/utility/algorithm_configuration.h"
#include "src/utility/definitions.h"
#include "src/datastructures/item.h"
#include "src/datastructures/translation_table.h"
#include "src/profiling/stat_collector.h"
#include "src/utility/context.h"

namespace SharedMap {
    std::vector<u64> solve_serial(const Graph &g, const AlgorithmConfiguration &config, StatCollector &stat_collector);
}

#endif //SHAREDMAP_SERIAL_H
