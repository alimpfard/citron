## Citron :: A Programming Language for the crazy and people that like functions [![Build Status](https://travis-ci.com/alimpfard/citron.svg?branch=master)](https://travis-ci.com/alimpfard/citron) [![Codacy Badge](https://api.codacy.com/project/badge/Grade/d55fb1e699e14d50b9882af24cde137d)](https://www.codacy.com/app/Citron/citron?utm_source=github.com&amp;utm_medium=referral&amp;utm_content=alimpfard/citron&amp;utm_campaign=Badge_Grade)

### Release
A binary release (x86_64 ELF) is available (see [Default Module Resolution](#default-module-resolution) for dynamic modules)

### Build guide
minimum required libraries:
* `libdl`   -- for dynamic loading of modules
* `libbsd`  -- for utility functions (compile without forLinux defined to get rid of this dependency)
* `libpcre` -- for regular expressions
* `libpthread` -- for threading support
* `libgc` -- for Garbage Collection

Basic steps to build and install:

clone this repository

```sh
$ cd citron/autohell
$ autoreconf
$ ./configure --with-ffi
$ make
$ make install
```
The normal configure options apply.

##### Note
If the binary is built without ffi, to launch the repl, you must pass these flags to it:
+ `--assume-non-tty --without-signals`

i.e. the invocation would be `citron --assume-non-tty --without-signals`


### Basic language syntax:
All operations are done through sending and receiving messages between objects.

`receiver message`

`receiver keyword: 'string' messages: 1, can be: 'chained', like so.`

there are some examples in the `examples` directory.


### Default Module Resolution
A default module resolution path is assigned at build-time, which normally points to the data directory of the install;
However, this can always be overridden by specifying an environment variable (CITRON_EXT_PATH by default).
