cmake_minimum_required(VERSION 4.0.0)

set(CMAKE_CXX_STANDARD 23)

project(treelib)

option(BUILD_EXAMPLES       "builds examples for treelib"       OFF)
option(BUILD_TESTS          "builds unit-tests for treelib"     OFF)
option(BUILD_PYTHON_BINDING "builds python-binding for treelib" OFF)

set(BUILD_TESTS ON)

if (BUILD_EXAMPLES)
    add_subdirectory("examples")
endif()

if (BUILD_TESTS)
    add_subdirectory("tests")
endif()

if (BUILD_PYTHON_BINDING)
    add_subdirectory("python/binding")
endif()
