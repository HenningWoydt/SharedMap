#ifndef SHAREDMAP_UTILS_H
#define SHAREDMAP_UTILS_H

#include <fstream>
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
    std::vector<T> convert(const std::vector<std::string> &&vec) {
        std::vector<T> v;

        for (auto &s: vec) {
            v.push_back(convert_to<T>(s));
        }

        return v;
    }

    /**
     * Converts a string that holds integers to a vector of integers.
     *
     * @param str The string.
     * @param ints The vector that will hold the solution.
     */
    void str_to_ints(const std::string &str,
                     std::vector<u64> &ints);

    /**
     * Multiplies all elements in the vector.
     *
     * @tparam T Vector type.
     * @param vec The vector.
     * @return The product.
     */
    template<typename T>
    T product(const std::vector<T> &vec) {
        T p = (T) 1;
        for (auto &x: vec) { p *= x; }
        return p;
    }

    /**
     * Checks if a file exists at the specified path.
     *
     * @param path The file path.
     * @return True if the file exists, false else.
     */
    bool file_exists(const std::string &path);
}

#endif //SHAREDMAP_UTILS_H
