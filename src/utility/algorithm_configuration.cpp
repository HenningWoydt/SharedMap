#include "algorithm_configuration.h"

namespace SharedMap {
    u64 parse_partitioning_algorithm(const std::string& alg) {
        std::vector<std::pair<std::string, u64>> algs = {
                {"greedy", GREEDY},
                {"kaffpa_strong", KAFFPA_STRONG},
                {"kaffpa_eco", KAFFPA_ECO},
                {"kaffpa_fast", KAFFPA_FAST},
                {"mtkahypar_default", MTKAHYPAR_DEFAULT},
                {"mtkahypar_quality", MTKAHYPAR_QUALITY},
                {"mtkahypar_highest_quality", MTKAHYPAR_HIGHEST_QUALITY},
            };

        for (auto& pair : algs) {
            if (pair.first == alg) {
                return pair.second;
            }
        }

        std::cerr << "Algorithm " << alg << " not recognized" << std::endl;
        exit(EXIT_FAILURE);
    }

    std::string parse_config_to_serial(const std::string& config, size_t n_layers) {
        std::string alg;
        for (size_t i = 0; i < n_layers; ++i) {
            std::string temp;
            if (config == "strong") {
                temp = "kaffpa_strong";
            } else if (config == "eco") {
                temp = "kaffpa_eco";
            } else if (config == "fast") {
                temp = "kaffpa_fast";
            } else {
                std::cout << "config " << config << " not recognized!" << std::endl;
                exit(EXIT_FAILURE);
            }
            alg += temp + ":";
        }
        alg.pop_back();
        return alg;
    }

    std::string parse_config_to_parallel(const std::string& config, size_t n_layers) {
        std::string alg;
        for (size_t i = 0; i < n_layers; ++i) {
            std::string temp;
            if (config == "strong") {
                temp = "mtkahypar_highest_quality";
            } else if (config == "eco") {
                temp = "mtkahypar_quality";
            } else if (config == "fast") {
                temp = "mtkahypar_default";
            } else {
                std::cout << "config " << config << " not recognized!" << std::endl;
                exit(EXIT_FAILURE);
            }
            alg += temp + ":";
        }
        alg.pop_back();
        return alg;
    }

    u64 parse_parallel_strategy(const std::string& strategy) {
        std::vector<std::pair<std::string, u64>> strategies = {
                {"serial", SERIAL},
                {"naive", NAIVE},
                {"layer", LAYER},
                {"queue", QUEUE},
                {"layer_nb", LAYER_NB}
            };

        for (auto& pair : strategies) {
            if (pair.first == strategy) {
                return pair.second;
            }
        }

        std::cerr << "Strategy " << strategy << " not recognized" << std::endl;
        exit(EXIT_FAILURE);
    }
}
