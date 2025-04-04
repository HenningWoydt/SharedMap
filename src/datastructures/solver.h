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

#ifndef SHAREDMAP_SOLVER_H
#define SHAREDMAP_SOLVER_H

#include "src/datastructures/graph.h"
#include "src/datastructures/translation_table.h"
#include "src/partitioning/layer.h"
#include "src/partitioning/naive.h"
#include "src/partitioning/nb_layer.h"
#include "src/partitioning/queue.h"
#include "src/partitioning/serial.h"
#include "src/profiling/stat_collector.h"
#include "src/utility/algorithm_configuration.h"
#include "src/utility/definitions.h"
#include "src/utility/JSON_utils.h"
#include "src/utility/utils.h"
#include "src/utility/qap.h"

namespace SharedMap {
    /**
     * Solver
     */
    class Solver {
    private:
        const AlgorithmConfiguration &m_ac;
        StatCollector                stat_collector;

        f64 io_time    = 0.0;
        f64 solve_time = 0.0;

    public:
        /**
         * Default constructor.
         *
         * @param ac Configuration to use.
         */
        explicit Solver(AlgorithmConfiguration &ac) : m_ac(ac) {
            stat_collector.initialize(m_ac.hierarchy.size());
        }

        /**
         * Solves the problem.
         */
        void solve() {
            // read graph
            auto  sp = std::chrono::steady_clock::now();
            Graph g(m_ac.graph_in);
            auto  ep = std::chrono::steady_clock::now();
            io_time += (f64) std::chrono::duration_cast<std::chrono::nanoseconds>(ep - sp).count() / 1e9;

            // solve problem
            sp = std::chrono::steady_clock::now();
            std::vector<u64> partition = internal_solve(g);
            ep = std::chrono::steady_clock::now();
            solve_time += (f64) std::chrono::duration_cast<std::chrono::nanoseconds>(ep - sp).count() / 1e9;

            // write output
            sp = std::chrono::steady_clock::now();
            write_solution(partition);
            ep = std::chrono::steady_clock::now();
            io_time += (f64) std::chrono::duration_cast<std::chrono::nanoseconds>(ep - sp).count() / 1e9;

            print_statistics();
        }

        /**
         * Solves the problem.
         */
        void solve(Graph &g, int *partition, int &comm_cost, bool verbose = false) {
            io_time = 0;

            // solve problem
            auto sp = std::chrono::steady_clock::now();

            std::vector<u64> internal_partition = internal_solve(g);

            for (size_t i = 0; i < g.get_n(); ++i) {
                partition[i] = (int) internal_partition[i];
            }

            comm_cost = (int) determine_qap(g, m_ac.hierarchy, m_ac.distance, internal_partition);

            auto ep = std::chrono::steady_clock::now();
            solve_time += (f64) std::chrono::duration_cast<std::chrono::nanoseconds>(ep - sp).count() / 1e9;

            if (verbose) {
                print_statistics();
            }
        }

    private:
        /**
         * Internal solve function.
         *
         * @param g The graph.
         * @return The partition.
         */
        std::vector<u64> internal_solve(const Graph &g) {
            // if only one thread, then just solve serial
            if (m_ac.n_threads == 1) {
                return solve_serial(g, m_ac, stat_collector);
            }

            switch (m_ac.parallel_strategy_id) {
                case NAIVE:
                    return solve_naive(g, m_ac, stat_collector);
                case LAYER:
                    return solve_layer(g, m_ac, stat_collector);
                case QUEUE:
                    return solve_queue(g, m_ac, stat_collector);
                case NB_LAYER:
                    return solve_nb_layer(g, m_ac, stat_collector);
                default:
                    std::cerr << "Strategy ID " << m_ac.parallel_strategy_id << " not recognized!" << std::endl;
                    abort();
            }
        }

        /**
         * Writes the partition to a file.
         *
         * @param partition The partition.
         */
        void write_solution(const std::vector<u64> &partition) const {
            std::stringstream ss;

            for (u64      i: partition) {
                ss << i << "\n";
            }
            std::ofstream out(m_ac.mapping_out);
            out << ss.rdbuf();
            out.close();
        }

        /**
         * Prints interesting statistics to std::cout.
         */
        void print_statistics() const {
            std::string s = "{\n";

            s += to_JSON_MACRO(io_time);
            s += to_JSON_MACRO(solve_time);
            s += "\"algorithm-configuration\" : " + m_ac.to_JSON(2) + ",\n";
            s += "\"statistics\" : " + stat_collector.to_JSON(2) + ",\n";

            s.pop_back();
            s.pop_back();
            s += "\n}";
            std::cout << s << std::endl;
        }
    };
}

#endif //SHAREDMAP_SOLVER_H
