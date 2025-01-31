#ifndef SHAREDMAP_ALGORITHM_CONFIGURATION_H
#define SHAREDMAP_ALGORITHM_CONFIGURATION_H

#include <string>

#include "src/utility/definitions.h"
#include "src/utility/JSON_utils.h"
#include "src/utility/macros.h"
#include "src/utility/utils.h"
#include "include/libsharedmaptypes.h"

namespace SharedMap {
    /**
     * Parses the partitioning algorithm given in the command line to the
     * correct id.
     *
     * @param alg The Algorithm.
     * @return The algorithm id.
     */
    u64 parse_partitioning_algorithm(const std::string &alg);

    /**
     * Generates the serial alg string.
     *
     * @param config The config e.g. "strong", "eco", "fast".
     * @param n_layers Number of layers.
     * @return The serial configuration.
     */
    std::string parse_config_to_serial(const std::string &config,
                                       size_t n_layers);

    /**
     * Generates the parallel alg string.
     *
     * @param config The config e.g. "strong", "eco", "fast".
     * @param n_layers Number of layers.
     * @return The parallel configuration.
     */
    std::string parse_config_to_parallel(const std::string &config,
                                         size_t n_layers);

    /**
     * Parses the parallel strategy given in the command line to the
     * correct id.
     *
     * @param strategy The Strategy.
     * @return The strategy id.
     */
    u64 parse_parallel_strategy(const std::string &strategy);

    /**
    * Class to store the configuration of the algorithm.
    */
    class AlgorithmConfiguration {
    public:
        // graph information
        std::string graph_in;
        std::string mapping_out;

        // hierarchy information
        std::string      hierarchy_string;
        std::vector<u64> hierarchy;
        u64              k;

        // distance information
        std::string      distance_string;
        std::vector<u64> distance;

        // info for correctly identifying subgraphs
        std::vector<u64> index_vec; // index vector to correctly offset all resulting graphs
        std::vector<u64> k_rem_vec; // remaining k vector

        // balancing information
        f64 imbalance;

        // partitioning algorithm
        std::string      parallel_alg_string;
        std::vector<u64> parallel_alg_id;
        std::string      serial_alg_string;
        std::vector<u64> serial_alg_id;

        // number of threads
        u64 n_threads;

        // parallel strategy
        std::string parallel_strategy_string;
        u64         parallel_strategy_id;

        // random initialization
        u64 seed;

        AlgorithmConfiguration(const std::string &graph_in,
                               const std::string &mapping_out,
                               const std::string &hierarchy_string,
                               const std::string &distance_string,
                               const f64 imbalance,
                               const std::string &parallel_alg_string,
                               const std::string &serial_alg_string,
                               const u64 n_threads,
                               const std::string &parallel_strategy_string,
                               const u64 seed) {
            // graph information
            this->graph_in    = graph_in;
            this->mapping_out = mapping_out;

            // hierarchy information
            this->hierarchy_string = hierarchy_string;
            this->hierarchy        = convert<u64>(split(hierarchy_string, ':'));
            this->k                = product(hierarchy);

            // distance information
            this->distance_string = distance_string;
            this->distance        = convert<u64>(split(distance_string, ':'));

            // info for correctly identifying subgraphs
            index_vec = {1};
            for (u64 i = 0; i < hierarchy.size() - 1; ++i) {
                index_vec.push_back(index_vec[i] * hierarchy[i]);
            }

            k_rem_vec.resize(hierarchy.size());
            u64      p = 1;
            for (u64 i = 0; i < hierarchy.size(); ++i) {
                k_rem_vec[i] = p * hierarchy[i];
                p *= hierarchy[i];
            }

            // balancing information
            this->imbalance = imbalance;

            // partitioning algorithm
            this->parallel_alg_string = parallel_alg_string;
            std::vector<std::string> temp_parallel = split(parallel_alg_string, ':');
            for (auto                &s: temp_parallel) { parallel_alg_id.push_back(parse_partitioning_algorithm(s)); }
            this->serial_alg_string = serial_alg_string;
            std::vector<std::string> temp_serial = split(serial_alg_string, ':');
            for (auto                &s: temp_serial) { serial_alg_id.push_back(parse_partitioning_algorithm(s)); }

            // number of threads
            this->n_threads = n_threads;

            // parallel strategy
            this->parallel_strategy_string = parallel_strategy_string;
            this->parallel_strategy_id     = parse_parallel_strategy(parallel_strategy_string);

            // random initialization
            this->seed = seed;

            if (hierarchy.size() != distance.size()) {
                std::cout << "Hierarchy (size " << hierarchy.size() << ") and Distance (size " << distance.size() << ") are not equal!" << std::endl;
                exit(EXIT_FAILURE);
            }
            if (hierarchy.size() != serial_alg_id.size()) {
                std::cout << "Hierarchy (size " << hierarchy.size() << ") and Serial Alg (size " << serial_alg_id.size() << ") are not equal!" << std::endl;
                exit(EXIT_FAILURE);
            }
            if (hierarchy.size() != parallel_alg_id.size()) {
                std::cout << "Hierarchy (size " << hierarchy.size() << ") and Parallel Alg (size " << parallel_alg_id.size() << ") are not equal!" << std::endl;
                exit(EXIT_FAILURE);
            }
        }

