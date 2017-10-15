default: all

all: compile link copy

compile:
	cc -c gmp.c -Wall -Werror -fPIC -g3 -o gmp.o

link:
	cc -shared -o libctrbiginteger.so -lgmp gmp.o

copy: 
	mkdir -p ../../mods/biginteger
	cp -f libctrbiginteger.so ../../mods/biginteger
