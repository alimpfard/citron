#ifndef REVERSE_SIZES_H
#define REVERSE_SIZES_H

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <ffi.h>
#include "_struct.h"

struct reverse_ffi_type_size_map_item {
    ffi_type* type;
    size_t size;
};

static struct reverse_ffi_type_size_map_item reverse_ffi_type_size_map[] = {
    {&ffi_type_uint8,                                   sizeof(uint8_t)},
    {(ffi_type*) &wrapped_ffi_type_uint8,               sizeof(uint8_t)},
    {&ffi_type_sint8,                                   sizeof(int8_t)},
    {(ffi_type*) &wrapped_ffi_type_sint8,               sizeof(int8_t)},
    {&ffi_type_uint16,                                  sizeof(uint16_t)},
    {(ffi_type*) &wrapped_ffi_type_uint16,              sizeof(uint16_t)},
    {&ffi_type_sint16,                                  sizeof(int16_t)},
    {(ffi_type*) &wrapped_ffi_type_sint16,              sizeof(int16_t)},
    {&ffi_type_uint32,                                  sizeof(uint32_t)},
    {(ffi_type*) &wrapped_ffi_type_uint32,              sizeof(uint32_t)},
    {&ffi_type_sint32,                                  sizeof(int32_t)},
    {(ffi_type*) &wrapped_ffi_type_sint32,              sizeof(int32_t)},
    {&ffi_type_uint64,                                  sizeof(uint64_t)},
    {(ffi_type*) &wrapped_ffi_type_uint64,              sizeof(uint64_t)},
    {&ffi_type_sint64,                                  sizeof(int64_t)},
    {(ffi_type*) &wrapped_ffi_type_sint64,              sizeof(int64_t)},
    {&ffi_type_float,                                   sizeof(float)},
    {(ffi_type*) &wrapped_ffi_type_float,               sizeof(float)},
    {&ffi_type_double,                                  sizeof(double)},
    {(ffi_type*) &wrapped_ffi_type_double,              sizeof(double)},
    {&ffi_type_uchar,                                   sizeof(unsigned char)},
    {(ffi_type*) &wrapped_ffi_type_uchar,               sizeof(unsigned char)},
    {&ffi_type_schar,                                   sizeof(char)},
    {(ffi_type*) &wrapped_ffi_type_schar,               sizeof(char)},
    {&ffi_type_ushort,                                  sizeof(unsigned short)},
    {(ffi_type*) &wrapped_ffi_type_ushort,              sizeof(unsigned short)},
    {&ffi_type_sshort,                                  sizeof(short)},
    {(ffi_type*) &wrapped_ffi_type_sshort,              sizeof(short)},
    {&ffi_type_uint,                                    sizeof(unsigned int)},
    {(ffi_type*) &wrapped_ffi_type_uint,                sizeof(unsigned int)},
    {&ffi_type_sint,                                    sizeof(int)},
    {(ffi_type*) &wrapped_ffi_type_sint,                sizeof(int)},
    {&ffi_type_ulong,                                   sizeof(unsigned long)},
    {(ffi_type*) &wrapped_ffi_type_ulong,               sizeof(unsigned long)},
    {&ffi_type_slong,                                   sizeof(long)},
    {(ffi_type*) &wrapped_ffi_type_slong,               sizeof(long)},
    {&ffi_type_longdouble,                              sizeof(long double)},
    {(ffi_type*) &wrapped_ffi_type_longdouble,          sizeof(long double)},
    {&ffi_type_pointer,                                 sizeof(void*)},
    {(ffi_type*) &wrapped_ffi_type_pointer,             sizeof(void*)},
    {NULL, 0}
};

static size_t reverse_ffi_type_size_map_lookup(ffi_type* type) {
  int i=0;
  return type->size;
  /*
  while(reverse_ffi_type_size_map[i].type!=NULL) {
    if(reverse_ffi_type_size_map[i].type == type) return reverse_ffi_type_size_map[i].size;
    i++;
  }
  printf("REVERSE LOOKUP %p\n", type);
  return 0;
  */
}

#endif /* end of include guard: REVERSE_SIZES_H */
