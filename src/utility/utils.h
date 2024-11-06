#ifndef SHAREDMAP_UTILS_H
#define SHAREDMAP_UTILS_H

#include <algorithm>
#include <cmath>
#include <fstream>
#include <mutex>
#include <sstream>
#include <string>
#include <vector>

#include "src/utility/definitions.h"

namespace SharedMap {
    /**
     * Splits a string into multiple sub-strings. The specified character will
     * serve as the delimiter and will not be present in any string.
     *
     * @param str The string.
     * @param c The character.
     * @return Vector of sub-strings.
     */
    std::vector<std::string> split(const std::string &str,
                                   char c);

    /**
     * Converts a string into the specified datatype. Conversion is done via
     * string stream and ">>" operator.
     *
     * @tparam T The desired datatype.
     * @param str The string.
     * @return The converted string.
     */
    template<typename T>
    T convert_to(const std::string &str) {
        T                  result;
        std::istringstream iss(str);
        iss >> result;
        return result;
    }

    /**
     * Converts the vector of strings into a vector of T's.
     *
     * @tparam T Type of conversion.
     * @param vec The vector.
     * @return Vector of transformed T's.
     */
    template<typename T>
    std::vector<T> convert(const std::vector<std::string> &vec) {
        std::vector<T> v;

        for (auto &s: vec) {
            v.push_back(convert_to<T>(s));
        }

        return v;
    }

    /**
     * Converts the vector of strings into a vector of T's.
     *
     * @tparam T Type of conversion.
     * @param vec The vector.
     * @return Vector of transformed T's.
     */
    template<typename T>
    std::vector<T> convert(const std::vector<std::string> &&vec) {
        std::vector<T> v;

        for (auto &s: vec) {
            v.push_back(convert_to<T>(s));
        }

        return v;
    }

    void line_to_ints(const std::string &line, std::vector<u64> &ints);

    /**
     * Multiplies all elements in the vector.
     *
     * @tparam T1 Resulting type.
     * @tparam T2 Type of vector elements.
     * @param vec The vector.
     * @return The product.
     */
    template<typename T1, typename T2>
    T1 prod(const std::vector<T2> &vec) {
        T1 p = (T1) 1;

        for (auto &x: vec) {
            p *= (T1) x;
        }

        return p;
    }

    /**
     * Sums all the elements in the vector.
     *
     * @tparam T1 Resulting type.
     * @tparam T2 Type of vector elements.
     * @param vec The vector.
     * @return The sum.
     */
    template<typename T1, typename T2>
    T1 sum(const std::vector<T2> &vec) {
        T1 s = (T1) 0;

        for (auto &x: vec) {
            s += (T1) x;
        }

        return s;
    }

    /**
     * Determines the maximum in the vector.
     *
     * @tparam T Resulting type.
     * @param vec The vector.
     * @return The maximum.
     */
    template<typename T>
    T max(const std::vector<T> &vec) {
        T m = vec[0];

        for (auto &x: vec) {
            m = std::max(m, x);
        }

        return m;
    }

    /**
     * Determines if an element exist in the vector. The "==" operator is used
     * to determine if elements are equal.
     *
     * @tparam T Type of element and vector elements.
     * @param vec The vector.
     * @param x The element to find.
     * @return Vector of transformed T's.
     */
    template<typename T>
    bool exists(const std::vector<T> &vec, const T &x) {
        return std::find(vec.begin(), vec.end(), x) != vec.end();
    }

    /**
     * Determines if a duplicate exists. The "==" operator is used to determine
     * if elements are equal.
     *
     * @tparam T Type of element and vector elements.
     * @param vec The vector.
     * @return True if no duplicate exists, false else.
     */
    template<typename T>
    bool no_duplicates(const std::vector<T> &vec) {
        for (u64 i = 0; i < vec.size(); ++i) {
            for (u64 j = i + 1; j < vec.size(); ++j) {
                if (vec[i] == vec[j]) {
                    return false;
                }
            }
        }
        return true;
    }

    /**
     * Checks if a file exists at the specified path.
     *
     * @param path The file path.
     * @return True if the file exists, false else.
     */
    bool file_exists(const std::string &path);

    std::string read_file(const std::string &path);

    template<typename T>
    std::string to_string(const std::vector<T> &vec) {
        std::string s;
        if (vec.empty()) {
            s = "[]";
            return s;
        }
        s = "[";
        for (size_t i = 0; i < vec.size() - 1; ++i) {
            s += std::to_string(vec[i]) + ", ";
        }
        s += std::to_string(vec.back()) + "]";

        return s;
    }

    template<typename T>
    std::string concat(const std::vector<T> &vec) {
        std::string s;
        if (vec.empty()) {
            s = "[]";
            return s;
        }
        s = "[";
        for (size_t i = 0; i < vec.size() - 1; ++i) {
            s += vec[i] + ", ";
        }
        s += vec.back() + "]";

        return s;
    }

    void busyFunction(float duration);

    // Function to trim leading and trailing spaces
    std::string trim(std::string str);

    bool startsWith(const std::string &s, const std::string &start);

    bool endsWith(const std::string &s, const std::string &end);

    void locked_print(std::mutex &lock, std::string);
}

#endif //SHAREDMAP_UTILS_H
