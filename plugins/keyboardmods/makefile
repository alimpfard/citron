INTO ?= ../../basemods/keyboardmods

default: all

all: compile link copy

compile:
	cc ${EXTRAS} -c keyboardmods.c -Wall -fPIC -g3 -Wno-unused-function -Wno-format -o keyboardmods.o
link:
	cc -shared -o libctrkeyboardmods.so keyboardmods.o

copy:
	mkdir -p $(INTO)
	cp -f libctrkeyboardmods.so $(INTO)
