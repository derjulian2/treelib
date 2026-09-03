
#include <treelib/binary>

#include <iostream>
#include <string>

namespace tl
{
    int binary_tree_tests()
    {
        using tree_type = binary_tree<std::string>;
        
        tree_type my_tree;
    }
}


int main(int argc, char** argv) 
{   
    tl::binary_tree_tests();
    return 0;
}