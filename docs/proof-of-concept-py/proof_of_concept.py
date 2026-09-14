
from typing import Type, TypeVar, Generic, Optional, Self, Iterable, Any
from abc import ABC, abstractmethod
from enum import Enum
from dataclasses import dataclass, field

# Source - https://stackoverflow.com/a/952952
# Posted by Alex Martelli, modified by community. See post 'Timeline' for change history
# Retrieved 2026-05-21, License - CC BY-SA 4.0, slightly altered with type-annotations

def flatten(xss: list[list[Any]]) -> list[Any]:
    return [x for xs in xss for x in xs]


@dataclass
class node_base:
    value: Any
    

class in_tree_node:
    
    @abstractmethod
    def in_hook_as(self, how: Any, where: Self) -> None: pass

    @abstractmethod
    def parents(self) -> Iterable[Self]: pass


class out_tree_node:
    
    @abstractmethod
    def out_hook_as(self, how: Any, where: Self): pass

    @abstractmethod
    def children(self) -> Iterable[Self]: pass


type node = node_base | in_tree_node | out_tree_node


@dataclass
class multi_tree:

    node_type: Type
    roots: list[node]  = field(default_factory=list)
    leaves: list[node] = field(default_factory=list)


    def is_in_tree(self) -> bool:
        return issubclass(self.node_type, in_tree_node)

    def is_out_tree(self) -> bool:
        return issubclass(self.node_type, out_tree_node)

    def is_full_tree(self) -> bool:
        return self.is_in_tree() and self.is_out_tree()

    def empty(self) -> bool:
        return len(self.roots) == 0 and len(self.leaves) == 0
    
    
    def insert(self,
               value: Any,
               where: Optional[node]  = None,
               how_out: Optional[Any] = None,
               how_in: Optional[Any]  = None) -> node:
        new_node: node = self.node_type(value)
        if (where is None):
            # if no node is specified and the tree:
            # - is an in-tree, we append as a leaf
            # - is an out-tree, we append as a root
            # - is a full-tree, we raise an error because
            #   we would have to choose what to do.
            if (self.is_full_tree()):
                raise TypeError("")
            elif (self.is_in_tree()):
                self.leaves.append()
            else: # (self.is_out_tree())
                self.roots.append(new_node)
        else:
            if (self.is_in_tree()):
                new_node.in_hook_as(how_in, where)
            if (self.is_out_tree()):
                new_node.out_hook_as(how_out, where)
        return new_node
    

    def df_pre_order(self) -> list[node]:
        
        def df_visit(n: node) -> list[node]:
            res = [n]
            for c in n.children():
                res.extend(df_visit(c))
            return res

        if (self.empty()):
            return list()
        return flatten([ df_visit(root) for root in self.roots ])


class bin_in(Enum):
    parent = 0

class bin_out(Enum):
    left  = 0,
    right = 1


@dataclass
class weak_binary_tree_node(node_base, out_tree_node):

    left: Optional[Self]   = None
    right: Optional[Self]  = None


    def out_hook_as(self, how: bin_out, where: Self) -> None:
        if (how == bin_out.left):
            where.left  = self
        else:
            where.right = self


    def children(self) -> list[Self]:
        return list(filter(lambda x: x is not None, [ self.left, self.right ]))


@dataclass
class in_binary_tree_node(node_base, in_tree_node):

    parent: Optional[Self] = None


@dataclass
class binary_tree_node(weak_binary_tree_node, in_binary_tree_node):
    pass

    
def binary_tree_test() -> None:

    bin_tree: multi_tree = multi_tree(weak_binary_tree_node)

    root: weak_binary_tree_node = bin_tree.insert(4)
    l: weak_binary_tree_node    = bin_tree.insert(2, root, bin_out.left)
    ll: weak_binary_tree_node   = bin_tree.insert(1, l, bin_out.left)
    lr: weak_binary_tree_node   = bin_tree.insert(3, l, bin_out.right)
    
    r: weak_binary_tree_node    = bin_tree.insert(6, root, bin_out.right)
    rl: weak_binary_tree_node   = bin_tree.insert(5, r, bin_out.left)
    rr: weak_binary_tree_node   = bin_tree.insert(7, r, bin_out.right)

    print(list(map(lambda x: x.value, bin_tree.df_pre_order())))

binary_tree_test()