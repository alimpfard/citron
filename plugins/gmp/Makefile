default: all

all: compile link copy

compile:
	${CC} ${EXTRAS} -c gmp.c -Wall -fPIC -g3 -o gmp.o

link:
	${CC} -shared -o libctrbiginteger.so -lgmp gmp.o

copy:
	mkdir -p ../../mods/biginteger
	cp -f libctrbiginteger.so ../../mods/biginteger
