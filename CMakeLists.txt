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

# CMake Options
option(SHAREDMAP_DOWNLOAD_TBB "Downloads TBB." ON)

# Set Release and Debug flags
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -g3 -DNDEBUG -march=native -mtune=native -flto -funroll-loops -fprefetch-loop-arrays -fomit-frame-pointer -falign-functions=32 -falign-loops=16 -Wall -Wextra -pedantic")
set(CMAKE_CXX_FLAGS_DEBUG   "-g3 -O0 -Wall -Wextra -pedantic -fsanitize=undefined,address -fno-omit-frame-pointer -fstack-protector-strong -D_GLIBCXX_DEBUG")

# -------------------- TBB --------------------
if (SHAREDMAP_DOWNLOAD_TBB)
    include(FetchContent)
    FetchContent_Populate(
            tbb
            URL https://github.com/oneapi-src/oneTBB/releases/download/v2021.12.0/oneapi-tbb-2021.12.0-lin.tgz
            SOURCE_DIR ${CMAKE_CURRENT_BINARY_DIR}/external_tools/tbb
    )
    # Hint for FindTBB
    set(TBB_ROOT ${CMAKE_CURRENT_BINARY_DIR}/external_tools/tbb CACHE PATH "Path to downloaded TBB" FORCE)
endif ()

# Find TBB via CMake's FindTBB module
find_package(TBB REQUIRED)

if (TBB_FOUND)
    if (TBB_INTERFACE_VERSION GREATER_EQUAL 12050)
        message(STATUS "TBB Version: ${TBB_VERSION_MAJOR}.${TBB_VERSION_MINOR} (Interface Version=${TBB_INTERFACE_VERSION})")
        message(STATUS "Found TBB library: inc=${TBB_INCLUDE_DIRS}, lib=${TBB_LIBRARIES}")
        message(STATUS "Found TBB Malloc library: inc=${TBB_MALLOC_INCLUDE_DIRS}, lib=${TBB_MALLOC_LIBRARIES}")
        message(STATUS "Found TBB Malloc Proxy library: inc=${TBB_MALLOC_PROXY_INCLUDE_DIRS}, lib=${TBB_MALLOC_PROXY_LIBRARIES}")
    else ()
        message(FATAL_ERROR
                "Your current TBB version ${TBB_INTERFACE_VERSION} is too old. The minimum required version is 12050 (OneTBB 2021.5).
Add -DSHAREDMAP_DOWNLOAD_TBB=ON to the cmake build command (may require to clear the cmake cache).")
    endif ()
else ()
    message(FATAL_ERROR
            "TBB library not found. Add -DSHAREDMAP_DOWNLOAD_TBB=ON to the cmake build command (may require to clear the cmake cache).")
endif ()

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
endif()

message(STATUS "Found KaHIP library: ${KAHIP_LIBRARY}")
message(STATUS "Found KaHIP include dir: ${KAHIP_INCLUDE_DIR}")

# -------------------- Mt-KaHyPar --------------------
find_library(MTKAHYPAR_LIBRARY
        NAMES mtkahypar
        PATHS
        ${PROJECT_SOURCE_DIR}/extern/local/mt-kahypar/lib
        ${PROJECT_SOURCE_DIR}/extern/local/mt-kahypar/lib64
        NO_DEFAULT_PATH
)

if (MTKAHYPAR_LIBRARY)
    message(STATUS "Found Mt-KaHyPar: ${MTKAHYPAR_LIBRARY}")
else()
    message(FATAL_ERROR "Mt-KaHyPar library not found in expected locations (lib or lib64) under extern/local/mt-kahypar.")
endif()

# -------------------- Includes & Sources --------------------
include_directories(${PROJECT_SOURCE_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/lib)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
include_directories(${KAHIP_INCLUDE_DIR})
include_directories(${PROJECT_SOURCE_DIR}/extern/local/mt-kahypar/include)

file(GLOB_RECURSE SHAREDMAP_SOURCES CONFIGURE_DEPENDS "src/*.cpp")
file(GLOB_RECURSE SHAREDMAP_HEADERS CONFIGURE_DEPENDS "src/*.h")

# -------------------- Executable --------------------
add_executable(SharedMap main.cpp ${SHAREDMAP_SOURCES} ${SHAREDMAP_HEADERS})

# Use the variables from FindTBB instead of non-existent imported targets
target_include_directories(SharedMap PRIVATE ${TBB_INCLUDE_DIRS})

target_link_libraries(SharedMap
        PRIVATE
        ${KAHIP_LIBRARY}
        ${MTKAHYPAR_LIBRARY}
        ${TBB_LIBRARIES}
        ${TBB_MALLOC_LIBRARIES}
        ${TBB_MALLOC_PROXY_LIBRARIES}
        "-Wl,--disable-new-dtags"   # without this TBB won't link properly in some setups
)

# -------------------- Shared Library --------------------
add_library(libsharedmap OBJECT ${SHAREDMAP_SOURCES} ${SHAREDMAP_HEADERS})
add_library(sharedmap SHARED lib/libsharedmap.cpp $<TARGET_OBJECTS:libsharedmap>)

target_include_directories(sharedmap
        PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/interface
        ${TBB_INCLUDE_DIRS}
        ${KAHIP_INCLUDE_DIR}
        ${PROJECT_SOURCE_DIR}/extern/local/mt-kahypar/include
)

target_link_libraries(sharedmap
        PUBLIC
        ${KAHIP_LIBRARY}
        ${MTKAHYPAR_LIBRARY}
        ${TBB_LIBRARIES}
        ${TBB_MALLOC_LIBRARIES}
        ${TBB_MALLOC_PROXY_LIBRARIES}
        "-Wl,--disable-new-dtags"
)

install(TARGETS sharedmap DESTINATION lib)