        AlgorithmConfiguration(const std::vector<u64> &hierarchy_vec,
                               const std::vector<u64> &distance_vec,
                               const f64 imbalance,
                               const shared_map_algorithm_type_t &parallel_alg,
                               const shared_map_algorithm_type_t &serial_alg,
                               const u64 n_threads,
                               const shared_map_strategy_type_t &distribution,
                               const u64 seed) {
            // hierarchy information
            this->hierarchy = hierarchy_vec;
            this->k         = product(hierarchy);

            // distance information
            this->distance = distance_vec;

            // info for correctly identifying subgraphs
            index_vec = {1};
            for (u64 i = 0; i < hierarchy.size() - 1; ++i) {
                index_vec.push_back(index_vec[i] * hierarchy[i]);
            }

            k_rem_vec.resize(hierarchy.size());
            u64      p = 1;
            for (u64 i = 0; i < hierarchy.size(); ++i) {
                k_rem_vec[i] = p * hierarchy[i];
                p *= hierarchy[i];
            }

            // balancing information
            this->imbalance = imbalance;

            // partitioning algorithm
            for (size_t i = 0; i < hierarchy.size(); ++i) {
                parallel_alg_id.push_back(parallel_alg);
                serial_alg_id.push_back(serial_alg);
            }

            // number of threads
            this->n_threads = n_threads;

            // parallel strategy
            this->parallel_strategy_id = distribution;

            // random initialization
            this->seed = seed;

            if (hierarchy.size() != distance.size()) {
                std::cout << "Hierarchy (size " << hierarchy.size() << ") and Distance (size " << distance.size() << ") are not equal!" << std::endl;
                exit(EXIT_FAILURE);
            }
            if (hierarchy.size() != serial_alg_id.size()) {
                std::cout << "Hierarchy (size " << hierarchy.size() << ") and Serial Alg (size " << serial_alg_id.size() << ") are not equal!" << std::endl;
                exit(EXIT_FAILURE);
            }
            if (hierarchy.size() != parallel_alg_id.size()) {
                std::cout << "Hierarchy (size " << hierarchy.size() << ") and Parallel Alg (size " << parallel_alg_id.size() << ") are not equal!" << std::endl;
                exit(EXIT_FAILURE);
            }
        }

        /**
         * Converts algorithm configuration into a string in JSON format.
         *
         * @param n_tabs Number of tabs appended in front of each line (for visual purposes).
         * @return String in JSON format.
         */
        std::string to_JSON(const u64 n_tabs = 0) const {
            std::string tabs;
            for (size_t i = 0; i < n_tabs; ++i) { tabs.push_back('\t'); }

            std::string s = "{\n";

            s += tabs + to_JSON_MACRO(graph_in);
            s += tabs + to_JSON_MACRO(mapping_out);
            s += tabs + to_JSON_MACRO(hierarchy_string);
            s += tabs + to_JSON_MACRO(hierarchy);
            s += tabs + to_JSON_MACRO(k);
            s += tabs + to_JSON_MACRO(distance_string);
            s += tabs + to_JSON_MACRO(distance);
            s += tabs + to_JSON_MACRO(index_vec);
            s += tabs + to_JSON_MACRO(k_rem_vec);
            s += tabs + to_JSON_MACRO(imbalance);
            s += tabs + to_JSON_MACRO(parallel_alg_string);
            s += tabs + to_JSON_MACRO(parallel_alg_id);
            s += tabs + to_JSON_MACRO(serial_alg_string);
            s += tabs + to_JSON_MACRO(serial_alg_id);
            s += tabs + to_JSON_MACRO(n_threads);
            s += tabs + to_JSON_MACRO(parallel_strategy_string);
            s += tabs + to_JSON_MACRO(parallel_strategy_id);

            s.pop_back();
            s.pop_back();
            s += "\n" + tabs + "}";
            return s;
        }
    };
}

#endif //SHAREDMAP_ALGORITHM_CONFIGURATION_H
