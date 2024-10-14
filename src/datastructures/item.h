#ifndef SHAREDMAP_ITEM_H
#define SHAREDMAP_ITEM_H

#include "graph.h"
#include "translation_table.h"

namespace SharedMap {
    class Item {
    public:
        std::vector<u64> *identifier = nullptr; // used to identify the current graph in the recursive partitioning
        Graph *g = nullptr; // graph to partition
        TranslationTable *tt = nullptr; // the tt from original graph to this graph
        bool to_delete; // whether to free the memory

        Item() {
            identifier = nullptr;
            g = nullptr;
            tt = nullptr;
            to_delete = false;
        }

        /**
         * Default constructor.
         *
         * @param identifier The Identifier of the subgraph in the current hierarchy.
         * @param g The graph to process.
         * @param tt The Translation Table to the original graph.
         * @param to_delete Whether to delete the memory of the item after processing.
         */
        Item(std::vector<u64> *identifier, Graph *g, TranslationTable *tt, bool to_delete) {
            this->identifier = identifier;
            this->g = g;
            this->tt = tt;
            this->to_delete = to_delete;
        }

        /**
         * Frees all memory associate with the item, if the flag 'to_delete' is
         * set to true.
         */
        void free() {
            delete identifier;
            identifier = nullptr;
            if (to_delete) {
                delete g;
                delete tt;
                g = nullptr;
                tt = nullptr;
            }
        }

        bool operator<(const Item &item) const {
            return g->get_n() < item.g->get_n();
        }
    };
}

#endif //SHAREDMAP_ITEM_H
