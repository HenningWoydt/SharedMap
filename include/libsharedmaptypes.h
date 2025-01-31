#ifndef SHAREDMAP_LIBSHAREDMAPTYPES_H
#define SHAREDMAP_LIBSHAREDMAPTYPES_H

#include <string>

typedef enum {
    NAIVE,
    LAYER,
    QUEUE,
    NB_LAYER
} shared_map_strategy_type_t;

/**
 * Converts the given enum into a string.
 *
 * @param strategy The strategy.
 * @return String representation.
 */
std::string strategy_to_string(shared_map_strategy_type_t strategy);

typedef enum {
    KAFFPA_FAST,
    KAFFPA_ECO,
    KAFFPA_STRONG,
    MTKAHYPAR_DEFAULT,
    MTKAHYPAR_QUALITY,
    MTKAHYPAR_HIGHEST_QUALITY
} shared_map_algorithm_type_t;

/**
 * Converts the given enum into a string.
 *
 * @param algorithm The algorithm.
 * @return String representation.
 */
std::string algorithm_to_string(shared_map_algorithm_type_t algorithm);

#endif //SHAREDMAP_LIBSHAREDMAPTYPES_H
