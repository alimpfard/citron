//#include <ctype.h>
#include "_struct.h"
#include <ffi.h>
#include <inttypes.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
// #include "structmember.h"
#define max(x, y) ((x) > (y) ? (x) : (y))

#ifndef TEST
#include "citron.h"
#else
#include <stddef.h>
#define ctr_heap_allocate malloc
#endif

#define TEST
#undef TEST
int initd = 0;

#define IS_WRAPPED(x)                                                          \
  ((((wrapped_ffi_type *)(x))->extension_data & WRAPPED_FFI_TYPE_MAGIC) ==     \
   WRAPPED_FFI_TYPE_MAGIC)
#define WRAP_DATA(x)                                                           \
  ((((wrapped_ffi_type *)(x))->extension_data & ~WRAPPED_FFI_TYPE_MAGIC))

void ctr_struct_initialize_internal() {
  if (likely(initd))
    return;
  initd = 1;
  wrapped_ffi_type_void =
      (wrapped_ffi_type){.size = ffi_type_void.size,
                         .alignment = ffi_type_void.alignment,
                         .type = ffi_type_void.type,
                         .elements = ffi_type_void.elements,
                         .extension_data = WRAPPED_FFI_TYPE_MAGIC};
  wrapped_ffi_type_uint8 =
      (wrapped_ffi_type){.size = ffi_type_uint8.size,
                         .alignment = ffi_type_uint8.alignment,
                         .type = ffi_type_uint8.type,
                         .elements = ffi_type_uint8.elements,
                         .extension_data = WRAPPED_FFI_TYPE_MAGIC};
  wrapped_ffi_type_sint8 =
      (wrapped_ffi_type){.size = ffi_type_sint8.size,
                         .alignment = ffi_type_sint8.alignment,
                         .type = ffi_type_sint8.type,
                         .elements = ffi_type_sint8.elements,
                         .extension_data = WRAPPED_FFI_TYPE_MAGIC};
  wrapped_ffi_type_uint16 =
      (wrapped_ffi_type){.size = ffi_type_uint16.size,
                         .alignment = ffi_type_uint16.alignment,
                         .type = ffi_type_uint16.type,
                         .elements = ffi_type_uint16.elements,
                         .extension_data = WRAPPED_FFI_TYPE_MAGIC};
  wrapped_ffi_type_sint16 =
      (wrapped_ffi_type){.size = ffi_type_sint16.size,
                         .alignment = ffi_type_sint16.alignment,
                         .type = ffi_type_sint16.type,
                         .elements = ffi_type_sint16.elements,
                         .extension_data = WRAPPED_FFI_TYPE_MAGIC};
  wrapped_ffi_type_uint32 =
      (wrapped_ffi_type){.size = ffi_type_uint32.size,
                         .alignment = ffi_type_uint32.alignment,
                         .type = ffi_type_uint32.type,
                         .elements = ffi_type_uint32.elements,
                         .extension_data = WRAPPED_FFI_TYPE_MAGIC};
  wrapped_ffi_type_sint32 =
      (wrapped_ffi_type){.size = ffi_type_sint32.size,
                         .alignment = ffi_type_sint32.alignment,
                         .type = ffi_type_sint32.type,
                         .elements = ffi_type_sint32.elements,
                         .extension_data = WRAPPED_FFI_TYPE_MAGIC};
  wrapped_ffi_type_uint64 =
      (wrapped_ffi_type){.size = ffi_type_uint64.size,
                         .alignment = ffi_type_uint64.alignment,
                         .type = ffi_type_uint64.type,
                         .elements = ffi_type_uint64.elements,
                         .extension_data = WRAPPED_FFI_TYPE_MAGIC};
  wrapped_ffi_type_sint64 =
      (wrapped_ffi_type){.size = ffi_type_sint64.size,
                         .alignment = ffi_type_sint64.alignment,
                         .type = ffi_type_sint64.type,
                         .elements = ffi_type_sint64.elements,
                         .extension_data = WRAPPED_FFI_TYPE_MAGIC};
  wrapped_ffi_type_uchar =
      (wrapped_ffi_type){.size = ffi_type_uchar.size,
                         .alignment = ffi_type_uchar.alignment,
                         .type = ffi_type_uchar.type,
                         .elements = ffi_type_uchar.elements,
                         .extension_data = WRAPPED_FFI_TYPE_MAGIC};
  wrapped_ffi_type_schar =
      (wrapped_ffi_type){.size = ffi_type_schar.size,
                         .alignment = ffi_type_schar.alignment,
                         .type = ffi_type_schar.type,
                         .elements = ffi_type_schar.elements,
                         .extension_data = WRAPPED_FFI_TYPE_MAGIC};
  wrapped_ffi_type_ushort =
      (wrapped_ffi_type){.size = ffi_type_ushort.size,
                         .alignment = ffi_type_ushort.alignment,
                         .type = ffi_type_ushort.type,
                         .elements = ffi_type_ushort.elements,
                         .extension_data = WRAPPED_FFI_TYPE_MAGIC};
  wrapped_ffi_type_sshort =
      (wrapped_ffi_type){.size = ffi_type_sshort.size,
                         .alignment = ffi_type_sshort.alignment,
                         .type = ffi_type_sshort.type,
                         .elements = ffi_type_sshort.elements,
                         .extension_data = WRAPPED_FFI_TYPE_MAGIC};
  wrapped_ffi_type_uint =
      (wrapped_ffi_type){.size = ffi_type_uint.size,
                         .alignment = ffi_type_uint.alignment,
                         .type = ffi_type_uint.type,
                         .elements = ffi_type_uint.elements,
                         .extension_data = WRAPPED_FFI_TYPE_MAGIC};
  wrapped_ffi_type_sint =
      (wrapped_ffi_type){.size = ffi_type_sint.size,
                         .alignment = ffi_type_sint.alignment,
                         .type = ffi_type_sint.type,
                         .elements = ffi_type_sint.elements,
                         .extension_data = WRAPPED_FFI_TYPE_MAGIC};
  wrapped_ffi_type_ulong =
      (wrapped_ffi_type){.size = ffi_type_ulong.size,
                         .alignment = ffi_type_ulong.alignment,
                         .type = ffi_type_ulong.type,
                         .elements = ffi_type_ulong.elements,
                         .extension_data = WRAPPED_FFI_TYPE_MAGIC};
  wrapped_ffi_type_slong =
      (wrapped_ffi_type){.size = ffi_type_slong.size,
                         .alignment = ffi_type_slong.alignment,
                         .type = ffi_type_slong.type,
                         .elements = ffi_type_slong.elements,
                         .extension_data = WRAPPED_FFI_TYPE_MAGIC};
  wrapped_ffi_type_float =
      (wrapped_ffi_type){.size = ffi_type_float.size,
                         .alignment = ffi_type_float.alignment,
                         .type = ffi_type_float.type,
                         .elements = ffi_type_float.elements,
                         .extension_data = WRAPPED_FFI_TYPE_MAGIC};
  wrapped_ffi_type_double =
      (wrapped_ffi_type){.size = ffi_type_double.size,
                         .alignment = ffi_type_double.alignment,
                         .type = ffi_type_double.type,
                         .elements = ffi_type_double.elements,
                         .extension_data = WRAPPED_FFI_TYPE_MAGIC};
  wrapped_ffi_type_pointer =
      (wrapped_ffi_type){.size = ffi_type_pointer.size,
                         .alignment = ffi_type_pointer.alignment,
                         .type = ffi_type_pointer.type,
                         .elements = ffi_type_pointer.elements,
                         .extension_data = WRAPPED_FFI_TYPE_MAGIC};
  wrapped_ffi_type_longdouble =
      (wrapped_ffi_type){.size = ffi_type_longdouble.size,
                         .alignment = ffi_type_longdouble.alignment,
                         .type = ffi_type_longdouble.type,
                         .elements = ffi_type_longdouble.elements,
                         .extension_data = WRAPPED_FFI_TYPE_MAGIC};
}
/*
    v      wrapped_ffi_type_void
    3ui    wrapped_ffi_type_uint8
    3si    wrapped_ffi_type_sint8
    4ui    wrapped_ffi_type_uint16
    4si    wrapped_ffi_type_sint16
    5ui    wrapped_ffi_type_uint32
    5si    wrapped_ffi_type_sint32
    6ui    wrapped_ffi_type_uint64
    6si    wrapped_ffi_type_sint64
    uc     wrapped_ffi_type_uchar
    sc     wrapped_ffi_type_schar
    us     wrapped_ffi_type_ushort
    ss     wrapped_ffi_type_sshort
    ui     wrapped_ffi_type_uint
    si     wrapped_ffi_type_sint
    ul     wrapped_ffi_type_ulong
    sl     wrapped_ffi_type_slong
    f      wrapped_ffi_type_float
    d      wrapped_ffi_type_double
    p      wrapped_ffi_type_pointer
    l      wrapped_ffi_type_longdouble
    [...]  new struct def
    {...}  union
    <\d+>  pad with size
 */
