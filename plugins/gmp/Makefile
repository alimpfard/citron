INTO ?= ../../mods/biginteger

default: all

all: compile link copy

compile:
	${CC} ${EXTRAS} -c gmp.c -Wall -fPIC -g3 -o gmp.o

link:
	${CC} -shared -o libctrbiginteger.so -lgmp gmp.o -lcitron

copy:
	mkdir -p $(INTO)
	cp -f libctrbiginteger.so $(INTO)
