INTO ?= ../../mods/python

default: all

all: compile copy

compile:
	${CC} ${EXTRAS} `python-config --libs --cflags` -rdynamic -fpic -shared python.c -o libctrpython.so

copy:
	mkdir -p $(INTO)
	cp -f libctrpython.so $(INTO)
