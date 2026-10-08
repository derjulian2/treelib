
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
```c++
      using namespace tl;
      using tree_type = outward_binary_tree<int>;

      using namespace initializers;

      tree_type my_tree(
          root_tree(
              node(1,
              {
                  node(2,
                  {
                      node(3),
                      node(4)
                  }),
                  node(5,
                  {
                      node(6),
                      node(7)
                  })
              })
          )
      );
      tree_type my_other_tree = my_tree;

      // member-types
      tree_type::value_type;
      tree_type::pointer;
      tree_type::const_pointer;
      tree_type::reference;
      tree_type::const_reference;

      tree_type::allocator_type;
      tree_type::size_type;
      tree_type::hook_type;
      tree_type::initializer_tree_type;

      tree_type::iterator;
      tree_type::const_iterator;

      // information
      (void)my_tree.size();
      (void)my_tree.max_size();
      (void)my_tree.empty();
      (void)my_tree.get_allocator();
      
      // iterators
      my_tree.begin();
      my_tree.cbegin();
      my_tree.end();
      my_tree.cend();
      my_tree.root();
      my_tree.croot();

      // queued-iterators
      my_tree.qbegin<traversal::depth_first, queued::greedy>();
      my_tree.cqbegin<traversal::breadth_first, queued::lazy>();
      my_tree.qend<traversal::depth_first_post_order, queued::greedy>();
      my_tree.cqend<traversal::depth_first, queued::lazy>();
      my_tree.qroot<traversal::depth_first_in_order>();
      my_tree.cqroot<traversal::depth_first>();

      // modifiers
      my_tree.emplace_root(2);
      my_tree.insert_root(3);

      my_tree.emplace(binary::left, my_tree.root(), 4);
      my_tree.insert(binary::left, my_tree.root(), 4);
      my_tree.insert_subtree(binary::left, my_tree.root(), ++my_tree.cqbegin<traversal::depth_first>());

      my_tree.erase_at(binary::left, my_tree.root());
      my_tree.splice_to(binary::left, my_tree.root(), std::move(my_other_tree));
      my_tree.splice_to(binary::right, my_tree.root(), std::move(my_other_tree), ++my_other_tree.cbegin());
```
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
- `#define _treelib_no_exception_specs`
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
    - [stl_hashtable_policy.h](https://github.com/gcc-mirror/gcc/blob/master/libstdc%2B%2B-v3/include/bits/hashtable_policy.h) to see how handling allocators of different types in a single container can be done.
- [recursive-macros](https://www.scs.stanford.edu/~dm/blog/va-opt.html) an article on using C++20 `__VA_OPT__`
  to implement recursive/folding macros.

it is truly insane to me how there are so many committed and passionate people out there developing
free open-source software. thanks to all of you, i was able to learn something.

# AI-Usage

the concept of the node-abstraction-interface and the code is made by me.
every now and then i asked Copilot for a little feedback or on certain questions
about C++, but no code is generated by AI. this is a fun/hobby-project.