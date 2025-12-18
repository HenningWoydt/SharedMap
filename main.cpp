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

#include <random>

#include "src/datastructures/solver.h"
#include "src/utility/algorithm_configuration.h"
#include "src/utility/command_line_parser.h"

using namespace SharedMap;

int main(const int argc, const char *argv[]) {
    std::string graph_in                 = "../../GraPaRepo/data/mapping/rgg23.graph";
    std::string hierarchy_string         = "4:8:6";
    std::string distance_string          = "1:10:100";
    f64         imbalance                = 0.03;
    std::string config                   = "fast"; // "fast", "eco", "strong"
    u64         n_threads                = 4;
    u64         seed                     = std::random_device{}();
    std::string parallel_strategy_string = "nb_layer"; // "naive", "layer", "queue", "nb_layer"
    std::string mapping_out              = "mapping.txt";

    size_t      n_layers            = 1 + std::count_if(hierarchy_string.begin(), hierarchy_string.end(), [](const char c) { return c == ':'; });
    std::string serial_alg_string   = parse_config_to_serial(config, n_layers);
    std::string parallel_alg_string = parse_config_to_parallel(config, n_layers);

    if (argc == 1) {
        CommandLineParser clp(argc, argv);
        clp.print_help_message();
        // exit(0);
    } else {
        CommandLineParser clp(argc, argv);
        graph_in         = clp.get("--graph");
        mapping_out      = clp.get("--mapping");
        hierarchy_string = clp.get("--hierarchy");
        distance_string  = clp.get("--distance");
        imbalance        = std::stod(clp.get("--imbalance"));

        n_layers            = 1 + std::count_if(hierarchy_string.begin(), hierarchy_string.end(), [](const char c) { return c == ':'; });
        config              = clp.get("--config");
        serial_alg_string   = parse_config_to_serial(config, n_layers);
        parallel_alg_string = parse_config_to_parallel(config, n_layers);

        n_threads                = std::stoll(clp.get("--threads"));
        parallel_strategy_string = clp.get("--strategy");

        if (clp.is_set("--seed")) {
            seed = std::stoll(clp.get("--seed"));
        }
    }

    AlgorithmConfiguration ac(graph_in,
                              mapping_out,
                              hierarchy_string,
                              distance_string,
                              imbalance,
                              parallel_alg_string,
                              serial_alg_string,
                              n_threads,
                              parallel_strategy_string,
                              seed);

    Solver solver(ac);
    solver.solve();

    return 0;
}
