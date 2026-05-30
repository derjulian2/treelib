
from typing import Optional, Self, Type
from dataclasses import dataclass, field
from enum import Enum

from node import in_node



class diagram_out_how(Enum):

    derived_class = 0



class diagram_in_how(Enum):

    base_class    = 0



@dataclass
class inheritance_diagram_node(in_node):

    out_hook_type: Type = diagram_out_how
    in_hook_type: Type  = diagram_in_how

    base_classes: list[Self]    = field(default_factory=list)
    derived_classes: list[Self] = field(default_factory=list)


    def hook_as(self, 
                h: diagram_out_how | diagram_in_how, 
                where: Self) -> None:
        if (isinstance(h, diagram_out_how)):
            where.derived_classes.append(self)
            self.base_classes.append(where)
        elif (isinstance(h, diagram_in_how)):
            where.base_classes.append(self)
            self.derived_classes.append(where)
        

    def children(self) -> list[Self]:
        return self.derived_classes
    

    def parents(self) -> list[Self]:
        return self.base_classes