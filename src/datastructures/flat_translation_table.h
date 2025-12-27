#ifndef SHAREDMAP_FLAT_TRANSLATION_TABLE_H
#define SHAREDMAP_FLAT_TRANSLATION_TABLE_H

#include <vector>
#include <limits>

#include "src/utility/definitions.h"
#include "src/utility/macros.h"

namespace SharedMap {

    class FlatTranslationTable {
    private:
        static constexpr u64 INVALID = std::numeric_limits<u64>::max();

        // old -> new (size = old_universe)
        std::vector<u64> m_o2n;

        // new -> old (size = new_size)
        std::vector<u64> m_n2o;

    public:
        FlatTranslationTable() = default;

        FlatTranslationTable(const u64 n_old, const u64 n_new) {
            m_o2n.resize(n_old + 1);
            m_n2o.resize(n_new + 1);
        }

        explicit FlatTranslationTable(const u64 n) {
            m_o2n.resize(n);
            m_n2o.resize(n);
            std::iota(m_o2n.begin(), m_o2n.end(), 0);
            std::iota(m_n2o.begin(), m_n2o.end(), 0);
        }

        /**
         * Add mapping old -> new.
         * Assumes each old ID is added at most once.
         */
        inline void add(u64 old_id, u64 new_id) {
            if (old_id >= m_o2n.size()) {
                m_o2n.resize(old_id + 1);
            }
            m_o2n[old_id] = new_id;

            if (new_id >= m_n2o.size()) {
                m_n2o.resize(new_id + 1);
            }
            m_n2o[new_id] = old_id;
        }

        /**
         * O(1) old -> new
         */
        inline u64 get_n(u64 old_id) const {
            ASSERT(old_id < m_o2n.size());
            ASSERT(m_o2n[old_id] != INVALID);
            return m_o2n[old_id];
        }

        /**
         * O(1) new -> old
         */
        inline u64 get_o(u64 new_id) const {
            ASSERT(new_id < m_n2o.size());
            return m_n2o[new_id];
        }

        inline void finalize() { }
    };

} // namespace SharedMap

#endif //SHAREDMAP_FLAT_TRANSLATION_TABLE_H