wrapped_ffi_type *ctr_ffi_type_get_format_splat(char **format,
                                                ssize_t *this_size) {
  ctr_struct_initialize_internal();
  switch (**format) {
    // case 'v': *this_size = sizeof(void); return &wrapped_ffi_type_void;
  case '3': {
    (*format)++;
    switch (**format) {
    case 'u': {
      (*format)++;
      if (**format != 'i')
        return NULL;
      ++*format;
      *this_size = sizeof(uint8_t);
      return &wrapped_ffi_type_uint8;
    }
    case 's': {
      (*format)++;
      if (**format != 'i')
        return NULL;
      ++*format;
      *this_size = sizeof(int8_t);
      return &wrapped_ffi_type_sint8;
    }
    default:
      return NULL;
    }
  }
  case '4': {
    (*format)++;
    switch (**format) {
    case 'u': {
      (*format)++;
      if (**format != 'i')
        return NULL;
      ++*format;
      *this_size = sizeof(uint16_t);
      return &wrapped_ffi_type_uint16;
    }
    case 's': {
      (*format)++;
      if (**format != 'i')
        return NULL;
      ++*format;
      *this_size = sizeof(int16_t);
      return &wrapped_ffi_type_sint16;
    }
    default:
      return NULL;
    }
  }
  case '5': {
    (*format)++;
    switch (**format) {
    case 'u': {
      (*format)++;
      if (**format != 'i')
        return NULL;
      ++*format;
      *this_size = sizeof(uint32_t);
      return &wrapped_ffi_type_uint32;
    }
    case 's': {
      (*format)++;
      if (**format != 'i')
        return NULL;
      ++*format;
      *this_size = sizeof(int32_t);
      return &wrapped_ffi_type_sint32;
    }
    default:
      return NULL;
    }
  }
  case '6': {
    (*format)++;
    switch (**format) {
    case 'u': {
      (*format)++;
      if (**format != 'i')
        return NULL;
      ++*format;
      *this_size = sizeof(uint64_t);
      return &wrapped_ffi_type_uint64;
    }
    case 's': {
      (*format)++;
      if (**format != 'i')
        return NULL;
      ++*format;
      *this_size = sizeof(int64_t);
      return &wrapped_ffi_type_sint64;
    }
    default:
      return NULL;
    }
  }
  case 'f':
    ++*format;
    *this_size = sizeof(float);
    return &wrapped_ffi_type_float;
  case 'd':
    ++*format;
    *this_size = sizeof(double);
    return &wrapped_ffi_type_double;
  case 'p':
    ++*format;
    *this_size = sizeof(void *);
    return &wrapped_ffi_type_pointer;
  case 'l':
    ++*format;
    *this_size = sizeof(long long);
    return &wrapped_ffi_type_longdouble;
  case 'u': {
    (*format)++;
    switch (**format) {
    case 'c':
      ++*format;
      *this_size = sizeof(unsigned char);
      return &wrapped_ffi_type_uchar;
    case 's':
      ++*format;
      *this_size = sizeof(unsigned short);
      return &wrapped_ffi_type_ushort;
    case 'i':
      ++*format;
      *this_size = sizeof(unsigned int);
      return &wrapped_ffi_type_uint;
    case 'l':
      ++*format;
      *this_size = sizeof(unsigned long);
      return &wrapped_ffi_type_ulong;
    default:
      return NULL;
    }
  }
  case 's': {
    (*format)++;
    switch (**format) {
    case 'c':
      ++*format;
      *this_size = sizeof(signed char);
      return &wrapped_ffi_type_schar;
    case 's':
      ++*format;
      *this_size = sizeof(signed short);
      return &wrapped_ffi_type_sshort;
    case 'i':
      ++*format;
      *this_size = sizeof(signed int);
      return &wrapped_ffi_type_sint;
    case 'l':
      ++*format;
      *this_size = sizeof(signed long);
      return &wrapped_ffi_type_slong;
    default:
      return NULL;
    }
  }
  case '<': {
    int size, length;
    int res = sscanf(*format, "<%i>%n", &size, &length);
    (*format) += length;
    if (res != 1) {
      return NULL;
    }
    *this_size = -size; // negative says "pad"
    return &wrapped_ffi_type_uchar;
  }
  case '{': {
    (*format)++;
    return ctr_create_ffi_type_descriptor(*format, 1);
  }
  case '[': {
    (*format)++;
    char *fmt = *format;
    ctr_ffi_type_get_format_splat(&fmt, this_size);
    return ctr_create_ffi_type_descriptor(*format, 0);
  }
  case ']':
  case '}':
  default:
    return NULL;
  }
}

