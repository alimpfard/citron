DEBUG_VERSION := 1603
DEBUG_BUILD_VERSION := "\"$(DEBUG_VERSION)\""
fv := $(strip $(shell ldconfig -p | grep libgc.so | cut -d ">" -f2 | head -n1))
location = $(CURDIR)/$(word $(words $(MAKEFILE_LIST)),$(MAKEFILE_LIST))
WHERE_ART_THOU := $(location)
new_makefile_l1 := $(shell perl -ne '/((DEBUG_VERSION := )(\d+))/ && print (sprintf("%s%s", "$$2", "$$3"+1));' $(WHERE_ART_THOU))
shell_cflags := ${CFLAGS}
shell_ldflags := ${LDFLAGS}

ifeq ($(strip ${WITH_ICU}),)
CFLAGS = -Wall -Wextra -Wno-unused-parameter -mtune=native\
              -march=native -D withTermios -D forLinux\
              -D CTR_STD_EXTENSION_PATH=\"`pwd`\"
else
CFLAGS = -Wall -Wextra -Wno-unused-parameter -mtune=native\
			 -march=native -D withTermios -D forLinux\
			 -D CTR_STD_EXTENSION_PATH=\"`pwd`\" -D withICU
LEXTRACF := ${LEXTRACF} -L/usr/lib -licui18n -licuuc -licudata
endif

ifeq ($(strip ${WITHOUT_BOEHM_GC}),)
CFLAGS := ${CFLAGS} "-D withBoehmGC"
LEXTRACF := ${LEXTRACF} ${fv}
else
endif

ifeq ($(strip ${WITH_TYPED_GC}),)
else
CFLAGS := ${CFLAGS} "-D withBoehmGC_P"
endif


.PHONY: gc_check
gc_check:
	@if [ "${fv}x" == "x" ]; then echo "Could not find libgc.so."; echo Failing; exit 1; else echo "Found libgc.so at ${fv}"; fi
	$(eval LEXTRACF := ${LEXTRACF} ${fv})
	echo CFLAGS = ${CFLAGS}

CFLAGS += ${shell_cflags}
LDFLAGS += ${shell_ldflags}
LEXTRACF := ${LDFLAGS} ${LEXTRACF} -flto -lstdc++

-include gc_check

OBJS = siphash.o utf8.o memory.o util.o base.o collections.o file.o system.o\
		lexer.o lexer_plug.o parser.o walker.o marshal.o reflect.o fiber.o\
		importlib.o coroutine.o symbol.o generator.o base_extensions.o citron.o\
		promise.o symbol_cxx.o world.o libsocket.so
EXTRAOBJS =

ifneq ($(findstring withCTypesNative=1,${CFLAGS}),)
OBJS := ${OBJS} _struct.o ctypes.o structmember.o
endif

ifneq ($(findstring withInlineAsm=1,${CFLAGS}),)
EXTRAOBJS := ${EXTRAOBJS} inline-asm.o
CFLAGS := ${CFLAGS} $(shell llvm-config --cflags --system-libs --libs core orcjit native)
CXXFLAGS := $(shell llvm-config --cxxflags --system-libs --libs core orcjit native) ${CXXFLAGS} -fexceptions
endif

ifneq ($(findstring withInjectNative=1,${CFLAGS}),)
OBJS := ${OBJS} inject.o tcc/libtcc1.a tcc/libtcc.a
endif

COBJS = ${OBJS} compiler.o

# .SUFFIXES:	.o .c

all: CFLAGS := $(CFLAGS) -O2
all: cxx
all: ctr ctrconfig

ctrconfig:
	$(CC) ctrconfig.c -o ctrconfig

debug: CFLAGS := ${CFLAGS} -DDEBUG_BUILD -DDEBUG_BUILD_VERSION=${DEBUG_BUILD_VERSION} -Og -g3 -ggdb3 -Wno-unused-function
debug: cxx
debug: ctr
debug:
	sed -i -e "1s/.*/${new_makefile_l1}/" makefile

install:
	echo -e "install directly from source not allowed.\nUse citron_autohell instead for installs"
	exit 1;
ctr:	$(OBJS) $(EXTRAOBJS)
	$(CXX) -fopenmp $(EXTRAOBJS) $(OBJS) ${CXXFLAGS}  -rdynamic -lm -ldl -lbsd -lpcre -lffi -lpthread ${LEXTRACF} -o ctr

libctr: CFLAGS := $(CFLAGS) -fPIC -DCITRON_LIBRARY
libctr: symbol_cxx
libctr: $(OBJS)
	$(CC) $(OBJS) -shared -export-dynamic -ldl -lbsd -lpcre -lffi -lpthread -o libctr.so

compiler: CFLAGS := $(CFLAGS) -D comp=1
compiler: cxx
compiler: $(COBJS)
	$(CC) $(COBJS) -rdynamic -lm -ldl -lbsd -lpcre -lffi -lprofiler -lpthread ${LEXTRACF} -o ctrc

cxx:
	echo "blah"

tcc/%.a:
	cd tcc && ./configure --prefix=$(realpath build) && $(MAKE) $<

# %.o: %.c
# 	echo "$<"
# 	$(CC) $(CFLAHS) -c $< -o $@ >/dev/null 2>&1

libsocket.so:
	make -C libsocket
	cp libsocket/libsocket.so libsocket.so

inline-asm.o:
	$(CXX) -g $(CFLAGS) -c inline-asm.cpp ${CXXFLAGS} -o inline-asm.o

%.o: %.c
	$(CC) -fopenmp $(CFLAGS) -c $< -o $@

define SHVAL =
for f in *.c; do\
	echo "Formatting $f";\
	cp "$f" ".bac/$f";\
	clang-format -i "$f";\
done
endef

format:
	mkdir -p .bac
	rm -rf .bac/*.c
	cp *.c .bac/
	$(value SHVAL)
	rm -rf *.c~

.ONESHELL:

unback:
	cp .bac/*.c .

clean:
	rm -rf ${OBJS} ctr
	make -C libsocket clean

cclean:
	rm -rf ${COBJS} ctrc

love:
	echo "Not war?"

war:
	echo "Not love?"
