#ifndef SHAREDMAP_COMMAND_LINE_PARSER_H
#define SHAREDMAP_COMMAND_LINE_PARSER_H

#include "src/utility/definitions.h"
#include "src/utility/macros.h"

namespace SharedMap {

    struct CommandLineOption {
        std::string large_key;
        std::string small_key;
        std::string description;
        std::string input;
        bool is_set = false;
    };

    class CommandLineParser {
    private:
        std::vector<CommandLineOption> options = {
                {"help", "", "produces help message", ""},
                {"graph", "g", "Filepath to the graph", ""},
                {"mapping", "m", "Output filepath to the generated mapping", ""},
                {"stats", "s", "Output filepath to measured statistics", ""},
                {"hierarchy", "h", "Hierarchy in the form a1:a2:...:al", ""},
                {"distance", "d", "Distance in the form d1:d2:...:dl", ""},
                {"imbalance", "e", "Allowed imbalance (for example 0.03)", ""},
                {"config", "c", "The configuration", ""},
                {"threads", "t", "Number of threads", ""},
                {"parallelStrategy", "", "Parallel strategy", ""},
                {"partitionAlgParallel", "", "Parallel Partitioning Algorithm", ""},
                {"partitionAlgSerial", "", "Serial Partitioning Algorithm", ""},
        };

    public:
        CommandLineParser(int argc, char *argv[]) {
            // read command lines into vector
            std::vector<std::string> args;
            args.reserve(argc);
            for (int i = 0; i < argc; ++i) {
                args.emplace_back(argv[i]);
            }

            // read all command line args
            for (int i = 1; i < argc; ++i) {
                if(args[i] == "help"){
                    print_help_message();
                    exit(EXIT_FAILURE);
                }

                for (auto &opt: options) {
                    if (opt.large_key == args[i] || opt.small_key == args[i]) {
                        opt.input = args[i+1];
                        opt.is_set = true;
                        i += 1;
                        break;
                    }
                }
            }
        }

        std::string get(const std::string &var) {
            for (const auto &opt: options) {
                if (opt.large_key == var || opt.small_key == var) {
                    if (opt.input.empty()) {
                        std::cout << "Command Line \"" << var << "\" not set!" << std::endl;
                        exit(EXIT_FAILURE);
                    }
                    return opt.input;
                }
            }
            std::cout << "Command Line \"" << var << "\" is not an allowed name!" << std::endl;
            exit(EXIT_FAILURE);
        }

        bool is_set(const std::string &var){
            for (const auto &opt: options) {
                if (opt.large_key == var || opt.small_key == var) {
                    return opt.is_set;
                }
            }
            std::cout << "Command Line \"" << var << "\" is not an allowed name!" << std::endl;
            exit(EXIT_FAILURE);
        }

        void print_help_message(){
            for (const auto &opt: options) {
                std::cout << "[ --" << opt.large_key << ", -" << opt.small_key << "] - " << opt.description << std::endl;
            }
        }
    };
}

#endif //SHAREDMAP_COMMAND_LINE_PARSER_H
