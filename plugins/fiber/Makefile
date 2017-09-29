all: comp link copy

comp:
	cc -c ctr_fiber.c -Wall -Werror -fPIC -g3 -o ctf.o
	#cc -c libfiber-uc.c -fPIC -g3 -o libfiber.o

link:
	cc -shared -o libctrfiber.so ctf.o

copy:
	cp -f libctrfiber.so ../../mods/fiber

clean:
	rm libctrfiber.so ctf.o

default: all
