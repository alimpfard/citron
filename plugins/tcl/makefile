INTO ?= ../../mods/tcl

default: all

all: compile copy

compile:
	${CC} tcl.c ${EXTRAS} `pkg-config --cflags --libs tcl` -rdynamic -fpic -shared -o libctrtcl.so

copy:
	mkdir -p $(INTO)
	cp -f libctrtcl.so $(INTO)
