
from typing import Optional, Self, Type
from dataclasses import dataclass
from enum import Enum

from node import in_node



class unary_in_how(Enum):

    parent = 0



@dataclass
class unary_in_node(in_node):

    in_hook_type: Type = unary_in_how

    parent: Optional[Self] = None


    def hook_as(self, h: unary_in_how, where: Self) -> None:
        # shift if parent already present
        if (where.parent is not None):
            self.parent = where
        where.parent = self


    def parents(self) -> list[Self]:
        if (self.parent is None):
            return [ ]
        return [ self.parent ]