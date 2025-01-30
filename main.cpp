#include <random>
#include "src/datastructures/solver.h"
#include "src/utility/algorithm_configuration.h"
#include "src/utility/command_line_parser.h"

using namespace SharedMap;

int main(const int argc, const char *argv[]) {
    // std::string graph_in = "../data/mapping/cop20k_A.mtx.graph";
    // std::string graph_in = "../data/mapping/afshell9.graph";
    std::string graph_in                 = "../data/mapping/2cubes_sphere.mtx.graph";
    // std::string graph_in = "../data/mapping/eur.graph";
    // std::string graph_in = "../data/mapping/144.graph";
    // std::string graph_in = "../data/mapping/bmwcra_1.mtx.graph";
    // std::string graph_in = "../data/mapping/deu.graph";
    // std::string graph_in = "../data/mapping/PGPgiantcompo.graph";
    // std::string graph_in = "../data/test/manual_graphs/0.graph";
    std::string hierarchy_string         = "4:8:6";
    std::string distance_string          = "1:10:100";
    f64         imbalance                = 0.03;
    // std::string config = "fast";
    std::string config                   = "strong";
    // std::string parallel_alg_string = "mtkahypar_default:mtkahypar_default:mtkahypar_default";
    std::string parallel_alg_string      = "mtkahypar_highest_quality:mtkahypar_highest_quality:mtkahypar_highest_quality";
    // std::string serial_alg_string = "kaffpa_fast:kaffpa_fast:kaffpa_fast";
    std::string serial_alg_string        = "kaffpa_strong:kaffpa_strong:kaffpa_strong";
    u64         n_threads                = 16;
    u64         seed                     = std::random_device{}();
    std::string parallel_strategy_string = "nb_layer";
    std::string mapping_out              = "mapping.txt";

    if (argc == 1) {
        CommandLineParser clp(argc, argv);
        clp.print_help_message();
        exit(0);
    }

    if (argc > 1) {
        CommandLineParser clp(argc, argv);

        graph_in         = clp.get("--graph");
        mapping_out      = clp.get("--mapping");
        hierarchy_string = clp.get("--hierarchy");
        distance_string  = clp.get("--distance");
        imbalance        = std::stod(clp.get("--imbalance"));

        size_t n_layers = 1 + std::count_if(hierarchy_string.begin(), hierarchy_string.end(), [](const char c) { return c == ':'; });
        config              = clp.get("--config");
        serial_alg_string   = parse_config_to_serial(config, n_layers);
        parallel_alg_string = parse_config_to_parallel(config, n_layers);

        n_threads                = std::stoi(clp.get("--threads"));
        parallel_strategy_string = clp.get("--parallelStrategy");

        if (clp.is_set("--seed")) {
            seed = std::stoi(clp.get("--seed"));
        } else {
            seed = std::random_device{}();
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
