default_target: all

all: libjansson ctrjson

libjansson: 
	cd jansson && make
ctrjson: 
	cc -c json.c -Wall -Werror -fPIC -g3 -o json.o
	cp jansson/lib/libjansson.a .
	cc -shared -o libctrjson.so json.o libjansson.a
	cp -f libctrjson.so ../../mods/json

clearctr: 
	rm -f *.o *.so *.a
clearlibjansson: 
	cd jansson && make clear
