#ifndef COMPCOMPAT_PTHREAD
#define COMPCOMPAT_PTHREAD

#ifndef _GNU_SOURCE
#    ifdef __has_include
#        if __has_include(<sys/prctl.h>)

#            include <sys/prctl.h>
#            include <unistd.h>

int pthread_getname_np(pthread_t pth, char* buf, size_t len)
{
    return prctl(PR_GET_NAME, buf);
}
int pthread_setname_np(pthread_t pth, const char* buf)
{
    return prctl(PR_SET_NAME, buf);
}
#        else //__has_include

int pthread_getname_np(pthread_t pth, char* buf, size_t len) { return 1; }
int pthread_setname_np(pthread_t pth, const char* buf) { return 1; }

#        endif //__has_include

#    else // ifdef __has_include

#        warning "We don't have __has_include. running blind and hoping for the best"
#        include <sys/prctl.h>

int pthread_getname_np(pthread_t pth, char* buf, size_t len)
{
    return prctl(PR_GET_NAME, buf);
}
int pthread_setname_np(pthread_t pth, const char* buf)
{
    return prctl(PR_SET_NAME, buf);
}
#    endif // ifdef  __has_include
#endif     // ifndef _GNU_SOURCE

#endif /* end of include guard: COMPCOMPAT_PTHREAD */
