
#ifndef TREELIB_NODE_HPP
#define TREELIB_NODE_HPP

/**
 * @file    treelib/detail/node.hpp
 * @author  Julian Benzel
 * @date    03.07.2026
 *
 * @brief   basic concepts and traits
 *          that every node-type should satisfy.
 *
 * @details a node consists of a value (attached via tl::value_node)
 *          and some sort of connection to another node, called
 *          a hook.
 *          how this connection is made is implementation-defined 
 *          by each node-type (the default-trees use mostly pointers).
 *         
 *          these connections are distinguished into two categories:
 *          - in-hooks:
 *            connections that lead towards the root of the tree.
 *          - out-hooks:
 *            connections that lead towards the leaves of the tree.
 *         
 *          this distinction raises the question of memory-ownership:
 *          which connection of two nodes is 'stronger', as in that
 *          if the connection is severed, one of the two nodes should
 *          stay while the other and all of it's descendants should be removed.
 *         
 *          this is handled by each tree-type in of itself, as the
 *          use case changes semantics (in-trees should probably keep
 *          nodes alive that are connected via in-hooks, not the other
 *          way round), but appropriate methods to implement
 *          recursive erasure are provided for 
 *          node-types satisfying the in/out-node-concepts.
 *          
 *          node-types with both in- and out-hooks are called strong-nodes,
 *          while nodes with only either one are called weak-nodes.
 *          naturally, weak-nodes are a subset of strong-nodes.
 *         
 *          the trees built by strong-nodes are connected in such a
 *          way that from any node of the tree, there is always
 *          a sequence of references that one can traverse to get
 *          to any other node in the tree, making them iteratively-traversable.
 */


/*
* e.g. for a binary-tree with .left, .right and .parent
* this code will unfold into:
* 
* cpy = copy(node)
* if (node.has_left):
*    left_copy = recursive_copy(node.left)
*    left_copy.hook_as<left>(cpy)
*    cpy.hook_as<parent>(left_copy)
* if (node.has_right):
*    right_copy = recursive_copy(node.right)
*    right_copy.hook_as<right>(cpy)
*    cpy.hook_as<parent>(right_copy)
*
* which will hook all child-nodes and
* parent-pointers correctly.
*/

#include <ranges>
#include <concepts>

namespace tl
{
    /**
     * @brief interface that any node with
     *        hooks pointing away from a root-node
     *        should implement.
     */
    template <typename T>
    concept out_node = requires(T& t, const T& ct)
    {
        typename T::out_hook_type;
        // { t.has_hook(std::declval<typename T::out_hook_type>()) } -> std::convertible_to<bool>;
        // { t.hook(std::declval<typename T::out_hook_type>()) };
        { t.hook_as(std::declval<typename T::out_hook_type>(), t) };
        { t.neighbours() } -> std::ranges::range;
        // { t.children() } -> std::ranges::range;
    };

    /**
     * @brief compile-time-requirement for out-nodes:
     *        if a connection between two nodes a, b is established
     *        via a.hook_as(<sth>, b) then a.neighbours() must contain b.
     */
    template <typename T>
    constexpr bool
    out_node_neighbours_req() 
    {
        using node_type = T;

        node_type x, y;

        x.hook_as();

        for (const auto& n : x.neighbours())
        {
            if (n == y)
            { }
        }
        return false;
    }

    /**
     * @brief compile-time-requirement for in-nodes:
     *        if a connection between two nodes a, b is established
     *        via a.hook_as(<sth>, b) then b.neighbours() must contain a.
     */
    template <typename T>
    constexpr bool
    in_node_neighbours_req() 
    {
        using node_type = T;

        node_type x, y;

        x.hook_as();

        for (const auto& n : x.neighbours())
        {
            if (n == y)
            { }
        }
        return false;
    }

    /**
     * @brief compile-time-requirement for full-nodes:
     *        if a connection between two nodes a, b is established
     *        via a.hook_as(<sth>, b) then a.neighbours() must contain b
     *        and b.neighbours().
     */
    template <typename T>
    constexpr bool
    full_node_neighbours_req() 
    { return out_node_neighbours_req<T>() && in_node_neighbours_req<T>(); }


