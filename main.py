class Node:
    invalid: bool = False
    left: Node | None = None
    right: Node | None = None

import Enum, auto


class Token():
    
class Token(Enum):
    IntegerLiteral = auto()
    Plus = auto()
    Minus = auto()
    Mult = auto()
    Div = auto()
    Div = auto()

[Token.IntegerLiteral, Token.Plus, Token.IntegerLiteral]
