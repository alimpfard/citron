LEXTRACF := ${LEXTRACF} -flto 
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
	LEXTRACF := ${LEXTRACF} /usr/lib/libgc.so
else
endif

ifeq ($(strip ${WITH_TYPED_GC}),)
else
	CFLAGS := ${CFLAGS} "-D withBoehmGC_P"
endif

OBJS = siphash.o utf8.o memory.o util.o base.o collections.o file.o system.o \
       world.o lexer.o lexer_plug.o parser.o walker.o marshal.o reflect.o fiber.o\
	   importlib.o coroutine.o symbol.o generator.o base_extensions.o citron.o

COBJS = ${OBJS} compiler.o

.SUFFIXES:	.o .c

all: CFALGS := $(CFLAGS) -O2
all: ctr

debug: CFLAGS := ${CFLAGS} -DDEBUG_BUILD -Og -g3 -ggdb3 -Wno-unused-function
	   #LEXTRACF := -lefence ${LEXTRACF}
debug: ctr

install:
	echo -e "install directly from source not allowed.\nUse citron_autohell instead for installs"
	exit 1;
ctr:	$(OBJS)
	$(CC) -fopenmp $(OBJS) -rdynamic -lm -ldl -lbsd -lpcre -lpthread ${LEXTRACF} -o ctr

libctr: CFLAGS := $(CFLAGS) -fPIC -DCITRON_LIBRARY
libctr: $(OBJS)
	$(CC) $(OBJS) -shared -export-dynamic -ldl -lbsd -lpcre -lpthread -o libctr.so

compiler: CFLAGS := $(CFLAGS) -D comp=1
compiler: $(COBJS)
	$(CC) $(COBJS) -rdynamic -lm -ldl -lbsd -lpcre -lprofiler -lpthread ${LEXTRACF} -o ctrc
.c.o:
	$(CC) -fopenmp $(CFLAGS) -c $<

clean:
	rm -rf ${OBJS} ctr

cclean:
	rm -rf ${COBJS} ctrc

love:
	echo "Not war?"

war:
	echo "Not love?"

.phony:

