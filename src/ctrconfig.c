#include <stdio.h>

int cflags = 0;
int ldflags = 0;

#ifndef forLinux
#    define BSD_L ""
#else
#    define BSD_L "-lbsd"
#endif
#ifdef withBoehmGC
#    define GC_L " -lgc"
#else
#    define GC_L ""
#endif
#if withInjectNative
#    define TCC_L " -ltcc "
#else
#    define TCC_L ""
#endif
#ifdef withCTypesNative
#    define FFI_L " -lffi "
#else
#    define FFI_L ""
#endif
#ifdef HAVE_OPENMP
#    define OMP_F "-fopenmp "
#else
#    define OMP_F ""
#endif
#define LDFLAGS "-lcitron " TCC_L " -lpcre -ldl -lm -pthread " FFI_L BSD_L GC_L
#define CFLAGS OMP_F "-rdynamic"

int main(int argc, char* argv[])
{
    for (int i = 0; i < argc; i++)
        if (strcmp(argv[i], "--cflags") == 0)
            cflags = 1;
        else if (strcmp(argv[i], "--ldflags") == 0)
            ldflags = 1;
    if (cflags)
        printf("%s ", CFLAGS);
    if (ldflags)
        printf("%s", LDFLAGS);
    putc('\n', stdout);
    return 0;
}
