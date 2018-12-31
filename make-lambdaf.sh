#!/bin/bash
# http://www.gnu.org/software/autoconf/manual/autoconf.html#Portable-Shell

OUT="lambdaf.a"
CC=${CC:-"gcc"}     # check for CC env var to support clang's scan-build
AR=ar

# stack-protector should be off by default, but some linux distributions
# patch gcc to turn it on by default, so we explicitly disable it here; it
# will cause the nostdlib build to fail if it is enabled

# no-asynchronous-unwind-tables decreases the executable size and there seems
# to be no downside because the information will still be included when
# compiling with "-g" https://stackoverflow.com/a/26302715

# no-ident removes a small unneeded comment from the executable

# use this command to find all warnings not enabled by -Wall -Wextra
# gcc -Wall -Wextra -Q --help=warning | grep "\[disabled\]"
# note: -Wswitch-enum looks helpful, but doesn't allow default case
# to catch multiple cases, which we are using
CC_FLAGS="-c -fsigned-char -std=c99 -Ilambdaf/ \
	-Wextra -Wshadow -Winit-self -Wwrite-strings \
-Wconversion -Wcast-qual -Wredundant-decls -fpic \
-fno-ident -fno-stack-protector -fno-asynchronous-unwind-tables"


# gcc sets --hash-style to a non-default setting when calling ld, which
# increases the executable size; here we set it back to the default

LINK_FLAGS="-Wl,--hash-style=sysv,--gc-sections,--strip-discarded,--print-gc-sections"
SOURCES=`find -L ./lambdaf -name "*.c" -not -path "./lambdaf/shared/lib/libc/*"`
HEADERS=`find -L ./lambdaf -name "*.h" -not -path "./lambdaf/shared/lib/libc/*"`
FOBJECTS=`find -L ./lambdaf -name "*.c" -not -path "./lambdaf/shared/lib/libc/*" | xargs basename -s .c | xargs -I {} echo {}.o`

echoexec() {
    echo "$@"
    "$@"
}

clean() {
    rm -f $FOBJECTS
    rm -f "$OUT"
}

is_up_to_date() {
    [[ -f "$1" && -z `find -L . -name "*.[c|h]" -newer "$1"` ]]
}

build_libc() {
    (cd lambdaf/shared/lib/libc && ./make)
}

build() {
    if ! is_up_to_date "$OUT"; then
        xxd -i 'lambdaf/stdlib.zero' | sed 's/};/, 0x00};/' > lambdaf/lstdlib.h &&
        echoexec $CC $CC_FLAGS $SOURCES &&
        echoexec $AR -cvq "$OUT" $FOBJECTS &&
        rm -f $FOBJECTS &&
        echo &&
        echo "BUILD SUCCESSFUL:" &&
        ls -gG "$OUT" || (rm -f $FOBJECTS ; false)
        return
    fi
    echo "$OUT up-to-date"
}

config_fast() {
    CC_FLAGS="-DNDEBUG -O3 -flto $CC_FLAGS"
    LINK_FLAGS="-flto -Wl,--strip-all $LINK_FLAGS"
}

config_debug() {
    CC_FLAGS="-g -Og $CC_FLAGS"
}

config_custom() {
    custom="-nostdlib -nostdinc -isystem ./lambdaf/shared/lib/libc/include/ -fno-builtin"
    CC_FLAGS="$custom $CC_FLAGS"
    LINK_FLAGS="-nostdlib $LINK_FLAGS"
    SOURCES=`find -L . -name "*.c"`
    FOBJECTS="*.o lambdaf/shared/lib/libc/sys.o"
}

config_small() {
    config_custom
    # would like to use -Os optimization level, but there is an issue with
    # -ftree-slp-vectorize and possibly other optimizations
    CC_FLAGS="-s -O0 -flto $CC_FLAGS"
    LINK_FLAGS="-flto -Wl,--strip-all $LINK_FLAGS"
}

config_unused() {
    config_custom
    CC_FLAGS="-O0 -fdata-sections -ffunction-sections $CC_FLAGS"
}

get_loc() {
    $CC -fpreprocessed -dD -E -P $HEADERS $SOURCES | grep -v assert | wc -l
}

run_cloc() {
    cloc --quiet --exclude-list-file=.clocignore --by-file $*
}

get_includes() {
    filename="$(basename "$1" | cut -d'.' -f 1)"
    gcc -MM -MT':' -I"." "$1" | tr -d ':\\\\' | xargs -n 1 echo |
        grep -v "lib/" | xargs -n 1 basename |
        xargs -n 1 echo "    $filename ->" |
        grep '\.h' | cut -d'.' -f 1 | paste -d';' - <(echo -n) |
        grep -v -- "-> $filename"
}

build_graph() {
    cd src
    echo "digraph graphname {"
    echo "    size=6;"
    echo "    ranksep=1.2;"
    echo "    nodesep=0.8;"
    echo "    mclimit=100;"
    echo
    export -f get_includes
    find . -name "*.c" | grep -v "lib/" |
        xargs -n 1 -I{} bash -c "get_includes {}"
    echo "}"
}

case "$1" in
    "") config_debug && build;;
    test) config_debug && build && test/test.sh;;
    fast) config_fast && build;;
    small) config_small && build_libc && build;;
    unused) clean && config_unused && build_libc && build;;
    clean) clean;;
    tree) tree --noreport -C -P *.[hc] -I lib src;;
    dot) build_graph;;
    graph) build_graph | dot -o architecture.pdf -T pdf;;
    loc) get_loc;;
    cloc) run_cloc --exclude-dir="libc" . | tail -n +3 | cut -c -78;;
    cloc-all) run_cloc . | tail -n +3 | cut -c -78;;
    *) echo "usage: $0 [{test|fast|small|unused|clean|graph|loc}]"; exit 1;;
esac
