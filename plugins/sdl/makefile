all: clean compile copy

clean:
	rm -f *.o *.so

compile:
	${CC} ${EXTRAS} -shared -fPIC -lSDL_gfx -lSDL_ttf -lSDL_image `sdl-config --libs --cflags` sdl.c -o sdl.so

copy:
	rm -f ../../mods/sdl/libctrsdl.so
	cp sdl.so ../../mods/sdl/libctrsdl.so
