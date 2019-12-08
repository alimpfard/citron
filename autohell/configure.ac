#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT([Citron], [0.0.9], [devanothertest@gmail.com])
AM_INIT_AUTOMAKE(Citron, 0.0.9)
AC_OUTPUT(Makefile src/Makefile doc/Makefile examples/Makefile man/Makefile scripts/Makefile)
AC_CONFIG_SRCDIR([src/base.c])
AC_CONFIG_HEADERS([config.h])
LT_INIT

# Checks for programs.
AC_PROG_AWK
AC_PROG_CC
AC_PROG_CXX
AC_PROG_MAKE_SET
AC_PROG_RANLIB


# Checks for header files.
AC_PATH_X
AC_FUNC_ALLOCA
AC_CHECK_HEADERS([arpa/inet.h fcntl.h inttypes.h malloc.h memory.h netdb.h netinet/in.h stddef.h stdint.h stdlib.h string.h strings.h sys/file.h sys/socket.h sys/time.h syslog.h termios.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_TYPE_UID_T
AC_C_INLINE
AC_TYPE_INT16_T
AC_TYPE_INT32_T
AC_TYPE_INT64_T
AC_TYPE_INT8_T
AC_TYPE_MODE_T
AC_TYPE_OFF_T
AC_TYPE_PID_T
AC_TYPE_SIZE_T
AC_TYPE_SSIZE_T
AC_TYPE_UINT16_T
AC_TYPE_UINT32_T
AC_TYPE_UINT64_T
AC_TYPE_UINT8_T
AC_CHECK_TYPES([ptrdiff_t])

# Checks for library functions.
AC_FUNC_FORK
AC_FUNC_FSEEKO
AC_FUNC_MALLOC
AC_FUNC_MKTIME
AC_FUNC_REALLOC
AC_FUNC_STRCOLL
AC_FUNC_STRERROR_R
AC_FUNC_STRNLEN
AC_FUNC_STRTOD
AC_CHECK_FUNCS([atexit bzero dup2 floor getdelim gethostbyname gettimeofday isascii mblen memchr memmove memset pow putenv realpath regcomp rpmatch select setenv socket sqrt stpcpy strcasecmp strchr strcspn strdup strerror strncasecmp strndup strpbrk strrchr strspn strstr strtol strtoul strtoull strtoumax])


AC_ARG_WITH([boehm-gc],
              AS_HELP_STRING([--with-boehm-gc], [Build without libgc present]),
              [],
              [enable_boehm_gc=yes])
AC_ARG_WITH([inject],
              AS_HELP_STRING([--with-inject], [Build with the Inject object]),
              [want_inject=yes],
              [])
AC_ARG_WITH([ffi],
              AS_HELP_STRING([--with-ffi], [Build with the CTypes object]),
              [want_ffi=yes],
              [])
AC_ARG_WITH([inlineasm],
              AS_HELP_STRING([--with-inline-asm], [Build with Inline assembly support (requires llvm)]),
              [want_inlineasm=yes],
              [])
AC_ARG_WITH([lffi],
            AS_HELP_STRING([--with-lffi], [Build with the specified ffi library]))
AC_ARG_WITH([stdpath],
            AS_HELP_STRING([--with-stdpath], [Set the standard lib path]))
AC_ARG_WITH([no-tls],
              AS_HELP_STRING([--with-no-tls], [Build without threadlocal storage (fucks up threads)]),
              [no_tls=yes],
              [])
AS_IF([test "x$no_tls" = "xyes"], [
              CFLAGS+=' -DNO_TLS'
])
AS_IF([test "x$enable_boehm_gc" = "xyes"], [
       CFLAGS+=" -DwithBoehmGC"
       AC_CHECK_LIB([gc], [GC_free])
])

AS_IF([test "x$with_lffi" = "x"], [
       with_lffi=ffi
])

AS_IF([test "x$with_stdpath" = "x"], [
       STD_PATH="../../share/Citron"
       ])
AS_IF([test "x$with_stdpath" != "x"], [
       STD_PATH="$with_stdpath"
    ])

AC_MSG_CHECKING(inject)
AS_IF([test "x$want_inject" = "xyes"], [
       CFLAGS+=" -DwithInjectNative=1"
       MAYBE_TCC=tcc
       MAYBE_TCC_L=-ltcc
       CTR_MAYBE_INJECT="inject.c"
    ])
AC_MSG_RESULT($want_inject)
AC_MSG_CHECKING(ffi)
AS_IF([test "x$want_ffi" = "xyes"], [
       CFLAGS+=" -DwithCTypesNative=1"
       MAYBE_FFI=ffi
       MAYBE_FFI_L=-l$with_lffi
       CTR_MAYBE_FFI="ctypes.c reverse_sizes.h _struct.h structmember.h ctypes.h _struct.c structmember.c"
       ])
AC_MSG_RESULT($want_ffi)
AC_MSG_CHECKING(inlineasm)
AS_IF([test "x$want_inlineasm" = "xyes"], [
       CFLAGS+=" -DwithInlineAsm=1"
       MAYBE_ASM_C=`llvm-config --cflags --system-libs --libs core orcjit native`
       MAYBE_ASM_CXX=`llvm-config --cxxflags --system-libs --libs core orcjit native`
       MAYBE_ASM_L=`llvm-config --system-libs --libs core orcjit native`
       CTR_MAYBE_ASM="inline-asm.cpp native-asm.h"
       ])
AC_MSG_RESULT($want_inlineasm)
AC_SUBST([MAYBE_TCC_L])
AC_SUBST([MAYBE_FFI_L])
AC_SUBST([MAYBE_ASM_L])
AM_CONDITIONAL([WITH_INJECT], [test "x$want_inject" = "xyes"])
AM_CONDITIONAL([WITH_FFI], [test "x$want_ffi" = "xyes"])
AM_CONDITIONAL([WITH_INLINEASM], [test "x$want_inlineasm" = "xyes"])

AC_ARG_WITH([linux],
              AS_HELP_STRING([--for-linux], [Build for linux OSs]),
              [enable_forlinux=yes],
              [enable_forlinux=no]
              )
AS_IF([test "x$enable_forlinux" != "xno"], [
       CFLAGS+=" -D forLinux"
])
AC_OUTPUT
