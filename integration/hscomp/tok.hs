module CtrTokenType
    (
      TokenType,
      utf8_bytes
    ) where

data TokenType = Ref          -- referenece
               | Quote        -- '
               | Number       -- 123 | 123.32 | 0xdeadbabe
               | ParOpen      -- (
               | ParClose     -- )
               | BlockOpen    -- {
           --  | BlockOpen_Autocapture -- {\
               | BlockClose   -- }
               | Colon        -- :
               | Dot          -- .
               | Chain        -- ,
               | BooleanYes   -- True
               | BooleanNo    -- False
               | Nil          -- Nil
               | Assignment   -- (var|my|const)? name is value
               | Ret          -- ^expression
               | TupOpen      -- [
               | TupClose     -- ]
               | Fin          -- EOF

utf8_bytes :: (Int,Int,Int)
utf8_bytes = (192, 224, 240)
