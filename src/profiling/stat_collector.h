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

#ifndef SHAREDMAP_STAT_COLLECTOR_H
#define SHAREDMAP_STAT_COLLECTOR_H

#include <mutex>

#include "src/utility/definitions.h"
#include "src/utility/JSON_utils.h"
#include "src/utility/utils.h"

namespace SharedMap {
    /**
     * Class for collecting interesting statistics.
     */
    class StatCollector {
    private:
        std::mutex lock; // for organized access

        std::chrono::high_resolution_clock::time_point log_sp = std::chrono::high_resolution_clock::now(); // start time for logging

        u32 n_layer = 0; // number of layers

        // statistics for partitioning
        std::vector<f64> partition_time_per_layer; // log time on each depth
        f64              partition_time = 0.0;

        std::vector<u64> size_per_graph;
        std::vector<f64> start_time_per_graph;
        std::vector<f64> time_per_graph;
        std::vector<u64> alg_per_graph;
        std::vector<u64> n_threads_per_graph;
        std::vector<f64> imbalance_per_graph;
        std::vector<u64> k_per_graph;
        std::vector<u64> depth_per_graph;

        // statistics for subgraph creation
        std::vector<f64> subgraph_creation_time_per_layer;
        f64              subgraph_creation_time = 0.0;

        std::vector<u64> subgraph_size_per_graph;
        std::vector<f64> subgraph_start_time_per_graph;
        std::vector<f64> subgraph_time_per_graph;
        std::vector<u64> subgraph_k_per_graph;
        std::vector<u64> subgraph_n_threads_per_graph;
        std::vector<u64> subgraph_depth_per_graph;

    public:
        /**
         * Default constructor.
         */
        StatCollector() = default;

        /**
         * Initializes the Statistic Collector.
         *
         * @param t_n_layer Number of layers in the hierarchy
         */
        void initialize(const u32 t_n_layer) {
            log_sp  = std::chrono::high_resolution_clock::now();
            n_layer = t_n_layer;
            partition_time_per_layer.resize(n_layer, 0.0);
            subgraph_creation_time_per_layer.resize(n_layer, 0.0);
        }

        /**
         * Logs statistics of a partitioning task.
         *
         * @param depth Depth of the partitioning.
         * @param graph_size Size of the partitioned graph
         * @param alg Algorithm used.
         * @param n_threads Number of threads used.
         * @param imbalance Imbalance used.
         * @param k Number of partitions created.
         * @param sp Start time point.
         * @param ep End time point.
         */
        void log_partition(u64 depth,
                           u64 graph_size,
                           u64 alg,
                           u64 n_threads,
                           f64 imbalance,
                           u64 k,
                           const std::chrono::high_resolution_clock::time_point &sp,
                           const std::chrono::high_resolution_clock::time_point &ep) {
            lock.lock();

            f64 time       = (f64) std::chrono::duration_cast<std::chrono::nanoseconds>(ep - sp).count() / 1e9;
            f64 start_time = (f64) std::chrono::duration_cast<std::chrono::nanoseconds>(sp - log_sp).count() / 1e9;

            partition_time_per_layer[depth] += time;
            partition_time += time;

            start_time_per_graph.emplace_back(start_time);
            size_per_graph.emplace_back(graph_size);
            time_per_graph.emplace_back(time);
            alg_per_graph.emplace_back(alg);
            n_threads_per_graph.emplace_back(n_threads);
            imbalance_per_graph.emplace_back(imbalance);
            k_per_graph.emplace_back(k);
            depth_per_graph.emplace_back(depth);

            lock.unlock();
        }

        /**
         * Logs statistics of subgraph creation.
         *
         * @param depth Depth of the creation.
         * @param graph_size Size of the graph.
         * @param n_threads Number of threads used.
         * @param k Number of subgraph created.
         * @param sp Start time point.
         * @param ep End time point.
         */
        void log_subgraph_creation(u64 depth,
                                   u64 graph_size,
                                   u64 n_threads,
                                   u64 k,
                                   const std::chrono::high_resolution_clock::time_point &sp,
                                   const std::chrono::high_resolution_clock::time_point &ep) {
            lock.lock();

            f64 time       = (f64) std::chrono::duration_cast<std::chrono::nanoseconds>(ep - sp).count() / 1e9;
            f64 start_time = (f64) std::chrono::duration_cast<std::chrono::nanoseconds>(sp - log_sp).count() / 1e9;

            subgraph_creation_time_per_layer[depth] += time;
            subgraph_creation_time += time;

            subgraph_start_time_per_graph.emplace_back(start_time);
            subgraph_size_per_graph.emplace_back(graph_size);
            subgraph_time_per_graph.emplace_back(time);
            subgraph_n_threads_per_graph.emplace_back(n_threads);
            subgraph_k_per_graph.emplace_back(k);
            subgraph_depth_per_graph.emplace_back(depth);

            lock.unlock();
        }

        /**
         * Packs all statistics into a string in JSON format.
         *
         * @param n_tabs Number of tabs appended in front of each line (for visual purposes).
         * @return String in JSON format.
         */
        std::string to_JSON(const u64 n_tabs = 0) const {
            std::string tabs;
            for (size_t i = 0; i < n_tabs; ++i) { tabs.push_back('\t'); }

            std::string s = "{\n";

            s += tabs + to_JSON_MACRO(n_layer);
            s += tabs + to_JSON_MACRO(partition_time_per_layer);
            s += tabs + to_JSON_MACRO(partition_time);
            s += tabs + to_JSON_MACRO(size_per_graph);
            s += tabs + to_JSON_MACRO(start_time_per_graph);
            s += tabs + to_JSON_MACRO(time_per_graph);
            s += tabs + to_JSON_MACRO(alg_per_graph);
            s += tabs + to_JSON_MACRO(n_threads_per_graph);
            s += tabs + to_JSON_MACRO(imbalance_per_graph);
            s += tabs + to_JSON_MACRO(k_per_graph);
            s += tabs + to_JSON_MACRO(depth_per_graph);
            s += tabs + to_JSON_MACRO(subgraph_creation_time_per_layer);
            s += tabs + to_JSON_MACRO(subgraph_creation_time);
            s += tabs + to_JSON_MACRO(subgraph_size_per_graph);
            s += tabs + to_JSON_MACRO(subgraph_start_time_per_graph);
            s += tabs + to_JSON_MACRO(subgraph_time_per_graph);
            s += tabs + to_JSON_MACRO(subgraph_n_threads_per_graph);
            s += tabs + to_JSON_MACRO(subgraph_k_per_graph);
            s += tabs + to_JSON_MACRO(subgraph_depth_per_graph);

            s.pop_back();
            s.pop_back();
            s += "\n" + tabs + "}";
            return s;
        }
    };
}

#endif //SHAREDMAP_STAT_COLLECTOR_H
