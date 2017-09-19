#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT([Citron], [0.0.2], [devanothertest@gmail.com])
AM_INIT_AUTOMAKE(Citron, 0.0.2)
AC_CONFIG_SRCDIR([src/base.c])
AC_CONFIG_HEADERS([config.h])

# Checks for programs.
AC_PROG_AWK
AC_PROG_CXX
AC_PROG_CC
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_MAKE_SET
AC_PROG_RANLIB

# Checks for libraries.
# FIXME: Replace `main' with a function in `-lbsd':
AC_CHECK_LIB([bsd], [main])
# FIXME: Replace `main' with a function in `-lccgi':
AC_CHECK_LIB([ccgi], [main])
# FIXME: Replace `main' with a function in `-lcrypto':
AC_CHECK_LIB([crypto], [main])
# FIXME: Replace `main' with a function in `-ldl':
AC_CHECK_LIB([dl], [main])
# FIXME: Replace `main' with a function in `-lm':
AC_CHECK_LIB([m], [main])

# Checks for header files.
AC_PATH_X
AC_FUNC_ALLOCA
AC_CHECK_HEADERS([arpa/inet.h fcntl.h inttypes.h limits.h locale.h malloc.h memory.h netdb.h netinet/in.h paths.h stddef.h stdint.h stdio_ext.h stdlib.h string.h strings.h sys/file.h sys/param.h sys/socket.h sys/time.h syslog.h termios.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_CHECK_HEADER_STDBOOL
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
AC_FUNC_MMAP
AC_FUNC_REALLOC
AC_FUNC_STRCOLL
AC_FUNC_STRERROR_R
AC_FUNC_STRNLEN
AC_FUNC_STRTOD
AC_CHECK_FUNCS([atexit bzero dup2 floor ftruncate getdelim gethostbyname getpass gettimeofday isascii localeconv mblen memchr memmove memset munmap pow putenv realpath regcomp rpmatch select setenv socket sqrt stpcpy strcasecmp strchr strcspn strdup strerror strncasecmp strndup strpbrk strrchr strspn strstr strtol strtoul strtoull strtoumax])

AC_CONFIG_FILES([libbsd/Makefile
                 makefile
                 misc/language-ctr/node_modules/oniguruma/build/Makefile
                 src/plugins/request/ccgi-1.2/Makefile
                 src/plugins/request/ccgi-1.2/examples/Makefile
                 src/plugins/request/ccgi-1.2/t/Makefile
                 src/plugins/sdl/Makefile
                 src/plugins/sfml/Makefile])
AC_CONFIG_SUBDIRS([misc/language-ctr/node_modules/oniguruma/deps/onig])
AC_OUTPUT
