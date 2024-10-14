#ifndef SHAREDMAP_CONTEXT_H
#define SHAREDMAP_CONTEXT_H

#include <string>

#include "src/utility/definitions.h"
#include "src/utility/macros.h"
#include "src/utility/utils.h"
#include "src/utility/JSON_utils.h"
#include "src/profiling/stat_collector.h"
#include "algorithm_configuration.h"

namespace SharedMap {

    class Context {
    public:
        const AlgorithmConfiguration &ac;
        const u64 global_g_weight;
        const std::vector<u64> &index_vec;
        const std::vector<u64> &k_rem_vec;
        StatCollector &stat_collector;

        std::mutex global_mutex;

        Context(const AlgorithmConfiguration &t_ac,
                u64 t_global_g_weight,
                const std::vector<u64> &t_index_vec,
                const std::vector<u64> &t_k_rem_vec,
                StatCollector &t_stat_collector) :
                ac(t_ac),
                global_g_weight(t_global_g_weight),
                index_vec(t_index_vec),
                k_rem_vec(t_k_rem_vec),
                stat_collector(t_stat_collector) {}
    };

}

#endif //SHAREDMAP_CONTEXT_H
