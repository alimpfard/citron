" Initialization {{{
" -----------------------------------------------------------------------------

if exists("b:current_syntax")
    finish
endif

let s:cpo_save = &cpo
set cpo&vim

" -----------------------------------------------------------------------------

syn case match

" TODO:  do something intelligent here.
setlocal iskeyword=a-z,A-Z,48-57,_

" Still not perfect.
syn sync minlines=300

" Errors {{{1
" -----------------------------------------------------------------------------

syn match ctrError /\S/ contained
syn match ctrBangError /!/ contained

syn match ctrDelimError /)/
syn match ctrDelimError /\]/
syn match ctrDelimError /}/

" ctrMethodMembers holds groups that can appear in a method.
syn cluster ctrMethodMembers contains=ctrDelimError

" Comments {{{1
" -----------------------------------------------------------------------------

syn keyword ctrTodo FIXME TODO XXX contained
syn region ctrComment
    \ start=/\#/ end=/\n/
    \ contains=ctrTodo,@Spell
    \ fold

if exists("ctr_stx_style_end_of_line_comments")
    syn match ctrComment +"/.*$+ contains=ctrTodo,@Spell
endif

syn cluster ctrMethodMembers add=ctrComment

" Reserved keywords {{{1
" -----------------------------------------------------------------------------

syn keyword ctrNil Nil
syn keyword ctrBoolean True False
syn keyword ctrKeyword me const var my frozen

syn cluster ctrMethodMembers add=ctrNil,ctrBoolean,ctrKeyword

" Methods {{{1
" -----------------------------------------------------------------------------

" syn region ctrMethods
"     \ matchgroup=ctrMethodDelims
"     \ start=
"         \/!\%(\K\k*\%(\.\|::\)\)*
"         \\K\k*\s\+\%(class\s\+\)\?
"         \methodsFor:[^!]\+!/
"     \ end=/\_s\@<=!/
"     \ contains=ctrError,ctrMethod,ctrComment
"     \ transparent fold

" -----------------------------------------------------------------------------

