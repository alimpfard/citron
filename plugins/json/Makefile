default_target: all

all: libjansson ctrjson

libjansson:
	cd jansson && cmake -DJANSSON_BUILD_SHARED_LIBS=1 . && make && make install
ctrjson:
	cc ${EXTRAS} -c json.c -fPIC -g3 -Ijansson/include -o json.o
	cp jansson/lib/libjansson.so .
	cc -shared -o libctrjson.so json.o libjansson.so
	mkdir -p ../../mods/json
	cp -f libctrjson.so ../../mods/json

clearctr:
	rm -f *.o *.so *.a
clearlibjansson:
	cd jansson && make clear
