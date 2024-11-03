#ifndef SHAREDMAP_STAT_COLLECTOR_H
#define SHAREDMAP_STAT_COLLECTOR_H

#ifndef STAT_COLLECTION
#define STAT_COLLECTION true
#endif

#include "src/utility/definitions.h"
#include "src/utility/JSON_utils.h"
#include "src/utility/utils.h"

namespace SharedMap {
    /**
     * Class for collecting interesting statistics.
     */
    class StatCollector {
    private:
#if STAT_COLLECTION
        std::mutex lock;

        u32 n_layer;

        std::vector<f64> partition_time_per_layer; // log time on each depth
        f64 partition_time = 0.0;

        std::vector<f64> statistic_time_per_layer; // log time on each depth
        f64 statistic_time = 0.0;

        std::vector<f64> sub_graph_creation_time_per_layer; // log time on each depth
        f64 sub_graph_creation_time = 0.0;

        std::vector<u64> size_per_graph; // log size per graph
        std::vector<u64> edge_weights_per_graph; // log the weight of edges
        std::vector<f64> time_per_graph; // log time per graph
        std::vector<u64> alg_per_graph; // log alg per graph
        std::vector<u64> n_threads_per_graph; // log n_threads per graph
        std::vector<f64> imbalance_per_graph; // log imbalance per graph
        std::vector<u64> k_per_graph; // log imbalance per graph
        std::vector<u64> distance_per_graph; // log distance per graph
        std::vector<u64> edge_cut_per_graph; // log the edge cut of partitioned graph
        std::vector<u64> weighted_edge_cut_per_graph; // log the weighted-edge cut of partitioned graph
        std::vector<u64> comm_cost_per_graph; // log the communication cost of partitioned graph
#endif

    public:
        explicit StatCollector() {
#if STAT_COLLECTION
            n_layer = 0;
#endif
        }

        void initialize(u32 t_n_layer) {
#if STAT_COLLECTION
            n_layer = t_n_layer;
            partition_time_per_layer.resize(n_layer, 0.0);
            statistic_time_per_layer.resize(n_layer, 0.0);
            sub_graph_creation_time_per_layer.resize(n_layer, 0.0);
#endif
        }

        void log_partition(u64 depth, u64 graph_size, u64 edge_weights, f64 time, u64 alg, u64 n_threads, f64 imbalance, u64 k, u64 distance, u64 edge_cut, u64 weighted_edge_cut, u64 comm_cost, f64 local_statistic_time) {
#if STAT_COLLECTION
            lock.lock();

            partition_time_per_layer[depth] += time;
            partition_time += time;

            statistic_time_per_layer[depth] += local_statistic_time;
            statistic_time += local_statistic_time;

            size_per_graph.emplace_back(graph_size);
            edge_weights_per_graph.emplace_back(edge_weights);
            time_per_graph.emplace_back(time);
            alg_per_graph.emplace_back(alg);
            n_threads_per_graph.emplace_back(n_threads);
            imbalance_per_graph.emplace_back(imbalance);
            k_per_graph.emplace_back(k);
            distance_per_graph.emplace_back(distance);
            edge_cut_per_graph.emplace_back(edge_cut);
            weighted_edge_cut_per_graph.emplace_back(weighted_edge_cut);
            comm_cost_per_graph.emplace_back(comm_cost);

            lock.unlock();
#endif
        }

        void add_subgraph_creation_time(u64 depth, f64 time) {
#if STAT_COLLECTION
            lock.lock();

            sub_graph_creation_time_per_layer[depth] += time;
            sub_graph_creation_time += time;

            lock.unlock();
#endif
        }

        std::string to_JSON(u64 n_tabs = 0) const {
#if STAT_COLLECTION
            std::string tabs;
            for (size_t i = 0; i < n_tabs; ++i) { tabs.push_back('\t'); }

            std::string s = "{\n";

            s += tabs + to_JSON_MACRO(n_layer);
            s += tabs + to_JSON_MACRO(partition_time_per_layer);
            s += tabs + to_JSON_MACRO(partition_time);
            s += tabs + to_JSON_MACRO(statistic_time_per_layer);
            s += tabs + to_JSON_MACRO(statistic_time);
            s += tabs + to_JSON_MACRO(sub_graph_creation_time_per_layer);
            s += tabs + to_JSON_MACRO(sub_graph_creation_time);
            s += tabs + to_JSON_MACRO(size_per_graph);
            s += tabs + to_JSON_MACRO(edge_weights_per_graph);
            s += tabs + to_JSON_MACRO(time_per_graph);
            s += tabs + to_JSON_MACRO(alg_per_graph);
            s += tabs + to_JSON_MACRO(n_threads_per_graph);
            s += tabs + to_JSON_MACRO(imbalance_per_graph);
            s += tabs + to_JSON_MACRO(k_per_graph);
            s += tabs + to_JSON_MACRO(distance_per_graph);
            s += tabs + to_JSON_MACRO(edge_cut_per_graph);
            s += tabs + to_JSON_MACRO(weighted_edge_cut_per_graph);
            s += tabs + to_JSON_MACRO(comm_cost_per_graph);

            s.pop_back();
            s.pop_back();
            s += "\n" + tabs + "}";
            return s;
#endif
            return "{}";
        }
    };
}

#endif //SHAREDMAP_STAT_COLLECTOR_H
