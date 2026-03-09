cmake_minimum_required(VERSION 3.16)
list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules)

project(SharedMap CXX C)
set(PROJECT_VENDOR "Henning Woydt")
set(PROJECT_CONTACT "henning.woydt@informatik.uni-heidelberg.de")
set(PROJECT_URL "https://github.com/HenningWoydt/SharedMap")
set(PROJECT_DESCRIPTION "SharedMap: Parallel Shared-Memory Process Mapping")
set(PROJECT_VERSION "1.0.0")

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# Set Release and Debug flags
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -g3 -DNDEBUG -march=native -mtune=native -flto -funroll-loops -fprefetch-loop-arrays -fomit-frame-pointer -falign-functions=32 -falign-loops=16 -Wall -Wextra -pedantic")
set(CMAKE_CXX_FLAGS_DEBUG "-g3 -O0 -Wall -Wextra -pedantic -fsanitize=undefined,address -fno-omit-frame-pointer -fstack-protector-strong -D_GLIBCXX_DEBUG")


# Threading / pthread
find_package(Threads REQUIRED)

# -------------------- KaHIP --------------------
set(KaHIP_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/extern/local/kahip")

find_library(KAHIP_LIBRARY
        NAMES kahip libkahip
        PATHS ${KaHIP_ROOT}/lib
        NO_DEFAULT_PATH
)

find_path(KAHIP_INCLUDE_DIR
        NAMES kaHIP_interface.h
        PATHS ${KaHIP_ROOT}/include
        NO_DEFAULT_PATH
)

if (NOT KAHIP_LIBRARY OR NOT KAHIP_INCLUDE_DIR)
    message(FATAL_ERROR "KaHIP not found under ${KaHIP_ROOT}. Did you run the build script to install KaHIP?")
endif ()

message(STATUS "Found KaHIP library: ${KAHIP_LIBRARY}")
message(STATUS "Found KaHIP include dir: ${KAHIP_INCLUDE_DIR}")

# -------------------- Mt-KaHyPar --------------------
list(APPEND CMAKE_PREFIX_PATH "${PROJECT_SOURCE_DIR}/extern/local/mt-kahypar")
find_package(MtKaHyPar CONFIG REQUIRED)
# --- Mt-KaHyPar libdir (lib vs lib64) ---
set(MTK_PREFIX "${PROJECT_SOURCE_DIR}/extern/local/mt-kahypar")
if (EXISTS "${MTK_PREFIX}/lib64")
    set(MTK_LIBDIR "${MTK_PREFIX}/lib64")
else()
    set(MTK_LIBDIR "${MTK_PREFIX}/lib")
endif()

message(STATUS "Found Mt-KaHyPar via CMake package: MtKaHyPar::mtkahypar")

# -------------------- Includes & Sources --------------------
include_directories(${PROJECT_SOURCE_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/lib)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
include_directories(${KAHIP_INCLUDE_DIR})
include_directories(${PROJECT_SOURCE_DIR}/extern/local/mt-kahypar/include)

file(GLOB_RECURSE SHAREDMAP_SOURCES CONFIGURE_DEPENDS "src/*.cpp")
file(GLOB_RECURSE SHAREDMAP_HEADERS CONFIGURE_DEPENDS "src/*.h")

# -------------------- Executable --------------------
# SharedMap executable
add_executable(SharedMap main.cpp ${SHAREDMAP_SOURCES} ${SHAREDMAP_HEADERS})
target_link_libraries(SharedMap PRIVATE
        ${KAHIP_LIBRARY}
        MtKaHyPar::mtkahypar
        Threads::Threads
)

# Shared library
add_library(libsharedmap OBJECT ${SHAREDMAP_SOURCES} ${SHAREDMAP_HEADERS})
add_library(sharedmap SHARED lib/libsharedmap.cpp $<TARGET_OBJECTS:libsharedmap>)

target_link_libraries(sharedmap PUBLIC
        ${KAHIP_LIBRARY}
        MtKaHyPar::mtkahypar
        Threads::Threads
)

# Runtime + link-time resolution for indirect deps of libmtkahypar.so
foreach(tgt SharedMap sharedmap)
    target_link_options(${tgt} PRIVATE
            "-Wl,--disable-new-dtags"
            "-Wl,-rpath-link,${MTK_LIBDIR}"
    )
    set_target_properties(${tgt} PROPERTIES
            BUILD_RPATH "${KaHIP_ROOT}/lib;${MTK_LIBDIR}"
    )
endforeach()

# -------------------- Python Bindings --------------------
if(SKBUILD)
    find_package(Python REQUIRED COMPONENTS Interpreter Development.Module)
    find_package(pybind11 CONFIG REQUIRED)
    
    pybind11_add_module(_sharedmap python/sharedmap_bindings.cpp $<TARGET_OBJECTS:libsharedmap>)
    target_link_libraries(_sharedmap PRIVATE
            ${KAHIP_LIBRARY}
            MtKaHyPar::mtkahypar
            Threads::Threads
    )
    target_link_options(_sharedmap PRIVATE
            "-Wl,--disable-new-dtags"
            "-Wl,-rpath-link,${MTK_LIBDIR}"
    )
    set_target_properties(_sharedmap PROPERTIES
            BUILD_RPATH "${KaHIP_ROOT}/lib;${MTK_LIBDIR}"
            INSTALL_RPATH "$ORIGIN"
    )
    
    install(TARGETS _sharedmap LIBRARY DESTINATION sharedmap)
endif()