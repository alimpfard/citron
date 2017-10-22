CFLAGS = -Wall -Wextra -Wno-unused-parameter -mtune=native -g3 -D forLinux
OBJS = siphash.o utf8.o memory.o util.o base.o collections.o file.o system.o \
       world.o lexer.o parser.o walker.o reflect.o fiber.o citron.o

LOBJS = siphash.o utf8.o memory.o util.o base.o collections.o file.o system.o \
        world.o lexer.o parser.o walker.o reflect.o fiber.o citron_lib.o

.SUFFIXES:	.o .c

all:ctr

install: ctr
	cp ./ctr /usr/bin/ctr

ctr:	$(OBJS)
	$(CC) $(OBJS) -rdynamic -lm -ldl -lbsd -lpcre -o ctr

libctr: CFLAGS := $(CFLAGS) -fPIC -D CTR_STD_EXTENSION_PATH='".."'
libctr: $(OBJS)
	$(CC) $(OBJS) -shared -export-dynamic -fPIC -ldl -lbsd -lpcre -o python/libctr.so

.c.o:
	$(CC) $(CFLAGS) -c $<

clean:
	rm -rf ${OBJS} ctr