int ctr_ffi_type_struct_sizeof(
    char *format) { // XXX: does not generate padding bytes
  ctr_struct_initialize_internal();
  int size = 0;
  char *beginning = format;
  int struct_opened = 1; // we are in a struct. a stray ']' will terminate this
  int union_opened =
      1; // we pretend we're in a union, a stray '}' will terminate this
  while ((union_opened > 0 && struct_opened > 0) && *format != '\0') {
    switch (*format) {
    case ']':
      struct_opened--;
      break;
    case '}':
      union_opened--;
      break;
    case 'f':
      if (union_opened > 1)
        size = max(size, sizeof(float));
      else
        size += sizeof(float);
      break;
    case 'd':
      size += sizeof(double);
      break;
    case 'p':
      if (union_opened > 1)
        size = max(size, sizeof(void *));
      else
        size += sizeof(void *);
      break;
    case 'l':
      if (union_opened > 1)
        size = max(size, sizeof(long long));
      else
        size += sizeof(long long);
      break;
      // case 'v': size += sizeof(void); break;
    case '3':
      if (union_opened > 1)
        size = max(size, sizeof(uint8_t));
      else
        size += sizeof(uint8_t);
      break;
    case '4':
      if (union_opened > 1)
        size = max(size, sizeof(uint16_t));
      else
        size += sizeof(uint16_t);
      break;
    case '5':
      if (union_opened > 1)
        size = max(size, sizeof(uint32_t));
      else
        size += sizeof(uint32_t);
      break;
    case '6':
      if (union_opened > 1)
        size = max(size, sizeof(uint64_t));
      else
        size += sizeof(uint64_t);
      break;
    case '{': {
      union_opened++;
      int s = ctr_ffi_type_struct_sizeof(format + 1);
      if (s < 0)
        return s - (format - beginning);
      size += s;
      break;
    }
    case '[': {
      struct_opened++; // so that we don't terminate after this struct
      int s = ctr_ffi_type_struct_sizeof(format + 1);
      if (s < 0)
        return s - (format - beginning);
      size += s;
      break;
    }
    case 'u': {
      format++;
      switch (*format) {
      case 'c':
        if (union_opened > 1)
          size = max(size, sizeof(unsigned char));
        else
          size += sizeof(unsigned char);
        break;
      case 's':
        if (union_opened > 1)
          size = max(size, sizeof(unsigned short));
        else
          size += sizeof(unsigned short);
        break;
      case 'i':
        if (union_opened > 1)
          size = max(size, sizeof(unsigned int));
        else
          size += sizeof(unsigned int);
        break;
      case 'l':
        if (union_opened > 1)
          size = max(size, sizeof(unsigned long));
        else
          size += sizeof(unsigned long);
        break;
      default:
        return -((int)(format - beginning + 1));
      }
      break;
    }
    case 's': {
      format++;
      switch (*format) {
      case 'c':
        if (union_opened > 1)
          size = max(size, sizeof(signed char));
        else
          size += sizeof(signed char);
        break;
      case 's':
        if (union_opened > 1)
          size = max(size, sizeof(signed short));
        else
          size += sizeof(signed short);
        break;
      case 'i':
        if (union_opened > 1)
          size = max(size, sizeof(signed int));
        else
          size += sizeof(signed int);
        break;
      case 'l':
        if (union_opened > 1)
          size = max(size, sizeof(signed long));
        else
          size += sizeof(signed long);
        break;
      default:
        return -((int)(format - beginning + 1));
      }
      break;
    }
    default:
      return -((int)(format - beginning + 1));
    }
    format++;
  }
  return size;
}

