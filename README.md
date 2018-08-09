## Citron :: A Programming Language for the crazy and people that like functions [![Build Status](https://travis-ci.com/alimpfard/citron.svg?branch=master)](https://travis-ci.com/alimpfard/citron)

### Release
No binary release is available for android.

### Build guide
minimum required libraries:
* `libpcre` -- for regular expressions
* `libgc` -- for Garbage Collection

```sh
# clone this branch
$ git clone -m android --single-branch https://github.com/alimpfard/citron
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

### Basic language syntax:
All operations are done through sending and receiving messages between objects.

`receiver message`

`receiver keyword: 'string' messages: 1, can be: 'chained', like so.`

there are some examples in the `examples` directory.

running the interpreter is quite simple: `ctr eval` in the directory that the file `eval` is.

### Default Module Resolution
A default module resolution path is assigned at build-time, which normally points to the data directory of the install;
However, this can always be overridden by specifying an environment variable (CITRON_EXT_PATH by default).
