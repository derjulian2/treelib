message("building treelib unit-tests and benchmarks")

set(treelib_UNIT_TEST_SOURCES
    flex_tree_unit_test.cpp)

add_executable(treelib_unit_tests ${treelib_UNIT_TEST_SOURCES})

set(treelib_BENCHMARK_SOURCES
    flex_tree_benchmark.cpp)

add_executable(treelib_benchmarks ${treelib_BENCHMARK_SOURCES})