struct_member_desc_t ctr_ffi_type_get_member_count(char *format,
                                                   size_t *size_out,
                                                   int record_pads,
                                                   int packed) {
  ctr_struct_initialize_internal();
  int mc = 0;
  char *beginning = format;
  int struct_opened = 1;
  int union_opened = 1;
  size_t current_offset = 0;
  size_t this_size = 0;
  size_t max_size = 0;
  size_t this_alignment = 0;
  size_t max_alignment = 0;
  size_t pad = 0;
  size_t padinfo_max = 8;
  size_t padinfo_index = 0;
  pad_info_node_t **padinfo;
  struct_member_desc_t ret;
  if (record_pads)
    padinfo = ctr_heap_allocate(sizeof(pad_info_node_t *) * padinfo_max);
  while (*format != '\0') {
    int element_count = 1;
    switch (*format) {
    case ']':
      struct_opened--;
      pad = 0;
      break;
    case '}':
      union_opened--;
      pad = 0;
      break;
    case 'f':
      this_size = sizeof(float);
      max_size = fmax(this_size, max_size);
      this_alignment = alignof(float);
      max_alignment = fmax(this_alignment, max_alignment);
      if (!packed && current_offset % this_alignment != 0) {
        pad = this_alignment - (current_offset % this_alignment);
        mc += pad;
        current_offset += pad;
      }
      mc++;
      break;
    case 'd':
      this_size = sizeof(double);
      max_size = fmax(this_size, max_size);
      this_alignment = alignof(double);
      max_alignment = fmax(this_alignment, max_alignment);
      if (!packed && current_offset % this_alignment != 0) {
        pad = this_alignment - (current_offset % this_alignment);
        mc += pad;
        current_offset += pad;
      }
      mc++;
      break;
    case 'p':
      this_size = sizeof(void *);
      max_size = fmax(this_size, max_size);
      this_alignment = alignof(void *);
      max_alignment = fmax(this_alignment, max_alignment);
      if (!packed && current_offset % this_alignment != 0) {
        pad = this_alignment - (current_offset % this_alignment);
        mc += pad;
        current_offset += pad;
      }
      mc++;
      break;
    case 'l':
      this_size = sizeof(long long);
      max_size = fmax(this_size, max_size);
      this_alignment = alignof(long long);
      max_alignment = fmax(this_alignment, max_alignment);
      if (!packed && current_offset % this_alignment != 0) {
        pad = this_alignment - (current_offset % this_alignment);
        mc += pad;
        current_offset += pad;
      }
      mc++;
      break;
      // case 'v': this_size = sizeof(void); this_alignment = alignof(void);
      // max_alignment = fmax(this_alignment, max_alignment);
      // if(current_offset%this_alignment!=0) { pad =
      // this_alignment-(current_offset%this_alignment); mc+=pad;
      // current_offset+=pad;} mc++; break;
    case '3':
      this_size = sizeof(uint8_t);
      max_size = fmax(this_size, max_size);
      this_alignment = alignof(uint8_t);
      max_alignment = fmax(this_alignment, max_alignment);
      {
        format++;
        if (*format != 'u' && *format != 's')
          goto exit_error;
        format++;
        if (*format != 'i')
          goto exit_error;
        this_size = sizeof(uint8_t);
        max_size = fmax(this_size, max_size);
        this_alignment = alignof(uint8_t);
        max_alignment = fmax(this_alignment, max_alignment);
        if (!packed && current_offset % this_alignment != 0) {
          pad = this_alignment - (current_offset % this_alignment);
          mc += pad;
          current_offset += pad;
        }
        mc++;
        break;
      }
    case '4':
      this_size = sizeof(uint16_t);
      max_size = fmax(this_size, max_size);
      this_alignment = alignof(uint16_t);
      max_alignment = fmax(this_alignment, max_alignment);
      {
        format++;
        if (*format != 'u' && *format != 's')
          goto exit_error;
        format++;
        if (*format != 'i')
          goto exit_error;
        this_size = sizeof(uint32_t);
        max_size = fmax(this_size, max_size);
        this_alignment = alignof(uint32_t);
        max_alignment = fmax(this_alignment, max_alignment);
        if (!packed && current_offset % this_alignment != 0) {
          pad = this_alignment - (current_offset % this_alignment);
          mc += pad;
          current_offset += pad;
        }
        mc++;
        break;
      }
    case '5':
      this_size = sizeof(uint32_t);
      max_size = fmax(this_size, max_size);
      this_alignment = alignof(uint32_t);
      max_alignment = fmax(this_alignment, max_alignment);
      {
        format++;
        if (*format != 'u' && *format != 's')
          goto exit_error;
        format++;
        if (*format != 'i')
          goto exit_error;
        this_size = sizeof(uint32_t);
        max_size = fmax(this_size, max_size);
        this_alignment = alignof(uint32_t);
        max_alignment = fmax(this_alignment, max_alignment);
        if (!packed && current_offset % this_alignment != 0) {
          pad = this_alignment - (current_offset % this_alignment);
          mc += pad;
          current_offset += pad;
        }
        mc++;
        break;
      }
    case '6':
      this_size = sizeof(uint64_t);
      max_size = fmax(this_size, max_size);
      this_alignment = alignof(uint64_t);
      max_alignment = fmax(this_alignment, max_alignment);
      {
        format++;
        if (*format != 'u' && *format != 's')
          goto exit_error;
        format++;
        if (*format != 'i')
          goto exit_error;
        this_size = sizeof(uint64_t);
        max_size = fmax(this_size, max_size);
        this_alignment = alignof(uint64_t);
        max_alignment = fmax(this_alignment, max_alignment);
        if (!packed && current_offset % this_alignment != 0) {
          pad = this_alignment - (current_offset % this_alignment);
          mc += pad;
          current_offset += pad;
        }
        mc++;
        break;
      }
    case '<': {
      int size, length;
      int res = sscanf(format, "<%i>%n", &size, &length);
      format += length;
      format--;
      if (res != 1)
        goto exit_error;
      this_size = 0;
      pad = size;
      mc += pad;
      current_offset += pad;
      break;
    }
    case '{': {
      pad = 0;
      union_opened++;
      size_t inner_size;
      struct_member_desc_t s =
          ctr_ffi_type_get_member_count(format + 1, &inner_size, 0, 1);
      this_size = s.max_size;
      max_size = fmax(this_size, max_size);
      this_alignment = s.max_alignment;
      max_alignment = fmax(this_alignment, max_alignment);
      if (!packed && current_offset % this_alignment != 0) {
        pad = this_alignment - (current_offset % this_alignment);
        mc += pad;
        current_offset += pad;
      }
      while (*format != '}') {
        if (*format == '\0')
          goto exit_error;
        format++;
      }
      mc += s.member_count;
      element_count = s.member_count - pad;
      break;
    }
    case '[': {
      pad = 0;
      struct_opened++;
      size_t inner_size;
      struct_member_desc_t s =
          ctr_ffi_type_get_member_count(format + 1, &inner_size, 0, 0);
      this_size = inner_size;
      max_size = fmax(this_size, max_size);
      this_alignment = s.max_alignment;
      max_alignment = fmax(this_alignment, max_alignment);
      if (!packed && current_offset % this_alignment != 0) {
        pad = this_alignment - (current_offset % this_alignment);
        mc += pad;
        current_offset += pad;
      }
      while (*format != ']') {
        if (*format == '\0')
          goto exit_error;
        format++;
      }
      mc++;
      break;
    }
    case 'u': {
      format++;
      switch (*format) {
      case 'c':
        this_size = sizeof(unsigned char);
        max_size = fmax(this_size, max_size);
        this_alignment = alignof(unsigned char);
        max_alignment = fmax(this_alignment, max_alignment);
        pad = 0;
        {
          mc++;
          break;
        } // no padding for chars
      case 's':
        this_size = sizeof(unsigned short);
        max_size = fmax(this_size, max_size);
        this_alignment = alignof(unsigned short);
        max_alignment = fmax(this_alignment, max_alignment);
        if (!packed && current_offset % this_alignment != 0) {
          pad = this_alignment - (current_offset % this_alignment);
          mc += pad;
          current_offset += pad;
        }
        {
          mc++;
          break;
        }
      case 'i':
        this_size = sizeof(unsigned int);
        max_size = fmax(this_size, max_size);
        this_alignment = alignof(unsigned int);
        max_alignment = fmax(this_alignment, max_alignment);
        if (!packed && current_offset % this_alignment != 0) {
          pad = this_alignment - (current_offset % this_alignment);
          mc += pad;
          current_offset += pad;
        }
        {
          mc++;
          break;
        }
      case 'l':
        this_size = sizeof(unsigned long);
        max_size = fmax(this_size, max_size);
        this_alignment = alignof(unsigned long);
        max_alignment = fmax(this_alignment, max_alignment);
        if (!packed && current_offset % this_alignment != 0) {
          pad = this_alignment - (current_offset % this_alignment);
          mc += pad;
          current_offset += pad;
        }
        {
          mc++;
          break;
        }
      default:
        goto exit_error;
      }
      break;
    }
    case 's': {
      format++;
      switch (*format) {
      case 'c':
        this_size = sizeof(unsigned char);
        max_size = fmax(this_size, max_size);
        this_alignment = alignof(unsigned char);
        max_alignment = fmax(this_alignment, max_alignment);
        pad = 0;
        {
          mc++;
          break;
        }
      case 's':
        this_size = sizeof(signed short);
        max_size = fmax(this_size, max_size);
        this_alignment = alignof(signed short);
        max_alignment = fmax(this_alignment, max_alignment);
        if (!packed && current_offset % this_alignment != 0) {
          pad = this_alignment - (current_offset % this_alignment);
          mc += pad;
          current_offset += pad;
        }
        {
          mc++;
          break;
        }
      case 'i':
        this_size = sizeof(signed int);
        max_size = fmax(this_size, max_size);
        this_alignment = alignof(signed int);
        max_alignment = fmax(this_alignment, max_alignment);
        if (!packed && current_offset % this_alignment != 0) {
          pad = this_alignment - (current_offset % this_alignment);
          mc += pad;
          current_offset += pad;
        }
        {
          mc++;
          break;
        }
      case 'l':
        this_size = sizeof(signed long);
        max_size = fmax(this_size, max_size);
        this_alignment = alignof(signed long);
        max_alignment = fmax(this_alignment, max_alignment);
        if (!packed && current_offset % this_alignment != 0) {
          pad = this_alignment - (current_offset % this_alignment);
          mc += pad;
          current_offset += pad;
        }
        {
          mc++;
          break;
        }
      default:
        goto exit_error;
      }
      break;
    }
    default:
      goto exit_error;
    }
#ifdef TEST
    printf("current offset %d, current type size %d, current alignment %d\n",
           current_offset, this_size, this_alignment);
#endif
    if (record_pads) {
      for (size_t p = 0; p < pad; p++) {
        if (padinfo_index >= padinfo_max) {
          padinfo_max *= 2;
          padinfo = ctr_heap_reallocate(padinfo, sizeof(pad_info_node_t *) *
                                                     padinfo_max);
        }
        pad_info_node_t *newinfo = ctr_heap_allocate(sizeof(pad_info_node_t));
        newinfo->pad = 1;
        newinfo->offset = current_offset - p + pad;
        padinfo[padinfo_index++] = newinfo;
      }
      for (int i = 0; i < element_count; i++) {
        pad_info_node_t *newinfo = ctr_heap_allocate(sizeof(pad_info_node_t));
        newinfo->pad = 0;
        newinfo->offset = current_offset;
        if (padinfo_index >= padinfo_max) {
          padinfo_max *= 2;
          padinfo = ctr_heap_reallocate(padinfo, sizeof(pad_info_node_t *) *
                                                     padinfo_max);
        }
        padinfo[padinfo_index++] = newinfo;
      }
    }
    current_offset += this_size;
    format++;
    pad = 0;
  }
  if (!packed && current_offset % max_alignment != 0) {
    pad = max_alignment - (current_offset % max_alignment);
    mc += pad;
    current_offset += pad;
  }
  if (record_pads) {
    for (size_t p = 0; p < pad; p++) {
      if (padinfo_index >= padinfo_max) {
        padinfo_max *= 2;
        padinfo =
            ctr_heap_reallocate(padinfo, sizeof(pad_info_node_t) * padinfo_max);
      }
      pad_info_node_t *newinfo = ctr_heap_allocate(sizeof(pad_info_node_t));
      newinfo->pad = 1;
      newinfo->offset = current_offset - p + pad;
      padinfo[padinfo_index++] = newinfo;
    }
  }
  if (record_pads) {
    pad_info_node_t *newinfo = ctr_heap_allocate(sizeof(pad_info_node_t));
    newinfo->pad = 0;
    newinfo->offset = current_offset;
    if (padinfo_index >= padinfo_max) {
      padinfo_max *= 2;
      padinfo =
          ctr_heap_reallocate(padinfo, sizeof(pad_info_node_t *) * padinfo_max);
    }
    padinfo[padinfo_index++] = newinfo;
    if (padinfo_index >= padinfo_max) {
      padinfo_max *= 2;
      padinfo =
          ctr_heap_reallocate(padinfo, sizeof(pad_info_node_t *) * padinfo_max);
    }
    padinfo[padinfo_index++] = NULL; // null terminate so it can be safely read.
  }
  *size_out = current_offset;

  ret.member_count = mc;
  ret.max_alignment = max_alignment;
  ret.max_size = max_size;
  ret.pad_structure = padinfo;
  return ret;

exit_error:;
  ret.member_count = -((int)(format - beginning + 1));
  return ret;
}

