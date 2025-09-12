
#ifndef TRL_FLEX_TREE_TEST_HPP
#define TRL_FLEX_TREE_TEST_HPP

#include <iostream>
#include <chrono>

namespace trl
{

    namespace tests 
    {
    
        /* timer to measure time elapsed between from a { brace to the next } brace (a scope entry and exit) */
        template <typename DurT>
        struct scoped_timer 
        {
            std::chrono::time_point<std::chrono::steady_clock> begin;    
            DurT& result;

            scoped_timer(DurT& duration_result) : result(duration_result) { this->begin = std::chrono::steady_clock::now(); }
            ~scoped_timer() { result = std::chrono::duration_cast<DurT>(std::chrono::steady_clock::now() - this->begin); }
        };

        int flex_tree_test();

    }

    namespace examples 
    {

        int flex_tree_example_1();
        int flex_tree_example_2();
        int flex_tree_example_3();
    
    }

}

#endif