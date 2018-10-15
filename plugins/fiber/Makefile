INTO ?= ../../mods/fiber

all: comp link copy

comp:
	cc -c ctr_fiber.c -Wall -Werror -fPIC -g3 -o ctf.o
	#cc -c libfiber-uc.c -fPIC -g3 -o libfiber.o

link:
	cc -shared -o libctrfiber.so ctf.o

copy:
	mkdir -p $(INTO)
	cp -f libctrfiber.so $(INTO)

clean:
	rm libctrfiber.so ctf.o

default: all
