default: all

all: compile link copy

compile:
	cc ${EXTRAS} -c ctypes.c  -Wall -fPIC -g3 -Wno-unused-function -Wno-format -o ctypes.o
	cc ${EXTRAS} -c _struct.c -Wall -fPIC -g3 -Wno-unused-function -Wno-format -o _struct.o
	cc ${EXTRAS} -c structmember.c -Wall -fPIC -g3 -Wno-unused-function -Wno-format -o structmember.o
link:
	cc -shared -o libctrctypes.so -l:libffi.so.6 -lm ctypes.o structmember.o _struct.o

copy:
	mkdir -p ../../mods/ctypes
	cp -f libctrctypes.so ../../mods/ctypes