" Binary messages
syn region ctrMethod
    \ matchgroup=ctrMessagePattern
    \ start=/\_s[+\-\*\&\%\$\@\!\~\_\=\/\<\>\"\;]\+/
    \ end=/\_s/
    \ transparent fold

" Keyword messages
syn region ctrMethod
    \ matchgroup=ctrMessagePattern
    \ start=/\K\k*:/
    \ end=/\_s/
    \ transparent fold

syn cluster ctrMethodMembers add=ctrMethod
" Smalltalk/X-style primitives {{{1
" -----------------------------------------------------------------------------

if exists("ctr_stx_style_primitives")
    syn include @cCode syntax/c.vim

    syn region ctrStxPrimitive
        \ matchgroup=ctrStxPrimitiveDelims
        \ start=/%{/ end=/%}/
        \ contains=@cCode
        \ transparent fold

    syn cluster ctrMethodMembers add=ctrStxPrimitive

    syn region ctrPrimitiveDefinitions
        \ matchgroup=ctrMethodDelims
        \ start=
            \/!\%(\K\k*\%(\.\|::\)\)*
            \\K\k*\s\+\%(class\s\+\)\?
            \primitiveDefinitions\s*!/
        \ end=/!\_s\+!/
        \ contains=ctrError,ctrComment,ctrStxPrimitive
        \ transparent fold

    syn region ctrPrimitiveFunctions
        \ matchgroup=ctrMethodDelims
        \ start=
            \/!\%(\K\k*\%(\.\|::\)\)*
            \\K\k*\s\+\%(class\s\+\)\?
            \primitiveFunctions\s*!/
        \ end=/!\_s\+!/
        \ contains=ctrError,ctrComment,ctrStxPrimitive
        \ transparent fold
endif

" Strings and characters {{{1
" -----------------------------------------------------------------------------

" Format spec, yeah right.  :)
syn match ctrFormatSpec /%./ contained

syn match ctrSpecialChar /\\./ contained

syn region ctrString
    \ matchgroup=ctrString
    \ start=/'/ skip=/\\'/ end=/'/
    \ contains=ctrSpecialChar,ctrFormatSpec,@Spell

syn region ctrString
    \ matchgroup=ctrString
    \ start=/?>/ end=/<?/
    \ contains=@Spell

" ctrLiterals holds all, uh, literals.
syn cluster ctrLiterals contains=ctrString
syn cluster ctrMethodMembers add=ctrString

" Symbols {{{1
" -----------------------------------------------------------------------------

syn region ctrSymbol
    \ matchgroup=ctrSymbol
    \ start=/\\\(:\)\@!/ end=/\_s/
    \ contains=ctrSpecialChar

if !exists("ctr_three_character_binary_selectors")
    syn match ctrSelector display /#[%&*+,/<=>?@\\~|-]\{1,2}/
else
    syn match ctrSelector display /#[%&*+,/<=>?@\\~|-]\{1,3}/
endif

syn cluster ctrLiterals add=ctrSymbol
syn cluster ctrMethodMembers add=ctrSymbol

" Numbers {{{1
" -----------------------------------------------------------------------------

syn match ctrInteger display /\%(-\s*\)\?\<\d\+\>/
syn match ctrRadixHexInteger display /-\?0[xX][0-9a-fA-F]\+/
syn match ctrRadixOctInteger display /-\?0[cC][0-7]\+/
syn match ctrRadixBinInteger display /-\?0[bB][01]\+/
syn match ctrFloat display /\%(-\s*\)\?\<\d\+\.\d\+\%([edq]-\?\d\+\)\?\>/
syn match ctrScaledDecimal display /\%(-\s*\)\?\<\d\+\%(\.\d\+\)\?s\%(\d\+\)\?\>/

" syn match ctrNumber
"     \ /-\?\<\d\+\%(\.\d\+\)\?\%([deqs]\%(-\?\d\+\)\?\)\?\>/
"     \ display
" syn match ctrNumber
"     \ /\<\d\+r-\?[0-9A-Z]\+\%(\.[0-9A-Z]\+\)\?\%([deqs]\%(-\?\d\+\)\?\)\?\>/
"     \ display

syn cluster ctrNumber contains=ctrInteger,ctrFloat,ctrScaledDecimal,ctrRadixHexInteger,ctrRadixOctInteger,ctrRadixBinInteger

syn cluster ctrLiterals add=@ctrNumber
syn cluster ctrMethodMembers add=@ctrNumber

" Highlighting mismatched parentheses {{{1
" -----------------------------------------------------------------------------

syn region ctrUnit matchgroup=ctrUnitDelims start=/(/ end=/)/ transparent
syn region ctrUnit matchgroup=ctrUnitDelims start=/\[/ end=/\]/ transparent

syn cluster ctrMethodMembers add=ctrUnit

" Arrays {{{1
" -----------------------------------------------------------------------------

syn region ctrArray
    \ matchgroup=ctrArrayDelims
    \ start=/\[/ end=/]/
    \ contains=@ctrLiterals,ctrComment,ctrArray,ctrNil,ctrBoolean
    \ contained transparent fold

" Variable binding and "eval" literals {{{1
" -----------------------------------------------------------------------------

syn match ctrBindingDelims /[{}]/ contained
syn match ctrBinding /#{\s*\%(\K\k*\.\)*\K\k*\s*}/ contains=ctrBindingDelims

syn region ctrEval
    \ matchgroup=ctrEvalDelims
    \ start=/##(/ end=/)/
    \ contains=@ctrMethodMembers
    \ transparent

syn cluster ctrLiterals add=ctrBinding,ctrEval
syn cluster ctrMethodMembers add=ctrBinding,ctrEval

" Pretty-printing for various groups {{{1
" -----------------------------------------------------------------------------

syn match ctrDelimiter /,/ contained display
syn match ctrIdentifier /\K\k*/ contained display

" Temporaries {{{1
" -----------------------------------------------------------------------------

" To FIXME or not to FIXME?  I will match at "boolean | boolean | boolean".
" syn region ctrTemps
"     \ matchgroup=ctrTempDelims
"     \ start=/|/ end=/|/
"     \ contains=ctrIdentifier,ctrError

syn match ctrTemps
    \ /\({\_s*\)\@<=\%(:\s*\%(\*\|&\)\?\K\k*\_s*\)\+/
    \ contains=ctrIdentifier,ctrDelimiter

syn cluster ctrMethodMembers add=ctrTemps

" Blocks {{{1
" -----------------------------------------------------------------------------

" I made up the name.
syn match ctrBlockConditional /\<whileTrue\>:\?/ contained
syn match ctrBlockConditional /\<whileFalse\>:\?/ contained

syn region ctrBlock
    \ matchgroup=ctrBlockDelims
    \ start=/{/ end=/}/
    \ contains=@ctrMethodMembers,ctrBangError
    \ nextgroup=ctrBlockConditional skipempty skipwhite
    \ transparent fold

syn cluster ctrMethodMembers add=ctrBlock

" Assignment and return operators {{{1
" -----------------------------------------------------------------------------

syn match ctrAssign /\<\(is\|:=\)\>/
syn match ctrAnswer /\^/
syn match ctrMeta   /$!\|$\((\)\@=\|$\(\[\)\@=/

syn cluster ctrMethodMembers add=ctrAssign,ctrAnswer,ctrMeta

" Common Boolean methods {{{1
" -----------------------------------------------------------------------------

syn match ctrConditional /\<ifTrue:/
syn match ctrConditional /\<ifFalse:/

syn match ctrConditional /\<equals:/
syn match ctrConditional /\s=\s/

syn match ctrConditional /\<and:/
syn match ctrConditional /\s&\s/

syn match ctrConditional /\<or:/
syn match ctrConditional /\s|\s/

syn match ctrConditional /\<xor:/
syn match ctrConditional /\s!\s/

syn match ctrConditional /\<not\>/

syn cluster ctrMethodMembers add=ctrConditional

" Link syntax groups {{{1
" -----------------------------------------------------------------------------

hi def link ctrAnswer           Statement
hi def link ctrArrayConst       Constant
hi def link ctrArrayDelims      Delimiter
hi def link ctrAssign           Operator
hi def link ctrBangError        Error
hi def link ctrBinding          Constant
hi def link ctrBindingDelims    Delimiter
hi def link ctrBlockConditional Conditional
hi def link ctrBlockDelims      Delimiter
hi def link ctrBoolean          Boolean
hi def link ctrCharacter        Character
hi def link ctrCollectDelims    Delimiter
hi def link ctrComment          Comment
hi def link ctrConditional      Conditional
hi def link ctrDelimError       Error
hi def link ctrDelimiter        Delimiter
hi def link ctrError            Error
hi def link ctrEvalDelims       PreProc
hi def link ctrFloat            Float
hi def link ctrFormatSpec       Special
hi def link ctrIdentifier       Identifier
hi def link ctrInteger          Number
hi def link ctrKeyword          Keyword
hi def link ctrMessagePattern   Conditional
hi def link ctrMethodDelims     Statement
hi def link ctrNil              Keyword
hi def link ctrRadixBinInteger  Number
hi def link ctrRadixOctInteger  Number
hi def link ctrRadixHexInteger  Number
hi def link ctrScaledDecimal    Float
hi def link ctrSelector         Constant
hi def link ctrSpecialChar      SpecialChar
hi def link ctrMeta             PreProc
hi def link ctrString           String
hi def link ctrSymbol           Constant
hi def link ctrTempDelims       Delimiter
hi def link ctrTodo             Todo

if exists("ctr_stx_style_primitives")
    hi def link ctrStxPrimitiveDelims Delimiter
endif

" Finalization {{{1
" -----------------------------------------------------------------------------

let b:current_syntax = "ctr"

let &cpo = s:cpo_save
unlet s:cpo_save

" -----------------------------------------------------------------------------
"}}}1

" vim:set sts=4 et ff=unix fdm=marker:
