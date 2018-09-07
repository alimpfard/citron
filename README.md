## Citron :: A Programming Language for the crazy and people that like functions [![Build Status](https://travis-ci.com/alimpfard/citron.svg?branch=master)](https://travis-ci.com/alimpfard/citron) [![Codacy Badge](https://api.codacy.com/project/badge/Grade/d55fb1e699e14d50b9882af24cde137d)](https://www.codacy.com/app/Citron/citron?utm_source=github.com&amp;utm_medium=referral&amp;utm_content=alimpfard/citron&amp;utm_campaign=Badge_Grade)

### Release
No binary release is available for android.

### Build guide
this is to be built on Msys2 (mingw)

minimum required libraries:
* `libpcre` -- for regular expressions
* `libgc` -- for Garbage Collection

```sh
# clone this branch
$ git clone -b windows --single-branch https://github.com/alimpfard/citron
$ cd citron
# compile everything and make a neat dist package
$ make distribute
```

the package is `citron-release.tar`.

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
