# treelib - tree data-structures for C++

- Version: 0.0.3
- Tested on C++-Compilers: MinGW
- Tested on Python-Versions: n/a
- Documentation: n/a

# Installation

to use treelib, you basically only need the desired header-files contained in `include/treelib`
and include them into your project. 

alternatively, you can also clone the entire repo and add it as a CMake-package:

`> git clone https://github.com/derjulian2/treelib.git`

in your project's `CMakeLists.txt`, you would add something like:
```cmake
set(treelib_DIR "path/to/where/you/cloned/it/to")
find_package(treelib CONFIG REQUIRED)
if (treelib_FOUND)
    include_directories(${treelib_INCLUDE_DIR})
endif()
```

# Features

- `trl::flex_tree` class-template: an STL-like 'arbitrary'-ary tree data-structure:
   a tree where each node can hold an arbitrary amount of child-nodes. 
   (sometiems referred to as an `n`-ary tree, but i found this name a bit misleading as `n`-ary sound like each node can hold exactly or at most `n` child-nodes)

## Tree-Construction

`trl::flex_tree`s can be constructed using the appropriate `.append()` or `.insert()` methods, or using a braced-initializer-list:

```cpp
/* 
 * construction of:
 * <root>
 * L'a'
 *  L'b'
 *  L'c'
 */
trl::flex_tree<char> char_tree;
auto a = char_tree.append(char_tree.begin(), 'a');
auto b = char_tree.append(a, 'b');
auto c = char_tree.append(a, 'c');

trl::flex_tree<char> char_tree_from_ilist
{
{ /* mind the additional braces to call the correct constructor */
    'a',
    {
        'b',
        'c'
    }
}
}
```

construction from a `std::initializer-list` should still be quite performant, as the tree basically gets constructed from the inside-out.
known limitations are:

> [!NOTE]
> construction using an initializer-list does not support custom allocator-instances, as the initializers are nested and will be called 
> recursively. this makes it hard to use the allocator within the tree, as it might not be constructed at the point where the inner 
> nodes are allocated and built.

> [!NOTE]
> when using the compile-option `TRL_FLEX_TREE_FAST_DEPTH`, construction from an initializer-list requires one full iteration of the tree at the 
> end of construction to ensure proper depth-value bookkeeping. when constructing from the inside-out, the depth-values cannot be inferred 
> inside of the individual node-constructors.

## Tree-Iteration

`trl::flex_tree<>::iterator` is a class-template, where the template-parameter `Traversal` determines the traversal-algorithm that is used
when `operator++` or `operator--` is called. the default algorithm used is __depth_first_pre_order__, but you can always override this
by defining the __TRL_FLEX_TREE_DEFAULT_TRAVERSAL__ before including `flex_tree.hpp`.

