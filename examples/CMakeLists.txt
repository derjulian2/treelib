cmake_minimum_required(VERSION 4.0.0)

set(CMAKE_CXX_STANDARD 23)

project(treelib_debug)

add_executable(treelib_debug 
        main.cpp 
        flex_tree_test.cpp)