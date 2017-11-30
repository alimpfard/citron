module CtrAST
    (
    ) where

data NodeModifier = My | Var | Const | None

data ASTNode   = ExprAssignment  NodeModifier [Char] [ASTNode]
               | ExprMessage  NodeModifier [Char] [ASTNode]
               | UnaMessage  NodeModifier [Char] [ASTNode]
               | BinMessage  NodeModifier [Char] [ASTNode]
               | KwMessage  NodeModifier [Char] [ASTNode]
               | LtrString  NodeModifier [Char] [ASTNode]
               | Reference  NodeModifier [Char] [ASTNode]
               | LtrNum  NodeModifier [Char] [ASTNode]
               | CodeBlock  NodeModifier [Char] [ASTNode]
               | ReturnFromBlock  NodeModifier [Char] [ASTNode]
               | Immutable  NodeModifier [Char] [ASTNode]
               | ParamList  NodeModifier [Char] [ASTNode]
               | InstrList  NodeModifier [Char] [ASTNode]
               | EndOfProgram  NodeModifier [Char] [ASTNode]
               | Nested  NodeModifier [Char] [ASTNode]
               | LtrBoolTrue  NodeModifier [Char] [ASTNode]
               | LtrBoolFalse  NodeModifier [Char] [ASTNode]
               | LtrNil  NodeModifier [Char] [ASTNode]
               | Program  NodeModifier [Char] [ASTNode]
