
from typing import Any, Iterable


def remove_if_present(l: list[Any], v: Any) -> None:
    try: l.remove(v)
    except ValueError: pass


class ranges:

    # Source - https://stackoverflow.com/a/952952
    # Posted by Alex Martelli, modified by community. See post 'Timeline' for change history
    # Retrieved 2026-05-21, License - CC BY-SA 4.0, altered with types and generators.
    @staticmethod
    def flatten(xss: Iterable[Iterable[Any]]) -> Iterable[Any]:
        for xs in xss:
            for x in xs:
                yield x

    
    @staticmethod
    def intersperse(xs: Iterable[Any], 
                    delimiter: Any,
                    interval: int = 1) -> Iterable[Any]:
        if (len(xs) == 0):
            return xs
        
        counter: int = 0
        for x in xs[:-1]:
            yield x
            counter += 1
            if (counter == interval):
                yield delimiter
                counter = 0
        yield xs[-1]
    

    @staticmethod
    def concat(xs: Iterable[Any], ys: Iterable[Any]) -> Iterable[Any]:
        for x in xs:
            yield x
        for y in ys:
            yield y

