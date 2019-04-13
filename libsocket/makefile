all:
	mkdir libsocket/build
	cd libsocket/build && cmake .. && make
	cp -r libsocket/build/C/libsocket.so .
	cp -r libsocket/build/C/libsocket.so $(realpath .)/..
clean:
	rm -rf libsocket/build
	rm -rf libsocket.so
install:
	make -C libsocket/build install

uninstall:
	# make -C libsocket/build uninstall

distclean:
	make -C libsocket/build distclean || true
