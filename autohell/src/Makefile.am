AM_CFLAGS = --pedantic -ggdb3 -Wall -fPIC -mtune=native -D withTermios -D forLinux -DCTR_STD_EXTENSION_PATH=\"$(pkgdatadir)\" -D WITH_AUTOMAKE=\"$(pkgdatadir)\" $(MAYBE_ASM_C)
AM_LDFLAGS = -rdynamic -lm -ldl -lbsd -lpcre -lpthread $(MAYBE_FFI_L) $(MAYBE_TCC_L) $(MAYBE_ASM_L) $(BUILD_DIR_L)/lambdaf.a -o ctr

SUBDIRS = $(MAYBE_TCC) lambdaf

AM_CXXFLAGS = -std=c++11 $(MAYBE_ASM_CXX) -fexceptions

bin_PROGRAMS = ctr ctrconfig
ctrconfig_SOURCES = ctrconfig.c
ctr_SOURCES = citron.c base.c world.c importlib.c fiber.c reflect.c\
			  walker.c parser.c lexer.c lexer_plug.c system.c file.c\
			  collections.c util.c memory.c utf8.c symbol.c siphash.c\
			  coroutine.c generator.c base_extensions.c dictionary.h\
			  siphash.h marshal.c promise.c symbol_cxx.cpp symbol.hpp\
			  promise.h native-asm.h

EXTRA_ctr_SOURCES = inject.c ctypes.c reverse_sizes.h\
			  _struct.h structmember.h ctypes.h _struct.c\
			  structmember.c inline-asm.cpp native-asm.h

ctrconfig_CFLAGS = $(AM_CFLAGS)
ctr_CFLAGS = $(AM_CFLAGS)
ctr_CXXFLAGS = $(AM_CXXFLAGS) $(MAYBE_ASM_CXX) -fexceptions

pkginclude_HEADERS = *.h

lib_LTLIBRARIES = libcitron.la
libcitron_la_CFLAGS= -DLIBRARY_BUILD $(AM_CFLAGS)
libcitron_la_LDFLAGS= -rdynamic -lm -ldl -lbsd -lpcre -lpthread $(MAYBE_FFI_L) $(MAYBE_TCC_L) $(MAYBE_ASM_L) -o ctr
libcitron_la_CXXFLAGS= $(AM_CXXFLAGS)

libcitron_la_SOURCES = citron.c base.c world.c importlib.c fiber.c reflect.c\
               		   walker.c parser.c lexer.c lexer_plug.c system.c file.c\
               		   collections.c util.c memory.c utf8.c symbol.c siphash.c\
                       coroutine.c generator.c base_extensions.c dictionary.h\
					   siphash.h marshal.c promise.c symbol_cxx.cpp symbol.hpp\
					   promise.h native-asm.h

EXTRA_libcitron_la_SOURCES = inject.c ctypes.c reverse_sizes.h\
					   _struct.h structmember.h ctypes.h\
					   _struct.c structmember.c inline-asm.cpp\
					   native-asm.h

if WITH_FFI
libcitron_la_SOURCES += ctypes.c reverse_sizes.h\
                         _struct.h structmember.h ctypes.h\
                         _struct.c structmember.c
ctr_SOURCES += ctypes.c reverse_sizes.h\
                         _struct.h structmember.h ctypes.h\
                         _struct.c structmember.c
endif

if WITH_INJECT
libcitron_la_SOURCES += inject.c
ctr_SOURCES += inject.c
endif

if WITH_INLINEASM
libcitron_la_SOURCES += inline-asm.cpp
ctr_SOURCES += inline-asm.cpp
endif
