default: all

all: compile link copy

compile:
	cc -c ctypes.c -I/lib/libffi-3.2.1/include -Wall -Werror -fPIC -g3 -o ctypes.o

link:
	cc -shared -o libctrctypes.so -lffi ctypes.o

copy: 
	cp -f libctrctypes.so ../../mods/ctypes
