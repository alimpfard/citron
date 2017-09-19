CFLAGS = -mtune=native -g3 -D forLinux
OBJS = siphash.o utf8.o memory.o util.o base.o collections.o file.o system.o \
       world.o lexer.o parser.o walker.o reflect.o citron.o

.SUFFIXES:	.o .c

all:ctr

install: ctr
	cp ./ctr /usr/bin/ctr

ctr:	$(OBJS)
	$(CC) $(OBJS) -rdynamic -lm -ldl -lbsd -o ctr

libctr:
	CFLAGS += -fPIC
	$(OBJS)
	$(CC) $(OBJS) -shared -fPIC -ldl -lbsd -o libctr.so

.c.o:
	$(CC) $(CFLAGS) -c $<

clean:
	rm -rf ${OBJS} ctr
