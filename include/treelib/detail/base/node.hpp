
#ifndef TREELIB_NODE_HPP
#define TREELIB_NODE_HPP

/**
 * @file    treelib/detail/base/node.hpp
 * @author  Julian Benzel
 * @date    07.07.2026
 *
 * @brief   requirements and traits
 *          for node-types used in trees.
 *
 * @details a node consists of a value (attached via tl::value_node)
 *          and some sort of connection to another node.
 *          
 *          a root-node is a node n without any connection
 *          n <- m by any other node m.
 *
 *          a leaf-node is a node without any connection
 *          n -> m to any other node m.
 *
 *          there are 4 different aspects to a connection between
 *          two nodes of the same node-type:
 *
 *          1.) the 'low-level' implementation of said connection
 *              (in the default-trees, mostly raw-pointers).
 *
 *          2.) the 'semantics' of a connection. trees are not very 
 *              straightforward data-structures. naturally there are often
 *              different ways in which it can make sense to insert
 *              a node into a tree (e.g. for binary trees a node can be
 *              inserted left or right).
 *              this can be implemented using additional arguments
 *              to tl::node_traits::hook_as<...>(n, ...).
 *
 *          3.) the 'kind' of connection that a function creates. this
 *              implementation distinguishes functions that create
 *              connections between nodes into 3 different categories:
 *              - out-connections:
 *                when f(n, m) creates a connection n -> m
 *              - in-connections
 *                when f(n, m) creates a connection n <- m
 *              - mutual-connections
 *                when f(n, m) creates a connection n <-> m
 *              what kind of a connection a function creates is important
 *              to classify trees into weak/strong to argue about their
 *              properties of traversability and erasability.
 *
 *          4.) the 'direction' of a connection, which is either:
 *              - outwards:
 *                when n -> m faces 'away' from a root-node. 
 *              - inwards:
 *                when n -> m points 'away' from a leaf-node. 
 *
 *          this distinction also raises the question of memory-ownership:
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
 *          node-types where all connections are mutual are called strong-nodes,
 *          while nodes with only either one are called weak-nodes.
 *          naturally, weak-nodes are a subset of strong-nodes.
 *         
 *          the trees built by strong-nodes are connected in such a
 *          way that from any node of the tree, there is always
 *          a sequence of references that one can traverse to get
 *          to any other node in the tree, making them iteratively-traversable.
 *
 *
 *          for example:
 *          consider a rose-tree-node that stores all of it's child-nodes
 *          as a 'std::vector<node*> m_children'. it can make sense to define
 *          the following enum to encode the possibilities of insertion:
 *
 *          enum struct rose_insert { as_first_child, as_last_child };
 *
 *          and a 'hook'-function as follows:
 *
 *          template <tl::edge_kind E>
 *              requires (E == tl::edge_kind::out)
 *          void hook_as(n, m, rose_insert how)
 *          { ... }
 *
 *          additionally, you can leverage std::vector to provide
 *          a mechanism to insert a node as the ith child via
 *          another overload:
 *
 *          template <tl::edge_kind E>
 *              requires (E == tl::edge_kind::out)
 *          void hook_as(n, m, std::size_t i)
 *          { n.m_children.insert(i, m); }
 *
 *          this construct enables specification of how a node should
 *          be inserted at runtime (or maybe compile-time if the compiler
 *          can inline .hook_as(n, m, rose_insert::first_child) if how is 
 *          a compile-time-constant), while preserving the fact that .hook_as(...) 
 *          will always only produce out-edges between nodes n and m, which will 
 *          be useful in reasoning about the possible structures a tree can have and
 *          which traversability-strategies may or may not apply to it.
 *          
 *          this however relies on the programmer to correctly identify that
 *          his .hook_as(...) methods produce the exact edge-types
 *          that it says it produces via the tl::edge_kind enum. otherwise
 *          there will likely be some error in the traversing-algorithms later
 *          (i didn't actually get to that yet, so we see how this actually plays out).
 */

#include <ranges>
#include <concepts>

namespace tl
{
    enum struct edge_kind
    { out, in, mutual };

    enum struct edge_direction
    { outwards, inwards };

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