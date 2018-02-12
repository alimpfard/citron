#ifndef COMPCOMPAT_PTHREAD
#define COMPCOMPAT_PTHREAD

#ifndef _GNU_SOURCE

#include <sys/prctl.h>

int pthread_getname_np(int pth, char* buf, int len) {
    return prctl(PR_GET_NAME, buf);
}
int pthread_setname_np(int pth, char* buf) {
    return prctl(PR_SET_NAME, buf);
}
#endif


#endif /* end of include guard: COMPCOMPAT_PTHREAD */