for further information on the actual traversal-algorithms see [Tree traversal](https://en.wikipedia.org/wiki/Tree_traversal).

because `::iterator` is templated around the `Traversal`-algorithm, there could be problems with certain algorithms that
expect `::iterator` to be a typename, not a template with a default-argument, and will likely not instantiate it with `::iterator<>`.
if there are any problems with `::iterator` being a template, you probably will have to `typedef` the iterator you want to use in
some wrapper around `trl::flex_tree`, something like this:

```cpp
struct flex_tree_breadth_first : public trl::flex_tree<my_type>
{ using iterator = trl::flex_tree<my_type>::iterator<breadth_first_in_order>; };
```

supported traversal-algorithms:
- depth-first-pre-order: `trl::flex_tree<>::iterator<depth_first_pre_order>`.
- breadth-first-in-order: `trl::flex_tree<>::iterator<breadth_first_in_order>`.
- breadth-first-reverse-order: `trl::flex_tree<>::iterator<breadth_first_reverse_order>`.

breadth-first in- and reverse-order determine if the algorithm starts from left-to-right or the other way around.

### Reverse-Iteration

the `trl::flex_tree` template defines a corresponding `reverse_iterator` and `const_reverse_iterator` template. these are
essentially wrappers around their underlying iterators with `operator++` and `operator--` swapped to act as a reverse-iterator.

when implementing these i found that `std::reverse_iterator` internally calculates `*(iter - 1)` on dereferencing the reverse_iterator, which for this data-structure means a potentially costly iteration-algorithm call on every dereference. for this reason `trl::flex_tree` uses a custom `reverse_iterator`-adaptor, that avoids this.
if you require full STL-compliance and `std::reverse_iterator()` calls on any flex-tree iterators are required to work as expected by the STL, use `#define TRL_FLEX_TREE_STL_REVERSE_ITER` and the code will be adjusted to use `std::reverse_iterator`. as explained, this comes with additional invokations of traversal-algorithm-calls on dereferencing the iterators.

## Tree-Operations

`trl::flex_tree`s support various operations that change their structure:

__append__/__prepend__ and __insert__ nodes using:
- `.append()`/`.prepend()` and `.emplace_append()`/`.emplace_prepend()` to add child-nodes to an individual node.
- `.insert_before()`/`.insert_after()` and `.emplace_before()`/`.emplace_after()` to add child-nodes inbetween nodes.

__concatenating__ trees:
- `.subtree()` to create a copy of the tree at a specific location in the tree.
- `.concatenate_append()`/`.concatenate_prepend()` to append a copy of a node with all it's child-nodes at a specific location.
- `.concatenate_before()`/`.concatenate_after()` to insert a copy of a node with all it's child-nodes at a specific location.

__splicing__ trees:
- `.splice_append()`/`.splice_prepend()` to move a node with all it's child-nodes to another location in the same or another tree.
- `.splice_before()`/`.splice_after()` to move a node with all it's child-nodes to another location in the same or another tree.

__erasing__ nodes and __clearing__ trees:
- `.erase()` to erase a node in the tree.
- `.clear()` to erase all nodes in a tree.

# Example

```cpp
    using namespace trl;

    flex_tree<int> ftr
    {
        1,
        2,
        4920,
    {
        6942,
        {
            6943,
            6944,
            6945
        }
    }
    };

    std::cout << "depth-first iteration:\n";
    for (flex_tree<int>::iterator i = ftr.begin(); i != ftr.end(); ++i)
    {
        std::cout << std::string(node_traits::depth(i), '=') << ' ' << *i << '\n';
    }

    std::cout << "breadth-first iteration:\n";
    for (flex_tree<int>::iterator<breadth_first_in_order> i = ftr.begin(); i != ftr.end(); ++i)
    {
        std::cout << std::string(node_traits::depth(i), '=') << ' ' << *i << '\n';
    }
    
    /* uses depth-first as the iteration-algorithm. note that .begin() and .end() are also templates! */
    flex_tree<int>::iterator i = std::find(ftr.begin(), ftr.end(), 6942);

    std::cout << "leaf iteration:\n";
    for (leaf_iterator j = node_traits::lbegin(i); j != node_traits::lend(i); ++j)
    {
        std::cout << std::string(node_traits::depth(j), '=') << ' ' << *j << '\n';
    }
```

# Data-Structure Design

the `trl::flex_tree` template is a node-based container (as e.g. `std::list`), where every value is contained within
a node that also contains metadata such as pointers to other nodes. the following diagram illustrates what pointers
each node holds and what pointer-relations nodes can have to each other:

![flex_tree_structure_diagram](flex_tree_structure.svg)

every `trl::flex_tree` has an invalid __header-node__, which does not hold an instance of `::value_type` and marks the end of the tree.
nodes are connected on every depth-layer on the tree and parents with their first and last child-nodes respectively.

note that nodes do not have to be siblings to be connected on the same-level, which allows for cleaner breadth-first iteration.
arrows going in a circle on a node indicate a `this`-pointer.

# Compile-Options:

- #define NDEBUG (should happen automatically by your compiler on release-builds):

## flex_tree.hpp

- #define TRL_FLEX_TREE_NOEXCEPT:

- #define TRL_FLEX_TREE_ITER_NOEXCEPT:

- #define TRL_FLEX_TREE_NO_RECURSION:

- #define TRL_FLEX_TREE_FAST_DEPTH:

- #define TRL_FLEX_TREE_STL_REVERSE_ITER:

## node_traits.hpp

- #define TRL_NODE_TRAITS_NOEXCEPT:

# Python-Binding

mostly for practice, i want to make a python-binding for `trl::flex_tree` using [pybind11](https://github.com/pybind/pybind11). maybe someday!

# Future-Ideas:

- `trl::n_ary_tree` class-template: optimized tree for holding exactly `n` child-nodes.
- `trl::flat_flex_tree` class-template: similiar to `trl::flex_tree` but should keep it's nodes in a contiguous sequence for fastest iteration (but slower modifications).

# Inspiration and Credits

while creating this i researched different resources and how they handled certain problems i encountered.
for this i want to thank and give credit to the sources that helped me find some general direction within all of this:

- [tree.hh](https://github.com/kpeeters/tree.hh), by kpeeters.
- [stl_list.h](https://gcc.gnu.org/onlinedocs/gcc-13.4.0/libstdc++/api/a00413_source.html), from gcc's STL `std::list` implemenation.
- [pybind11](https://github.com/pybind/pybind11), that i want to use to generate the python-binding.