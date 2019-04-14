// # 1 "collections.c"
// # 1 "<built-in>"
// # 1 "<command-line>"
// # 31 "<command-line>"
// # 1 "/usr/include/stdc-predef.h" 1 3 4
// # 32 "<command-line>" 2
// # 1 "collections.c"
// # 1 "/usr/include/stdlib.h" 1 3 4
// # 25 "/usr/include/stdlib.h" 3 4
// # 1 "/usr/include/bits/libc-header-start.h" 1 3 4
// # 33 "/usr/include/bits/libc-header-start.h" 3 4
// # 1 "/usr/include/features.h" 1 3 4
// # 428 "/usr/include/features.h" 3 4
// # 1 "/usr/include/sys/cdefs.h" 1 3 4
// # 442 "/usr/include/sys/cdefs.h" 3 4
// # 1 "/usr/include/bits/wordsize.h" 1 3 4
// # 443 "/usr/include/sys/cdefs.h" 2 3 4
// # 1 "/usr/include/bits/long-double.h" 1 3 4
// # 444 "/usr/include/sys/cdefs.h" 2 3 4
// # 429 "/usr/include/features.h" 2 3 4
// # 452 "/usr/include/features.h" 3 4
// # 1 "/usr/include/gnu/stubs.h" 1 3 4
// # 10 "/usr/include/gnu/stubs.h" 3 4
// # 1 "/usr/include/gnu/stubs-64.h" 1 3 4
// # 11 "/usr/include/gnu/stubs.h" 2 3 4
// # 453 "/usr/include/features.h" 2 3 4
// # 34 "/usr/include/bits/libc-header-start.h" 2 3 4
// # 26 "/usr/include/stdlib.h" 2 3 4

// # 1 "/usr/lib/gcc/x86_64-pc-linux-gnu/8.2.1/include/stddef.h" 1 3 4
// # 216 "/usr/lib/gcc/x86_64-pc-linux-gnu/8.2.1/include/stddef.h" 3 4

// # 216 "/usr/lib/gcc/x86_64-pc-linux-gnu/8.2.1/include/stddef.h" 3 4
typedef long unsigned int size_t;
// # 328 "/usr/lib/gcc/x86_64-pc-linux-gnu/8.2.1/include/stddef.h" 3 4
typedef int wchar_t;
// # 32 "/usr/include/stdlib.h" 2 3 4

// # 1 "/usr/include/bits/waitflags.h" 1 3 4
// # 40 "/usr/include/stdlib.h" 2 3 4
// # 1 "/usr/include/bits/waitstatus.h" 1 3 4
// # 41 "/usr/include/stdlib.h" 2 3 4
// # 55 "/usr/include/stdlib.h" 3 4
// # 1 "/usr/include/bits/floatn.h" 1 3 4
// # 119 "/usr/include/bits/floatn.h" 3 4
// # 1 "/usr/include/bits/floatn-common.h" 1 3 4
// # 24 "/usr/include/bits/floatn-common.h" 3 4
// # 1 "/usr/include/bits/long-double.h" 1 3 4
// # 25 "/usr/include/bits/floatn-common.h" 2 3 4
// # 120 "/usr/include/bits/floatn.h" 2 3 4
// # 56 "/usr/include/stdlib.h" 2 3 4

typedef struct {
  int quot;
  int rem;
} div_t;

typedef struct {
  long int quot;
  long int rem;
} ldiv_t;

__extension__ typedef struct {
  long long int quot;
  long long int rem;
} lldiv_t;
// # 97 "/usr/include/stdlib.h" 3 4
extern size_t __ctype_get_mb_cur_max(void)
    __attribute__((__nothrow__, __leaf__));

extern double atof(const char *__nptr) __attribute__((__nothrow__, __leaf__))
__attribute__((__pure__)) __attribute__((__nonnull__(1)));

extern int atoi(const char *__nptr) __attribute__((__nothrow__, __leaf__))
__attribute__((__pure__)) __attribute__((__nonnull__(1)));

extern long int atol(const char *__nptr) __attribute__((__nothrow__, __leaf__))
__attribute__((__pure__)) __attribute__((__nonnull__(1)));

__extension__ extern long long int atoll(const char *__nptr)
    __attribute__((__nothrow__, __leaf__)) __attribute__((__pure__))
    __attribute__((__nonnull__(1)));

extern double strtod(const char *__restrict __nptr, char **__restrict __endptr)
    __attribute__((__nothrow__, __leaf__)) __attribute__((__nonnull__(1)));

extern float strtof(const char *__restrict __nptr, char **__restrict __endptr)
    __attribute__((__nothrow__, __leaf__)) __attribute__((__nonnull__(1)));

extern long double strtold(const char *__restrict __nptr,
                           char **__restrict __endptr)
    __attribute__((__nothrow__, __leaf__)) __attribute__((__nonnull__(1)));
// # 176 "/usr/include/stdlib.h" 3 4
extern long int strtol(const char *__restrict __nptr,
                       char **__restrict __endptr, int __base)
    __attribute__((__nothrow__, __leaf__)) __attribute__((__nonnull__(1)));

extern unsigned long int strtoul(const char *__restrict __nptr,
                                 char **__restrict __endptr, int __base)
    __attribute__((__nothrow__, __leaf__)) __attribute__((__nonnull__(1)));

__extension__ extern long long int
strtoq(const char *__restrict __nptr, char **__restrict __endptr, int __base)
    __attribute__((__nothrow__, __leaf__)) __attribute__((__nonnull__(1)));

__extension__ extern unsigned long long int
strtouq(const char *__restrict __nptr, char **__restrict __endptr, int __base)
    __attribute__((__nothrow__, __leaf__)) __attribute__((__nonnull__(1)));

__extension__ extern long long int
strtoll(const char *__restrict __nptr, char **__restrict __endptr, int __base)
    __attribute__((__nothrow__, __leaf__)) __attribute__((__nonnull__(1)));

__extension__ extern unsigned long long int
strtoull(const char *__restrict __nptr, char **__restrict __endptr, int __base)
    __attribute__((__nothrow__, __leaf__)) __attribute__((__nonnull__(1)));
// # 385 "/usr/include/stdlib.h" 3 4
extern char *l64a(long int __n) __attribute__((__nothrow__, __leaf__));

extern long int a64l(const char *__s) __attribute__((__nothrow__, __leaf__))
__attribute__((__pure__)) __attribute__((__nonnull__(1)));

// # 1 "/usr/include/sys/types.h" 1 3 4
// # 27 "/usr/include/sys/types.h" 3 4

// # 1 "/usr/include/bits/types.h" 1 3 4
// # 27 "/usr/include/bits/types.h" 3 4
// # 1 "/usr/include/bits/wordsize.h" 1 3 4
// # 28 "/usr/include/bits/types.h" 2 3 4

typedef unsigned char __u_char;
typedef unsigned short int __u_short;
typedef unsigned int __u_int;
typedef unsigned long int __u_long;

typedef signed char __int8_t;
typedef unsigned char __uint8_t;
typedef signed short int __int16_t;
typedef unsigned short int __uint16_t;
typedef signed int __int32_t;
typedef unsigned int __uint32_t;

typedef signed long int __int64_t;
typedef unsigned long int __uint64_t;

typedef __int8_t __int_least8_t;
typedef __uint8_t __uint_least8_t;
typedef __int16_t __int_least16_t;
typedef __uint16_t __uint_least16_t;
typedef __int32_t __int_least32_t;
typedef __uint32_t __uint_least32_t;
typedef __int64_t __int_least64_t;
typedef __uint64_t __uint_least64_t;

typedef long int __quad_t;
typedef unsigned long int __u_quad_t;

typedef long int __intmax_t;
typedef unsigned long int __uintmax_t;
// # 140 "/usr/include/bits/types.h" 3 4
// # 1 "/usr/include/bits/typesizes.h" 1 3 4
// # 141 "/usr/include/bits/types.h" 2 3 4

typedef unsigned long int __dev_t;
typedef unsigned int __uid_t;
typedef unsigned int __gid_t;
typedef unsigned long int __ino_t;
typedef unsigned long int __ino64_t;
typedef unsigned int __mode_t;
typedef unsigned long int __nlink_t;
typedef long int __off_t;
typedef long int __off64_t;
typedef int __pid_t;
typedef struct {
  int __val[2];
} __fsid_t;
typedef long int __clock_t;
typedef unsigned long int __rlim_t;
typedef unsigned long int __rlim64_t;
typedef unsigned int __id_t;
typedef long int __time_t;
typedef unsigned int __useconds_t;
typedef long int __suseconds_t;

typedef int __daddr_t;
typedef int __key_t;

typedef int __clockid_t;

typedef void *__timer_t;

typedef long int __blksize_t;

typedef long int __blkcnt_t;
typedef long int __blkcnt64_t;

typedef unsigned long int __fsblkcnt_t;
typedef unsigned long int __fsblkcnt64_t;

typedef unsigned long int __fsfilcnt_t;
typedef unsigned long int __fsfilcnt64_t;

typedef long int __fsword_t;

typedef long int __ssize_t;

typedef long int __syscall_slong_t;

typedef unsigned long int __syscall_ulong_t;

typedef __off64_t __loff_t;
typedef char *__caddr_t;

typedef long int __intptr_t;

typedef unsigned int __socklen_t;

typedef int __sig_atomic_t;
// # 30 "/usr/include/sys/types.h" 2 3 4

typedef __u_char u_char;
typedef __u_short u_short;
typedef __u_int u_int;
typedef __u_long u_long;
typedef __quad_t quad_t;
typedef __u_quad_t u_quad_t;
typedef __fsid_t fsid_t;

typedef __loff_t loff_t;

typedef __ino_t ino_t;
// # 59 "/usr/include/sys/types.h" 3 4
typedef __dev_t dev_t;

typedef __gid_t gid_t;

typedef __mode_t mode_t;

typedef __nlink_t nlink_t;

typedef __uid_t uid_t;

typedef __off_t off_t;
// # 97 "/usr/include/sys/types.h" 3 4
typedef __pid_t pid_t;

typedef __id_t id_t;

typedef __ssize_t ssize_t;

typedef __daddr_t daddr_t;
typedef __caddr_t caddr_t;

typedef __key_t key_t;

// # 1 "/usr/include/bits/types/clock_t.h" 1 3 4

typedef __clock_t clock_t;
// # 127 "/usr/include/sys/types.h" 2 3 4

// # 1 "/usr/include/bits/types/clockid_t.h" 1 3 4

typedef __clockid_t clockid_t;
// # 129 "/usr/include/sys/types.h" 2 3 4
// # 1 "/usr/include/bits/types/time_t.h" 1 3 4

typedef __time_t time_t;
// # 130 "/usr/include/sys/types.h" 2 3 4
// # 1 "/usr/include/bits/types/timer_t.h" 1 3 4

typedef __timer_t timer_t;
// # 131 "/usr/include/sys/types.h" 2 3 4
// # 144 "/usr/include/sys/types.h" 3 4
// # 1 "/usr/lib/gcc/x86_64-pc-linux-gnu/8.2.1/include/stddef.h" 1 3 4
// # 145 "/usr/include/sys/types.h" 2 3 4

typedef unsigned long int ulong;
typedef unsigned short int ushort;
typedef unsigned int uint;

// # 1 "/usr/include/bits/stdint-intn.h" 1 3 4
// # 24 "/usr/include/bits/stdint-intn.h" 3 4
typedef __int8_t int8_t;
typedef __int16_t int16_t;
typedef __int32_t int32_t;
typedef __int64_t int64_t;
// # 156 "/usr/include/sys/types.h" 2 3 4
// # 177 "/usr/include/sys/types.h" 3 4
typedef unsigned int u_int8_t __attribute__((__mode__(__QI__)));
typedef unsigned int u_int16_t __attribute__((__mode__(__HI__)));
typedef unsigned int u_int32_t __attribute__((__mode__(__SI__)));
typedef unsigned int u_int64_t __attribute__((__mode__(__DI__)));

typedef int register_t __attribute__((__mode__(__word__)));
// # 193 "/usr/include/sys/types.h" 3 4
// # 1 "/usr/include/endian.h" 1 3 4
// # 36 "/usr/include/endian.h" 3 4
// # 1 "/usr/include/bits/endian.h" 1 3 4
// # 37 "/usr/include/endian.h" 2 3 4
// # 60 "/usr/include/endian.h" 3 4
// # 1 "/usr/include/bits/byteswap.h" 1 3 4
// # 33 "/usr/include/bits/byteswap.h" 3 4
static __inline __uint16_t __bswap_16(__uint16_t __bsx) {

  return __builtin_bswap16(__bsx);
}

static __inline __uint32_t __bswap_32(__uint32_t __bsx) {

  return __builtin_bswap32(__bsx);
}
// # 69 "/usr/include/bits/byteswap.h" 3 4
__extension__ static __inline __uint64_t __bswap_64(__uint64_t __bsx) {

  return __builtin_bswap64(__bsx);
}
// # 61 "/usr/include/endian.h" 2 3 4
// # 1 "/usr/include/bits/uintn-identity.h" 1 3 4
// # 32 "/usr/include/bits/uintn-identity.h" 3 4
static __inline __uint16_t __uint16_identity(__uint16_t __x) { return __x; }

static __inline __uint32_t __uint32_identity(__uint32_t __x) { return __x; }

static __inline __uint64_t __uint64_identity(__uint64_t __x) { return __x; }
// # 62 "/usr/include/endian.h" 2 3 4
// # 194 "/usr/include/sys/types.h" 2 3 4

// # 1 "/usr/include/sys/select.h" 1 3 4
// # 30 "/usr/include/sys/select.h" 3 4
// # 1 "/usr/include/bits/select.h" 1 3 4
// # 22 "/usr/include/bits/select.h" 3 4
// # 1 "/usr/include/bits/wordsize.h" 1 3 4
// # 23 "/usr/include/bits/select.h" 2 3 4
// # 31 "/usr/include/sys/select.h" 2 3 4

// # 1 "/usr/include/bits/types/sigset_t.h" 1 3 4

// # 1 "/usr/include/bits/types/__sigset_t.h" 1 3 4

typedef struct {
  unsigned long int __val[(1024 / (8 * sizeof(unsigned long int)))];
} __sigset_t;
// # 5 "/usr/include/bits/types/sigset_t.h" 2 3 4

typedef __sigset_t sigset_t;
// # 34 "/usr/include/sys/select.h" 2 3 4

// # 1 "/usr/include/bits/types/struct_timeval.h" 1 3 4

struct timeval {
  __time_t tv_sec;
  __suseconds_t tv_usec;
};
// # 38 "/usr/include/sys/select.h" 2 3 4

// # 1 "/usr/include/bits/types/struct_timespec.h" 1 3 4
// # 9 "/usr/include/bits/types/struct_timespec.h" 3 4
struct timespec {
  __time_t tv_sec;
  __syscall_slong_t tv_nsec;
};
// # 40 "/usr/include/sys/select.h" 2 3 4

typedef __suseconds_t suseconds_t;

typedef long int __fd_mask;
// # 59 "/usr/include/sys/select.h" 3 4
typedef struct {

  __fd_mask __fds_bits[1024 / (8 * (int)sizeof(__fd_mask))];

} fd_set;

typedef __fd_mask fd_mask;
// # 91 "/usr/include/sys/select.h" 3 4

// # 101 "/usr/include/sys/select.h" 3 4
extern int select(int __nfds, fd_set *__restrict __readfds,
                  fd_set *__restrict __writefds, fd_set *__restrict __exceptfds,
                  struct timeval *__restrict __timeout);
// # 113 "/usr/include/sys/select.h" 3 4
extern int pselect(int __nfds, fd_set *__restrict __readfds,
                   fd_set *__restrict __writefds,
                   fd_set *__restrict __exceptfds,
                   const struct timespec *__restrict __timeout,
                   const __sigset_t *__restrict __sigmask);
// # 126 "/usr/include/sys/select.h" 3 4

// # 197 "/usr/include/sys/types.h" 2 3 4

typedef __blksize_t blksize_t;

typedef __blkcnt_t blkcnt_t;

typedef __fsblkcnt_t fsblkcnt_t;

typedef __fsfilcnt_t fsfilcnt_t;
// # 244 "/usr/include/sys/types.h" 3 4
// # 1 "/usr/include/bits/pthreadtypes.h" 1 3 4
// # 23 "/usr/include/bits/pthreadtypes.h" 3 4
// # 1 "/usr/include/bits/thread-shared-types.h" 1 3 4
// # 77 "/usr/include/bits/thread-shared-types.h" 3 4
// # 1 "/usr/include/bits/pthreadtypes-arch.h" 1 3 4
// # 21 "/usr/include/bits/pthreadtypes-arch.h" 3 4
// # 1 "/usr/include/bits/wordsize.h" 1 3 4
// # 22 "/usr/include/bits/pthreadtypes-arch.h" 2 3 4
// # 65 "/usr/include/bits/pthreadtypes-arch.h" 3 4
struct __pthread_rwlock_arch_t {
  unsigned int __readers;
  unsigned int __writers;
  unsigned int __wrphase_futex;
  unsigned int __writers_futex;
  unsigned int __pad3;
  unsigned int __pad4;

  int __cur_writer;
  int __shared;
  signed char __rwelision;

  unsigned char __pad1[7];

  unsigned long int __pad2;

  unsigned int __flags;
// # 99 "/usr/include/bits/pthreadtypes-arch.h" 3 4
};
// # 78 "/usr/include/bits/thread-shared-types.h" 2 3 4

typedef struct __pthread_internal_list {
  struct __pthread_internal_list *__prev;
  struct __pthread_internal_list *__next;
} __pthread_list_t;
// # 118 "/usr/include/bits/thread-shared-types.h" 3 4
struct __pthread_mutex_s {
  int __lock;
  unsigned int __count;
  int __owner;

  unsigned int __nusers;

  int __kind;

  short __spins;
  short __elision;
  __pthread_list_t __list;
// # 145 "/usr/include/bits/thread-shared-types.h" 3 4
};

struct __pthread_cond_s {
  __extension__ union {
    __extension__ unsigned long long int __wseq;
    struct {
      unsigned int __low;
      unsigned int __high;
    } __wseq32;
  };
  __extension__ union {
    __extension__ unsigned long long int __g1_start;
    struct {
      unsigned int __low;
      unsigned int __high;
    } __g1_start32;
  };
  unsigned int __g_refs[2];
  unsigned int __g_size[2];
  unsigned int __g1_orig_size;
  unsigned int __wrefs;
  unsigned int __g_signals[2];
};
// # 24 "/usr/include/bits/pthreadtypes.h" 2 3 4

typedef unsigned long int pthread_t;

typedef union {
  char __size[4];
  int __align;
} pthread_mutexattr_t;

typedef union {
  char __size[4];
  int __align;
} pthread_condattr_t;

typedef unsigned int pthread_key_t;

typedef int pthread_once_t;

union pthread_attr_t {
  char __size[56];
  long int __align;
};

typedef union pthread_attr_t pthread_attr_t;

typedef union {
  struct __pthread_mutex_s __data;
  char __size[40];
  long int __align;
} pthread_mutex_t;

typedef union {
  struct __pthread_cond_s __data;
  char __size[48];
  __extension__ long long int __align;
} pthread_cond_t;

typedef union {
  struct __pthread_rwlock_arch_t __data;
  char __size[56];
  long int __align;
} pthread_rwlock_t;

typedef union {
  char __size[8];
  long int __align;
} pthread_rwlockattr_t;

typedef volatile int pthread_spinlock_t;

typedef union {
  char __size[32];
  long int __align;
} pthread_barrier_t;

typedef union {
  char __size[4];
  int __align;
} pthread_barrierattr_t;
// # 245 "/usr/include/sys/types.h" 2 3 4

// # 395 "/usr/include/stdlib.h" 2 3 4

extern long int random(void) __attribute__((__nothrow__, __leaf__));

extern void srandom(unsigned int __seed) __attribute__((__nothrow__, __leaf__));

extern char *initstate(unsigned int __seed, char *__statebuf, size_t __statelen)
    __attribute__((__nothrow__, __leaf__)) __attribute__((__nonnull__(2)));

extern char *setstate(char *__statebuf) __attribute__((__nothrow__, __leaf__))
__attribute__((__nonnull__(1)));

struct random_data {
  int32_t *fptr;
  int32_t *rptr;
  int32_t *state;
  int rand_type;
  int rand_deg;
  int rand_sep;
  int32_t *end_ptr;
};

extern int random_r(struct random_data *__restrict __buf,
                    int32_t *__restrict __result)
    __attribute__((__nothrow__, __leaf__)) __attribute__((__nonnull__(1, 2)));

extern int srandom_r(unsigned int __seed, struct random_data *__buf)
    __attribute__((__nothrow__, __leaf__)) __attribute__((__nonnull__(2)));

extern int initstate_r(unsigned int __seed, char *__restrict __statebuf,
                       size_t __statelen, struct random_data *__restrict __buf)
    __attribute__((__nothrow__, __leaf__)) __attribute__((__nonnull__(2, 4)));

extern int setstate_r(char *__restrict __statebuf,
                      struct random_data *__restrict __buf)
    __attribute__((__nothrow__, __leaf__)) __attribute__((__nonnull__(1, 2)));

extern int rand(void) __attribute__((__nothrow__, __leaf__));

extern void srand(unsigned int __seed) __attribute__((__nothrow__, __leaf__));

extern int rand_r(unsigned int *__seed) __attribute__((__nothrow__, __leaf__));

extern double drand48(void) __attribute__((__nothrow__, __leaf__));
extern double erand48(unsigned short int __xsubi[3])
    __attribute__((__nothrow__, __leaf__)) __attribute__((__nonnull__(1)));

extern long int lrand48(void) __attribute__((__nothrow__, __leaf__));
extern long int nrand48(unsigned short int __xsubi[3])
    __attribute__((__nothrow__, __leaf__)) __attribute__((__nonnull__(1)));

extern long int mrand48(void) __attribute__((__nothrow__, __leaf__));
extern long int jrand48(unsigned short int __xsubi[3])
    __attribute__((__nothrow__, __leaf__)) __attribute__((__nonnull__(1)));

extern void srand48(long int __seedval) __attribute__((__nothrow__, __leaf__));
extern unsigned short int *seed48(unsigned short int __seed16v[3])
    __attribute__((__nothrow__, __leaf__)) __attribute__((__nonnull__(1)));
extern void lcong48(unsigned short int __param[7])
    __attribute__((__nothrow__, __leaf__)) __attribute__((__nonnull__(1)));

struct drand48_data {
  unsigned short int __x[3];
  unsigned short int __old_x[3];
  unsigned short int __c;
  unsigned short int __init;
  __extension__ unsigned long long int __a;
};

extern int drand48_r(struct drand48_data *__restrict __buffer,
                     double *__restrict __result)
    __attribute__((__nothrow__, __leaf__)) __attribute__((__nonnull__(1, 2)));
extern int erand48_r(unsigned short int __xsubi[3],
                     struct drand48_data *__restrict __buffer,
                     double *__restrict __result)
    __attribute__((__nothrow__, __leaf__)) __attribute__((__nonnull__(1, 2)));

extern int lrand48_r(struct drand48_data *__restrict __buffer,
                     long int *__restrict __result)
    __attribute__((__nothrow__, __leaf__)) __attribute__((__nonnull__(1, 2)));
extern int nrand48_r(unsigned short int __xsubi[3],
                     struct drand48_data *__restrict __buffer,
                     long int *__restrict __result)
    __attribute__((__nothrow__, __leaf__)) __attribute__((__nonnull__(1, 2)));

extern int mrand48_r(struct drand48_data *__restrict __buffer,
                     long int *__restrict __result)
    __attribute__((__nothrow__, __leaf__)) __attribute__((__nonnull__(1, 2)));
extern int jrand48_r(unsigned short int __xsubi[3],
                     struct drand48_data *__restrict __buffer,
                     long int *__restrict __result)
    __attribute__((__nothrow__, __leaf__)) __attribute__((__nonnull__(1, 2)));

extern int srand48_r(long int __seedval, struct drand48_data *__buffer)
    __attribute__((__nothrow__, __leaf__)) __attribute__((__nonnull__(2)));

extern int seed48_r(unsigned short int __seed16v[3],
                    struct drand48_data *__buffer)
    __attribute__((__nothrow__, __leaf__)) __attribute__((__nonnull__(1, 2)));

extern int lcong48_r(unsigned short int __param[7],
                     struct drand48_data *__buffer)
    __attribute__((__nothrow__, __leaf__)) __attribute__((__nonnull__(1, 2)));

extern void *malloc(size_t __size) __attribute__((__nothrow__, __leaf__))
__attribute__((__malloc__));

extern void *calloc(size_t __nmemb, size_t __size)
    __attribute__((__nothrow__, __leaf__)) __attribute__((__malloc__));

extern void *realloc(void *__ptr, size_t __size)
    __attribute__((__nothrow__, __leaf__))
    __attribute__((__warn_unused_result__));
// # 563 "/usr/include/stdlib.h" 3 4
extern void free(void *__ptr) __attribute__((__nothrow__, __leaf__));

// # 1 "/usr/include/alloca.h" 1 3 4
// # 24 "/usr/include/alloca.h" 3 4
// # 1 "/usr/lib/gcc/x86_64-pc-linux-gnu/8.2.1/include/stddef.h" 1 3 4
// # 25 "/usr/include/alloca.h" 2 3 4

extern void *alloca(size_t __size) __attribute__((__nothrow__, __leaf__));

// # 567 "/usr/include/stdlib.h" 2 3 4

extern void *valloc(size_t __size) __attribute__((__nothrow__, __leaf__))
__attribute__((__malloc__));

extern int posix_memalign(void **__memptr, size_t __alignment, size_t __size)
    __attribute__((__nothrow__, __leaf__)) __attribute__((__nonnull__(1)));

extern void *aligned_alloc(size_t __alignment, size_t __size)
    __attribute__((__nothrow__, __leaf__)) __attribute__((__malloc__))
    __attribute__((__alloc_size__(2)));

extern void abort(void) __attribute__((__nothrow__, __leaf__))
__attribute__((__noreturn__));

extern int atexit(void (*__func)(void)) __attribute__((__nothrow__, __leaf__))
__attribute__((__nonnull__(1)));

extern int at_quick_exit(void (*__func)(void))
    __attribute__((__nothrow__, __leaf__)) __attribute__((__nonnull__(1)));

extern int on_exit(void (*__func)(int __status, void *__arg), void *__arg)
    __attribute__((__nothrow__, __leaf__)) __attribute__((__nonnull__(1)));

extern void exit(int __status) __attribute__((__nothrow__, __leaf__))
__attribute__((__noreturn__));

extern void quick_exit(int __status) __attribute__((__nothrow__, __leaf__))
__attribute__((__noreturn__));

extern void _Exit(int __status) __attribute__((__nothrow__, __leaf__))
__attribute__((__noreturn__));

extern char *getenv(const char *__name) __attribute__((__nothrow__, __leaf__))
__attribute__((__nonnull__(1)));
// # 644 "/usr/include/stdlib.h" 3 4
extern int putenv(char *__string) __attribute__((__nothrow__, __leaf__))
__attribute__((__nonnull__(1)));

extern int setenv(const char *__name, const char *__value, int __replace)
    __attribute__((__nothrow__, __leaf__)) __attribute__((__nonnull__(2)));

extern int unsetenv(const char *__name) __attribute__((__nothrow__, __leaf__))
__attribute__((__nonnull__(1)));

extern int clearenv(void) __attribute__((__nothrow__, __leaf__));
// # 672 "/usr/include/stdlib.h" 3 4
extern char *mktemp(char *__template) __attribute__((__nothrow__, __leaf__))
__attribute__((__nonnull__(1)));
// # 685 "/usr/include/stdlib.h" 3 4
extern int mkstemp(char *__template) __attribute__((__nonnull__(1)));
// # 707 "/usr/include/stdlib.h" 3 4
extern int mkstemps(char *__template, int __suffixlen)
    __attribute__((__nonnull__(1)));
// # 728 "/usr/include/stdlib.h" 3 4
extern char *mkdtemp(char *__template) __attribute__((__nothrow__, __leaf__))
__attribute__((__nonnull__(1)));
// # 781 "/usr/include/stdlib.h" 3 4
extern int system(const char *__command);
// # 797 "/usr/include/stdlib.h" 3 4
extern char *realpath(const char *__restrict __name,
                      char *__restrict __resolved)
    __attribute__((__nothrow__, __leaf__));

typedef int (*__compar_fn_t)(const void *, const void *);
// # 817 "/usr/include/stdlib.h" 3 4
extern void *bsearch(const void *__key, const void *__base, size_t __nmemb,
                     size_t __size, __compar_fn_t __compar)
    __attribute__((__nonnull__(1, 2, 5)));

extern void qsort(void *__base, size_t __nmemb, size_t __size,
                  __compar_fn_t __compar) __attribute__((__nonnull__(1, 4)));
// # 837 "/usr/include/stdlib.h" 3 4
extern int abs(int __x) __attribute__((__nothrow__, __leaf__))
__attribute__((__const__));
extern long int labs(long int __x) __attribute__((__nothrow__, __leaf__))
__attribute__((__const__));

__extension__ extern long long int llabs(long long int __x)
    __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));

extern div_t div(int __numer, int __denom)
    __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));
extern ldiv_t ldiv(long int __numer, long int __denom)
    __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));

__extension__ extern lldiv_t lldiv(long long int __numer, long long int __denom)
    __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));
// # 869 "/usr/include/stdlib.h" 3 4
extern char *ecvt(double __value, int __ndigit, int *__restrict __decpt,
                  int *__restrict __sign) __attribute__((__nothrow__, __leaf__))
__attribute__((__nonnull__(3, 4)));

extern char *fcvt(double __value, int __ndigit, int *__restrict __decpt,
                  int *__restrict __sign) __attribute__((__nothrow__, __leaf__))
__attribute__((__nonnull__(3, 4)));

extern char *gcvt(double __value, int __ndigit, char *__buf)
    __attribute__((__nothrow__, __leaf__)) __attribute__((__nonnull__(3)));

extern char *qecvt(long double __value, int __ndigit, int *__restrict __decpt,
                   int *__restrict __sign)
    __attribute__((__nothrow__, __leaf__)) __attribute__((__nonnull__(3, 4)));
extern char *qfcvt(long double __value, int __ndigit, int *__restrict __decpt,
                   int *__restrict __sign)
    __attribute__((__nothrow__, __leaf__)) __attribute__((__nonnull__(3, 4)));
extern char *qgcvt(long double __value, int __ndigit, char *__buf)
    __attribute__((__nothrow__, __leaf__)) __attribute__((__nonnull__(3)));

extern int ecvt_r(double __value, int __ndigit, int *__restrict __decpt,
                  int *__restrict __sign, char *__restrict __buf, size_t __len)
    __attribute__((__nothrow__, __leaf__))
    __attribute__((__nonnull__(3, 4, 5)));
extern int fcvt_r(double __value, int __ndigit, int *__restrict __decpt,
                  int *__restrict __sign, char *__restrict __buf, size_t __len)
    __attribute__((__nothrow__, __leaf__))
    __attribute__((__nonnull__(3, 4, 5)));

extern int qecvt_r(long double __value, int __ndigit, int *__restrict __decpt,
                   int *__restrict __sign, char *__restrict __buf, size_t __len)
    __attribute__((__nothrow__, __leaf__))
    __attribute__((__nonnull__(3, 4, 5)));
extern int qfcvt_r(long double __value, int __ndigit, int *__restrict __decpt,
                   int *__restrict __sign, char *__restrict __buf, size_t __len)
    __attribute__((__nothrow__, __leaf__))
    __attribute__((__nonnull__(3, 4, 5)));

extern int mblen(const char *__s, size_t __n)
    __attribute__((__nothrow__, __leaf__));

extern int mbtowc(wchar_t *__restrict __pwc, const char *__restrict __s,
                  size_t __n) __attribute__((__nothrow__, __leaf__));

extern int wctomb(char *__s, wchar_t __wchar)
    __attribute__((__nothrow__, __leaf__));

extern size_t mbstowcs(wchar_t *__restrict __pwcs, const char *__restrict __s,
                       size_t __n) __attribute__((__nothrow__, __leaf__));

extern size_t wcstombs(char *__restrict __s, const wchar_t *__restrict __pwcs,
                       size_t __n) __attribute__((__nothrow__, __leaf__));

extern int rpmatch(const char *__response)
    __attribute__((__nothrow__, __leaf__)) __attribute__((__nonnull__(1)));
// # 954 "/usr/include/stdlib.h" 3 4
extern int getsubopt(char **__restrict __optionp,
                     char *const *__restrict __tokens,
                     char **__restrict __valuep)
    __attribute__((__nothrow__, __leaf__))
    __attribute__((__nonnull__(1, 2, 3)));
// # 1000 "/usr/include/stdlib.h" 3 4
extern int getloadavg(double __loadavg[], int __nelem)
    __attribute__((__nothrow__, __leaf__)) __attribute__((__nonnull__(1)));
// # 1010 "/usr/include/stdlib.h" 3 4
// # 1 "/usr/include/bits/stdlib-float.h" 1 3 4
// # 1011 "/usr/include/stdlib.h" 2 3 4
// # 1020 "/usr/include/stdlib.h" 3 4

// # 2 "collections.c" 2
// # 1 "/usr/include/stdio.h" 1 3 4
// # 27 "/usr/include/stdio.h" 3 4
// # 1 "/usr/include/bits/libc-header-start.h" 1 3 4
// # 28 "/usr/include/stdio.h" 2 3 4

// # 1 "/usr/lib/gcc/x86_64-pc-linux-gnu/8.2.1/include/stddef.h" 1 3 4
// # 34 "/usr/include/stdio.h" 2 3 4

// # 1 "/usr/lib/gcc/x86_64-pc-linux-gnu/8.2.1/include/stdarg.h" 1 3 4
// # 40 "/usr/lib/gcc/x86_64-pc-linux-gnu/8.2.1/include/stdarg.h" 3 4
typedef __builtin_va_list __gnuc_va_list;
// # 37 "/usr/include/stdio.h" 2 3 4

// # 1 "/usr/include/bits/types/__fpos_t.h" 1 3 4

// # 1 "/usr/include/bits/types/__mbstate_t.h" 1 3 4
// # 13 "/usr/include/bits/types/__mbstate_t.h" 3 4
typedef struct {
  int __count;
  union {
    unsigned int __wch;
    char __wchb[4];
  } __value;
} __mbstate_t;
// # 6 "/usr/include/bits/types/__fpos_t.h" 2 3 4

typedef struct _G_fpos_t {
  __off_t __pos;
  __mbstate_t __state;
} __fpos_t;
// # 40 "/usr/include/stdio.h" 2 3 4
// # 1 "/usr/include/bits/types/__fpos64_t.h" 1 3 4
// # 10 "/usr/include/bits/types/__fpos64_t.h" 3 4
typedef struct _G_fpos64_t {
  __off64_t __pos;
  __mbstate_t __state;
} __fpos64_t;
// # 41 "/usr/include/stdio.h" 2 3 4
// # 1 "/usr/include/bits/types/__FILE.h" 1 3 4

struct _IO_FILE;
typedef struct _IO_FILE __FILE;
// # 42 "/usr/include/stdio.h" 2 3 4
// # 1 "/usr/include/bits/types/FILE.h" 1 3 4

struct _IO_FILE;

typedef struct _IO_FILE FILE;
// # 43 "/usr/include/stdio.h" 2 3 4
// # 1 "/usr/include/bits/types/struct_FILE.h" 1 3 4
// # 35 "/usr/include/bits/types/struct_FILE.h" 3 4
struct _IO_FILE;
struct _IO_marker;
struct _IO_codecvt;
struct _IO_wide_data;

typedef void _IO_lock_t;

struct _IO_FILE {
  int _flags;

  char *_IO_read_ptr;
  char *_IO_read_end;
  char *_IO_read_base;
  char *_IO_write_base;
  char *_IO_write_ptr;
  char *_IO_write_end;
  char *_IO_buf_base;
  char *_IO_buf_end;

  char *_IO_save_base;
  char *_IO_backup_base;
  char *_IO_save_end;

  struct _IO_marker *_markers;

  struct _IO_FILE *_chain;

  int _fileno;
  int _flags2;
  __off_t _old_offset;

  unsigned short _cur_column;
  signed char _vtable_offset;
  char _shortbuf[1];

  _IO_lock_t *_lock;

  __off64_t _offset;

  struct _IO_codecvt *_codecvt;
  struct _IO_wide_data *_wide_data;
  struct _IO_FILE *_freeres_list;
  void *_freeres_buf;
  size_t __pad5;
  int _mode;

  char _unused2[15 * sizeof(int) - 4 * sizeof(void *) - sizeof(size_t)];
};
// # 44 "/usr/include/stdio.h" 2 3 4
// # 52 "/usr/include/stdio.h" 3 4
typedef __gnuc_va_list va_list;
// # 84 "/usr/include/stdio.h" 3 4
typedef __fpos_t fpos_t;
// # 133 "/usr/include/stdio.h" 3 4
// # 1 "/usr/include/bits/stdio_lim.h" 1 3 4
// # 134 "/usr/include/stdio.h" 2 3 4

extern FILE *stdin;
extern FILE *stdout;
extern FILE *stderr;

extern int remove(const char *__filename)
    __attribute__((__nothrow__, __leaf__));

extern int rename(const char *__old, const char *__new)
    __attribute__((__nothrow__, __leaf__));

extern int renameat(int __oldfd, const char *__old, int __newfd,
                    const char *__new) __attribute__((__nothrow__, __leaf__));
// # 173 "/usr/include/stdio.h" 3 4
extern FILE *tmpfile(void);
// # 187 "/usr/include/stdio.h" 3 4
extern char *tmpnam(char *__s) __attribute__((__nothrow__, __leaf__));

extern char *tmpnam_r(char *__s) __attribute__((__nothrow__, __leaf__));
// # 204 "/usr/include/stdio.h" 3 4
extern char *tempnam(const char *__dir, const char *__pfx)
    __attribute__((__nothrow__, __leaf__)) __attribute__((__malloc__));

extern int fclose(FILE *__stream);

extern int fflush(FILE *__stream);
// # 227 "/usr/include/stdio.h" 3 4
extern int fflush_unlocked(FILE *__stream);
// # 246 "/usr/include/stdio.h" 3 4
extern FILE *fopen(const char *__restrict __filename,
                   const char *__restrict __modes);

extern FILE *freopen(const char *__restrict __filename,
                     const char *__restrict __modes, FILE *__restrict __stream);
// # 279 "/usr/include/stdio.h" 3 4
extern FILE *fdopen(int __fd, const char *__modes)
    __attribute__((__nothrow__, __leaf__));
// # 292 "/usr/include/stdio.h" 3 4
extern FILE *fmemopen(void *__s, size_t __len, const char *__modes)
    __attribute__((__nothrow__, __leaf__));

extern FILE *open_memstream(char **__bufloc, size_t *__sizeloc)
    __attribute__((__nothrow__, __leaf__));

extern void setbuf(FILE *__restrict __stream, char *__restrict __buf)
    __attribute__((__nothrow__, __leaf__));

extern int setvbuf(FILE *__restrict __stream, char *__restrict __buf,
                   int __modes, size_t __n)
    __attribute__((__nothrow__, __leaf__));

extern void setbuffer(FILE *__restrict __stream, char *__restrict __buf,
                      size_t __size) __attribute__((__nothrow__, __leaf__));

extern void setlinebuf(FILE *__stream) __attribute__((__nothrow__, __leaf__));

extern int fprintf(FILE *__restrict __stream, const char *__restrict __format,
                   ...);

extern int printf(const char *__restrict __format, ...);

extern int sprintf(char *__restrict __s, const char *__restrict __format, ...)
    __attribute__((__nothrow__));

extern int vfprintf(FILE *__restrict __s, const char *__restrict __format,
                    __gnuc_va_list __arg);

extern int vprintf(const char *__restrict __format, __gnuc_va_list __arg);

extern int vsprintf(char *__restrict __s, const char *__restrict __format,
                    __gnuc_va_list __arg) __attribute__((__nothrow__));

extern int snprintf(char *__restrict __s, size_t __maxlen,
                    const char *__restrict __format, ...)
    __attribute__((__nothrow__)) __attribute__((__format__(__printf__, 3, 4)));

extern int vsnprintf(char *__restrict __s, size_t __maxlen,
                     const char *__restrict __format, __gnuc_va_list __arg)
    __attribute__((__nothrow__)) __attribute__((__format__(__printf__, 3, 0)));
// # 379 "/usr/include/stdio.h" 3 4
extern int vdprintf(int __fd, const char *__restrict __fmt,
                    __gnuc_va_list __arg)
    __attribute__((__format__(__printf__, 2, 0)));
extern int dprintf(int __fd, const char *__restrict __fmt, ...)
    __attribute__((__format__(__printf__, 2, 3)));

extern int fscanf(FILE *__restrict __stream, const char *__restrict __format,
                  ...);

extern int scanf(const char *__restrict __format, ...);

extern int sscanf(const char *__restrict __s, const char *__restrict __format,
                  ...) __attribute__((__nothrow__, __leaf__));
// # 409 "/usr/include/stdio.h" 3 4
extern int fscanf(FILE *__restrict __stream, const char *__restrict __format,
                  ...) __asm__(""
                               "__isoc99_fscanf")

    ;
extern int scanf(const char *__restrict __format,
                 ...) __asm__(""
                              "__isoc99_scanf");
extern int sscanf(const char *__restrict __s, const char *__restrict __format,
                  ...) __asm__(""
                               "__isoc99_sscanf")
    __attribute__((__nothrow__, __leaf__))

    ;
// # 434 "/usr/include/stdio.h" 3 4
extern int vfscanf(FILE *__restrict __s, const char *__restrict __format,
                   __gnuc_va_list __arg)
    __attribute__((__format__(__scanf__, 2, 0)));

extern int vscanf(const char *__restrict __format, __gnuc_va_list __arg)
    __attribute__((__format__(__scanf__, 1, 0)));

extern int vsscanf(const char *__restrict __s, const char *__restrict __format,
                   __gnuc_va_list __arg) __attribute__((__nothrow__, __leaf__))
__attribute__((__format__(__scanf__, 2, 0)));
// # 457 "/usr/include/stdio.h" 3 4
extern int vfscanf(FILE *__restrict __s, const char *__restrict __format,
                   __gnuc_va_list __arg) __asm__(""
                                                 "__isoc99_vfscanf")

    __attribute__((__format__(__scanf__, 2, 0)));
extern int vscanf(const char *__restrict __format,
                  __gnuc_va_list __arg) __asm__(""
                                                "__isoc99_vscanf")

    __attribute__((__format__(__scanf__, 1, 0)));
extern int vsscanf(const char *__restrict __s, const char *__restrict __format,
                   __gnuc_va_list __arg) __asm__(""
                                                 "__isoc99_vsscanf")
    __attribute__((__nothrow__, __leaf__))

    __attribute__((__format__(__scanf__, 2, 0)));
// # 491 "/usr/include/stdio.h" 3 4
extern int fgetc(FILE *__stream);
extern int getc(FILE *__stream);

extern int getchar(void);

extern int getc_unlocked(FILE *__stream);
extern int getchar_unlocked(void);
// # 516 "/usr/include/stdio.h" 3 4
extern int fgetc_unlocked(FILE *__stream);
// # 527 "/usr/include/stdio.h" 3 4
extern int fputc(int __c, FILE *__stream);
extern int putc(int __c, FILE *__stream);

extern int putchar(int __c);
// # 543 "/usr/include/stdio.h" 3 4
extern int fputc_unlocked(int __c, FILE *__stream);

extern int putc_unlocked(int __c, FILE *__stream);
extern int putchar_unlocked(int __c);

extern int getw(FILE *__stream);

extern int putw(int __w, FILE *__stream);

extern char *fgets(char *__restrict __s, int __n, FILE *__restrict __stream);
// # 609 "/usr/include/stdio.h" 3 4
extern __ssize_t __getdelim(char **__restrict __lineptr, size_t *__restrict __n,
                            int __delimiter, FILE *__restrict __stream);
extern __ssize_t getdelim(char **__restrict __lineptr, size_t *__restrict __n,
                          int __delimiter, FILE *__restrict __stream);

extern __ssize_t getline(char **__restrict __lineptr, size_t *__restrict __n,
                         FILE *__restrict __stream);

extern int fputs(const char *__restrict __s, FILE *__restrict __stream);

extern int puts(const char *__s);

extern int ungetc(int __c, FILE *__stream);

extern size_t fread(void *__restrict __ptr, size_t __size, size_t __n,
                    FILE *__restrict __stream);

extern size_t fwrite(const void *__restrict __ptr, size_t __size, size_t __n,
                     FILE *__restrict __s);
// # 679 "/usr/include/stdio.h" 3 4
extern size_t fread_unlocked(void *__restrict __ptr, size_t __size, size_t __n,
                             FILE *__restrict __stream);
extern size_t fwrite_unlocked(const void *__restrict __ptr, size_t __size,
                              size_t __n, FILE *__restrict __stream);

extern int fseek(FILE *__stream, long int __off, int __whence);

extern long int ftell(FILE *__stream);

extern void rewind(FILE *__stream);
// # 713 "/usr/include/stdio.h" 3 4
extern int fseeko(FILE *__stream, __off_t __off, int __whence);

extern __off_t ftello(FILE *__stream);
// # 737 "/usr/include/stdio.h" 3 4
extern int fgetpos(FILE *__restrict __stream, fpos_t *__restrict __pos);

extern int fsetpos(FILE *__stream, const fpos_t *__pos);
// # 763 "/usr/include/stdio.h" 3 4
extern void clearerr(FILE *__stream) __attribute__((__nothrow__, __leaf__));

extern int feof(FILE *__stream) __attribute__((__nothrow__, __leaf__));

extern int ferror(FILE *__stream) __attribute__((__nothrow__, __leaf__));

extern void clearerr_unlocked(FILE *__stream)
    __attribute__((__nothrow__, __leaf__));
extern int feof_unlocked(FILE *__stream) __attribute__((__nothrow__, __leaf__));
extern int ferror_unlocked(FILE *__stream)
    __attribute__((__nothrow__, __leaf__));

extern void perror(const char *__s);

// # 1 "/usr/include/bits/sys_errlist.h" 1 3 4
// # 26 "/usr/include/bits/sys_errlist.h" 3 4
extern int sys_nerr;
extern const char *const sys_errlist[];
// # 788 "/usr/include/stdio.h" 2 3 4

extern int fileno(FILE *__stream) __attribute__((__nothrow__, __leaf__));

extern int fileno_unlocked(FILE *__stream)
    __attribute__((__nothrow__, __leaf__));
// # 806 "/usr/include/stdio.h" 3 4
extern FILE *popen(const char *__command, const char *__modes);

extern int pclose(FILE *__stream);

extern char *ctermid(char *__s) __attribute__((__nothrow__, __leaf__));
// # 846 "/usr/include/stdio.h" 3 4
extern void flockfile(FILE *__stream) __attribute__((__nothrow__, __leaf__));

extern int ftrylockfile(FILE *__stream) __attribute__((__nothrow__, __leaf__));

extern void funlockfile(FILE *__stream) __attribute__((__nothrow__, __leaf__));
// # 864 "/usr/include/stdio.h" 3 4
extern int __uflow(FILE *);
extern int __overflow(FILE *, int);
// # 879 "/usr/include/stdio.h" 3 4

// # 3 "collections.c" 2
// # 1 "/usr/include/string.h" 1 3 4
// # 26 "/usr/include/string.h" 3 4
// # 1 "/usr/include/bits/libc-header-start.h" 1 3 4
// # 27 "/usr/include/string.h" 2 3 4

// # 1 "/usr/lib/gcc/x86_64-pc-linux-gnu/8.2.1/include/stddef.h" 1 3 4
// # 34 "/usr/include/string.h" 2 3 4
// # 42 "/usr/include/string.h" 3 4
extern void *memcpy(void *__restrict __dest, const void *__restrict __src,
                    size_t __n) __attribute__((__nothrow__, __leaf__))
__attribute__((__nonnull__(1, 2)));

extern void *memmove(void *__dest, const void *__src, size_t __n)
    __attribute__((__nothrow__, __leaf__)) __attribute__((__nonnull__(1, 2)));

extern void *memccpy(void *__restrict __dest, const void *__restrict __src,
                     int __c, size_t __n) __attribute__((__nothrow__, __leaf__))
__attribute__((__nonnull__(1, 2)));

extern void *memset(void *__s, int __c, size_t __n)
    __attribute__((__nothrow__, __leaf__)) __attribute__((__nonnull__(1)));

extern int memcmp(const void *__s1, const void *__s2, size_t __n)
    __attribute__((__nothrow__, __leaf__)) __attribute__((__pure__))
    __attribute__((__nonnull__(1, 2)));
// # 90 "/usr/include/string.h" 3 4
extern void *memchr(const void *__s, int __c, size_t __n)
    __attribute__((__nothrow__, __leaf__)) __attribute__((__pure__))
    __attribute__((__nonnull__(1)));
// # 121 "/usr/include/string.h" 3 4
extern char *strcpy(char *__restrict __dest, const char *__restrict __src)
    __attribute__((__nothrow__, __leaf__)) __attribute__((__nonnull__(1, 2)));

extern char *strncpy(char *__restrict __dest, const char *__restrict __src,
                     size_t __n) __attribute__((__nothrow__, __leaf__))
__attribute__((__nonnull__(1, 2)));

extern char *strcat(char *__restrict __dest, const char *__restrict __src)
    __attribute__((__nothrow__, __leaf__)) __attribute__((__nonnull__(1, 2)));

extern char *strncat(char *__restrict __dest, const char *__restrict __src,
                     size_t __n) __attribute__((__nothrow__, __leaf__))
__attribute__((__nonnull__(1, 2)));

extern int strcmp(const char *__s1, const char *__s2)
    __attribute__((__nothrow__, __leaf__)) __attribute__((__pure__))
    __attribute__((__nonnull__(1, 2)));

extern int strncmp(const char *__s1, const char *__s2, size_t __n)
    __attribute__((__nothrow__, __leaf__)) __attribute__((__pure__))
    __attribute__((__nonnull__(1, 2)));

extern int strcoll(const char *__s1, const char *__s2)
    __attribute__((__nothrow__, __leaf__)) __attribute__((__pure__))
    __attribute__((__nonnull__(1, 2)));

extern size_t strxfrm(char *__restrict __dest, const char *__restrict __src,
                      size_t __n) __attribute__((__nothrow__, __leaf__))
__attribute__((__nonnull__(2)));

// # 1 "/usr/include/bits/types/locale_t.h" 1 3 4
// # 22 "/usr/include/bits/types/locale_t.h" 3 4
// # 1 "/usr/include/bits/types/__locale_t.h" 1 3 4
// # 28 "/usr/include/bits/types/__locale_t.h" 3 4
struct __locale_struct {

  struct __locale_data *__locales[13];

  const unsigned short int *__ctype_b;
  const int *__ctype_tolower;
  const int *__ctype_toupper;

  const char *__names[13];
};

typedef struct __locale_struct *__locale_t;
// # 23 "/usr/include/bits/types/locale_t.h" 2 3 4

typedef __locale_t locale_t;
// # 153 "/usr/include/string.h" 2 3 4

extern int strcoll_l(const char *__s1, const char *__s2, locale_t __l)
    __attribute__((__nothrow__, __leaf__)) __attribute__((__pure__))
    __attribute__((__nonnull__(1, 2, 3)));

extern size_t strxfrm_l(char *__dest, const char *__src, size_t __n,
                        locale_t __l) __attribute__((__nothrow__, __leaf__))
__attribute__((__nonnull__(2, 4)));

extern char *strdup(const char *__s) __attribute__((__nothrow__, __leaf__))
__attribute__((__malloc__)) __attribute__((__nonnull__(1)));

extern char *strndup(const char *__string, size_t __n)
    __attribute__((__nothrow__, __leaf__)) __attribute__((__malloc__))
    __attribute__((__nonnull__(1)));
// # 225 "/usr/include/string.h" 3 4
extern char *strchr(const char *__s, int __c)
    __attribute__((__nothrow__, __leaf__)) __attribute__((__pure__))
    __attribute__((__nonnull__(1)));
// # 252 "/usr/include/string.h" 3 4
extern char *strrchr(const char *__s, int __c)
    __attribute__((__nothrow__, __leaf__)) __attribute__((__pure__))
    __attribute__((__nonnull__(1)));
// # 272 "/usr/include/string.h" 3 4
extern size_t strcspn(const char *__s, const char *__reject)
    __attribute__((__nothrow__, __leaf__)) __attribute__((__pure__))
    __attribute__((__nonnull__(1, 2)));

extern size_t strspn(const char *__s, const char *__accept)
    __attribute__((__nothrow__, __leaf__)) __attribute__((__pure__))
    __attribute__((__nonnull__(1, 2)));
// # 302 "/usr/include/string.h" 3 4
extern char *strpbrk(const char *__s, const char *__accept)
    __attribute__((__nothrow__, __leaf__)) __attribute__((__pure__))
    __attribute__((__nonnull__(1, 2)));
// # 329 "/usr/include/string.h" 3 4
extern char *strstr(const char *__haystack, const char *__needle)
    __attribute__((__nothrow__, __leaf__)) __attribute__((__pure__))
    __attribute__((__nonnull__(1, 2)));

extern char *strtok(char *__restrict __s, const char *__restrict __delim)
    __attribute__((__nothrow__, __leaf__)) __attribute__((__nonnull__(2)));

extern char *__strtok_r(char *__restrict __s, const char *__restrict __delim,
                        char **__restrict __save_ptr)
    __attribute__((__nothrow__, __leaf__)) __attribute__((__nonnull__(2, 3)));

extern char *strtok_r(char *__restrict __s, const char *__restrict __delim,
                      char **__restrict __save_ptr)
    __attribute__((__nothrow__, __leaf__)) __attribute__((__nonnull__(2, 3)));
// # 384 "/usr/include/string.h" 3 4
extern size_t strlen(const char *__s) __attribute__((__nothrow__, __leaf__))
__attribute__((__pure__)) __attribute__((__nonnull__(1)));

extern size_t strnlen(const char *__string, size_t __maxlen)
    __attribute__((__nothrow__, __leaf__)) __attribute__((__pure__))
    __attribute__((__nonnull__(1)));

extern char *strerror(int __errnum) __attribute__((__nothrow__, __leaf__));
// # 409 "/usr/include/string.h" 3 4
extern int strerror_r(int __errnum, char *__buf,
                      size_t __buflen) __asm__(""
                                               "__xpg_strerror_r")
    __attribute__((__nothrow__, __leaf__))

    __attribute__((__nonnull__(2)));
// # 427 "/usr/include/string.h" 3 4
extern char *strerror_l(int __errnum, locale_t __l)
    __attribute__((__nothrow__, __leaf__));

// # 1 "/usr/include/strings.h" 1 3 4
// # 23 "/usr/include/strings.h" 3 4
// # 1 "/usr/lib/gcc/x86_64-pc-linux-gnu/8.2.1/include/stddef.h" 1 3 4
// # 24 "/usr/include/strings.h" 2 3 4

extern int bcmp(const void *__s1, const void *__s2, size_t __n)
    __attribute__((__nothrow__, __leaf__)) __attribute__((__pure__))
    __attribute__((__nonnull__(1, 2)));

extern void bcopy(const void *__src, void *__dest, size_t __n)
    __attribute__((__nothrow__, __leaf__)) __attribute__((__nonnull__(1, 2)));

extern void bzero(void *__s, size_t __n) __attribute__((__nothrow__, __leaf__))
__attribute__((__nonnull__(1)));
// # 68 "/usr/include/strings.h" 3 4
extern char *index(const char *__s, int __c)
    __attribute__((__nothrow__, __leaf__)) __attribute__((__pure__))
    __attribute__((__nonnull__(1)));
// # 96 "/usr/include/strings.h" 3 4
extern char *rindex(const char *__s, int __c)
    __attribute__((__nothrow__, __leaf__)) __attribute__((__pure__))
    __attribute__((__nonnull__(1)));

extern int ffs(int __i) __attribute__((__nothrow__, __leaf__))
__attribute__((__const__));

extern int ffsl(long int __l) __attribute__((__nothrow__, __leaf__))
__attribute__((__const__));
__extension__ extern int ffsll(long long int __ll)
    __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));

extern int strcasecmp(const char *__s1, const char *__s2)
    __attribute__((__nothrow__, __leaf__)) __attribute__((__pure__))
    __attribute__((__nonnull__(1, 2)));

extern int strncasecmp(const char *__s1, const char *__s2, size_t __n)
    __attribute__((__nothrow__, __leaf__)) __attribute__((__pure__))
    __attribute__((__nonnull__(1, 2)));

extern int strcasecmp_l(const char *__s1, const char *__s2, locale_t __loc)
    __attribute__((__nothrow__, __leaf__)) __attribute__((__pure__))
    __attribute__((__nonnull__(1, 2, 3)));

extern int strncasecmp_l(const char *__s1, const char *__s2, size_t __n,
                         locale_t __loc) __attribute__((__nothrow__, __leaf__))
__attribute__((__pure__)) __attribute__((__nonnull__(1, 2, 4)));

// # 432 "/usr/include/string.h" 2 3 4

extern void explicit_bzero(void *__s, size_t __n)
    __attribute__((__nothrow__, __leaf__)) __attribute__((__nonnull__(1)));

extern char *strsep(char **__restrict __stringp, const char *__restrict __delim)
    __attribute__((__nothrow__, __leaf__)) __attribute__((__nonnull__(1, 2)));

extern char *strsignal(int __sig) __attribute__((__nothrow__, __leaf__));

extern char *__stpcpy(char *__restrict __dest, const char *__restrict __src)
    __attribute__((__nothrow__, __leaf__)) __attribute__((__nonnull__(1, 2)));
extern char *stpcpy(char *__restrict __dest, const char *__restrict __src)
    __attribute__((__nothrow__, __leaf__)) __attribute__((__nonnull__(1, 2)));

extern char *__stpncpy(char *__restrict __dest, const char *__restrict __src,
                       size_t __n) __attribute__((__nothrow__, __leaf__))
__attribute__((__nonnull__(1, 2)));
extern char *stpncpy(char *__restrict __dest, const char *__restrict __src,
                     size_t __n) __attribute__((__nothrow__, __leaf__))
__attribute__((__nonnull__(1, 2)));
// # 498 "/usr/include/string.h" 3 4

// # 4 "collections.c" 2
// # 1 "/usr/include/ctype.h" 1 3 4
// # 28 "/usr/include/ctype.h" 3 4

// # 46 "/usr/include/ctype.h" 3 4
enum {
  _ISupper = ((0) < 8 ? ((1 << (0)) << 8) : ((1 << (0)) >> 8)),
  _ISlower = ((1) < 8 ? ((1 << (1)) << 8) : ((1 << (1)) >> 8)),
  _ISalpha = ((2) < 8 ? ((1 << (2)) << 8) : ((1 << (2)) >> 8)),
  _ISdigit = ((3) < 8 ? ((1 << (3)) << 8) : ((1 << (3)) >> 8)),
  _ISxdigit = ((4) < 8 ? ((1 << (4)) << 8) : ((1 << (4)) >> 8)),
  _ISspace = ((5) < 8 ? ((1 << (5)) << 8) : ((1 << (5)) >> 8)),
  _ISprint = ((6) < 8 ? ((1 << (6)) << 8) : ((1 << (6)) >> 8)),
  _ISgraph = ((7) < 8 ? ((1 << (7)) << 8) : ((1 << (7)) >> 8)),
  _ISblank = ((8) < 8 ? ((1 << (8)) << 8) : ((1 << (8)) >> 8)),
  _IScntrl = ((9) < 8 ? ((1 << (9)) << 8) : ((1 << (9)) >> 8)),
  _ISpunct = ((10) < 8 ? ((1 << (10)) << 8) : ((1 << (10)) >> 8)),
  _ISalnum = ((11) < 8 ? ((1 << (11)) << 8) : ((1 << (11)) >> 8))
};
// # 79 "/usr/include/ctype.h" 3 4
extern const unsigned short int **__ctype_b_loc(void)
    __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));
extern const __int32_t **__ctype_tolower_loc(void)
    __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));
extern const __int32_t **__ctype_toupper_loc(void)
    __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));
// # 108 "/usr/include/ctype.h" 3 4
extern int isalnum(int) __attribute__((__nothrow__, __leaf__));
extern int isalpha(int) __attribute__((__nothrow__, __leaf__));
extern int iscntrl(int) __attribute__((__nothrow__, __leaf__));
extern int isdigit(int) __attribute__((__nothrow__, __leaf__));
extern int islower(int) __attribute__((__nothrow__, __leaf__));
extern int isgraph(int) __attribute__((__nothrow__, __leaf__));
extern int isprint(int) __attribute__((__nothrow__, __leaf__));
extern int ispunct(int) __attribute__((__nothrow__, __leaf__));
extern int isspace(int) __attribute__((__nothrow__, __leaf__));
extern int isupper(int) __attribute__((__nothrow__, __leaf__));
extern int isxdigit(int) __attribute__((__nothrow__, __leaf__));

extern int tolower(int __c) __attribute__((__nothrow__, __leaf__));

extern int toupper(int __c) __attribute__((__nothrow__, __leaf__));

extern int isblank(int) __attribute__((__nothrow__, __leaf__));
// # 142 "/usr/include/ctype.h" 3 4
extern int isascii(int __c) __attribute__((__nothrow__, __leaf__));

extern int toascii(int __c) __attribute__((__nothrow__, __leaf__));

extern int _toupper(int) __attribute__((__nothrow__, __leaf__));
extern int _tolower(int) __attribute__((__nothrow__, __leaf__));
// # 251 "/usr/include/ctype.h" 3 4
extern int isalnum_l(int, locale_t) __attribute__((__nothrow__, __leaf__));
extern int isalpha_l(int, locale_t) __attribute__((__nothrow__, __leaf__));
extern int iscntrl_l(int, locale_t) __attribute__((__nothrow__, __leaf__));
extern int isdigit_l(int, locale_t) __attribute__((__nothrow__, __leaf__));
extern int islower_l(int, locale_t) __attribute__((__nothrow__, __leaf__));
extern int isgraph_l(int, locale_t) __attribute__((__nothrow__, __leaf__));
extern int isprint_l(int, locale_t) __attribute__((__nothrow__, __leaf__));
extern int ispunct_l(int, locale_t) __attribute__((__nothrow__, __leaf__));
extern int isspace_l(int, locale_t) __attribute__((__nothrow__, __leaf__));
extern int isupper_l(int, locale_t) __attribute__((__nothrow__, __leaf__));
extern int isxdigit_l(int, locale_t) __attribute__((__nothrow__, __leaf__));

extern int isblank_l(int, locale_t) __attribute__((__nothrow__, __leaf__));

extern int __tolower_l(int __c, locale_t __l)
    __attribute__((__nothrow__, __leaf__));
extern int tolower_l(int __c, locale_t __l)
    __attribute__((__nothrow__, __leaf__));

extern int __toupper_l(int __c, locale_t __l)
    __attribute__((__nothrow__, __leaf__));
extern int toupper_l(int __c, locale_t __l)
    __attribute__((__nothrow__, __leaf__));
// # 327 "/usr/include/ctype.h" 3 4

// # 5 "collections.c" 2
// # 1 "/usr/lib/gcc/x86_64-pc-linux-gnu/8.2.1/include/stdarg.h" 1 3 4
// # 6 "collections.c" 2
// # 1 "/usr/include/math.h" 1 3 4
// # 27 "/usr/include/math.h" 3 4
// # 1 "/usr/include/bits/libc-header-start.h" 1 3 4
// # 28 "/usr/include/math.h" 2 3 4

// # 1 "/usr/include/bits/math-vector.h" 1 3 4
// # 25 "/usr/include/bits/math-vector.h" 3 4
// # 1 "/usr/include/bits/libm-simd-decl-stubs.h" 1 3 4
// # 26 "/usr/include/bits/math-vector.h" 2 3 4
// # 41 "/usr/include/math.h" 2 3 4
// # 138 "/usr/include/math.h" 3 4
// # 1 "/usr/include/bits/flt-eval-method.h" 1 3 4
// # 139 "/usr/include/math.h" 2 3 4
// # 149 "/usr/include/math.h" 3 4
typedef float float_t;
typedef double double_t;
// # 190 "/usr/include/math.h" 3 4
// # 1 "/usr/include/bits/fp-logb.h" 1 3 4
// # 191 "/usr/include/math.h" 2 3 4
// # 233 "/usr/include/math.h" 3 4
// # 1 "/usr/include/bits/fp-fast.h" 1 3 4
// # 234 "/usr/include/math.h" 2 3 4
// # 289 "/usr/include/math.h" 3 4
// # 1 "/usr/include/bits/mathcalls-helper-functions.h" 1 3 4
// # 21 "/usr/include/bits/mathcalls-helper-functions.h" 3 4
extern int __fpclassify(double __value) __attribute__((__nothrow__, __leaf__))
__attribute__((__const__));

extern int __signbit(double __value) __attribute__((__nothrow__, __leaf__))
__attribute__((__const__));

extern int __isinf(double __value) __attribute__((__nothrow__, __leaf__))
__attribute__((__const__));

extern int __finite(double __value) __attribute__((__nothrow__, __leaf__))
__attribute__((__const__));

extern int __isnan(double __value) __attribute__((__nothrow__, __leaf__))
__attribute__((__const__));

extern int __iseqsig(double __x, double __y)
    __attribute__((__nothrow__, __leaf__));

extern int __issignaling(double __value) __attribute__((__nothrow__, __leaf__))
__attribute__((__const__));
// # 290 "/usr/include/math.h" 2 3 4
// # 1 "/usr/include/bits/mathcalls.h" 1 3 4
// # 53 "/usr/include/bits/mathcalls.h" 3 4
extern double acos(double __x) __attribute__((__nothrow__, __leaf__));
extern double __acos(double __x) __attribute__((__nothrow__, __leaf__));

extern double asin(double __x) __attribute__((__nothrow__, __leaf__));
extern double __asin(double __x) __attribute__((__nothrow__, __leaf__));

extern double atan(double __x) __attribute__((__nothrow__, __leaf__));
extern double __atan(double __x) __attribute__((__nothrow__, __leaf__));

extern double atan2(double __y, double __x)
    __attribute__((__nothrow__, __leaf__));
extern double __atan2(double __y, double __x)
    __attribute__((__nothrow__, __leaf__));

extern double cos(double __x) __attribute__((__nothrow__, __leaf__));
extern double __cos(double __x) __attribute__((__nothrow__, __leaf__));

extern double sin(double __x) __attribute__((__nothrow__, __leaf__));
extern double __sin(double __x) __attribute__((__nothrow__, __leaf__));

extern double tan(double __x) __attribute__((__nothrow__, __leaf__));
extern double __tan(double __x) __attribute__((__nothrow__, __leaf__));

extern double cosh(double __x) __attribute__((__nothrow__, __leaf__));
extern double __cosh(double __x) __attribute__((__nothrow__, __leaf__));

extern double sinh(double __x) __attribute__((__nothrow__, __leaf__));
extern double __sinh(double __x) __attribute__((__nothrow__, __leaf__));

extern double tanh(double __x) __attribute__((__nothrow__, __leaf__));
extern double __tanh(double __x) __attribute__((__nothrow__, __leaf__));
// # 85 "/usr/include/bits/mathcalls.h" 3 4
extern double acosh(double __x) __attribute__((__nothrow__, __leaf__));
extern double __acosh(double __x) __attribute__((__nothrow__, __leaf__));

extern double asinh(double __x) __attribute__((__nothrow__, __leaf__));
extern double __asinh(double __x) __attribute__((__nothrow__, __leaf__));

extern double atanh(double __x) __attribute__((__nothrow__, __leaf__));
extern double __atanh(double __x) __attribute__((__nothrow__, __leaf__));

extern double exp(double __x) __attribute__((__nothrow__, __leaf__));
extern double __exp(double __x) __attribute__((__nothrow__, __leaf__));

extern double frexp(double __x, int *__exponent)
    __attribute__((__nothrow__, __leaf__));
extern double __frexp(double __x, int *__exponent)
    __attribute__((__nothrow__, __leaf__));

extern double ldexp(double __x, int __exponent)
    __attribute__((__nothrow__, __leaf__));
extern double __ldexp(double __x, int __exponent)
    __attribute__((__nothrow__, __leaf__));

extern double log(double __x) __attribute__((__nothrow__, __leaf__));
extern double __log(double __x) __attribute__((__nothrow__, __leaf__));

extern double log10(double __x) __attribute__((__nothrow__, __leaf__));
extern double __log10(double __x) __attribute__((__nothrow__, __leaf__));

extern double modf(double __x, double *__iptr)
    __attribute__((__nothrow__, __leaf__));
extern double __modf(double __x, double *__iptr)
    __attribute__((__nothrow__, __leaf__)) __attribute__((__nonnull__(2)));
// # 119 "/usr/include/bits/mathcalls.h" 3 4
extern double expm1(double __x) __attribute__((__nothrow__, __leaf__));
extern double __expm1(double __x) __attribute__((__nothrow__, __leaf__));

extern double log1p(double __x) __attribute__((__nothrow__, __leaf__));
extern double __log1p(double __x) __attribute__((__nothrow__, __leaf__));

extern double logb(double __x) __attribute__((__nothrow__, __leaf__));
extern double __logb(double __x) __attribute__((__nothrow__, __leaf__));

extern double exp2(double __x) __attribute__((__nothrow__, __leaf__));
extern double __exp2(double __x) __attribute__((__nothrow__, __leaf__));

extern double log2(double __x) __attribute__((__nothrow__, __leaf__));
extern double __log2(double __x) __attribute__((__nothrow__, __leaf__));

extern double pow(double __x, double __y)
    __attribute__((__nothrow__, __leaf__));
extern double __pow(double __x, double __y)
    __attribute__((__nothrow__, __leaf__));

extern double sqrt(double __x) __attribute__((__nothrow__, __leaf__));
extern double __sqrt(double __x) __attribute__((__nothrow__, __leaf__));

extern double hypot(double __x, double __y)
    __attribute__((__nothrow__, __leaf__));
extern double __hypot(double __x, double __y)
    __attribute__((__nothrow__, __leaf__));

extern double cbrt(double __x) __attribute__((__nothrow__, __leaf__));
extern double __cbrt(double __x) __attribute__((__nothrow__, __leaf__));

extern double ceil(double __x) __attribute__((__nothrow__, __leaf__))
__attribute__((__const__));
extern double __ceil(double __x) __attribute__((__nothrow__, __leaf__))
__attribute__((__const__));

extern double fabs(double __x) __attribute__((__nothrow__, __leaf__))
__attribute__((__const__));
extern double __fabs(double __x) __attribute__((__nothrow__, __leaf__))
__attribute__((__const__));

extern double floor(double __x) __attribute__((__nothrow__, __leaf__))
__attribute__((__const__));
extern double __floor(double __x) __attribute__((__nothrow__, __leaf__))
__attribute__((__const__));

extern double fmod(double __x, double __y)
    __attribute__((__nothrow__, __leaf__));
extern double __fmod(double __x, double __y)
    __attribute__((__nothrow__, __leaf__));
// # 177 "/usr/include/bits/mathcalls.h" 3 4
extern int isinf(double __value) __attribute__((__nothrow__, __leaf__))
__attribute__((__const__));

extern int finite(double __value) __attribute__((__nothrow__, __leaf__))
__attribute__((__const__));

extern double drem(double __x, double __y)
    __attribute__((__nothrow__, __leaf__));
extern double __drem(double __x, double __y)
    __attribute__((__nothrow__, __leaf__));

extern double significand(double __x) __attribute__((__nothrow__, __leaf__));
extern double __significand(double __x) __attribute__((__nothrow__, __leaf__));

extern double copysign(double __x, double __y)
    __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));
extern double __copysign(double __x, double __y)
    __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));

extern double nan(const char *__tagb) __attribute__((__nothrow__, __leaf__));
extern double __nan(const char *__tagb) __attribute__((__nothrow__, __leaf__));
// # 211 "/usr/include/bits/mathcalls.h" 3 4
extern int isnan(double __value) __attribute__((__nothrow__, __leaf__))
__attribute__((__const__));

extern double j0(double) __attribute__((__nothrow__, __leaf__));
extern double __j0(double) __attribute__((__nothrow__, __leaf__));
extern double j1(double) __attribute__((__nothrow__, __leaf__));
extern double __j1(double) __attribute__((__nothrow__, __leaf__));
extern double jn(int, double) __attribute__((__nothrow__, __leaf__));
extern double __jn(int, double) __attribute__((__nothrow__, __leaf__));
extern double y0(double) __attribute__((__nothrow__, __leaf__));
extern double __y0(double) __attribute__((__nothrow__, __leaf__));
extern double y1(double) __attribute__((__nothrow__, __leaf__));
extern double __y1(double) __attribute__((__nothrow__, __leaf__));
extern double yn(int, double) __attribute__((__nothrow__, __leaf__));
extern double __yn(int, double) __attribute__((__nothrow__, __leaf__));

extern double erf(double) __attribute__((__nothrow__, __leaf__));
extern double __erf(double) __attribute__((__nothrow__, __leaf__));
extern double erfc(double) __attribute__((__nothrow__, __leaf__));
extern double __erfc(double) __attribute__((__nothrow__, __leaf__));
extern double lgamma(double) __attribute__((__nothrow__, __leaf__));
extern double __lgamma(double) __attribute__((__nothrow__, __leaf__));

extern double tgamma(double) __attribute__((__nothrow__, __leaf__));
extern double __tgamma(double) __attribute__((__nothrow__, __leaf__));

extern double gamma(double) __attribute__((__nothrow__, __leaf__));
extern double __gamma(double) __attribute__((__nothrow__, __leaf__));

extern double lgamma_r(double, int *__signgamp)
    __attribute__((__nothrow__, __leaf__));
extern double __lgamma_r(double, int *__signgamp)
    __attribute__((__nothrow__, __leaf__));

extern double rint(double __x) __attribute__((__nothrow__, __leaf__));
extern double __rint(double __x) __attribute__((__nothrow__, __leaf__));

extern double nextafter(double __x, double __y)
    __attribute__((__nothrow__, __leaf__));
extern double __nextafter(double __x, double __y)
    __attribute__((__nothrow__, __leaf__));

extern double nexttoward(double __x, long double __y)
    __attribute__((__nothrow__, __leaf__));
extern double __nexttoward(double __x, long double __y)
    __attribute__((__nothrow__, __leaf__));
// # 272 "/usr/include/bits/mathcalls.h" 3 4
extern double remainder(double __x, double __y)
    __attribute__((__nothrow__, __leaf__));
extern double __remainder(double __x, double __y)
    __attribute__((__nothrow__, __leaf__));

extern double scalbn(double __x, int __n)
    __attribute__((__nothrow__, __leaf__));
extern double __scalbn(double __x, int __n)
    __attribute__((__nothrow__, __leaf__));

extern int ilogb(double __x) __attribute__((__nothrow__, __leaf__));
extern int __ilogb(double __x) __attribute__((__nothrow__, __leaf__));
// # 290 "/usr/include/bits/mathcalls.h" 3 4
extern double scalbln(double __x, long int __n)
    __attribute__((__nothrow__, __leaf__));
extern double __scalbln(double __x, long int __n)
    __attribute__((__nothrow__, __leaf__));

extern double nearbyint(double __x) __attribute__((__nothrow__, __leaf__));
extern double __nearbyint(double __x) __attribute__((__nothrow__, __leaf__));

extern double round(double __x) __attribute__((__nothrow__, __leaf__))
__attribute__((__const__));
extern double __round(double __x) __attribute__((__nothrow__, __leaf__))
__attribute__((__const__));

extern double trunc(double __x) __attribute__((__nothrow__, __leaf__))
__attribute__((__const__));
extern double __trunc(double __x) __attribute__((__nothrow__, __leaf__))
__attribute__((__const__));

extern double remquo(double __x, double __y, int *__quo)
    __attribute__((__nothrow__, __leaf__));
extern double __remquo(double __x, double __y, int *__quo)
    __attribute__((__nothrow__, __leaf__));

extern long int lrint(double __x) __attribute__((__nothrow__, __leaf__));
extern long int __lrint(double __x) __attribute__((__nothrow__, __leaf__));
__extension__ extern long long int llrint(double __x)
    __attribute__((__nothrow__, __leaf__));
extern long long int __llrint(double __x)
    __attribute__((__nothrow__, __leaf__));

extern long int lround(double __x) __attribute__((__nothrow__, __leaf__));
extern long int __lround(double __x) __attribute__((__nothrow__, __leaf__));
__extension__ extern long long int llround(double __x)
    __attribute__((__nothrow__, __leaf__));
extern long long int __llround(double __x)
    __attribute__((__nothrow__, __leaf__));

extern double fdim(double __x, double __y)
    __attribute__((__nothrow__, __leaf__));
extern double __fdim(double __x, double __y)
    __attribute__((__nothrow__, __leaf__));

extern double fmax(double __x, double __y)
    __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));
extern double __fmax(double __x, double __y)
    __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));

extern double fmin(double __x, double __y)
    __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));
extern double __fmin(double __x, double __y)
    __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));

extern double fma(double __x, double __y, double __z)
    __attribute__((__nothrow__, __leaf__));
extern double __fma(double __x, double __y, double __z)
    __attribute__((__nothrow__, __leaf__));
// # 396 "/usr/include/bits/mathcalls.h" 3 4
extern double scalb(double __x, double __n)
    __attribute__((__nothrow__, __leaf__));
extern double __scalb(double __x, double __n)
    __attribute__((__nothrow__, __leaf__));
// # 291 "/usr/include/math.h" 2 3 4
// # 306 "/usr/include/math.h" 3 4
// # 1 "/usr/include/bits/mathcalls-helper-functions.h" 1 3 4
// # 21 "/usr/include/bits/mathcalls-helper-functions.h" 3 4
extern int __fpclassifyf(float __value) __attribute__((__nothrow__, __leaf__))
__attribute__((__const__));

extern int __signbitf(float __value) __attribute__((__nothrow__, __leaf__))
__attribute__((__const__));

extern int __isinff(float __value) __attribute__((__nothrow__, __leaf__))
__attribute__((__const__));

extern int __finitef(float __value) __attribute__((__nothrow__, __leaf__))
__attribute__((__const__));

extern int __isnanf(float __value) __attribute__((__nothrow__, __leaf__))
__attribute__((__const__));

extern int __iseqsigf(float __x, float __y)
    __attribute__((__nothrow__, __leaf__));

extern int __issignalingf(float __value) __attribute__((__nothrow__, __leaf__))
__attribute__((__const__));
// # 307 "/usr/include/math.h" 2 3 4
// # 1 "/usr/include/bits/mathcalls.h" 1 3 4
// # 53 "/usr/include/bits/mathcalls.h" 3 4
extern float acosf(float __x) __attribute__((__nothrow__, __leaf__));
extern float __acosf(float __x) __attribute__((__nothrow__, __leaf__));

extern float asinf(float __x) __attribute__((__nothrow__, __leaf__));
extern float __asinf(float __x) __attribute__((__nothrow__, __leaf__));

extern float atanf(float __x) __attribute__((__nothrow__, __leaf__));
extern float __atanf(float __x) __attribute__((__nothrow__, __leaf__));

extern float atan2f(float __y, float __x)
    __attribute__((__nothrow__, __leaf__));
extern float __atan2f(float __y, float __x)
    __attribute__((__nothrow__, __leaf__));

extern float cosf(float __x) __attribute__((__nothrow__, __leaf__));
extern float __cosf(float __x) __attribute__((__nothrow__, __leaf__));

extern float sinf(float __x) __attribute__((__nothrow__, __leaf__));
extern float __sinf(float __x) __attribute__((__nothrow__, __leaf__));

extern float tanf(float __x) __attribute__((__nothrow__, __leaf__));
extern float __tanf(float __x) __attribute__((__nothrow__, __leaf__));

extern float coshf(float __x) __attribute__((__nothrow__, __leaf__));
extern float __coshf(float __x) __attribute__((__nothrow__, __leaf__));

extern float sinhf(float __x) __attribute__((__nothrow__, __leaf__));
extern float __sinhf(float __x) __attribute__((__nothrow__, __leaf__));

extern float tanhf(float __x) __attribute__((__nothrow__, __leaf__));
extern float __tanhf(float __x) __attribute__((__nothrow__, __leaf__));
// # 85 "/usr/include/bits/mathcalls.h" 3 4
extern float acoshf(float __x) __attribute__((__nothrow__, __leaf__));
extern float __acoshf(float __x) __attribute__((__nothrow__, __leaf__));

extern float asinhf(float __x) __attribute__((__nothrow__, __leaf__));
extern float __asinhf(float __x) __attribute__((__nothrow__, __leaf__));

extern float atanhf(float __x) __attribute__((__nothrow__, __leaf__));
extern float __atanhf(float __x) __attribute__((__nothrow__, __leaf__));

extern float expf(float __x) __attribute__((__nothrow__, __leaf__));
extern float __expf(float __x) __attribute__((__nothrow__, __leaf__));

extern float frexpf(float __x, int *__exponent)
    __attribute__((__nothrow__, __leaf__));
extern float __frexpf(float __x, int *__exponent)
    __attribute__((__nothrow__, __leaf__));

extern float ldexpf(float __x, int __exponent)
    __attribute__((__nothrow__, __leaf__));
extern float __ldexpf(float __x, int __exponent)
    __attribute__((__nothrow__, __leaf__));

extern float logf(float __x) __attribute__((__nothrow__, __leaf__));
extern float __logf(float __x) __attribute__((__nothrow__, __leaf__));

extern float log10f(float __x) __attribute__((__nothrow__, __leaf__));
extern float __log10f(float __x) __attribute__((__nothrow__, __leaf__));

extern float modff(float __x, float *__iptr)
    __attribute__((__nothrow__, __leaf__));
extern float __modff(float __x, float *__iptr)
    __attribute__((__nothrow__, __leaf__)) __attribute__((__nonnull__(2)));
// # 119 "/usr/include/bits/mathcalls.h" 3 4
extern float expm1f(float __x) __attribute__((__nothrow__, __leaf__));
extern float __expm1f(float __x) __attribute__((__nothrow__, __leaf__));

extern float log1pf(float __x) __attribute__((__nothrow__, __leaf__));
extern float __log1pf(float __x) __attribute__((__nothrow__, __leaf__));

extern float logbf(float __x) __attribute__((__nothrow__, __leaf__));
extern float __logbf(float __x) __attribute__((__nothrow__, __leaf__));

extern float exp2f(float __x) __attribute__((__nothrow__, __leaf__));
extern float __exp2f(float __x) __attribute__((__nothrow__, __leaf__));

extern float log2f(float __x) __attribute__((__nothrow__, __leaf__));
extern float __log2f(float __x) __attribute__((__nothrow__, __leaf__));

extern float powf(float __x, float __y) __attribute__((__nothrow__, __leaf__));
extern float __powf(float __x, float __y)
    __attribute__((__nothrow__, __leaf__));

extern float sqrtf(float __x) __attribute__((__nothrow__, __leaf__));
extern float __sqrtf(float __x) __attribute__((__nothrow__, __leaf__));

extern float hypotf(float __x, float __y)
    __attribute__((__nothrow__, __leaf__));
extern float __hypotf(float __x, float __y)
    __attribute__((__nothrow__, __leaf__));

extern float cbrtf(float __x) __attribute__((__nothrow__, __leaf__));
extern float __cbrtf(float __x) __attribute__((__nothrow__, __leaf__));

extern float ceilf(float __x) __attribute__((__nothrow__, __leaf__))
__attribute__((__const__));
extern float __ceilf(float __x) __attribute__((__nothrow__, __leaf__))
__attribute__((__const__));

extern float fabsf(float __x) __attribute__((__nothrow__, __leaf__))
__attribute__((__const__));
extern float __fabsf(float __x) __attribute__((__nothrow__, __leaf__))
__attribute__((__const__));

extern float floorf(float __x) __attribute__((__nothrow__, __leaf__))
__attribute__((__const__));
extern float __floorf(float __x) __attribute__((__nothrow__, __leaf__))
__attribute__((__const__));

extern float fmodf(float __x, float __y) __attribute__((__nothrow__, __leaf__));
extern float __fmodf(float __x, float __y)
    __attribute__((__nothrow__, __leaf__));
// # 177 "/usr/include/bits/mathcalls.h" 3 4
extern int isinff(float __value) __attribute__((__nothrow__, __leaf__))
__attribute__((__const__));

extern int finitef(float __value) __attribute__((__nothrow__, __leaf__))
__attribute__((__const__));

extern float dremf(float __x, float __y) __attribute__((__nothrow__, __leaf__));
extern float __dremf(float __x, float __y)
    __attribute__((__nothrow__, __leaf__));

extern float significandf(float __x) __attribute__((__nothrow__, __leaf__));
extern float __significandf(float __x) __attribute__((__nothrow__, __leaf__));

extern float copysignf(float __x, float __y)
    __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));
extern float __copysignf(float __x, float __y)
    __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));

extern float nanf(const char *__tagb) __attribute__((__nothrow__, __leaf__));
extern float __nanf(const char *__tagb) __attribute__((__nothrow__, __leaf__));
// # 211 "/usr/include/bits/mathcalls.h" 3 4
extern int isnanf(float __value) __attribute__((__nothrow__, __leaf__))
__attribute__((__const__));

extern float j0f(float) __attribute__((__nothrow__, __leaf__));
extern float __j0f(float) __attribute__((__nothrow__, __leaf__));
extern float j1f(float) __attribute__((__nothrow__, __leaf__));
extern float __j1f(float) __attribute__((__nothrow__, __leaf__));
extern float jnf(int, float) __attribute__((__nothrow__, __leaf__));
extern float __jnf(int, float) __attribute__((__nothrow__, __leaf__));
extern float y0f(float) __attribute__((__nothrow__, __leaf__));
extern float __y0f(float) __attribute__((__nothrow__, __leaf__));
extern float y1f(float) __attribute__((__nothrow__, __leaf__));
extern float __y1f(float) __attribute__((__nothrow__, __leaf__));
extern float ynf(int, float) __attribute__((__nothrow__, __leaf__));
extern float __ynf(int, float) __attribute__((__nothrow__, __leaf__));

extern float erff(float) __attribute__((__nothrow__, __leaf__));
extern float __erff(float) __attribute__((__nothrow__, __leaf__));
extern float erfcf(float) __attribute__((__nothrow__, __leaf__));
extern float __erfcf(float) __attribute__((__nothrow__, __leaf__));
extern float lgammaf(float) __attribute__((__nothrow__, __leaf__));
extern float __lgammaf(float) __attribute__((__nothrow__, __leaf__));

extern float tgammaf(float) __attribute__((__nothrow__, __leaf__));
extern float __tgammaf(float) __attribute__((__nothrow__, __leaf__));

extern float gammaf(float) __attribute__((__nothrow__, __leaf__));
extern float __gammaf(float) __attribute__((__nothrow__, __leaf__));

extern float lgammaf_r(float, int *__signgamp)
    __attribute__((__nothrow__, __leaf__));
extern float __lgammaf_r(float, int *__signgamp)
    __attribute__((__nothrow__, __leaf__));

extern float rintf(float __x) __attribute__((__nothrow__, __leaf__));
extern float __rintf(float __x) __attribute__((__nothrow__, __leaf__));

extern float nextafterf(float __x, float __y)
    __attribute__((__nothrow__, __leaf__));
extern float __nextafterf(float __x, float __y)
    __attribute__((__nothrow__, __leaf__));

extern float nexttowardf(float __x, long double __y)
    __attribute__((__nothrow__, __leaf__));
extern float __nexttowardf(float __x, long double __y)
    __attribute__((__nothrow__, __leaf__));
// # 272 "/usr/include/bits/mathcalls.h" 3 4
extern float remainderf(float __x, float __y)
    __attribute__((__nothrow__, __leaf__));
extern float __remainderf(float __x, float __y)
    __attribute__((__nothrow__, __leaf__));

extern float scalbnf(float __x, int __n) __attribute__((__nothrow__, __leaf__));
extern float __scalbnf(float __x, int __n)
    __attribute__((__nothrow__, __leaf__));

extern int ilogbf(float __x) __attribute__((__nothrow__, __leaf__));
extern int __ilogbf(float __x) __attribute__((__nothrow__, __leaf__));
// # 290 "/usr/include/bits/mathcalls.h" 3 4
extern float scalblnf(float __x, long int __n)
    __attribute__((__nothrow__, __leaf__));
extern float __scalblnf(float __x, long int __n)
    __attribute__((__nothrow__, __leaf__));

extern float nearbyintf(float __x) __attribute__((__nothrow__, __leaf__));
extern float __nearbyintf(float __x) __attribute__((__nothrow__, __leaf__));

extern float roundf(float __x) __attribute__((__nothrow__, __leaf__))
__attribute__((__const__));
extern float __roundf(float __x) __attribute__((__nothrow__, __leaf__))
__attribute__((__const__));

extern float truncf(float __x) __attribute__((__nothrow__, __leaf__))
__attribute__((__const__));
extern float __truncf(float __x) __attribute__((__nothrow__, __leaf__))
__attribute__((__const__));

extern float remquof(float __x, float __y, int *__quo)
    __attribute__((__nothrow__, __leaf__));
extern float __remquof(float __x, float __y, int *__quo)
    __attribute__((__nothrow__, __leaf__));

extern long int lrintf(float __x) __attribute__((__nothrow__, __leaf__));
extern long int __lrintf(float __x) __attribute__((__nothrow__, __leaf__));
__extension__ extern long long int llrintf(float __x)
    __attribute__((__nothrow__, __leaf__));
extern long long int __llrintf(float __x)
    __attribute__((__nothrow__, __leaf__));

extern long int lroundf(float __x) __attribute__((__nothrow__, __leaf__));
extern long int __lroundf(float __x) __attribute__((__nothrow__, __leaf__));
__extension__ extern long long int llroundf(float __x)
    __attribute__((__nothrow__, __leaf__));
extern long long int __llroundf(float __x)
    __attribute__((__nothrow__, __leaf__));

extern float fdimf(float __x, float __y) __attribute__((__nothrow__, __leaf__));
extern float __fdimf(float __x, float __y)
    __attribute__((__nothrow__, __leaf__));

extern float fmaxf(float __x, float __y) __attribute__((__nothrow__, __leaf__))
__attribute__((__const__));
extern float __fmaxf(float __x, float __y)
    __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));

extern float fminf(float __x, float __y) __attribute__((__nothrow__, __leaf__))
__attribute__((__const__));
extern float __fminf(float __x, float __y)
    __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));

extern float fmaf(float __x, float __y, float __z)
    __attribute__((__nothrow__, __leaf__));
extern float __fmaf(float __x, float __y, float __z)
    __attribute__((__nothrow__, __leaf__));
// # 396 "/usr/include/bits/mathcalls.h" 3 4
extern float scalbf(float __x, float __n)
    __attribute__((__nothrow__, __leaf__));
extern float __scalbf(float __x, float __n)
    __attribute__((__nothrow__, __leaf__));
// # 308 "/usr/include/math.h" 2 3 4
// # 349 "/usr/include/math.h" 3 4
// # 1 "/usr/include/bits/mathcalls-helper-functions.h" 1 3 4
// # 21 "/usr/include/bits/mathcalls-helper-functions.h" 3 4
extern int __fpclassifyl(long double __value)
    __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));

extern int __signbitl(long double __value)
    __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));

extern int __isinfl(long double __value) __attribute__((__nothrow__, __leaf__))
__attribute__((__const__));

extern int __finitel(long double __value) __attribute__((__nothrow__, __leaf__))
__attribute__((__const__));

extern int __isnanl(long double __value) __attribute__((__nothrow__, __leaf__))
__attribute__((__const__));

extern int __iseqsigl(long double __x, long double __y)
    __attribute__((__nothrow__, __leaf__));

extern int __issignalingl(long double __value)
    __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));
// # 350 "/usr/include/math.h" 2 3 4
// # 1 "/usr/include/bits/mathcalls.h" 1 3 4
// # 53 "/usr/include/bits/mathcalls.h" 3 4
extern long double acosl(long double __x)
    __attribute__((__nothrow__, __leaf__));
extern long double __acosl(long double __x)
    __attribute__((__nothrow__, __leaf__));

extern long double asinl(long double __x)
    __attribute__((__nothrow__, __leaf__));
extern long double __asinl(long double __x)
    __attribute__((__nothrow__, __leaf__));

extern long double atanl(long double __x)
    __attribute__((__nothrow__, __leaf__));
extern long double __atanl(long double __x)
    __attribute__((__nothrow__, __leaf__));

extern long double atan2l(long double __y, long double __x)
    __attribute__((__nothrow__, __leaf__));
extern long double __atan2l(long double __y, long double __x)
    __attribute__((__nothrow__, __leaf__));

extern long double cosl(long double __x) __attribute__((__nothrow__, __leaf__));
extern long double __cosl(long double __x)
    __attribute__((__nothrow__, __leaf__));

extern long double sinl(long double __x) __attribute__((__nothrow__, __leaf__));
extern long double __sinl(long double __x)
    __attribute__((__nothrow__, __leaf__));

extern long double tanl(long double __x) __attribute__((__nothrow__, __leaf__));
extern long double __tanl(long double __x)
    __attribute__((__nothrow__, __leaf__));

extern long double coshl(long double __x)
    __attribute__((__nothrow__, __leaf__));
extern long double __coshl(long double __x)
    __attribute__((__nothrow__, __leaf__));

extern long double sinhl(long double __x)
    __attribute__((__nothrow__, __leaf__));
extern long double __sinhl(long double __x)
    __attribute__((__nothrow__, __leaf__));

extern long double tanhl(long double __x)
    __attribute__((__nothrow__, __leaf__));
extern long double __tanhl(long double __x)
    __attribute__((__nothrow__, __leaf__));
// # 85 "/usr/include/bits/mathcalls.h" 3 4
extern long double acoshl(long double __x)
    __attribute__((__nothrow__, __leaf__));
extern long double __acoshl(long double __x)
    __attribute__((__nothrow__, __leaf__));

extern long double asinhl(long double __x)
    __attribute__((__nothrow__, __leaf__));
extern long double __asinhl(long double __x)
    __attribute__((__nothrow__, __leaf__));

extern long double atanhl(long double __x)
    __attribute__((__nothrow__, __leaf__));
extern long double __atanhl(long double __x)
    __attribute__((__nothrow__, __leaf__));

extern long double expl(long double __x) __attribute__((__nothrow__, __leaf__));
extern long double __expl(long double __x)
    __attribute__((__nothrow__, __leaf__));

extern long double frexpl(long double __x, int *__exponent)
    __attribute__((__nothrow__, __leaf__));
extern long double __frexpl(long double __x, int *__exponent)
    __attribute__((__nothrow__, __leaf__));

extern long double ldexpl(long double __x, int __exponent)
    __attribute__((__nothrow__, __leaf__));
extern long double __ldexpl(long double __x, int __exponent)
    __attribute__((__nothrow__, __leaf__));

extern long double logl(long double __x) __attribute__((__nothrow__, __leaf__));
extern long double __logl(long double __x)
    __attribute__((__nothrow__, __leaf__));

extern long double log10l(long double __x)
    __attribute__((__nothrow__, __leaf__));
extern long double __log10l(long double __x)
    __attribute__((__nothrow__, __leaf__));

extern long double modfl(long double __x, long double *__iptr)
    __attribute__((__nothrow__, __leaf__));
extern long double __modfl(long double __x, long double *__iptr)
    __attribute__((__nothrow__, __leaf__)) __attribute__((__nonnull__(2)));
// # 119 "/usr/include/bits/mathcalls.h" 3 4
extern long double expm1l(long double __x)
    __attribute__((__nothrow__, __leaf__));
extern long double __expm1l(long double __x)
    __attribute__((__nothrow__, __leaf__));

extern long double log1pl(long double __x)
    __attribute__((__nothrow__, __leaf__));
extern long double __log1pl(long double __x)
    __attribute__((__nothrow__, __leaf__));

extern long double logbl(long double __x)
    __attribute__((__nothrow__, __leaf__));
extern long double __logbl(long double __x)
    __attribute__((__nothrow__, __leaf__));

extern long double exp2l(long double __x)
    __attribute__((__nothrow__, __leaf__));
extern long double __exp2l(long double __x)
    __attribute__((__nothrow__, __leaf__));

extern long double log2l(long double __x)
    __attribute__((__nothrow__, __leaf__));
extern long double __log2l(long double __x)
    __attribute__((__nothrow__, __leaf__));

extern long double powl(long double __x, long double __y)
    __attribute__((__nothrow__, __leaf__));
extern long double __powl(long double __x, long double __y)
    __attribute__((__nothrow__, __leaf__));

extern long double sqrtl(long double __x)
    __attribute__((__nothrow__, __leaf__));
extern long double __sqrtl(long double __x)
    __attribute__((__nothrow__, __leaf__));

extern long double hypotl(long double __x, long double __y)
    __attribute__((__nothrow__, __leaf__));
extern long double __hypotl(long double __x, long double __y)
    __attribute__((__nothrow__, __leaf__));

extern long double cbrtl(long double __x)
    __attribute__((__nothrow__, __leaf__));
extern long double __cbrtl(long double __x)
    __attribute__((__nothrow__, __leaf__));

extern long double ceill(long double __x) __attribute__((__nothrow__, __leaf__))
__attribute__((__const__));
extern long double __ceill(long double __x)
    __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));

extern long double fabsl(long double __x) __attribute__((__nothrow__, __leaf__))
__attribute__((__const__));
extern long double __fabsl(long double __x)
    __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));

extern long double floorl(long double __x)
    __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));
extern long double __floorl(long double __x)
    __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));

extern long double fmodl(long double __x, long double __y)
    __attribute__((__nothrow__, __leaf__));
extern long double __fmodl(long double __x, long double __y)
    __attribute__((__nothrow__, __leaf__));
// # 177 "/usr/include/bits/mathcalls.h" 3 4
extern int isinfl(long double __value) __attribute__((__nothrow__, __leaf__))
__attribute__((__const__));

extern int finitel(long double __value) __attribute__((__nothrow__, __leaf__))
__attribute__((__const__));

extern long double dreml(long double __x, long double __y)
    __attribute__((__nothrow__, __leaf__));
extern long double __dreml(long double __x, long double __y)
    __attribute__((__nothrow__, __leaf__));

extern long double significandl(long double __x)
    __attribute__((__nothrow__, __leaf__));
extern long double __significandl(long double __x)
    __attribute__((__nothrow__, __leaf__));

extern long double copysignl(long double __x, long double __y)
    __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));
extern long double __copysignl(long double __x, long double __y)
    __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));

extern long double nanl(const char *__tagb)
    __attribute__((__nothrow__, __leaf__));
extern long double __nanl(const char *__tagb)
    __attribute__((__nothrow__, __leaf__));
// # 211 "/usr/include/bits/mathcalls.h" 3 4
extern int isnanl(long double __value) __attribute__((__nothrow__, __leaf__))
__attribute__((__const__));

extern long double j0l(long double) __attribute__((__nothrow__, __leaf__));
extern long double __j0l(long double) __attribute__((__nothrow__, __leaf__));
extern long double j1l(long double) __attribute__((__nothrow__, __leaf__));
extern long double __j1l(long double) __attribute__((__nothrow__, __leaf__));
extern long double jnl(int, long double) __attribute__((__nothrow__, __leaf__));
extern long double __jnl(int, long double)
    __attribute__((__nothrow__, __leaf__));
extern long double y0l(long double) __attribute__((__nothrow__, __leaf__));
extern long double __y0l(long double) __attribute__((__nothrow__, __leaf__));
extern long double y1l(long double) __attribute__((__nothrow__, __leaf__));
extern long double __y1l(long double) __attribute__((__nothrow__, __leaf__));
extern long double ynl(int, long double) __attribute__((__nothrow__, __leaf__));
extern long double __ynl(int, long double)
    __attribute__((__nothrow__, __leaf__));

extern long double erfl(long double) __attribute__((__nothrow__, __leaf__));
extern long double __erfl(long double) __attribute__((__nothrow__, __leaf__));
extern long double erfcl(long double) __attribute__((__nothrow__, __leaf__));
extern long double __erfcl(long double) __attribute__((__nothrow__, __leaf__));
extern long double lgammal(long double) __attribute__((__nothrow__, __leaf__));
extern long double __lgammal(long double)
    __attribute__((__nothrow__, __leaf__));

extern long double tgammal(long double) __attribute__((__nothrow__, __leaf__));
extern long double __tgammal(long double)
    __attribute__((__nothrow__, __leaf__));

extern long double gammal(long double) __attribute__((__nothrow__, __leaf__));
extern long double __gammal(long double) __attribute__((__nothrow__, __leaf__));

extern long double lgammal_r(long double, int *__signgamp)
    __attribute__((__nothrow__, __leaf__));
extern long double __lgammal_r(long double, int *__signgamp)
    __attribute__((__nothrow__, __leaf__));

extern long double rintl(long double __x)
    __attribute__((__nothrow__, __leaf__));
extern long double __rintl(long double __x)
    __attribute__((__nothrow__, __leaf__));

extern long double nextafterl(long double __x, long double __y)
    __attribute__((__nothrow__, __leaf__));
extern long double __nextafterl(long double __x, long double __y)
    __attribute__((__nothrow__, __leaf__));

extern long double nexttowardl(long double __x, long double __y)
    __attribute__((__nothrow__, __leaf__));
extern long double __nexttowardl(long double __x, long double __y)
    __attribute__((__nothrow__, __leaf__));
// # 272 "/usr/include/bits/mathcalls.h" 3 4
extern long double remainderl(long double __x, long double __y)
    __attribute__((__nothrow__, __leaf__));
extern long double __remainderl(long double __x, long double __y)
    __attribute__((__nothrow__, __leaf__));

extern long double scalbnl(long double __x, int __n)
    __attribute__((__nothrow__, __leaf__));
extern long double __scalbnl(long double __x, int __n)
    __attribute__((__nothrow__, __leaf__));

extern int ilogbl(long double __x) __attribute__((__nothrow__, __leaf__));
extern int __ilogbl(long double __x) __attribute__((__nothrow__, __leaf__));
// # 290 "/usr/include/bits/mathcalls.h" 3 4
extern long double scalblnl(long double __x, long int __n)
    __attribute__((__nothrow__, __leaf__));
extern long double __scalblnl(long double __x, long int __n)
    __attribute__((__nothrow__, __leaf__));

extern long double nearbyintl(long double __x)
    __attribute__((__nothrow__, __leaf__));
extern long double __nearbyintl(long double __x)
    __attribute__((__nothrow__, __leaf__));

extern long double roundl(long double __x)
    __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));
extern long double __roundl(long double __x)
    __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));

extern long double truncl(long double __x)
    __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));
extern long double __truncl(long double __x)
    __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));

extern long double remquol(long double __x, long double __y, int *__quo)
    __attribute__((__nothrow__, __leaf__));
extern long double __remquol(long double __x, long double __y, int *__quo)
    __attribute__((__nothrow__, __leaf__));

extern long int lrintl(long double __x) __attribute__((__nothrow__, __leaf__));
extern long int __lrintl(long double __x)
    __attribute__((__nothrow__, __leaf__));
__extension__ extern long long int llrintl(long double __x)
    __attribute__((__nothrow__, __leaf__));
extern long long int __llrintl(long double __x)
    __attribute__((__nothrow__, __leaf__));

extern long int lroundl(long double __x) __attribute__((__nothrow__, __leaf__));
extern long int __lroundl(long double __x)
    __attribute__((__nothrow__, __leaf__));
__extension__ extern long long int llroundl(long double __x)
    __attribute__((__nothrow__, __leaf__));
extern long long int __llroundl(long double __x)
    __attribute__((__nothrow__, __leaf__));

extern long double fdiml(long double __x, long double __y)
    __attribute__((__nothrow__, __leaf__));
extern long double __fdiml(long double __x, long double __y)
    __attribute__((__nothrow__, __leaf__));

extern long double fmaxl(long double __x, long double __y)
    __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));
extern long double __fmaxl(long double __x, long double __y)
    __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));

extern long double fminl(long double __x, long double __y)
    __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));
extern long double __fminl(long double __x, long double __y)
    __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));

extern long double fmal(long double __x, long double __y, long double __z)
    __attribute__((__nothrow__, __leaf__));
extern long double __fmal(long double __x, long double __y, long double __z)
    __attribute__((__nothrow__, __leaf__));
// # 396 "/usr/include/bits/mathcalls.h" 3 4
extern long double scalbl(long double __x, long double __n)
    __attribute__((__nothrow__, __leaf__));
extern long double __scalbl(long double __x, long double __n)
    __attribute__((__nothrow__, __leaf__));
// # 351 "/usr/include/math.h" 2 3 4
// # 420 "/usr/include/math.h" 3 4
// # 1 "/usr/include/bits/mathcalls-helper-functions.h" 1 3 4
// # 21 "/usr/include/bits/mathcalls-helper-functions.h" 3 4
extern int __fpclassifyf128(_Float128 __value)
    __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));

extern int __signbitf128(_Float128 __value)
    __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));

extern int __isinff128(_Float128 __value) __attribute__((__nothrow__, __leaf__))
__attribute__((__const__));

extern int __finitef128(_Float128 __value)
    __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));

extern int __isnanf128(_Float128 __value) __attribute__((__nothrow__, __leaf__))
__attribute__((__const__));

extern int __iseqsigf128(_Float128 __x, _Float128 __y)
    __attribute__((__nothrow__, __leaf__));

extern int __issignalingf128(_Float128 __value)
    __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));
// # 421 "/usr/include/math.h" 2 3 4
// # 773 "/usr/include/math.h" 3 4
extern int signgam;
// # 853 "/usr/include/math.h" 3 4
enum {
  FP_NAN =

      0,
  FP_INFINITE =

      1,
  FP_ZERO =

      2,
  FP_SUBNORMAL =

      3,
  FP_NORMAL =

      4
};
// # 1553 "/usr/include/math.h" 3 4

// # 7 "collections.c" 2
// # 1 "/usr/include/unistd.h" 1 3 4
// # 27 "/usr/include/unistd.h" 3 4

// # 202 "/usr/include/unistd.h" 3 4
// # 1 "/usr/include/bits/posix_opt.h" 1 3 4
// # 203 "/usr/include/unistd.h" 2 3 4

// # 1 "/usr/include/bits/environments.h" 1 3 4
// # 22 "/usr/include/bits/environments.h" 3 4
// # 1 "/usr/include/bits/wordsize.h" 1 3 4
// # 23 "/usr/include/bits/environments.h" 2 3 4
// # 207 "/usr/include/unistd.h" 2 3 4
// # 226 "/usr/include/unistd.h" 3 4
// # 1 "/usr/lib/gcc/x86_64-pc-linux-gnu/8.2.1/include/stddef.h" 1 3 4
// # 227 "/usr/include/unistd.h" 2 3 4
// # 255 "/usr/include/unistd.h" 3 4
typedef __useconds_t useconds_t;
// # 267 "/usr/include/unistd.h" 3 4
typedef __intptr_t intptr_t;

typedef __socklen_t socklen_t;
// # 287 "/usr/include/unistd.h" 3 4
extern int access(const char *__name, int __type)
    __attribute__((__nothrow__, __leaf__)) __attribute__((__nonnull__(1)));
// # 304 "/usr/include/unistd.h" 3 4
extern int faccessat(int __fd, const char *__file, int __type, int __flag)
    __attribute__((__nothrow__, __leaf__)) __attribute__((__nonnull__(2)));
// # 334 "/usr/include/unistd.h" 3 4
extern __off_t lseek(int __fd, __off_t __offset, int __whence)
    __attribute__((__nothrow__, __leaf__));
// # 353 "/usr/include/unistd.h" 3 4
extern int close(int __fd);

extern ssize_t read(int __fd, void *__buf, size_t __nbytes);

extern ssize_t write(int __fd, const void *__buf, size_t __n);
// # 376 "/usr/include/unistd.h" 3 4
extern ssize_t pread(int __fd, void *__buf, size_t __nbytes, __off_t __offset);

extern ssize_t pwrite(int __fd, const void *__buf, size_t __n,
                      __off_t __offset);
// # 417 "/usr/include/unistd.h" 3 4
extern int pipe(int __pipedes[2]) __attribute__((__nothrow__, __leaf__));
// # 432 "/usr/include/unistd.h" 3 4
extern unsigned int alarm(unsigned int __seconds)
    __attribute__((__nothrow__, __leaf__));
// # 444 "/usr/include/unistd.h" 3 4
extern unsigned int sleep(unsigned int __seconds);

extern __useconds_t ualarm(__useconds_t __value, __useconds_t __interval)
    __attribute__((__nothrow__, __leaf__));

extern int usleep(__useconds_t __useconds);
// # 469 "/usr/include/unistd.h" 3 4
extern int pause(void);

extern int chown(const char *__file, __uid_t __owner, __gid_t __group)
    __attribute__((__nothrow__, __leaf__)) __attribute__((__nonnull__(1)));

extern int fchown(int __fd, __uid_t __owner, __gid_t __group)
    __attribute__((__nothrow__, __leaf__));

extern int lchown(const char *__file, __uid_t __owner, __gid_t __group)
    __attribute__((__nothrow__, __leaf__)) __attribute__((__nonnull__(1)));

extern int fchownat(int __fd, const char *__file, __uid_t __owner,
                    __gid_t __group, int __flag)
    __attribute__((__nothrow__, __leaf__)) __attribute__((__nonnull__(2)));

extern int chdir(const char *__path) __attribute__((__nothrow__, __leaf__))
__attribute__((__nonnull__(1)));

extern int fchdir(int __fd) __attribute__((__nothrow__, __leaf__));
// # 511 "/usr/include/unistd.h" 3 4
extern char *getcwd(char *__buf, size_t __size)
    __attribute__((__nothrow__, __leaf__));
// # 525 "/usr/include/unistd.h" 3 4
extern char *getwd(char *__buf) __attribute__((__nothrow__, __leaf__))
__attribute__((__nonnull__(1))) __attribute__((__deprecated__));

extern int dup(int __fd) __attribute__((__nothrow__, __leaf__));

extern int dup2(int __fd, int __fd2) __attribute__((__nothrow__, __leaf__));
// # 543 "/usr/include/unistd.h" 3 4
extern char **__environ;

extern int execve(const char *__path, char *const __argv[],
                  char *const __envp[]) __attribute__((__nothrow__, __leaf__))
__attribute__((__nonnull__(1, 2)));

extern int fexecve(int __fd, char *const __argv[], char *const __envp[])
    __attribute__((__nothrow__, __leaf__)) __attribute__((__nonnull__(2)));

extern int execv(const char *__path, char *const __argv[])
    __attribute__((__nothrow__, __leaf__)) __attribute__((__nonnull__(1, 2)));

extern int execle(const char *__path, const char *__arg, ...)
    __attribute__((__nothrow__, __leaf__)) __attribute__((__nonnull__(1, 2)));

extern int execl(const char *__path, const char *__arg, ...)
    __attribute__((__nothrow__, __leaf__)) __attribute__((__nonnull__(1, 2)));

extern int execvp(const char *__file, char *const __argv[])
    __attribute__((__nothrow__, __leaf__)) __attribute__((__nonnull__(1, 2)));

extern int execlp(const char *__file, const char *__arg, ...)
    __attribute__((__nothrow__, __leaf__)) __attribute__((__nonnull__(1, 2)));
// # 598 "/usr/include/unistd.h" 3 4
extern int nice(int __inc) __attribute__((__nothrow__, __leaf__));

extern void _exit(int __status) __attribute__((__noreturn__));

// # 1 "/usr/include/bits/confname.h" 1 3 4
// # 24 "/usr/include/bits/confname.h" 3 4
enum {
  _PC_LINK_MAX,

  _PC_MAX_CANON,

  _PC_MAX_INPUT,

  _PC_NAME_MAX,

  _PC_PATH_MAX,

  _PC_PIPE_BUF,

  _PC_CHOWN_RESTRICTED,

  _PC_NO_TRUNC,

  _PC_VDISABLE,

  _PC_SYNC_IO,

  _PC_ASYNC_IO,

  _PC_PRIO_IO,

  _PC_SOCK_MAXBUF,

  _PC_FILESIZEBITS,

  _PC_REC_INCR_XFER_SIZE,

  _PC_REC_MAX_XFER_SIZE,

  _PC_REC_MIN_XFER_SIZE,

  _PC_REC_XFER_ALIGN,

  _PC_ALLOC_SIZE_MIN,

  _PC_SYMLINK_MAX,

  _PC_2_SYMLINKS

};

enum {
  _SC_ARG_MAX,

  _SC_CHILD_MAX,

  _SC_CLK_TCK,

  _SC_NGROUPS_MAX,

  _SC_OPEN_MAX,

  _SC_STREAM_MAX,

  _SC_TZNAME_MAX,

  _SC_JOB_CONTROL,

  _SC_SAVED_IDS,

  _SC_REALTIME_SIGNALS,

  _SC_PRIORITY_SCHEDULING,

  _SC_TIMERS,

  _SC_ASYNCHRONOUS_IO,

  _SC_PRIORITIZED_IO,

  _SC_SYNCHRONIZED_IO,

  _SC_FSYNC,

  _SC_MAPPED_FILES,

  _SC_MEMLOCK,

  _SC_MEMLOCK_RANGE,

  _SC_MEMORY_PROTECTION,

  _SC_MESSAGE_PASSING,

  _SC_SEMAPHORES,

  _SC_SHARED_MEMORY_OBJECTS,

  _SC_AIO_LISTIO_MAX,

  _SC_AIO_MAX,

  _SC_AIO_PRIO_DELTA_MAX,

  _SC_DELAYTIMER_MAX,

  _SC_MQ_OPEN_MAX,

  _SC_MQ_PRIO_MAX,

  _SC_VERSION,

  _SC_PAGESIZE,

  _SC_RTSIG_MAX,

  _SC_SEM_NSEMS_MAX,

  _SC_SEM_VALUE_MAX,

  _SC_SIGQUEUE_MAX,

  _SC_TIMER_MAX,

  _SC_BC_BASE_MAX,

  _SC_BC_DIM_MAX,

  _SC_BC_SCALE_MAX,

  _SC_BC_STRING_MAX,

  _SC_COLL_WEIGHTS_MAX,

  _SC_EQUIV_CLASS_MAX,

  _SC_EXPR_NEST_MAX,

  _SC_LINE_MAX,

  _SC_RE_DUP_MAX,

  _SC_CHARCLASS_NAME_MAX,

  _SC_2_VERSION,

  _SC_2_C_BIND,

  _SC_2_C_DEV,

  _SC_2_FORT_DEV,

  _SC_2_FORT_RUN,

  _SC_2_SW_DEV,

  _SC_2_LOCALEDEF,

  _SC_PII,

  _SC_PII_XTI,

  _SC_PII_SOCKET,

  _SC_PII_INTERNET,

  _SC_PII_OSI,

  _SC_POLL,

  _SC_SELECT,

  _SC_UIO_MAXIOV,

  _SC_IOV_MAX = _SC_UIO_MAXIOV,

  _SC_PII_INTERNET_STREAM,

  _SC_PII_INTERNET_DGRAM,

  _SC_PII_OSI_COTS,

  _SC_PII_OSI_CLTS,

  _SC_PII_OSI_M,

  _SC_T_IOV_MAX,

  _SC_THREADS,

  _SC_THREAD_SAFE_FUNCTIONS,

  _SC_GETGR_R_SIZE_MAX,

  _SC_GETPW_R_SIZE_MAX,

  _SC_LOGIN_NAME_MAX,

  _SC_TTY_NAME_MAX,

  _SC_THREAD_DESTRUCTOR_ITERATIONS,

  _SC_THREAD_KEYS_MAX,

  _SC_THREAD_STACK_MIN,

  _SC_THREAD_THREADS_MAX,

  _SC_THREAD_ATTR_STACKADDR,

  _SC_THREAD_ATTR_STACKSIZE,

  _SC_THREAD_PRIORITY_SCHEDULING,

  _SC_THREAD_PRIO_INHERIT,

  _SC_THREAD_PRIO_PROTECT,

  _SC_THREAD_PROCESS_SHARED,

  _SC_NPROCESSORS_CONF,

  _SC_NPROCESSORS_ONLN,

  _SC_PHYS_PAGES,

  _SC_AVPHYS_PAGES,

  _SC_ATEXIT_MAX,

  _SC_PASS_MAX,

  _SC_XOPEN_VERSION,

  _SC_XOPEN_XCU_VERSION,

  _SC_XOPEN_UNIX,

  _SC_XOPEN_CRYPT,

  _SC_XOPEN_ENH_I18N,

  _SC_XOPEN_SHM,

  _SC_2_CHAR_TERM,

  _SC_2_C_VERSION,

  _SC_2_UPE,

  _SC_XOPEN_XPG2,

  _SC_XOPEN_XPG3,

  _SC_XOPEN_XPG4,

  _SC_CHAR_BIT,

  _SC_CHAR_MAX,

  _SC_CHAR_MIN,

  _SC_INT_MAX,

  _SC_INT_MIN,

  _SC_LONG_BIT,

  _SC_WORD_BIT,

  _SC_MB_LEN_MAX,

  _SC_NZERO,

  _SC_SSIZE_MAX,

  _SC_SCHAR_MAX,

  _SC_SCHAR_MIN,

  _SC_SHRT_MAX,

  _SC_SHRT_MIN,

  _SC_UCHAR_MAX,

  _SC_UINT_MAX,

  _SC_ULONG_MAX,

  _SC_USHRT_MAX,

  _SC_NL_ARGMAX,

  _SC_NL_LANGMAX,

  _SC_NL_MSGMAX,

  _SC_NL_NMAX,

  _SC_NL_SETMAX,

  _SC_NL_TEXTMAX,

  _SC_XBS5_ILP32_OFF32,

  _SC_XBS5_ILP32_OFFBIG,

  _SC_XBS5_LP64_OFF64,

  _SC_XBS5_LPBIG_OFFBIG,

  _SC_XOPEN_LEGACY,

  _SC_XOPEN_REALTIME,

  _SC_XOPEN_REALTIME_THREADS,

  _SC_ADVISORY_INFO,

  _SC_BARRIERS,

  _SC_BASE,

  _SC_C_LANG_SUPPORT,

  _SC_C_LANG_SUPPORT_R,

  _SC_CLOCK_SELECTION,

  _SC_CPUTIME,

  _SC_THREAD_CPUTIME,

  _SC_DEVICE_IO,

  _SC_DEVICE_SPECIFIC,

  _SC_DEVICE_SPECIFIC_R,

  _SC_FD_MGMT,

  _SC_FIFO,

  _SC_PIPE,

  _SC_FILE_ATTRIBUTES,

  _SC_FILE_LOCKING,

  _SC_FILE_SYSTEM,

  _SC_MONOTONIC_CLOCK,

  _SC_MULTI_PROCESS,

  _SC_SINGLE_PROCESS,

  _SC_NETWORKING,

  _SC_READER_WRITER_LOCKS,

  _SC_SPIN_LOCKS,

  _SC_REGEXP,

  _SC_REGEX_VERSION,

  _SC_SHELL,

  _SC_SIGNALS,

  _SC_SPAWN,

  _SC_SPORADIC_SERVER,

  _SC_THREAD_SPORADIC_SERVER,

  _SC_SYSTEM_DATABASE,

  _SC_SYSTEM_DATABASE_R,

  _SC_TIMEOUTS,

  _SC_TYPED_MEMORY_OBJECTS,

  _SC_USER_GROUPS,

  _SC_USER_GROUPS_R,

  _SC_2_PBS,

  _SC_2_PBS_ACCOUNTING,

  _SC_2_PBS_LOCATE,

  _SC_2_PBS_MESSAGE,

  _SC_2_PBS_TRACK,

  _SC_SYMLOOP_MAX,

  _SC_STREAMS,

  _SC_2_PBS_CHECKPOINT,

  _SC_V6_ILP32_OFF32,

  _SC_V6_ILP32_OFFBIG,

  _SC_V6_LP64_OFF64,

  _SC_V6_LPBIG_OFFBIG,

  _SC_HOST_NAME_MAX,

  _SC_TRACE,

  _SC_TRACE_EVENT_FILTER,

  _SC_TRACE_INHERIT,

  _SC_TRACE_LOG,

  _SC_LEVEL1_ICACHE_SIZE,

  _SC_LEVEL1_ICACHE_ASSOC,

  _SC_LEVEL1_ICACHE_LINESIZE,

  _SC_LEVEL1_DCACHE_SIZE,

  _SC_LEVEL1_DCACHE_ASSOC,

  _SC_LEVEL1_DCACHE_LINESIZE,

  _SC_LEVEL2_CACHE_SIZE,

  _SC_LEVEL2_CACHE_ASSOC,

  _SC_LEVEL2_CACHE_LINESIZE,

  _SC_LEVEL3_CACHE_SIZE,

  _SC_LEVEL3_CACHE_ASSOC,

  _SC_LEVEL3_CACHE_LINESIZE,

  _SC_LEVEL4_CACHE_SIZE,

  _SC_LEVEL4_CACHE_ASSOC,

  _SC_LEVEL4_CACHE_LINESIZE,

  _SC_IPV6 = _SC_LEVEL1_ICACHE_SIZE + 50,

  _SC_RAW_SOCKETS,

  _SC_V7_ILP32_OFF32,

  _SC_V7_ILP32_OFFBIG,

  _SC_V7_LP64_OFF64,

  _SC_V7_LPBIG_OFFBIG,

  _SC_SS_REPL_MAX,

  _SC_TRACE_EVENT_NAME_MAX,

  _SC_TRACE_NAME_MAX,

  _SC_TRACE_SYS_MAX,

  _SC_TRACE_USER_EVENT_MAX,

  _SC_XOPEN_STREAMS,

  _SC_THREAD_ROBUST_PRIO_INHERIT,

  _SC_THREAD_ROBUST_PRIO_PROTECT

};

enum {
  _CS_PATH,

  _CS_V6_WIDTH_RESTRICTED_ENVS,

  _CS_GNU_LIBC_VERSION,

  _CS_GNU_LIBPTHREAD_VERSION,

  _CS_V5_WIDTH_RESTRICTED_ENVS,

  _CS_V7_WIDTH_RESTRICTED_ENVS,

  _CS_LFS_CFLAGS = 1000,

  _CS_LFS_LDFLAGS,

  _CS_LFS_LIBS,

  _CS_LFS_LINTFLAGS,

  _CS_LFS64_CFLAGS,

  _CS_LFS64_LDFLAGS,

  _CS_LFS64_LIBS,

  _CS_LFS64_LINTFLAGS,

  _CS_XBS5_ILP32_OFF32_CFLAGS = 1100,

  _CS_XBS5_ILP32_OFF32_LDFLAGS,

  _CS_XBS5_ILP32_OFF32_LIBS,

  _CS_XBS5_ILP32_OFF32_LINTFLAGS,

  _CS_XBS5_ILP32_OFFBIG_CFLAGS,

  _CS_XBS5_ILP32_OFFBIG_LDFLAGS,

  _CS_XBS5_ILP32_OFFBIG_LIBS,

  _CS_XBS5_ILP32_OFFBIG_LINTFLAGS,

  _CS_XBS5_LP64_OFF64_CFLAGS,

  _CS_XBS5_LP64_OFF64_LDFLAGS,

  _CS_XBS5_LP64_OFF64_LIBS,

  _CS_XBS5_LP64_OFF64_LINTFLAGS,

  _CS_XBS5_LPBIG_OFFBIG_CFLAGS,

  _CS_XBS5_LPBIG_OFFBIG_LDFLAGS,

  _CS_XBS5_LPBIG_OFFBIG_LIBS,

  _CS_XBS5_LPBIG_OFFBIG_LINTFLAGS,

  _CS_POSIX_V6_ILP32_OFF32_CFLAGS,

  _CS_POSIX_V6_ILP32_OFF32_LDFLAGS,

  _CS_POSIX_V6_ILP32_OFF32_LIBS,

  _CS_POSIX_V6_ILP32_OFF32_LINTFLAGS,

  _CS_POSIX_V6_ILP32_OFFBIG_CFLAGS,

  _CS_POSIX_V6_ILP32_OFFBIG_LDFLAGS,

  _CS_POSIX_V6_ILP32_OFFBIG_LIBS,

  _CS_POSIX_V6_ILP32_OFFBIG_LINTFLAGS,

  _CS_POSIX_V6_LP64_OFF64_CFLAGS,

  _CS_POSIX_V6_LP64_OFF64_LDFLAGS,

  _CS_POSIX_V6_LP64_OFF64_LIBS,

  _CS_POSIX_V6_LP64_OFF64_LINTFLAGS,

  _CS_POSIX_V6_LPBIG_OFFBIG_CFLAGS,

  _CS_POSIX_V6_LPBIG_OFFBIG_LDFLAGS,

  _CS_POSIX_V6_LPBIG_OFFBIG_LIBS,

  _CS_POSIX_V6_LPBIG_OFFBIG_LINTFLAGS,

  _CS_POSIX_V7_ILP32_OFF32_CFLAGS,

  _CS_POSIX_V7_ILP32_OFF32_LDFLAGS,

  _CS_POSIX_V7_ILP32_OFF32_LIBS,

  _CS_POSIX_V7_ILP32_OFF32_LINTFLAGS,

  _CS_POSIX_V7_ILP32_OFFBIG_CFLAGS,

  _CS_POSIX_V7_ILP32_OFFBIG_LDFLAGS,

  _CS_POSIX_V7_ILP32_OFFBIG_LIBS,

  _CS_POSIX_V7_ILP32_OFFBIG_LINTFLAGS,

  _CS_POSIX_V7_LP64_OFF64_CFLAGS,

  _CS_POSIX_V7_LP64_OFF64_LDFLAGS,

  _CS_POSIX_V7_LP64_OFF64_LIBS,

  _CS_POSIX_V7_LP64_OFF64_LINTFLAGS,

  _CS_POSIX_V7_LPBIG_OFFBIG_CFLAGS,

  _CS_POSIX_V7_LPBIG_OFFBIG_LDFLAGS,

  _CS_POSIX_V7_LPBIG_OFFBIG_LIBS,

  _CS_POSIX_V7_LPBIG_OFFBIG_LINTFLAGS,

  _CS_V6_ENV,

  _CS_V7_ENV

};
// # 610 "/usr/include/unistd.h" 2 3 4

extern long int pathconf(const char *__path, int __name)
    __attribute__((__nothrow__, __leaf__)) __attribute__((__nonnull__(1)));

extern long int fpathconf(int __fd, int __name)
    __attribute__((__nothrow__, __leaf__));

extern long int sysconf(int __name) __attribute__((__nothrow__, __leaf__));

extern size_t confstr(int __name, char *__buf, size_t __len)
    __attribute__((__nothrow__, __leaf__));

extern __pid_t getpid(void) __attribute__((__nothrow__, __leaf__));

extern __pid_t getppid(void) __attribute__((__nothrow__, __leaf__));

extern __pid_t getpgrp(void) __attribute__((__nothrow__, __leaf__));

extern __pid_t __getpgid(__pid_t __pid) __attribute__((__nothrow__, __leaf__));

extern __pid_t getpgid(__pid_t __pid) __attribute__((__nothrow__, __leaf__));

extern int setpgid(__pid_t __pid, __pid_t __pgid)
    __attribute__((__nothrow__, __leaf__));
// # 660 "/usr/include/unistd.h" 3 4
extern int setpgrp(void) __attribute__((__nothrow__, __leaf__));

extern __pid_t setsid(void) __attribute__((__nothrow__, __leaf__));

extern __pid_t getsid(__pid_t __pid) __attribute__((__nothrow__, __leaf__));

extern __uid_t getuid(void) __attribute__((__nothrow__, __leaf__));

extern __uid_t geteuid(void) __attribute__((__nothrow__, __leaf__));

extern __gid_t getgid(void) __attribute__((__nothrow__, __leaf__));

extern __gid_t getegid(void) __attribute__((__nothrow__, __leaf__));

extern int getgroups(int __size, __gid_t __list[])
    __attribute__((__nothrow__, __leaf__));
// # 700 "/usr/include/unistd.h" 3 4
extern int setuid(__uid_t __uid) __attribute__((__nothrow__, __leaf__));

extern int setreuid(__uid_t __ruid, __uid_t __euid)
    __attribute__((__nothrow__, __leaf__));

extern int seteuid(__uid_t __uid) __attribute__((__nothrow__, __leaf__));

extern int setgid(__gid_t __gid) __attribute__((__nothrow__, __leaf__));

extern int setregid(__gid_t __rgid, __gid_t __egid)
    __attribute__((__nothrow__, __leaf__));

extern int setegid(__gid_t __gid) __attribute__((__nothrow__, __leaf__));
// # 756 "/usr/include/unistd.h" 3 4
extern __pid_t fork(void) __attribute__((__nothrow__));

extern __pid_t vfork(void) __attribute__((__nothrow__, __leaf__));

extern char *ttyname(int __fd) __attribute__((__nothrow__, __leaf__));

extern int ttyname_r(int __fd, char *__buf, size_t __buflen)
    __attribute__((__nothrow__, __leaf__)) __attribute__((__nonnull__(2)));

extern int isatty(int __fd) __attribute__((__nothrow__, __leaf__));

extern int ttyslot(void) __attribute__((__nothrow__, __leaf__));

extern int link(const char *__from, const char *__to)
    __attribute__((__nothrow__, __leaf__)) __attribute__((__nonnull__(1, 2)));

extern int linkat(int __fromfd, const char *__from, int __tofd,
                  const char *__to, int __flags)
    __attribute__((__nothrow__, __leaf__)) __attribute__((__nonnull__(2, 4)));

extern int symlink(const char *__from, const char *__to)
    __attribute__((__nothrow__, __leaf__)) __attribute__((__nonnull__(1, 2)));

extern ssize_t readlink(const char *__restrict __path, char *__restrict __buf,
                        size_t __len) __attribute__((__nothrow__, __leaf__))
__attribute__((__nonnull__(1, 2)));

extern int symlinkat(const char *__from, int __tofd, const char *__to)
    __attribute__((__nothrow__, __leaf__)) __attribute__((__nonnull__(1, 3)));

extern ssize_t readlinkat(int __fd, const char *__restrict __path,
                          char *__restrict __buf, size_t __len)
    __attribute__((__nothrow__, __leaf__)) __attribute__((__nonnull__(2, 3)));

extern int unlink(const char *__name) __attribute__((__nothrow__, __leaf__))
__attribute__((__nonnull__(1)));

extern int unlinkat(int __fd, const char *__name, int __flag)
    __attribute__((__nothrow__, __leaf__)) __attribute__((__nonnull__(2)));

extern int rmdir(const char *__path) __attribute__((__nothrow__, __leaf__))
__attribute__((__nonnull__(1)));

extern __pid_t tcgetpgrp(int __fd) __attribute__((__nothrow__, __leaf__));

extern int tcsetpgrp(int __fd, __pid_t __pgrp_id)
    __attribute__((__nothrow__, __leaf__));

extern char *getlogin(void);

extern int getlogin_r(char *__name, size_t __name_len)
    __attribute__((__nonnull__(1)));

extern int setlogin(const char *__name) __attribute__((__nothrow__, __leaf__))
__attribute__((__nonnull__(1)));

// # 1 "/usr/include/bits/getopt_posix.h" 1 3 4
// # 27 "/usr/include/bits/getopt_posix.h" 3 4
// # 1 "/usr/include/bits/getopt_core.h" 1 3 4
// # 28 "/usr/include/bits/getopt_core.h" 3 4

extern char *optarg;
// # 50 "/usr/include/bits/getopt_core.h" 3 4
extern int optind;

extern int opterr;

extern int optopt;
// # 91 "/usr/include/bits/getopt_core.h" 3 4
extern int getopt(int ___argc, char *const *___argv, const char *__shortopts)
    __attribute__((__nothrow__, __leaf__)) __attribute__((__nonnull__(2, 3)));

// # 28 "/usr/include/bits/getopt_posix.h" 2 3 4

// # 49 "/usr/include/bits/getopt_posix.h" 3 4

// # 870 "/usr/include/unistd.h" 2 3 4

extern int gethostname(char *__name, size_t __len)
    __attribute__((__nothrow__, __leaf__)) __attribute__((__nonnull__(1)));

extern int sethostname(const char *__name, size_t __len)
    __attribute__((__nothrow__, __leaf__)) __attribute__((__nonnull__(1)));

extern int sethostid(long int __id) __attribute__((__nothrow__, __leaf__));

extern int getdomainname(char *__name, size_t __len)
    __attribute__((__nothrow__, __leaf__)) __attribute__((__nonnull__(1)));
extern int setdomainname(const char *__name, size_t __len)
    __attribute__((__nothrow__, __leaf__)) __attribute__((__nonnull__(1)));

extern int vhangup(void) __attribute__((__nothrow__, __leaf__));

extern int revoke(const char *__file) __attribute__((__nothrow__, __leaf__))
__attribute__((__nonnull__(1)));

extern int profil(unsigned short int *__sample_buffer, size_t __size,
                  size_t __offset, unsigned int __scale)
    __attribute__((__nothrow__, __leaf__)) __attribute__((__nonnull__(1)));

extern int acct(const char *__name) __attribute__((__nothrow__, __leaf__));

extern char *getusershell(void) __attribute__((__nothrow__, __leaf__));
extern void endusershell(void) __attribute__((__nothrow__, __leaf__));
extern void setusershell(void) __attribute__((__nothrow__, __leaf__));

extern int daemon(int __nochdir, int __noclose)
    __attribute__((__nothrow__, __leaf__));

extern int chroot(const char *__path) __attribute__((__nothrow__, __leaf__))
__attribute__((__nonnull__(1)));

extern char *getpass(const char *__prompt) __attribute__((__nonnull__(1)));

extern int fsync(int __fd);
// # 967 "/usr/include/unistd.h" 3 4
extern long int gethostid(void);

extern void sync(void) __attribute__((__nothrow__, __leaf__));

extern int getpagesize(void) __attribute__((__nothrow__, __leaf__))
__attribute__((__const__));

extern int getdtablesize(void) __attribute__((__nothrow__, __leaf__));
// # 991 "/usr/include/unistd.h" 3 4
extern int truncate(const char *__file, __off_t __length)
    __attribute__((__nothrow__, __leaf__)) __attribute__((__nonnull__(1)));
// # 1014 "/usr/include/unistd.h" 3 4
extern int ftruncate(int __fd, __off_t __length)
    __attribute__((__nothrow__, __leaf__));
// # 1035 "/usr/include/unistd.h" 3 4
extern int brk(void *__addr) __attribute__((__nothrow__, __leaf__));

extern void *sbrk(intptr_t __delta) __attribute__((__nothrow__, __leaf__));
// # 1056 "/usr/include/unistd.h" 3 4
extern long int syscall(long int __sysno, ...)
    __attribute__((__nothrow__, __leaf__));
// # 1079 "/usr/include/unistd.h" 3 4
extern int lockf(int __fd, int __cmd, __off_t __len);
// # 1115 "/usr/include/unistd.h" 3 4
extern int fdatasync(int __fildes);
// # 1124 "/usr/include/unistd.h" 3 4
extern char *crypt(const char *__key, const char *__salt)
    __attribute__((__nothrow__, __leaf__)) __attribute__((__nonnull__(1, 2)));
// # 1161 "/usr/include/unistd.h" 3 4
int getentropy(void *__buffer, size_t __length);

// # 8 "collections.c" 2
// # 1 "/usr/lib/gcc/x86_64-pc-linux-gnu/8.2.1/include/stdint.h" 1 3 4
// # 9 "/usr/lib/gcc/x86_64-pc-linux-gnu/8.2.1/include/stdint.h" 3 4
// # 1 "/usr/include/stdint.h" 1 3 4
// # 26 "/usr/include/stdint.h" 3 4
// # 1 "/usr/include/bits/libc-header-start.h" 1 3 4
// # 27 "/usr/include/stdint.h" 2 3 4

// # 1 "/usr/include/bits/wchar.h" 1 3 4
// # 29 "/usr/include/stdint.h" 2 3 4
// # 1 "/usr/include/bits/wordsize.h" 1 3 4
// # 30 "/usr/include/stdint.h" 2 3 4

// # 1 "/usr/include/bits/stdint-uintn.h" 1 3 4
// # 24 "/usr/include/bits/stdint-uintn.h" 3 4
typedef __uint8_t uint8_t;
typedef __uint16_t uint16_t;
typedef __uint32_t uint32_t;
typedef __uint64_t uint64_t;
// # 38 "/usr/include/stdint.h" 2 3 4

typedef __int_least8_t int_least8_t;
typedef __int_least16_t int_least16_t;
typedef __int_least32_t int_least32_t;
typedef __int_least64_t int_least64_t;

typedef __uint_least8_t uint_least8_t;
typedef __uint_least16_t uint_least16_t;
typedef __uint_least32_t uint_least32_t;
typedef __uint_least64_t uint_least64_t;

typedef signed char int_fast8_t;

typedef long int int_fast16_t;
typedef long int int_fast32_t;
typedef long int int_fast64_t;
// # 71 "/usr/include/stdint.h" 3 4
typedef unsigned char uint_fast8_t;

typedef unsigned long int uint_fast16_t;
typedef unsigned long int uint_fast32_t;
typedef unsigned long int uint_fast64_t;
// # 90 "/usr/include/stdint.h" 3 4
typedef unsigned long int uintptr_t;
// # 101 "/usr/include/stdint.h" 3 4
typedef __intmax_t intmax_t;
typedef __uintmax_t uintmax_t;
// # 10 "/usr/lib/gcc/x86_64-pc-linux-gnu/8.2.1/include/stdint.h" 2 3 4
// # 9 "collections.c" 2
// # 1 "/usr/include/time.h" 1 3 4
// # 29 "/usr/include/time.h" 3 4
// # 1 "/usr/lib/gcc/x86_64-pc-linux-gnu/8.2.1/include/stddef.h" 1 3 4
// # 30 "/usr/include/time.h" 2 3 4

// # 1 "/usr/include/bits/time.h" 1 3 4
// # 34 "/usr/include/time.h" 2 3 4

// # 1 "/usr/include/bits/types/struct_tm.h" 1 3 4

struct tm {
  int tm_sec;
  int tm_min;
  int tm_hour;
  int tm_mday;
  int tm_mon;
  int tm_year;
  int tm_wday;
  int tm_yday;
  int tm_isdst;

  long int tm_gmtoff;
  const char *tm_zone;
};
// # 40 "/usr/include/time.h" 2 3 4
// # 48 "/usr/include/time.h" 3 4
// # 1 "/usr/include/bits/types/struct_itimerspec.h" 1 3 4

struct itimerspec {
  struct timespec it_interval;
  struct timespec it_value;
};
// # 49 "/usr/include/time.h" 2 3 4
struct sigevent;
// # 68 "/usr/include/time.h" 3 4

extern clock_t clock(void) __attribute__((__nothrow__, __leaf__));

extern time_t time(time_t *__timer) __attribute__((__nothrow__, __leaf__));

extern double difftime(time_t __time1, time_t __time0)
    __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));

extern time_t mktime(struct tm *__tp) __attribute__((__nothrow__, __leaf__));

extern size_t strftime(char *__restrict __s, size_t __maxsize,
                       const char *__restrict __format,
                       const struct tm *__restrict __tp)
    __attribute__((__nothrow__, __leaf__));
// # 104 "/usr/include/time.h" 3 4
extern size_t strftime_l(char *__restrict __s, size_t __maxsize,
                         const char *__restrict __format,
                         const struct tm *__restrict __tp, locale_t __loc)
    __attribute__((__nothrow__, __leaf__));
// # 119 "/usr/include/time.h" 3 4
extern struct tm *gmtime(const time_t *__timer)
    __attribute__((__nothrow__, __leaf__));

extern struct tm *localtime(const time_t *__timer)
    __attribute__((__nothrow__, __leaf__));

extern struct tm *gmtime_r(const time_t *__restrict __timer,
                           struct tm *__restrict __tp)
    __attribute__((__nothrow__, __leaf__));

extern struct tm *localtime_r(const time_t *__restrict __timer,
                              struct tm *__restrict __tp)
    __attribute__((__nothrow__, __leaf__));

extern char *asctime(const struct tm *__tp)
    __attribute__((__nothrow__, __leaf__));

extern char *ctime(const time_t *__timer)
    __attribute__((__nothrow__, __leaf__));

extern char *asctime_r(const struct tm *__restrict __tp, char *__restrict __buf)
    __attribute__((__nothrow__, __leaf__));

extern char *ctime_r(const time_t *__restrict __timer, char *__restrict __buf)
    __attribute__((__nothrow__, __leaf__));

extern char *__tzname[2];
extern int __daylight;
extern long int __timezone;

extern char *tzname[2];

extern void tzset(void) __attribute__((__nothrow__, __leaf__));

extern int daylight;
extern long int timezone;

extern int stime(const time_t *__when) __attribute__((__nothrow__, __leaf__));
// # 196 "/usr/include/time.h" 3 4
extern time_t timegm(struct tm *__tp) __attribute__((__nothrow__, __leaf__));

extern time_t timelocal(struct tm *__tp) __attribute__((__nothrow__, __leaf__));

extern int dysize(int __year) __attribute__((__nothrow__, __leaf__))
__attribute__((__const__));
// # 211 "/usr/include/time.h" 3 4
extern int nanosleep(const struct timespec *__requested_time,
                     struct timespec *__remaining);

extern int clock_getres(clockid_t __clock_id, struct timespec *__res)
    __attribute__((__nothrow__, __leaf__));

extern int clock_gettime(clockid_t __clock_id, struct timespec *__tp)
    __attribute__((__nothrow__, __leaf__));

extern int clock_settime(clockid_t __clock_id, const struct timespec *__tp)
    __attribute__((__nothrow__, __leaf__));

extern int clock_nanosleep(clockid_t __clock_id, int __flags,
                           const struct timespec *__req,
                           struct timespec *__rem);

extern int clock_getcpuclockid(pid_t __pid, clockid_t *__clock_id)
    __attribute__((__nothrow__, __leaf__));

extern int timer_create(clockid_t __clock_id, struct sigevent *__restrict __evp,
                        timer_t *__restrict __timerid)
    __attribute__((__nothrow__, __leaf__));

extern int timer_delete(timer_t __timerid)
    __attribute__((__nothrow__, __leaf__));

extern int timer_settime(timer_t __timerid, int __flags,
                         const struct itimerspec *__restrict __value,
                         struct itimerspec *__restrict __ovalue)
    __attribute__((__nothrow__, __leaf__));

extern int timer_gettime(timer_t __timerid, struct itimerspec *__value)
    __attribute__((__nothrow__, __leaf__));

extern int timer_getoverrun(timer_t __timerid)
    __attribute__((__nothrow__, __leaf__));

extern int timespec_get(struct timespec *__ts, int __base)
    __attribute__((__nothrow__, __leaf__)) __attribute__((__nonnull__(1)));
// # 307 "/usr/include/time.h" 3 4

// # 10 "collections.c" 2

// # 1 "citron.h" 1

// # 1 "config.h" 1
// # 5 "citron.h" 2

// # 6 "citron.h"
char const *ctr_file_stdext_path_raw();
// # 23 "citron.h"
// # 1 "dictionary.h" 1
// # 24 "citron.h" 2
// # 1 "/usr/include/inttypes.h" 1 3 4
// # 34 "/usr/include/inttypes.h" 3 4

// # 34 "/usr/include/inttypes.h" 3 4
typedef int __gwchar_t;
// # 266 "/usr/include/inttypes.h" 3 4

typedef struct {
  long int quot;
  long int rem;
} imaxdiv_t;
// # 290 "/usr/include/inttypes.h" 3 4
extern intmax_t imaxabs(intmax_t __n) __attribute__((__nothrow__, __leaf__))
__attribute__((__const__));

extern imaxdiv_t imaxdiv(intmax_t __numer, intmax_t __denom)
    __attribute__((__nothrow__, __leaf__)) __attribute__((__const__));

extern intmax_t strtoimax(const char *__restrict __nptr,
                          char **__restrict __endptr, int __base)
    __attribute__((__nothrow__, __leaf__));

extern uintmax_t strtoumax(const char *__restrict __nptr,
                           char **__restrict __endptr, int __base)
    __attribute__((__nothrow__, __leaf__));

extern intmax_t wcstoimax(const __gwchar_t *__restrict __nptr,
                          __gwchar_t **__restrict __endptr, int __base)
    __attribute__((__nothrow__, __leaf__));

extern uintmax_t wcstoumax(const __gwchar_t *__restrict __nptr,
                           __gwchar_t **__restrict __endptr, int __base)
    __attribute__((__nothrow__, __leaf__));
// # 432 "/usr/include/inttypes.h" 3 4

// # 25 "citron.h" 2
// # 182 "citron.h"

// # 182 "citron.h"
typedef unsigned int ctr_bool;
typedef double ctr_number;
typedef char *ctr_raw_string;

typedef size_t ctr_size;

struct ctr_string {
  char *value;
  ctr_size vlen;
};
typedef struct ctr_string ctr_string;

struct ctr_map {
  struct ctr_mapitem *head;
  int size;
};
typedef struct ctr_map ctr_map;

struct ctr_mapitem {
  int hits;
  uint64_t hashKey;
  struct ctr_object *key;
  struct ctr_object *value;
  struct ctr_mapitem *prev;
  struct ctr_mapitem *next;
};
typedef struct ctr_mapitem ctr_mapitem;

struct ctr_argument {
  struct ctr_object *object;
  struct ctr_argument *next;
};
typedef struct ctr_argument ctr_argument;
typedef void(*voidptrfn_t(void *));

struct ctr_interfaces {
  int count;
  struct ctr_object *link;
  struct ctr_object **ifs;
};
typedef struct ctr_interfaces ctr_interfaces;

struct ctr_overload_set {
  int bucket_count;
  struct ctr_overload_set **sub_buckets;
  struct ctr_object *this_terminating_value;
  struct ctr_object *this_terminating_bucket;
};

typedef struct ctr_overload_set ctr_overload_set;

struct ctr_object {
  ctr_map *properties;
  ctr_map *methods;
  struct {
    unsigned int type : 4;
    unsigned int mark : 1;
    unsigned int sticky : 1;
    unsigned int asyncMode : 1;
    unsigned int chainMode : 1;
    unsigned int remote : 1;
    unsigned int shared : 1;
    unsigned int raw : 1;
    unsigned int overloaded : 1;
  } info;
  struct ctr_interfaces *interfaces;
  struct ctr_object *lexical_name;
  union uvalue {
    ctr_bool bvalue;
    ctr_number nvalue;
    ctr_string *svalue;
    struct ctr_tnode *block;
    struct ctr_collection *avalue;
    struct ctr_resource *rvalue;
    struct ctr_object *(*fvalue)(struct ctr_object *myself,
                                 struct ctr_argument *argumentList);
    struct ctr_object *defvalue;
  } value;
  union {
    voidptrfn_t *release_hook;
    struct ctr_overload_set *overloads;
  };

  struct ctr_object *gnext;
};
typedef struct ctr_object ctr_object;

struct ctr_resource {
  unsigned int type;
  void *ptr;
};
typedef struct ctr_resource ctr_resource;

struct ctr_collection {
  int immutable : 1;
  ctr_size length;
  ctr_size head;
  ctr_size tail;
  ctr_object **elements;
};
typedef struct ctr_collection ctr_collection;

struct ctr_tnode {
  char type;
  char modifier;
  char *value;
  ctr_size vlen;
  struct ctr_tlistitem *nodes;
  unsigned int lexical : 1;
};
typedef struct ctr_tnode ctr_tnode;

struct ctr_tlistitem {
  ctr_tnode *node;
  struct ctr_tlistitem *next;
};
typedef struct ctr_tlistitem ctr_tlistitem;

struct ctr_source_map {
  ctr_tnode *node;
  char *p_ptr;
  char *s_ptr;
  char *e_ptr;
  uint32_t line;
  struct ctr_source_map *next;
};
typedef struct ctr_source_map ctr_source_map;

ctr_source_map *ctr_source_map_head;
int ctr_source_mapping;

struct ctr_code_pragma {
  char type;
  int value;
  int value_e;
};
typedef struct ctr_code_pragma ctr_code_pragma;

struct fixity_lookup_rv {
  int fix;
  int prec;
  int lazy;
};
typedef struct fixity_lookup_rv fixity_lookup_rv;

int CTR_CCOMP_SIMULATION;
ctr_object *ctr_ccomp_get_stub(ctr_object *(*nfunc)(ctr_object *,
                                                    ctr_argument *),
                               ctr_object *receiver, ctr_argument *args);
ctr_argument *CtrCompilerStub;
extern int with_stdlib;
// # 374 "citron.h"
ctr_object *CtrStdWorld;
ctr_object *CtrStdObject;
ctr_object *CtrStdBlock;
ctr_object *CtrStdString;
ctr_object *CtrStdNumber;
ctr_object *CtrStdBool;
ctr_object *CtrStdConsole;
ctr_object *CtrStdNil;
ctr_object *CtrStdGC;
ctr_object *CtrStdMap;
ctr_object *CtrStdHashMap;
ctr_object *CtrStdArray;
ctr_object *CtrStdIter;
ctr_object *CtrStdFile;
ctr_object *CtrStdSystem;
ctr_object *CtrStdDice;
ctr_object *CtrStdCommand;
ctr_object *CtrStdSlurp;
ctr_object *CtrStdShell;
ctr_object *CtrStdClock;

extern __thread ctr_object *CtrStdFlow;

ctr_object *CtrExceptionType;
ctr_object *CtrStdBreak;
ctr_object *CtrStdContinue;
ctr_object *CtrStdExit;
ctr_object *CtrStdReflect;
ctr_object *CtrStdReflect_cons;
ctr_object *CtrStdFiber;
ctr_object *CtrStdThread;
ctr_object *CtrStdSymbol;
ctr_object *ctr_first_object;

ctr_object *CTR_FILE_STDIN;
ctr_object *CTR_FILE_STDOUT;
ctr_object *CTR_FILE_STDERR;
ctr_object *CTR_FILE_STDIN_STR;
ctr_object *CTR_FILE_STDOUT_STR;
ctr_object *CTR_FILE_STDERR_STR;

ctr_object **get_CtrStdWorld();
ctr_object **get_CtrStdObject();
ctr_object **get_CtrStdBlock();
ctr_object **get_CtrStdString();
ctr_object **get_CtrStdNumber();
ctr_object **get_CtrStdBool();
ctr_object **get_CtrStdConsole();
ctr_object **get_CtrStdNil();
ctr_object **get_CtrStdGC();
ctr_object **get_CtrStdMap();
ctr_object **get_CtrStdHashMap();
ctr_object **get_CtrStdArray();
ctr_object **get_CtrStdIter();
ctr_object **get_CtrStdFile();
ctr_object **get_CtrStdSystem();
ctr_object **get_CtrStdDice();
ctr_object **get_CtrStdCommand();
ctr_object **get_CtrStdSlurp();
ctr_object **get_CtrStdShell();
ctr_object **get_CtrStdClock();
ctr_object **get_CtrStdFlow();
ctr_object **get_CtrExceptionType();
ctr_object **get_CtrStdBreak();
ctr_object **get_CtrStdContinue();
ctr_object **get_CtrStdExit();
ctr_object **get_CtrStdReflect();
ctr_object **get_CtrStdReflect_cons();
ctr_object **get_CtrStdFiber();
ctr_object **get_CtrStdThread();
ctr_object **get_CtrStdSymbol();
ctr_object **get_ctr_first_object();
ctr_object **get_CTR_FILE_STDIN();
ctr_object **get_CTR_FILE_STDOUT();
ctr_object **get_CTR_FILE_STDERR();
ctr_object **get_CTR_FILE_STDIN_STR();
ctr_object **get_CTR_FILE_STDOUT_STR();
ctr_object **get_CTR_FILE_STDERR_STR();

ctr_object *ctr_exception_getinfo(ctr_object *myself,
                                  ctr_argument *argumentList);
ctr_object *ctr_internal_ex_data();

ctr_object *ctr_internal_find_overload(ctr_object *, ctr_argument *);

ctr_object *ctr_instrumentor_funcs;
ctr_object *ctr_global_instrum;
int ctr_instrument;

int ctr_internal_next_return;

char CtrHashKey[16];

int ctr_argc;
char **ctr_argv;

char *ctr_mode_input_file;
char *ctr_mode_interactive;

struct lexer_state {
  int string_interpolation;
  int ivarlen;
  ctr_size ctr_clex_tokvlen;
  char *ctr_clex_buffer;
  char *ctr_code;
  char *ctr_code_st;
  char *ctr_code_eoi;
  char *ctr_eofcode;
  char *ctr_clex_oldptr;
  char *ctr_clex_olderptr;
  int ctr_clex_verbatim_mode;
  uintptr_t ctr_clex_verbatim_mode_insert_quote;
  int ctr_clex_old_line_number;
  int ctr_transform_lambda_shorthand;
  char *ivarname;
  int inject_index;
};
void ctr_clex_load(char *prg);
int ctr_clex_tok();
char *ctr_clex_tok_value();
long ctr_clex_tok_value_length();
void ctr_clex_putback();
char *ctr_clex_readstr();
char *ctr_clex_scan(char c);
char *ctr_clex_scan_balanced(char c, char d);
char *ctr_clex_tok_describe(int token);
int ctr_clex_save_state();
int ctr_clex_dump_state(struct lexer_state *);
int ctr_clex_restore_state(int id);
int ctr_clex_load_state(struct lexer_state);
int ctr_clex_inject_token(int token, const char *value, const int vlen,
                          const int real_vlen);
void ctr_match_toggle_pragma();
void ctr_lex_parse_pragma();
char *ctr_clex_code_init;
char *ctr_clex_code_end;
char *ctr_clex_oldptr;
char *ctr_clex_olderptr;
int ctr_clex_quiet;
char *ctr_clex_keyword_var;
char *ctr_clex_keyword_me;
char *ctr_clex_keyword_my;
char *ctr_clex_keyword_const;
char *ctr_clex_keyword_static;
int ctr_clex_line_number;
int ctr_clex_old_line_number;

ctr_size ctr_clex_keyword_const_len;
ctr_size ctr_clex_keyword_my_len;
ctr_size ctr_clex_keyword_var_len;
ctr_size ctr_clex_keyword_static_len;

ctr_size ctr_clex_len;
ctr_size ctr_program_length;
int ctr_clex_line_number;
int ctr_clex_char_num;
int ctr_transform_lambda_shorthand;

unsigned long ctr_lex_position();
void ctr_lex_load(char *prg, size_t len);
char *ctr_lex_tok_value();
long ctr_lex_tok_value_length();
void ctr_lex_putback();
int ctr_lex_tok();
char *ctr_lex_readstr();
void ctr_lex_skip(int len);
ctr_object *ctr_lex_get_buf_str();

ctr_code_pragma *oneLineExpressions;
ctr_code_pragma *flexibleConstructs;
ctr_code_pragma *regexLineCheck;
ctr_code_pragma *callShorthand;
ctr_code_pragma *nextCallLazy;
ctr_code_pragma *extensionsPra;
// # 578 "citron.h"
extern int ctr_did_side_effect;
void ctr_mksrands(char *buf);

ctr_size getBytesUtf8(char *strval, long startByte, ctr_size lenUChar);
ctr_size ctr_getutf8len(char *strval, ctr_size max);
ctr_size ctr_getutf8clustercount(char *strval, ctr_size max);
int ctr_utf8_is_one_cluster(char *strval, ctr_size max);
int ctr_utf8size(char c);

ctr_tnode *ctr_cparse_parse(char *prg, char *pathString);
ctr_tnode *ctr_cparse_expr(int mode);
ctr_tnode *ctr_cparse_ret();
int ctr_cparse_quiet;
ctr_object *ctr_ast_from_node(ctr_tnode *node);
int ctr_ast_is_splice(ctr_object *obj);
ctr_object *ctr_ast_splice(ctr_object *obj);

char *ctr_last_parser_error;
int ctr_cparse_calltime_name_id;
ctr_tnode *ctr_cparse_calltime_names[50000];

uint64_t ctr_cwlk_subprogram;
ctr_object *ctr_cwlk_run(ctr_tnode *program);
ctr_object *ctr_cwlk_expr(ctr_tnode *node, char *wasReturn);
ctr_tnode *ctr_unmarshal_ast(char *stream, size_t avail, size_t *consumed);
void ctr_marshal_ast(ctr_tnode *rnode, char **stream, size_t *stream_len,
                     size_t *used);

ctr_object *ctr_get_last_trace(ctr_object *myself, ctr_argument *argumentList);
ctr_object *ctr_get_last_trace_stringified(ctr_object *myself,
                                           ctr_argument *argumentList);
void ctr_initialize_world();
void ctr_initialize_world_minimal();
void ctr_initialize_ex();
char *ctr_internal_memmem(char *haystack, long hlen, char *needle, long nlen,
                          int reverse);
void ctr_internal_object_add_property(ctr_object *owner, ctr_object *key,
                                      ctr_object *value, int m);
void ctr_internal_object_add_property_with_hash(ctr_object *owner,
                                                ctr_object *key, uint64_t hash,
                                                ctr_object *value, int m);
void ctr_internal_object_set_property(ctr_object *owner, ctr_object *key,
                                      ctr_object *value, int is_method);
void ctr_internal_object_delete_property(ctr_object *owner, ctr_object *key,
                                         int is_method);
void ctr_internal_object_delete_property_with_hash(ctr_object *owner,
                                                   ctr_object *key,
                                                   uint64_t hash,
                                                   int is_method);
ctr_object *ctr_internal_object_find_property(ctr_object *owner,
                                              ctr_object *key, int is_method);
ctr_object *ctr_internal_object_find_property_with_hash(ctr_object *owner,
                                                        ctr_object *key,
                                                        uint64_t hash,
                                                        int is_method);
ctr_object *ctr_internal_object_find_property_or_create_with_hash(
    ctr_object *owner, ctr_object *key, uint64_t hashKey, int is_method);
ctr_object *ctr_internal_object_find_property_ignore(ctr_object *owner,
                                                     ctr_object *key,
                                                     int is_method, int ignore);
uint64_t ctr_internal_index_hash(ctr_object *key);
uint64_t ctr_internal_alt_hash(ctr_object *key);
void ctr_internal_object_add_property(ctr_object *owner, ctr_object *key,
                                      ctr_object *value, int m);
ctr_object *ctr_internal_cast2bool(ctr_object *o);
ctr_object *ctr_internal_cast2number(ctr_object *o);
ctr_object *ctr_internal_cast2string(ctr_object *o);
ctr_object *ctr_internal_create_object(int type);
ctr_object *ctr_internal_create_mapped_object(int type, int shared);
ctr_object *ctr_internal_create_standalone_object(int type);
void ctr_transfer_object_ownership(ctr_object *to, ctr_object *what);
inline ctr_object *ctr_internal_create_mapped_standalone_object(int type,
                                                                int shared);
void ctr_internal_delete_standalone_object(ctr_object *o);
void *ctr_internal_plugin_find(ctr_object *key);
void *ctr_internal_plugin_find_base(char const *key);
ctr_object *ctr_format_str(const char *str_format, ...);
ctr_object *ctr_find_(ctr_object *key, int noerror);
ctr_object *ctr_find(ctr_object *key);
ctr_object *ctr_find_in_my(ctr_object *key);
ctr_object *ctr_assign_value(ctr_object *key, ctr_object *val);
ctr_object *ctr_assign_value_to_my(ctr_object *key, ctr_object *val);
ctr_object *ctr_assign_value_to_local(ctr_object *key, ctr_object *val);
ctr_object *ctr_assign_value_to_local_by_ref(ctr_object *key, ctr_object *val);
ctr_object *ctr_hand_value_to_global(ctr_object *key, ctr_object *val);
ctr_object *ctr_const_assign_value(ctr_object *key, ctr_object *o,
                                   ctr_object *context);
char *ctr_internal_readf(char *file_name, uint64_t *size_allocated);
char *ctr_internal_readfp(FILE *fp, uint64_t *size_allocated);
void ctr_internal_debug_tree(ctr_tnode *ti, int indent);
void ctr_capture_refs(ctr_tnode *ti, ctr_object *block);
ctr_object *ctr_get_responder(ctr_object *receiverObject, char *message,
                              long vlen);
ctr_object *ctr_send_message(ctr_object *receiver, char *message, long len,
                             ctr_argument *argumentList);
ctr_object *ctr_send_message_blocking(ctr_object *receiver, char *message,
                                      long len, ctr_argument *argumentList);
ctr_object *ctr_send_message_async(ctr_object *receiver, char *message,
                                   long len, ctr_argument *argumentList);
ctr_object *ctr_send_message_variadic(ctr_object *myself, char *message,
                                      int msglen, int count, ...);
ctr_object *ctr_invoke_variadic(ctr_object *myself,
                                ctr_object *(*fun)(ctr_object *,
                                                   ctr_argument *),
                                int count, ...);
ctr_argument *ctr_allocate_argumentList(int count, ...);
void ctr_free_argumentList(ctr_argument *argumentList);
void ctr_internal_create_func(ctr_object *o, ctr_object *key,
                              ctr_object *(*func)(ctr_object *,
                                                  ctr_argument *));
int ctr_is_primitive(ctr_object *object);
ctr_object *ctr_get_stack_trace();
void ctr_print_stack_trace();
ctr_object *ctr_get_or_create_symbol_table_entry(const char *name,
                                                 ctr_size len);
ctr_object *ctr_get_or_create_symbol_table_entry_s(const char *name,
                                                   ctr_size len);
fixity_lookup_rv ctr_lookup_fix(const char *name, int length);
void ctr_set_fix(const char *name, int length, int fix, int prec, int lazy);
void clear_fixity_map();
int ctr_str_count_substr(char *str, char *substr, int overlap);

char ignore_in_trace[2048][1024];
int trace_ignore_count;
extern char *SystemTZ;

void ctr_open_context();
void ctr_close_context();
void ctr_switch_context();

struct ctr_context_t {
  ctr_object *contexts[50000];
  int id;
};
void ctr_dump_context(struct ctr_context_t *);
void ctr_load_context(struct ctr_context_t);

extern ctr_object *ctr_contexts[50000];

int ctr_context_id;
ctr_tnode *ctr_callstack[50000];
uint8_t ctr_callstack_index;

ctr_object *ctr_nil_is_nil(ctr_object *myself, ctr_argument *argumentList);
ctr_object *ctr_nil_to_string(ctr_object *myself, ctr_argument *argumentList);
ctr_object *ctr_nil_to_number(ctr_object *myself, ctr_argument *argumentList);
ctr_object *ctr_nil_to_boolean(ctr_object *myself, ctr_argument *argumentList);
ctr_object *ctr_nil_assign(ctr_object *myself, ctr_argument *argumentList);

ctr_object *ctr_object_make(ctr_object *myself, ctr_argument *argumentList);
ctr_object *ctr_object_ctor(ctr_object *myself, ctr_argument *argumentList);
ctr_object *ctr_object_assign(ctr_object *myself, ctr_argument *argumentList);
ctr_object *ctr_object_hash(ctr_object *myself, ctr_argument *argumentList);

ctr_object *ctr_object_attr_accessor(ctr_object *myself,
                                     ctr_argument *argumentList);
ctr_object *ctr_object_attr_reader(ctr_object *myself,
                                   ctr_argument *argumentList);
ctr_object *ctr_object_attr_writer(ctr_object *myself,
                                   ctr_argument *argumentList);
ctr_object *ctr_object_make_hiding(ctr_object *myself,
                                   ctr_argument *argumentList);
ctr_object *ctr_object_swap(ctr_object *myself, ctr_argument *argumentList);
ctr_object *ctr_object_equals(ctr_object *myself, ctr_argument *argumentList);
ctr_object *ctr_object_id(ctr_object *myself, ctr_argument *argumentList);
ctr_object *ctr_object_on_do(ctr_object *myself, ctr_argument *argumentList);
ctr_object *ctr_object_respond(ctr_object *myself, ctr_argument *argumentList);
ctr_object *ctr_object_type(ctr_object *myself, ctr_argument *argumentList);
ctr_object *ctr_object_is_nil(ctr_object *myself, ctr_argument *argumentList);
ctr_object *ctr_object_myself(ctr_object *myself, ctr_argument *argumentList);
ctr_object *ctr_object_do(ctr_object *myself, ctr_argument *argumentList);
ctr_object *ctr_object_done(ctr_object *myself, ctr_argument *argumentList);
ctr_object *ctr_object_message(ctr_object *myself, ctr_argument *argumentList);
ctr_object *ctr_object_elvis_op(ctr_object *myself, ctr_argument *argumentList);
ctr_object *ctr_object_if_false(ctr_object *myself, ctr_argument *argumentList);
ctr_object *ctr_object_if_true(ctr_object *myself, ctr_argument *argumentList);
ctr_object *ctr_object_if_tf(ctr_object *myself, ctr_argument *argumentList);
ctr_object *ctr_object_learn_meaning(ctr_object *myself,
                                     ctr_argument *ctr_argumentList);
ctr_object *ctr_object_to_string(ctr_object *myself,
                                 ctr_argument *ctr_argumentList);
ctr_object *ctr_object_to_number(ctr_object *myself,
                                 ctr_argument *ctr_argumentList);
ctr_object *ctr_object_to_boolean(ctr_object *myself,
                                  ctr_argument *ctr_argumentList);
ctr_object *ctr_object_remote(ctr_object *myself,
                              ctr_argument *ctr_argumentList);
ctr_object *ctr_object_respond_and(ctr_object *myself,
                                   ctr_argument *ctr_argumentList);
ctr_object *ctr_object_respond_and_and(ctr_object *myself,
                                       ctr_argument *ctr_argumentList);

ctr_object *ctr_bool_if_true(ctr_object *myself, ctr_argument *argumentList);
ctr_object *ctr_bool_if_false(ctr_object *myself, ctr_argument *argumentList);
ctr_object *ctr_bool_if_tf(ctr_object *myself, ctr_argument *argumentList);
ctr_object *ctr_bool_and(ctr_object *myself, ctr_argument *argumentList);
ctr_object *ctr_bool_nor(ctr_object *myself, ctr_argument *argumentList);
ctr_object *ctr_bool_or(ctr_object *myself, ctr_argument *argumentList);
ctr_object *ctr_bool_eq(ctr_object *myself, ctr_argument *argumentList);
ctr_object *ctr_bool_neq(ctr_object *myself, ctr_argument *argumentList);
ctr_object *ctr_bool_xor(ctr_object *myself, ctr_argument *argumentList);
ctr_object *ctr_bool_to_number(ctr_object *myself, ctr_argument *argumentList);
ctr_object *ctr_bool_to_string(ctr_object *myself, ctr_argument *argumentList);
ctr_object *ctr_bool_not(ctr_object *myself, ctr_argument *argumentList);
ctr_object *ctr_bool_flip(ctr_object *myself, ctr_argument *argumentList);
ctr_object *ctr_bool_either_or(ctr_object *myself, ctr_argument *argumentList);
ctr_object *ctr_bool_break(ctr_object *myself, ctr_argument *argumentList);
ctr_object *ctr_bool_continue(ctr_object *myself, ctr_argument *argumentList);
ctr_object *ctr_bool_assign(ctr_object *myself, ctr_argument *argumentList);

ctr_object *ctr_number_add(ctr_object *myself, ctr_argument *argumentList);
ctr_object *ctr_number_inc(ctr_object *myself, ctr_argument *argumentList);
ctr_object *ctr_number_minus(ctr_object *myself, ctr_argument *argumentList);
ctr_object *ctr_number_dec(ctr_object *myself, ctr_argument *argumentList);
ctr_object *ctr_number_multiply(ctr_object *myself, ctr_argument *argumentList);
ctr_object *ctr_number_times(ctr_object *myself, ctr_argument *argumentList);
ctr_object *ctr_number_mul(ctr_object *myself, ctr_argument *argumentList);
ctr_object *ctr_number_divide(ctr_object *myself, ctr_argument *argumentList);
ctr_object *ctr_number_div(ctr_object *myself, ctr_argument *argumentList);
ctr_object *ctr_number_higherThan(ctr_object *myself,
                                  ctr_argument *argumentList);
ctr_object *ctr_number_higherEqThan(ctr_object *myself,
                                    ctr_argument *argumentList);
ctr_object *ctr_number_lowerThan(ctr_object *myself,
                                 ctr_argument *argumentList);
ctr_object *ctr_number_lowerEqThan(ctr_object *myself,
                                   ctr_argument *argumentList);
ctr_object *ctr_number_eq(ctr_object *myself, ctr_argument *argumentList);
ctr_object *ctr_number_neq(ctr_object *myself, ctr_argument *argumentList);
ctr_object *ctr_number_modulo(ctr_object *myself, ctr_argument *argumentList);
ctr_object *ctr_number_factorial(ctr_object *myself,
                                 ctr_argument *argumentList);
ctr_object *ctr_number_floor(ctr_object *myself, ctr_argument *argumentList);
ctr_object *ctr_number_ceil(ctr_object *myself, ctr_argument *argumentList);
ctr_object *ctr_number_round(ctr_object *myself, ctr_argument *argumentList);
ctr_object *ctr_number_abs(ctr_object *myself, ctr_argument *argumentList);
ctr_object *ctr_number_sin(ctr_object *myself, ctr_argument *argumentList);
ctr_object *ctr_number_cos(ctr_object *myself, ctr_argument *argumentList);
ctr_object *ctr_number_exp(ctr_object *myself, ctr_argument *argumentList);
ctr_object *ctr_number_sqrt(ctr_object *myself, ctr_argument *argumentList);
ctr_object *ctr_number_tan(ctr_object *myself, ctr_argument *argumentList);
ctr_object *ctr_number_atan(ctr_object *myself, ctr_argument *argumentList);
ctr_object *ctr_number_log(ctr_object *myself, ctr_argument *argumentList);
ctr_object *ctr_number_shr(ctr_object *myself, ctr_argument *argumentList);
ctr_object *ctr_number_shl(ctr_object *myself, ctr_argument *argumentList);
ctr_object *ctr_number_pow(ctr_object *myself, ctr_argument *argumentList);
ctr_object *ctr_number_min(ctr_object *myself, ctr_argument *argumentList);
ctr_object *ctr_number_max(ctr_object *myself, ctr_argument *argumentList);
ctr_object *ctr_number_odd(ctr_object *myself, ctr_argument *argumentList);
ctr_object *ctr_number_even(ctr_object *myself, ctr_argument *argumentList);
ctr_object *ctr_number_xor(ctr_object *myself, ctr_argument *argumentList);
ctr_object *ctr_number_or(ctr_object *myself, ctr_argument *argumentList);
ctr_object *ctr_number_and(ctr_object *myself, ctr_argument *argumentList);
ctr_object *ctr_number_uint_binrep(ctr_object *myself,
                                   ctr_argument *argumentList);
ctr_object *ctr_number_to_string(ctr_object *myself,
                                 ctr_argument *argumentList);
ctr_object *ctr_number_to_string_base(ctr_object *myself,
                                      ctr_argument *argumentList);
ctr_object *ctr_number_to_boolean(ctr_object *myself,
                                  ctr_argument *argumentList);
ctr_object *ctr_number_between(ctr_object *myself, ctr_argument *argumentList);
ctr_object *ctr_number_to_step_do(ctr_object *myself,
                                  ctr_argument *argumentList);
ctr_object *ctr_number_positive(ctr_object *myself, ctr_argument *argumentList);
ctr_object *ctr_number_negative(ctr_object *myself, ctr_argument *argumentList);
ctr_object *ctr_number_to_byte(ctr_object *myself, ctr_argument *argumentList);
ctr_object *ctr_number_qualify(ctr_object *myself, ctr_argument *argumentList);
ctr_object *ctr_number_respond_to(ctr_object *myself,
                                  ctr_argument *argumentList);
ctr_object *ctr_number_assign(ctr_object *myself, ctr_argument *argumentList);
ctr_object *ctr_number_negate(ctr_object *myself, ctr_argument *argumentList);

ctr_object *ctr_string_escape_ascii(ctr_object *myself,
                                    ctr_argument *argumentList);
ctr_object *ctr_string_bytes(ctr_object *myself, ctr_argument *argumentList);
ctr_object *ctr_string_length(ctr_object *myself, ctr_argument *argumentList);
ctr_object *ctr_string_empty(ctr_object *myself, ctr_argument *argumentList);
ctr_object *ctr_string_fromto(ctr_object *myself, ctr_argument *argumentList);
ctr_object *ctr_string_from_length(ctr_object *myself,
                                   ctr_argument *argumentList);
ctr_object *ctr_string_concat(ctr_object *myself, ctr_argument *argumentList);
ctr_object *ctr_string_append(ctr_object *myself, ctr_argument *argumentList);
ctr_object *ctr_string_multiply(ctr_object *myself, ctr_argument *argumentList);
ctr_object *ctr_string_eq(ctr_object *myself, ctr_argument *argumentList);
ctr_object *ctr_string_neq(ctr_object *myself, ctr_argument *argumentList);
ctr_object *ctr_string_trim(ctr_object *myself, ctr_argument *argumentList);
ctr_object *ctr_string_ltrim(ctr_object *myself, ctr_argument *argumentList);
ctr_object *ctr_string_rtrim(ctr_object *myself, ctr_argument *argumentList);
ctr_object *ctr_string_padding_left(ctr_object *myself,
                                    ctr_argument *argumentList);
ctr_object *ctr_string_padding_right(ctr_object *myself,
                                     ctr_argument *argumentList);
ctr_object *ctr_string_html_escape(ctr_object *myself,
                                   ctr_argument *argumentList);

ctr_object *ctr_string_count_of(ctr_object *myself, ctr_argument *argumentList);
ctr_object *ctr_string_slice(ctr_object *myself, ctr_argument *argumentList);
ctr_object *ctr_string_at(ctr_object *myself, ctr_argument *argumentList);
ctr_object *ctr_string_put_at(ctr_object *myself, ctr_argument *argumentList);
ctr_object *ctr_string_byte_at(ctr_object *myself, ctr_argument *argumentList);
ctr_object *ctr_string_fmap(ctr_object *myself, ctr_argument *argumentList);
ctr_object *ctr_string_imap(ctr_object *myself, ctr_argument *argumentList);
ctr_object *ctr_string_filter(ctr_object *myself, ctr_argument *argumentList);
ctr_object *ctr_string_index_of(ctr_object *myself, ctr_argument *argumentList);
ctr_object *ctr_string_re_index_of(ctr_object *myself,
                                   ctr_argument *argumentList);
ctr_object *ctr_string_starts_with(ctr_object *myself,
                                   ctr_argument *argumentList);
ctr_object *ctr_string_ends_with(ctr_object *myself,
                                 ctr_argument *argumentList);
ctr_object *ctr_string_last_index_of(ctr_object *myself,
                                     ctr_argument *argumentList);
ctr_object *ctr_string_re_last_index_of(ctr_object *myself,
                                        ctr_argument *argumentList);
ctr_object *ctr_string_replace_with(ctr_object *myself,
                                    ctr_argument *argumentList);
ctr_object *ctr_string_split(ctr_object *myself, ctr_argument *argumentList);
ctr_object *ctr_string_split_re(ctr_object *myself, ctr_argument *argumentList);
ctr_object *ctr_string_to_number(ctr_object *myself,
                                 ctr_argument *argumentList);
ctr_object *ctr_string_to_boolean(ctr_object *myself,
                                  ctr_argument *argumentList);
ctr_object *ctr_string_to_lower(ctr_object *myself, ctr_argument *argumentList);
ctr_object *ctr_string_to_upper(ctr_object *myself, ctr_argument *argumentList);
ctr_object *ctr_string_skip(ctr_object *myself, ctr_argument *argumentList);
ctr_object *ctr_string_to_lower1st(ctr_object *myself,
                                   ctr_argument *argumentList);
ctr_object *ctr_string_to_upper1st(ctr_object *myself,
                                   ctr_argument *argumentList);
ctr_object *ctr_string_find_pattern_do(ctr_object *myself,
                                       ctr_argument *argumentList);
ctr_object *ctr_string_find_pattern_options_do(ctr_object *myself,
                                               ctr_argument *argumentList);
ctr_object *ctr_string_reg_replace(ctr_object *myself,
                                   ctr_argument *argumentList);
ctr_object *ctr_string_reg_compile(ctr_object *myself,
                                   ctr_argument *argumentList);
ctr_object *ctr_string_is_regex_pcre(ctr_object *myself,
                                     ctr_argument *argumentList);
ctr_object *ctr_string_contains_pattern(ctr_object *myself,
                                        ctr_argument *argumentList);
ctr_object *ctr_string_contains(ctr_object *myself, ctr_argument *argumentList);
ctr_object *ctr_string_hash_with_key(ctr_object *myself,
                                     ctr_argument *argumentList);
ctr_object *ctr_string_to_string(ctr_object *myself,
                                 ctr_argument *argumentList);
ctr_object *ctr_string_format(ctr_object *myself, ctr_argument *argumentList);
ctr_object *ctr_string_format_map(ctr_object *myself,
                                  ctr_argument *argumentList);
ctr_object *ctr_string_eval(ctr_object *myself, ctr_argument *argumentList);
ctr_object *ctr_string_exec(ctr_object *myself, ctr_argument *argumentList);
ctr_object *ctr_string_quotes_escape(ctr_object *myself,
                                     ctr_argument *argumentList);
ctr_object *ctr_string_dquotes_escape(ctr_object *myself,
                                      ctr_argument *argumentList);
ctr_object *ctr_string_characters(ctr_object *myself,
                                  ctr_argument *argumentList);
ctr_object *ctr_string_to_byte_array(ctr_object *myself,
                                     ctr_argument *argumentList);
ctr_object *ctr_string_append_byte(ctr_object *myself,
                                   ctr_argument *argumentList);
ctr_object *ctr_string_csub(ctr_object *myself, ctr_argument *argumentList);
ctr_object *ctr_string_cadd(ctr_object *myself, ctr_argument *argumentList);
ctr_object *ctr_string_randomize_bytes(ctr_object *myself,
                                       ctr_argument *argumentList);
ctr_object *ctr_string_reverse(ctr_object *myself, ctr_argument *argumentList);
ctr_object *ctr_string_is_ctor(ctr_object *myself, ctr_argument *argumentList);
ctr_object *ctr_string_assign(ctr_object *myself, ctr_argument *argumentList);

ctr_object *ctr_block_runIt(ctr_object *myself, ctr_argument *argumentList);
ctr_object *ctr_block_specialise(ctr_object *myself,
                                 ctr_argument *argumentList);
ctr_object *ctr_block_runall(ctr_object *myself, ctr_argument *argumentList);
ctr_object *ctr_block_run_variadic(ctr_object *myself, int count, ...);
ctr_object *ctr_block_run_variadic_my(ctr_object *myself, ctr_object *my,
                                      int count, ...);
ctr_object *ctr_block_set(ctr_object *myself, ctr_argument *argumentList);
ctr_object *ctr_block_error(ctr_object *myself, ctr_argument *argumentList);
ctr_object *ctr_block_catch(ctr_object *myself, ctr_argument *argumentList);
ctr_object *ctr_block_catch_type(ctr_object *myself,
                                 ctr_argument *argumentList);
ctr_object *ctr_block_while_true(ctr_object *myself,
                                 ctr_argument *argumentList);
ctr_object *ctr_block_while_false(ctr_object *myself,
                                  ctr_argument *argumentList);
ctr_object *ctr_block_forever(ctr_object *myself, ctr_argument *argumentList);
ctr_object *ctr_block_run(ctr_object *myself, ctr_argument *argList,
                          ctr_object *my);
ctr_object *ctr_block_run_here(ctr_object *myself, ctr_argument *argList,
                               ctr_object *my);
ctr_object *ctr_block_times(ctr_object *myself, ctr_argument *argumentList);
ctr_object *ctr_block_to_string(ctr_object *myself, ctr_argument *argumentList);
ctr_object *ctr_block_assign(ctr_object *myself, ctr_argument *argumentList);

ctr_object *ctr_array_new(ctr_object *myself, ctr_argument *argumentList);
ctr_object *ctr_array_copy(ctr_object *myself, ctr_argument *argumentList);
ctr_object *ctr_array_new_and_push(ctr_object *myself,
                                   ctr_argument *argumentList);
ctr_object *ctr_array_type(ctr_object *myself, ctr_argument *argumentList);
ctr_object *ctr_array_fmap(ctr_object *myself, ctr_argument *argumentList);
ctr_object *ctr_array_imap(ctr_object *myself, ctr_argument *argumentList);
ctr_object *ctr_array_fmap_inp(ctr_object *myself, ctr_argument *argumentList);
ctr_object *ctr_array_imap_inp(ctr_object *myself, ctr_argument *argumentList);
ctr_object *ctr_array_foldl(ctr_object *myself, ctr_argument *argumentList);
ctr_object *ctr_array_foldl0(ctr_object *myself, ctr_argument *argumentList);
ctr_object *ctr_array_filter(ctr_object *myself, ctr_argument *argumentList);
ctr_object *ctr_array_filter_v(ctr_object *myself, ctr_argument *argumentList);
ctr_object *ctr_array_select_from_if(ctr_object *myself,
                                     ctr_argument *argumentList);
ctr_object *ctr_array_push(ctr_object *myself, ctr_argument *argumentList);
ctr_object *ctr_array_push_imm(ctr_object *myself, ctr_argument *argumentList);
ctr_object *ctr_array_reverse(ctr_object *myself, ctr_argument *argumentList);
ctr_object *ctr_array_unshift(ctr_object *myself, ctr_argument *argumentList);
ctr_object *ctr_array_shift(ctr_object *myself, ctr_argument *argumentList);
ctr_object *ctr_array_count(ctr_object *myself, ctr_argument *argumentList);
ctr_object *ctr_array_empty(ctr_object *myself, ctr_argument *argumentList);
ctr_object *ctr_array_join(ctr_object *myself, ctr_argument *argumentList);
ctr_object *ctr_array_pop(ctr_object *myself, ctr_argument *argumentList);
ctr_object *ctr_array_index(ctr_object *myself, ctr_argument *argumentList);
ctr_object *ctr_array_contains(ctr_object *myself, ctr_argument *argumentList);
ctr_object *ctr_array_get(ctr_object *myself, ctr_argument *argumentList);
ctr_object *ctr_array_sort(ctr_object *myself, ctr_argument *argumentList);
ctr_object *ctr_array_put(ctr_object *myself, ctr_argument *argumentList);
ctr_object *ctr_array_from_length(ctr_object *myself,
                                  ctr_argument *argumentList);
ctr_object *ctr_array_skip(ctr_object *myself, ctr_argument *argumentList);
ctr_object *ctr_array_zip(ctr_object *myself, ctr_argument *argumentList);
ctr_object *ctr_array_internal_product(ctr_object *myself,
                                       ctr_argument *argumentList);
ctr_object *ctr_array_zip_with(ctr_object *myself, ctr_argument *argumentList);
ctr_object *ctr_array_head(ctr_object *myself, ctr_argument *argumentList);
ctr_object *ctr_array_tail(ctr_object *myself, ctr_argument *argumentList);
ctr_object *ctr_array_init(ctr_object *myself, ctr_argument *argumentList);
ctr_object *ctr_array_last(ctr_object *myself, ctr_argument *argumentList);
ctr_object *ctr_array_add(ctr_object *myself, ctr_argument *argumentList);
ctr_object *ctr_array_map(ctr_object *myself, ctr_argument *argumentList);
ctr_object *ctr_array_map_v(ctr_object *myself, ctr_argument *argumentList);
ctr_object *ctr_array_min(ctr_object *myself, ctr_argument *argumentList);
ctr_object *ctr_array_max(ctr_object *myself, ctr_argument *argumentList);
ctr_object *ctr_array_sum(ctr_object *myself, ctr_argument *argumentList);
ctr_object *ctr_array_product(ctr_object *myself, ctr_argument *argumentList);
ctr_object *ctr_array_multiply(ctr_object *myself, ctr_argument *argumentList);
ctr_object *ctr_array_to_string(ctr_object *myself, ctr_argument *argumentList);
ctr_object *ctr_array_fill(ctr_object *myself, ctr_argument *argumentList);
ctr_object *ctr_array_column(ctr_object *myself, ctr_argument *argumentList);
ctr_object *ctr_array_assign(ctr_object *myself, ctr_argument *argumentList);
ctr_object *ctr_array_intersperse(ctr_object *myself,
                                  ctr_argument *argumentList);
ctr_object *ctr_array_slice(ctr_object *myself, ctr_argument *argumentList);
ctr_object *ctr_array_chunks(ctr_object *myself, ctr_argument *argumentList);
ctr_object *ctr_array_every_do_fill(ctr_object *myself,
                                    ctr_argument *argumentList);
ctr_object *ctr_array_all(ctr_object *myself, ctr_argument *argumentList);
ctr_object *ctr_array_any(ctr_object *myself, ctr_argument *argumentList);
ctr_object *ctr_build_immutable(ctr_tnode *node);

ctr_object *ctr_hmap_new(ctr_object *myself, ctr_argument *argumentList);
ctr_object *ctr_hmap_type(ctr_object *myself, ctr_argument *argumentList);
ctr_object *ctr_hmap_merge(ctr_object *myself, ctr_argument *argumentList);
ctr_object *ctr_hmap_keys(ctr_object *myself, ctr_argument *argumentList);
ctr_object *ctr_hmap_values(ctr_object *myself, ctr_argument *argumentList);
ctr_object *ctr_hmap_put(ctr_object *myself, ctr_argument *argumentList);
ctr_object *ctr_hmap_rm(ctr_object *myself, ctr_argument *argumentList);
ctr_object *ctr_hmap_get(ctr_object *myself, ctr_argument *argumentList);
ctr_object *ctr_hmap_get_or_insert(ctr_object *myself,
                                   ctr_argument *argumentList);
ctr_object *ctr_hmap_count(ctr_object *myself, ctr_argument *argumentList);
ctr_object *ctr_hmap_empty(ctr_object *myself, ctr_argument *argumentList);
ctr_object *ctr_hmap_each(ctr_object *myself, ctr_argument *argumentList);
ctr_object *ctr_hmap_fmap(ctr_object *myself, ctr_argument *argumentList);
ctr_object *ctr_hmap_fmap_inp(ctr_object *myself, ctr_argument *argumentList);
ctr_object *ctr_hmap_kvmap(ctr_object *myself, ctr_argument *argumentList);
ctr_object *ctr_hmap_kvlist(ctr_object *myself, ctr_argument *argumentList);
ctr_object *ctr_hmap_contains(ctr_object *myself, ctr_argument *argumentList);
ctr_object *ctr_hmap_flip(ctr_object *myself, ctr_argument *argumentList);
ctr_object *ctr_hmap_assign(ctr_object *myself, ctr_argument *argumentList);
ctr_object *ctr_hmap_to_string(ctr_object *myself, ctr_argument *argumentList);

ctr_object *ctr_map_new(ctr_object *myself, ctr_argument *argumentList);
ctr_object *ctr_map_new_(ctr_object *myself, ctr_argument *argumentList);
ctr_object *ctr_map_type(ctr_object *myself, ctr_argument *argumentList);
ctr_object *ctr_map_merge(ctr_object *myself, ctr_argument *argumentList);
ctr_object *ctr_map_keys(ctr_object *myself, ctr_argument *argumentList);
ctr_object *ctr_map_values(ctr_object *myself, ctr_argument *argumentList);
ctr_object *ctr_map_put(ctr_object *myself, ctr_argument *argumentList);
ctr_object *ctr_map_rm(ctr_object *myself, ctr_argument *argumentList);
ctr_object *ctr_map_get(ctr_object *myself, ctr_argument *argumentList);
ctr_object *ctr_map_get_or_insert(ctr_object *myself,
                                  ctr_argument *argumentList);
ctr_object *ctr_map_count(ctr_object *myself, ctr_argument *argumentList);
ctr_object *ctr_map_empty(ctr_object *myself, ctr_argument *argumentList);
ctr_object *ctr_map_each(ctr_object *myself, ctr_argument *argumentList);
ctr_object *ctr_map_fmap(ctr_object *myself, ctr_argument *argumentList);
ctr_object *ctr_map_fmap_inp(ctr_object *myself, ctr_argument *argumentList);
ctr_object *ctr_map_kvmap(ctr_object *myself, ctr_argument *argumentList);
ctr_object *ctr_map_kvlist(ctr_object *myself, ctr_argument *argumentList);
ctr_object *ctr_map_contains(ctr_object *myself, ctr_argument *argumentList);
ctr_object *ctr_map_flip(ctr_object *myself, ctr_argument *argumentList);
ctr_object *ctr_map_assign(ctr_object *myself, ctr_argument *argumentList);
ctr_object *ctr_map_to_string(ctr_object *myself, ctr_argument *argumentList);

ctr_object *ctr_console_writeln(ctr_object *myself, ctr_argument *argumentList);
ctr_object *ctr_console_write(ctr_object *myself, ctr_argument *argumentList);
ctr_object *ctr_console_brk(ctr_object *myself, ctr_argument *argumentList);
ctr_object *ctr_console_red(ctr_object *myself, ctr_argument *argumentList);
ctr_object *ctr_console_green(ctr_object *myself, ctr_argument *argumentList);
ctr_object *ctr_console_yellow(ctr_object *myself, ctr_argument *argumentList);
ctr_object *ctr_console_blue(ctr_object *myself, ctr_argument *argumentList);
ctr_object *ctr_console_magenta(ctr_object *myself, ctr_argument *argumentList);
ctr_object *ctr_console_cyan(ctr_object *myself, ctr_argument *argumentList);
ctr_object *ctr_console_reset(ctr_object *myself, ctr_argument *argumentList);
ctr_object *ctr_console_tab(ctr_object *myself, ctr_argument *argumentList);
ctr_object *ctr_console_line(ctr_object *myself, ctr_argument *argumentList);
ctr_object *ctr_console_clear(ctr_object *myself, ctr_argument *argumentList);
ctr_object *ctr_console_clear_line(ctr_object *myself,
                                   ctr_argument *argumentList);

ctr_object *ctr_file_new(ctr_object *myself, ctr_argument *argumentList);
ctr_object *ctr_file_special(ctr_object *myself, ctr_argument *argumentList);
ctr_object *ctr_file_get_descriptor(ctr_object *myself,
                                    ctr_argument *argumentList);
ctr_object *ctr_file_ddup(ctr_object *myself, ctr_argument *argumentList);
ctr_object *ctr_file_memopen(ctr_object *myself, ctr_argument *argumentList);
ctr_object *ctr_file_path(ctr_object *myself, ctr_argument *argumentList);
ctr_object *ctr_file_rpath(ctr_object *myself, ctr_argument *argumentList);
ctr_object *ctr_file_expand(ctr_object *myself, ctr_argument *argumentList);
ctr_object *ctr_file_read(ctr_object *myself, ctr_argument *argumentList);
ctr_object *ctr_file_write(ctr_object *myself, ctr_argument *argumentList);
ctr_object *ctr_file_append(ctr_object *myself, ctr_argument *argumentList);
ctr_object *ctr_file_exists(ctr_object *myself, ctr_argument *argumentList);
ctr_object *ctr_file_size(ctr_object *myself, ctr_argument *argumentList);
ctr_object *ctr_file_delete(ctr_object *myself, ctr_argument *argumentList);
ctr_object *ctr_file_include(ctr_object *myself, ctr_argument *argumentList);
ctr_object *ctr_file_include_here(ctr_object *myself,
                                  ctr_argument *argumentList);
ctr_object *ctr_file_open(ctr_object *myself, ctr_argument *argumentList);
ctr_object *ctr_file_close(ctr_object *myself, ctr_argument *argumentList);
ctr_object *ctr_file_flush(ctr_object *myself, ctr_argument *argumentList);
ctr_object *ctr_file_read_bytes(ctr_object *myself, ctr_argument *argumentList);
ctr_object *ctr_file_write_bytes(ctr_object *myself,
                                 ctr_argument *argumentList);
ctr_object *ctr_file_seek(ctr_object *myself, ctr_argument *argumentList);
ctr_object *ctr_file_seek_rewind(ctr_object *myself,
                                 ctr_argument *argumentList);
ctr_object *ctr_file_seek_end(ctr_object *myself, ctr_argument *argumentList);
ctr_object *ctr_file_descriptor(ctr_object *myself, ctr_argument *argumentList);
ctr_object *ctr_file_lock_generic(ctr_object *myself,
                                  ctr_argument *argumentList, int lock);
ctr_object *ctr_file_lock(ctr_object *myself, ctr_argument *argumentList);
ctr_object *ctr_file_unlock(ctr_object *myself, ctr_argument *argumentList);
ctr_object *ctr_file_type(ctr_object *myself, ctr_argument *argumentList);
ctr_object *ctr_file_list(ctr_object *myself, ctr_argument *argumentList);
ctr_object *ctr_file_tmp(ctr_object *myself, ctr_argument *argumentList);
ctr_object *ctr_file_stdext_path(ctr_object *myself,
                                 ctr_argument *argumentList);
ctr_object *ctr_file_assign(ctr_object *myself, ctr_argument *argumentList);
ctr_object *ctr_file_to_string(ctr_object *myself, ctr_argument *argumentList);
ctr_object *ctr_file_mkdir(ctr_object *myself, ctr_argument *argumentList);

void ctr_int_handler(int signal);
ctr_object *ctr_signal_map;
ctr_object *ctr_command_argument(ctr_object *myself,
                                 ctr_argument *argumentList);
ctr_object *ctr_command_num_of_args(ctr_object *myself,
                                    ctr_argument *argumentList);
ctr_object *ctr_command_waitforinput(ctr_object *myself,
                                     ctr_argument *argumentList);
ctr_object *ctr_command_getc(ctr_object *myself, ctr_argument *argumentList);
ctr_object *ctr_command_input(ctr_object *myself, ctr_argument *argumentList);
ctr_object *ctr_command_set_INT_handler(ctr_object *myself,
                                        ctr_argument *argumentList);
ctr_object *ctr_command_get_env(ctr_object *myself, ctr_argument *argumentList);
ctr_object *ctr_command_set_env(ctr_object *myself, ctr_argument *argumentList);
ctr_object *ctr_command_exit(ctr_object *myself, ctr_argument *argumentList);
ctr_object *ctr_command_flush(ctr_object *myself,
                              ctr_argument *ctr_argumentList);
ctr_object *ctr_command_forbid_shell(ctr_object *myself,
                                     ctr_argument *ctr_argumentList);
ctr_object *ctr_command_forbid_file_write(ctr_object *myself,
                                          ctr_argument *ctr_argumentList);
ctr_object *ctr_command_forbid_file_read(ctr_object *myself,
                                         ctr_argument *ctr_argumentList);
ctr_object *ctr_command_forbid_include(ctr_object *myself,
                                       ctr_argument *ctr_argumentList);
ctr_object *ctr_command_forbid_fork(ctr_object *myself,
                                    ctr_argument *ctr_argumentList);
ctr_object *ctr_command_countdown(ctr_object *myself,
                                  ctr_argument *ctr_argumentList);
ctr_object *ctr_command_fork(ctr_object *myself,
                             ctr_argument *ctr_argumentList);
ctr_object *ctr_command_message(ctr_object *myself, ctr_argument *argumentList);
ctr_object *ctr_command_listen(ctr_object *myself, ctr_argument *argumentList);
ctr_object *ctr_command_join(ctr_object *myself, ctr_argument *argumentList);
ctr_object *ctr_command_log(ctr_object *myself, ctr_argument *argumentList);
ctr_object *ctr_command_warn(ctr_object *myself, ctr_argument *argumentList);
ctr_object *ctr_command_err(ctr_object *myself, ctr_argument *argumentList);
ctr_object *ctr_command_crit(ctr_object *myself, ctr_argument *argumentList);
ctr_object *ctr_command_pid(ctr_object *myself, ctr_argument *argumentList);
ctr_object *ctr_command_sig(ctr_object *myself, ctr_argument *argumentList);
ctr_object *ctr_command_sigmap(ctr_object *myself, ctr_argument *argumentList);
ctr_object *ctr_command_accept(ctr_object *myself, ctr_argument *argumentList);
ctr_object *ctr_command_accepti4(ctr_object *myself,
                                 ctr_argument *argumentList);
ctr_object *ctr_command_accept_number(ctr_object *myself,
                                      ctr_argument *argumentList);
ctr_object *ctr_command_remote(ctr_object *myself, ctr_argument *argumentList);
ctr_object *ctr_command_default_port(ctr_object *myself,
                                     ctr_argument *argumentList);
ctr_object *ctr_command_chdir(ctr_object *myself, ctr_argument *argumentList);

void ctr_check_permission(uint8_t operationID);
int ctr_check_permission_internal(uint8_t operationID);
uint8_t ctr_command_security_profile;
uint64_t ctr_command_tick;
uint64_t ctr_command_maxtick;
ctr_object *(*ctr_secpro_eval_whitelist[64])(ctr_object *, ctr_argument *);
ctr_object *ctr_global_interrupt_handler;

ctr_object *ctr_clock_change(ctr_object *myself, ctr_argument *argumentList,
                             uint8_t forward);
ctr_object *ctr_clock_wait(ctr_object *myself, ctr_argument *argumentList);
ctr_object *ctr_clock_time(ctr_object *myself, ctr_argument *argumentList);
ctr_object *ctr_clock_new(ctr_object *myself, ctr_argument *argumentList);
ctr_object *ctr_clock_new_set(ctr_object *myself, ctr_argument *argumentList);
ctr_object *ctr_clock_like(ctr_object *myself, ctr_argument *argumentList);
ctr_object *ctr_clock_day(ctr_object *myself, ctr_argument *argumentList);
ctr_object *ctr_clock_month(ctr_object *myself, ctr_argument *argumentList);
ctr_object *ctr_clock_year(ctr_object *myself, ctr_argument *argumentList);
ctr_object *ctr_clock_hour(ctr_object *myself, ctr_argument *argumentList);
ctr_object *ctr_clock_minute(ctr_object *myself, ctr_argument *argumentList);
ctr_object *ctr_clock_second(ctr_object *myself, ctr_argument *argumentList);
ctr_object *ctr_clock_weekday(ctr_object *myself, ctr_argument *argumentList);
ctr_object *ctr_clock_yearday(ctr_object *myself, ctr_argument *argumentList);
ctr_object *ctr_clock_week(ctr_object *myself, ctr_argument *argumentList);
ctr_object *ctr_clock_set_day(ctr_object *myself, ctr_argument *argumentList);
ctr_object *ctr_clock_set_month(ctr_object *myself, ctr_argument *argumentList);
ctr_object *ctr_clock_set_year(ctr_object *myself, ctr_argument *argumentList);
ctr_object *ctr_clock_set_hour(ctr_object *myself, ctr_argument *argumentList);
ctr_object *ctr_clock_set_minute(ctr_object *myself,
                                 ctr_argument *argumentList);
ctr_object *ctr_clock_set_second(ctr_object *myself,
                                 ctr_argument *argumentList);
ctr_object *ctr_clock_get_time(ctr_object *myself, ctr_argument *argumentList,
                               char part);
ctr_object *ctr_clock_set_time(ctr_object *myself, ctr_argument *argumentList,
                               char part);
ctr_object *ctr_clock_set_zone(ctr_object *myself, ctr_argument *argumentList);
ctr_object *ctr_clock_get_zone(ctr_object *myself, ctr_argument *argumentList);
ctr_object *ctr_clock_to_string(ctr_object *myself, ctr_argument *argumentList);
ctr_object *ctr_clock_isdst(ctr_object *myself, ctr_argument *argumentList);
ctr_object *ctr_clock_format(ctr_object *myself, ctr_argument *argumentList);
ctr_object *ctr_clock_add(ctr_object *myself, ctr_argument *argumentList);
ctr_object *ctr_clock_subtract(ctr_object *myself, ctr_argument *argumentList);
ctr_object *ctr_clock_processor_time(ctr_object *myself,
                                     ctr_argument *argumentList);
ctr_object *ctr_clock_processor_ticks_ps(ctr_object *myself,
                                         ctr_argument *argumentList);
ctr_object *ctr_clock_time_exec(ctr_object *myself, ctr_argument *argumentList);
ctr_object *ctr_clock_time_exec_s(ctr_object *myself,
                                  ctr_argument *argumentList);
void ctr_clock_init(ctr_object *clock);

ctr_object *ctr_slurp_obtain(ctr_object *myself, ctr_argument *argumentList);
ctr_object *ctr_slurp_respond_to(ctr_object *myself,
                                 ctr_argument *argumentList);
ctr_object *ctr_slurp_respond_to_and(ctr_object *myself,
                                     ctr_argument *argumentList);

ctr_object *ctr_shell_call(ctr_object *myself, ctr_argument *argumentList);
ctr_object *ctr_shell_open(ctr_object *myself, ctr_argument *argumentList);
ctr_object *ctr_shell_respond_to(ctr_object *myself,
                                 ctr_argument *argumentList);
ctr_object *ctr_shell_respond_to_and(ctr_object *myself,
                                     ctr_argument *argumentList);

int CTR_LIMIT_MEM;
ctr_object *ctr_gc_collect(ctr_object *myself, ctr_argument *argumentList);
ctr_object *ctr_gc_with_gc_disabled(ctr_object *myself,
                                    ctr_argument *argumentList);
ctr_object *ctr_gc_dust(ctr_object *myself, ctr_argument *argumentList);
ctr_object *ctr_gc_object_count(ctr_object *myself, ctr_argument *argumentList);
ctr_object *ctr_gc_kept_count(ctr_object *myself, ctr_argument *argumentList);
ctr_object *ctr_gc_kept_alloc(ctr_object *myself, ctr_argument *argumentList);
ctr_object *ctr_gc_sticky_count(ctr_object *myself, ctr_argument *argumentList);
ctr_object *ctr_gc_setmode(ctr_object *myself, ctr_argument *argumentList);
ctr_object *ctr_gc_setautoalloc(ctr_object *myself, ctr_argument *argumentList);
ctr_object *ctr_gc_setmemlimit(ctr_object *myself, ctr_argument *argumentList);
void ctr_gc_sweep(int all);
void ctr_gc_pin(void *alloc_ptr);
void ctr_gc_unpin(void *alloc_ptr);

static ctr_object *ctr_reflect_map_type_descriptor __attribute__((unused));

ctr_object *ctr_reflect_add_glob(ctr_object *myself,
                                 ctr_argument *argumentList);
ctr_object *ctr_reflect_add_local(ctr_object *myself,
                                  ctr_argument *argumentList);
ctr_object *ctr_reflect_add_my(ctr_object *myself, ctr_argument *argumentList);
ctr_object *ctr_reflect_dump_context_prop(ctr_object *myself,
                                          ctr_argument *argumentList);
ctr_object *ctr_reflect_dump_context_spec_prop(ctr_object *myself,
                                               ctr_argument *argumentList);
ctr_object *ctr_reflect_dump_context_spec(ctr_object *myself,
                                          ctr_argument *argumentList);
ctr_object *ctr_reflect_dump_context(ctr_object *myself,
                                     ctr_argument *argumentList);
ctr_object *ctr_reflect_this_context(ctr_object *myself,
                                     ctr_argument *argumentList);
ctr_object *ctr_reflect_find_obj(ctr_object *myself,
                                 ctr_argument *argumentList);
ctr_object *ctr_reflect_find_obj_ex(ctr_object *myself,
                                    ctr_argument *argumentList);
ctr_object *ctr_reflect_new(ctr_object *myself, ctr_argument *argumentList);
ctr_object *ctr_reflect_set_to(ctr_object *myself, ctr_argument *argumentList);
ctr_object *ctr_reflect_cb_ac(ctr_object *myself, ctr_argument *argumentList);
ctr_object *ctr_reflect_cb_add_param(ctr_object *myself,
                                     ctr_argument *argumentList);
ctr_object *ctr_reflect_fn_copy(ctr_object *myself, ctr_argument *argumentList);
ctr_tnode *ctr_reflect_internal_term_rewrite(ctr_tnode *pfn, int nodety,
                                             ctr_tnode *rewrite_term,
                                             int replace_mask);
ctr_object *ctr_reflect_share_memory(ctr_object *myself,
                                     ctr_argument *argumentList);
ctr_object *ctr_reflect_link_to(ctr_object *myself, ctr_argument *argumentList);
ctr_object *ctr_reflect_child_of(ctr_object *myself,
                                 ctr_argument *argumentList);
ctr_object *ctr_reflect_is_linked_to(ctr_object *myself,
                                     ctr_argument *argumentList);
ctr_object *ctr_reflect_generate_inheritance_tree(ctr_object *myself,
                                                  ctr_argument *argumentList);
ctr_object *ctr_reflect_get_primitive_link(ctr_object *object);
ctr_object *ctr_reflect_describe_type(ctr_object *myself,
                                      ctr_argument *argumentList);
ctr_object *ctr_reflect_describe_value(ctr_object *myself,
                                       ctr_argument *argumentList);
ctr_object *ctr_reflect_type_descriptor_print(ctr_object *myself,
                                              ctr_argument *argumentList);
int ctr_reflect_check_bind_valid(ctr_object *from, ctr_object *to, int err);
ctr_object *ctr_reflect_check_bind_valid_v(ctr_object *myself,
                                           ctr_argument *argumentList);
ctr_object *ctr_reflect_bind(ctr_object *myself, ctr_argument *argumentList);
ctr_object *ctr_reflect_cons_of(ctr_object *myself, ctr_argument *argumentList);
ctr_object *ctr_reflect_try_serialize_block(ctr_object *myself,
                                            ctr_argument *argumentList);
ctr_object *ctr_reflect_cons_value(ctr_object *myself,
                                   ctr_argument *argumentList);
ctr_object *ctr_reflect_cons_str(ctr_object *myself,
                                 ctr_argument *argumentList);
ctr_object *ctr_reflect_get_first_link(ctr_object *myself,
                                       ctr_argument *argumentList);
ctr_object *ctr_reflect_get_responder(ctr_object *myself,
                                      ctr_argument *argumentList);
ctr_object *ctr_reflect_run_for_object(ctr_object *myself,
                                       ctr_argument *argumentList);
ctr_object *ctr_reflect_run_for_object_ctx(ctr_object *myself,
                                           ctr_argument *argumentList);
ctr_object *ctr_reflect_closure_of(ctr_object *myself,
                                   ctr_argument *argumentList);
ctr_object *ctr_reflect_get_primitive(ctr_object *myself,
                                      ctr_argument *argumentList);
ctr_object *ctr_reflect_get_property(ctr_object *myself,
                                     ctr_argument *argumentList);
ctr_object *ctr_reflect_set_property(ctr_object *myself,
                                     ctr_argument *argumentList);
ctr_object *ctr_reflect_has_own_responder(ctr_object *myself,
                                          ctr_argument *argumentList);
ctr_object *
ctr_reflect_object_delegate_get_responder(ctr_object *itself,
                                          ctr_argument *argumentList);
ctr_object *ctr_reflect_object_get_responder(ctr_object *itself,
                                             ctr_argument *argumentList);
ctr_object *ctr_reflect_rawmsg(ctr_object *myself, ctr_argument *argumentList);
ctr_object *ctr_reflect_instrmsg(ctr_object *myself,
                                 ctr_argument *argumentList);
ctr_object *ctr_reflect_register_instrumentor(ctr_object *myself,
                                              ctr_argument *argumentList);
ctr_object *ctr_reflect_unregister_instrumentor(ctr_object *myself,
                                                ctr_argument *argumentList);
ctr_object *ctr_reflect_get_instrumentor(ctr_object *myself,
                                         ctr_argument *argumentList);
ctr_object *ctr_reflect_ginstr(ctr_object *myself, ctr_argument *argumentList);
ctr_object *ctr_reflect_noginstr(ctr_object *myself,
                                 ctr_argument *argumentList);
ctr_object *ctr_reflect_world_snap(ctr_object *, ctr_argument *);
ctr_object *ctr_reflect_run_glob(ctr_object *myself,
                                 ctr_argument *argumentList);
ctr_object *ctr_reflect_run_for_object_in_ctx(ctr_object *myself,
                                              ctr_argument *argumentList);
ctr_object *ctr_reflect_run_for_object_inside_ctx(ctr_object *myself,
                                                  ctr_argument *argumentList);
ctr_object *
ctr_reflect_run_for_object_in_ctx_as_world(ctr_object *myself,
                                           ctr_argument *argumentList);
ctr_object *
ctr_reflect_run_for_object_in_ctx_as_main(ctr_object *myself,
                                          ctr_argument *argumentList);
ctr_object *ctr_reflect_run_in_new_ctx(ctr_object *myself,
                                       ctr_argument *argumentList);
ctr_object *ctr_reflect_compilerinfo(ctr_object *myself,
                                     ctr_argument *argumentList);
ctr_object *
ctr_reflect_delegate_set_private_property(ctr_object *itself,
                                          ctr_argument *argumentList);
ctr_object *ctr_reflect_set_ignore_file(ctr_object *myself,
                                        ctr_argument *argumentList);
ctr_object *ctr_reflect_nat_type(ctr_object *myself,
                                 ctr_argument *argumentList);
ctr_argument *ctr_array_to_argument_list(ctr_object *arr,
                                         ctr_argument *provided);
int ctr_internal_has_own_responder(ctr_object *myself, ctr_object *meth);
int ctr_internal_has_responder(ctr_object *myself, ctr_object *meth);

ctr_object *ctr_reflect_marshal(ctr_object *myself, ctr_argument *argumentList);
ctr_object *ctr_reflect_unmarshal(ctr_object *myself,
                                  ctr_argument *argumentList);

ctr_object *ctr_fiber_spawn(ctr_object *myself, ctr_argument *argumentList);
ctr_object *ctr_fiber_yield(ctr_object *myself, ctr_argument *argumentList);
ctr_object *ctr_fiber_join_all(ctr_object *myself, ctr_argument *argumentList);
ctr_object *ctr_fiber_tostring(ctr_object *myself, ctr_argument *argumentList);
ctr_object *ctr_fiber_yielded(ctr_object *myself, ctr_argument *argumentList);
ctr_object *ctr_fiber_assign(ctr_object *myself, ctr_argument *argumentList);
void ctr_fiber_begin_init(void);

ctr_object *ctr_thread_make(ctr_object *myself, ctr_argument *argumentList);
ctr_object *ctr_thread_make_set_target(ctr_object *myself,
                                       ctr_argument *argumentList);
ctr_object *ctr_thread_set_target(ctr_object *myself,
                                  ctr_argument *argumentList);
ctr_object *ctr_thread_run(ctr_object *myself, ctr_argument *argumentList);
ctr_object *ctr_thread_join(ctr_object *myself, ctr_argument *argumentList);
ctr_object *ctr_thread_id(ctr_object *myself, ctr_argument *argumentList);
ctr_object *ctr_thread_name(ctr_object *myself, ctr_argument *argumentList);
ctr_object *ctr_thread_names(ctr_object *myself, ctr_argument *argumentList);
ctr_object *ctr_thread_to_string(ctr_object *myself,
                                 ctr_argument *argumentList);

ctr_object *CtrStdImportLib;
ctr_object *CtrStdImportLib_SearchPaths;
ctr_object *ctr_importlib_begin(ctr_object *myself, ctr_argument *argumentList);

ctr_object *CtrStdInject;
voidptrfn_t ctr_inject_compiled_state_release_hook;
ctr_object *ctr_inject_make(ctr_object *myself, ctr_argument *argumentList);
ctr_object *ctr_inject_defined_functions(ctr_object *myself,
                                         ctr_argument *argumentList);
ctr_object *ctr_inject_compile(ctr_object *myself, ctr_argument *argumentList);
ctr_object *ctr_inject_get_symbol(ctr_object *myself,
                                  ctr_argument *argumentList);
ctr_object *ctr_inject_run(ctr_object *myself, ctr_argument *argumentList);
ctr_object *ctr_inject_run_named(ctr_object *myself,
                                 ctr_argument *argumentList);
ctr_object *ctr_inject_add_inclp(ctr_object *myself,
                                 ctr_argument *argumentList);
ctr_object *ctr_inject_get_inclp(ctr_object *myself,
                                 ctr_argument *argumentList);
ctr_object *ctr_inject_add_libp(ctr_object *myself, ctr_argument *argumentList);
ctr_object *ctr_inject_get_libp(ctr_object *myself, ctr_argument *argumentList);
ctr_object *ctr_inject_export_f(ctr_object *myself, ctr_argument *argumentList);
ctr_object *ctr_inject_add_lib(ctr_object *myself, ctr_argument *argumentList);
ctr_object *ctr_inject_set_error_handler(ctr_object *myself,
                                         ctr_argument *argumentList);
ctr_object *ctr_inject_generate_output(ctr_object *myself,
                                       ctr_argument *argumentList);
ctr_object *ctr_inject_finish(ctr_object *myself, ctr_argument *argumentList);

// # 1 "ctypes.h" 1
// # 23 "ctypes.h"
// # 1 "/usr/local/include/ffi.h" 1 3
// # 58 "/usr/local/include/ffi.h" 3
// # 1 "/usr/local/include/ffitarget.h" 1 3
// # 75 "/usr/local/include/ffitarget.h" 3

// # 75 "/usr/local/include/ffitarget.h" 3
typedef unsigned long ffi_arg;
typedef signed long ffi_sarg;

typedef enum ffi_abi {
// # 93 "/usr/local/include/ffitarget.h" 3
  FFI_FIRST_ABI = 1,
  FFI_UNIX64,
  FFI_WIN64,
  FFI_EFI64 = FFI_WIN64,
  FFI_GNUW64,
  FFI_LAST_ABI,
  FFI_DEFAULT_ABI = FFI_UNIX64
// # 124 "/usr/local/include/ffitarget.h" 3
} ffi_abi;
// # 59 "/usr/local/include/ffi.h" 2 3

// # 1 "/usr/lib/gcc/x86_64-pc-linux-gnu/8.2.1/include/stddef.h" 1 3 4
// # 149 "/usr/lib/gcc/x86_64-pc-linux-gnu/8.2.1/include/stddef.h" 3 4
typedef long int ptrdiff_t;
// # 426 "/usr/lib/gcc/x86_64-pc-linux-gnu/8.2.1/include/stddef.h" 3 4
typedef struct {
  long long __max_align_ll __attribute__((__aligned__(__alignof__(long long))));
  long double __max_align_ld
      __attribute__((__aligned__(__alignof__(long double))));
// # 437 "/usr/lib/gcc/x86_64-pc-linux-gnu/8.2.1/include/stddef.h" 3 4
} max_align_t;
// # 67 "/usr/local/include/ffi.h" 2 3
// # 1 "/usr/lib/gcc/x86_64-pc-linux-gnu/8.2.1/include-fixed/limits.h" 1 3 4
// # 34 "/usr/lib/gcc/x86_64-pc-linux-gnu/8.2.1/include-fixed/limits.h" 3 4
// # 1 "/usr/lib/gcc/x86_64-pc-linux-gnu/8.2.1/include-fixed/syslimits.h" 1 3 4

// # 1 "/usr/lib/gcc/x86_64-pc-linux-gnu/8.2.1/include-fixed/limits.h" 1 3 4
// # 194 "/usr/lib/gcc/x86_64-pc-linux-gnu/8.2.1/include-fixed/limits.h" 3 4
// # 1 "/usr/include/limits.h" 1 3 4
// # 26 "/usr/include/limits.h" 3 4
// # 1 "/usr/include/bits/libc-header-start.h" 1 3 4
// # 27 "/usr/include/limits.h" 2 3 4
// # 183 "/usr/include/limits.h" 3 4
// # 1 "/usr/include/bits/posix1_lim.h" 1 3 4
// # 27 "/usr/include/bits/posix1_lim.h" 3 4
// # 1 "/usr/include/bits/wordsize.h" 1 3 4
// # 28 "/usr/include/bits/posix1_lim.h" 2 3 4
// # 161 "/usr/include/bits/posix1_lim.h" 3 4
// # 1 "/usr/include/bits/local_lim.h" 1 3 4
// # 38 "/usr/include/bits/local_lim.h" 3 4
// # 1 "/usr/include/linux/limits.h" 1 3 4
// # 39 "/usr/include/bits/local_lim.h" 2 3 4
// # 162 "/usr/include/bits/posix1_lim.h" 2 3 4
// # 184 "/usr/include/limits.h" 2 3 4

// # 1 "/usr/include/bits/posix2_lim.h" 1 3 4
// # 188 "/usr/include/limits.h" 2 3 4
// # 195 "/usr/lib/gcc/x86_64-pc-linux-gnu/8.2.1/include-fixed/limits.h" 2 3 4
// # 8 "/usr/lib/gcc/x86_64-pc-linux-gnu/8.2.1/include-fixed/syslimits.h" 2 3 4
// # 35 "/usr/lib/gcc/x86_64-pc-linux-gnu/8.2.1/include-fixed/limits.h" 2 3 4
// # 68 "/usr/local/include/ffi.h" 2 3
// # 103 "/usr/local/include/ffi.h" 3
typedef struct _ffi_type {
  size_t size;
  unsigned short alignment;
  unsigned short type;
  struct _ffi_type **elements;
} ffi_type;
// # 187 "/usr/local/include/ffi.h" 3
extern ffi_type ffi_type_void;
extern ffi_type ffi_type_uint8;
extern ffi_type ffi_type_sint8;
extern ffi_type ffi_type_uint16;
extern ffi_type ffi_type_sint16;
extern ffi_type ffi_type_uint32;
extern ffi_type ffi_type_sint32;
extern ffi_type ffi_type_uint64;
extern ffi_type ffi_type_sint64;
extern ffi_type ffi_type_float;
extern ffi_type ffi_type_double;
extern ffi_type ffi_type_pointer;

extern ffi_type ffi_type_longdouble;

extern ffi_type ffi_type_complex_float;
extern ffi_type ffi_type_complex_double;

extern ffi_type ffi_type_complex_longdouble;

typedef enum { FFI_OK = 0, FFI_BAD_TYPEDEF, FFI_BAD_ABI } ffi_status;

typedef struct {
  ffi_abi abi;
  unsigned nargs;
  ffi_type **arg_types;
  ffi_type *rtype;
  unsigned bytes;
  unsigned flags;

} ffi_cif;
// # 249 "/usr/local/include/ffi.h" 3
typedef union {
  ffi_sarg sint;
  ffi_arg uint;
  float flt;
  char data[8];
  void *ptr;
} ffi_raw;
// # 268 "/usr/local/include/ffi.h" 3
typedef ffi_raw ffi_java_raw;

void ffi_raw_call(ffi_cif *cif, void (*fn)(void), void *rvalue,
                  ffi_raw *avalue);

void ffi_ptrarray_to_raw(ffi_cif *cif, void **args, ffi_raw *raw);
void ffi_raw_to_ptrarray(ffi_cif *cif, ffi_raw *raw, void **args);
size_t ffi_raw_size(ffi_cif *cif);

void ffi_java_raw_call(ffi_cif *cif, void (*fn)(void), void *rvalue,
                       ffi_java_raw *avalue);

void ffi_java_ptrarray_to_raw(ffi_cif *cif, void **args, ffi_java_raw *raw);

void ffi_java_raw_to_ptrarray(ffi_cif *cif, ffi_java_raw *raw, void **args);

size_t ffi_java_raw_size(ffi_cif *cif);
// # 306 "/usr/local/include/ffi.h" 3
typedef struct {

  char tramp[24];

  ffi_cif *cif;
  void (*fun)(ffi_cif *, void *, void **, void *);
  void *user_data;
} ffi_closure

    __attribute__((aligned(8)))

    ;

void *ffi_closure_alloc(size_t size, void **code);
void ffi_closure_free(void *);

ffi_status ffi_prep_closure(ffi_closure *, ffi_cif *,
                            void (*fun)(ffi_cif *, void *, void **, void *),
                            void *user_data)

    __attribute__((deprecated("use ffi_prep_closure_loc instead")))

    ;

ffi_status ffi_prep_closure_loc(ffi_closure *, ffi_cif *,
                                void (*fun)(ffi_cif *, void *, void **, void *),
                                void *user_data, void *codeloc);

typedef struct {

  char tramp[24];

  ffi_cif *cif;

  void (*translate_args)(ffi_cif *, void *, void **, void *);
  void *this_closure;

  void (*fun)(ffi_cif *, void *, ffi_raw *, void *);
  void *user_data;

} ffi_raw_closure;

typedef struct {

  char tramp[24];

  ffi_cif *cif;

  void (*translate_args)(ffi_cif *, void *, void **, void *);
  void *this_closure;

  void (*fun)(ffi_cif *, void *, ffi_java_raw *, void *);
  void *user_data;

} ffi_java_raw_closure;

ffi_status ffi_prep_raw_closure(ffi_raw_closure *, ffi_cif *cif,
                                void (*fun)(ffi_cif *, void *, ffi_raw *,
                                            void *),
                                void *user_data);

ffi_status ffi_prep_raw_closure_loc(ffi_raw_closure *, ffi_cif *cif,
                                    void (*fun)(ffi_cif *, void *, ffi_raw *,
                                                void *),
                                    void *user_data, void *codeloc);

ffi_status ffi_prep_java_raw_closure(ffi_java_raw_closure *, ffi_cif *cif,
                                     void (*fun)(ffi_cif *, void *,
                                                 ffi_java_raw *, void *),
                                     void *user_data);

ffi_status ffi_prep_java_raw_closure_loc(ffi_java_raw_closure *, ffi_cif *cif,
                                         void (*fun)(ffi_cif *, void *,
                                                     ffi_java_raw *, void *),
                                         void *user_data, void *codeloc);

typedef struct {
  void *tramp;
  ffi_cif *cif;
  void (*fun)(ffi_cif *, void *, void **, void *);
} ffi_go_closure;

ffi_status ffi_prep_go_closure(ffi_go_closure *, ffi_cif *,
                               void (*fun)(ffi_cif *, void *, void **, void *));

void ffi_call_go(ffi_cif *cif, void (*fn)(void), void *rvalue, void **avalue,
                 void *closure);

ffi_status ffi_prep_cif(ffi_cif *cif, ffi_abi abi, unsigned int nargs,
                        ffi_type *rtype, ffi_type **atypes);

ffi_status ffi_prep_cif_var(ffi_cif *cif, ffi_abi abi, unsigned int nfixedargs,
                            unsigned int ntotalargs, ffi_type *rtype,
                            ffi_type **atypes);

void ffi_call(ffi_cif *cif, void (*fn)(void), void *rvalue, void **avalue);

ffi_status ffi_get_struct_offsets(ffi_abi abi, ffi_type *struct_type,
                                  size_t *offsets);
// # 24 "ctypes.h" 2

// # 1 "citron.h" 1
// # 29 "ctypes.h" 2

// # 1 "_struct.h" 1

// # 1 "/usr/lib/gcc/x86_64-pc-linux-gnu/8.2.1/include/stdbool.h" 1 3 4
// # 5 "_struct.h" 2

// # 8 "_struct.h"
typedef struct {
  char c;
  short x;
} st_short;
typedef struct {
  char c;
  int x;
} st_int;
typedef struct {
  char c;
  long x;
} st_long;
typedef struct {
  char c;
  float x;
} st_float;
typedef struct {
  char c;
  double x;
} st_double;
typedef struct {
  char c;
  void *x;
} st_void_p;
typedef struct {
  char c;
  size_t x;
} st_size_t;
typedef struct {
  char c;
// # 15 "_struct.h" 3 4
  _Bool
// # 15 "_struct.h"
      x;
} st_bool;
// # 26 "_struct.h"
typedef struct {
  char c;
  long long x;
} s_long_long;

typedef struct {
  unsigned int pad : 1;
  ptrdiff_t offset;
} pad_info_node_t;

typedef struct {
  size_t member_count;
  size_t max_alignment;
  pad_info_node_t **pad_structure;
} struct_member_desc_t;

struct_member_desc_t
ctr_ffi_type_get_member_count(char *format, size_t *size_out, int record_pads);
int ctr_ffi_type_struct_sizeof(char *format);
ffi_type *ctr_create_ffi_type_descriptor(char *format);
ffi_type *ctr_create_ffi_type_descriptor_(char *format, int member_count);
int ctr_create_ffi_str_descriptor(ffi_type *type, char *buf);
// # 31 "ctypes.h" 2
// # 1 "structmember.h" 1

// # 1 "/usr/lib/gcc/x86_64-pc-linux-gnu/8.2.1/include-fixed/limits.h" 1 3 4
// # 5 "structmember.h" 2
// # 1 "/usr/lib/gcc/x86_64-pc-linux-gnu/8.2.1/include/float.h" 1 3 4
// # 6 "structmember.h" 2

// # 1 "ctypes.h" 1
// # 9 "structmember.h" 2
// # 1 "reverse_sizes.h" 1
// # 9 "reverse_sizes.h"
struct reverse_ffi_type_size_map_item {
  ffi_type *type;
  size_t size;
};

static struct reverse_ffi_type_size_map_item reverse_ffi_type_size_map[] = {
    {&ffi_type_uint8, sizeof(uint8_t)},
    {&ffi_type_sint8, sizeof(int8_t)},
    {&ffi_type_uint16, sizeof(uint16_t)},
    {&ffi_type_sint16, sizeof(int16_t)},
    {&ffi_type_uint32, sizeof(uint32_t)},
    {&ffi_type_sint32, sizeof(int32_t)},
    {&ffi_type_uint64, sizeof(uint64_t)},
    {&ffi_type_sint64, sizeof(int64_t)},
    {&ffi_type_float, sizeof(float)},
    {&ffi_type_double, sizeof(double)},
    {&
// # 25 "reverse_sizes.h" 3
     ffi_type_uint8
// # 25 "reverse_sizes.h"
     ,
     sizeof(unsigned char)},
    {&
// # 26 "reverse_sizes.h" 3
     ffi_type_sint8
// # 26 "reverse_sizes.h"
     ,
     sizeof(char)},
    {&
// # 27 "reverse_sizes.h" 3
     ffi_type_uint16
// # 27 "reverse_sizes.h"
     ,
     sizeof(unsigned short)},
    {&
// # 28 "reverse_sizes.h" 3
     ffi_type_sint16
// # 28 "reverse_sizes.h"
     ,
     sizeof(short)},
    {&
// # 29 "reverse_sizes.h" 3
     ffi_type_uint32
// # 29 "reverse_sizes.h"
     ,
     sizeof(unsigned int)},
    {&
// # 30 "reverse_sizes.h" 3
     ffi_type_sint32
// # 30 "reverse_sizes.h"
     ,
     sizeof(int)},
    {&
// # 31 "reverse_sizes.h" 3
     ffi_type_uint64
// # 31 "reverse_sizes.h"
     ,
     sizeof(unsigned long)},
    {&
// # 32 "reverse_sizes.h" 3
     ffi_type_sint64
// # 32 "reverse_sizes.h"
     ,
     sizeof(long)},
    {&ffi_type_longdouble, sizeof(long double)},
    {&ffi_type_pointer, sizeof(void *)},
    {
// # 35 "reverse_sizes.h" 3 4
        ((void *)0)
// # 35 "reverse_sizes.h"
            ,
        0}};

static size_t reverse_ffi_type_size_map_lookup(ffi_type *type) {
  int i = 0;
  while (reverse_ffi_type_size_map[i].type !=
// # 40 "reverse_sizes.h" 3 4
         ((void *)0)
// # 40 "reverse_sizes.h"
  ) {
    if (reverse_ffi_type_size_map[i].type == type)
      return reverse_ffi_type_size_map[i].size;
    i++;
  }
  printf("REVERSE LOOKUP %p\n", type);
  return 0;
}
// # 10 "structmember.h" 2

ctr_object *nuchar(char *p);
ctr_object *nuuchar(char *p);
ctr_object *nubyte(char *p);
ctr_object *nuubyte(char *p);
ctr_object *nushort(char *p);
ctr_object *nuushort(char *p);
ctr_object *nuint(char *p);
ctr_object *nuuint(char *p);
ctr_object *nuint8(char *p);
ctr_object *nuuint8(char *p);
ctr_object *nuint16(char *p);
ctr_object *nuuint16(char *p);
ctr_object *nuint32(char *p);
ctr_object *nuuint32(char *p);
ctr_object *nuint64(char *p);
ctr_object *nuuint64(char *p);
ctr_object *nulong(char *p);
ctr_object *nuulong(char *p);
ctr_object *nulonglong(char *p);
ctr_object *nuulonglong(char *p);
ctr_object *nubool(char *p);
ctr_object *nufloat(char *p);
ctr_object *nudouble(char *p);
ctr_object *nupointer(char *p);
int npdispatch(char *p, ctr_object *o, ffi_type *type);
ctr_object *nudispatch(char *p, ffi_type *type);
int npbyte(char *p, ctr_object *o);
int npubyte(char *p, ctr_object *o);
int npchar(char *p, ctr_object *o);
int npuchar(char *p, ctr_object *o);
int npshort(char *p, ctr_object *o);
int npushort(char *p, ctr_object *o);
int npint(char *p, ctr_object *o);
int npuint(char *p, ctr_object *o);
int npint8(char *p, ctr_object *o);
int npuint8(char *p, ctr_object *o);
int npint16(char *p, ctr_object *o);
int npuint16(char *p, ctr_object *o);
int npint32(char *p, ctr_object *o);
int npuint32(char *p, ctr_object *o);
int npint64(char *p, ctr_object *o);
int npuint64(char *p, ctr_object *o);
int nplong(char *p, ctr_object *o);
int nplonglong(char *p, ctr_object *o);
int npbool(char *p, ctr_object *o);
int npfloat(char *p, ctr_object *o);
int npdouble(char *p, ctr_object *o);
int nppointer(char *p, ctr_object *o);
// # 32 "ctypes.h" 2

struct ctr_ffi_cfi {
  ffi_cif *cif;
  ffi_type *rtype;
  ffi_type **atypes;
};
typedef struct ctr_ffi_cfi ctr_ffi_cfi;

enum ctr_ctype {
  CTR_CTYPE_INVALID = -1,
  CTR_CTYPE_VOID = 0,
  CTR_CTYPE_UINT8,
  CTR_CTYPE_SINT8,
  CTR_CTYPE_UINT16,
  CTR_CTYPE_SINT16,
  CTR_CTYPE_UINT32,
  CTR_CTYPE_SINT32,
  CTR_CTYPE_UINT64,
  CTR_CTYPE_SINT64,
  CTR_CTYPE_FLOAT,
  CTR_CTYPE_DOUBLE,
  CTR_CTYPE_UCHAR,
  CTR_CTYPE_SCHAR,
  CTR_CTYPE_USHORT,
  CTR_CTYPE_SSHORT,
  CTR_CTYPE_UINT,
  CTR_CTYPE_SINT,
  CTR_CTYPE_ULONG,
  CTR_CTYPE_SLONG,
  CTR_CTYPE_LONGDOUBLE,
  CTR_CTYPE_POINTER,
  CTR_CTYPE_CIF,
  CTR_CTYPE_DYN_LIB,
  CTR_CTYPE_STRUCT,

  CTR_CTYPE_STRING,
  CTR_CTYPE_FUNCTION_POINTER,
  CTR_CTYPE_CONTIGUOUS_ARRAY
};
typedef enum ctr_ctype ctr_ctype;

typedef struct {
  int member_count;
  size_t size;
  ffi_type *type;
  pad_info_node_t **padinfo;
  void *value;
  char *original_format;
} ctr_ctypes_ffi_struct_value;

typedef struct ctr_ctypes_cont_array {
  size_t count;
  size_t esize;
  void *storage;
  ffi_type *etype;
} ctr_ctypes_cont_array_t;

ctr_object *CtrStdCType;
ctr_object *CtrStdCType_ffi_cif;

ctr_object *CtrStdCType_void;
ctr_object *ctr_ctypes_make_void(ctr_object *myself,
                                 ctr_argument *argumentList);
ctr_object *ctr_ctypes_set_void(ctr_object *myself, ctr_argument *argumentList);
ctr_object *ctr_ctypes_get_void(ctr_object *myself, ctr_argument *argumentList);
ctr_object *ctr_ctypes_to_str_void(ctr_object *myself,
                                   ctr_argument *argumentList);

ctr_object *CtrStdCType_uint8;
ctr_object *ctr_ctypes_make_uint8(ctr_object *myself,
                                  ctr_argument *argumentList);
ctr_object *ctr_ctypes_set_uint8(ctr_object *myself,
                                 ctr_argument *argumentList);
ctr_object *ctr_ctypes_get_uint8(ctr_object *myself,
                                 ctr_argument *argumentList);
ctr_object *ctr_ctypes_to_str_uint8(ctr_object *myself,
                                    ctr_argument *argumentList);

ctr_object *CtrStdCType_sint8;
ctr_object *ctr_ctypes_make_sint8(ctr_object *myself,
                                  ctr_argument *argumentList);
ctr_object *ctr_ctypes_set_sint8(ctr_object *myself,
                                 ctr_argument *argumentList);
ctr_object *ctr_ctypes_get_sint8(ctr_object *myself,
                                 ctr_argument *argumentList);
ctr_object *ctr_ctypes_to_str_sint8(ctr_object *myself,
                                    ctr_argument *argumentList);

ctr_object *CtrStdCType_uint16;
ctr_object *ctr_ctypes_make_uint16(ctr_object *myself,
                                   ctr_argument *argumentList);
ctr_object *ctr_ctypes_set_uint16(ctr_object *myself,
                                  ctr_argument *argumentList);
ctr_object *ctr_ctypes_get_uint16(ctr_object *myself,
                                  ctr_argument *argumentList);
ctr_object *ctr_ctypes_to_str_uint16(ctr_object *myself,
                                     ctr_argument *argumentList);

ctr_object *CtrStdCType_sint16;
ctr_object *ctr_ctypes_make_sint16(ctr_object *myself,
                                   ctr_argument *argumentList);
ctr_object *ctr_ctypes_set_sint16(ctr_object *myself,
                                  ctr_argument *argumentList);
ctr_object *ctr_ctypes_get_sint16(ctr_object *myself,
                                  ctr_argument *argumentList);
ctr_object *ctr_ctypes_to_str_sint16(ctr_object *myself,
                                     ctr_argument *argumentList);

ctr_object *CtrStdCType_uint32;
ctr_object *ctr_ctypes_make_uint32(ctr_object *myself,
                                   ctr_argument *argumentList);
ctr_object *ctr_ctypes_set_uint32(ctr_object *myself,
                                  ctr_argument *argumentList);
ctr_object *ctr_ctypes_get_uint32(ctr_object *myself,
                                  ctr_argument *argumentList);
ctr_object *ctr_ctypes_to_str_uint32(ctr_object *myself,
                                     ctr_argument *argumentList);

ctr_object *CtrStdCType_sint32;
ctr_object *ctr_ctypes_make_sint32(ctr_object *myself,
                                   ctr_argument *argumentList);
ctr_object *ctr_ctypes_set_sint32(ctr_object *myself,
                                  ctr_argument *argumentList);
ctr_object *ctr_ctypes_get_sint32(ctr_object *myself,
                                  ctr_argument *argumentList);
ctr_object *ctr_ctypes_to_str_sint32(ctr_object *myself,
                                     ctr_argument *argumentList);

ctr_object *CtrStdCType_uint64;
ctr_object *ctr_ctypes_make_uint64(ctr_object *myself,
                                   ctr_argument *argumentList);
ctr_object *ctr_ctypes_set_uint64(ctr_object *myself,
                                  ctr_argument *argumentList);
ctr_object *ctr_ctypes_get_uint64(ctr_object *myself,
                                  ctr_argument *argumentList);
ctr_object *ctr_ctypes_to_str_uint64(ctr_object *myself,
                                     ctr_argument *argumentList);

ctr_object *CtrStdCType_sint64;
ctr_object *ctr_ctypes_make_sint64(ctr_object *myself,
                                   ctr_argument *argumentList);
ctr_object *ctr_ctypes_set_sint64(ctr_object *myself,
                                  ctr_argument *argumentList);
ctr_object *ctr_ctypes_get_sint64(ctr_object *myself,
                                  ctr_argument *argumentList);
ctr_object *ctr_ctypes_to_str_sint64(ctr_object *myself,
                                     ctr_argument *argumentList);

ctr_object *CtrStdCType_float;
ctr_object *ctr_ctypes_make_float(ctr_object *myself,
                                  ctr_argument *argumentList);
ctr_object *ctr_ctypes_set_float(ctr_object *myself,
                                 ctr_argument *argumentList);
ctr_object *ctr_ctypes_get_float(ctr_object *myself,
                                 ctr_argument *argumentList);
ctr_object *ctr_ctypes_to_str_float(ctr_object *myself,
                                    ctr_argument *argumentList);

ctr_object *CtrStdCType_double;
ctr_object *ctr_ctypes_make_double(ctr_object *myself,
                                   ctr_argument *argumentList);
ctr_object *ctr_ctypes_set_double(ctr_object *myself,
                                  ctr_argument *argumentList);
ctr_object *ctr_ctypes_get_double(ctr_object *myself,
                                  ctr_argument *argumentList);
ctr_object *ctr_ctypes_to_str_double(ctr_object *myself,
                                     ctr_argument *argumentList);

ctr_object *CtrStdCType_uchar;
ctr_object *ctr_ctypes_make_uchar(ctr_object *myself,
                                  ctr_argument *argumentList);
ctr_object *ctr_ctypes_set_uchar(ctr_object *myself,
                                 ctr_argument *argumentList);
ctr_object *ctr_ctypes_get_uchar(ctr_object *myself,
                                 ctr_argument *argumentList);
ctr_object *ctr_ctypes_to_str_uchar(ctr_object *myself,
                                    ctr_argument *argumentList);

ctr_object *CtrStdCType_schar;
ctr_object *ctr_ctypes_make_schar(ctr_object *myself,
                                  ctr_argument *argumentList);
ctr_object *ctr_ctypes_set_schar(ctr_object *myself,
                                 ctr_argument *argumentList);
ctr_object *ctr_ctypes_get_schar(ctr_object *myself,
                                 ctr_argument *argumentList);
ctr_object *ctr_ctypes_to_str_schar(ctr_object *myself,
                                    ctr_argument *argumentList);

ctr_object *CtrStdCType_ushort;
ctr_object *ctr_ctypes_make_ushort(ctr_object *myself,
                                   ctr_argument *argumentList);
ctr_object *ctr_ctypes_set_ushort(ctr_object *myself,
                                  ctr_argument *argumentList);
ctr_object *ctr_ctypes_get_ushort(ctr_object *myself,
                                  ctr_argument *argumentList);
ctr_object *ctr_ctypes_to_str_ushort(ctr_object *myself,
                                     ctr_argument *argumentList);

ctr_object *CtrStdCType_sshort;
ctr_object *ctr_ctypes_make_sshort(ctr_object *myself,
                                   ctr_argument *argumentList);
ctr_object *ctr_ctypes_set_sshort(ctr_object *myself,
                                  ctr_argument *argumentList);
ctr_object *ctr_ctypes_get_sshort(ctr_object *myself,
                                  ctr_argument *argumentList);
ctr_object *ctr_ctypes_to_str_sshort(ctr_object *myself,
                                     ctr_argument *argumentList);

ctr_object *CtrStdCType_uint;
ctr_object *ctr_ctypes_make_uint(ctr_object *myself,
                                 ctr_argument *argumentList);
ctr_object *ctr_ctypes_set_uint(ctr_object *myself, ctr_argument *argumentList);
ctr_object *ctr_ctypes_get_uint(ctr_object *myself, ctr_argument *argumentList);
ctr_object *ctr_ctypes_to_str_uint(ctr_object *myself,
                                   ctr_argument *argumentList);

ctr_object *CtrStdCType_sint;
ctr_object *ctr_ctypes_make_sint(ctr_object *myself,
                                 ctr_argument *argumentList);
ctr_object *ctr_ctypes_set_sint(ctr_object *myself, ctr_argument *argumentList);
ctr_object *ctr_ctypes_get_sint(ctr_object *myself, ctr_argument *argumentList);
ctr_object *ctr_ctypes_to_str_sint(ctr_object *myself,
                                   ctr_argument *argumentList);

ctr_object *CtrStdCType_ulong;
ctr_object *ctr_ctypes_make_ulong(ctr_object *myself,
                                  ctr_argument *argumentList);
ctr_object *ctr_ctypes_set_ulong(ctr_object *myself,
                                 ctr_argument *argumentList);
ctr_object *ctr_ctypes_get_ulong(ctr_object *myself,
                                 ctr_argument *argumentList);
ctr_object *ctr_ctypes_to_str_ulong(ctr_object *myself,
                                    ctr_argument *argumentList);

ctr_object *CtrStdCType_slong;
ctr_object *ctr_ctypes_make_slong(ctr_object *myself,
                                  ctr_argument *argumentList);
ctr_object *ctr_ctypes_set_slong(ctr_object *myself,
                                 ctr_argument *argumentList);
ctr_object *ctr_ctypes_get_slong(ctr_object *myself,
                                 ctr_argument *argumentList);
ctr_object *ctr_ctypes_to_str_slong(ctr_object *myself,
                                    ctr_argument *argumentList);

ctr_object *CtrStdCType_longdouble;
ctr_object *ctr_ctypes_make_longdouble(ctr_object *myself,
                                       ctr_argument *argumentList);
ctr_object *ctr_ctypes_set_longdouble(ctr_object *myself,
                                      ctr_argument *argumentList);
ctr_object *ctr_ctypes_get_longdouble(ctr_object *myself,
                                      ctr_argument *argumentList);
ctr_object *ctr_ctypes_to_str_longdouble(ctr_object *myself,
                                         ctr_argument *argumentList);

ctr_object *CtrStdCType_pointer;
ctr_object *ctr_ctypes_make_pointer(ctr_object *myself,
                                    ctr_argument *argumentList);
ctr_object *ctr_ctypes_set_pointer(ctr_object *myself,
                                   ctr_argument *argumentList);
ctr_object *ctr_ctypes_str_pointer(ctr_object *myself,
                                   ctr_argument *argumentList);

ctr_object *CtrStdCType_dynamic_lib;
ctr_object *ctr_ctypes_make_dynamic_lib(ctr_object *myself,
                                        ctr_argument *argumentList);
ctr_object *ctr_ctypes_get_dynamic_lib(ctr_object *myself,
                                       ctr_argument *argumentList);
ctr_object *ctr_ctypes_to_str_dynamic_lib(ctr_object *myself,
                                          ctr_argument *argumentList);
ctr_object *ctr_ctypes_str_dynamic_lib(ctr_object *myself,
                                       ctr_argument *argumentList);

ctr_object *CtrStdCType_struct;
ctr_object *ctr_ctypes_make_struct(ctr_object *myself,
                                   ctr_argument *argumentList);

ctr_object *CtrStdCType_cont_pointer;
ctr_object *ctr_ctypes_make_cont_pointer(ctr_object *myself,
                                         ctr_argument *argumentList);

ctr_object *CtrStdCType_string;
ctr_object *ctr_ctypes_make_string(ctr_object *myself,
                                   ctr_argument *argumentList);
ctr_object *ctr_ctypes_set_string(ctr_object *myself,
                                  ctr_argument *argumentList);
ctr_object *ctr_ctypes_str_string(ctr_object *myself,
                                  ctr_argument *argumentList);

ctr_object *CtrStdCType_functionptr;
ctr_object *ctr_ctypes_make_functionptr(ctr_object *myself,
                                        ctr_argument *argumentList);
ctr_object *ctr_ctypes_set_functionptr(ctr_object *myself,
                                       ctr_argument *argumentList);
ctr_object *ctr_ctypes_str_functionptr(ctr_object *myself,
                                       ctr_argument *argumentList);

ctr_object *ctr_ctype_ffi_prep_cif(ctr_object *myself,
                                   ctr_argument *argumentList);
ctr_object *ctr_ctype_ffi_cif_new(ctr_object *myself,
                                  ctr_argument *argumentList);
ctr_object *ctr_ctype_ffi_call(ctr_object *myself, ctr_argument *argumentList);

ffi_type *ctr_ctypes_ffi_convert_to_ffi_type(ctr_object *type);
ctr_object *ctr_ctypes_get_first_meta(ctr_object *object, ctr_object *last);
// # 1327 "citron.h" 2
void ctr_ffi_begin();

int ctr_gc_dust_counter;
int ctr_gc_object_counter;
int ctr_gc_kept_counter;
int ctr_gc_sticky_counter;
int ctr_gc_mode;

uint64_t ctr_gc_alloc;
uint64_t ctr_gc_memlimit;

ctr_object *ctr_dice_throw(ctr_object *myself, ctr_argument *argumentList);
ctr_object *ctr_dice_sides(ctr_object *myself, ctr_argument *argumentList);
ctr_object *ctr_dice_rand(ctr_object *myself, ctr_argument *argumentList);

ctr_object *ctr_build_empty_string();
ctr_object *ctr_build_string(char *object, long vlen);
ctr_object *ctr_build_string_from_cformat(char *format, int count, ...);
ctr_object *ctr_build_block(ctr_tnode *node);
ctr_object *ctr_build_listcomp(ctr_tnode *node);
ctr_object *ctr_build_number(char *object);
ctr_object *ctr_build_number_from_string(char *fixedStr, ctr_size strLength);
ctr_object *ctr_build_number_from_float(ctr_number floatNumber);
ctr_object *ctr_build_bool(int truth);
ctr_object *ctr_build_nil();
ctr_object *ctr_build_string_from_cstring(char *str);
void ctr_gc_internal_collect();
ctr_object *ctr_gc_sweep_this(ctr_object *myself, ctr_argument *argumentList);

void *ctr_heap_allocate_shared(size_t size);
void *ctr_heap_allocate(size_t size);

void *ctr_heap_allocate_tracked(size_t size);
void ctr_heap_free(void *ptr);
void ctr_heap_free_shared(void *ptr);
void ctr_heap_free_rest();
void *ctr_heap_reallocate(void *oldptr, size_t size);
size_t ctr_heap_get_latest_tracking_id();
void *ctr_heap_reallocate_tracked(size_t tracking_id, size_t size);
char *ctr_heap_allocate_cstring(ctr_object *o);
char *ctr_heap_allocate_cstring_shared(ctr_object *o);

void initiailize_base_extensions();

uint8_t ctr_accept_n_connections;
uint16_t ctr_default_port;

// # 1 "citron_ensure.h" 1

typedef struct {
  int typecode;
  const char *typestring;
} typeType_t;

static const typeType_t typeType[] = {{0, "Nil"},
                                      {1, "Bool"},
                                      {2, "Number"},
                                      {3, "String"},
                                      {4, "Block"},
                                      {5, "Object"},
                                      {6, "NativeFunction"},
                                      {7, "Array"},
                                      {8, "Miscellaneous"},
                                      {9, "ExternalResource"}};

static inline ctr_object *ctr_can_cast_to_string(ctr_object *object) {
  ctr_object *out = ctr_internal_cast2string(object);
  if (CtrStdFlow) {
    CtrStdFlow =
// # 28 "citron_ensure.h" 3 4
        ((void *)0)
// # 28 "citron_ensure.h"
        ;
    out =
// # 29 "citron_ensure.h" 3 4
        ((void *)0)
// # 29 "citron_ensure.h"
        ;
  }
  return out;
}
static inline ctr_object *ctr_can_cast_to_number(ctr_object *object) {
  ctr_object *out = ctr_internal_cast2number(object);
  if (CtrStdFlow) {
    CtrStdFlow =
// # 36 "citron_ensure.h" 3 4
        ((void *)0)
// # 36 "citron_ensure.h"
        ;
    out =
// # 37 "citron_ensure.h" 3 4
        ((void *)0)
// # 37 "citron_ensure.h"
        ;
  }
  return out;
}
static inline ctr_object *ctr_can_cast_to_bool(ctr_object *object) {
  ctr_object *out = ctr_internal_cast2bool(object);
  if (CtrStdFlow) {
    CtrStdFlow =
// # 44 "citron_ensure.h" 3 4
        ((void *)0)
// # 44 "citron_ensure.h"
        ;
    out =
// # 45 "citron_ensure.h" 3 4
        ((void *)0)
// # 45 "citron_ensure.h"
        ;
  }
  return out;
}
// # 1387 "citron.h" 2
// # 1 "citron_conv.h" 1
// # 1388 "citron.h" 2

ctr_string CTR_CLEX_KW_ME_SV, CTR_CLEX_KW_THIS_SV, CTR_CLEX_US_SV,
    CTR_CLEX_KW_RESPONDTO_SV;
ctr_object CTR_CLEX_KW_ME, CTR_CLEX_KW_THIS, CTR_CLEX_US, CTR_CLEX_KW_RESPONDTO;
int ctr_current_node_is_return;

static inline void ctr_linkstr();
void ctr_set_link_all(ctr_object *, ctr_object *);
void ctr_deallocate_argument_list(ctr_argument *);
int ctr_internal_object_is_equal(ctr_object *, ctr_object *);
int ctr_internal_object_is_constructible_(ctr_object *, ctr_object *, int);

static ctr_object *ctr_world_ptr;
// # 12 "collections.c" 2
// # 1 "siphash.h" 1

uint64_t siphash24(const void *src, unsigned long src_sz, const char key[16]);
// # 13 "collections.c" 2
// # 1 "symbol.h" 1

inline ctr_object *ctr_create_symbol(const char *, ctr_size);
ctr_object *ctr_symbol_as_string(ctr_object *symbol);
ctr_object *ctr_symbol_to_string(ctr_object *myself,
                                 ctr_argument *argumentList);
ctr_object *ctr_symbol_to_string_s(ctr_object *myself,
                                   ctr_argument *argumentList);
ctr_object *ctr_symbol_type(ctr_object *myself, ctr_argument *argumentList);
ctr_object *ctr_symbol_equals(ctr_object *myself, ctr_argument *argumentList);
ctr_object *ctr_symbol_ihash(ctr_object *myself, ctr_argument *argumentList);
ctr_object *ctr_symbol_unpack(ctr_object *myself, ctr_argument *argumentList);
// # 14 "collections.c" 2

// # 1 "khash.h" 1
// # 98 "khash.h"
// # 1 "/usr/lib/gcc/x86_64-pc-linux-gnu/8.2.1/include-fixed/limits.h" 1 3 4
// # 99 "khash.h" 2

typedef unsigned int khint32_t;

typedef unsigned long khint64_t;
// # 131 "khash.h"
typedef khint32_t khint_t;
typedef khint_t khiter_t;
// # 161 "khash.h"
static const double __ac_HASH_UPPER = 0.77;
// # 364 "khash.h"
static inline khint_t __ac_X31_hash_string(const char *s) {
  khint_t h = (khint_t)*s;
  if (h)
    for (++s; *s; ++s)
      h = (h << 5) - h + (khint_t)*s;
  return h;
}
// # 381 "khash.h"
static inline khint_t __ac_Wang_hash(khint_t key) {
  key += ~(key << 15);
  key ^= (key >> 10);
  key += (key << 3);
  key ^= (key >> 6);
  key += ~(key << 11);
  key ^= (key >> 16);
  return key;
}
// # 580 "khash.h"
typedef const char *kh_cstr_t;
// # 20 "collections.c" 2
// # 40 "collections.c"
ctr_object *ctr_array_new(ctr_object *myclass, ctr_argument *argumentList) {
  ctr_object *s = ctr_internal_create_object(7);
  ctr_set_link_all(s, myclass);
  int initial_length = 2;
  if (argumentList && argumentList->object) {
    int len = ctr_internal_cast2number(argumentList->object)->value.nvalue;
    if (len > 1)
      initial_length = len;
  }
  s->value.avalue = (ctr_collection *)ctr_heap_allocate(sizeof(ctr_collection));
  s->value.avalue->immutable = 0;
  s->value.avalue->length = initial_length;
  s->value.avalue->elements =
      (ctr_object **)ctr_heap_allocate(sizeof(ctr_object *) * initial_length);
  s->value.avalue->head = 0;
  s->value.avalue->tail = 0;
  return s;
}

ctr_object *ctr_array_copy(ctr_object *myself, ctr_argument *argumentList) {
  ctr_object *arr = ctr_array_new(CtrStdArray,
// # 69 "collections.c" 3 4
                                  ((void *)0)
// # 69 "collections.c"
  );
  ctr_argument parg = {0}, *pushArg = &parg;
  for (ctr_size i = myself->value.avalue->tail; i < myself->value.avalue->head;
       i++) {
    pushArg->object = *(myself->value.avalue->elements + i);
    ctr_array_push(arr, pushArg);
  }
  return arr;
}

ctr_object *ctr_array_type(ctr_object *myself, ctr_argument *argumentList) {
  return ctr_build_string_from_cstring("Array");
}
// # 101 "collections.c"
ctr_object *ctr_array_push(ctr_object *myself, ctr_argument *argumentList) {
  if (myself->value.avalue->immutable) {
    CtrStdFlow = ctr_build_string_from_cstring(
        "Cannot change immutable array's structure");
    return myself;
  }
  ctr_object *pushValue;
  if (myself->value.avalue->length <= (myself->value.avalue->head + 1)) {
    myself->value.avalue->length = myself->value.avalue->length * 2;
    myself->value.avalue->elements = (ctr_object **)ctr_heap_reallocate(
        myself->value.avalue->elements,
        (sizeof(ctr_object *) * (myself->value.avalue->length)));
  }
  pushValue = argumentList->object;
  *(myself->value.avalue->elements + myself->value.avalue->head) = pushValue;
  myself->value.avalue->head++;
  return myself;
}

ctr_object *ctr_array_push_imm(ctr_object *myself, ctr_argument *argumentList) {
  ctr_object *pushValue;
  if (myself->value.avalue->length <= (myself->value.avalue->head + 1)) {
    myself->value.avalue->length = myself->value.avalue->length * 2;
    myself->value.avalue->elements = (ctr_object **)ctr_heap_reallocate(
        myself->value.avalue->elements,
        (sizeof(ctr_object *) * (myself->value.avalue->length)));
  }
  pushValue = argumentList->object;
  *(myself->value.avalue->elements + myself->value.avalue->head) = pushValue;
  myself->value.avalue->head++;
  return myself;
}
// # 149 "collections.c"
ctr_object *ctr_array_min(ctr_object *myself, ctr_argument *argumentList) {
  double min = 0;
  double v = 0;
  ctr_object *el;
  ctr_size i = 0;
  for (i = myself->value.avalue->tail; i < myself->value.avalue->head; i++) {
    el = *(myself->value.avalue->elements + i);
    v = ctr_internal_cast2number(el)->value.nvalue;
    if (i == 0 || v < min) {
      min = v;
    }
  }
  return ctr_build_number_from_float(min);
}
// # 179 "collections.c"
ctr_object *ctr_array_all(ctr_object *myself, ctr_argument *argumentList) {
  ctr_object *el;
  ctr_size i = 0;
  ctr_object *fn;
  if ((fn = argumentList->object)) {
    ctr_open_context();
    for (i = myself->value.avalue->tail; i < myself->value.avalue->head; i++) {
      argumentList->object = *(myself->value.avalue->elements + i);
      el = ctr_block_run_here(fn, argumentList, fn);
      if ((el->info.type == 1 && el->value.bvalue) ||
          (ctr_internal_cast2bool(el)->value.bvalue))
        ;
      else {
        ctr_close_context();
        return ctr_build_bool(0);
      }
    }
    ctr_close_context();
    return ctr_build_bool(1);
  } else {
    for (i = myself->value.avalue->tail; i < myself->value.avalue->head; i++) {
      el = *(myself->value.avalue->elements + i);
      if ((el->info.type == 1 && el->value.bvalue) ||
          (ctr_internal_cast2bool(el)->value.bvalue))
        ;
      else
        return ctr_build_bool(0);
    }
    return ctr_build_bool(1);
  }
}
// # 226 "collections.c"
ctr_object *ctr_array_any(ctr_object *myself, ctr_argument *argumentList) {
  ctr_object *el;
  ctr_size i = 0;
  ctr_object *fn;
  if ((fn = argumentList->object)) {
    ctr_open_context();
    for (i = myself->value.avalue->tail; i < myself->value.avalue->head; i++) {
      argumentList->object = *(myself->value.avalue->elements + i);
      el = ctr_block_run_here(fn, argumentList, fn);
      if ((el->info.type == 1 && el->value.bvalue) ||
          (ctr_internal_cast2bool(el)->value.bvalue)) {
        ctr_close_context();
        return ctr_build_bool(1);
      }
    }
    ctr_close_context();
    return ctr_build_bool(0);
  } else {
    for (i = myself->value.avalue->tail; i < myself->value.avalue->head; i++) {
      el = *(myself->value.avalue->elements + i);
      if ((el->info.type == 1 && el->value.bvalue) ||
          (ctr_internal_cast2bool(el)->value.bvalue)) {
        return ctr_build_bool(1);
      }
    }
    return ctr_build_bool(0);
  }
}
// # 273 "collections.c"
ctr_object *ctr_array_max(ctr_object *myself, ctr_argument *argumentList) {
  double max = 0;
  double v = 0;
  ctr_object *el;
  ctr_size i = 0;
  for (i = myself->value.avalue->tail; i < myself->value.avalue->head; i++) {
    el = *(myself->value.avalue->elements + i);
    v = ctr_internal_cast2number(el)->value.nvalue;
    if (i == 0 || max < v) {
      max = v;
    }
  }
  return ctr_build_number_from_float(max);
}
// # 306 "collections.c"
ctr_object *ctr_array_sum(ctr_object *myself, ctr_argument *argumentList) {
  ctr_object *sum = CtrStdNil;
  ctr_object *el;
  ctr_size i = 0;
  for (i = myself->value.avalue->tail; i < myself->value.avalue->head; i++) {
    if (i == myself->value.avalue->tail) {
      sum = *myself->value.avalue->elements;
      continue;
    }
    el = *(myself->value.avalue->elements + i);
    sum = ctr_send_message_variadic(sum, "+", 1, 1, el);
  }
  return (sum);
}
// # 341 "collections.c"
ctr_object *ctr_array_product(ctr_object *myself, ctr_argument *argumentList) {
  ctr_object *prod = CtrStdNil;
  ctr_object *el;
  ctr_size i = 0;
  for (i = myself->value.avalue->tail; i < myself->value.avalue->head; i++) {
    if (i == myself->value.avalue->tail) {
      prod = *myself->value.avalue->elements;
      continue;
    }
    el = *(myself->value.avalue->elements + i);
    prod = ctr_send_message_variadic(prod, "*", 1, 1, el);
  }
  return (prod);
}

ctr_object *ctr_array_multiply(ctr_object *myself, ctr_argument *argumentList) {
  ctr_object *mand = argumentList->object;
  if (mand->info.type == 2) {
    int rep = (int)mand->value.nvalue;
    if (rep == 0)
      return ctr_array_new(CtrStdArray,
// # 374 "collections.c" 3 4
                           ((void *)0)
// # 374 "collections.c"
      );
    else if (rep == 1)
      return myself;
    else {
      rep--;
      ctr_object *newArr = ctr_array_new(CtrStdArray,
// # 380 "collections.c" 3 4
                                         ((void *)0)
// # 380 "collections.c"
      );
      ctr_argument varg = {0}, *arg = &varg;
      for (; rep >= 0; rep--) {
        arg->object = myself;
        newArr = ctr_array_add(newArr, arg);
      }
      return newArr;
    }
  }
  if (mand->info.type == 7) {
    ctr_object *newArr = ctr_array_new(CtrStdArray,
// # 392 "collections.c" 3 4
                                       ((void *)0)
// # 392 "collections.c"
    );
    ctr_argument arg = {0}, *args = &arg;
    for (ctr_size i = myself->value.avalue->tail;
         i < myself->value.avalue->head; i++) {
      ctr_object *elem0 = *(myself->value.avalue->elements + i);
      for (ctr_size j = mand->value.avalue->tail; j < mand->value.avalue->head;
           j++) {
        ctr_object *lst = ctr_array_new(CtrStdArray,
// # 399 "collections.c" 3 4
                                        ((void *)0)
// # 399 "collections.c"
        );
        args->object = elem0;
        ctr_array_push(lst, args);
        args->object = *(mand->value.avalue->elements + j);
        ctr_array_push(lst, args);
        lst->value.avalue->immutable = 1;
        args->object = lst;
        ctr_array_push(newArr, args);
      }
    }
    return newArr;
  }
}

ctr_object *ctr_array_intersperse(ctr_object *myself,
                                  ctr_argument *argumentList) {
  ctr_object *newArr = ctr_array_new(CtrStdArray,
// # 422 "collections.c" 3 4
                                     ((void *)0)
// # 422 "collections.c"
  );
  ctr_size i = 0;
  ctr_argument parg = {0}, *pushArg = &parg;
  for (i = myself->value.avalue->tail; i < myself->value.avalue->head; i++) {
    pushArg->object = *(myself->value.avalue->elements + i);
    newArr = ctr_array_push(newArr, pushArg);
    if (i < myself->value.avalue->head - 1) {
      pushArg->object = argumentList->object;
      newArr = ctr_array_push(newArr, pushArg);
    }
  }
  return newArr;
}
// # 455 "collections.c"
ctr_object *ctr_array_map(ctr_object *myself, ctr_argument *argumentList) {
  ctr_object *block = argumentList->object;
  ctr_size i = 0;
  if (block->info.type != 4) {
    CtrStdFlow = ctr_build_string_from_cstring("Expected Block.");
    CtrStdFlow->info.sticky = 1;
    return myself;
  }
  block->info.sticky = 1;
  ctr_argument a0 = {0}, *arguments = &a0;
  ctr_argument a1 = {0}, *argument2 = &a1;
  ctr_argument a2 = {0}, *argument3 =
// # 469 "collections.c" 3 4
                             ((void *)0)
// # 469 "collections.c"
      ;

  if (!myself->value.avalue->immutable)
    argument3 = &a2;

  for (i = myself->value.avalue->tail; i < myself->value.avalue->head; i++) {
    arguments->object = ctr_build_number_from_float((double)i);
    argument2->object = *(myself->value.avalue->elements + i);
    if (!myself->value.avalue->immutable)
      argument3->object = myself;
    arguments->next = argument2;
    if (!myself->value.avalue->immutable)
      argument2->next = argument3;
    ctr_block_run(block, arguments,
// # 483 "collections.c" 3 4
                  ((void *)0)
// # 483 "collections.c"
    );
    if (CtrStdFlow == CtrStdContinue)
      CtrStdFlow =
// # 485 "collections.c" 3 4
          ((void *)0)
// # 485 "collections.c"
          ;
    if (CtrStdFlow)
      break;
  }
  if (CtrStdFlow == CtrStdBreak)
    CtrStdFlow =
// # 490 "collections.c" 3 4
        ((void *)0)
// # 490 "collections.c"
        ;
  block->info.mark = 0;
  block->info.sticky = 0;
  return myself;
}
// # 522 "collections.c"
ctr_object *ctr_array_map_v(ctr_object *myself, ctr_argument *argumentList) {
  ctr_object *func = argumentList->object;
  do {
    do {
      if (func ==
// # 526 "collections.c" 3 4
          ((void *)0)
// # 526 "collections.c"
      ) {
        CtrStdFlow = ctr_build_string_from_cstring(
            "Object was null when we expected nonnull.");
        return CtrStdFlow;
      }
    } while (0);
    if (func->info.type != 4 && func->info.type != 6)
      do {
        char CTR_TYPE_ERR_STR[1000] =
            ("No direct conversion to type "
             "CodeBlock"
             ", nor any implicit cast to it arises from the use of type ");
        strcat(CTR_TYPE_ERR_STR, typeType[func->info.type].typestring);
        CtrStdFlow = ctr_build_string_from_cstring(CTR_TYPE_ERR_STR);
        return CtrStdNil;
      } while (0);
  } while (0);

  ctr_argument parg = {0}, *pushArg = &parg;
  ctr_argument earg = {0}, *elnumArg = &earg;
  ctr_size i;
  for (i = 0; i < myself->value.avalue->head - myself->value.avalue->tail;
       i++) {
    ctr_object *elnum = ctr_build_number_from_float((ctr_number)i);
    elnumArg->object = elnum;
    pushArg->object = ctr_array_get(myself, elnumArg);
    ctr_block_run(func, pushArg, func);
    if (CtrStdFlow == CtrStdContinue) {
      CtrStdFlow =
// # 539 "collections.c" 3 4
          ((void *)0)
// # 539 "collections.c"
          ;
      continue;
    }
    if (CtrStdFlow)
      break;
  }
  if (CtrStdFlow == CtrStdBreak)
    CtrStdFlow =
// # 546 "collections.c" 3 4
        ((void *)0)
// # 546 "collections.c"
        ;
  return myself;
}
// # 557 "collections.c"
ctr_object *ctr_array_slice(ctr_object *myself, ctr_argument *argumentList) {
  ctr_argument varg = {0}, *arg = &varg;
  ctr_object *elnum;
  ctr_object *startElement = ctr_internal_cast2number(argumentList->object);
  ctr_object *count = ctr_internal_cast2number(argumentList->next->object);
  ctr_size start = (ctr_size)startElement->value.nvalue;
  if (start >= myself->value.avalue->head - myself->value.avalue->tail)
    return ctr_array_new(CtrStdArray,
// # 566 "collections.c" 3 4
                         ((void *)0)
// # 566 "collections.c"
    );
  ctr_size len = (ctr_size)count->value.nvalue;
  len = fmin(myself->value.avalue->head - myself->value.avalue->tail - start,
             len);
  ctr_size i = 0;
  ctr_object *newArray = ctr_array_new(CtrStdArray,
// # 570 "collections.c" 3 4
                                       ((void *)0)
// # 570 "collections.c"
  );
  for (i = start; i < start + len; i++) {
    elnum = ctr_build_number_from_float((ctr_number)i);
    arg->object = elnum;
    arg->object = ctr_array_get(myself, arg);
    ctr_array_push(newArray, arg);
  }
  return newArray;
}
// # 589 "collections.c"
ctr_object *ctr_array_every_do_fill(ctr_object *myself,
                                    ctr_argument *argumentList) {
  ctr_size icount =
      ctr_internal_cast2number(argumentList->object)->value.nvalue;
  ctr_object *func = argumentList->next->object;
  do {
    do {
      if (func ==
// # 594 "collections.c" 3 4
          ((void *)0)
// # 594 "collections.c"
      ) {
        CtrStdFlow = ctr_build_string_from_cstring(
            "Object was null when we expected nonnull.");
        return CtrStdFlow;
      }
    } while (0);
    if (func->info.type != 4 && func->info.type != 6)
      do {
        char CTR_TYPE_ERR_STR[1000] =
            ("No direct conversion to type "
             "CodeBlock"
             ", nor any implicit cast to it arises from the use of type ");
        strcat(CTR_TYPE_ERR_STR, typeType[func->info.type].typestring);
        CtrStdFlow = ctr_build_string_from_cstring(CTR_TYPE_ERR_STR);
        return CtrStdNil;
      } while (0);
  } while (0);
  ctr_object *fill = ctr_build_nil();
  if (argumentList->next->next && argumentList->next->next->object)
    fill = argumentList->next->next->object;
  ctr_object *parr;
  ctr_argument varg = {0}, *args = &varg;
  ctr_argument vcaa = {0}, *caa = &vcaa;
  for (ctr_size
           s = ceil((myself->value.avalue->head - myself->value.avalue->tail) /
                    (double)icount),
           i = 0;
       i < s; i++) {
    args->object = ctr_build_number_from_float(i * icount);
    args->next = caa;
    args->next->object = ctr_build_number_from_float(icount);
    parr = ctr_array_slice(myself, args);
    while (parr->value.avalue->head - parr->value.avalue->tail < icount) {
      args->object = fill;
      ctr_array_push(parr, args);
    }
    args->object = parr;
    args->next =
// # 613 "collections.c" 3 4
        ((void *)0)
// # 613 "collections.c"
        ;
    ctr_block_run(func, args, parr);
  }
  return myself;
}
// # 626 "collections.c"
ctr_object *ctr_array_chunks(ctr_object *myself, ctr_argument *argumentList) {
  ctr_size icount =
      ctr_internal_cast2number(argumentList->object)->value.nvalue;
  ctr_object *fill = CtrStdNil;
  if (argumentList->next && argumentList->next->object)
    fill = argumentList->next->object;
  ctr_object *parr;
  ctr_object *ret = ctr_array_new(CtrStdArray,
// # 634 "collections.c" 3 4
                                  ((void *)0)
// # 634 "collections.c"
  );
  ctr_argument varg = {0}, *args = &varg;
  ctr_argument vcaa = {0}, *caa = &vcaa;
  for (ctr_size
           s = ceil((myself->value.avalue->head - myself->value.avalue->tail) /
                    (double)icount),
           i = 0;
       i < s; i++) {
    args->object = ctr_build_number_from_float(i * icount);
    args->next = caa;
    args->next->object = ctr_build_number_from_float(icount);
    parr = ctr_array_slice(myself, args);
    while (parr->value.avalue->head - parr->value.avalue->tail < icount) {
      args->object = fill;
      parr = ctr_array_push(parr, args);
    }
    args->object = parr;
    args->next =
// # 649 "collections.c" 3 4
        ((void *)0)
// # 649 "collections.c"
        ;
    ctr_array_push(ret, args);
  }
  return ret;
}
// # 672 "collections.c"
ctr_object *ctr_array_new_and_push(ctr_object *myclass,
                                   ctr_argument *argumentList) {
  ctr_object *s = ctr_array_new(myclass,
// # 675 "collections.c" 3 4
                                ((void *)0)
// # 675 "collections.c"
  );
  return ctr_array_push(s, argumentList);
}
// # 691 "collections.c"
ctr_object *ctr_array_unshift(ctr_object *myself, ctr_argument *argumentList) {
  if (myself->value.avalue->immutable) {
    CtrStdFlow = ctr_build_string_from_cstring(
        "Cannot change immutable array's structure");
    return myself;
  }
  ctr_object *pushValue = argumentList->object;
  if (myself->value.avalue->tail > 0) {
    myself->value.avalue->tail--;
  } else {
    if (myself->value.avalue->length <= (myself->value.avalue->head + 1)) {
      myself->value.avalue->length = myself->value.avalue->length * 3;
      myself->value.avalue->elements = (ctr_object **)ctr_heap_reallocate(
          myself->value.avalue->elements,
          (sizeof(ctr_object *) * (myself->value.avalue->length)));
    }
    myself->value.avalue->head++;
    memmove(myself->value.avalue->elements + 1, myself->value.avalue->elements,
            myself->value.avalue->head * sizeof(ctr_object *));
  }
  *(myself->value.avalue->elements + myself->value.avalue->tail) = pushValue;
  return myself;
}

ctr_object *ctr_array_reverse(ctr_object *myself, ctr_argument *argumentList) {
  ctr_object *newArr = ctr_array_new(CtrStdArray,
// # 725 "collections.c" 3 4
                                     ((void *)0)
// # 725 "collections.c"
  );
  ctr_size i = ctr_array_count(myself,
// # 726 "collections.c" 3 4
                               ((void *)0)
// # 726 "collections.c"
                                   )
                   ->value.nvalue;
  ctr_argument varg = {0}, *args = &varg;
  for (; i > 0; i--) {
    args->object = ctr_build_number_from_float(i - 1);
    args->object = ctr_array_get(myself, args);
    ctr_array_push(newArr, args);
  }
  return newArr;
}
// # 749 "collections.c"
ctr_object *ctr_array_join(ctr_object *myself, ctr_argument *argumentList) {
  ctr_size i;
  char *result;
  ctr_size len = 0;
  ctr_size pos;
  ctr_object *o;
  ctr_object *str;
  ctr_object *resultStr;
  ctr_object *glue = ctr_internal_cast2string(argumentList->object);
  ctr_size glen = glue->value.svalue->vlen;
  for (i = myself->value.avalue->tail; i < myself->value.avalue->head; i++) {
    o = *(myself->value.avalue->elements + i);
    str = ctr_internal_cast2string(o);
    pos = len;
    if (len == 0) {
      len = str->value.svalue->vlen;
      result = ctr_heap_allocate(sizeof(char) * len);
    } else {
      len += str->value.svalue->vlen + glen;
      result = ctr_heap_reallocate(result, sizeof(char) * len);
      memcpy(result + pos, glue->value.svalue->value, glen);
      pos += glen;
    }
    memcpy(result + pos, str->value.svalue->value, str->value.svalue->vlen);
  }
  resultStr = ctr_build_string(result, len);
  if (len > 0)
    ctr_heap_free(result);
  return resultStr;
}
// # 797 "collections.c"
ctr_object *ctr_array_get(ctr_object *myself, ctr_argument *argumentList) {
  ctr_object *getIndex = argumentList->object;
  long long i;
  if (getIndex->info.type != 2) {

    char buf[1024];
    char *typename_ = ctr_heap_allocate_cstring(
             ctr_internal_cast2string(ctr_send_message(getIndex, "type", 4,
// # 809 "collections.c" 3 4
                                                       ((void *)0)
// # 809 "collections.c"
                                                           ))),
         *value = ctr_heap_allocate_cstring(ctr_internal_cast2string(getIndex));
    sprintf(buf, "Array index must be a number (not %s type %d(%s)).", value,
            getIndex->info.type, typename_);
    CtrStdFlow = ctr_build_string_from_cstring(buf);
    ctr_heap_free(typename_);
    ctr_heap_free(value);
    return CtrStdNil;
  }
  i = (int)getIndex->value.nvalue;
  if (i < 0)
    i += myself->value.avalue->head - myself->value.avalue->tail;
  if (myself->value.avalue->head <= (i + myself->value.avalue->tail) || i < 0) {
    CtrStdFlow = ctr_build_string_from_cstring("Index out of bounds.");
    CtrStdFlow->info.sticky = 1;
    return CtrStdNil;
  }
  return *(myself->value.avalue->elements + myself->value.avalue->tail + i);
}

ctr_object *ctr_array_index(ctr_object *myself, ctr_argument *argumentList) {
  ctr_size i;
  ctr_object *o;
  ctr_object *needle = argumentList->object;
  for (i = myself->value.avalue->tail; i < myself->value.avalue->head; i++) {
    o = *(myself->value.avalue->elements + i);
    if (ctr_internal_object_is_equal(o, needle))
      return ctr_build_number_from_float(i);
  }
  return ctr_build_number_from_float(-1);
}

ctr_object *ctr_array_contains(ctr_object *myself, ctr_argument *argumentList) {
  ctr_size i;
  ctr_object *o;
  ctr_object *needle = argumentList->object;
  for (i = myself->value.avalue->tail; i < myself->value.avalue->head; i++) {
    o = *(myself->value.avalue->elements + i);
    if (ctr_internal_object_is_equal(o, needle))
      return ctr_build_bool(1);
  }
  return ctr_build_bool(0);
}
// # 887 "collections.c"
ctr_object *ctr_array_put(ctr_object *myself, ctr_argument *argumentList) {
  if (myself->value.avalue->immutable) {
    CtrStdFlow = ctr_build_string_from_cstring(
        "Cannot change immutable array's structure");
    return myself;
  }
  ctr_object *putValue = argumentList->object;
  ctr_object *putIndex = ctr_internal_cast2number(argumentList->next->object);
  ctr_size putIndexNumber;
  ctr_size head;
  ctr_size tail;
  ctr_argument arg = {0}, *argument = &arg;
  if (putIndex->value.nvalue < 0) {
    CtrStdFlow = ctr_build_string_from_cstring("Index out of bounds.");
    CtrStdFlow->info.sticky = 1;
    return myself;
  }

  head = (ctr_size)myself->value.avalue->head;
  tail = (ctr_size)myself->value.avalue->tail;
  putIndexNumber = (ctr_size)putIndex->value.nvalue;
  if (head <= putIndexNumber) {
    if (myself->value.avalue->immutable) {
      CtrStdFlow = ctr_build_string_from_cstring(
          "Cannot change immutable array's structure");
      return myself;
    }
    ctr_size j;
    for (j = head; j <= putIndexNumber; j++) {
      argument->object = CtrStdNil;
      ctr_array_push(myself, argument);
    }
    myself->value.avalue->head = putIndexNumber + 1;
  }
  if (putIndexNumber < tail) {
    ctr_size j;
    for (j = tail; j > putIndexNumber; j--) {
      *(myself->value.avalue->elements + j) = CtrStdNil;
    }
    myself->value.avalue->tail = putIndexNumber;
  }
  *(myself->value.avalue->elements + putIndexNumber) = putValue;
  return myself;
}

ctr_object *ctr_array_pop(ctr_object *myself, ctr_argument *argumentList) {
  if (myself->value.avalue->immutable) {
    CtrStdFlow = ctr_build_string_from_cstring(
        "Cannot change immutable array's structure");
    return myself;
  }
  if (myself->value.avalue->tail >= myself->value.avalue->head) {
    return CtrStdNil;
  }
  if (argumentList->object && argumentList->object->info.type == 2) {
    ctr_object *val = *(myself->value.avalue->elements +
                        (ctr_size)(argumentList->object->value.nvalue));
    myself->value.avalue->head--;
    for (int i = (ctr_size)(argumentList->object->value.nvalue);
         i < myself->value.avalue->head - myself->value.avalue->tail; i++)
      myself->value.avalue->elements[i] = myself->value.avalue->elements[i + 1];
    return val;
  } else {
    myself->value.avalue->head--;
    return *(myself->value.avalue->elements + myself->value.avalue->head);
  }
}

ctr_object *ctr_array_shift(ctr_object *myself, ctr_argument *argumentList) {
  if (myself->value.avalue->immutable) {
    CtrStdFlow = ctr_build_string_from_cstring(
        "Cannot change immutable array's structure");
    return myself;
  }
  ctr_object *shiftedOff;
  if (myself->value.avalue->tail >= myself->value.avalue->head) {
    return CtrStdNil;
  }
  shiftedOff = *(myself->value.avalue->elements + myself->value.avalue->tail);
  myself->value.avalue->tail++;
  return shiftedOff;
}

ctr_object *ctr_array_count(ctr_object *myself, ctr_argument *argumentList) {
  ctr_number d = 0;
  d = (ctr_number)myself->value.avalue->head - myself->value.avalue->tail;
  return ctr_build_number_from_float((ctr_number)d);
}

ctr_object *ctr_array_empty(ctr_object *myself, ctr_argument *argumentList) {
  int d = myself->value.avalue->head - myself->value.avalue->tail;
  return ctr_build_bool(d == 0);
}

ctr_object *ctr_array_from_length(ctr_object *myself,
                                  ctr_argument *argumentList) {
  ctr_object *elnum;
  ctr_object *startElement = ctr_internal_cast2number(argumentList->object);
  ctr_object *count = ctr_internal_cast2number(argumentList->next->object);
  ctr_size start = (int)startElement->value.nvalue;
  ctr_size len = (int)count->value.nvalue;
  ctr_size i = 0;
  ctr_object *newArray = ctr_array_new(CtrStdArray,
// # 1035 "collections.c" 3 4
                                       ((void *)0)
// # 1035 "collections.c"
  );
  ctr_argument parg = {0}, *pushArg = &parg;
  ctr_argument earg = {0}, *elnumArg = &earg;
  for (i = start; i < start + len; i++) {
    elnum = ctr_build_number_from_float((ctr_number)i);
    elnumArg->object = elnum;
    pushArg->object = ctr_array_get(myself, elnumArg);
    ctr_array_push(newArray, pushArg);
  }
  return newArray;
}

ctr_object *ctr_array_skip(ctr_object *myself, ctr_argument *argumentList) {
  ctr_object *elnum;
  ctr_object *startElement = ctr_internal_cast2number(argumentList->object);
  ctr_size start = (int)startElement->value.nvalue;
  ctr_size len = (int)myself->value.avalue->length - 1;
  ctr_size i = 0;
  ctr_object *newArray = ctr_array_new(CtrStdArray,
// # 1061 "collections.c" 3 4
                                       ((void *)0)
// # 1061 "collections.c"
  );
  ctr_argument parg = {0}, *pushArg = &parg;
  ctr_argument earg = {0}, *elnumArg = &earg;
  for (i = start; i < start + len; i++) {
    elnum = ctr_build_number_from_float((ctr_number)i);
    elnumArg->object = elnum;
    pushArg->object = ctr_array_get(myself, elnumArg);
    ctr_array_push(newArray, pushArg);
  }
  return newArray;
}
// # 1083 "collections.c"
ctr_object *ctr_array_zip(ctr_object *myself, ctr_argument *argumentList) {
  ctr_object *res = ctr_array_new(CtrStdArray,
// # 1086 "collections.c" 3 4
                                  ((void *)0)
// # 1086 "collections.c"
  );
  for (int i = 0;; i++) {
    ctr_object *tarr = ctr_array_new(CtrStdArray,
// # 1088 "collections.c" 3 4
                                     ((void *)0)
// # 1088 "collections.c"
    );
    for (int x = myself->value.avalue->tail; x < myself->value.avalue->head;
         x++) {
      ctr_object *element = myself->value.avalue->elements[x];
      if (element->info.type != 7)
        ctr_array_push(tarr, &(ctr_argument){element,

// # 1096 "collections.c" 3 4
                                             ((void *)0)

// # 1097 "collections.c"
                             });
      else if (element->value.avalue->head - element->value.avalue->tail > i)
        ctr_array_push(
            tarr,
            &(ctr_argument){element->value.avalue
                                ->elements[i + element->value.avalue->tail],

// # 1104 "collections.c" 3 4
                            ((void *)0)

// # 1105 "collections.c"
            });
      else
        goto thatsenough;
    }
    ctr_array_push(res, &(ctr_argument){tarr});
  }
thatsenough:;
  return res;
}
// # 1124 "collections.c"
ctr_object *ctr_array_zip_with(ctr_object *myself, ctr_argument *argumentList) {
  ctr_object *blk = argumentList->object;
  ctr_object *res = ctr_array_new(CtrStdArray,
// # 1128 "collections.c" 3 4
                                  ((void *)0)
// # 1128 "collections.c"
  );
  for (int i = 0;; i++) {
    ctr_object *tarr = ctr_array_new(CtrStdArray,
// # 1130 "collections.c" 3 4
                                     ((void *)0)
// # 1130 "collections.c"
    );
    for (int x = myself->value.avalue->tail; x < myself->value.avalue->head;
         x++) {
      ctr_object *element = myself->value.avalue->elements[x];
      if (element->info.type != 7)
        ctr_array_push(tarr, &(ctr_argument){element,

// # 1138 "collections.c" 3 4
                                             ((void *)0)

// # 1139 "collections.c"
                             });
      else if (element->value.avalue->head - element->value.avalue->tail > i)
        ctr_array_push(
            tarr,
            &(ctr_argument){element->value.avalue
                                ->elements[i + element->value.avalue->tail],

// # 1146 "collections.c" 3 4
                            ((void *)0)

// # 1147 "collections.c"
            });
      else
        goto thatsenough;
    }
    ctr_array_push(
        res, &(ctr_argument){ctr_block_runall(blk, &(ctr_argument){tarr})});
  }
thatsenough:;
  return res;
}
// # 1164 "collections.c"
ctr_object *ctr_array_head(ctr_object *myself, ctr_argument *argumentList) {
  return ctr_send_message_variadic(myself, "at:", 3, 1,
                                   ctr_build_number_from_float(0));
}
// # 1177 "collections.c"
ctr_object *ctr_array_tail(ctr_object *myself, ctr_argument *argumentList) {
  ctr_object *arr = ctr_array_new(CtrStdArray,
// # 1180 "collections.c" 3 4
                                  ((void *)0)
// # 1180 "collections.c"
  );
  if (myself->value.avalue->head == myself->value.avalue->tail)
    return arr;
  else {
    ctr_argument varg = {0}, *arg = &varg;
    for (ctr_size i = 1;
         i < myself->value.avalue->head - myself->value.avalue->tail; i++) {
      arg->object = ctr_build_number_from_float(i);
      arg->object = ctr_send_message(myself, "at:", 3, arg);
      ctr_send_message(arr, "push:", 5, arg);
    }
  }
  arr->value.avalue->immutable = myself->value.avalue->immutable;
  return arr;
}
// # 1204 "collections.c"
ctr_object *ctr_array_init(ctr_object *myself, ctr_argument *argumentList) {
  ctr_object *arr = ctr_array_new(CtrStdArray,
// # 1207 "collections.c" 3 4
                                  ((void *)0)
// # 1207 "collections.c"
  );
  if (myself->value.avalue->head == myself->value.avalue->tail)
    return arr;
  else {
    ctr_argument varg = {0}, *arg = &varg;
    for (ctr_size i = 0;
         i < myself->value.avalue->head - myself->value.avalue->tail - 1; i++) {
      arg->object = ctr_build_number_from_float(i);
      arg->object = ctr_send_message(myself, "at:", 3, arg);
      ctr_send_message(arr, "push:", 5, arg);
    }
  }
  arr->value.avalue->immutable = myself->value.avalue->immutable;
  return arr;
}
// # 1231 "collections.c"
ctr_object *ctr_array_last(ctr_object *myself, ctr_argument *argumentList) {
  return ctr_send_message_variadic(
      myself, "at:", 3, 1,
      ctr_build_number_from_float(myself->value.avalue->head -
                                  myself->value.avalue->tail - 1));
}

ctr_object *ctr_array_add(ctr_object *myself, ctr_argument *argumentList) {
  ctr_object *otherArray = argumentList->object;
  ctr_object *newArray = ctr_array_new(CtrStdArray,
// # 1247 "collections.c" 3 4
                                       ((void *)0)
// # 1247 "collections.c"
  );
  ctr_argument parg = {0}, *pushArg = &parg;
  ctr_argument earg = {0}, *elnumArg = &earg;
  ctr_size i;
  for (i = 0; i < myself->value.avalue->head - myself->value.avalue->tail;
       i++) {
    ctr_object *elnum = ctr_build_number_from_float((ctr_number)i);
    elnumArg->object = elnum;
    pushArg->object = ctr_array_get(myself, elnumArg);
    ctr_array_push(newArray, pushArg);
  }
  if (otherArray->info.type == 7) {
    for (i = otherArray->value.avalue->tail; i < otherArray->value.avalue->head;
         i++) {
      ctr_object *elnum = ctr_build_number_from_float((ctr_number)i);
      elnumArg->object = elnum;
      pushArg->object = ctr_array_get(otherArray, elnumArg);
      ctr_array_push(newArray, pushArg);
    }
  }
  return newArray;
}
// # 1278 "collections.c"
ctr_object *ctr_array_fmap(ctr_object *myself, ctr_argument *argumentList) {
  ctr_object *func = argumentList->object;
  do {
    do {
      if (func ==
// # 1282 "collections.c" 3 4
          ((void *)0)
// # 1282 "collections.c"
      ) {
        CtrStdFlow = ctr_build_string_from_cstring(
            "Object was null when we expected nonnull.");
        return CtrStdFlow;
      }
    } while (0);
    if (func->info.type != 4 && func->info.type != 6)
      do {
        char CTR_TYPE_ERR_STR[1000] =
            ("No direct conversion to type "
             "CodeBlock"
             ", nor any implicit cast to it arises from the use of type ");
        strcat(CTR_TYPE_ERR_STR, typeType[func->info.type].typestring);
        CtrStdFlow = ctr_build_string_from_cstring(CTR_TYPE_ERR_STR);
        return CtrStdNil;
      } while (0);
  } while (0);

  ctr_object *newArray = ctr_array_new(CtrStdArray,
// # 1284 "collections.c" 3 4
                                       ((void *)0)
// # 1284 "collections.c"
  );
  ctr_argument varg = {0}, *arg = &varg;
  ctr_size i;
  ctr_object **elems = myself->value.avalue->elements;
  for (i = 0; i < myself->value.avalue->head - myself->value.avalue->tail;
       i++) {
    arg->object = elems[i];
    arg->object = ctr_block_run(func, arg, func);
    if (CtrStdFlow) {
      if (CtrStdFlow == CtrStdContinue) {
        CtrStdFlow =
// # 1296 "collections.c" 3 4
            ((void *)0)
// # 1296 "collections.c"
            ;
        continue;
      }
      if (CtrStdFlow == CtrStdBreak)
        CtrStdFlow =
// # 1300 "collections.c" 3 4
            ((void *)0)
// # 1300 "collections.c"
            ;
      break;
    }
    ctr_array_push(newArray, arg);
  }
  return newArray;
}
// # 1315 "collections.c"
ctr_object *ctr_array_imap(ctr_object *myself, ctr_argument *argumentList) {
  ctr_object *func = argumentList->object;
  do {
    do {
      if (func ==
// # 1319 "collections.c" 3 4
          ((void *)0)
// # 1319 "collections.c"
      ) {
        CtrStdFlow = ctr_build_string_from_cstring(
            "Object was null when we expected nonnull.");
        return CtrStdFlow;
      }
    } while (0);
    if (func->info.type != 4 && func->info.type != 6)
      do {
        char CTR_TYPE_ERR_STR[1000] =
            ("No direct conversion to type "
             "CodeBlock"
             ", nor any implicit cast to it arises from the use of type ");
        strcat(CTR_TYPE_ERR_STR, typeType[func->info.type].typestring);
        CtrStdFlow = ctr_build_string_from_cstring(CTR_TYPE_ERR_STR);
        return CtrStdNil;
      } while (0);
  } while (0);

  ctr_object *newArray = ctr_array_new(CtrStdArray,
// # 1321 "collections.c" 3 4
                                       ((void *)0)
// # 1321 "collections.c"
  );
  ctr_argument parg = {0}, *pushArg = &parg, pnext = {0};
  pushArg->next = &pnext;
  ctr_size i;
  ctr_object **elements = myself->value.avalue->elements;
  for (i = 0; i < myself->value.avalue->head - myself->value.avalue->tail;
       i++) {
    ctr_object *elnum = ctr_build_number_from_float((ctr_number)i);
    pushArg->next->object = elements[i];
    pushArg->object = elnum;
    pushArg->object = ctr_block_run(func, pushArg, func);
    ctr_array_push(newArray, pushArg);
  }
  return newArray;
}
// # 1345 "collections.c"
ctr_object *ctr_array_fmap_inp(ctr_object *myself, ctr_argument *argumentList) {
  ctr_object *func = argumentList->object;
  do {
    do {
      if (func ==
// # 1349 "collections.c" 3 4
          ((void *)0)
// # 1349 "collections.c"
      ) {
        CtrStdFlow = ctr_build_string_from_cstring(
            "Object was null when we expected nonnull.");
        return CtrStdFlow;
      }
    } while (0);
    if (func->info.type != 4 && func->info.type != 6)
      do {
        char CTR_TYPE_ERR_STR[1000] =
            ("No direct conversion to type "
             "CodeBlock"
             ", nor any implicit cast to it arises from the use of type ");
        strcat(CTR_TYPE_ERR_STR, typeType[func->info.type].typestring);
        CtrStdFlow = ctr_build_string_from_cstring(CTR_TYPE_ERR_STR);
        return CtrStdNil;
      } while (0);
  } while (0);

  ctr_argument varg = {0}, *arg = &varg;
  ctr_size i;
  ctr_object **elements = myself->value.avalue->elements;
  for (i = 0; i < myself->value.avalue->head - myself->value.avalue->tail;
       i++) {
    arg->object = elements[i];
    ctr_object *value = ctr_block_run(func, arg, func);
    if (CtrStdFlow) {
      if (CtrStdFlow == CtrStdContinue) {
        CtrStdFlow =
// # 1362 "collections.c" 3 4
            ((void *)0)
// # 1362 "collections.c"
            ;
        continue;
      }
      if (CtrStdFlow == CtrStdBreak)
        CtrStdFlow =
// # 1366 "collections.c" 3 4
            ((void *)0)
// # 1366 "collections.c"
            ;
      break;
    }
    elements[i + myself->value.avalue->tail] = value;
  }
  return myself;
}
// # 1382 "collections.c"
ctr_object *ctr_array_imap_inp(ctr_object *myself, ctr_argument *argumentList) {
  ctr_object *func = argumentList->object;
  do {
    do {
      if (func ==
// # 1386 "collections.c" 3 4
          ((void *)0)
// # 1386 "collections.c"
      ) {
        CtrStdFlow = ctr_build_string_from_cstring(
            "Object was null when we expected nonnull.");
        return CtrStdFlow;
      }
    } while (0);
    if (func->info.type != 4 && func->info.type != 6)
      do {
        char CTR_TYPE_ERR_STR[1000] =
            ("No direct conversion to type "
             "CodeBlock"
             ", nor any implicit cast to it arises from the use of type ");
        strcat(CTR_TYPE_ERR_STR, typeType[func->info.type].typestring);
        CtrStdFlow = ctr_build_string_from_cstring(CTR_TYPE_ERR_STR);
        return CtrStdNil;
      } while (0);
  } while (0);

  ctr_argument parg = {0}, *pushArg = &parg, pargnext = {0}, earg,
               *elnumArg = &earg;
  pushArg->next = &pargnext;
  ctr_size i;
  ctr_object **elements = myself->value.avalue->elements;
  for (i = 0; i < myself->value.avalue->head - myself->value.avalue->tail;
       i++) {
    ctr_object *elnum = ctr_build_number_from_float((ctr_number)i);
    elnumArg->object = elnum;
    pushArg->next->object = ctr_array_get(myself, elnumArg);
    pushArg->object = elnumArg->object;
    ctr_object *value = ctr_block_run(func, pushArg, func);
    elements[i + myself->value.avalue->tail] = value;
  }
  return myself;
}
// # 1411 "collections.c"
ctr_object *ctr_array_foldl(ctr_object *myself, ctr_argument *argumentList) {
  ctr_object *func = argumentList->object;
  do {
    do {
      if (func ==
// # 1415 "collections.c" 3 4
          ((void *)0)
// # 1415 "collections.c"
      ) {
        CtrStdFlow = ctr_build_string_from_cstring(
            "Object was null when we expected nonnull.");
        return CtrStdFlow;
      }
    } while (0);
    if (func->info.type != 4 && func->info.type != 6)
      do {
        char CTR_TYPE_ERR_STR[1000] =
            ("No direct conversion to type "
             "CodeBlock"
             ", nor any implicit cast to it arises from the use of type ");
        strcat(CTR_TYPE_ERR_STR, typeType[func->info.type].typestring);
        CtrStdFlow = ctr_build_string_from_cstring(CTR_TYPE_ERR_STR);
        return CtrStdNil;
      } while (0);
  } while (0);

  ctr_object *accumulator = argumentList->next->object;
  ctr_argument earg = {0}, *elnumArg = &earg;
  ctr_argument aarg = {0}, *accArg = &aarg, anext = {0};
  accArg->next = &anext;
  accArg->object = accumulator;
  ctr_size i;
  for (i = 0; i < myself->value.avalue->head - myself->value.avalue->tail;
       i++) {
    ctr_object *elnum = ctr_build_number_from_float((ctr_number)i);
    elnumArg->object = elnum;
    accArg->next->object = ctr_array_get(myself, elnumArg);
    accArg->object = ctr_block_run(func, accArg, func);
  }
  accumulator = accArg->object;
  return accumulator;
}
// # 1442 "collections.c"
ctr_object *ctr_array_foldl0(ctr_object *myself, ctr_argument *argumentList) {
  ctr_object *func = argumentList->object;
  do {
    do {
      if (func ==
// # 1446 "collections.c" 3 4
          ((void *)0)
// # 1446 "collections.c"
      ) {
        CtrStdFlow = ctr_build_string_from_cstring(
            "Object was null when we expected nonnull.");
        return CtrStdFlow;
      }
    } while (0);
    if (func->info.type != 4 && func->info.type != 6)
      do {
        char CTR_TYPE_ERR_STR[1000] =
            ("No direct conversion to type "
             "CodeBlock"
             ", nor any implicit cast to it arises from the use of type ");
        strcat(CTR_TYPE_ERR_STR, typeType[func->info.type].typestring);
        CtrStdFlow = ctr_build_string_from_cstring(CTR_TYPE_ERR_STR);
        return CtrStdNil;
      } while (0);
  } while (0);

  ctr_size i = 0;
  ctr_argument earg = {0}, *elnumArg = &earg;
  ctr_object *elnum = ctr_build_number_from_float((ctr_number)i);
  elnumArg->object = elnum;
  ctr_object *accumulator = ctr_array_get(myself, elnumArg);
  if (CtrStdFlow)
    return CtrStdNil;
  ctr_argument aarg = {0}, *accArg = &aarg, anext = {0};
  accArg->next = &anext;
  accArg->object = accumulator;
  for (i = 1; i < myself->value.avalue->head - myself->value.avalue->tail;
       i++) {
    elnumArg->object->value.nvalue = i;
    accArg->next->object = ctr_array_get(myself, elnumArg);
    accArg->object = ctr_block_run(func, accArg, func);
  }
  accumulator = accArg->object;
  return accumulator;
}

ctr_object *ctr_array_filter(ctr_object *myself, ctr_argument *argumentList) {
  ctr_object *block = argumentList->object;
  do {
    do {
      if (block ==
// # 1477 "collections.c" 3 4
          ((void *)0)
// # 1477 "collections.c"
      ) {
        CtrStdFlow = ctr_build_string_from_cstring(
            "Object was null when we expected nonnull.");
        return CtrStdFlow;
      }
    } while (0);
    if (block->info.type != 4 && block->info.type != 6)
      do {
        char CTR_TYPE_ERR_STR[1000] =
            ("No direct conversion to type "
             "CodeBlock"
             ", nor any implicit cast to it arises from the use of type ");
        strcat(CTR_TYPE_ERR_STR, typeType[block->info.type].typestring);
        CtrStdFlow = ctr_build_string_from_cstring(CTR_TYPE_ERR_STR);
        return CtrStdNil;
      } while (0);
  } while (0);
  ctr_size i = 0;
  block->info.sticky = 1;
  ctr_argument args = {0}, *arguments = &args;
  ctr_argument arg2 = {0}, *argument2 = &arg2;
  arguments->next = argument2;
  ctr_object *newArr = ctr_array_new(CtrStdArray,
// # 1483 "collections.c" 3 4
                                     ((void *)0)
// # 1483 "collections.c"
  );
  ctr_object *current;
  for (i = myself->value.avalue->tail; i < myself->value.avalue->head; i++) {
    current = *(myself->value.avalue->elements + i);
    arguments->object = ctr_build_number_from_float(i);
    argument2->object = current;
    ctr_object *fv = ctr_block_run(block, arguments, block);
    if (ctr_internal_cast2bool(fv)->value.bvalue) {
      argument2->object = current;
      ctr_array_push(newArr, argument2);
    }
    if (CtrStdFlow == CtrStdContinue)
      CtrStdFlow =
// # 1497 "collections.c" 3 4
          ((void *)0)
// # 1497 "collections.c"
          ;
    if (CtrStdFlow)
      break;
  }
  if (CtrStdFlow == CtrStdBreak)
    CtrStdFlow =
// # 1502 "collections.c" 3 4
        ((void *)0)
// # 1502 "collections.c"
        ;
  block->info.mark = 0;
  block->info.sticky = 0;
  return newArr;
}

ctr_object *ctr_array_filter_v(ctr_object *myself, ctr_argument *argumentList) {
  ctr_object *block = argumentList->object;
  do {
    do {
      if (block ==
// # 1517 "collections.c" 3 4
          ((void *)0)
// # 1517 "collections.c"
      ) {
        CtrStdFlow = ctr_build_string_from_cstring(
            "Object was null when we expected nonnull.");
        return CtrStdFlow;
      }
    } while (0);
    if (block->info.type != 4 && block->info.type != 6)
      do {
        char CTR_TYPE_ERR_STR[1000] =
            ("No direct conversion to type "
             "CodeBlock"
             ", nor any implicit cast to it arises from the use of type ");
        strcat(CTR_TYPE_ERR_STR, typeType[block->info.type].typestring);
        CtrStdFlow = ctr_build_string_from_cstring(CTR_TYPE_ERR_STR);
        return CtrStdNil;
      } while (0);
  } while (0);
  ctr_size i = 0;
  block->info.sticky = 1;
  ctr_argument args = {0}, *arguments = &args;
  ctr_object *newArr = ctr_array_new(CtrStdArray,
// # 1521 "collections.c" 3 4
                                     ((void *)0)
// # 1521 "collections.c"
  );
  ctr_object *current;
  for (i = myself->value.avalue->tail; i < myself->value.avalue->head; i++) {
    current = *(myself->value.avalue->elements + i);
    arguments->object = current;
    ctr_object *fv = ctr_block_run(block, arguments, block);
    if (ctr_internal_cast2bool(fv)->value.bvalue) {
      arguments->object = current;
      ctr_array_push(newArr, arguments);
    }
    if (CtrStdFlow == CtrStdContinue)
      CtrStdFlow =
// # 1534 "collections.c" 3 4
          ((void *)0)
// # 1534 "collections.c"
          ;
    if (CtrStdFlow)
      break;
  }
  if (CtrStdFlow == CtrStdBreak)
    CtrStdFlow =
// # 1539 "collections.c" 3 4
        ((void *)0)
// # 1539 "collections.c"
        ;
  block->info.mark = 0;
  block->info.sticky = 0;
  return newArr;
}

ctr_argument *ctr_array_to_argument_list(ctr_object *arr,
                                         ctr_argument *provided) {
  if (!arr)
    return
// # 1555 "collections.c" 3 4
        ((void *)0)
// # 1555 "collections.c"
            ;
  ctr_size i = arr->value.avalue->tail,
           arr_max_len = arr->value.avalue->head - arr->value.avalue->tail;
  if (arr_max_len == 0)
    return provided;
  if (!provided)
    provided = ctr_heap_allocate(sizeof(ctr_argument));
  ctr_argument *arg = provided;
  while (provided) {
    if (!provided->next)
      break;
    if (i >= arr_max_len)
      provided->object =
// # 1567 "collections.c" 3 4
          ((void *)0)
// # 1567 "collections.c"
          ;
    else
      provided->object = arr->value.avalue->elements[i];
    provided = provided->next;
    i++;
  }
  while (i < arr_max_len) {
    provided->object = arr->value.avalue->elements[i++];
    if (__builtin_expect(!!(i == arr_max_len), 1)) {
      provided->next =
// # 1578 "collections.c" 3 4
          ((void *)0)
// # 1578 "collections.c"
          ;
      break;
    }
    provided->next = ctr_heap_allocate(sizeof(ctr_argument));
    provided = provided->next;
  }
  return arg;
}

void ctr_deallocate_argument_list(ctr_argument *argumentList) {
  ctr_argument *arg;
  while (argumentList) {
    arg = argumentList->next;
    ctr_heap_free(argumentList);
    argumentList = arg;
  }
}

void ctr_nullify_argument_list(ctr_argument *argumentList) {
  ctr_argument *arg;
  while (argumentList) {
    arg = argumentList->next;
    argumentList->object =
// # 1606 "collections.c" 3 4
        ((void *)0)
// # 1606 "collections.c"
        ;
    argumentList = arg;
  }
}

ctr_object *ctr_array_select_from_if(ctr_object *myself,
                                     ctr_argument *argumentList) {
  ctr_object *selector = argumentList->object,
             *from = argumentList->next->object,
             *ifexp =
                 argumentList->next->next && argumentList->next->next->object
                     ? argumentList->next->next->object
                     :
// # 1622 "collections.c" 3 4
                     ((void *)0)
// # 1622 "collections.c"
                 ,
             *result = ctr_array_new(CtrStdArray,
// # 1622 "collections.c" 3 4
                                     ((void *)0)
// # 1622 "collections.c"
                                         ),
             *elem, *old_from;
  ctr_argument *argument = ctr_heap_allocate(sizeof(ctr_argument));
  if (from->info.type == 4)
    from = ctr_block_run(from,
// # 1625 "collections.c" 3 4
                         ((void *)0)
// # 1625 "collections.c"
                             ,
// # 1625 "collections.c" 3 4
                         ((void *)0)
// # 1625 "collections.c"
    );
  if (from->info.type == 7) {
    printf("%p\n", from);
    for (ctr_size i = from->value.avalue->tail; i < from->value.avalue->head;
         i++) {
      elem = from->value.avalue->elements[i];
      if (elem->info.type == 7)
        argument = ctr_array_to_argument_list(elem, argument);
      else {
        ctr_nullify_argument_list(argument);
        argument->object = elem;
      }
      if (ifexp) {
        ctr_object *clause =
            ctr_internal_cast2bool(ctr_block_run(ifexp, argument, ifexp));
        if (!clause->value.bvalue)
          continue;
      }
      ctr_object *maybe = ctr_block_run(selector, argument, selector);
      argument->object = maybe;
      ctr_array_push(result, argument);
    }
    ctr_deallocate_argument_list(argument);
    return result;
  } else if (ctr_internal_has_responder(
                 from, ctr_build_string_from_cstring("items"))) {
    from = ctr_send_message(from, "items", 5,
// # 1655 "collections.c" 3 4
                            ((void *)0)
// # 1655 "collections.c"
    );
    if (from->info.type != 7) {
      ctr_deallocate_argument_list(argument);
      CtrStdFlow = ctr_build_string_from_cstring(
          "Generator 'items' does not return a valid value.");
      return CtrStdNil;
    }
    argumentList->next->object = from;
    return ctr_array_select_from_if(myself, argumentList);
  }
}
// # 1678 "collections.c"
ctr_object *ctr_array_assign(ctr_object *myself, ctr_argument *argumentList) {
  ctr_object *to = argumentList->object;
  if (!ctr_reflect_check_bind_valid(myself, to, 0)) {
    CtrStdFlow = ctr_build_string_from_cstring("Invalid bind");
    return CtrStdNil;
  }

  ctr_argument earg = {0}, *elnumArg = &earg;
  ctr_argument aarg = {0}, *accArg = &aarg;
  accArg->next = argumentList->next;
  ctr_size i;
  if (to->info.type == 7) {
    int other;
    int saw_catch_all = 0;
    int m_count = myself->value.avalue->head - myself->value.avalue->tail;
    for (other = (i = to->value.avalue->tail); i < to->value.avalue->head;
         other++, i++) {
      ctr_object *elnum = ctr_build_number_from_float((ctr_number)i);
      elnumArg->object = elnum;
      ctr_object *to_elem;
      accArg->object = ctr_array_get(to, elnumArg);
      if (accArg->object->interfaces->link == CtrStdSymbol &&
          accArg->object->value.svalue->vlen == 1 &&
          *accArg->object->value.svalue->value == '_')
        continue;
      if (accArg->object->interfaces->link == CtrStdSymbol &&
          *accArg->object->value.svalue->value == '*') {

        if (saw_catch_all) {
          ctr_object *cc = ctr_build_string_from_cstring("Pattern (");
          ctr_send_message_variadic(cc, "append:", 7, 1, to);
          ctr_send_message_variadic(cc, "append:", 7, 1,
                                    ctr_build_string_from_cstring(
                                        ") cannot contain two starred names."));
          CtrStdFlow = cc;
          goto clear;
        }
        saw_catch_all = 1;
        accArg->object = ctr_get_or_create_symbol_table_entry(
            accArg->object->value.svalue->value + 1,
            accArg->object->value.svalue->vlen - 1);
        int g_skip = m_count - other - 1;
        to_elem = ctr_array_new(CtrStdArray,
// # 1720 "collections.c" 3 4
                                ((void *)0)
// # 1720 "collections.c"
        );
        if (other - to->value.avalue->tail <=
            myself->value.avalue->head - myself->value.avalue->tail)
          for (int _i =
                   other - to->value.avalue->tail + myself->value.avalue->tail;
               _i < g_skip + other - to->value.avalue->tail +
                        myself->value.avalue->tail;
               _i++) {
            elnumArg->object = myself->value.avalue->elements[_i];
            ctr_array_push(to_elem, elnumArg);
          }
        to_elem->value.avalue->immutable = myself->value.avalue->immutable;

      } else {
        elnumArg->object = ctr_build_number_from_float(other);
        to_elem = ctr_array_get(myself, elnumArg);
      }
      ctr_send_message(to_elem, "unpack:", 7, accArg);
    }
  } else if (to->interfaces->link == CtrStdSymbol) {
    if (argumentList->object->value.svalue->vlen == 0 ||
        (argumentList->object->value.svalue->vlen == 1 &&
         *argumentList->object->value.svalue->value == '_'))
      goto clear;
    ctr_internal_object_add_property(argumentList->next->object
                                         ?: ctr_contexts[ctr_context_id],
                                     ctr_symbol_as_string(to), myself, 0);
  }
clear:
  return myself;
}

ctr_object *temp_sorter;
int ctr_sort_cmp(const void *a, const void *b) {
  ctr_argument earg = {0}, *arg1 = &earg;
  ctr_argument aarg = {0}, *arg2 = &aarg;
  ctr_object *result;
  ctr_object *numResult;
  arg1->next = arg2;
  arg1->object = *((ctr_object **)a);
  arg2->object = *((ctr_object **)b);
  result = ctr_block_run(temp_sorter, arg1,
// # 1767 "collections.c" 3 4
                         ((void *)0)
// # 1767 "collections.c"
  );
  numResult = ctr_internal_cast2number(result);
  return (int)numResult->value.nvalue;
}

ctr_object *ctr_array_sort(ctr_object *myself, ctr_argument *argumentList) {
  ctr_object *sorter = argumentList->object;
  if (sorter->info.type != 4) {
    CtrStdFlow = ctr_build_string_from_cstring("Expected block.");
    CtrStdFlow->info.sticky = 1;
    return myself;
  }
  temp_sorter = sorter;
  qsort((myself->value.avalue->elements + myself->value.avalue->tail),
        myself->value.avalue->head - myself->value.avalue->tail,
        sizeof(ctr_object *), ctr_sort_cmp);
  return myself;
}
// # 1813 "collections.c"
ctr_object *ctr_array_to_string(ctr_object *myself,
                                ctr_argument *argumentList) {
  ctr_size i;
  ctr_object *arrayElement;
  ctr_argument narg = {0}, *newArgumentList;
  ctr_object *string = ctr_build_empty_string();
  newArgumentList = &narg;
  if (myself->value.avalue->tail == myself->value.avalue->head) {
    newArgumentList->object = ctr_build_string_from_cstring(
        myself->value.avalue->immutable ? "[" : "Array new ");
    string = ctr_string_append(string, newArgumentList);
  } else {
    newArgumentList->object = ctr_build_string_from_cstring(
        myself->value.avalue->immutable ? "[" : "Array ← ");
    string = ctr_string_append(string, newArgumentList);
  }
  for (i = myself->value.avalue->tail; i < myself->value.avalue->head; i++) {
    arrayElement = *(myself->value.avalue->elements + i);
    if (!arrayElement) {
      newArgumentList->object = ctr_build_string_from_cstring("':NULLPtr:'");
      string = ctr_string_append(string, newArgumentList);
    } else if (arrayElement == myself) {
      newArgumentList->object =
          ctr_build_string_from_cstring("':selfReference:'");
      string = ctr_string_append(string, newArgumentList);
    } else if (arrayElement->info.type == 1 || arrayElement->info.type == 2 ||
               arrayElement->info.type == 0) {
      newArgumentList->object = arrayElement;
      string = ctr_string_append(string, newArgumentList);
    } else if (arrayElement->info.type == 3) {
      newArgumentList->object = ctr_build_string_from_cstring("'");
      string = ctr_string_append(string, newArgumentList);
      newArgumentList->object =
          ctr_string_quotes_escape(arrayElement, newArgumentList);
      string = ctr_string_append(string, newArgumentList);
      newArgumentList->object = ctr_build_string_from_cstring("'");
      string = ctr_string_append(string, newArgumentList);
    } else if (arrayElement->info.type == 7 &&
               arrayElement->value.avalue->immutable) {
      newArgumentList->object = arrayElement;
      string = ctr_string_append(string, newArgumentList);
    } else {
      newArgumentList->object = ctr_build_string_from_cstring("(");
      ctr_string_append(string, newArgumentList);
      newArgumentList->object = arrayElement;
      string = ctr_string_append(string, newArgumentList);
      newArgumentList->object = ctr_build_string_from_cstring(")");
      ctr_string_append(string, newArgumentList);
    }
    if ((i + 1) < myself->value.avalue->head) {
      newArgumentList->object = ctr_build_string_from_cstring(
          myself->value.avalue->immutable ? ", " : " ; ");
      string = ctr_string_append(string, newArgumentList);
    }
  }
  if (myself->value.avalue->immutable) {
    newArgumentList->object = ctr_build_string_from_cstring("]");
    string = ctr_string_append(string, newArgumentList);
  }
  return string;
}

ctr_object *ctr_array_fill(ctr_object *myself, ctr_argument *argumentList) {
  ctr_size n;
  ctr_size i;
  ctr_argument narg = {0}, *newArgumentList = &narg;
  n = ctr_internal_cast2number(argumentList->object)->value.nvalue;
  ctr_collection *coll = myself->value.avalue;
  ctr_size empty = (coll->head - coll->tail);
  if (empty < n) {
    coll->elements = ctr_heap_reallocate(
        coll->elements, (coll->length + n - empty) * sizeof(ctr_object *));
    coll->length = n;
  }
  ctr_object *memb = argumentList->next->object;
  ctr_object *res =
// # 1902 "collections.c" 3 4
      ((void *)0)
// # 1902 "collections.c"
      ;
  if (memb->info.type == 4 || memb->info.type == 6) {
    ctr_object *_i = ctr_internal_create_standalone_object(2);
    ctr_set_link_all(_i, CtrStdNumber);
    ctr_open_context();
    for (i = 0; i < n; i++) {
      _i->value.nvalue = i;
      newArgumentList->object = _i;
      res = ctr_block_run_here(memb, newArgumentList, memb);
      coll->elements[coll->head++] = res;
    }
    ctr_close_context();
  } else {
    for (i = 0; i < n; i++) {
      coll->elements[coll->head++] = memb;
    }
  }
  return myself;
}

ctr_object *ctr_array_column(ctr_object *myself, ctr_argument *argumentList) {
  ctr_size i;
  ctr_size n;
  ctr_argument narg = {0}, *newArgumentList = &narg;
  ctr_object *newArray;
  ctr_object *element;
  newArray = ctr_array_new(CtrStdArray,
// # 1935 "collections.c" 3 4
                           ((void *)0)
// # 1935 "collections.c"
  );
  n = ctr_internal_cast2number(argumentList->object)->value.nvalue;
  if (n <= 0) {
    return newArray;
  }
  for (i = myself->value.avalue->tail; i < myself->value.avalue->head; i++) {
    element = *(myself->value.avalue->elements + i);
    if (element->info.type != 7)
      continue;
    if (n >= element->value.avalue->head)
      continue;
    newArgumentList->object =
        *(element->value.avalue->elements + element->value.avalue->tail + n);
    ctr_array_push(newArray, newArgumentList);
  }
  return newArray;
}
// # 1974 "collections.c"
ctr_object *ctr_map_new(ctr_object *myclass, ctr_argument *argumentList) {
  ctr_object *s = ctr_internal_create_object(5);
  ctr_set_link_all(s, myclass);
  s->value.defvalue = CtrStdNil;
  return s;
}

ctr_object *ctr_map_new_(ctr_object *myclass, ctr_argument *argumentList) {
  ctr_object *s = ctr_internal_create_object(5);
  ctr_set_link_all(s, myclass);
  ctr_object *defaultv = ctr_build_nil();
  if (argumentList->object) {
    switch (argumentList->object->info.type) {
    case 4:
      defaultv = argumentList->object;
      break;
    default:
      CtrStdFlow =
          ctr_format_str("EMap::'new:' expects a block, not %s",
                         ctr_send_message(argumentList->object, "type", 4,
// # 1997 "collections.c" 3 4
                                          ((void *)0)
// # 1997 "collections.c"
                                              ));
    }
  }
  s->value.defvalue = defaultv;
  return s;
}

ctr_object *ctr_map_type(ctr_object *myself, ctr_argument *argumentList) {
  return ctr_build_string_from_cstring("Map");
}
// # 2026 "collections.c"
ctr_object *ctr_map_put(ctr_object *myself, ctr_argument *argumentList) {
  char *key;
  long keyLen;
  ctr_object *putValue = argumentList->object;
  ctr_argument *nextArgument = argumentList->next;
  ctr_object *hasher =
      ctr_get_responder(argumentList->next->object, "iHash", 5);
  if (!hasher) {
    ctr_object *putKey;
    putKey = ctr_send_message(nextArgument->object, "toString", 8,
// # 2037 "collections.c" 3 4
                              ((void *)0)
// # 2037 "collections.c"
    );

    if (putKey->info.type != 3) {
      putKey = ctr_internal_cast2string(putKey);
    }

    key = ctr_heap_allocate(putKey->value.svalue->vlen * sizeof(char));
    keyLen = putKey->value.svalue->vlen;
    memcpy(key, putKey->value.svalue->value, keyLen);
    ctr_internal_object_delete_property(myself, ctr_build_string(key, keyLen),
                                        0);
    ctr_internal_object_add_property(myself, ctr_build_string(key, keyLen),
                                     putValue, 0);
    return myself;
  } else {
    ctr_object *putKeyHash;
    putKeyHash = ctr_send_message(nextArgument->object, "iHash", 5,
// # 2055 "collections.c" 3 4
                                  ((void *)0)
// # 2055 "collections.c"
    );

    ctr_number hashk;
    if (putKeyHash->info.type != 2)
      hashk = ctr_internal_index_hash(
          ctr_internal_cast2string(nextArgument->object));
    else
      hashk = putKeyHash->value.nvalue;
    ctr_internal_object_delete_property_with_hash(myself, nextArgument->object,
                                                  *(uint64_t *)(&hashk), 0);
    ctr_internal_object_add_property_with_hash(
        myself, nextArgument->object, *(uint64_t *)(&hashk), putValue, 0);
    return myself;
  }
}
// # 2079 "collections.c"
ctr_object *ctr_map_rm(ctr_object *myself, ctr_argument *argumentList) {
  char *key;
  ctr_object *putKey = argumentList->object;
  ctr_argument earg = {0}, *emptyArgumentList = &earg;
  emptyArgumentList->next =
// # 2085 "collections.c" 3 4
      ((void *)0)
// # 2085 "collections.c"
      ;
  emptyArgumentList->object =
// # 2086 "collections.c" 3 4
      ((void *)0)
// # 2086 "collections.c"
      ;
  ctr_object *hasher = ctr_get_responder(putKey, "iHash", 5);
  if (!hasher) {
    putKey = ctr_send_message(putKey, "toString", 8, emptyArgumentList);

    if (putKey->info.type != 3) {
      putKey = ctr_internal_cast2string(putKey);
    }

    key = ctr_heap_allocate_cstring(putKey);
    ctr_internal_object_delete_property(myself,
                                        ctr_build_string_from_cstring(key), 0);
    ctr_heap_free(key);
    return myself;
  } else {
    ctr_number hashk;
    ctr_object *putKeyHash = ctr_send_message(putKey, "iHash", 5,
// # 2106 "collections.c" 3 4
                                              ((void *)0)
// # 2106 "collections.c"
    );

    if (putKeyHash->info.type != 2)
      hashk = ctr_internal_index_hash(putKey);
    else
      hashk = putKeyHash->value.nvalue;
    ctr_internal_object_delete_property_with_hash(myself, putKey,
                                                  *(uint64_t *)&hashk, 0);
    return myself;
  }
}
// # 2129 "collections.c"
ctr_object *ctr_map_get(ctr_object *myself, ctr_argument *argumentList) {
  ctr_object *searchKey;
  ctr_object *foundObject;

  searchKey = argumentList->object;
  ctr_object *hasher = ctr_get_responder(searchKey, "iHash", 5);

  if (!hasher) {
    searchKey = ctr_send_message(searchKey, "toString", 8,
// # 2140 "collections.c" 3 4
                                 ((void *)0)
// # 2140 "collections.c"
    );

    if (searchKey->info.type != 3) {
      searchKey = ctr_internal_cast2string(searchKey);
    }

    foundObject = ctr_internal_object_find_property(myself, searchKey, 0);
    goto retv;
  } else {
    ctr_number hashk;
    ctr_object *searchKeyHasho = ctr_send_message(searchKey, "iHash", 5,
// # 2154 "collections.c" 3 4
                                                  ((void *)0)
// # 2154 "collections.c"
    );
    if (searchKeyHasho->info.type != 2) {
      foundObject = ctr_internal_object_find_property(myself, searchKey, 0);
    } else {
      hashk = searchKeyHasho->value.nvalue;
      foundObject = ctr_internal_object_find_property_with_hash(
          myself, searchKey, *(uint64_t *)&hashk, 0);
    }
  retv:
    if (foundObject ==
// # 2165 "collections.c" 3 4
        ((void *)0)
// # 2165 "collections.c"
    ) {
      ctr_object *kvres = myself->value.defvalue;
      if (kvres == CtrStdNil)
        foundObject = kvres;
      else
        foundObject = ctr_block_run(kvres, argumentList, myself);
    }
    return foundObject;
  }
}
// # 2186 "collections.c"
ctr_object *ctr_map_get_or_insert(ctr_object *myself,
                                  ctr_argument *argumentList) {
  ctr_object *searchKey;
  ctr_object *foundObject;

  searchKey = argumentList->object;
  ctr_object *hasher = ctr_get_responder(searchKey, "iHash", 5);

  if (!hasher) {
    searchKey = ctr_send_message(searchKey, "toString", 8,
// # 2197 "collections.c" 3 4
                                 ((void *)0)
// # 2197 "collections.c"
    );

    if (searchKey->info.type != 3) {
      searchKey = ctr_internal_cast2string(searchKey);
    }

    foundObject =
        ctr_internal_object_find_property_or_create(myself, searchKey, 0);
    goto retv;
  } else {
    ctr_number hashk;
    ctr_object *searchKeyHasho = ctr_send_message(searchKey, "iHash", 5,
// # 2211 "collections.c" 3 4
                                                  ((void *)0)
// # 2211 "collections.c"
    );
    if (searchKeyHasho->info.type != 2) {
      foundObject = ctr_internal_object_find_property(myself, searchKey, 0);
    } else {
      hashk = searchKeyHasho->value.nvalue;
      foundObject = ctr_internal_object_find_property_or_create_with_hash(
          myself, searchKey, *(uint64_t *)&hashk, 0);
    }
  retv:
    if (foundObject ==
// # 2222 "collections.c" 3 4
        ((void *)0)
// # 2222 "collections.c"
    ) {
      ctr_object *kvres = myself->value.defvalue;
      if (kvres == CtrStdNil)
        foundObject = kvres;
      else
        foundObject = ctr_block_run(kvres, argumentList, myself);
    }
    return foundObject;
  }
}

ctr_object *ctr_map_count(ctr_object *myself, ctr_argument *argumentList) {
  return ctr_build_number_from_float(myself->properties->size);
}

ctr_object *ctr_map_empty(ctr_object *myself, ctr_argument *argumentList) {
  return ctr_build_bool(myself->properties->size == 0);
}

ctr_object *ctr_map_each(ctr_object *myself, ctr_argument *argumentList) {
  ctr_object *block = argumentList->object;
  ctr_mapitem *m;
  if (block->info.type != 4) {
    CtrStdFlow = ctr_build_string_from_cstring("Expected Block.");
    CtrStdFlow->info.sticky = 1;
  }
  block->info.sticky = 1;
  m = myself->properties->head;
  ctr_argument args = {0}, *arguments = &args;
  ctr_argument arg2 = {0}, *argument2 = &arg2;
  ctr_argument arg3 = {0}, *argument3 = &arg3;
  while (m && !CtrStdFlow) {
    arguments->object = m->key;
    argument2->object = m->value;
    argument3->object = myself;
    arguments->next = argument2;
    argument2->next = argument3;
    ctr_block_run(block, arguments,
// # 2284 "collections.c" 3 4
                  ((void *)0)
// # 2284 "collections.c"
    );
    if (CtrStdFlow == CtrStdContinue)
      CtrStdFlow =
// # 2286 "collections.c" 3 4
          ((void *)0)
// # 2286 "collections.c"
          ;
    m = m->next;
  }
  if (CtrStdFlow == CtrStdBreak)
    CtrStdFlow =
// # 2290 "collections.c" 3 4
        ((void *)0)
// # 2290 "collections.c"
        ;
  block->info.mark = 0;
  block->info.sticky = 0;
  return myself;
}

void ctr_internal_object_set_property_with_hash(ctr_object *owner,
                                                ctr_object *key,
                                                uint64_t hashKey,
                                                ctr_object *value,
                                                int is_method);

ctr_object *ctr_map_merge(ctr_object *myself, ctr_argument *argumentList) {
  if (!argumentList)
    goto error_invalid_input;
  ctr_object *smap = argumentList->object;
  if (!smap)
    goto error_invalid_input;
  if (ctr_reflect_get_primitive_link(smap) != CtrStdMap)
    goto error_invalid_input;
  ctr_mapitem *s = smap->properties->head;
  ctr_size sl = smap->properties->size;
  for (; sl--;) {
    ctr_internal_object_set_property_with_hash(myself, s->key, s->hashKey,
                                               s->value, 0);
    s = s->next;
  }
  return myself;
error_invalid_input:
  CtrStdFlow =
      ctr_build_string_from_cstring("Expected an argument of type Map");
  return CtrStdNil;
}

ctr_object *ctr_map_keys(ctr_object *myself, ctr_argument *argumentList) {
  ctr_object *keys = ctr_array_new(CtrStdArray,
// # 2325 "collections.c" 3 4
                                   ((void *)0)
// # 2325 "collections.c"
  );
  ctr_mapitem *s = myself->properties->head;
  ctr_size sl = myself->properties->size;
  ctr_argument arg = {
// # 2328 "collections.c" 3 4
      ((void *)0)
// # 2328 "collections.c"
          ,
// # 2328 "collections.c" 3 4
      ((void *)0)
// # 2328 "collections.c"
  };
  for (; sl--;) {
    arg.object = s->key;
    ctr_array_push(keys, &arg);
    s = s->next;
  }
  return keys;
}

ctr_object *ctr_map_values(ctr_object *myself, ctr_argument *argumentList) {
  ctr_object *values = ctr_array_new(CtrStdArray,
// # 2341 "collections.c" 3 4
                                     ((void *)0)
// # 2341 "collections.c"
  );
  ctr_mapitem *s = myself->properties->head;
  ctr_size sl = myself->properties->size;
  ctr_argument arg = {
// # 2344 "collections.c" 3 4
      ((void *)0)
// # 2344 "collections.c"
          ,
// # 2344 "collections.c" 3 4
      ((void *)0)
// # 2344 "collections.c"
  };
  for (; sl--;) {
    arg.object = s->value;
    ctr_array_push(values, &arg);
    s = s->next;
  }
  return values;
}
// # 2361 "collections.c"
ctr_object *ctr_map_fmap(ctr_object *myself, ctr_argument *argumentList) {
  ctr_object *block = argumentList->object;
  ctr_mapitem *m;
  if (block->info.type != 4) {
    CtrStdFlow = ctr_build_string_from_cstring("Expected Block.");
    CtrStdFlow->info.sticky = 1;
    return myself;
  }
  block->info.sticky = 1;
  m = myself->properties->head;
  ctr_argument arg = {0}, *arguments = &arg, varg = {0};
  arguments->next = &varg;
  ctr_object *newmap = ctr_map_new(CtrStdMap,
// # 2376 "collections.c" 3 4
                                   ((void *)0)
// # 2376 "collections.c"
  );
  while (m) {
    arguments->object = m->key;
    arguments->next->object = m->value;
    arguments->object = ctr_block_run(block, arguments, myself);
    if (CtrStdFlow) {
      if (CtrStdFlow == CtrStdContinue) {
        CtrStdFlow =
// # 2386 "collections.c" 3 4
            ((void *)0)
// # 2386 "collections.c"
            ;
        m = m->next;
        continue;
      }
      if (CtrStdFlow == CtrStdBreak)
        CtrStdFlow =
// # 2391 "collections.c" 3 4
            ((void *)0)
// # 2391 "collections.c"
            ;
      break;
    }
    if (arguments->object == block) {
      arguments->object = m->key;
      arguments->next->object = m->value;
    } else
      arguments->next->object = m->key;
    ctr_map_put(newmap, arguments);
    m = m->next;
  }
  block->info.mark = 0;
  block->info.sticky = 0;
  return newmap;
}
// # 2417 "collections.c"
ctr_object *ctr_map_fmap_inp(ctr_object *myself, ctr_argument *argumentList) {
  ctr_object *block = argumentList->object;
  ctr_mapitem *m;
  if (block->info.type != 4) {
    CtrStdFlow = ctr_build_string_from_cstring("Expected a Block.");
    CtrStdFlow->info.sticky = 1;
    return myself;
  }
  block->info.sticky = 1;
  m = myself->properties->head;
  ctr_argument arg = {0}, *arguments = &arg, varg = {0};
  arguments->next = &varg;
  ctr_object *value;
  while (m) {
    arguments->object = m->key;
    arguments->next->object = m->value;
    value = ctr_block_run(block, arguments, myself);
    if (CtrStdFlow) {
      if (CtrStdFlow == CtrStdContinue) {
        CtrStdFlow =
// # 2442 "collections.c" 3 4
            ((void *)0)
// # 2442 "collections.c"
            ;
        m = m->next;
        continue;
      }
      if (CtrStdFlow == CtrStdBreak)
        CtrStdFlow =
// # 2447 "collections.c" 3 4
            ((void *)0)
// # 2447 "collections.c"
            ;
      break;
    }
    if (value == block) {
      value = m->value;
    }
    m->value = value;
    m = m->next;
  }
  block->info.mark = 0;
  block->info.sticky = 0;
  return myself;
}
// # 2469 "collections.c"
ctr_object *ctr_map_kvmap(ctr_object *myself, ctr_argument *argumentList) {
  ctr_object *block = argumentList->object;
  ctr_mapitem *m;
  if (block->info.type != 4) {
    CtrStdFlow = ctr_build_string_from_cstring("Expected Block.");
    CtrStdFlow->info.sticky = 1;
    return myself;
  }
  block->info.sticky = 1;
  m = myself->properties->head;
  ctr_argument args = {0}, vargs = {0}, *arguments = &args;
  arguments->next = &vargs;
  ctr_argument largs = {0}, *larguments = &largs;
  ctr_object *kvtup = ctr_array_new(CtrStdArray,
// # 2485 "collections.c" 3 4
                                    ((void *)0)
// # 2485 "collections.c"
  );
  ctr_object *newmap = ctr_map_new(CtrStdMap,
// # 2486 "collections.c" 3 4
                                   ((void *)0)
// # 2486 "collections.c"
  );
  while (m) {
    arguments->object = m->key;
    arguments->next->object = ctr_build_number_from_float(0);
    ctr_array_put(kvtup, arguments);

    arguments->object = m->value;
    arguments->next->object = ctr_build_number_from_float(1);
    ctr_array_put(kvtup, arguments);

    arguments->object = kvtup;
    arguments->object = ctr_block_run(block, arguments, myself);
    if (arguments->object->info.type != 7) {
      CtrStdFlow = ctr_build_string_from_cstring(
          "kvmap block must only return a 2-tuple or nothing.");
      return CtrStdNil;
    }
    if (arguments->object == block) {
      arguments->object = m->key;
      arguments->next->object = m->value;
    } else {
      ctr_object *ret = arguments->object;
      larguments->object = ctr_build_number_from_float(1);
      arguments->object = ctr_array_get(ret, larguments);
      larguments->object = ctr_build_number_from_float(0);
      arguments->next->object = ctr_array_get(ret, larguments);
    }
    ctr_map_put(newmap, arguments);
    m = m->next;
  }
  kvtup->info.mark = 1;
  block->info.mark = 0;
  block->info.sticky = 0;
  return newmap;
}
// # 2533 "collections.c"
ctr_object *ctr_map_kvlist(ctr_object *myself, ctr_argument *argumentList) {
  ctr_object *block = argumentList->object;
  ctr_mapitem *m;
  if (block->info.type != 4) {
    CtrStdFlow = ctr_build_string_from_cstring("Expected Block.");
    CtrStdFlow->info.sticky = 1;
    return myself;
  }
  block->info.sticky = 1;
  m = myself->properties->head;
  ctr_argument args = {0}, vargs = {0}, *arguments = &args;
  arguments->next = &vargs;
  ctr_argument largs = {0}, *larguments = &largs;
  ctr_object *kvtup = ctr_array_new(CtrStdArray,
// # 2549 "collections.c" 3 4
                                    ((void *)0)
// # 2549 "collections.c"
  );
  ctr_object *list = ctr_array_new(CtrStdArray,
// # 2550 "collections.c" 3 4
                                   ((void *)0)
// # 2550 "collections.c"
  );
  while (m) {
    arguments->object = m->key;
    arguments->next->object = ctr_build_number_from_float(0);
    ctr_array_put(kvtup, arguments);

    arguments->object = m->value;
    arguments->next->object = ctr_build_number_from_float(1);
    ctr_array_put(kvtup, arguments);

    arguments->object = kvtup;
    arguments->object = ctr_block_run(block, arguments, myself);

    if (arguments->object == block) {
      ctr_object *lst = ctr_array_new(CtrStdArray,
// # 2570 "collections.c" 3 4
                                      ((void *)0)
// # 2570 "collections.c"
      );
      arguments->object = m->key;
      ctr_array_push(lst, arguments);
      arguments->object = m->value;
      ctr_array_push(lst, arguments);
      arguments->object = lst;
    }
    ctr_array_push(list, arguments);
    m = m->next;
  }
  kvtup->info.mark = 1;
  block->info.mark = 0;
  block->info.sticky = 0;
  return list;
}

ctr_object *ctr_map_flip(ctr_object *myself, ctr_argument *argumentList) {
  ctr_mapitem *m;
  m = myself->properties->head;
  ctr_object *map_new = ctr_map_new(CtrStdMap,
// # 2596 "collections.c" 3 4
                                    ((void *)0)
// # 2596 "collections.c"
  );
  ctr_argument args = {0}, *arguments = &args;
  ctr_argument arg2 = {0}, *argument2 = &arg2;
  while (m) {
    arguments->object = m->key;
    argument2->object = m->value;
    arguments->next = argument2;
    ctr_map_put(map_new, arguments);
    m = m->next;
  }
  return map_new;
}
// # 2643 "collections.c"
ctr_object *ctr_map_to_string(ctr_object *myself, ctr_argument *argumentList) {
  ctr_object *string;
  ctr_mapitem *mapItem;
  ctr_argument nargs = {0}, *newArgumentList = &nargs;
  string = ctr_build_string_from_cstring("(Map new) ");
  mapItem = myself->properties->head;
  while (mapItem) {
    if (!mapItem->value || !mapItem->key)
      goto next;
    newArgumentList->object = ctr_build_string_from_cstring("put:");
    ctr_string_append(string, newArgumentList);
    if (mapItem->value == myself) {
      newArgumentList->object =
          ctr_build_string_from_cstring("':selfReference:'");
      ctr_string_append(string, newArgumentList);
    } else if (mapItem->value->info.type == 1 ||
               mapItem->value->info.type == 2 ||
               mapItem->value->info.type == 0) {
      newArgumentList->object = mapItem->value;
      ctr_string_append(string, newArgumentList);
    } else if (mapItem->value->info.type == 3) {
      newArgumentList->object = ctr_build_string_from_cstring("'");
      ctr_string_append(string, newArgumentList);
      newArgumentList->object =
          ctr_string_quotes_escape(mapItem->value, newArgumentList);
      ctr_string_append(string, newArgumentList);
      newArgumentList->object = ctr_build_string_from_cstring("'");
      ctr_string_append(string, newArgumentList);
    } else {
      newArgumentList->object = ctr_build_string_from_cstring("(");
      ctr_string_append(string, newArgumentList);
      newArgumentList->object = mapItem->value;
      ctr_string_append(string, newArgumentList);
      newArgumentList->object = ctr_build_string_from_cstring(")");
      ctr_string_append(string, newArgumentList);
    }
    newArgumentList->object = ctr_build_string_from_cstring(" at:");
    ctr_string_append(string, newArgumentList);
    if (mapItem->key->info.type == 1 || mapItem->key->info.type == 2 ||
        mapItem->value->info.type == 0) {
      newArgumentList->object = mapItem->key;
      ctr_string_append(string, newArgumentList);
    } else if (mapItem->key->info.type == 3) {
      newArgumentList->object = ctr_build_string_from_cstring("'");
      ctr_string_append(string, newArgumentList);
      newArgumentList->object =
          ctr_string_quotes_escape(mapItem->key, newArgumentList);
      ctr_string_append(string, newArgumentList);
      newArgumentList->object = ctr_build_string_from_cstring("'");
      ctr_string_append(string, newArgumentList);
    } else {
      newArgumentList->object = ctr_build_string_from_cstring("(");
      ctr_string_append(string, newArgumentList);
      newArgumentList->object = mapItem->key;
      ctr_string_append(string, newArgumentList);
      newArgumentList->object = ctr_build_string_from_cstring(")");
      ctr_string_append(string, newArgumentList);
    }
  next:
    mapItem = mapItem->next;
    if (mapItem) {
      newArgumentList->object = ctr_build_string_from_cstring(", ");
      ctr_string_append(string, newArgumentList);
    }
  }
  return string;
}
// # 2733 "collections.c"
ctr_object *ctr_map_assign(ctr_object *myself, ctr_argument *argumentList) {
  ctr_object *to = argumentList->object;
  if (!ctr_reflect_check_bind_valid(myself, to, 0)) {
    CtrStdFlow = ctr_build_string_from_cstring("Invalid bind");
    return CtrStdNil;
  }

  ctr_mapitem *mapItem;
  ctr_argument narg = {0}, *newArgumentList = &narg;
  mapItem = myself->properties->head;
  newArgumentList->next = argumentList->next;
  while (mapItem) {
    if (mapItem->value == myself) {
      CtrStdFlow =
          ctr_build_string_from_cstring("Assign cannot have selfReference.");
      return myself;
    }
    ctr_object *value;
    if (!(value = ctr_internal_object_find_property(to, mapItem->key, 0))) {
      mapItem = mapItem->next;
      continue;
    }
    value->info.raw = 1;
    if (!ctr_reflect_check_bind_valid(mapItem->value, value, 0)) {

      CtrStdFlow = ctr_build_string_from_cstring("Invalid bind");
      break;
    } else {

      newArgumentList->object = value;
      ctr_send_message(mapItem->value, "unpack:", 7, newArgumentList);
    }
    mapItem = mapItem->next;
  }
  return myself;
}

ctr_object *ctr_map_contains(ctr_object *myself, ctr_argument *argumentList) {
  ctr_object *searchKey;
  ctr_object *foundObject;

  searchKey = argumentList->object;
  ctr_object *hasher = ctr_get_responder(searchKey, "iHash", 5);

  if (!hasher) {
    searchKey = ctr_send_message(searchKey, "toString", 8,
// # 2797 "collections.c" 3 4
                                 ((void *)0)
// # 2797 "collections.c"
    );

    if (searchKey->info.type != 3) {
      searchKey = ctr_internal_cast2string(searchKey);
    }

    foundObject = ctr_internal_object_find_property(myself, searchKey, 0);
    return ctr_build_bool(foundObject !=
// # 2806 "collections.c" 3 4
                          ((void *)0)
// # 2806 "collections.c"
    );
  } else {
    ctr_number hashk;
    ctr_object *searchKeyHasho = ctr_send_message(searchKey, "iHash", 5,
// # 2811 "collections.c" 3 4
                                                  ((void *)0)
// # 2811 "collections.c"
    );
    if (searchKeyHasho->info.type != 2) {
      foundObject = ctr_internal_object_find_property(myself, searchKey, 0);
    } else {
      hashk = searchKeyHasho->value.nvalue;
      foundObject = ctr_internal_object_find_property_with_hash(
          myself, searchKey, *(uint64_t *)&hashk, 0);
    }
    return ctr_build_bool(foundObject !=
// # 2821 "collections.c" 3 4
                          ((void *)0)
// # 2821 "collections.c"
    );
  }
}

uint32_t ctr_internal_value_hash(ctr_object *searchKey) {
  ctr_object *hasher = ctr_get_responder(searchKey, "iHash", 5);

  if (!hasher) {
    searchKey = ctr_send_message(searchKey, "toString", 8,
// # 2831 "collections.c" 3 4
                                 ((void *)0)
// # 2831 "collections.c"
    );

    if (searchKey->info.type != 3)
      searchKey = ctr_internal_cast2string(searchKey);
    return ctr_internal_index_hash(searchKey);
  } else {
    ctr_object *searchKeyHasho = ctr_send_message(searchKey, "iHash", 5,
// # 2839 "collections.c" 3 4
                                                  ((void *)0)
// # 2839 "collections.c"
    );
    if (searchKeyHasho->info.type != 2)
      return ctr_internal_index_hash(searchKeyHasho);
    else
      return searchKeyHasho->value.nvalue;
  }
}
typedef struct kh_ctr_hmap_t_s {
  khint_t n_buckets, size, n_occupied, upper_bound;
  khint32_t *flags;
  ctr_object **keys;
  ctr_object **vals;
} kh_ctr_hmap_t_t;
static inline __attribute__((__unused__)) kh_ctr_hmap_t_t *
kh_init_ctr_hmap_t(void) {
  return (kh_ctr_hmap_t_t *)ctr_heap_allocate((1) * (sizeof(kh_ctr_hmap_t_t)));
}
static inline __attribute__((__unused__)) void
kh_destroy_ctr_hmap_t(kh_ctr_hmap_t_t *h) {
  if (h) {
    ctr_heap_free((void *)h->keys);
    ctr_heap_free(h->flags);
    ctr_heap_free((void *)h->vals);
    ctr_heap_free(h);
  }
}
static inline __attribute__((__unused__)) void
kh_clear_ctr_hmap_t(kh_ctr_hmap_t_t *h) {
  if (h && h->flags) {
    memset(h->flags, 0xaa,
           ((h->n_buckets) < 16 ? 1 : (h->n_buckets) >> 4) * sizeof(khint32_t));
    h->size = h->n_occupied = 0;
  }
}
static inline __attribute__((__unused__)) khint_t
kh_get_ctr_hmap_t(const kh_ctr_hmap_t_t *h, ctr_object *key) {
  if (h->n_buckets) {
    khint_t k, i, last, mask, step = 0;
    mask = h->n_buckets - 1;
    k = ctr_internal_value_hash(key);
    i = k & mask;
    last = i;
    while (!((h->flags[i >> 4] >> ((i & 0xfU) << 1)) & 2) &&
           (((h->flags[i >> 4] >> ((i & 0xfU) << 1)) & 1) ||
            !ctr_internal_object_is_equal(h->keys[i], key))) {
      i = (i + (++step)) & mask;
      if (i == last)
        return h->n_buckets;
    }
    return ((h->flags[i >> 4] >> ((i & 0xfU) << 1)) & 3) ? h->n_buckets : i;
  } else
    return 0;
}
static inline __attribute__((__unused__)) int
kh_resize_ctr_hmap_t(kh_ctr_hmap_t_t *h, khint_t new_n_buckets) {
  khint32_t *new_flags = 0;
  khint_t j = 1;
  {
    (--(new_n_buckets), (new_n_buckets) |= (new_n_buckets) >> 1,
     (new_n_buckets) |= (new_n_buckets) >> 2,
     (new_n_buckets) |= (new_n_buckets) >> 4,
     (new_n_buckets) |= (new_n_buckets) >> 8,
     (new_n_buckets) |= (new_n_buckets) >> 16, ++(new_n_buckets));
    if (new_n_buckets < 4)
      new_n_buckets = 4;
    if (h->size >= (khint_t)(new_n_buckets * __ac_HASH_UPPER + 0.5))
      j = 0;
    else {
      new_flags = (khint32_t *)ctr_heap_allocate(
          ((new_n_buckets) < 16 ? 1 : (new_n_buckets) >> 4) *
          sizeof(khint32_t));
      if (!new_flags)
        return -1;
      memset(new_flags, 0xaa,
             ((new_n_buckets) < 16 ? 1 : (new_n_buckets) >> 4) *
                 sizeof(khint32_t));
      if (h->n_buckets < new_n_buckets) {
        ctr_object **new_keys = (ctr_object **)ctr_heap_reallocate(
            (void *)h->keys, new_n_buckets * sizeof(ctr_object *));
        if (!new_keys) {
          ctr_heap_free(new_flags);
          return -1;
        }
        h->keys = new_keys;
        if (1) {
          ctr_object **new_vals = (ctr_object **)ctr_heap_reallocate(
              (void *)h->vals, new_n_buckets * sizeof(ctr_object *));
          if (!new_vals) {
            ctr_heap_free(new_flags);
            return -1;
          }
          h->vals = new_vals;
        }
      }
    }
  }
  if (j) {
    for (j = 0; j != h->n_buckets; ++j) {
      if (((h->flags[j >> 4] >> ((j & 0xfU) << 1)) & 3) == 0) {
        ctr_object *key = h->keys[j];
        ctr_object *val;
        khint_t new_mask;
        new_mask = new_n_buckets - 1;
        if (1)
          val = h->vals[j];
        (h->flags[j >> 4] |= 1ul << ((j & 0xfU) << 1));
        while (1) {
          khint_t k, i, step = 0;
          k = ctr_internal_value_hash(key);
          i = k & new_mask;
          while (!((new_flags[i >> 4] >> ((i & 0xfU) << 1)) & 2))
            i = (i + (++step)) & new_mask;
          (new_flags[i >> 4] &= ~(2ul << ((i & 0xfU) << 1)));
          if (i < h->n_buckets &&
              ((h->flags[i >> 4] >> ((i & 0xfU) << 1)) & 3) == 0) {
            {
              ctr_object *tmp = h->keys[i];
              h->keys[i] = key;
              key = tmp;
            }
            if (1) {
              ctr_object *tmp = h->vals[i];
              h->vals[i] = val;
              val = tmp;
            }
            (h->flags[i >> 4] |= 1ul << ((i & 0xfU) << 1));
          } else {
            h->keys[i] = key;
            if (1)
              h->vals[i] = val;
            break;
          }
        }
      }
    }
    if (h->n_buckets > new_n_buckets) {
      h->keys = (ctr_object **)ctr_heap_reallocate(
          (void *)h->keys, new_n_buckets * sizeof(ctr_object *));
      if (1)
        h->vals = (ctr_object **)ctr_heap_reallocate(
            (void *)h->vals, new_n_buckets * sizeof(ctr_object *));
    }
    ctr_heap_free(h->flags);
    h->flags = new_flags;
    h->n_buckets = new_n_buckets;
    h->n_occupied = h->size;
    h->upper_bound = (khint_t)(h->n_buckets * __ac_HASH_UPPER + 0.5);
  }
  return 0;
}
static inline __attribute__((__unused__)) khint_t
kh_put_ctr_hmap_t(kh_ctr_hmap_t_t *h, ctr_object *key, int *ret) {
  khint_t x;
  if (h->n_occupied >= h->upper_bound) {
    if (h->n_buckets > (h->size << 1)) {
      if (kh_resize_ctr_hmap_t(h, h->n_buckets - 1) < 0) {
        *ret = -1;
        return h->n_buckets;
      }
    } else if (kh_resize_ctr_hmap_t(h, h->n_buckets + 1) < 0) {
      *ret = -1;
      return h->n_buckets;
    }
  }
  {
    khint_t k, i, site, last, mask = h->n_buckets - 1, step = 0;
    x = site = h->n_buckets;
    k = ctr_internal_value_hash(key);
    i = k & mask;
    if (((h->flags[i >> 4] >> ((i & 0xfU) << 1)) & 2))
      x = i;
    else {
      last = i;
      while (!((h->flags[i >> 4] >> ((i & 0xfU) << 1)) & 2) &&
             (((h->flags[i >> 4] >> ((i & 0xfU) << 1)) & 1) ||
              !ctr_internal_object_is_equal(h->keys[i], key))) {
        if (((h->flags[i >> 4] >> ((i & 0xfU) << 1)) & 1))
          site = i;
        i = (i + (++step)) & mask;
        if (i == last) {
          x = site;
          break;
        }
      }
      if (x == h->n_buckets) {
        if (((h->flags[i >> 4] >> ((i & 0xfU) << 1)) & 2) &&
            site != h->n_buckets)
          x = site;
        else
          x = i;
      }
    }
  }
  if (((h->flags[x >> 4] >> ((x & 0xfU) << 1)) & 2)) {
    h->keys[x] = key;
    (h->flags[x >> 4] &= ~(3ul << ((x & 0xfU) << 1)));
    ++h->size;
    ++h->n_occupied;
    *ret = 1;
  } else if (((h->flags[x >> 4] >> ((x & 0xfU) << 1)) & 1)) {
    h->keys[x] = key;
    (h->flags[x >> 4] &= ~(3ul << ((x & 0xfU) << 1)));
    ++h->size;
    *ret = 2;
  } else
    *ret = 0;
  return x;
}
static inline __attribute__((__unused__)) void
kh_del_ctr_hmap_t(kh_ctr_hmap_t_t *h, khint_t x) {
  if (x != h->n_buckets && !((h->flags[x >> 4] >> ((x & 0xfU) << 1)) & 3)) {
    (h->flags[x >> 4] |= 1ul << ((x & 0xfU) << 1));
    --h->size;
  }
};

ctr_object *ctr_hmap_wrap(void *ptr) {
  ctr_object *obj = ctr_internal_create_object(9);
  ctr_set_link_all(obj, CtrStdHashMap);
  ctr_resource *res = ctr_heap_allocate(sizeof(*res));
  res->type = 1;
  res->ptr = ptr;
  obj->value.rvalue = res;
  return obj;
}
void *ctr_hmap_unwrap(ctr_object *obj) {
  ctr_resource *res = obj->value.rvalue;
  if (!res)
    return
// # 2866 "collections.c" 3 4
        ((void *)0)
// # 2866 "collections.c"
            ;
  return res->ptr;
}
ctr_object *ctr_hmap_new(ctr_object *myself, ctr_argument *argumentList) {
  kh_ctr_hmap_t_t *p = kh_init_ctr_hmap_t();
  return ctr_hmap_wrap(p);
}
ctr_object *ctr_hmap_type(ctr_object *myself, ctr_argument *argumentList) {
  return ctr_build_string_from_cstring("HashMap");
}
ctr_object *ctr_hmap_merge(ctr_object *myself, ctr_argument *argumentList) {
  return myself;
}
ctr_object *ctr_hmap_keys(ctr_object *myself, ctr_argument *argumentList) {
  kh_ctr_hmap_t_t *map = ctr_hmap_unwrap(myself);
  if (!map) {
    CtrStdFlow = ctr_build_string_from_cstring((char *)__func__);
    return CtrStdNil;
  };
  ctr_object *keys = ctr_array_new(CtrStdArray,
// # 2883 "collections.c" 3 4
                                   ((void *)0)
// # 2883 "collections.c"
  );
  for (khiter_t k = (khint_t)(0); k != ((map)->n_buckets); ++k)
    if ((!(((map)->flags[(k) >> 4] >> (((k)&0xfU) << 1)) & 3)))
      ctr_array_push(keys, &(ctr_argument){((map)->keys[k]),
// # 2886 "collections.c" 3 4
                                           ((void *)0)
// # 2886 "collections.c"
                           });
  return keys;
}
ctr_object *ctr_hmap_values(ctr_object *myself, ctr_argument *argumentList) {
  kh_ctr_hmap_t_t *map = ctr_hmap_unwrap(myself);
  if (!map) {
    CtrStdFlow = ctr_build_string_from_cstring((char *)__func__);
    return CtrStdNil;
  };
  ctr_object *values = ctr_array_new(CtrStdArray,
// # 2893 "collections.c" 3 4
                                     ((void *)0)
// # 2893 "collections.c"
  );
  for (khiter_t k = (khint_t)(0); k != ((map)->n_buckets); ++k)
    if ((!(((map)->flags[(k) >> 4] >> (((k)&0xfU) << 1)) & 3)))
      ctr_array_push(values, &(ctr_argument){((map)->vals[k]),
// # 2896 "collections.c" 3 4
                                             ((void *)0)
// # 2896 "collections.c"
                             });
  return values;
}
ctr_object *ctr_hmap_put(ctr_object *myself, ctr_argument *argumentList) {
  kh_ctr_hmap_t_t *map = ctr_hmap_unwrap(myself);
  if (!map) {
    CtrStdFlow = ctr_build_string_from_cstring((char *)__func__);
    return CtrStdNil;
  };
  int ret;
  khiter_t k = kh_put_ctr_hmap_t(map, argumentList->next->object, &ret);
  ((map)->vals[k]) = argumentList->object;
  return myself;
}
ctr_object *ctr_hmap_rm(ctr_object *myself, ctr_argument *argumentList) {
  kh_ctr_hmap_t_t *map = ctr_hmap_unwrap(myself);
  if (!map) {
    CtrStdFlow = ctr_build_string_from_cstring((char *)__func__);
    return CtrStdNil;
  };
  khiter_t k = kh_get_ctr_hmap_t(map, argumentList->object);
  if (k == ((map)->n_buckets))
    return myself;
  kh_del_ctr_hmap_t(map, k);
  return myself;
}
ctr_object *ctr_hmap_get(ctr_object *myself, ctr_argument *argumentList) {
  kh_ctr_hmap_t_t *map = ctr_hmap_unwrap(myself);
  if (!map) {
    CtrStdFlow = ctr_build_string_from_cstring((char *)__func__);
    return CtrStdNil;
  };
  khiter_t k = kh_get_ctr_hmap_t(map, argumentList->object);
  if (k == ((map)->n_buckets))
    return CtrStdNil;
  return ((map)->vals[k]);
}
ctr_object *ctr_hmap_get_or_insert(ctr_object *myself,
                                   ctr_argument *argumentList) {
  kh_ctr_hmap_t_t *map = ctr_hmap_unwrap(myself);
  if (!map) {
    CtrStdFlow = ctr_build_string_from_cstring((char *)__func__);
    return CtrStdNil;
  };
  int ret;
  khiter_t k = kh_get_ctr_hmap_t(map, argumentList->object);
  if (k == ((map)->n_buckets)) {
    k = kh_put_ctr_hmap_t(map, argumentList->object, &ret);
    ctr_object *nil = ctr_internal_create_object(0);
    ctr_set_link_all(nil, CtrStdNil);
    ((map)->vals[k]) = nil;
  }
  return ((map)->vals[k]);
}
ctr_object *ctr_hmap_count(ctr_object *myself, ctr_argument *argumentList) {
  kh_ctr_hmap_t_t *map = ctr_hmap_unwrap(myself);
  if (!map) {
    CtrStdFlow = ctr_build_string_from_cstring((char *)__func__);
    return CtrStdNil;
  };
  return ctr_build_number_from_float(((map)->size));
}
ctr_object *ctr_hmap_empty(ctr_object *myself, ctr_argument *argumentList) {
  kh_ctr_hmap_t_t *map = ctr_hmap_unwrap(myself);
  if (!map) {
    CtrStdFlow = ctr_build_string_from_cstring((char *)__func__);
    return CtrStdNil;
  };
  return ctr_build_bool(((map)->size) == 0);
}
ctr_object *ctr_hmap_each(ctr_object *myself, ctr_argument *argumentList) {
  kh_ctr_hmap_t_t *map = ctr_hmap_unwrap(myself);
  if (!map) {
    CtrStdFlow = ctr_build_string_from_cstring((char *)__func__);
    return CtrStdNil;
  };
  ctr_object *blk = argumentList->object;
  for (khiter_t k = (khint_t)(0); k != ((map)->n_buckets); ++k)
    if ((!(((map)->flags[(k) >> 4] >> (((k)&0xfU) << 1)) & 3))) {
      ctr_block_run(blk,
                    &(ctr_argument){((map)->keys[k]),
// # 2960 "collections.c" 3 4
                                    ((void *)0)
// # 2960 "collections.c"
                    },
// # 2960 "collections.c" 3 4
                    ((void *)0)
// # 2960 "collections.c"
      );
    }
  return myself;
}
ctr_object *ctr_hmap_fmap(ctr_object *myself, ctr_argument *argumentList) {
  kh_ctr_hmap_t_t *map = ctr_hmap_unwrap(myself);
  if (!map) {
    CtrStdFlow = ctr_build_string_from_cstring((char *)__func__);
    return CtrStdNil;
  };
  ctr_object *blk = argumentList->object;
  int ret;
  kh_ctr_hmap_t_t *p = kh_init_ctr_hmap_t();
  for (khiter_t k = (khint_t)(0); k != ((map)->n_buckets); ++k)
    if ((!(((map)->flags[(k) >> 4] >> (((k)&0xfU) << 1)) & 3))) {
      ctr_object *val = ctr_block_run(blk,
                                      &(ctr_argument){((map)->keys[k]),
// # 2973 "collections.c" 3 4
                                                      ((void *)0)
// # 2973 "collections.c"
                                      },
// # 2973 "collections.c" 3 4
                                      ((void *)0)
// # 2973 "collections.c"
      );
      khiter_t k = kh_put_ctr_hmap_t(p, argumentList->object, &ret);
      ((p)->vals[k]) = val;
    }
  return ctr_hmap_wrap(p);
}
ctr_object *ctr_hmap_fmap_inp(ctr_object *myself, ctr_argument *argumentList) {
  kh_ctr_hmap_t_t *map = ctr_hmap_unwrap(myself);
  if (!map) {
    CtrStdFlow = ctr_build_string_from_cstring((char *)__func__);
    return CtrStdNil;
  };
  ctr_object *blk = argumentList->object;
  for (khiter_t k = (khint_t)(0); k != ((map)->n_buckets); ++k)
    if ((!(((map)->flags[(k) >> 4] >> (((k)&0xfU) << 1)) & 3))) {
      ((map)->vals[k]) = ctr_block_run(blk,
                                       &(ctr_argument){((map)->keys[k]),
// # 2986 "collections.c" 3 4
                                                       ((void *)0)
// # 2986 "collections.c"
                                       },
// # 2986 "collections.c" 3 4
                                       ((void *)0)
// # 2986 "collections.c"
      );
    }
  return myself;
}
ctr_object *ctr_hmap_kvmap(ctr_object *myself, ctr_argument *argumentList) {
  return myself;
}
ctr_object *ctr_hmap_kvlist(ctr_object *myself, ctr_argument *argumentList) {
  return myself;
}
ctr_object *ctr_hmap_contains(ctr_object *myself, ctr_argument *argumentList) {
  kh_ctr_hmap_t_t *map = ctr_hmap_unwrap(myself);
  if (!map) {
    CtrStdFlow = ctr_build_string_from_cstring((char *)__func__);
    return CtrStdNil;
  };
  return ctr_build_bool(kh_get_ctr_hmap_t(map, argumentList->object) !=
                        ((map)->n_buckets));
}
ctr_object *ctr_hmap_flip(ctr_object *myself, ctr_argument *argumentList) {
  return myself;
}
ctr_object *ctr_hmap_assign(ctr_object *myself, ctr_argument *argumentList) {
  return myself;
}
ctr_object *ctr_hmap_to_string(ctr_object *myself, ctr_argument *argumentList) {
  kh_ctr_hmap_t_t *map = ctr_hmap_unwrap(myself);
  if (!map) {
    CtrStdFlow = ctr_build_string_from_cstring((char *)__func__);
    return CtrStdNil;
  };
  return ctr_format_str("[HashMap with %d elements]", ((map)->size));
}
