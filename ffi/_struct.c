//#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include <ffi.h>
#include "_struct.h"
#include <math.h>
// #include "structmember.h"

#define TEST
#undef TEST

#ifndef TEST
#include "../citron.h"
#else
#include <stddef.h>
#define ctr_heap_allocate malloc
#endif

/*
    v      ffi_type_void
    3ui    ffi_type_uint8
    3si    ffi_type_sint8
    4ui    ffi_type_uint16
    4si    ffi_type_sint16
    5ui    ffi_type_uint32
    5si    ffi_type_sint32
    6ui    ffi_type_uint64
    6si    ffi_type_sint64
    uc     ffi_type_uchar
    sc     ffi_type_schar
    us     ffi_type_ushort
    ss     ffi_type_sshort
    ui     ffi_type_uint
    si     ffi_type_sint
    ul     ffi_type_ulong
    sl     ffi_type_slong
    f      ffi_type_float
    d      ffi_type_double
    p      ffi_type_pointer
    l      ffi_type_longdouble
    [...]  new struct def
 */
ffi_type* ctr_ffi_type_get_format_splat(char* *format, size_t* this_size) {
  switch(**format) {
    //case 'v': *this_size = sizeof(void); return &ffi_type_void;
    case '3': {
      (*format)++;
      switch(**format) {
        case 'u': {
          (*format)++;
          if(**format!='i') return NULL;
          *this_size = sizeof(uint8_t);
          return &ffi_type_uint8;
        }
        case 's':{
          (*format)++;
          if(**format!='i') return NULL;
          *this_size = sizeof(int8_t);
          return &ffi_type_sint8;
        }
        default: return NULL;
      }
    }
    case '4': {
      (*format)++;
      switch(**format) {
        case 'u': {
          (*format)++;
          if(**format!='i') return NULL;
          *this_size = sizeof(uint16_t);
          return &ffi_type_uint16;
        }
        case 's':{
          (*format)++;
          if(**format!='i') return NULL;
          *this_size = sizeof(int16_t);
          return &ffi_type_sint16;
        }
        default: return NULL;
      }
    }
    case '5': {
      (*format)++;
      switch(**format) {
        case 'u': {
          (*format)++;
          if(**format!='i') return NULL;
          *this_size = sizeof(uint32_t);
          return &ffi_type_uint32;
        }
        case 's':{
          (*format)++;
          if(**format!='i') return NULL;
          *this_size = sizeof(int32_t);
          return &ffi_type_sint32;
        }
        default: return NULL;
      }
    }
    case '6': {
      (*format)++;
      switch(**format) {
        case 'u': {
          (*format)++;
          if(**format!='i') return NULL;
          *this_size = sizeof(uint64_t);
          return &ffi_type_uint64;
        }
        case 's':{
          (*format)++;
          if(**format!='i') return NULL;
          *this_size = sizeof(int64_t);
          return &ffi_type_sint64;
        }
        default: return NULL;
      }
    }
    case 'f': *this_size = sizeof(float); return &ffi_type_float;
    case 'd': *this_size = sizeof(double); return &ffi_type_double;
    case 'p': *this_size = sizeof(void*); return &ffi_type_pointer;
    case 'l': *this_size = sizeof(long long); return &ffi_type_longdouble;
    case 'u': {
      (*format)++;
      switch(**format) {
        case 'c': *this_size = sizeof(unsigned char); return &ffi_type_uchar;
        case 's': *this_size = sizeof(unsigned short); return &ffi_type_ushort;
        case 'i': *this_size = sizeof(unsigned int); return &ffi_type_uint;
        case 'l': *this_size = sizeof(unsigned long); return &ffi_type_ulong;
        default: return NULL;
      }
    }
    case 's': {
      (*format)++;
      switch(**format) {
        case 'c': *this_size = sizeof(signed char); return &ffi_type_schar;
        case 's': *this_size = sizeof(signed short); return &ffi_type_sshort;
        case 'i': *this_size = sizeof(signed int); return &ffi_type_sint;
        case 'l': *this_size = sizeof(signed long); return &ffi_type_slong;
        default: return NULL;
      }
    }
    case '[': {
      (*format)++;
      char* fmt = *format;
      ctr_ffi_type_get_format_splat(&fmt, this_size);
      return ctr_create_ffi_type_descriptor(*format);
    }
    case ']':
    default : return NULL;
  }
}

