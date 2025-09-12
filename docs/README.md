# treelib - tree data-structures for C++

- Version: 0.0.1
- Tested on C++-Compilers: MinGW
- Tested on Python-Versions:
- Documentation:

## Features

- `trl::flex_tree` class-template: an STL-like 'arbitrary'-ary tree data-structure:
   a tree where each node can hold an arbitrary amount of child-nodes.

### Tree-Construction

`trl::flex_trees` can be constructed using the appropriate `.append()` or `.insert()` methods, or using a braced-initializer-list.
construction from a `std::initializer-list` should still be quite performant, as the tree basically gets constructed from the inside-out.
this, however, also has it's limitations:

[!NOTE]
construction using an initializer-list does not support custom allocator-instances, as the initializers are nested and will be called recursively. this effect makes it hard to use the allocator within the tree, as it might not be constructed at the point where the inner nodes are allocated and built.

[!NOTE]
when using the compile-option FLEX_TREE_FAST_DEPTH, construction from an initializer-list requires one full iteration of the tree at the end of construction to ensure proper depth-value bookkeeping. when constructing from the inside-out, the depth-values cannot be inferred inside of the individual node-constructors.

### Tree-Iteration

`trl::flex_tree<>::iterator` is a class-template, where the template-parameter `Traversal` determines the traversal-algorithm that is used
when `operator++` or `operator--` is called. the default algorithm used is __depth_first_post_order__.

for further information see [Tree Traversal](https://en.wikipedia.org/wiki/Tree_traversal).

if there are any problems with `::iterator` being a template, you probably will have to `typedef` the iterator you want to use in
some wrapper around `trl::flex_tree`.

supported traversal-algorithms:
- depth-first-post-order: `trl::flex_tree<>::iterator<depth_first_post_order>`.
- depth-first-post-order: `trl::flex_tree<>::iterator<depth_first_pre_order>`.
- breadth-first-in-order: `trl::flex_tree<>::iterator<breadth_first_in_order>`.
- breadth-first-reverse-order: `trl::flex_tree<>::iterator<breadth_first_reverse_order>`.

breadth-first in- and reverse-order determine if the algorithm starts from left-to-right or the other way around.

### Tree-Operations

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

__erasing__ nodes / __clearing__ trees:
- `.erase()` to erase a node in the tree.
- `.clear()` to erase all nodes in a tree.

## Examples

[!NOTE]
the flex_tree container is built similiarly to a standard-library container. compile-time errors such as passing an iterator from one tree into another or dereferencing a past-the-end-iterator are not caught or handled (or only via asserts if the macro NDEBUG is not defined) and must be prevented by the user.

## Data-Structure Design

the `trl::flex_tree` template is a <ins>node-based</ins> container (as e.g. `std::list`), where every value is contained within
a node that also contains metadata such as pointers to other nodes. the following diagram illustrates what pointers
each node holds and what pointer-relations nodes can have to each other:

![flex_tree_diagram](flex_tree_diagram.svg)

arrows pointing to nothing indicate a __nullptr__. nodes that are on the same level are always connected to each other as
if they were in a doubly-linked-list (without the ends tied to a ring), even if they are not of the same parent.

## Python-Binding

mostly as an exercise, there will be a python-binding for this data-structure created using [pybind11](https://github.com/pybind/pybind11)

### Examples

## Future-Ideas:

- `trl::n_ary_tree` class-template: optimized tree for holding exactly `n` child-nodes.
- `trl::array_tree` class-template: similiar to `trl::flex_tree` but should keep it's nodes in a contigous sequence for fastest iteration (but slower modifications).

## Inspiration and Credits

while creating this i researched different resources and how they handled certain problems i encountered.
for this i want to thank and give credit to the sources that helped me find some general direction within all of this:

- [tree.hh](https://github.com/kpeeters/tree.hh), by kpeeters.
- [stl_list.h](https://gcc.gnu.org/onlinedocs/gcc-13.4.0/libstdc++/api/a00413_source.html), from gcc's STL std::list implemenation.