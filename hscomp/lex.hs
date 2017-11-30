import qualified Data.Map.Strict as SM
import CtrAST
import CtrTokenType (TokenType)
import qualified Control.Exception as Ex
import Data.Char

module CtrLex
    (
    ) where

type TokenizingState = (
                          String, -- current token buffer
                          String, -- the rest of the code
                          String, -- old 'rest of the code'
                          String, -- older 'rest of the code'
                          Bool, -- verbatim
                          Int, -- line no
                          Int, -- old lineno
                          Int, -- string interpol state
                          String, -- ivar name
                       )

tokenDescriptions :: SM CtrTokenType String
tokenDescriptions = SM.fromList [
                        (Ref, "Reference"),
                        (Quote, "'"),
                        (ParOpen, "("),
                        (ParClose, ")"),
                        (BlockOpen, "{"),
                        (BlockClose, "}"),
                        (Colon, ":"),
                        (Dot, "."),
                        (Chain, ","),
                        (BooleanYes, "True"),
                        (BooleanNo, "False"),
                        (Nil, "Nil"),
                        (Assignment, "is"),
                        (Ret, "^expression"),
                        (TupOpen, "["),
                        (TupClose, "]"),
                        (Fin, "End Of Program")]

describeToken :: CtrTokenType -> String
describeToken tok = findWithDefault "(unknown token)" tok tokenDescriptions

isDelimiter :: Char -> Bool
isDelimiter c = c `elem` "(),.: "

formatError :: String -> Int -> Exception String
formatError msg lineno = Ex.toException (msg : "on line: " : (show lineno) : ".")

tokenValue :: TokenizingState -> (TokenizingState, String)
tokenValue state = (state, fst state)

tokenLength :: TokenizingState -> (TokenizingState, Int)
tokenLength state = (state, length $ fst state)

putBackToken :: TokenizingState -> TokenizingState
putBackToken (buf, code, ocode, orcode, verbatim, lineno, olineno, sinterpol, ivar) =
    (buf, ocode, orcode, orcode, verbatim, olineno, olineno, (if sinterpol > 0 then sinterpol - 1 else sinterpol), ivar)

isValidDigitInBase :: Char -> Int -> Bool
isValidDigitInBase c base
  | base == 8  = isOctDigit c
  | base == 10 = isDigit c
  | base == 16 = isHexDigit c
  | base <= 10 && c >= '0' && c < (chr ((ord '0') + base)) = True
  | base <= 36 = ((c >= '0' && c <= '9') || (c >= 'A' && c < (chr ((ord 'A') + base - 10))))
  | otherwise = False

getToken :: TokenizingState -> (TokenizingState, TokenType)
getToken state@(buf, code, ocode, orcode, verbatim, lineno, olineno, sinterpol, ivar) =
  case expression of
    pattern -> expression
    otherwise -> expression
