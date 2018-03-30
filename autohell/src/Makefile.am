AM_CFLAGS = --pedantic -O2 -Wall -mtune=native -D withTermios -D forLinux -DCTR_STD_EXTENSION_PATH=\"$(pkgdatadir)\"
AM_LDFLAGS = -rdynamic -lm -ldl -lbsd -lpcre -lpthread -o ctr

bin_PROGRAMS = ctr
ctr_SOURCES = citron.c base.c world.c importlib.c fiber.c reflect.c\
			  walker.c parser.c lexer.c lexer_plug.c system.c file.c\
			  collections.c util.c memory.c utf8.c symbol.c siphash.c\
			  coroutine.c generator.c base_extensions.c dictionary.h\
			  siphash.h marshal.c
ctr_CFLAGS = $(AM_CFLAGS)

pkginclude_HEADERS = citron.h citron_conv.h citron_ensure.h coroutine.h dictionary.h\
			  siphash.h generator.h symbol.h

lib_LTLIBRARIES = libcitron.la
libcitron_la_SOURCES = citron.c base.c world.c importlib.c fiber.c reflect.c\
               		   walker.c parser.c lexer.c lexer_plug.c system.c file.c\
               		   collections.c util.c memory.c utf8.c symbol.c siphash.c\
                       coroutine.c generator.c base_extensions.c dictionary.h\
					   siphash.h marshal.c

