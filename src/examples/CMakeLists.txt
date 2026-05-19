
set(treelib_TESTS_SOURCES
    main.cpp)

add_executable(treelib_tests 
    ${treelib_TESTS_SOURCES})

target_include_directories(treelib_tests 
    PUBLIC ${treelib_INCLUDE_DIR}
           ${rangev3_INCLUDE_DIR})