
# Throw a fatal error if version is < 3.6.
cmake_minimum_required (VERSION 3.6 FATAL_ERROR)

# Variables used to represent the version.
set (DATASTRUCT_PROJECT_NAME C-DataStructures)
set (DATASTRUCT_VERSION_MAJOR 5)
set (DATASTRUCT_VERSION_MINOR 0)
set (DATASTRUCT_VERSION_PATCH 0)
set (DATASTRUCT_VERSION_STR "${DATASTRUCT_VERSION_MAJOR}.${DATASTRUCT_VERSION_MINOR}.${DATASTRUCT_VERSION_PATCH}")

# Build type.
set (CMAKE_BUILD_TYPE Release)

# Set the project source dir (just convention)
set (DATASTRUCT_SOURCE_DIR src)
set (DATASTRUCT_INCLUDE_DIR include)
set (DATASTRUCT_TOOLS_DIR ${DATASTRUCT_SOURCE_DIR}/tools)

# Source files for compiling.
set (DATASTRUCT_SOURCES
        ${DATASTRUCT_SOURCE_DIR}/Dictionary.c
        ${DATASTRUCT_SOURCE_DIR}/HashTable.c
        ${DATASTRUCT_SOURCE_DIR}/LinkedList.c
        ${DATASTRUCT_SOURCE_DIR}/Vector.c

        ${DATASTRUCT_TOOLS_DIR}/IO.c
        ${DATASTRUCT_TOOLS_DIR}/Math.c
        ${DATASTRUCT_TOOLS_DIR}/Memory.c
        ${DATASTRUCT_TOOLS_DIR}/Stopwatch.c
        ${DATASTRUCT_TOOLS_DIR}/Synchronize.c)

# Send the variables (version number) to source code header
set (DATASTRUCT_CONFIG_FILE Config.h)
configure_file (
        "${DATASTRUCT_CONFIG_FILE}.in"
        "${DATASTRUCT_INCLUDE_DIR}/${DATASTRUCT_CONFIG_FILE}")

# Project setup.
project(${DATASTRUCT_PROJECT_NAME} C)
# Add the library to be linked.
add_library(${DATASTRUCT_PROJECT_NAME} STATIC ${DATASTRUCT_SOURCES})
target_link_libraries(${DATASTRUCT_PROJECT_NAME} C-Random)
