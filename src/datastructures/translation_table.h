#ifndef SHAREDMAP_TRANSLATION_TABLE_H
#define SHAREDMAP_TRANSLATION_TABLE_H

#include <unordered_map>
#include <numeric>

#include "src/utility/definitions.h"
#include "src/utility/macros.h"

namespace SharedMap {
    class TranslationTable {
    private:
        std::vector<std::pair<u64, u64>> m_translation_table_o_to_n;
        std::vector<u64> m_translation_n_to_o;

    public:
        /**
         * Default constructor.
         */
        TranslationTable() = default;

        /**
         * Initializes the translation table with the Identity mapping.
         */
        explicit TranslationTable(u64 n) {
            m_translation_table_o_to_n.resize(n);
            for(size_t u = 0; u < n; ++u){
                m_translation_table_o_to_n[u] = {u, u};
            }

            m_translation_n_to_o.resize(n);
            std::iota(m_translation_n_to_o.begin(), m_translation_n_to_o.end(), 0);
        }

        /**
         * Adds a translation from o to n and from n to o.
         *
         * @param o Old value.
         * @param n New value.
         */
        void add(u64 o, u64 n) {
            m_translation_table_o_to_n.emplace_back(o, n);

            m_translation_n_to_o.resize(n + 1);
            m_translation_n_to_o[n] = o;
        }

        void finalize(){
            std::sort(m_translation_table_o_to_n.begin(), m_translation_table_o_to_n.end(), [](auto &left, auto &right) { return left.first < right.first; });
        }

        /**
         * Get the new value for o.
         *
         * @param o Old value
         */
        u64 get_n(u64 o) const {
            auto it = std::lower_bound(m_translation_table_o_to_n.begin(), m_translation_table_o_to_n.end(), std::make_pair(o, (u64) 0), [](auto &left, auto &right) { return left.first < right.first; });
            u64 n = it->second;
            return n;
        }

        /**
         * Get the old value for n.
         *
         * @param n New value.
         */
        u64 get_o(u64 n) const {
            return m_translation_n_to_o[n];
        }
    };
}

#endif //SHAREDMAP_TRANSLATION_TABLE_H
