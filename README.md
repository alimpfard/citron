[![Build Status](https://travis-ci.com/alimpfard/citron.svg?branch=master)](https://travis-ci.com/alimpfard/citron)
## Citron :: A Programming Language for the crazy and people that like functions

### Build guide
minimum required libraries:
* `libdl`   -- for dynamic loading of modules
* `libbsd`  -- for utility functions (compile without forLinux defined to get rid of this dependency)
* `libpcre` -- for regular expressions
* `libpthread` -- for threading support
* `libgc` -- if compiling with the boehm garbage collector

building for debug:

+ with boehm gc: `WITH_BOEHMGC=1 make debug`
+ without: `make debug`

Since this is an alpha version, `install` is not allowed.

### Basic language syntax:
All operations are done through sending and receiving messages between objects.

`receiver message`

`receiver keyword: 'string' messages: 1, can be: 'chained', like so.`

there are some examples in the `examples` directory.

running the interpreter is quite simple: `ctr eval` in the directory that the file `eval` is.
