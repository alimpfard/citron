# Project Citron

Citron is a general-purpose programming language mainly for unix-like systems.

Design goals: No such thing exists. it is made for the novelty of fun.

Many features of the standard library are implemented, but are stupidly bug-prone.

# Tell-me-how-this-works-I-dont-have-any-time

Citron is a Purely Object Oriented language, which means every action is carried out
by sending messages to objects; there are three types of messages:
+ Unary: `receiver message`
+ Binary: `receiver + operand` Binary message names can only be a single unicode code point.
+ Keyword: `receiver kw0: op0 kw1: op1` Keyword messages can of an arbitrary number.

statements must end in a dot (`.`).

Comments are only single-line comments and begin with a '#'.

variable bindings are mostly just bindings, but the objects are aware of their current binding.
`var name is expression`

`var` is a modifier.

There are three modifiers available:
+ `var` : a local variable. only required when declaring said variable.
+ `my` : a private variable. it is always required for access.
+ `const` : a captured variable. more on this later.


Assignment has two forms:
+ assignment with `is` or `:=` : in the form `expression is expression` or `name is expression`.
+ assignment with `=>` : in the form `name => expression`. creates private bindings.

The first from of assignment allows the user to decompose objects and values, or to create a simple binding:

`[a, b, c] is [1, 2, 3]` this form of assignment allows decomposition. the rhs of the assignment is a constructor.

all the normal decompositions follow. the user may override them, or add new decomposition behaviours by overriding the `unpack:` method.

| Type | behaviour of `unpack:`     |
| :------------- | :------------- |
| Object      | binds to reference if the type is right, or throws an exception |
| Array       | binds member-wise, unless the reference is `_`, in which case the value is discarded, or if the reference begins with `*`, in which case the reference is assigned multiple values.       |
| Map         | binds values to new keys |

any other object defaults to Object's `unpack:` method, unless overridden.

example:
```citron
var map is Map cnew: {
  key0 => ['Hello', 'World!'].
  theAnswerToLife => 42.
}.
(Map cnew: {
  key0 => [hello, _].
  theAnswerToLife => fourtyTwo.
}) is map.

#Now hello contains 'Hello', and 'World!' is discarded.
#And fourtyTwo contains 42.
```

general IO:

```citron
#reading from stdin
var in is Program waitForInput. #reads until a newline.
var raw is Program input. #reads until an EOF

#Printing to the term.
Pen write: 'test'. #writes to stdout. no terminating newline.
Pen brk. #writes a newline '\n'
Pen writeln: 'string'. #writes to stdout with a terminating newline.

Pen red.
Pen blue.
Pen resetColor.
#many colors are available. look them up in the docs, or look at them live with Reflect.

#Program arguments
var argc is Program argumentCount.
var args is Iterator rangeFrom: 0 to: argc step: 1, fmap: (Reflect closure: 'argument:' of: Program).
```