int ctr_ffi_type_struct_sizeof(char* format) {//XXX: does not generate padding bytes
  int size = 0;
  char* beginning = format;
  int struct_opened = 1; //we are in a struct. a stray ']' will terminate this
  while(struct_opened>0 && *format!='\0') {
    switch(*format) {
      case ']': struct_opened--; break;
      case 'f': size += sizeof(float); break;
      case 'd': size += sizeof(double); break;
      case 'p': size += sizeof(void*); break;
      case 'l': size += sizeof(long long); break;
      //case 'v': size += sizeof(void); break;
      case '3': size += sizeof(uint8_t); break;
      case '4': size += sizeof(uint16_t); break;
      case '5': size += sizeof(uint32_t); break;
      case '6': size += sizeof(uint64_t); break;
      case '[': {
        struct_opened++; //so that we don't terminate after this struct
        int s = ctr_ffi_type_struct_sizeof(format+1);
        if(s<0) return s-(format-beginning);
        size += s;
        break;
      }
      case 'u': {
        format++;
        switch(*format) {
          case 'c': size += sizeof(unsigned char); break;
          case 's': size += sizeof(unsigned short); break;
          case 'i': size += sizeof(unsigned int); break;
          case 'l': size += sizeof(unsigned long); break;
          default: return -((int)(format-beginning+1));
        }
        break;
      }
      case 's': {
        format++;
        switch(*format) {
          case 'c': size += sizeof(signed char); break;
          case 's': size += sizeof(signed short); break;
          case 'i': size += sizeof(signed int); break;
          case 'l': size += sizeof(signed long); break;
          default: return -((int)(format-beginning+1));
        }
        break;
      }
      default: return -((int)(format-beginning+1));
    }
    format++;
  }
  return size;
}

