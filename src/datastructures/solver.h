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
#include "src/partitioning/layer.h"
#include "src/partitioning/naive.h"
#include "src/partitioning/nb_layer.h"
#include "src/partitioning/queue.h"
#include "src/partitioning/serial.h"
#include "src/profiling/stat_collector.h"
#include "src/utility/algorithm_configuration.h"
#include "src/utility/definitions.h"
#include "src/utility/JSON_utils.h"
#include "src/utility/qap.h"

namespace SharedMap {
    /**
     * Solver
     */
    class Solver {
    private:
        const AlgorithmConfiguration &m_ac;
        StatCollector stat_collector;

        f64 io_time = 0.0;
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
        void solve(bool verbose = false) {
            // read graph
            auto sp_total = std::chrono::steady_clock::now();
            auto sp = std::chrono::steady_clock::now();
            Graph g(m_ac.graph_in);
            auto ep = std::chrono::steady_clock::now();
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

            auto ep_total = std::chrono::steady_clock::now();;
            print_statistics();

            if (verbose) {
                u64 comm_cost = determine_qap(g, m_ac.hierarchy, m_ac.distance, partition);
                u64 lmax = std::ceil((1.0 + m_ac.imbalance) * ((f64) g.get_weight() / (f64) m_ac.k));

                std::cout << "Total time        : " << (f64) std::chrono::duration_cast<std::chrono::nanoseconds>(ep_total - sp_total).count() / 1e9 << std::endl;
                std::cout << "#Nodes            : " << g.get_n() << std::endl;
                std::cout << "#Edges            : " << g.get_m() << std::endl;
                std::cout << "k                 : " << m_ac.k << std::endl;
                std::cout << "Lmax              : " << lmax << std::endl;
                std::cout << "Final QAP         : " << comm_cost << std::endl;

                std::vector<u64> weights(m_ac.k, 0);
                for (u64 u = 0; u < g.get_n(); ++u) { weights[partition[u]] += g.get_vertex_weight(u); }

                size_t n_empty_partitions = 0;
                size_t n_overloaded_partitions = 0;
                u64 sum_too_much = 0;
                u64 max_w = 0;
                for (u64 id = 0; id < m_ac.k; ++id) {
                    n_empty_partitions += weights[id] == 0;
                    n_overloaded_partitions += weights[id] > lmax;
                    if (weights[id] > lmax) { sum_too_much += std::max((u64) 0, weights[id] - lmax); }
                    max_w = std::max(max_w, weights[id]);
                }
                std::cout << "max block w       : " << max_w << std::endl;
                std::cout << "#empty partitions : " << n_empty_partitions << std::endl;
                std::cout << "#oload partitions : " << n_overloaded_partitions << std::endl;
                std::cout << "Sum oload weights : " << sum_too_much << std::endl;
            }
        }

        /**
         * Solves the problem.
         */
        void solve(const Graph &g, int *partition, int &comm_cost, bool verbose = false) {
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

                u64 lmax = std::ceil((1.0 + m_ac.imbalance) * ((f64) g.get_weight() / (f64) m_ac.k));

                std::cout << "Total time        : " << (f64) std::chrono::duration_cast<std::chrono::nanoseconds>(ep - sp).count() / 1e9 << std::endl;
                std::cout << "#Nodes            : " << g.get_n() << std::endl;
                std::cout << "#Edges            : " << g.get_m() << std::endl;
                std::cout << "k                 : " << m_ac.k << std::endl;
                std::cout << "Lmax              : " << lmax << std::endl;
                std::cout << "Final QAP         : " << comm_cost << std::endl;

                std::vector<u64> weights(m_ac.k, 0);
                for (u64 u = 0; u < g.get_n(); ++u) { weights[internal_partition[u]] += g.get_vertex_weight(u); }

                size_t n_empty_partitions = 0;
                size_t n_overloaded_partitions = 0;
                u64 sum_too_much = 0;
                u64 max_w = 0;
                for (u64 id = 0; id < m_ac.k; ++id) {
                    n_empty_partitions += weights[id] == 0;
                    n_overloaded_partitions += weights[id] > lmax;
                    sum_too_much += std::max((u64) 0, weights[id] - lmax);
                    max_w = std::max(max_w, weights[id]);
                }
                std::cout << "max block w       : " << max_w << std::endl;
                std::cout << "#empty partitions : " << n_empty_partitions << std::endl;
                std::cout << "#oload partitions : " << n_overloaded_partitions << std::endl;
                std::cout << "Sum oload weights : " << sum_too_much << std::endl;
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

            for (u64 i: partition) {
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
