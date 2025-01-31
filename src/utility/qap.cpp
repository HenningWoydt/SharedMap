/*******************************************************************************
 * MIT License
 *
 * This file is part of SharedMap.
 *
 * Copyright (C) 2025 Henning Woydt <henning.woydt@informatik.uni-heidelberg.de>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 ******************************************************************************/

#include "qap.h"

namespace SharedMap {
    void determine_location(const u64 p_id,
                            const std::vector<u64> &hierarchy,
                            const u64 k,
                            std::vector<u64> &loc) {
        u64 r_start = 0;
        u64 r_end   = k;

        const size_t s = hierarchy.size();
        for (size_t  i = 0; i < s; ++i) {
            const u64 n_parts = hierarchy[s - 1 - i];
            const u64 add     = (r_end - r_start) / n_parts;

            for (u64 j = 0; j < n_parts; ++j) {
                if (r_start <= p_id && p_id < r_start + add) {
                    loc[s - 1 - i] = j;
                    r_end = r_start + add;
                    break;
                }
                r_start += add;
            }
        }
    }

    u64 determine_distance(const u64 u_id,
                           const u64 v_id,
                           const u64 k,
                           const std::vector<u64> &hierarchy,
                           const std::vector<u64> &distance,
                           std::vector<u64> &u_loc,
                           std::vector<u64> &v_loc) {
        // special case
        if (u_id == v_id) {
            return 0;
        }

        determine_location(u_id, hierarchy, k, u_loc);
        determine_location(v_id, hierarchy, k, v_loc);

        // determine the distance
        const u64 s = hierarchy.size();
        for (u64  i = 0; i < s; ++i) {
            if (u_loc[s - 1 - i] != v_loc[s - 1 - i]) {
                return distance[s - 1 - i];
            }
        }
        // unreachable
        abort();
    }

    u64 determine_qap(const Graph &g,
                      const std::vector<u64> &hierarchy,
                      const std::vector<u64> &distance,
                      const std::vector<u64> &partition) {
        std::vector<u64> u_loc(hierarchy.size());
        std::vector<u64> v_loc(hierarchy.size());
        u64              k = product(hierarchy);

        u64 comm_cost = 0;

        for (u64 u = 0; u < g.get_n(); ++u) {
            for (auto &[v, w]: g[u]) {

                const u64 u_id = partition[u];
                const u64 v_id = partition[v];

                if (u_id != v_id) {
                    const u64 u_v_distance = determine_distance(u_id, v_id, k, hierarchy, distance, u_loc, v_loc);

                    // comm cost
                    comm_cost += w * u_v_distance;
                }
            }
        }

        return comm_cost;
    }
}