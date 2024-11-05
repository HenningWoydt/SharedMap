cmake_minimum_required(VERSION 3.16)

project(SharedMap CXX C)
set(PROJECT_VENDOR "Henning Woydt")
set(PROJECT_CONTACT "henning.woydt@informatik.uni-heidelberg.de")
set(PROJECT_URL "https://github.com/HenningWoydt/SharedMap")
set(PROJECT_DESCRIPTION "SharedMap: Parallel Shared Memory Process Mapping")
set(PROJECT_VERSION "1.0.0")
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 17)

# Set Release and Debug flags
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -g -DNDEBUG -march=native -Wall -Wextra -pedantic")
set(CMAKE_CXX_FLAGS_DEBUG "-g3 -O0 -Wall -Wextra -pedantic")

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

# without this TBB wont link properly
target_link_libraries(SharedMap "-Wl,--disable-new-dtags")

# Link Mt-KaHyPar library
target_include_directories(SharedMap PRIVATE ${PROJECT_SOURCE_DIR}/extern/mt_kahypar_local/include)
target_link_libraries(SharedMap ${PROJECT_SOURCE_DIR}/extern/mt_kahypar_local/lib/libmtkahypar.so)
# target_link_libraries(SharedMap ${PROJECT_SOURCE_DIR}/extern/mt_kahypar_local/lib64/libmtkahypar.so)
