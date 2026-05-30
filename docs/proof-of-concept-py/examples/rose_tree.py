
from typing import Optional, Self, Type
from dataclasses import dataclass
from enum import Enum

from node import out_node
from examples.unary_in_node import unary_in_node, unary_in_how



class rose_out_how(Enum):

    first_child  = 1
    next_sibling = 3



@dataclass
class rose_out_node(out_node):

    out_hook_type: Type = rose_out_how

    first_child:  Optional[Self] = None
    next_sibling: Optional[Self] = None


    def hook_as(self, h: rose_out_how, where: Self) -> None:
        if (h == rose_out_how.first_child):
            if (where.first_child is not None):
                self.first_child = where.first_child
            where.first_child  = self
        elif (h == rose_out_how.next_sibling):
            if (where.next_sibling is not None):
                self.next_sibling = where.next_sibling
            where.next_sibling = self


    def children(self) -> list[Self]:
        res = list()
        sibling_iter = self.first_child
        while (sibling_iter is not None):
            res.append(sibling_iter)
            sibling_iter = sibling_iter.next_sibling
        return res



@dataclass
class rose_full_node(unary_in_node, rose_out_node):
    

    def hook_as(self, 
                h: rose_out_how | unary_in_how,
                where: Self) -> None:
        if (isinstance(h, rose_out_how)):
            rose_out_node.hook_as(self, h, where)
            unary_in_node.hook_as(where, unary_in_how.parent, self)
        elif (isinstance(h, unary_in_how)):
            
            # update all children
            for c in where.children():
                unary_in_node.hook_as(self, h, c)
            unary_in_node.hook_as(self, h, where)