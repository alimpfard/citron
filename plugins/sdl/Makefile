all: comp link copy

comp:
	cc -c sdl.c -Wall -D CTR_STD_EXTENSION_PATH='"/usr/local/share/Citron"' -fPIC -g3 -o sdl.o `sdl-config --cflags --libs`

link:
	cc -shared -o libctrsdl.so sdl.o `sdl-config --cflags --libs`

copy:
	cp -f libctrsdl.so ../../mods/sdl

clean:
	rm libctrsdl.so sdl.o

default: all
