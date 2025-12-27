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

#ifndef SHAREDMAP_ITEM_H
#define SHAREDMAP_ITEM_H

#include "../utility/types.h"

namespace SharedMap {
    /**
     * Item holding one partitioning task.
     */
    class Item {
    public:
        std::vector<u64> *identifier = nullptr; // used to identify the current graph in the recursive partitioning
        CSRGraph *g = nullptr; // graph to partition
        TransTable *tt = nullptr; // the tt from original graph to this graph
        bool to_delete = false; // whether to free the memory

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
        Item(std::vector<u64> *identifier, CSRGraph *g, TransTable *tt, bool to_delete) {
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
            // definitely delete the identifier
            delete identifier;
            identifier = nullptr;

            // release memory of the graph and translation table
            if (to_delete) {
                delete g;
                delete tt;
                g = nullptr;
                tt = nullptr;
            }
        }

        /**
         * Compares the number of vertices of this graph and the other graph.
         *
         * @param item The other item.
         * @return True if there are fewer vertices than in the other graph.
         */
        bool operator<(const Item &item) const {
            return g->n < item.g->n;
        }
    };
}

#endif //SHAREDMAP_ITEM_H
