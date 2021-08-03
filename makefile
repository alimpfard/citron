DEBUG_VERSION := 1603
DEBUG_BUILD_VERSION := "\"$(DEBUG_VERSION)\""
fv := $(strip $(shell ldconfig -p | grep libgc.so | cut -d ">" -f2 | head -n1))
location = $(CURDIR)/$(word $(words $(MAKEFILE_LIST)),$(MAKEFILE_LIST))
new_makefile_l1 := $(shell perl -ne '/((DEBUG_VERSION := )(\d+))/ && print (sprintf("%s%s", "$$2", "$$3"+1));' $(location))
LIBSOCKETDIR = src/lib/libsocket
BUILDDIR ?= build
DESTDIR ?= /usr/local/bin
LIBDIR ?= /usr/local/lib

.PHONY: all ctrconfig libctr ctr install cxx

CFLAGS += -Wall -Wextra -Wno-unused-parameter -mtune=native\
		  -march=native -D withTermios -D forLinux\
		  -D CTR_STD_EXTENSION_PATH=\"`pwd`\"

ifneq ($(strip $(WITH_ICU)),)
	CFLAGS += -D withICU
	LEXTRACF += -L/usr/lib -licui18n -licuuc -licudata
endif

ifeq ($(strip $(WITHOUT_BOEHM_GC)),)
	CFLAGS += "-D withBoehmGC"
	LEXTRACF += $(fv)
endif

ifneq ($(strip $(WITH_TYPED_GC)),)
	CFLAGS += "-D withBoehmGC_P"
endif

CFLAGS += -Isrc/lib/
LEXTRACF += $(LDFLAGS) -flto -lstdc++

OBJS = siphash.o utf8.o memory.o util.o base.o collections.o file.o system.o \
	   lexer.o lexer_plug.o parser.o walker.o marshal.o reflect.o fiber.o \
	   importlib.o coroutine.o symbol.o generator.o base_extensions.o citron.o \
	   promise.o symbol_cxx.o world.o libsocket.so
EXTRAOBJS =

ifneq ($(findstring withCTypesNative=1,$(CFLAGS)),)
	OBJS += _struct.o ctypes.o structmember.o
endif

ifneq ($(findstring withInlineAsm=1,$(CFLAGS)),)
	EXTRAOBJS += inline-asm.o
	CFLAGS += $(shell llvm-config --cflags --system-libs --libs core orcjit native)
	CXXFLAGS += $(shell llvm-config --cxxflags --system-libs --libs core orcjit native) -fexceptions
endif

ifneq ($(findstring withInjectNative=1,$(CFLAGS)),)
	OBJS += inject.o tcc/libtcc1.a tcc/libtcc.a
endif

COBJS = $(OBJS) compiler.o

# FIXME: maybe a bit hackish?
OBJS := $(foreach obj,$(OBJS),$(BUILDDIR)/$(obj))
COBJS := $(foreach obj,$(COBJS),$(BUILDDIR)/$(obj))
EXTRAOBJS := $(foreach obj,$(EXTRAOBJS),$(BUILDDIR)/$(obj))

all: CFLAGS += -O2
all: cxx ctr ctrconfig

ctrconfig:
	$(CC) src/ctrconfig.c -o $(BUILDDIR)/ctrconfig

$(BUILDDIR):
	mkdir -p $@

$(BUILDDIR)/ctr: $(BUILDDIR) $(OBJS) $(EXTRAOBJS)
	$(CXX) -fopenmp $(EXTRAOBJS) $(OBJS) $(CXXFLAGS) -rdynamic -lm -ldl -lbsd -lpcre -lffi -lpthread $(LEXTRACF) -o $@

ctr: $(BUILDDIR)/ctr

$(BUILDDIR)/libctr.so: CFLAGS += -fPIC -DCITRON_LIBRARY
$(BUILDDIR)/libctr.so: $(OBJS)
	$(CC) $< -shared -export-dynamic -ldl -lbsd -lpcre -lffi -lpthread -o $@

libctr: $(BUILDDIR)/libctr.so

# compiler: CFLAGS := $(CFLAGS) -D comp=1
# compiler: cxx
# compiler: $(COBJS)
# 	cd $(BUILDDIR)
# 	$(CC) $< -rdynamic -lm -ldl -lbsd -lpcre -lffi -lprofiler -lpthread $(LEXTRACF) -o ctrc
# 	cd -

install: $(BUILDDIR)/ctr $(BUILDDIR)/libctr.so $(BUILDDIR)/ctrconfig
	sudo install $(BUILDDIR)/ctr $(DESTDIR)
	sudo install $(BUILDDIR)/libctr.so $(LIBDIR)
	sudo install $(BUILDDIR)/ctrconfig $(DESTDIR)

debug: CFLAGS += -DDEBUG_BUILD -DDEBUG_BUILD_VERSION=$(DEBUG_BUILD_VERSION) -Og -g3 -ggdb3 -Wno-unused-function
debug: cxx ctr
	sed -i -e "1s/.*/$(new_makefile_l1)/" makefile

clean:
	rm -rf $(BUILDDIR)
	make -C $(LIBSOCKETDIR) clean

cxx:
	@ echo "\033[31;1mblah \033[32;1mblah \033[33;1mblah\033[0m"

src/lib/tcc/%.a:
	cd src/lib/tcc
	./configure --prefix=$(realpath build)
	$(MAKE) $<

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
