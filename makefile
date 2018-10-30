DEBUG_VERSION := 543
DEBUG_BUILD_VERSION := "\"$(DEBUG_VERSION)\""
o_cflags := -I/mingw64/include
#it ain't so easy
#$(shell pkg-config --cflags tcl)
o_ldflags := -L/mingw64/lib -ltcl8.6 -ldl -lz -lpthread -lz
#$(shell pkg-config --libs tcl)
CFLAGS := ${CFLAGS} ${o_cflags} -I/usr/include
LEXTRACF := ${LEXTRACF} ${o_ldflags} -flto -lstdc++ -static-libgcc -static-libstdc++
location = $(CURDIR)/$(word $(words $(MAKEFILE_LIST)),$(MAKEFILE_LIST))
WHERE_ART_THOU := $(location)
new_makefile_l1 := $(shell perl -ne '/((DEBUG_VERSION := )(\d+))/ && print (sprintf("%s%s", "$$2", "$$3"+1));' $(WHERE_ART_THOU))
shell_cflags := ${CFLAGS}
shell_ldflags := ${LDFLAGS}

ifeq ($(strip ${WITH_ICU}),)
	CFLAGS = -Wall -Wextra -Wno-unused-parameter -D withTermios \
              -D CTR_STD_EXTENSION_PATH=\"`pwd`\"
else
	CFLAGS = -Wall -Wextra -Wno-unused-parameter \
			  -D withTermios \
			 -D CTR_STD_EXTENSION_PATH=\"`pwd`\" -D withICU
LEXTRACF := ${LEXTRACF} -L/usr/lib -licui18n -licuuc -licudata
endif

CFALGS := ${CFLAGS}

ifeq ($(strip ${WITHOUT_BOEHM_GC}),)
CFLAGS := ${CFLAGS} -D withBoehmGC
else
endif

ifeq ($(strip ${WITH_TYPED_GC}),)
else
CFLAGS := ${CFLAGS} "-D withBoehmGC_P"
endif

CFLAGS := ${CFLAGS} -pthread -fsigned-char

CFLAGS += ${shell_cflags}
LDFLAGS += ${shell_ldflags}

OBJS = siphash.o utf8.o memory.o util.o base.o collections.o file.o system.o\
		lexer.o lexer_plug.o parser.o walker.o marshal.o reflect.o fiber.o\
		importlib.o coroutine.o symbol.o generator.o base_extensions.o citron.o\
		promise.o symbol_cxx.o world.o

ifneq ($(findstring withCTypesNative=1,${CFLAGS}),)
OBJS := ${OBJS} _struct.o ctypes.o structmember.o
endif
ifneq ($(findstring withInjectNative=1,${CFLAGS}),)
OBJS := ${OBJS} tcc/libtcc1.a tcc/libtcc.a inject.o
endif

COBJS = ${OBJS} compiler.o

# .SUFFIXES:	.o .c

deps:
	# This is just a hacky way of getting bdwgc "libgc" from MSYS2,
	# Do not expect it to work anywhere else
	# TODO: Find a better solution
	pacman --noconfirm -S libgc-devel libgc
	# These guys are dynamically linked
	cp /usr/bin/msys-gc-1.dll .
	cp /usr/bin/msys-2.0.dll .
	cp /usr/bin/msys-gcc_s-seh-1.dll .

all: CFALGS := $(CFLAGS) -O2
all: deps cxx
all: ctr ctrconfig

ctrconfig:
	$(CC) ctrconfig.c -o ctrconfig

debug: CFLAGS := ${CFLAGS} -DDEBUG_BUILD -DDEBUG_BUILD_VERSION=${DEBUG_BUILD_VERSION} -Og -g3 -ggdb3 -Wno-unused-function
debug: deps
debug: cxx
debug: ctr
debug:
	sed -i -e "1s/.*/${new_makefile_l1}/" makefile

install:
	echo -e "install directly from source not allowed.\nUse citron_autohell instead for installs"
	exit 1;
ctr: $(OBJS) modules
	$(CC) -fopenmp ${LDFLAGS} $(OBJS) modules.o -static -lm -ldl -lpcre -lpthread /usr/lib/libgc.dll.a ${LEXTRACF} -o ctr

libctr: CFLAGS := $(CFLAGS) -fPIC -DCITRON_LIBRARY
libctr: deps
libctr: symbol_cxx
libctr: $(OBJS)
	$(CC) $(OBJS) -shared -export-dynamic -ldl -lpcre -lpthread /usr/lib/libgc.dll.a -o libctr.so

compiler: CFLAGS := $(CFLAGS) -D comp=1
compiler: deps
compiler: cxx
compiler: $(COBJS)
	$(CC) $(COBJS) -rdynamic -lm -ldl -lpcre -lpthread /usr/lib/libgc.dll.a ${LEXTRACF} -o ctrc
cxx:
	echo "blah"

tcc/%.a:
	cd tcc && ./configure --prefix=`realpath build` && $(MAKE) $<

# %.o: %.c
# 	echo "$<"
# 	$(CC) $(CFLAHS) -c $< -o $@ >/dev/null 2>&1

%.o: %.c
	$(CC) -fopenmp $(CFLAGS) -static -c $< -o $@


define SHVAL =
for f in *.c; do\
	echo "Formatting $f";\
	indent -l150 -hnl -bbo $f;\
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

cclean:
	rm -rf ${COBJS} ctrc

love:
	echo "Not war?"

war:
	echo "Not love?"

modules:
	# tcl
	pacman -S --noconfirm mingw64/mingw-w64-x86_64-tcl mingw64/mingw-w64-x86_64-tk
	# fuckin windows shit
	# create a symlink so ld will be happy
	ln -s /mingw64/lib/libtcl8.6.dll.a /usr/lib/libtcl8.6.a
	$(CC) -fopenmp $(CFLAGS) -static -c modules.c -o modules.o
	cp /mingw64/bin/tcl86.dll tcl86.dll
	cp /mingw64/bin/zlib1.dll zlib1.dll
	
package:
	# create a package for windows people
	# really, how lazy can y'all get?
	mkdir -p package/prepared/basemods
	cp misc/citron_windows.bat package/prepared/citron.bat
	cp -r image examples docs extensions Library *.dll ctr.exe eval compile.ctr package/prepared
	cp dist_windows package/prepared/ctr.bat
	cp misc/prepare_install.ctr package/prepare_install.ctr
	cp misc/install.bat install.bat
	tar cf citron-release.tar package install.bat
	rm -rf package install.bat

distribute: all package
