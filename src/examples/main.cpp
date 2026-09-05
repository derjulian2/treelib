
#include <treelib/binary>

#include <iostream>
#include <string>

namespace tl
{
    int outward_binary_tree_tests()
    {
        using tree_type = outward_binary_tree<std::string>;
        
        std::allocator<std::string> my_alloc;

        tree_type my_tree_a("root_a", my_alloc);
        tree_type my_tree_b("root_b");
        static_assert(tl::detail::_Is_Node<tl::detail::_K_Node<2>>);
        static_assert(tl::detail::_Is_Node<tl::detail::_Parent_K_Node<2>>);

        return 0;
    }

    int binary_tree_tests()
    {
        // using tree_type = binary_tree<std::string>;
        // static_assert(tl::detail::node<tl::detail::bidirectional_k_node<2>>);

        // std::allocator<std::string> my_alloc;

        // tree_type my_tree_a("root_a", my_alloc);
        // tree_type my_tree_b("root_b");

        return 0;
    }
}


int main(int argc, char** argv) 
{   
    tl::binary_tree_tests();
    return 0;
}