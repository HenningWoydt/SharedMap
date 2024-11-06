#include "utils.h"

#include <iostream>

namespace SharedMap {
    std::vector<std::string> split(const std::string &str,
                                   char c) {
        std::vector<std::string> splits;

        std::istringstream iss(str);
        std::string        token;

        while (std::getline(iss, token, c)) {
            splits.push_back(token);
        }

        return splits;
    }

    bool file_exists(const std::string &path) {
        std::ifstream f(path.c_str());
        return f.good();
    }

    std::string read_file(const std::string &path) {
        std::ifstream     t(path);
        std::stringstream buffer;
        buffer << t.rdbuf();
        return buffer.str();
    }

    void line_to_ints(const std::string &line, std::vector<u64> &ints) {
        ints.resize(line.size());
        u64 idx         = 0;
        u64 curr_number = 0;

        for (char c: line) {
            if (c == ' ') {
                ints[idx]   = curr_number;
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

    void busyFunction(float duration) {
        auto           start         = std::chrono::high_resolution_clock::now();
        auto           end           = start;
        volatile float uselessResult = 0.0f;

        // Continue running until the specified duration has passed
        while (std::chrono::duration<float>(end - start).count() < duration) {
            // Perform some meaningless calculations
            for (int i = 0; i < 1000; ++i) {
                uselessResult += std::sqrt(static_cast<float>(i)) * std::sqrt(static_cast<float>(i + 1));
                uselessResult -= std::sqrt(static_cast<float>(i + 1)) * std::sqrt(static_cast<float>(i));
            }
            // Update the end time
            end        = std::chrono::high_resolution_clock::now();
        }
    }

    // Function to trim leading and trailing spaces
    std::string trim(std::string str) {
        if (str.empty()) {
            return str;
        }

        // Find the first non-space character
        size_t start = 0;
        while (start < str.size() && std::isspace(str[start])) {
            ++start;
        }

        // Find the last non-space character
        size_t end = str.size() - 1;
        while (end > start && std::isspace(str[end])) {
            --end;
        }

        // Resize and move the string to contain only the trimmed part
        return str.substr(start, end - start + 1);
    }

    bool startsWith(const std::string &s, const std::string &start) {
        if (s.size() < start.size()) return false;
        return s.compare(0, start.size(), start) == 0;
    }

    bool endsWith(const std::string &s, const std::string &end) {
        if (s.size() < end.size()) return false;
        return s.compare(s.size() - end.size(), end.size(), end) == 0;
    }

    void locked_print(std::mutex &lock, const std::string &s) {
        lock.lock();
        std::cout << s << std::endl;
        lock.unlock();
    }
}