wrapped_ffi_type *ctr_create_ffi_type_descriptor(char *format, int union_) {
  ctr_struct_initialize_internal();
  size_t size;
  struct_member_desc_t desc =
      ctr_ffi_type_get_member_count(format, &size, 0, union_);
  return ctr_create_ffi_type_descriptor_(format, desc.member_count, union_);
}

wrapped_ffi_type *
ctr_create_ffi_type_descriptor_(char *format, int member_count, int union_) {
  ctr_struct_initialize_internal();
  wrapped_ffi_type *new_type = ctr_heap_allocate(sizeof(wrapped_ffi_type));
  new_type->size = 0;
  new_type->alignment = 0;
  new_type->type = FFI_TYPE_STRUCT;
  new_type->extension_data =
      WRAPPED_FFI_TYPE_MAGIC | (union_ ? WRAPPED_FFI_TYPE_UNION : 0);
  if (member_count < 0) {
    char err[512];
    int len = sprintf(
        err,
        "Struct: Error at char %d of format string '%s' :: Unrecognized format",
        -member_count - 1, format);
#ifdef TEST
    perror(err);
    exit(1);
#else
    CtrStdFlow = ctr_build_string(err, len);
    return NULL;
#endif // TEST
  }
  wrapped_ffi_type **elems = ctr_heap_allocate(
      sizeof(wrapped_ffi_type *) *
      (member_count + 1)); // plus one for the terminating NULL
  size_t current_offset = 0;
  ssize_t this_size = 0;
  size_t alignment = 0;
  char *fff = format;
  for (int i = 0; i < member_count; i++) {
    wrapped_ffi_type *member =
        ctr_ffi_type_get_format_splat(&format, &this_size);
    alignment = fmax(alignment, member->alignment);
    if (this_size < 0) {
      size_t pad = -this_size;
      current_offset += pad;
      for (int j = 0; j < pad; j++)
        elems[i++] = &wrapped_ffi_type_uchar;
    } else {
      if (!union_ && current_offset % this_size != 0) { // no padding for unions
        size_t pad = this_size - (current_offset % this_size);
        current_offset += pad;
        for (int j = 0; j < pad; j++) {
          elems[i++] = &wrapped_ffi_type_uchar; // insert a bunch of pads
        }
      }
      elems[i] = member;
      current_offset += this_size;
    }
  }
  elems[member_count] = NULL;
  new_type->elements = (ffi_type **)elems;
  new_type->size = current_offset;
  new_type->alignment = alignment;
  return new_type;
}

