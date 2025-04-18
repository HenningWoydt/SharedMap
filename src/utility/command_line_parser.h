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

#ifndef SHAREDMAP_COMMAND_LINE_PARSER_H
#define SHAREDMAP_COMMAND_LINE_PARSER_H

namespace SharedMap {
    /**
     * Holds one option for the command line.
     */
    struct CommandLineOption {
        std::string large_key;
        std::string small_key;
        std::string description;
        std::string input;
        bool is_set = false;
    };

    /**
     * Class to parse the command line.
     */
    class CommandLineParser {
    private:
        std::vector<CommandLineOption> options = {
                {"--help", "", "produces help message", ""},
                {"--graph", "-g", "Filepath to the graph", ""},
                {"--mapping", "-m", "Output filepath to the generated mapping", ""},
                {"--hierarchy", "-h", "Hierarchy in the form a1:a2:...:al", ""},
                {"--distance", "-d", "Distance in the form d1:d2:...:dl", ""},
                {"--imbalance", "-e", "Allowed imbalance (for example 0.03)", ""},
                {"--config", "-c", "The configuration", ""},
                {"--threads", "-t", "Number of threads", ""},
                {"--strategy", "-s", "Parallel strategy", ""},
                {"--seed", "", "Seed for diversifying partitioning", ""},
            };

    public:
        /**
         * Constructor taking in the command line.
         *
         * @param argc Number of arguments.
         * @param argv The arguments.
         */
        CommandLineParser(const int argc, const char* argv[]) {
            // read command lines into vector
            std::vector<std::string> args(argv, argv + argc);

            // read all command line args
            for (int i = 1; i < argc; ++i) {
                if (args[i] == "--help") {
                    print_help_message();
                    exit(EXIT_SUCCESS);
                }

                for (auto& [large_key, small_key, description, input, is_set] : options) {
                    if (large_key == args[i] || small_key == args[i]) {
                        input  = args[i + 1];
                        is_set = true;
                        i += 1;
                        break;
                    }
                }
            }
        }

        /**
         * Gets the entered input as a string.
         *
         * @param var The option in interest.
         * @return The input.
         */
        std::string get(const std::string& var) {
            for (const auto& [large_key, small_key, description, input, is_set] : options) {
                if (large_key == var || small_key == var) {
                    if (input.empty()) {
                        std::cout << "Command Line \"" << var << "\" not set!" << std::endl;
                        exit(EXIT_FAILURE);
                    }
                    return input;
                }
            }
            std::cout << "Command Line \"" << var << "\" is not an allowed name!" << std::endl;
            exit(EXIT_FAILURE);
        }

        /**
         * Returns whether the option was entered.
         *
         * @param var The option in interest.
         * @return True if the option was entered, false else.
         */
        bool is_set(const std::string& var) {
            for (const auto& [large_key, small_key, description, input, is_set] : options) {
                if (large_key == var || small_key == var) {
                    return is_set;
                }
            }
            std::cout << "Command Line \"" << var << "\" is not an allowed name!" << std::endl;
            exit(EXIT_FAILURE);
        }

        /**
         * Prints the help message.
         */
        void print_help_message() {
            for (const auto& [large_key, small_key, description, input, is_set] : options) {
                if (small_key.empty()) {
                    std::cout << "[ " << large_key << "] - " << description << std::endl;
                } else {
                    std::cout << "[ " << large_key << ", " << small_key << "] - " << description << std::endl;
                }
            }
        }
    };
}

#endif //SHAREDMAP_COMMAND_LINE_PARSER_H
