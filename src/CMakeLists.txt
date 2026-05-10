
cmake_minimum_required(VERSION 3.30.0)

set(CMAKE_CXX_STANDARD 20 REQUIRED)

project(treelib)

option(BUILD_EXAMPLES       "builds examples for treelib"       OFF)
option(BUILD_TESTS          "builds unit-tests for treelib"     OFF)
# option(BUILD_PYTHON_BINDING "builds python-binding for treelib" OFF)

cmake_path(GET CMAKE_SOURCE_DIR PARENT_PATH treelib_DIR)

set(BUILD_EXAMPLES ON)

find_package(treelib REQUIRED)

if (BUILD_EXAMPLES)
    add_subdirectory("examples")
endif()

# if (BUILD_TESTS)
#     add_subdirectory("tests")
# endif()

# if (BUILD_PYTHON_BINDING)
#     add_subdirectory("python/binding")
# endif()