// pass null for buf to get length
int ctr_create_ffi_str_descriptor(wrapped_ffi_type *type, char *buf) {
  ctr_struct_initialize_internal();
  if (type->type != FFI_TYPE_STRUCT) {
    size_t size = 0;
    if (type == &wrapped_ffi_type_void ||
        type == (wrapped_ffi_type *)&ffi_type_void) {
      if (buf != NULL)
        memcpy(buf, "v", 1);
      size += 1;
    } else if (type == &wrapped_ffi_type_sint ||
               type == (wrapped_ffi_type *)&ffi_type_sint) {
      if (buf != NULL)
        memcpy(buf, "si", 2);
      size += 2;
    } else if (type == &wrapped_ffi_type_uint ||
               type == (wrapped_ffi_type *)&ffi_type_uint) {
      if (buf != NULL)
        memcpy(buf, "ui", 2);
      size += 2;
    } else if (type == &wrapped_ffi_type_sshort ||
               type == (wrapped_ffi_type *)&ffi_type_sshort) {
      if (buf != NULL)
        memcpy(buf, "ss", 2);
      size += 2;
    } else if (type == &wrapped_ffi_type_ushort ||
               type == (wrapped_ffi_type *)&ffi_type_ushort) {
      if (buf != NULL)
        memcpy(buf, "us", 2);
      size += 2;
    } else if (type == &wrapped_ffi_type_slong ||
               type == (wrapped_ffi_type *)&ffi_type_slong) {
      if (buf != NULL)
        memcpy(buf, "sl", 2);
      size += 2;
    } else if (type == &wrapped_ffi_type_ulong ||
               type == (wrapped_ffi_type *)&ffi_type_ulong) {
      if (buf != NULL)
        memcpy(buf, "ul", 2);
      size += 2;
    } else if (type == &wrapped_ffi_type_schar ||
               type == (wrapped_ffi_type *)&ffi_type_schar) {
      if (buf != NULL)
        memcpy(buf, "sc", 2);
      size += 2;
    } else if (type == &wrapped_ffi_type_uchar ||
               type == (wrapped_ffi_type *)&ffi_type_uchar) {
      if (buf != NULL)
        memcpy(buf, "uc", 2);
      size += 2;
    } else if (type == &wrapped_ffi_type_pointer ||
               type == (wrapped_ffi_type *)&ffi_type_pointer) {
      if (buf != NULL)
        memcpy(buf, "p", 1);
      size += 1;
    } else if (type == &wrapped_ffi_type_float ||
               type == (wrapped_ffi_type *)&ffi_type_float) {
      if (buf != NULL)
        memcpy(buf, "f", 1);
      size += 1;
    } else if (type == &wrapped_ffi_type_double ||
               type == (wrapped_ffi_type *)&ffi_type_double) {
      if (buf != NULL)
        memcpy(buf, "d", 1);
      size += 1;
    } else if (type == &wrapped_ffi_type_sint8 ||
               type == (wrapped_ffi_type *)&ffi_type_sint8) {
      if (buf != NULL)
        memcpy(buf, "3si", 3);
      size += 3;
    } else if (type == &wrapped_ffi_type_uint8 ||
               type == (wrapped_ffi_type *)&ffi_type_uint8) {
      if (buf != NULL)
        memcpy(buf, "3ui", 3);
      size += 3;
    } else if (type == &wrapped_ffi_type_sint16 ||
               type == (wrapped_ffi_type *)&ffi_type_sint16) {
      if (buf != NULL)
        memcpy(buf, "4si", 3);
      size += 3;
    } else if (type == &wrapped_ffi_type_sint32 ||
               type == (wrapped_ffi_type *)&ffi_type_sint32) {
      if (buf != NULL)
        memcpy(buf, "5si", 3);
      size += 3;
    } else if (type == &wrapped_ffi_type_sint64 ||
               type == (wrapped_ffi_type *)&ffi_type_sint64) {
      if (buf != NULL)
        memcpy(buf, "6si", 3);
      size += 3;
    } else if (type == &wrapped_ffi_type_uint16 ||
               type == (wrapped_ffi_type *)&ffi_type_uint16) {
      if (buf != NULL)
        memcpy(buf, "4ui", 3);
      size += 3;
    } else if (type == &wrapped_ffi_type_uint32 ||
               type == (wrapped_ffi_type *)&ffi_type_uint32) {
      if (buf != NULL)
        memcpy(buf, "5ui", 3);
      size += 3;
    } else if (type == &wrapped_ffi_type_uint64 ||
               type == (wrapped_ffi_type *)&ffi_type_uint64) {
      if (buf != NULL)
        memcpy(buf, "6ui", 3);
      size += 3;
    } else if (type == &wrapped_ffi_type_longdouble ||
               type == (wrapped_ffi_type *)&ffi_type_longdouble) {
      if (buf != NULL)
        memcpy(buf, "l", 1);
      size += 1;
    }
    return size;
  }
  ffi_type **elems = type->elements;
  int size = 0;
  int imm_size = 0;
  for (int i = 0; elems[i] != NULL; i++) {
    if (elems[i] == (ffi_type *)&wrapped_ffi_type_void ||
        elems[i] == &ffi_type_void) {
      if (buf != NULL)
        memcpy(buf, "v", 1);
      size += 1;
    } else if (elems[i] == (ffi_type *)&wrapped_ffi_type_sint ||
               elems[i] == &ffi_type_sint) {
      if (buf != NULL)
        memcpy(buf, "si", 2);
      size += 2;
    } else if (elems[i] == (ffi_type *)&wrapped_ffi_type_uint ||
               elems[i] == &ffi_type_uint) {
      if (buf != NULL)
        memcpy(buf, "ui", 2);
      size += 2;
    } else if (elems[i] == (ffi_type *)&wrapped_ffi_type_sshort ||
               elems[i] == &ffi_type_sshort) {
      if (buf != NULL)
        memcpy(buf, "ss", 2);
      size += 2;
    } else if (elems[i] == (ffi_type *)&wrapped_ffi_type_ushort ||
               elems[i] == &ffi_type_ushort) {
      if (buf != NULL)
        memcpy(buf, "us", 2);
      size += 2;
    } else if (elems[i] == (ffi_type *)&wrapped_ffi_type_slong ||
               elems[i] == &ffi_type_slong) {
      if (buf != NULL)
        memcpy(buf, "sl", 2);
      size += 2;
    } else if (elems[i] == (ffi_type *)&wrapped_ffi_type_ulong ||
               elems[i] == &ffi_type_ulong) {
      if (buf != NULL)
        memcpy(buf, "ul", 2);
      size += 2;
    } else if (elems[i] == (ffi_type *)&wrapped_ffi_type_schar ||
               elems[i] == &ffi_type_schar) {
      if (buf != NULL)
        memcpy(buf, "sc", 2);
      size += 2;
    } else if (elems[i] == (ffi_type *)&wrapped_ffi_type_uchar ||
               elems[i] == &ffi_type_uchar) {
      if (buf != NULL)
        memcpy(buf, "uc", 2);
      size += 2;
    } else if (elems[i] == (ffi_type *)&wrapped_ffi_type_pointer ||
               elems[i] == &ffi_type_pointer) {
      if (buf != NULL)
        memcpy(buf, "p", 1);
      size += 1;
    } else if (elems[i] == (ffi_type *)&wrapped_ffi_type_float ||
               elems[i] == &ffi_type_float) {
      if (buf != NULL)
        memcpy(buf, "f", 1);
      size += 1;
    } else if (elems[i] == (ffi_type *)&wrapped_ffi_type_double ||
               elems[i] == &ffi_type_double) {
      if (buf != NULL)
        memcpy(buf, "d", 1);
      size += 1;
    } else if (elems[i] == (ffi_type *)&wrapped_ffi_type_sint8 ||
               elems[i] == &ffi_type_sint8) {
      if (buf != NULL)
        memcpy(buf, "3si", 3);
      size += 3;
    } else if (elems[i] == (ffi_type *)&wrapped_ffi_type_uint8 ||
               elems[i] == &ffi_type_uint8) {
      if (buf != NULL)
        memcpy(buf, "3ui", 3);
      size += 3;
    } else if (elems[i] == (ffi_type *)&wrapped_ffi_type_sint16 ||
               elems[i] == &ffi_type_sint16) {
      if (buf != NULL)
        memcpy(buf, "4si", 3);
      size += 3;
    } else if (elems[i] == (ffi_type *)&wrapped_ffi_type_sint32 ||
               elems[i] == &ffi_type_sint32) {
      if (buf != NULL)
        memcpy(buf, "5si", 3);
      size += 3;
    } else if (elems[i] == (ffi_type *)&wrapped_ffi_type_sint64 ||
               elems[i] == &ffi_type_sint64) {
      if (buf != NULL)
        memcpy(buf, "6si", 3);
      size += 3;
    } else if (elems[i] == (ffi_type *)&wrapped_ffi_type_uint16 ||
               elems[i] == &ffi_type_uint16) {
      if (buf != NULL)
        memcpy(buf, "4ui", 3);
      size += 3;
    } else if (elems[i] == (ffi_type *)&wrapped_ffi_type_uint32 ||
               elems[i] == &ffi_type_uint32) {
      if (buf != NULL)
        memcpy(buf, "5ui", 3);
      size += 3;
    } else if (elems[i] == (ffi_type *)&wrapped_ffi_type_uint64 ||
               elems[i] == &ffi_type_uint64) {
      if (buf != NULL)
        memcpy(buf, "6ui", 3);
      size += 3;
    } else if (elems[i] == (ffi_type *)&wrapped_ffi_type_longdouble ||
               elems[i] == &ffi_type_longdouble) {
      if (buf != NULL)
        memcpy(buf, "l", 1);
      size += 1;
    } else if (elems[i]->type == FFI_TYPE_STRUCT) {
      int is_wrapped = IS_WRAPPED(elems[i]);
      int is_union =
          is_wrapped && (WRAP_DATA(elems[i]) == WRAPPED_FFI_TYPE_UNION);
      if (buf != NULL)
        memcpy(buf, &"[{"[is_union], 1);
      int skip = ctr_create_ffi_str_descriptor((wrapped_ffi_type *)elems[i],
                                               buf != NULL ? buf + 1 : NULL);
      if (buf != NULL)
        memcpy(buf + skip + 1, &"]}"[is_union], 1);
      size += skip + 2;
    } else
      return -1;
    if (buf != NULL)
      buf += size - imm_size;
    imm_size = size;
  }
  return size;
}