struct_member_desc_t ctr_ffi_type_get_member_count(char* format, size_t* size_out, int record_pads) {
  int mc = 0;
  char* beginning = format;
  int struct_opened = 1;
  size_t current_offset = 0;
  size_t this_size = 0;
  size_t this_alignment = 0;
  size_t max_alignment = 0;
  size_t pad = 0;
  size_t padinfo_max = 8;
  size_t padinfo_index = 0;
  pad_info_node_t** padinfo;
  struct_member_desc_t ret;
  if (record_pads) padinfo=ctr_heap_allocate(sizeof(pad_info_node_t*)*padinfo_max);
  while(*format!='\0') {
    switch(*format) {
      case ']': struct_opened--; pad=0; break;
      case 'f': this_size = sizeof(float); this_alignment = alignof(float); max_alignment = fmax(this_alignment, max_alignment); if(current_offset%this_alignment!=0) { pad = this_alignment-(current_offset%this_alignment); mc+=pad; current_offset+=pad;} mc++; break;
      case 'd': this_size = sizeof(double); this_alignment = alignof(double); max_alignment = fmax(this_alignment, max_alignment); if(current_offset%this_alignment!=0) { pad = this_alignment-(current_offset%this_alignment); mc+=pad; current_offset+=pad;} mc++; break;
      case 'p': this_size = sizeof(void*); this_alignment = alignof(void*); max_alignment = fmax(this_alignment, max_alignment); if(current_offset%this_alignment!=0) { pad = this_alignment-(current_offset%this_alignment); mc+=pad; current_offset+=pad;} mc++; break;
      case 'l': this_size = sizeof(long long); this_alignment = alignof(long long); max_alignment = fmax(this_alignment, max_alignment);if(current_offset%this_alignment!=0) { pad = this_alignment-(current_offset%this_alignment); mc+=pad; current_offset+=pad;} mc++; break;
      //case 'v': this_size = sizeof(void); this_alignment = alignof(void); max_alignment = fmax(this_alignment, max_alignment); if(current_offset%this_alignment!=0) { pad = this_alignment-(current_offset%this_alignment); mc+=pad; current_offset+=pad;} mc++; break;
      case '3': this_size = sizeof(uint8_t); this_alignment = alignof(uint8_t); max_alignment = fmax(this_alignment, max_alignment); {
        format++;
        if(*format != 'u' && *format != 's') goto exit_error;
        format++;
        if(*format != 'i') goto exit_error;
        this_size = sizeof(uint8_t); this_alignment = alignof(uint8_t); max_alignment = fmax(this_alignment, max_alignment);if(current_offset%this_alignment!=0) { pad = this_alignment-(current_offset%this_alignment); mc+=pad; current_offset+=pad;}
        mc++;
        break;
      }
      case '4': this_size = sizeof(uint16_t); this_alignment = alignof(uint16_t); max_alignment = fmax(this_alignment, max_alignment); {
        format++;
        if(*format != 'u' && *format != 's') goto exit_error;
        format++;
        if(*format != 'i') goto exit_error;
        this_size = sizeof(uint32_t); this_alignment = alignof(uint32_t); max_alignment = fmax(this_alignment, max_alignment);if(current_offset%this_alignment!=0) { pad = this_alignment-(current_offset%this_alignment); mc+=pad; current_offset+=pad;}
        mc++;
        break;
      }
      case '5': this_size = sizeof(uint32_t); this_alignment = alignof(uint32_t); max_alignment = fmax(this_alignment, max_alignment); {
        format++;
        if(*format != 'u' && *format != 's') goto exit_error;
        format++;
        if(*format != 'i') goto exit_error;
        this_size = sizeof(uint32_t); this_alignment = alignof(uint32_t); max_alignment = fmax(this_alignment, max_alignment);if(current_offset%this_alignment!=0) { pad = this_alignment-(current_offset%this_alignment); mc+=pad; current_offset+=pad;}
        mc++;
        break;
      }
      case '6': this_size = sizeof(uint64_t); this_alignment = alignof(uint64_t); max_alignment = fmax(this_alignment, max_alignment); {
        format++;
        if(*format != 'u' && *format != 's') goto exit_error;
        format++;
        if(*format != 'i') goto exit_error;
        this_size = sizeof(uint64_t); this_alignment = alignof(uint64_t); max_alignment = fmax(this_alignment, max_alignment);if(current_offset%this_alignment!=0) { pad = this_alignment-(current_offset%this_alignment); mc+=pad; current_offset+=pad;}
        mc++;
        break;
      }
      case '[': {
        pad=0;
        struct_opened++;
        size_t inner_size;
        struct_member_desc_t s = ctr_ffi_type_get_member_count(format+1, &inner_size, 0);
        this_size = inner_size;
        this_alignment = s.max_alignment;
        max_alignment = fmax(this_alignment, max_alignment);if(current_offset%this_alignment!=0) { pad = this_alignment-(current_offset%this_alignment); mc+=pad; current_offset+=pad;}
        while(*format!=']') {
          format++;
          if(*format=='\0') goto exit_error;
        }
        format++;
        mc++;
        break;
      }
      case 'u': {
        format++;
        switch(*format) {
          case 'c': this_size = sizeof(unsigned char); this_alignment = alignof(unsigned char); max_alignment = fmax(this_alignment, max_alignment); pad=0; {mc++; break;}//no padding for chars
          case 's': this_size = sizeof(unsigned short); this_alignment = alignof(unsigned short); max_alignment = fmax(this_alignment, max_alignment);if(current_offset%this_alignment!=0) { pad = this_alignment-(current_offset%this_alignment); mc+=pad; current_offset+=pad;} {mc++; break;}
          case 'i': this_size = sizeof(unsigned int); this_alignment = alignof(unsigned int); max_alignment = fmax(this_alignment, max_alignment);if(current_offset%this_alignment!=0) { pad = this_alignment-(current_offset%this_alignment); mc+=pad; current_offset+=pad;} {mc++; break;}
          case 'l': this_size = sizeof(unsigned long); this_alignment = alignof(unsigned long); max_alignment = fmax(this_alignment, max_alignment);if(current_offset%this_alignment!=0) { pad = this_alignment-(current_offset%this_alignment); mc+=pad; current_offset+=pad;} {mc++; break;}
          default: goto exit_error;
        }
        break;
      }
      case 's': {
        format++;
        switch(*format) {
          case 'c': this_size = sizeof(unsigned char); this_alignment = alignof(unsigned char); max_alignment = fmax(this_alignment, max_alignment); pad=0; {mc++; break;}
          case 's': this_size = sizeof(signed short); this_alignment = alignof(signed short); max_alignment = fmax(this_alignment, max_alignment);if(current_offset%this_alignment!=0) { pad = this_alignment-(current_offset%this_alignment); mc+=pad; current_offset+=pad;} {mc++; break;}
          case 'i': this_size = sizeof(signed int); this_alignment = alignof(signed int); max_alignment = fmax(this_alignment, max_alignment);if(current_offset%this_alignment!=0) { pad = this_alignment-(current_offset%this_alignment); mc+=pad; current_offset+=pad;} {mc++; break;}
          case 'l': this_size = sizeof(signed long); this_alignment = alignof(signed long); max_alignment = fmax(this_alignment, max_alignment);if(current_offset%this_alignment!=0) { pad = this_alignment-(current_offset%this_alignment); mc+=pad; current_offset+=pad;} {mc++; break;}
          default: goto exit_error;
        }
        break;
      }
      default: goto exit_error;
    }
    #ifdef TEST
    printf("current offset %d, current type size %d, current alignment %d\n", current_offset, this_size, this_alignment);
    #endif
    if (record_pads) {
      for(size_t p=0; p<pad; p++) {
        if(padinfo_index>=padinfo_max) {
          padinfo_max *= 2;
          padinfo = ctr_heap_reallocate(padinfo, sizeof(pad_info_node_t*)*padinfo_max);
        }
        pad_info_node_t* newinfo = ctr_heap_allocate(sizeof(pad_info_node_t));
        newinfo->pad = 1;
        newinfo->offset = current_offset - p + pad;
        padinfo[padinfo_index++] = newinfo;
      }
      pad_info_node_t* newinfo = ctr_heap_allocate(sizeof(pad_info_node_t));
      newinfo->pad = 0;
      newinfo->offset = current_offset;
      if(padinfo_index>=padinfo_max) {
        padinfo_max *= 2;
        padinfo = ctr_heap_reallocate(padinfo, sizeof(pad_info_node_t*)*padinfo_max);
      }
      padinfo[padinfo_index++] = newinfo;
    }
    current_offset += this_size;
    format++;
    pad = 0;
  }
  if(current_offset%max_alignment!=0) { pad = max_alignment-(current_offset%max_alignment); mc+=pad; current_offset+=pad;}
  if (record_pads) {
    for(size_t p=0; p<pad; p++) {
      if(padinfo_index>=padinfo_max) {
        padinfo_max *= 2;
        padinfo = ctr_heap_reallocate(padinfo, sizeof(pad_info_node_t)*padinfo_max);
      }
      pad_info_node_t* newinfo = ctr_heap_allocate(sizeof(pad_info_node_t));
      newinfo->pad = 1;
      newinfo->offset = current_offset - p + pad;
      padinfo[padinfo_index++] = newinfo;
    }
  }
  if (record_pads) {
    pad_info_node_t* newinfo = ctr_heap_allocate(sizeof(pad_info_node_t));
    newinfo->pad = 0;
    newinfo->offset = current_offset;
    if(padinfo_index>=padinfo_max) {
      padinfo_max *= 2;
      padinfo = ctr_heap_reallocate(padinfo, sizeof(pad_info_node_t*)*padinfo_max);
    }
    padinfo[padinfo_index++] = newinfo;
    if(padinfo_index>=padinfo_max) {
      padinfo_max *= 2;
      padinfo = ctr_heap_reallocate(padinfo, sizeof(pad_info_node_t*)*padinfo_max);
    }
    padinfo[padinfo_index++] = NULL;  //null terminate so it can be safely read.
  }
  *size_out = current_offset;

  ret.member_count = mc;
  ret.max_alignment = max_alignment;
  ret.pad_structure = padinfo;
  return ret;

  exit_error:;
    ret.member_count = -((int)(format-beginning+1));
    return ret;
}

