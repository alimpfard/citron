{
module Main where
}
%name Citron
%tokentype { Token }
%error { parseError }
%token
      ref         { TokenRef $$ }
      number      { TokenNumber $$ }
      is          { TokenAssign }
      my          { TokenModMy }
      var         { TokenModVar }
      const       { TokenModConst }
      Nil         { TokenNil }
      True        { TokenBooleanYes }
      False       { TokenBooleanNo }
      '\''        { TokenQuote }
      '('         { TokenParOpen }
      ')'         { TokenParClose }
      '{'         { TokenBlockOpen }
      '}'         { TokenBlockClose }
      ':'         { TokenColon }
      '.'         { TokenDot }
      ','         { TokenChain }
      '['         { TokenTupOpen }
      ']'         { TokenTupClose }
      '^'         { TokenRet }

%%

Statement : Exp '.'        { Statement $1 }

Exp : ref is Exp           { Assign $1 $3 }
    | number               { Number $1 }
    | ref                  { Ref $1 }
    | Nil                  { Nil }
    | True                 { BooleanYes }
    | False                { BooleanNo }
    | Exp Messages         { Messages $1 $2 }
    | Exp1                 { Exp1 $1 }

Exp1 : '(' Exp1 ')'          { Exp1 $2 }
     | '^' Exp               { Return $2 }
     | '[' ChainSepExp ']'   { Tuple $2 }

ChainSepExp : ChainSepExp ',' Exp { TupleImm $1 $3 }
            | Exp                 { TupleGen $1 }

Messages : UnaryMsg Messages  { Messages (Unary $1) $2 }
         | BinMsg Messages    { Messages (Binary $1) $2 }
         | KWMsg ',' Messages { Messages (Kw $1) $2 }
         | Messages

UnaryMsg : ref                { ($1) }
BinMsg : ref Exp              { ($1, $2) }
KWMsg  : ref ':' Exp KWMsg    { (Keyword $1 $3, $4)}
       | ref ':' Exp          { (Keyword $1 $3) }

{
parseError :: [Token] -> a
parseError t = error ("Parse error on Token " ++ (show t))

data Statement = Exp Exp
data Exp
      = Assign Ref Exp
      | Number Number
      | Ref String
      | Nil
      | BooleanYes
      | BooleanNo
      | Messages Exp Messages
      | Exp1 Exp1
data Exp1
      = Exp1 Exp1
      | Return Exp
      | Tuple ChainSepExp
data ChainSepExp
      = TupleImm Exp ChainSepExp
      | TupleGen Exp
data Messages
      = Messages Message Messages
      | End
}