#ifdef TEST
struct test_str {
  double a;
  long long b;
  void *c;
  char d;
  union {
    char e;
    int f;
  };
};
int main(void) {
  char *fmt = "dlpsc{scsi}";
  size_t size;
  struct_member_desc_t desc = ctr_ffi_type_get_member_count(fmt, &size, 1, 0);
  int count = desc.member_count;
  if (count < 0) {
    printf("Error at %d\n", count);
    return 0;
  }
  pad_info_node_t **pads = desc.pad_structure;
  int i = 0;
  while (pads[i]) {
    printf("member number %d at %lu is %sa pad.\n", i, pads[i]->offset,
           pads[i]->pad ? "" : "not ");
    ctr_heap_free(pads[i]);
    i++;
  }
  ctr_heap_free(desc.pad_structure);
  printf("member count of %s: %d, size (calc): %lu, size (real): %lu\n", fmt,
         count, size, sizeof(struct test_str));
  printf("layout:\n a: %lu\n b: %lu\n c: %lu\n d: %lu\n e: %lu\n f: %lu\n",
         offsetof(struct test_str, a), offsetof(struct test_str, b),
         offsetof(struct test_str, c), offsetof(struct test_str, d),
         offsetof(struct test_str, e), offsetof(struct test_str, f));
  // ffi_type* type = ctr_create_ffi_type_descriptor(fmt);
  // free(type);
  return 0;
}
#endif // TEST
