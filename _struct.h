#ifndef CTR__STRUCT_H
#define CTR__STRUCT_H

#include <stdbool.h>
#include <stdlib.h>
#include <ffi.h>

#ifndef CTR_H_DECLSPEC
#ifdef CTR_DEFINE
#define CTR_H_DECLSPEC
#else
#define CTR_H_DECLSPEC extern
#endif
#endif

struct wrapped_ffi_type {
  /* struct _ffi_type */
  size_t size;
  unsigned short alignment;
  unsigned short type;
  struct _ffi_type **elements;
  /* extension */
  unsigned int extension_data;
};

/* consider this a joke */
#define WRAPPED_FFI_TYPE_MAGIC 0xdeafbabe
#define WRAPPED_FFI_TYPE_UNION 64

typedef struct wrapped_ffi_type wrapped_ffi_type;

typedef struct { char c; short x; } st_short;
typedef struct { char c; int x; } st_int;
typedef struct { char c; long x; } st_long;
typedef struct { char c; float x; } st_float;
typedef struct { char c; double x; } st_double;
typedef struct { char c; void *x; } st_void_p;
typedef struct { char c; size_t x; } st_size_t;
typedef struct { char c; bool x; } st_bool;

#define SHORT_ALIGN (sizeof(st_short) - sizeof(short))
#define INT_ALIGN (sizeof(st_int) - sizeof(int))
#define LONG_ALIGN (sizeof(st_long) - sizeof(long))
#define FLOAT_ALIGN (sizeof(st_float) - sizeof(float))
#define DOUBLE_ALIGN (sizeof(st_double) - sizeof(double))
#define VOID_P_ALIGN (sizeof(st_void_p) - sizeof(void *))
#define SIZE_T_ALIGN (sizeof(st_size_t) - sizeof(size_t))
#define BOOL_ALIGN (sizeof(st_bool) - sizeof(bool))

typedef struct { char c; long long x; } s_long_long;
#define LONG_LONG_ALIGN (sizeof(s_long_long) - sizeof(long long))

#ifndef __cplusplus
#define alignof(x) (offsetof(struct { char a; x b; }, b))
#endif

typedef struct {
    unsigned int pad : 1;
    ptrdiff_t offset;
} pad_info_node_t;

typedef struct {
    size_t member_count;
    size_t max_alignment;
    size_t max_size;
    pad_info_node_t** pad_structure;
} struct_member_desc_t;

struct_member_desc_t ctr_ffi_type_get_member_count(char* format, size_t* size_out, int record_pads, int packed);//such a big header, eh?
int ctr_ffi_type_struct_sizeof(char* format); //XXX: Hack: specify offsets with the format for now
wrapped_ffi_type* ctr_create_ffi_type_descriptor(char* format, int union_);
wrapped_ffi_type* ctr_create_ffi_type_descriptor_(char* format, int member_count, int union_);
int ctr_create_ffi_str_descriptor(wrapped_ffi_type* type, char* buf);

void ctr_struct_initialize_internal();

CTR_H_DECLSPEC wrapped_ffi_type  wrapped_ffi_type_void      ;
CTR_H_DECLSPEC wrapped_ffi_type  wrapped_ffi_type_uint8     ;
CTR_H_DECLSPEC wrapped_ffi_type  wrapped_ffi_type_sint8     ;
CTR_H_DECLSPEC wrapped_ffi_type  wrapped_ffi_type_uint16    ;
CTR_H_DECLSPEC wrapped_ffi_type  wrapped_ffi_type_sint16    ;
CTR_H_DECLSPEC wrapped_ffi_type  wrapped_ffi_type_uint32    ;
CTR_H_DECLSPEC wrapped_ffi_type  wrapped_ffi_type_sint32    ;
CTR_H_DECLSPEC wrapped_ffi_type  wrapped_ffi_type_uint64    ;
CTR_H_DECLSPEC wrapped_ffi_type  wrapped_ffi_type_sint64    ;
CTR_H_DECLSPEC wrapped_ffi_type  wrapped_ffi_type_uchar     ;
CTR_H_DECLSPEC wrapped_ffi_type  wrapped_ffi_type_schar     ;
CTR_H_DECLSPEC wrapped_ffi_type  wrapped_ffi_type_ushort    ;
CTR_H_DECLSPEC wrapped_ffi_type  wrapped_ffi_type_sshort    ;
CTR_H_DECLSPEC wrapped_ffi_type  wrapped_ffi_type_uint      ;
CTR_H_DECLSPEC wrapped_ffi_type  wrapped_ffi_type_sint      ;
CTR_H_DECLSPEC wrapped_ffi_type  wrapped_ffi_type_ulong     ;
CTR_H_DECLSPEC wrapped_ffi_type  wrapped_ffi_type_slong     ;
CTR_H_DECLSPEC wrapped_ffi_type  wrapped_ffi_type_float     ;
CTR_H_DECLSPEC wrapped_ffi_type  wrapped_ffi_type_double    ;
CTR_H_DECLSPEC wrapped_ffi_type  wrapped_ffi_type_pointer   ;
CTR_H_DECLSPEC wrapped_ffi_type  wrapped_ffi_type_longdouble;


#endif
