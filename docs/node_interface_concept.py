
from typing import Iterable, Any, Self, Optional, TypeAlias, Type
from abc import ABC, abstractmethod
from enum import Enum


class NodeInterface(ABC):

    HookType: TypeAlias = None


    @abstractmethod
    def get_value(self) -> Any:
        pass


    @abstractmethod
    def hooks(self) -> Iterable[HookType]:
        pass


    @abstractmethod
    def hook_as(self, h: HookType, parent: Self) -> None:
        pass


    @abstractmethod
    def children(self) -> Iterable[Self]:
        pass



class Tree:

    def __init__(self, node_type: Type[NodeInterface]) -> None:
        self.node_type: Type               = node_type
        self.root: Optional[NodeInterface] = None


    def append(self, 
               value: Any, 
               how: Optional[Any] = None,
               where: Optional[NodeInterface] = None) -> NodeInterface:
        if (how is None and where is None):
            self.root = self.node_type(value)
            return self.root
        elif (how is not None and where is not None):
            new: NodeInterface = self.node_type(value)
            new.hook_as(how, where)
            return new
        else:
            raise ValueError("invalid combination of None arguments")


    def erase(self, where: NodeInterface, how) -> NodeInterface:
        pass


    def empty(self) -> bool:
        return self.root is None
    

    def df_pre_order(self) -> Iterable[NodeInterface]:
        
        def df_visit(n: NodeInterface) -> Iterable[NodeInterface]:
            res = [n]
            for c in n.children():
                res.extend(df_visit(c))
            return res
        

        if (self.empty()):
            return list()
        return df_visit(self.root)



class BinaryTreeNode(NodeInterface):

    class Hooks(Enum):
        left  = 0
        right = 1

    HookType: TypeAlias = Hooks


    def __init__(self, value: Any) -> None:
        self.value: Any            = value
        self.left: Optional[Self]  = None
        self.right: Optional[Self] = None


    def get_value(self) -> Any:
        return self.value
    

    def hooks(self) -> Iterable[HookType]:
        return BinaryTreeNode.Hooks


    def hook_as(self, h: HookType, parent: Self) -> None:
        if (h == BinaryTreeNode.Hooks.left):
            parent.left = self
        else:
            parent.right = self


    def children(self) -> Iterable[Self]:
        return list(filter(lambda x: x is not None, [ self.left, self.right ]))



class RoseTreeNode(NodeInterface):

    class Hooks(Enum):
        first = 0
        last  = 1

    HookType: TypeAlias = Hooks


    def __init__(self, value: Any) -> None:
        self.value: Any           = value
        self.children_list: list[Self] = list()


    def get_value(self) -> Any:
        return self.value


    def hooks(self) -> Iterable[HookType]:
        return RoseTreeNode.Hooks


    def hook_as(self, h: HookType, parent: Self) -> None:
        if (h == RoseTreeNode.Hooks.first):
            parent.children_list.insert(0, self)
        else:
            parent.children_list.append(self)


    def children(self) -> Iterable[Self]:
        return self.children_list


class RoseTreeNodeAlt(NodeInterface):

    class Hooks(Enum):
        first = 0
        next  = 1

    HookType: TypeAlias = Hooks


    def __init__(self, value: Any) -> None:
        self.value: Any            = value
        self.first: Optional[Self] = None
        self.next: Optional[Self]  = None


    def get_value(self) -> Any:
        return self.value


    def hooks(self) -> Iterable[HookType]:
        return RoseTreeNodeAlt.Hooks


    def hook_as(self, h: HookType, parent: Self) -> None:
        if (h == RoseTreeNodeAlt.Hooks.first):
            parent.first = self
        else:
            parent.next = self


    def children(self) -> Iterable[Self]:
        res = list()
        iter: Self = self.first
        while (iter is not None):
            res.append(iter)
            iter = iter.next
        return res


def binary_tree_test() -> None:
    binTree: Tree = Tree(BinaryTreeNode)

    """
    example-tree:
            4
           | |
          |   |
         2     6
        | |   | |
       1   3 5   7

    df_pre_order() == [ 4 2 1 3 6 5 7 ]
    """

    root: BinaryTreeNode = binTree.append(4)
    l: BinaryTreeNode    = binTree.append(2, BinaryTreeNode.Hooks.left, root)
    ll: BinaryTreeNode   = binTree.append(1, BinaryTreeNode.Hooks.left, l)
    lr: BinaryTreeNode   = binTree.append(3, BinaryTreeNode.Hooks.right, l)
    
    r: BinaryTreeNode    = binTree.append(6, BinaryTreeNode.Hooks.right, root)
    rl: BinaryTreeNode   = binTree.append(5, BinaryTreeNode.Hooks.left, r)
    rr: BinaryTreeNode   = binTree.append(7, BinaryTreeNode.Hooks.right, r)

    print([ 4, 2, 1, 3, 6, 5, 7 ] == [ n.get_value() for n in binTree.df_pre_order() ])


def rose_tree_test() -> None:
    roseTree: Tree = Tree(RoseTreeNode)
    roseTreeAlt: Tree = Tree(RoseTreeNodeAlt)

    """
    example-tree:
        >"my_project"
        |>"app"
        ||>"Main.hs"
        ||>"Util.hs"
        |>my_project.cabal

    df_pre_order() == [ "my_project" "app" "Main.hs" "Util.hs" "my_project.cabal" ]
    """

    root1: RoseTreeNode = roseTree.append("my_project")
    app1: RoseTreeNode  = roseTree.append("app", RoseTreeNode.Hooks.last, root1)
    main1: RoseTreeNode  = roseTree.append("Main.hs", RoseTreeNode.Hooks.last, app1)
    util1: RoseTreeNode  = roseTree.append("Util.hs", RoseTreeNode.Hooks.last, app1)
    cab: RoseTreeNode  = roseTree.append("my_project.cabal", RoseTreeNode.Hooks.last, root1)

    root2: RoseTreeNodeAlt = roseTreeAlt.append("my_project")
    app2: RoseTreeNodeAlt  = roseTreeAlt.append("app", RoseTreeNodeAlt.Hooks.first, root2)
    main2: RoseTreeNodeAlt  = roseTreeAlt.append("Main.hs", RoseTreeNodeAlt.Hooks.first, app2)
    util2: RoseTreeNodeAlt  = roseTreeAlt.append("Util.hs", RoseTreeNodeAlt.Hooks.next, main2)
    cab: RoseTreeNodeAlt  = roseTreeAlt.append("my_project.cabal", RoseTreeNodeAlt.Hooks.next, app2)

    print([ n.get_value() for n in roseTree.df_pre_order() ])
    print([ n.get_value() for n in roseTreeAlt.df_pre_order() ])


binary_tree_test()
rose_tree_test()