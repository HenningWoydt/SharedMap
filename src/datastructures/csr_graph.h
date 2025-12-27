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

#ifndef SHAREDMAP_CSR_GRAPH_H
#define SHAREDMAP_CSR_GRAPH_H

#include <iostream>
#include <vector>

#include "../utility/definitions.h"
#include "src/utility/utils.h"

namespace SharedMap {
    /**
    * Standard undirected Graph that can hold vertex and edge weights.
    */
    class CSRGraph {
    public:
        u64 n = 0; // number of vertices
        u64 m = 0; // number of edges

        std::vector<u64> neighborhoods; // edges per vertex

        std::vector<u64> weights; // vertex weight
        u64 g_weight = 0; // graph weight

        std::vector<u64> edges_v; // edges vertex
        std::vector<u64> edges_w; // edges weight

        CSRGraph() = default;

        explicit CSRGraph(const std::string &file_path) {
            if (!file_exists(file_path)) {
                std::cerr << "File " << file_path << " does not exist!" << std::endl;
                exit(EXIT_FAILURE);
            }

            // mmap the whole file
            MMap mm = mmap_file_ro(file_path);
            char *p = mm.data;
            const char *end = mm.data + mm.size; // (tiny fix: don't do -1)

            // skip comment lines
            while (*p == '%') {
                while (*p != '\n') { ++p; }
                ++p;
            }

            // skip whitespace
            while (*p == ' ') { ++p; }

            // read number of vertices
            n = 0;
            while (*p != ' ' && *p != '\n') {
                n = n * 10 + (u64) (*p - '0');
                ++p;
            }

            // skip whitespace
            while (*p == ' ') { ++p; }

            // read number of edges
            m = 0;
            while (*p != ' ' && *p != '\n') {
                m = m * 10 + (u64) (*p - '0');
                ++p;
            }
            m *= 2;

            // search end of line or fmt
            std::string fmt = "000";
            bool has_v_weights = false;
            bool has_e_weights = false;
            while (*p == ' ') { ++p; }
            if (*p != '\n') {
                // found fmt
                fmt[0] = *p;
                ++p;
                if (*p != '\n') {
                    // found fmt
                    fmt[1] = *p;
                    ++p;
                    if (*p != '\n') {
                        // found fmt
                        fmt[2] = *p;
                        ++p;
                    }
                }
                // skip whitespaces
                while (*p == ' ') { ++p; }
            }

            g_weight = 0;
            weights.resize(n);
            neighborhoods.resize(n + 1);
            neighborhoods[0] = 0;
            edges_v.resize(m);
            edges_w.resize(m);
            has_v_weights = fmt[1] == '1';
            has_e_weights = fmt[2] == '1';

            ++p;
            u64 u = 0;
            size_t curr_m = 0;
            while (p < end) {
                // skip comment lines
                while (*p == '%') {
                    while (*p != '\n') { ++p; }
                    ++p;
                }

                // skip whitespaces
                while (*p == ' ') { ++p; }

                // read in vertex weight
                u64 vw = 1;
                if (has_v_weights) {
                    vw = 0;
                    while (*p != ' ' && *p != '\n') {
                        vw = vw * 10 + (u64) (*p - '0');
                        ++p;
                    }

                    // skip whitespaces
                    while (*p == ' ') { ++p; }
                }
                weights[u] = vw;
                g_weight += vw;

                // read in edges
                while (*p != '\n' && p < end) {
                    u64 v = 0;
                    u64 w = 1;

                    while (*p != ' ' && *p != '\n') {
                        v = v * 10 + (u64) (*p - '0');
                        ++p;
                    }

                    // skip whitespaces
                    while (*p == ' ') { ++p; }

                    if (has_e_weights) {
                        w = 0;
                        while (*p != ' ' && *p != '\n') {
                            w = w * 10 + (u64) (*p - '0');
                            ++p;
                        }

                        // skip whitespaces
                        while (*p == ' ') { ++p; }
                    }

                    edges_v[curr_m] = v - 1;
                    edges_w[curr_m] = w;
                    ++curr_m;
                }
                neighborhoods[u + 1] = curr_m;
                ++u;
                ++p;
            }

            if (curr_m != m) {
                std::cerr << "Number of expected edges " << m << " not equal to number edges " << curr_m << " found!\n";
                munmap_file(mm);
                exit(EXIT_FAILURE);
            }

            // done with the file
            munmap_file(mm);
        }

        explicit CSRGraph(const u64 t_n, const u64 t_m, const u64 t_w) {
            n = t_n;
            m = t_m;
            g_weight = t_w;

            neighborhoods.resize(n + 1);
            neighborhoods[0] = 0;
            weights.resize(n);
            edges_v.resize(m);
            edges_w.resize(m);
        }
    };
}

#endif //SHAREDMAP_CSR_GRAPH_H
