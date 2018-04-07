default: all

all: compile link copy

compile:
	cc -c keyboardmods.c  -I/usr/local/include -Wall -fPIC -g3 -Wno-unused-function -Wno-format -o keyboardmods.o
link:
	cc -shared -o libctrkeyboardmods.so -L/usr/local/lib keyboardmods.o

copy:
	mkdir -p ../../basemods/keyboardmods
	cp -f libctrkeyboardmods.so ../../basemods/keyboardmods
