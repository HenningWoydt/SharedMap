#include "src/datastructures/solver.h"
#include "src/utility/algorithm_configuration.h"
#include "src/utility/command_line_parser.h"

using namespace SharedMap;

int main(int argc, char *argv[]) {
    // std::string graph_in = "../data/mapping/cop20k_A.mtx.graph";
    // std::string graph_in = "../data/mapping/afshell9.graph";
    // std::string graph_in = "../data/mapping/2cubes_sphere.mtx.graph";
    std::string graph_in = "../data/mapping/eur.graph";
    // std::string graph_in = "../data/mapping/deu.graph";
    // std::string graph_in = "../data/mapping/PGPgiantcompo.graph";
    // std::string graph_in = "../data/test/manual_graphs/0.graph";
    std::string hierarchy_string = "4:8:6";
    std::string distance_string = "1:10:100";
    f64 imbalance = 0.03;
    std::string config = "fast";
    std::string parallel_alg_string = "mtkahypar_default:mtkahypar_default:mtkahypar_default";
    std::string serial_alg_string = "kaffpa_fast:kaffpa_fast:kaffpa_fast";
    u64 n_threads = 10;
    std::string parallel_strategy_string = "queue";
    std::string mapping_out = "mapping.txt";
    std::string statistics_out = parallel_strategy_string + "_statistics.JSON";

    bool use_command_line = argc > 1;
    if (use_command_line) {
        CommandLineParser clp(argc, argv);

        graph_in = clp.get("graph");
        mapping_out = clp.get("mapping");
        statistics_out = clp.get("stats");
        hierarchy_string = clp.get("hierarchy");
        distance_string = clp.get("distance");
        imbalance = std::stod(clp.get("imbalance"));

        size_t n_layers = 1 + std::count_if( hierarchy_string.begin(), hierarchy_string.end(), []( char c ){return c ==':';});
        if(clp.is_set("config")){
            config = clp.get("config");
            serial_alg_string = parse_config_to_serial(config, n_layers);
            parallel_alg_string = parse_config_to_parallel(config, n_layers);
        }
        if(clp.is_set("partitionAlgSerial")){
            serial_alg_string = clp.get("partitionAlgParallel");
        }
        if(clp.is_set("partitionAlgParallel")){
            parallel_alg_string = clp.get("partitionAlgParallel");
        }

        n_threads = std::stoi(clp.get("threads"));
        parallel_strategy_string = clp.get("parallelStrategy");
    }

    AlgorithmConfiguration ac(graph_in,
                              mapping_out,
                              statistics_out,
                              hierarchy_string,
                              distance_string,
                              imbalance,
                              parallel_alg_string,
                              serial_alg_string,
                              n_threads,
                              parallel_strategy_string);

    Solver solver(ac);
    solver.solve();

    return 0;
}