    /**
     * @brief interface that any node with
     *        hooks pointing towards a root-node
     *        should implement.
     */
    template <typename T>
    concept in_node = requires(T t, const T ct)
    {
        typename T::in_hook_type;
        { t.has_hook(std::declval<typename T::in_hook_type>()) } -> std::convertible_to<bool>;
        { t.hook(std::declval<typename T::in_hook_type>()) };
        { t.hook_as(std::declval<typename T::in_hook_type>(), t) };
        { t.parents() } -> std::ranges::range;
    };


    /**
     * @brief additional named-requirements
     *        based on in/out-node.
     */

    template <typename T>
    concept node = out_node<T> || in_node<T>;

    template <typename T>
    concept weak_node = out_node<T> != in_node<T>; /* XOR */

    template <typename T>
    concept strong_node = out_node<T> && in_node<T> && requires(T& t, const T& ct)
    { { t.unhook() } -> std::same_as<void>; };

    template <typename T, typename NodeType>
    concept is_hook_type =  (out_node<NodeType> && std::convertible_to<T, typename NodeType::out_hook_type>)
                         || (in_node<NodeType> && std::convertible_to<T, typename NodeType::in_hook_type>);


    /**
     * @brief traits to make out_node<T> requirements accessible.
     */
    template <typename T>
        requires out_node<T>
    struct out_node_traits
    {
        using node_type = T;
        using pointer   = node_type*;
        using reference = node_type&;

        using out_hook_type = typename node_type::out_hook_type;


        static constexpr
        void hook_as(reference node, out_hook_type what, reference where)
        { node.hook_as(what, where); }
    };


    /**
     * @brief traits to make in_node<T> requirements accessible.
     */
    template <typename T>
        requires in_node<T>
    struct in_node_traits
    {
        using node_type = T;
        using pointer   = node_type*;
        using reference = node_type&;

        using in_hook_type = typename node_type::in_hook_type;

        
        static constexpr
        void hook_as(reference node, in_hook_type what, reference where)
        { node.hook_as(what, where); }
    };


    /**
     * @brief traits to make strong_node<T> requirements accessible.
     */
    template <typename T>
        requires strong_node<T>
    struct strong_node_traits
        : public out_node_traits<T>
        , public in_node_traits<T>
    {
        using reference = typename out_node_traits<T>::reference;

        static constexpr
        void unhook(reference node)
        { node.unhook(); }
    };


    /**
     * @brief unified interface for weak/strong-nodes,
     *        selected based on concept-requirements.
     */
    template <typename T>
    struct node_traits;


    template <typename T>
        requires (out_node<T> && weak_node<T>)
    struct node_traits<T>
        : public out_node_traits<T>
    { };


    template <typename T>
        requires (in_node<T> && weak_node<T>)
    struct node_traits<T>
        : public in_node_traits<T>
    { };


    template <typename T>
        requires strong_node<T>
    struct node_traits<T>
        : public strong_node_traits<T>
    { };


    /**
     * @brief empty base-class for node-types
     *        used inside of trees. will cause some 
     *        static-casting later on, but this
     *        basically enables the sentinel-node
     *        in strong trees to be treated as
     *        a regular node without actually
     *        having all the members of an actual
     *        node (so just to save some memory).
     */
    struct node_base { };


    /**
     * @brief extends the passed node-type
     *        by an instance of value-type 
     *        using CRTP. 
     *       
     *        these will be the actual node-types
     *        allocated by the respective tree-types.
     */
    template <typename ValueType,
              typename NodeType>
        requires node<NodeType>
    class value_node
        : public NodeType
        , public node_base
    {
    public:

        using value_type = ValueType;
        using node_type  = NodeType;

    private:

        value_type m_value;

    public:

        template <typename... Args>
        value_node(Args&&... args)
            : m_value(std::forward<Args>(args)...)
        { }
        

        [[nodiscard]]
        constexpr
        value_type&
        value() noexcept
        { return m_value; }


        [[nodiscard]]
        constexpr
        const value_type&
        value() const noexcept
        { return m_value; }

    };

    
}

#endif