default: all

all: compile copy

compile:
	${CC} ${EXTRAS} -I/software/tcl-7.4/include -I/software/tk-4.0/include -I/software/x11r5_dev/Include -L/software/tcl-7.4/lib -L/software/tk-4.0/lib -L/software/x11r5_dev/lib -ltk -ltcl -lX11 -lcitron -rdynamic -fpic -shared tcl.c -o libctrtcl.so

copy:
	mkdir -p ../../mods/tcl
	cp -f libctrtcl.so ../../mods/tcl
