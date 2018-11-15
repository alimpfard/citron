## Citron :: A Programming Language for the crazy and people that like functions [![Build Status](https://travis-ci.com/alimpfard/citron.svg?branch=master)](https://travis-ci.com/alimpfard/citron) [![Codacy Badge](https://api.codacy.com/project/badge/Grade/d55fb1e699e14d50b9882af24cde137d)](https://www.codacy.com/app/Citron/citron?utm_source=github.com&amp;utm_medium=referral&amp;utm_content=alimpfard/citron&amp;utm_campaign=Badge_Grade)

### Release
No binary release is available for android.

### Build guide
minimum required libraries:
* `libpcre` -- for regular expressions
* `libgc` -- for Garbage Collection

```sh
# clone this branch
$ git clone -b android --single-branch https://github.com/alimpfard/citron
$ cd citron
# compile everything
$ make all
# put a helper script for the eval in usr/bin
$ bash create_eval_link.sh
```

building for debug:
`make debug`

building without debug info:
`make all`

to create a link to the evaluator in the system binaries:
`bash create_eval_link.sh`
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
However, this can always be overridden by specifying an environment variable (`CITRON_EXT_PATH` by default).


### Scratchpad fun
Assuming the Tcl extension is built and accessible (the binary build has it); a basic scratchpad can be accessed:
```sh
$ citron -m scratchpad
```
For a basic introduction, see [Scratchpad](https://github.com/alimpfard/citron_book/blob/master/scratchpad.md) on the [citron-book](https://github.com/alimpfard/citron_book)
