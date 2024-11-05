cmake_minimum_required(VERSION 3.16)
list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules)

project(SharedMap CXX C)
set(PROJECT_VENDOR "Henning Woydt")
set(PROJECT_CONTACT "henning.woydt@informatik.uni-heidelberg.de")
set(PROJECT_URL "https://github.com/HenningWoydt/SharedMap")
set(PROJECT_DESCRIPTION "SharedMap: Parallel Shared Memory Process Mapping")
set(PROJECT_VERSION "1.0.0")
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 17)

# CMake Options
option(SHAREDMAP_ENABLE_ASSERTIONS "Enables Assertions." OFF)
# option(SHAREDMAP_DOWNLOAD_TBB "Downloads TBB." ON)

# Set Release and Debug flags
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -g3 -DNDEBUG -march=native -Wall -Wextra -pedantic")
set(CMAKE_CXX_FLAGS_DEBUG "-g3 -O0 -Wall -Wextra -pedantic")

if (SHAREDMAP_ENABLE_ASSERTIONS)
    message(STATUS "Assertions activated")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -D ASSERT_ENABLED=true ")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -D ASSERT_ENABLED=true ")
else ()
    message(STATUS "Assertions deactivated")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -D ASSERT_ENABLED=false ")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -D ASSERT_ENABLED=false ")
endif()

# Find Threads
# find_package(Threads REQUIRED)
# message(STATUS "Found Threads: ${CMAKE_THREAD_LIBS_INIT}")

# Download TBB Library
# if (SHAREDMAP_DOWNLOAD_TBB)
#     include(FetchContent)
#     FetchContent_Populate(
#             tbb
#             URL https://github.com/oneapi-src/oneTBB/releases/download/v2021.12.0/oneapi-tbb-2021.12.0-lin.tgz
#             SOURCE_DIR external_tools/tbb
#     )
#
#     set(TBB_ROOT ${CMAKE_CURRENT_BINARY_DIR}/external_tools/tbb)
# ENDIF ()
#
# # Find TBB Library
# find_package(TBB REQUIRED)
# if (TBB_FOUND)
#     if (TBB_INTERFACE_VERSION GREATER_EQUAL 12050)
#         MESSAGE(STATUS "TBB Version: ${TBB_VERSION_MAJOR}.${TBB_VERSION_MINOR} (Interface Version=${TBB_INTERFACE_VERSION})")
#         MESSAGE(STATUS "Found TBB library: inc=${TBB_INCLUDE_DIRS}, lib=${TBB_LIBRARIES}")
#         MESSAGE(STATUS "Found TBB Malloc library: inc=${TBB_MALLOC_INCLUDE_DIRS}, lib=${TBB_MALLOC_LIBRARIES}")
#         MESSAGE(STATUS "Found TBB Malloc Proxy library: inc=${TBB_MALLOC_PROXY_INCLUDE_DIRS}, lib=${TBB_MALLOC_PROXY_LIBRARIES}")
#     ELSE ()
#         MESSAGE(FATAL_ERROR "
#       Your current TBB version ${TBB_INTERFACE_VERSION} is too old. The minimum required version is 12050 (OneTBB 2021.5).
#       Add -DMTRECPROMAP_DOWNLOAD_TBB=ON to the cmake build command (may require to clear the cmake cache).")
#     ENDIF ()
# ELSE ()
#     MESSAGE(FATAL_ERROR "
#     TBB library not found. Add -DMTRECPROMAP_DOWNLOAD_TBB=ON to the cmake build command (may require to clear the cmake cache).")
# ENDIF ()

# include KaHIP library
set(NOMPI ON CACHE BOOL "For KaHIP: disable all targets that depend on MPI (kaffpaE, ParHIP)")
include_directories(extern/KaHIP)
link_directories(extern/KaHIP)
add_subdirectory(extern/KaHIP)

# Find all source and header files
include_directories(${PROJECT_SOURCE_DIR})
file(GLOB_RECURSE SHAREDMAP_SOURCES CONFIGURE_DEPENDS "src/*.cpp")
file(GLOB_RECURSE SHAREDMAP_HEADERS CONFIGURE_DEPENDS "src/*.h")

# create executable
add_executable(SharedMap main.cpp ${SHAREDMAP_SOURCES} ${SHAREDMAP_HEADERS})

# Link KaHIP library
target_link_libraries(SharedMap kahip)

# Link TBB library
# target_link_libraries(SharedMap TBB::tbb TBB::tbbmalloc_proxy TBB::tbbmalloc)

# without this TBB wont link properly
target_link_libraries(SharedMap "-Wl,--disable-new-dtags")

# Link Mt-KaHyPar library
target_include_directories(SharedMap PRIVATE ${PROJECT_SOURCE_DIR}/extern/mt_kahypar_local/include)
target_link_libraries(SharedMap ${PROJECT_SOURCE_DIR}/extern/mt_kahypar_local/lib/libmtkahypar.so)
# target_link_libraries(SharedMap ${PROJECT_SOURCE_DIR}/extern/mt_kahypar_local/lib64/libmtkahypar.so)
