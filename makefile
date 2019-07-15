DEBUG_VERSION := 1223
DEBUG_BUILD_VERSION := "\"$(DEBUG_VERSION)\""
o_cflags := -I/mingw64/include
#it ain't so easy
#$(shell pkg-config --cflags tcl)
o_ldflags := -L/mingw64/lib -ltcl8.6 -lz -lpthread -lz
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
LEXTRACF := ${LDFLAGS} ${LEXTRACF} -flto -lstdc++

OBJS = siphash.o utf8.o memory.o util.o base.o collections.o file.o system.o\
		lexer.o lexer_plug.o parser.o walker.o marshal.o reflect.o fiber.o\
		importlib.o coroutine.o symbol.o generator.o base_extensions.o citron.o\
		promise.o symbol_cxx.o modules.o world.o lambdaf.a
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
OUTF = citron-release.tar
TARGET = 64

# .SUFFIXES:	.o .c

deps:
	# This is just a hacky way of getting bdwgc "libgc" from MSYS2,
	# Do not expect it to work anywhere else
	# TODO: Find a better solution
	pacman --noconfirm -S libgc-devel libgc mingw-w64-i686-dlfcn mingw-w64-x86_64-dlfcn mingw-w64-x86_64-pcre mingw-w64-i686-pcre git

all: CFALGS := $(CFLAGS) -O2
all: deps modules cxx
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
ctr:	$(OBJS) $(EXTRAOBJS)
	$(CXX) -fopenmp $(EXTRAOBJS) $(OBJS) ${CXXFLAGS}  -rdynamic -lm -lpthread /usr/lib/libgc.dll.a /usr/lib/libdl.a /usr/lib/libpcre.a ${LEXTRACF} -o ctr

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

lambdaf.a:
	./make-lambdaf.sh

libsocket.so:
	make -C libsocket
	cp libsocket/libsocket.so libsocket.so

inline-asm.o:
	$(CXX) -g $(CFLAGS) -c inline-asm.cpp ${CXXFLAGS} -o inline-asm.o

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
	./make-lambdaf.sh clean
	make -C libsocket clean
	# tell me what you will, but why not?
	git add citron-release.tar || echo "nothing was built (x86_64)"
	git clean -xffd

cclean:
	rm -rf ${COBJS} ctrc

love:
	echo "Not war?"

war:
	echo "Not love?"

modules:
	# tcl
	pacman -S --overwrite '*' --noconfirm mingw64/mingw-w64-x86_64-tcl mingw64/mingw-w64-x86_64-tk
	# fuckin windows shit
	# create a symlink so ld will be happy
	mkdir -p /usr/lib/
	# assumptions are bad, apparently
	ln -s `pacman -Ql mingw-w64-x86_64-tcl | grep 'libtcl86.dll.a' | cut -d' ' -f2` /usr/lib/libtcl8.6.a
	#grab lib/tcl86 and tk
	mkdir -p lib
	cp -r `pacman -Ql mingw-w64-x86_64-tcl | grep 'lib/tcl8.6/$$' | cut -d' ' -f2` lib
	cp -r `pacman -Ql mingw-w64-x86_64-tk | grep 'lib/tk8.6/$$' | cut -d' ' -f2` lib
	$(CC) -fopenmp $(CFLAGS) -static -c modules.c -o modules.o
	cp /mingw64/bin/tcl86.dll tcl86.dll
	mkdir bin
	cp /mingw64/bin/tk86.dll bin/tk86.dll
	cp /mingw64/bin/zlib1.dll zlib1.dll
	# TODO: the rest of the modules

package:
	ls -l /usr/lib
	ldd ./ctr
	for dep in `ldd ./ctr | grep '=>' | cut -d' ' -f3 | xargs realpath`; do \
		printf '%s ... ' "Resolving dependancy '$$dep'" ; \
		if [[ $$dep == *msys* ]]; then \
			cp $$dep . && echo "Check" || echo "Failed" ; \
		else \
			echo "Not useful" ; \
		fi ; \
	done
	# create a package for windows people
	# really, how lazy can y'all get?
	mkdir -p package/prepared/basemods
	cp misc/citron_windows.bat package/prepared/citron.bat
	cp -r image examples docs extensions Library lib bin *.dll ctr.exe eval compile.ctr package/prepared
	cp dist_windows package/prepared/ctr.bat
	cp misc/prepare_install.ctr package/prepare_install.ctr
	cp misc/install.bat install.bat
	tar cf $(OUTF) package install.bat
	rm -rf package install.bat

release:
	  wget -c https://github.com/probonopd/uploadtool/raw/master/upload.sh
		export UPLOADTOOL_SUFFIX="windows"
		export REPO_SLUG="alimpfard/citron"
	  if [ "x$$BUILD_I686" = "xYES" ]; then \
	  	./upload.sh citron-i686-release.tar citron-release.tar; \
	  else \
	  	./upload.sh citron-release.tar; \
	  fi


distribute: all package clean
distribute:
	if [ "x$$BUILD_I686" = "xYES" ]; then \
		PATH="$$PATH:/mingw32/bin" make -f makefile.32 distribute; \
	fi
	make -f makefile release
