autocmd BufEnter * :syntax sync fromstart

syntax match citronMessage "\<[a-zA-Z_;!@%^&*+=-~`\/\"][0-9a-zA-Z_;!@%^&*+=-~`/\"]*:"

syntax match citronIdent "\<[a-zA-Z_;!@%^&*+=-~`\/\"][0-9a-zA-Z_;!@%^&*+=-~`/\"]*\>|\<.\>"
syntax match citronDelimiter "[()\[\]{},.]"

syn cluster citronStmtGroup contains=citronMessage,citronIdent,citronDelimiter,citronBlock,citronTypeName,citronInteger,citronFloat,itronString,citronLineComment,citronBoolean

syntax match citronBlockInit "\\\(\:[a-zA-Z_;!@%^&*+=-~`/\"][0-9a-zA-Z_;!@%^&*+=-~`/\"]*\)\+" contains=@citronStmtGroup
syntax region citronBlockIni0 start="{\(:[a-zA-Z_;!@%^&*+=-~`/\"][0-9a-zA-Z_;!@%^&*+=-~`/\"]*\)*" end=/}/ contains=@citronStmtGroup
syntax region citronBlockIni2 start="{\\\(:[a-zA-Z_;!@%^&*+=-~`/\"][0-9a-zA-Z_;!@%^&*+=-~`/\"]*\)*" end=/}/ contains=@citronStmtGroup
syntax cluster citronBlock contains=citronBlockInit,citronBlockIni0,citronBlockInit2

syntax match citronTypeName "\<[A-Z][a-zA-Z_;!@%^&*+=-~`]*\>"
syntax match citronInteger "\<\d\+"

syn match       citronFloat             "\<-\=\d\+\.\d*\%([Ee][-+]\=\d\+\)\=\>"
syn match       citronFloat             "\<-\=\.\d\+\%([Ee][-+]\=\d\+\)\=\>"

hi def link     citronFloat             Float


syn match     citronEscapeOctal       display contained "\\[0-7]\{3}"
syn match     citronEscapeC           display contained +\\[abfnrtv\\'"]+
syn match     citronEscapeX           display contained "\\x\x\{2}"
syn match     citronEscapeU           display contained "\\u\x\{4}"
syn match     citronEscapeBigU        display contained "\\U\x\{8}"
syn match     citronEscapeError       display contained +\\[^0-7xuUabfnrtv\\'"]+
syn match     citronStringEmbed       display contained :$$[a-zA-Z_;!@%^&*+=-~`/"][0-9a-zA-Z_;!@%^&*+=-~`/"]*:

hi def link citronEscapeOctal citronSpecialString
hi def link citronEscapeC citronSpecialString 
hi def link citronEscapeX citronSpecialString
hi def link citronEscapeU citronSpecialString
hi def link citronEscapeBigU citronSpecialString
hi def link citronSpecialString citronSpecialString
hi def link citronEscapeError Error

hi def link citronStringEmbed citronStringEmb 
hi def link citronSpecialString Special
hi def link citronStringEmb Character

syn cluster citronStringGroup contains=citronEscapeOctal,citronEscapeC,citronEscapeX,citronEscapeU,citronEscapeBigU,citronEscapeError,citronStringEmbed

syntax region citronString start=/>?/ skip=/\\./ end=/<?/
syntax region citronRString start=/'/ skip=/\\./ end=/'/ contains=@citronStringGroup

syntax match citronLineComment "#.*$" contains=citronTodo

syn keyword citronTodo contained NOTE
hi def link citronTodo Todo

syntax keyword citronKW var const my is frozen
syntax keyword citronConstant String Object Program Clock CodeBlock Reflect Generator Array Pen Nil Boolean
syn keyword citronBoolean True False

hi def link citronBoolean Boolean

highlight link citronIdent Identifier
highlight link citronBlockInit  Operator
highlight link citronBlockInit0 Operator
highlight link citronBlockInit2 Operator
highlight link citronMessage Operator
highlight link citronDelimiter Operator
highlight link citronInteger Constant
highlight link citronConstant Constant 
highlight link citronTypeName Constant
highlight link citronString String
highlight link citronRString String
highlight link citronLineComment Comment
highlight link citronKW Keyword

let b:current_syntax = "citron"

" vim: sw=2 ts=2 et