ffi_type* ctr_create_ffi_type_descriptor(char* format) {
  size_t size;
  struct_member_desc_t desc = ctr_ffi_type_get_member_count(format, &size, 0);
  return ctr_create_ffi_type_descriptor_(format, desc.member_count);
}
ffi_type* ctr_create_ffi_type_descriptor_(char* format, int member_count) {
  ffi_type* new_type = ctr_heap_allocate(sizeof(ffi_type));
  new_type->size = 0;
  new_type->alignment = 0;
  new_type->type = FFI_TYPE_STRUCT;
  if(member_count<0) {
    char err[512];
    int len = sprintf(err, "Struct: Error at char %d of format string '%s' :: Unrecognized format", -member_count-1, format);
    #ifdef TEST
    perror(err);
    exit(1);
    #else
    CtrStdFlow = ctr_build_string(err, len);
    return NULL;
    #endif //TEST
  }
  ffi_type** elems = ctr_heap_allocate(sizeof(ffi_type*) * (member_count+1)); //plus one for the terminating NULL
  size_t current_offset = 0;
  size_t this_size = 0;
  size_t this_alignment = 0;
  for(int i=0; i<member_count; i++) {
    if(0);
    ffi_type* member = ctr_ffi_type_get_format_splat(&format, &this_size);
    if(current_offset%this_size != 0) {
      size_t pad = this_size-(current_offset%this_size);
      current_offset += pad;
      for(int j=0; j<pad; j++) {
        elems[i++] = &ffi_type_uchar;//insert a bunch of pads
      }
    }
    format++;
    elems[i] = member;
    current_offset += this_size;
  }
  elems[member_count] = NULL;
  new_type->elements = elems;
  return new_type;
}


