all:
	mkdir libsocket/build
	cd libsocket/build && cmake .. -G "Unix Makefiles" && make
	cp -r libsocket/build/C/libsocket.so .
	cp -r libsocket/build/C/libsocket.so $(realpath .)/..
clean:
	rm -rf libsocket/build
	rm -rf libsocket.so
install:
	make -C libsocket/build install
