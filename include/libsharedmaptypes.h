#ifndef SHAREDMAP_LIBSHAREDMAPTYPES_H
#define SHAREDMAP_LIBSHAREDMAPTYPES_H

/** TODO */
typedef enum {
    NAIVE,
    LAYER,
    QUEUE,
    NB_LAYER
} shared_map_distribution_type_t;

/** TODO */
typedef enum {
    KAFFPA_FAST,
    KAFFPA_ECO,
    KAFFPA_STRONG,
    MTKAHYPAR_DEFAULT,
    MTKAHYPAR_QUALITY,
    MTKAHYPAR_HIGHEST_QUALITY
} shared_map_partitioning_type_t;

class shared_map_graph;

#endif //SHAREDMAP_LIBSHAREDMAPTYPES_H
