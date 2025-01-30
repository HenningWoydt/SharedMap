#ifndef SHAREDMAP_LIBSHAREDMAPTYPES_H
#define SHAREDMAP_LIBSHAREDMAPTYPES_H

/** TODO */
typedef enum {
    NAIVE,
    LAYER,
    QUEUE,
    NB_LAYER
} shared_map_distribution_type_t;

std::string distribution_to_string(shared_map_distribution_type_t distribution);

/** TODO */
typedef enum {
    KAFFPA_FAST,
    KAFFPA_ECO,
    KAFFPA_STRONG,
    MTKAHYPAR_DEFAULT,
    MTKAHYPAR_QUALITY,
    MTKAHYPAR_HIGHEST_QUALITY
} shared_map_algorithm_type_t;

std::string algorithm_to_string(shared_map_algorithm_type_t algorithm);

#endif //SHAREDMAP_LIBSHAREDMAPTYPES_H
