#include <stdio.h>


int cflags = 0;
int ldflags = 0;

#define CFLAGS "-rdynamic"
#ifndef forLinux
#define BSD_L ""
#else 
#define BSD_L "-lbsd"
#endif
#define LDFLAGS "-lcitron -ltcc -lpcre -ldl -lm -pthread -lffi " BSD_L


int main(int argc, char* argv[]) {
    for(int i=0; i<argc; i++)
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