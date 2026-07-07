
#ifndef TREELIB_STRATEGY_HPP
#define TREELIB_STRATEGY_HPP

/**
 * @file   treelib/detail/traversal/strategy.hpp
 * @author Julian Benzel
 * @date   07.07.2026
 *
 * @brief  common types and requirements
 *         for types that define strategies
 *         for tree-traversal.
 */

#include <utility>
#include <concepts>

namespace tl
{
    enum struct traversal_strategy
    {
        depth_first_pre_order,
        depth_first_post_order,
        breadth_first_in_order,
        breadth_first_reverse_order
    };


    /**
     * @brief traversal-type to be used in tl::queued_iterator
     *        and tl::iterator when the iteration-strategy
     *        is dependent on a runtime-condition.
     *
     *        dispatches the .next()-calls based on the
     *        value of a 'strategy'-member-field.
     */
    template <typename NodeType>
    struct dynamic_traversal
    {
        traversal_strategy m_strategy;
    };


    /**
     * @brief requirements for types that provide the 
     *        traversing-algorithms to be used in queued-iterators.
     */
    template <typename T>
    concept queued_traversal_provider = requires ()
    {
        typename T::node_type;
        { T::enqueue(std::declval<typename T::node_type&>()) } 
        -> std::convertible_to<typename T::node_pointer>;
    };


    /**
    * @brief requirements for types that provide the 
     *       traversing-algorithms to be used in traversing-iterators.
     */
    template <typename T>
    concept iterative_traversal_provider = requires ()
    {
        typename T::node_type;
        { T::next(std::declval<typename T::node_type&>()) } 
        -> std::convertible_to<typename T::node_pointer>;
        { T::prev(std::declval<typename T::node_type&>()) } 
        -> std::convertible_to<typename T::node_pointer>;
    };

}

#endif