CFLAGS = -Wall -Wextra -Wno-unused-parameter -mtune=native -march=native -g3 -O1 -D withTermios -D forLinux -D CTR_STD_EXTENSION_PATH=\"`pwd`\"
OBJS = siphash.o utf8.o memory.o util.o base.o collections.o file.o system.o \
       world.o lexer.o parser.o walker.o reflect.o fiber.o importlib.o\
	   base_extensions.o citron.o

LOBJS = siphash.o utf8.o memory.o util.o base.o collections.o file.o system.o \
        world.o lexer.o parser.o walker.o reflect.o fiber.o importlib.o citron_lib.o

COBJS = siphash.o utf8.o memory.o util.o base.o collections.o file.o system.o \
        world.o lexer.o parser.o walker.o reflect.o fiber.o importlib.o\
		compilerutils.o compiler.o citron.o

.SUFFIXES:	.o .c

all: CFALGS := $(CFLAGS) -O3
all:ctr

install: ctr
	cp ./ctr /usr/bin/ctr

ctr:	$(OBJS)
	$(CC) $(OBJS) -rdynamic -lm -ldl -lbsd -lpcre -o ctr

libctr: CFLAGS := $(CFLAGS) -fPIC -D CTR_STD_EXTENSION_PATH='".."'
libctr: $(OBJS)
	$(CC) $(OBJS) -shared -export-dynamic -fPIC -ldl -lbsd -lpcre -o python/libctr.so

compiler: CFLAGS := $(CFLAGS) -D comp=1
compiler: $(COBJS)
	$(CC) $(COBJS) -rdynamic -lm -ldl -lbsd -lpcre -lprofiler -o ctrc
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
