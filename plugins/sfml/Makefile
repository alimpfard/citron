comp: 
	cc -c sfml.c -Wall -Werror -fPIC -o sfml.o
link: 
	cc -shared -o libctrsfml.so sfml.o -lcsfml-graphics -lcsfml-window -lcsfml-system
copy: 
	cp -f libctrsfml.so ../../mods
clean:
	rm -f *.o *.so
all: comp link copy
debug: g3comp g3link copy
default: all

