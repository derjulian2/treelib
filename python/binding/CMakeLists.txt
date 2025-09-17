message("building treelib python-binding")

# change hardcoded paths to finding python and pybind11 directories
set(CMAKE_PREFIX_PATH "C:/Users/Julia/AppData/Local/Programs/Python/Python313/Lib/site-packages/pybind11")
# statically include source-code into the binary for now, maybe make link against a dll optional?
# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -static")

set(PYBIND11_FINDPYTHON ON)
find_package(pybind11 CONFIG REQUIRED)

pybind11_add_module(treelib flex_tree.cpp)

include_directories("C:/Users/Julia/AppData/Local/Programs/Python/Python313/Lib/site-packages/pybind11/include")
include_directories("C:/Users/Julia/AppData/Local/Programs/Python/Python313/include")

# target_link_libraries(treelib_python_binding "C:/Users/Julia/AppData/Local/Programs/Python/Python313/libs/python3.lib")