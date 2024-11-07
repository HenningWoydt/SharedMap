#include "utils.h"

namespace SharedMap {
    std::vector<std::string> split(const std::string& str,
                                   const char c) {
        std::vector<std::string> splits;

        std::istringstream iss(str);
        std::string token;

        while (std::getline(iss, token, c)) {
            splits.push_back(token);
        }

        return splits;
    }

    bool file_exists(const std::string& path) {
        std::ifstream f(path.c_str());
        return f.good();
    }

    void str_to_ints(const std::string& str,
                     std::vector<u64>& ints) {
        ints.resize(str.size());

        u64 idx         = 0;
        u64 curr_number = 0;

        for (const char c : str) {
            if (c == ' ') {
                ints[idx] = curr_number;
                idx += curr_number != 0;
                curr_number = 0;
            } else {
                curr_number = curr_number * 10 + (c - '0');
            }
        }

        ints[idx] = curr_number;
        idx += curr_number != 0;
        ints.resize(idx);
    }
}
