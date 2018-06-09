## Citron :: A Programming Language for the crazy and people that like functions [![Build Status](https://travis-ci.com/alimpfard/citron.svg?branch=master)](https://travis-ci.com/alimpfard/citron)

### Release
A binary release (x86_64 ELF) is available (see [Default Module Resolution](#default-module-resolution) for dynamic modules)

### Build guide
minimum required libraries:
* `libdl`   -- for dynamic loading of modules
* `libbsd`  -- for utility functions (compile without forLinux defined to get rid of this dependency)
* `libpcre` -- for regular expressions
* `libpthread` -- for threading support
* `libgc` -- for Garbage Collection

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

### Default Module Resolution
A default module resolution path is assigned at build-time, which normally points to the data directory of the install;
However, this can always be overridden by specifying an environment variable (CITRON_EXT_PATH by default).
