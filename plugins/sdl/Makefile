all: comp link copy

comp:
	cc -c sdl.c -Wall -D CTR_STD_EXTENSION_PATH='"/usr/local/share/Citron"' -fPIC -g3 -o sdl.o `sdl-config --cflags --libs` -lSDL_image -lSDL_gfx -lSDL_ttf

link:
	cc -shared -o libctrsdl.so sdl.o `sdl-config --cflags --libs` -lSDL_image -lSDL_gfx -lSDL_ttf

copy:
	mkdir -p ../../mods/sdl
	cp -f libctrsdl.so ../../mods/sdl

clean:
	rm libctrsdl.so sdl.o

default: all