//pass null for buf to get length
int ctr_create_ffi_str_descriptor(ffi_type* type, char* buf) {
  if(type->type!=FFI_TYPE_STRUCT) return -1;//bullshit requested
  ffi_type** elems = type->elements;
  int size = 0;
  int imm_size = 0;
  for(int i=0; elems[i]!=NULL; i++) {
    if (elems[i] == &ffi_type_void) { if (buf != NULL) memcpy(buf, "v", 1); size += 1; }
    else if (elems[i] == &ffi_type_sint) { if (buf != NULL) memcpy(buf, "si", 2); size += 2;}
    else if (elems[i] == &ffi_type_uint) { if (buf != NULL) memcpy(buf, "ui", 2); size += 2;}
    else if (elems[i] == &ffi_type_sshort) { if (buf != NULL) memcpy(buf, "ss", 2); size += 2;}
    else if (elems[i] == &ffi_type_ushort) { if (buf != NULL) memcpy(buf, "us", 2); size += 2;}
    else if (elems[i] == &ffi_type_slong) { if (buf != NULL) memcpy(buf, "sl", 2); size += 2;}
    else if (elems[i] == &ffi_type_ulong) {if (buf != NULL) memcpy(buf, "ul", 2); size += 2;}
    else if (elems[i] == &ffi_type_schar) { if (buf != NULL) memcpy(buf, "sc", 2); size += 2;}
    else if (elems[i] == &ffi_type_uchar) { if (buf != NULL) memcpy(buf, "uc", 2); size += 2;}
    else if (elems[i] == &ffi_type_pointer) {if (buf != NULL) memcpy(buf, "p", 1); size += 1;}
    else if (elems[i] == &ffi_type_float) { if (buf != NULL) memcpy(buf, "f", 1); size += 1;}
    else if (elems[i] == &ffi_type_double) { if (buf != NULL) memcpy(buf, "d", 1); size += 1;}
    else if (elems[i] == &ffi_type_sint8) { if (buf != NULL) memcpy(buf, "3si", 3); size += 3;}
    else if (elems[i] == &ffi_type_uint8) { if (buf != NULL) memcpy(buf, "3ui", 3); size += 3;}
    else if (elems[i] == &ffi_type_sint16) { if (buf != NULL) memcpy(buf, "4si", 3); size += 3;}
    else if (elems[i] == &ffi_type_sint32) {if (buf != NULL) memcpy(buf, "5si", 3); size += 3;}
    else if (elems[i] == &ffi_type_sint64) {if (buf != NULL) memcpy(buf, "6si", 3); size += 3;}
    else if (elems[i] == &ffi_type_uint16) {if (buf != NULL) memcpy(buf, "4ui", 3); size += 3;}
    else if (elems[i] == &ffi_type_uint32) {if (buf != NULL) memcpy(buf, "5ui", 3); size += 3;}
    else if (elems[i] == &ffi_type_uint64) {if (buf != NULL) memcpy(buf, "6ui", 3); size += 3;}
    else if (elems[i] == &ffi_type_longdouble) {if (buf != NULL) memcpy(buf, "l", 1); size += 1;}
    else if (elems[i]->type == FFI_TYPE_STRUCT){
      if (buf != NULL) memcpy(buf, "[", 1);
      int skip = ctr_create_ffi_str_descriptor(elems[i], buf!=NULL?buf+1:NULL);
      if (buf != NULL) memcpy(buf+skip, "]", 1);
      size += skip + 2;
    }
    else return -1;
    if (buf!=NULL)
      buf += size - imm_size;
    imm_size = size;
  }
  return size;
}

#ifdef TEST
struct test_str {
  double a;
  long long b;
  void* c;
  char d;
  char e;
  int f;
};
int main(void) {
  char* fmt = "dlpscscsi";
  size_t size;
  struct_member_desc_t desc = ctr_ffi_type_get_member_count(fmt, &size, 1);
  int count = desc.member_count;
  pad_info_node_t** pads = desc.pad_structure;
  int i = 0;
  while(pads[i]) {
    printf("member number %d is %sa pad.\n", i, pads[i]->pad?"":"not ");
    ctr_heap_free(pads[i]);
    i++;
  }
  ctr_heap_free(desc->pad_structure);
  printf("member count of %s: %d, size (calc): %d, size (real): %d\n", fmt, count, size, sizeof(struct test_str));
  printf("layout:\n a: %d\n b: %d\n d: %d\n e: %d\n c: %d\n f: %d\n", offsetof(struct test_str, a), offsetof(struct test_str, b), offsetof(struct test_str, d), offsetof(struct test_str, e), offsetof(struct test_str, c), offsetof(struct test_str, f));
  // ffi_type* type = ctr_create_ffi_type_descriptor(fmt);
  // free(type);
  return 0;
}
#endif //TEST
