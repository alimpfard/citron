DEBUG_VERSION := 1603
DEBUG_BUILD_VERSION := "\"$(DEBUG_VERSION)\""
location = $(CURDIR)/$(word $(words $(MAKEFILE_LIST)),$(MAKEFILE_LIST))
new_makefile_l1 := $(shell perl -ne '/((DEBUG_VERSION := )(\d+))/ && print (sprintf("%s%s", "$$2", "$$3"+1));' $(location))
LIBSOCKETDIR = src/lib/libsocket
BUILDDIR ?= build
DESTDIR ?= /
BINDIR ?= $(DESTDIR)/usr/local/bin
DATADIR ?= $(DESTDIR)/usr/share/Citron
HEADERDIR ?= $(DESTDIR)/usr/local/include/Citron
DLLDIR ?= $(DESTDIR)/usr/local/lib
INCLUDE_DIR ?= $(DESTDIR)/usr/local/include/Citron

.PHONY: all ctrconfig libctr ctr install cxx

enable_inject ?= true
enable_ctypes ?= true
enable_inline_asm ?= false
enable_boehm_gc ?= true
use_libbsd ?= true

CFLAGS += -Wall -Wextra -Wno-unused-parameter\
		  -D withTermios -D CTR_STD_EXTENSION_PATH=\"$(DATADIR)\"

LIBS = -lm -ldl -lpcre -pthread -lffi -lc

ifeq ($(enable_inject),true)
	CFLAGS += -DwithInjectNative=1
endif

ifeq ($(enable_ctypes),true)
	CFLAGS += -DwithCTypesNative=1
endif

ifeq ($(enable_inline_asm),true)
	CFLAGS += -DwithInlineAsm=1
endif

ifeq ($(use_libbsd),true)
	LIBS += -lbsd
endif

ifneq ($(strip $(WITH_ICU)),)
	CFLAGS += -D withICU
	LEXTRACF += -L/usr/lib -licui18n -licuuc -licudata
endif

ifeq ($(enable_boehm_gc),true)
	fv := $(strip $(shell ldconfig -p | grep libgc.so | cut -d ">" -f2 | head -n1))
	CFLAGS += -D withBoehmGC
	LEXTRACF += $(fv)
endif

CFLAGS += -Isrc/lib/
LEXTRACF += $(LDFLAGS) -flto -lstdc++

OBJS = siphash.o utf8.o memory.o util.o base.o collections.o file.o system.o \
	   lexer.o lexer_plug.o parser.o walker.o marshal.o reflect.o fiber.o \
	   importlib.o coroutine.o symbol.o generator.o base_extensions.o citron.o \
	   promise.o symbol_cxx.o world.o libsocket.so
EXTRAOBJS =
TCC_STATICS = 

ifneq ($(findstring withInjectNative=1,$(CFLAGS)),)
	OBJS += inject.o libtcc1.a libtcc.a
	CFLAGS += -DwithCTypesNative=1
	TCC_STATICS = build_tcc_statics
endif

ifneq ($(findstring withCTypesNative=1,$(CFLAGS)),)
	OBJS += _struct.o ctypes.o structmember.o
endif

ifneq ($(findstring withInlineAsm=1,$(CFLAGS)),)
	EXTRAOBJS += inline-asm.o
	CFLAGS += $(shell llvm-config --cflags --system-libs --libs core orcjit native)
	CXXFLAGS += $(shell llvm-config --cxxflags --system-libs --libs core orcjit native) -fexceptions
endif

COBJS = $(OBJS) compiler.o

# FIXME: maybe a bit hackish?
OBJS := $(foreach obj,$(OBJS),$(BUILDDIR)/$(obj))
COBJS := $(foreach obj,$(COBJS),$(BUILDDIR)/$(obj))
EXTRAOBJS := $(foreach obj,$(EXTRAOBJS),$(BUILDDIR)/$(obj))
CFLAGS += -O3 -fpic

all: remove_libsocket_build cxx ctr ctrconfig

remove_libsocket_build:
	@ rm -rf src/lib/libsocket/libsocket/build/

ctrconfig:
	$(CC) src/ctrconfig.c -o $(BUILDDIR)/ctrconfig

$(BUILDDIR):
	mkdir -p $@

$(BUILDDIR)/ctr: $(TCC_STATICS) $(BUILDDIR) $(OBJS) $(EXTRAOBJS)
	$(CXX) -fopenmp $(EXTRAOBJS) $(OBJS) $(CXXFLAGS) $(CFLAGS) $(LIBS) $(LEXTRACF) -o $@

ctr: $(BUILDDIR)/ctr

$(BUILDDIR)/libctr.so: CFLAGS += -DCITRON_LIBRARY
$(BUILDDIR)/libctr.so: $(OBJS)
	$(CC) $(OBJS) -shared -export-dynamic $(LIBS) -o $@

libctr: $(BUILDDIR)/libctr.so

# compiler: CFLAGS := $(CFLAGS) -D comp=1
# compiler: cxx
# compiler: $(COBJS)
# 	cd $(BUILDDIR)
# 	$(CC) $< $(LIBS) -lprofiler $(LEXTRACF) -o ctrc
# 	cd -

install: $(BUILDDIR)/ctr $(BUILDDIR)/libctr.so $(BUILDDIR)/ctrconfig Library mods extensions
	mkdir -p $(BINDIR) $(DATADIR) $(HEADERDIR) $(DLLDIR)
	install $(BUILDDIR)/ctr $(BINDIR)
	install $(BUILDDIR)/ctrconfig $(BINDIR)
	install $(BUILDDIR)/libctr.so $(DLLDIR)
	install -m a+rx eval $(BINDIR)/citron
	cp -fr Library/ $(DATADIR)/
	cp -fr extensions/ $(DATADIR)/
	cp -fr  mods/ $(DATADIR)/
	install src/*.h $(HEADERDIR)

debug: CFLAGS += -DDEBUG_BUILD -DDEBUG_BUILD_VERSION=$(DEBUG_BUILD_VERSION) -Og -g3 -ggdb3 -Wno-unused-function
debug: cxx ctr
	sed -i -e "1s/.*/$(new_makefile_l1)/" makefile

clean:
	rm -rf $(BUILDDIR)
	$(MAKE) -C $(LIBSOCKETDIR) clean
	$(MAKE) -C src/lib/tcc clean

cxx:
	@ echo -e "\033[31;1mblah \033[32;1mblah \033[33;1mblah\033[0m"

build_tcc_statics:
	pushd src/lib/tcc
	./configure
	popd
	$(MAKE) -C src/lib/tcc

$(BUILDDIR)/%.a: src/lib/tcc/%.a
	cp $< $@

$(BUILDDIR)/libsocket.so:
	make -C $(LIBSOCKETDIR)
	cp $(LIBSOCKETDIR)/libsocket.so $(BUILDDIR)/libsocket.so

$(BUILDDIR)/%.o: src/%.c
	$(CC) -fopenmp $(CFLAGS) -c $< -o $@

$(BUILDDIR)/%.o: src/%.cpp
	$(CXX) -g $(CFLAGS) -c $< $(CXXFLAGS) -o $@

define SHVAL =
for f in src/*.c; do\
	echo "Formatting $f";\
	cp "$f" ".bac/$f";\
	clang-format -i "$f";\
done
endef

format:
	mkdir -p .bac
	rm -rf .bac/*.c
	cp src/*.c .bac/
	$(value SHVAL)
	rm -rf src/*.c~

.ONESHELL:

unback:
	cp .bac/*.c src

love:
	@ echo "Not war?"

war:
	@ echo "Not love?"
