
#include <treelib/detail/utility.hpp>

#include <concepts>
#include <range/v3/view/any_view.hpp>
#include <range/v3/view.hpp>

namespace tl
{
    // #ifdef _TREELIB_USE_STD_RANGES_
    //     namespace ranges = std::ranges; 
    // #else
    //     namespace ranges = ranges;
    // #endif

    template <typename NodeType>
    struct node_traits { };

    /**
     * a node consists of a value (attached via tl::value_node)
     * and some sort of connection to another node, called
     * a hook.
     * how this connection is made is implementation-defined 
     * by each node-type (the default-trees use mostly pointers).
     *
     * these connections are distinguished into two categories:
     * - in-hooks:
     *   connections that lead towards the root of the tree.
     * - out-hooks:
     *   connections that lead towards the leaves of the tree.
     *
     * this distinction raises the question of memory-ownership:
     * which connection of two nodes is 'stronger', as in that
     * if the connection is severed, one of the two nodes should
     * stay while the other and all of it's descendants should be removed.
     *
     * this is handled by each tree-type in of itself, as the
     * use case changes semantics (in-trees should probably keep
     * nodes alive that are connected via in-hooks, not the other
     * way round), but appropriate methods to implement
     * recursive erasure are provided for 
     * node-types satisfying the in/out-node-concepts.
     * 
     * node-types with both in- and out-hooks are called strong-nodes,
     * while nodes with only either one are called weak-nodes.
     * naturally, weak-nodes are a subset of strong-nodes.
     *
     * the trees built by strong-nodes are connected in such a
     * way that from any node of the tree, there is always
     * a sequence of references that one can traverse to get
     * to any other node in the tree, making them iteratively-traversable.
     *
     */

    template <typename T>
    concept in_node = requires (T &a, T &b) 
    { 
        typename node_traits<T>::in_hook_type;
        { node_traits<T>::template has_hook<std::declval<node_traits<T>::in_hook_type>()>(a) } -> std::convertible_to<bool>;
        { node_traits<T>::template hook<std::declval<node_traits<T>::in_hook_type>()>(a) };
        { node_traits<T>::template hook_as<std::declval<node_traits<T>::in_hook_type>()>(a, b) };
        { node_traits<T>::for_in_hooks([](auto i) { }) };
        { node_traits<T>::parents() } -> ranges::range;
    };

    template <typename T>
    concept out_node = requires (T &a, T &b) 
    { 
        typename node_traits<T>::out_hook_type;
        { 
            node_traits<T>::for_in_hooks([&a](auto i) 
            { node_traits<T>::template has_hook<i>(std::declval<T&>()); })
        };
        { node_traits<T>::template has_hook <std::declval<node_traits<T>::out_hook_type>()>(a) } -> std::convertible_to<bool>;
        { node_traits<T>::template hook     <std::declval<node_traits<T>::out_hook_type>()>(a) };
        { node_traits<T>::template hook_as  <std::declval<node_traits<T>::out_hook_type>()>(a, b) };
        { node_traits<T>::children() } -> ranges::range;
    };

    template <typename T>
    concept weak_node = in_node<T> || out_node<T>;

    template <typename T>
    concept strong_node = in_node<T> && out_node<T>; 

}

namespace tl
{
    template <size K>
    struct weak_k_tree_node 
    {
        std::array<weak_k_tree_node*, K> children { nullptr };
    };

    template <size K>
    struct k_tree_node 
    {
        k_tree_node                 *parent   { nullptr };
        std::array<k_tree_node*, K>  children { nullptr };
    };

    template <size K>
    struct node_traits<weak_k_tree_node<K>>
    {
        using node_type = weak_k_tree_node<K>;
        using reference = node_type&;
        using pointer   = node_type*;

        using out_hook_type = size;

        template <out_hook_type H>
        static constexpr
        void
        hook_as(reference a, reference b)
        { a.children[H] = std::addressof(b); }


        static constexpr
        auto
        children(reference node)
        {
            return node.children
                   | ranges::views::filter    ([](pointer p) { return !tl::is_null(p); })
                   | ranges::views::transform (tl::dereference<node_type>);
        }


        static constexpr
        ranges::any_view<reference>
        total_children(reference node)
        {
            return ranges::views::concat(children(node),
                        children(node)
                        | ranges::views::transform (total_children)
                        | ranges::views::join
                   );
        }

    };

    template <size K>
    struct node_traits<k_tree_node<K>>
    {
        using node_type = k_tree_node<K>;
        using reference = node_type&;
        using pointer   = node_type*;

        using out_hook_type = size;
        using in_hook_type  = enum { parent };

        template <out_hook_type H>
        static constexpr
        void
        hook_as(reference a, reference b)
        { a.children[H] = std::addressof(b); }

        template <in_hook_type H>
        static constexpr
        void
        hook_as(reference a, reference b)
        { b.parent = std::addressof(a); }
    };

    template <typename node_type>
    node_type* structural_copy(node_type*) {
        using traits = node_traits<node_type>;
        
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
        if constexpr (in_node<node_type>) {

        }
        if constexpr (out_node<node_type>) {

        }
    }
    
}


int main(int argc, char** argv) {
    using weak_node = tl::weak_k_tree_node<2>;


    return 0;
}