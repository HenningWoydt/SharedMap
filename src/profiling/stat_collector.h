#ifndef SHAREDMAP_STAT_COLLECTOR_H
#define SHAREDMAP_STAT_COLLECTOR_H

#include "src/utility/definitions.h"
#include "src/utility/JSON_utils.h"
#include "src/utility/utils.h"

namespace SharedMap {
    /**
     * Class for collecting interesting statistics.
     */
    class StatCollector {
    private:
        std::mutex lock;
        std::chrono::high_resolution_clock::time_point log_sp;

        u32 n_layer;

        // statistics for partitioning
        std::vector<f64> partition_time_per_layer; // log time on each depth
        f64 partition_time = 0.0;

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
        f64 subgraph_creation_time = 0.0;

        std::vector<u64> subgraph_size_per_graph;
        std::vector<f64> subgraph_start_time_per_graph;
        std::vector<f64> subgraph_time_per_graph;
        std::vector<u64> subgraph_k_per_graph;
        std::vector<u64> subgraph_n_threads_per_graph;
        std::vector<u64> subgraph_depth_per_graph;

    public:
        explicit StatCollector() {
            log_sp = std::chrono::high_resolution_clock::now();
            n_layer = 0;
        }

        void initialize(u32 t_n_layer) {
            log_sp = std::chrono::high_resolution_clock::now();
            n_layer = t_n_layer;
            partition_time_per_layer.resize(n_layer, 0.0);
            subgraph_creation_time_per_layer.resize(n_layer, 0.0);
        }

        void log_partition(u64 depth,
                           u64 graph_size,
                           u64 alg,
                           u64 n_threads,
                           f64 imbalance,
                           u64 k,
                           const std::chrono::high_resolution_clock::time_point &sp,
                           const std::chrono::high_resolution_clock::time_point &ep) {
            lock.lock();

            f64 time = (f64) std::chrono::duration_cast<std::chrono::nanoseconds>(ep - sp).count() / 1e9;
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

        void log_subgraph_creation(u64 depth,
                                   u64 graph_size,
                                   u64 n_threads,
                                   u64 k,
                                   const std::chrono::high_resolution_clock::time_point &sp,
                                   const std::chrono::high_resolution_clock::time_point &ep) {
            lock.lock();

            f64 time = (f64) std::chrono::duration_cast<std::chrono::nanoseconds>(ep - sp).count() / 1e9;
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

        std::string to_JSON(u64 n_tabs = 0) const {
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
