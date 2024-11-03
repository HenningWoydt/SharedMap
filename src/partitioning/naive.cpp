#include "naive.h"

#include "src/datastructures/translation_table.h"

namespace SharedMap {
    std::vector<u64> solve_naive(const Graph& original_g, const AlgorithmConfiguration& config, StatCollector& stat_collector) {
        std::vector<u64> partition(original_g.get_n()); // end partition
        TranslationTable original_tt(original_g.get_n()); // default translation table

        return partition;
    }
}
