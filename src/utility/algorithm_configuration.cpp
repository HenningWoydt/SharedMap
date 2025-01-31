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

#include "algorithm_configuration.h"

namespace SharedMap {
    u64 parse_partitioning_algorithm(const std::string &alg) {
        std::vector<std::pair<std::string, u64>> algs = {
                {"kaffpa_fast",               KAFFPA_FAST},
                {"kaffpa_eco",                KAFFPA_ECO},
                {"kaffpa_strong",             KAFFPA_STRONG},
                {"mtkahypar_default",         MTKAHYPAR_DEFAULT},
                {"mtkahypar_quality",         MTKAHYPAR_QUALITY},
                {"mtkahypar_highest_quality", MTKAHYPAR_HIGHEST_QUALITY},
        };

        for (const auto &[fst, snd]: algs) {
            if (fst == alg) {
                return snd;
            }
        }

        std::cerr << "Algorithm " << alg << " not recognized" << std::endl;
        exit(EXIT_FAILURE);
    }

    std::string parse_config_to_serial(const std::string &config,
                                       const size_t n_layers) {
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

    std::string parse_config_to_parallel(const std::string &config,
                                         const size_t n_layers) {
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

    u64 parse_parallel_strategy(const std::string &strategy) {
        std::vector<std::pair<std::string, u64>> strategies = {
                {"naive",    NAIVE},
                {"layer",    LAYER},
                {"queue",    QUEUE},
                {"nb_layer", NB_LAYER}
        };

        for (const auto &[fst, snd]: strategies) {
            if (fst == strategy) {
                return snd;
            }
        }

        std::cerr << "Strategy " << strategy << " not recognized" << std::endl;
        exit(EXIT_FAILURE);
    }
}
