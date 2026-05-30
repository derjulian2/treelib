
from typing import Any, Self, Iterable, Type
from abc import abstractmethod
from dataclasses import dataclass
from enum import Enum




@dataclass
class node_base:
    """
    for testing-purposes, every
    node should contain a value.
    """
    
    value: Any
    


class out_node(node_base):
    """
    object that can be part
    of a tree holding references to
    nodes facing away from the root(s).
    """

    out_hook_type: Type
    

    @abstractmethod
    def hook_as(self, h: Type, where: Self) -> None: pass

    @abstractmethod
    def children(self) -> Iterable[Self]: pass



class in_node(node_base):
    """
    object that can be part
    of a tree holding references to
    nodes facing towards the root(s).
    """

    in_hook_type: Type


    @abstractmethod
    def hook_as(self, h: Type, where: Self) -> None: pass    

    @abstractmethod
    def parents(self) -> Iterable[Self]: pass



type node = out_node | in_node