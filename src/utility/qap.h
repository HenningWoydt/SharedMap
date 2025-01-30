#ifndef SHAREDMAP_QAP_H
#define SHAREDMAP_QAP_H

#include <numeric>

#include "src/utility/definitions.h"

#include "src/datastructures/graph.h"

namespace SharedMap {
    u64 determine_qap(const Graph &g,
                      const std::vector<u64> &hierarchy,
                      const std::vector<u64> &distance,
                      const std::vector<u64> &partition);
}

#endif //SHAREDMAP_QAP_H
