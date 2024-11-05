#ifndef SHAREDMAP_TRANSLATION_TABLE_H
#define SHAREDMAP_TRANSLATION_TABLE_H

#include <unordered_map>

#include "src/utility/definitions.h"
#include "src/utility/macros.h"

namespace SharedMap {
    class TranslationTable {
    private:
        std::unordered_map<u64, u64> m_translation_o_to_n;
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
            for (u64 u = 0; u < n; ++u) {
                add(u, u);
            }
        }

        /**
         * Adds a translation from o to n and from n to o.
         *
         * @param o Old value.
         * @param n New value.
         */
        void add(u64 o, u64 n) {
            ASSERT(m_translation_o_to_n.find(o) == m_translation_o_to_n.end());
            ASSERT(m_translation_n_to_o.find(n) == m_translation_n_to_o.end());

            m_translation_o_to_n[o] = n;

            m_translation_n_to_o.resize(n+1);
            m_translation_n_to_o[n] = o;
        }

        /**
         * Get the new value for o.
         *
         * @param o Old value
         */
        u64 get_n(u64 o) const {
            ASSERT(m_translation_o_to_n.find(o) != m_translation_o_to_n.end());

            return m_translation_o_to_n.at(o);
        }

        /**
         * Get the old value for n.
         *
         * @param n New value.
         */
        u64 get_o(u64 n) const {
            ASSERT(m_translation_n_to_o.find(n) != m_translation_n_to_o.end());

            return m_translation_n_to_o[n];
        }
    };
}

#endif //SHAREDMAP_TRANSLATION_TABLE_H
