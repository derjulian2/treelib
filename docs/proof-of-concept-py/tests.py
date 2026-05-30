
from examples.unary_in_node import unary_in_node, unary_in_how as uhw
from examples.inheritance_diagram import inheritance_diagram_node, diagram_out_how as iohw, diagram_in_how as iihw
from examples.rose_tree import rose_out_node, rose_full_node, rose_out_how as rhw
from examples.binary_tree import binary_out_node, binary_full_node, binary_out_how as bhw


from trees import root_tree, leaf_tree, full_tree, dfs, reverse_dfs



def inhertiance_diagram_tests() -> None:
    """
    constructs an example inheritance diagram
    encoding the following diamond-class-hierarchy:

          foo      fiz
         /   \    /
        /     \  /
      bar      baz
        \     /
         \   /
          faz

    """
    tree = full_tree(inheritance_diagram_node)

    foo = tree.insert("foo")
    bar = tree.insert("bar", foo, iohw.derived_class)
    baz = tree.insert("baz", foo, iohw.derived_class)
    fiz = tree.insert("fiz", baz, iihw.base_class)
    faz = tree.insert("faz", [ bar, baz ], iohw.derived_class)

    faz_direct_bases = list(map(lambda x: x.value, faz.parents()))
    faz_all_bases    = set(map(lambda x: x.value, reverse_dfs(faz)))

    print(f"direct bases of faz: {faz_direct_bases}")
    print(f"all bases of faz: {faz_all_bases}")



def rose_out_tree_tests() -> None:
    """
    constructs an example rose-tree
    resembling a project-directory:

        ("MyProject")
        |-("docs")
        | \-("readme")
        |-("src")
        | |-("main.cpp")
        | \-("node.cpp")
        \-("build")
          \-("out.exe")

    """

    tree = root_tree(rose_out_node)

    root = tree.insert("MyProject")
    docs = tree.insert("docs", root, rhw.first_child)
    readme = tree.insert("readme", docs, rhw.first_child)

    src  = tree.insert("src", docs, rhw.next_sibling)
    main = tree.insert("main.cpp", src, rhw.first_child)
    node = tree.insert("node.cpp", main, rhw.next_sibling)

    build = tree.insert("build", src, rhw.next_sibling)
    out   = tree.insert("out.exe", build, rhw.first_child)

    print(list(map(lambda x: x.value, dfs(root))))


def unary_in_tree_test() -> None:
    """
    constructs an example in-tree:

            ( 1 )
           /     \
          /       \
        (-2)      (-3)
       /              \
     (1)              (7)

    with leaves 1 and 7.
    """
    tree = leaf_tree(unary_in_node)

    one   = tree.insert(1)
    seven = tree.insert(7)

    l = tree.insert(-2, one, uhw.parent)
    r = tree.insert(-3, seven, uhw.parent)

    root = tree.insert(1, [ l, r ], uhw.parent)

    print(list(map(lambda x: x.value, reverse_dfs(one))))
    print(list(map(lambda x: x.value, reverse_dfs(seven))))


def binary_out_tree_test() -> None:
    """
    constructs an example binary-out-tree:

            ( 4 )
           /     \
          /       \
        (2)       (6)
       /   \     /   \
     (1)   (3) (5)   (7)

    """
    tree = root_tree(binary_out_node)

    root = tree.insert(4)
    
    l    = tree.insert(2, root, bhw.left)
    ll   = tree.insert(1, l,    bhw.left)
    lr   = tree.insert(3, l,    bhw.right)

    r    = tree.insert(6, root, bhw.right)
    rl   = tree.insert(5, r,    bhw.left)
    rr   = tree.insert(7, r,    bhw.right)

    print(list(map(lambda x: x.value, dfs(root))))


def binary_full_tree_test() -> None:
    """
    constructs an example binary-full-tree:

            ( 4 )
           /     \ 
          /       \
        (2)       (6)
       /   \     /   \
     (1)   (3) (5)    \
                      (-1)
                         \
                         (7)

    """
    tree = full_tree(binary_full_node)

    root = tree.insert(4)

    l    = tree.insert(2, root, bhw.left)
    ll   = tree.insert(1, l,    bhw.left)
    lr   = tree.insert(3, l,    bhw.right)

    r    = tree.insert(6, root, bhw.right)
    rl   = tree.insert(5, r,    bhw.left)
    
    print(f"leaves: {list(map(lambda x: x.value, tree.leaves))}")
    rr   = tree.insert(7, r,    bhw.right)

    print(list(map(lambda x: x.value, dfs(root))))
    print(f"leaves after 7: {list(map(lambda x: x.value, tree.leaves))}")

    # shifting via insertion as parent
    new_rr = tree.insert(-1, rr, uhw.parent)

    print(list(map(lambda x: x.value, dfs(root))))
    print(f"leaves after shift: {list(map(lambda x: x.value, tree.leaves))}")



inhertiance_diagram_tests()
rose_out_tree_tests()
unary_in_tree_test()
binary_out_tree_test()
binary_full_tree_test()