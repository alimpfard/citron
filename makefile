CFLAGS = -Wall -Wextra -Wno-unused-parameter -mtune=native -march=native -g3 -O0 -D withTermios -D forLinux -D CTR_STD_EXTENSION_PATH=\"`pwd`\"
OBJS = siphash.o utf8.o memory.o util.o base.o collections.o file.o system.o \
       world.o lexer.o parser.o walker.o reflect.o fiber.o importlib.o\
	   coroutine.o base_extensions.o citron.o

LOBJS = siphash.o utf8.o memory.o util.o base.o collections.o file.o system.o \
        world.o lexer.o parser.o walker.o reflect.o fiber.o importlib.o citron_lib.o

COBJS = ${OBJS} compiler.o

.SUFFIXES:	.o .c

all: CFALGS := $(CFLAGS) -O3
all:ctr

install: 
	echo -e "install directly from source not allowed.\nUse citron_autohell instead for installs"
	exit 1;
ctr:	$(OBJS)
	$(CC) $(OBJS) -rdynamic -lm -ldl -lbsd -lpcre -lpthread ${LEXTRACF} -o ctr

libctr: CFLAGS := $(CFLAGS) -fPIC -D CTR_STD_EXTENSION_PATH='".."'
libctr: $(OBJS)
	$(CC) $(OBJS) -shared -export-dynamic -fPIC -ldl -lbsd -lpcre -lpthread -o python/libctr.so

compiler: CFLAGS := $(CFLAGS) -D comp=1
compiler: $(COBJS)
	$(CC) $(COBJS) -rdynamic -lm -ldl -lbsd -lpcre -lprofiler -lpthread ${LEXTRACF} -o ctrc
.c.o:
	$(CC) $(CFLAGS) -c $<

clean:
	rm -rf ${OBJS} ctr

cclean:
	rm -rf ${COBJS} ctrc

love:
	echo "Not war?"

war:
	echo "Not love?"
