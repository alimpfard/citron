all: comp link copy

comp:
	cc -c ctypes.c -Wall -Werror -fPIC -g3 -o ct.o

link:
	cc -shared -o libctrctypes.so ct.o

copy:
	cp -f libctrctypes.so ../../mods/ctypes

clean:
	rm libctrctypes.so ct.o

default: all
