default_target: all

all: libjansson ctrjson

libjansson: 
	cd jansson && make
ctrjson: 
	cc -c json.c -fPIC -g3 -o json.o
	cp jansson/lib/libjansson.a .
	cc -shared -o libctrjson.so json.o libjansson.a
	mkdir -p ../../mods/json
	cp -f libctrjson.so ../../mods/json

clearctr: 
	rm -f *.o *.so *.a
clearlibjansson: 
	cd jansson && make clear
