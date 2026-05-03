
from __future__ import annotations

from copy import copy
from dataclasses import dataclass, field
from typing import Any, Optional, Iterable, TypeAlias
from abc import ABC, abstractmethod
from enum import Enum


class node_interface:    

    class Identifier:
        pass

    @property
    def value(self) -> Any:
        pass

    @value.setter
    def value(self, v) -> None:
        self.value = v

    @property
    def subordinate(self, i: Identifier) -> node_interface:
        pass 

    @subordinate.setter
    def subordinate(self, i: Identifier, n: node_interface) -> None:
        self.subordinate(i) = n
    



@dataclass
class binary_tree_node(node_interface):

    v: Any

    l: Optional[binary_tree_node]  = None
    r: Optional[binary_tree_node] = None 
    

    class Identifier(Enum):
        left  = 0
        right = 1

    @property
    def value(self) -> Any:
        return self.v

    @property
    def subordinate(self, i: Identifier) -> node_interface:
        if (i == self.Identifier.left):
            return self.l
        else:
            return self.r



@dataclass
class rose_tree_node(node_interface):

    v: Any

    parent: Optional[rose_tree_node] = None
    children: list[rose_tree_node]   = field(default_factory=list)

    Identifier: TypeAlias = int

    @property
    def value(self) -> Any:
        return self.v

    @property
    def subordinate(self, i: Identifier) -> node_interface:
        return self.children[i]



def copy_binary_tree_node(node: binary_tree_node) -> binary_tree_node:
    if (node is None):
        return None
    
    cpy = binary_tree_node(copy(node.v))
    cpy.l = copy_binary_tree_node(node.l)
    cpy.r = copy_binary_tree_node(node.r)
    
    return cpy


def copy_rose_tree_node(node: rose_tree_node) -> rose_tree_node:
    if (node is None):
        return None
    
    cpy = rose_tree_node(copy(node.v))
    for c in node.children:
        cpy.children.append(copy_rose_tree_node(c))
    
    return cpy


def copy_node(node: node_interface) -> node_interface:
    if (node is None):
        return None
    
    cpy: node_interface = copy(node)
    cpy.value = copy(node.value)

    for ident in node.Identifier:
        cpy.subordinate(ident) = copy_node(node.subordinate(ident))

    return cpy
    

def depth_first(node: node_interface) -> list[node_interface]:
    res = list()
    if (node is None):
        return res
    
    res.append(node)
    for c in node.Identifier:
        res += depth_first(node.subordinate(c))
    return res


a = binary_tree_node(1,
                     binary_tree_node(2),
                     binary_tree_node(3))

print(depth_first(a))