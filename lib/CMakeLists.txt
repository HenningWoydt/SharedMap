cmake_minimum_required(VERSION 3.10)
project(SharedMapLibrary)

# Set the library name
set(LIBRARY_NAME sharedmap)

# Include header files
include_directories(${CMAKE_SOURCE_DIR}/include)

# Collect all source files
file(GLOB LIB_SOURCES ${CMAKE_SOURCE_DIR}/lib/*.cpp)
file(GLOB SRC_SOURCES ${CMAKE_SOURCE_DIR}/src/*.cpp)

# Create the shared library
add_library(${LIBRARY_NAME} SHARED ${LIB_SOURCES} ${SRC_SOURCES})

# Specify output directory for the library
set_target_properties(${LIBRARY_NAME} PROPERTIES
        LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib
)

# Install rules (optional)
install(TARGETS ${LIBRARY_NAME} LIBRARY DESTINATION lib)
install(FILES ${CMAKE_SOURCE_DIR}/include/SharedMap.h DESTINATION include)
