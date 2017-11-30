default: all

all: compile link copy

compile:
	cc -c ctypes.c  -I/lib/libffi-3.2.1/include -Wall -fPIC -g3 -Wno-unused-function -Wno-format -o ctypes.o
	cc -c _struct.c -I/lib/libffi-3.2.1/include -Wall -fPIC -g3 -Wno-unused-function -Wno-format -o _struct.o
	cc -c structmember.c -I/lib/libffi-3.2.1/include -Wall -fPIC -g3 -Wno-unused-function -Wno-format -o structmember.o
link:
	cc -shared -o libctrctypes.so -lffi ctypes.o structmember.o _struct.o

copy: 
	cp -f libctrctypes.so ../../mods/ctypes
