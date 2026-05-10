
#ifndef _TREELIB_NODE_BASE_HPP_
#define _TREELIB_NODE_BASE_HPP_

/**
 * 
 * @brief defines the abstraction-layer-interface
 *        that any type that should be used as
 *        a node in a tree should implement.
 * @details
 *
 *
 */

#include <memory>
#include <concepts>

namespace tl
{
    /*
    * empty base-class that all node-types
    * should inherit from. this type
    * basically only exists to address actual
    * value-holding nodes with the same type
    * as the valueless header-node. 
    *
    * this results in some static-casting later,
    * but it allows for the header to basically only
    * be one pointer to the root-node of the tree and
    * still act as a sentinel-node for the range.
    */
    struct weak_node_base
    { };

    struct node_base
    { };

    /**
     * interfaces throught which the
     * structural information of each
     * node-type is accessed.
     */

    template <typename NodeType>
    struct node_traits;

    template <typename NodeType>
    struct weak_node_traits;

    /*
    * defines the shared node-interface that
    * a type that should be used as a node 
    * in a tree should implement.
    *
    * 1.) weak-nodes:
    * these kinds of nodes hold some sort of reference
    * (usually pointers) to other nodes in the tree, but
    * only to the next node, not backwards.
    * this allows for a smaller memory-footprint but makes
    * some tradeoffs in traversability, as not every
    * node is reachable from everywhere.
    *
    * 2.) strong-nodes:
    * basically the opposite of weak-nodes, allowing
    * full traversability from anywhere in the tree.
    *
    */

    template <typename T>
    concept is_weak_node = 
        std::derived_from<T, weak_node_base> 
        && std::default_initializable<T>
        && requires (T t)
        {
            // { weak_node_traits<T>::hooks() } -> std::integral;
            true;
        };


    template <typename T>
    concept is_node = 
        std::derived_from<T, node_base>
        && std::default_initializable<T>
        && requires (T& tr, const T& ctr)
        {
            // { node_traits<T>::parent(tr)  } -> std::convertible_to<T&>;
            // { node_traits<T>::parent(ctr) } -> std::convertible_to<const T&>;
            // { node_traits<T>::depth(tr)   } -> std::unsigned_integral;
            true;
        };


    /*
    * extends the passed node-type
    * by an instance of value-type and
    * makes the value accessible by reference.
    */
    template <typename ValueType,
              typename NodeType>
        requires is_weak_node<NodeType> || is_node<NodeType>
    class value_node 
        : public NodeType
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