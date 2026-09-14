
# treelib - STL-like tree-data-structures for C++

- Version: 0.1.1
- Tested on C++-Compilers: GCC 15.2.1-1 on Fedora-Linux 42
- Documentation: n/a (doxygon-comments in-code)

# Installation

because the code consists mostly of templates, treelib is a header-only library,
which means you only need to include the header-files into your project.

## CMake

clone the repo to your desired location using the command:

`$ git clone https://github.com/derjulian2/treelib.git`

now to point CMake to the include-directory, you can add something like the 
following snippet to your build-script after you cloned the repo:

```cmake
set(treelib_DIR "path/to/where/you/cloned/it/to")
find_package(treelib CONFIG REQUIRED)
if (treelib_FOUND)
    include_directories(${treelib_INCLUDE_DIR})
endif()
```

where you set the variable `treelib_DIR` to the root of the cloned repo.

# Reference and Examples

## k-ary and binary-trees

- `tl::outward_k_ary_tree`
- `tl::k_ary_tree`

- `tl::outward_binary_tree`
- `tl::binary_tree`

# rose-trees

- `tl::outward_vecrose_tree`
- `tl::vecrose_tree`

- `tl::outward_listrose_tree`
- `tl::listrose_tree`

- `tl::rose_tree`

# avl-tree

- `tl::avl_tree`

# Compile-Options:

- `#define NDEBUG` (should happen automatically by your compiler on release-builds):
- `#define _treelib_no_exceptions`
- `#define _treelib_store_depth` 

# Inspiration and Credits

throughout this project i took inspiration and help from other
great open-source projects that i wish to credit here:

- [tree.hh](https://github.com/kpeeters/tree.hh), by kpeeters, which is an awesome STL-like
  rose-tree implementation that i often looked at for a reference-implementation.
- [libstdc++](https://github.com/gcc-mirror/gcc/tree/master/libstdc%2B%2B-v3), the GCC/GNU C++-standard-library implementation was also a big help for 
  seeing how the industry-standard implements tree-data-structures and learning from that.
  the main headers that i took inspiration from were:
    - [stl_list.h](https://github.com/gcc-mirror/gcc/blob/master/libstdc%2B%2B-v3/include/bits/stl_list.h)
    - [stl_forward_list.h](https://github.com/gcc-mirror/gcc/blob/master/libstdc%2B%2B-v3/include/bits/forward_list.h)
    - [stl_tree.h](https://github.com/gcc-mirror/gcc/blob/master/libstdc%2B%2B-v3/include/bits/stl_tree.h)

it is truly insane to me how there are so many committed and passionate people out there developing
free open-source software. thanks to all of you, i was able to learn.