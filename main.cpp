#include <random>
#include "src/datastructures/solver.h"
#include "src/utility/algorithm_configuration.h"
#include "src/utility/command_line_parser.h"

using namespace SharedMap;

int main(const int argc, const char *argv[]) {
    std::string graph_in                 = "../data/training/af_shell10.graph";
    std::string hierarchy_string         = "4:8:6";
    std::string distance_string          = "1:10:100";
    f64         imbalance                = 0.03;
    std::string config                   = "strong"; // "fast", "eco", "strong"
    u64         n_threads                = 16;
    u64         seed                     = std::random_device{}();
    std::string parallel_strategy_string = "nb_layer"; // "naive", "layer", "queue", "nb_layer"
    std::string mapping_out              = "mapping.txt";

    size_t n_layers = 1 + std::count_if(hierarchy_string.begin(), hierarchy_string.end(), [](const char c) { return c == ':'; });
    std::string serial_alg_string   = parse_config_to_serial(config, n_layers);
    std::string parallel_alg_string = parse_config_to_parallel(config, n_layers);

    if (argc == 1) {
        CommandLineParser clp(argc, argv);
        clp.print_help_message();
        exit(0);
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

        n_threads                = std::stoi(clp.get("--threads"));
        parallel_strategy_string = clp.get("--parallelStrategy");

        if (clp.is_set("--seed")) {
            seed = std::stoi(clp.get("--seed"));
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
