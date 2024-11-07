#ifndef SHAREDMAP_ITEM_H
#define SHAREDMAP_ITEM_H

#include "src/datastructures/graph.h"
#include "src/datastructures/translation_table.h"

namespace SharedMap {
    /**
     * Item holding one partitioning task.
     */
    class Item {
    public:
        std::vector<u64>* identifier = nullptr; // used to identify the current graph in the recursive partitioning
        Graph* g                     = nullptr; // graph to partition
        TranslationTable* tt         = nullptr; // the tt from original graph to this graph
        bool to_delete               = false; // whether to free the memory

        /**
         * Default constructor.
         */
        Item() = default;

        /**
         * Default constructor.
         *
         * @param identifier The Identifier of the subgraph in the current hierarchy.
         * @param g The graph to process.
         * @param tt The Translation Table to the original graph.
         * @param to_delete Whether to delete the memory of the item after processing.
         */
        Item(std::vector<u64>* identifier, Graph* g, TranslationTable* tt, bool to_delete) {
            this->identifier = identifier;
            this->g          = g;
            this->tt         = tt;
            this->to_delete  = to_delete;
        }

        /**
         * Frees all memory associate with the item, if the flag 'to_delete' is
         * set to true.
         */
        void free() {
            // definitely delete the identifier
            delete identifier;
            identifier = nullptr;

            // release memory of the graph and translation table
            if (to_delete) {
                delete g;
                delete tt;
                g  = nullptr;
                tt = nullptr;
            }
        }

        /**
         * Compares the number of vertices of this graph and the other graph.
         *
         * @param item The other item.
         * @return True if there are fewer vertices than in the other graph.
         */
        bool operator<(const Item& item) const {
            return g->get_n() < item.g->get_n();
        }
    };
}

#endif //SHAREDMAP_ITEM_H
