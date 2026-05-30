
from typing import Type, Optional, Self, Iterable, Any
from abc import ABC, abstractmethod
from enum import Enum
from dataclasses import dataclass, field

from util import ranges, remove_if_present
from node import out_node, in_node, node



@dataclass
class tree_base:
    
    node_type: Type[node]


    def is_out_tree(self) -> bool:
        return issubclass(self.node_type, out_node)

    def is_in_tree(self) -> bool:
        return issubclass(self.node_type, in_node)

    def is_full_tree(self) -> bool:
        return self.is_out_tree() and self.is_in_tree()


    def single_insert(self,
               nodes: list[node],
               value: Any,
               where: Optional[node | Iterable[node]] = None,
               how: Optional[Any] = None) -> node:
        # instantiate new node from value
        # would be done with an allocator in C++
        new_node: node = self.node_type(value)

        # 'where' or 'how' None will
        # insert as a new root
        if (where is None):
            nodes.append(new_node)
        else:
            # hook at multiple nodes
            # (no idea if this should be supported in C++ later,
            #  for demonstration of inheritance-diagrams, this is
            #  quite useful here)
            if (isinstance(where, Iterable)):
                for wh in where:
                    new_node.hook_as(how, wh)
            else:
                new_node.hook_as(how, where)

        return new_node



@dataclass
class root_tree(tree_base):
    """
    tree that will keep track
    of all of it's root nodes.

    nodes that are inserted with the in_hook_type
    are automatically considered root-nodes.
    """

    roots: list[node] = field(default_factory=list)

    def insert(self,
               value: Any,
               where: Optional[node | Iterable[node]] = None,
               how: Optional[Any] = None) -> node:
        return self.single_insert(self.roots, value, where, how)
    



@dataclass
class leaf_tree(tree_base):
    """
    tree that will keep track
    of all of it's leaf-nodes.

    nodes that are inserted with the out_hook_type
    are automatically considered leaf-nodes.
    """

    leaves: list[node] = field(default_factory=list)


    def insert(self,
               value: Any,
               where: Optional[node | Iterable[node]] = None,
               how: Optional[Any] = None) -> node:
        return self.single_insert(self.leaves, value, where, how)



@dataclass
class full_tree(root_tree, leaf_tree):
    """
    tree that will keep track
    of all of it's root and leaf-nodes.

    combination of root_tree and leaf_tree.
    """

    def insert(self,
               value: Any,
               where: Optional[node | Iterable[node]] = None,
               how: Optional[Any] = None,) -> node:
        new_node: node = self.single_insert(self.roots, value, where, how)
        # insertion with in-hook-type
        # creates a new root at where
        if (isinstance(how, self.node_type.in_hook_type)):
            self.roots.append(new_node)
            remove_if_present(self.roots, where)
        # insertion with out-hook-type
        # creates a new leaf at where
        elif (isinstance(how, self.node_type.out_hook_type)):
            self.leaves.append(new_node)
            remove_if_present(self.leaves, where)
        return new_node



def dfs(n: out_node) -> list[out_node]:
    res = [n]
    for c in n.children():
        res.extend(dfs(c))
    return res


def reverse_dfs(n: in_node) -> list[in_node]:
    res = [n]
    for p in n.parents():
        res.extend(reverse_dfs(p))
    return res