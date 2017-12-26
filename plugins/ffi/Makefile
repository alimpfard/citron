default: all

all: compile link copy

compile:
	cc -c ctypes.c  -I/usr/local/include -Wall -fPIC -g3 -Wno-unused-function -Wno-format -o ctypes.o
	cc -c _struct.c -I/usr/local/include -Wall -fPIC -g3 -Wno-unused-function -Wno-format -o _struct.o
	cc -c structmember.c -I/usr/local/include -Wall -fPIC -g3 -Wno-unused-function -Wno-format -o structmember.o
link:
	cc -shared -o libctrctypes.so -L/usr/local/lib -lffi ctypes.o structmember.o _struct.o

copy:
	cp -f libctrctypes.so ../../mods/ctypes
