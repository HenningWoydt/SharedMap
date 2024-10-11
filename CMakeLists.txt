cmake_minimum_required(VERSION 3.29)

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

# Set Release and Debug flags
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG -fopenmp -march=native -Wall -Wextra -pedantic ")
set(CMAKE_CXX_FLAGS_DEBUG "-g3 -O0 -Wall -Wextra -pedantic -fopenmp ")

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

# Link Mt-KaHyPar library
target_include_directories(SharedMap PRIVATE ${PROJECT_SOURCE_DIR}/extern/mt_kahypar_local/include)
target_link_libraries(SharedMap ${PROJECT_SOURCE_DIR}/extern/mt_kahypar_local/lib/libmtkahypar.so)
# target_link_libraries(recpromap ${PROJECT_SOURCE_DIR}/extern/mt_kahypar_local/lib64/libmtkahypar.so)
