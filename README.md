## Citron :: A Programming Language for the crazy and people that like functions [![Build Status](https://travis-ci.com/alimpfard/citron.svg?branch=master)](https://travis-ci.com/alimpfard/citron) [![codecov](https://codecov.io/gh/alimpfard/citron/branch/master/graph/badge.svg)](https://codecov.io/gh/alimpfard/citron)

### Release
A binary release (x86_64 ELF) is available (see [Default Module Resolution](#default-module-resolution) for dynamic modules)

You can also use the docker image (will probably be auto-built later...)
```sh
# to run an interpreter
$ docker run --rm -it anothertest/citron:latest
```

### Build guide
required libraries:
* `libdl`   -- for dynamic loading of modules
* `libbsd`  -- for utility functions (compile without forLinux defined to get rid of this dependency)
* `libpcre` -- for regular expressions
* `libpthread` -- for threading support
* `libgc` -- for Garbage Collection
* `libsparsehash` -- Symbol storage
* `llvm` -- (>= 7) for inline asm (if you're on a braindead distro that adds version numbers to these, add an alias for `llvm-config`)

build-time tools:
* cmake -- for libsocket

#### for the braindead distros (don't kill me)
```sh
$ sudo apt install build-essential cmake libbsd-dev libpcre3-dev libgc-dev libsparsehash-dev llvm-8 llvm-8-dev
```

#### Basic steps to build and install:

clone this repository

```sh
$ cd citron
$ git submodule update --init
$ make
$ sudo make install
```

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
However, this can always be overridden by specifying an environment variable (`CITRON_EXT_PATH` by default).


### Scratchpad fun
Assuming the Tcl extension is built and accessible (the binary build has it); a basic scratchpad can be accessed:
```sh
$ citron -m scratchpad
```
For a basic introduction, see [Scratchpad](https://alimpfard.github.io/citron_book/book/scratchpad.html) on the [citron-book](https://alimpfard.github.io/citron_book/book/)
