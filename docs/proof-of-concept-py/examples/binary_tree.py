
from typing import Optional, Self, Type
from dataclasses import dataclass
from enum import Enum

from node import out_node
from examples.unary_in_node import unary_in_node, unary_in_how



class binary_out_how(Enum):
    
    left   = 1
    right  = 2



@dataclass
class binary_out_node(out_node):

    out_hook_type: Type    = binary_out_how
    
    left: Optional[Self]   = None
    right: Optional[Self]  = None
    

    def hook_as(self, h: binary_out_how, where: Self) -> None:
        if (h == binary_out_how.left):
            # shift if present
            if (where.left is not None):
                self.left = where.left
            where.left = self
        elif (h == binary_out_how.right):
            # shift if present
            if (where.right is not None):
                self.right = where.right
            where.right = self


    def children(self) -> list[Self]:
        return list(filter(lambda x: x is not None, [ self.left, self.right ]))



@dataclass
class binary_full_node(unary_in_node, binary_out_node):
    

    def hook_as(self, 
                h: binary_out_how | unary_in_how, 
                where: Self) -> None:
        if (isinstance(h, binary_out_how)):
            binary_out_node.hook_as(self, h, where)
            unary_in_node.hook_as(where, unary_in_how.parent, self)
        elif (isinstance(h, unary_in_how)):
            # determine at which hook 'self' is in
            # the parent node and shift there
            if (where.parent is not None):
                if (where.parent.left is where):
                    binary_out_node.hook_as(self, binary_out_how.left, where.parent)
                elif (where.parent.right is where):
                    binary_out_node.hook_as(self, binary_out_how.right, where.parent)
                unary_in_node.hook_as(where.parent, h, self)
                unary_in_node.hook_as(self, h, where)
            
