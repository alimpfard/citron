#include <inttypes.h>
#include <stdlib.h>
#include <stdio.h>
#include <dlfcn.h>
#include <math.h>
#include "ctypes.h"

//Common
#define CTR_CREATE_CTOBJECT(name) ctr_object* name = ctr_internal_create_object(CTR_OBJECT_TYPE_OTEX)

void ctr_ctypes_set_type(ctr_object* object, ctr_ctype type) {
  ctr_internal_object_set_property(object, ctr_build_string_from_cstring(":cType"), ctr_build_number_from_float((int)type), 0);
  object->value.rvalue = ctr_heap_allocate(sizeof(ctr_resource));
  switch(type) {
    case CTR_CTYPE_VOID: object->link = CtrStdCType_void; break;
    case CTR_CTYPE_UINT8: object->value.rvalue->ptr = ctr_heap_allocate(sizeof(uint8_t)); object->link = CtrStdCType_uint8; break;
    case CTR_CTYPE_SINT8: object->value.rvalue->ptr = ctr_heap_allocate(sizeof(int8_t)); object->link = CtrStdCType_sint8; break;
    case CTR_CTYPE_UINT16: object->value.rvalue->ptr = ctr_heap_allocate(sizeof(uint16_t)); object->link = CtrStdCType_uint16; break;
    case CTR_CTYPE_SINT16: object->value.rvalue->ptr = ctr_heap_allocate(sizeof(int16_t)); object->link = CtrStdCType_sint16; break;
    case CTR_CTYPE_UINT32: object->value.rvalue->ptr = ctr_heap_allocate(sizeof(uint32_t)); object->link = CtrStdCType_uint32; break;
    case CTR_CTYPE_SINT32: object->value.rvalue->ptr = ctr_heap_allocate(sizeof(int32_t)); object->link = CtrStdCType_sint32; break;
    case CTR_CTYPE_UINT64: object->value.rvalue->ptr = ctr_heap_allocate(sizeof(uint64_t)); object->link = CtrStdCType_uint64; break;
    case CTR_CTYPE_SINT64: object->value.rvalue->ptr = ctr_heap_allocate(sizeof(int64_t)); object->link = CtrStdCType_sint64; break;
    case CTR_CTYPE_FLOAT: object->value.rvalue->ptr = ctr_heap_allocate(sizeof(float)); object->link = CtrStdCType_float; break;
    case CTR_CTYPE_DOUBLE: object->value.rvalue->ptr = ctr_heap_allocate(sizeof(double)); object->link = CtrStdCType_double; break;
    case CTR_CTYPE_UCHAR: object->value.rvalue->ptr = ctr_heap_allocate(sizeof(unsigned char)); object->link = CtrStdCType_uchar; break;
    case CTR_CTYPE_SCHAR: object->value.rvalue->ptr = ctr_heap_allocate(sizeof(char)); object->link = CtrStdCType_schar; break;
    case CTR_CTYPE_USHORT: object->value.rvalue->ptr = ctr_heap_allocate(sizeof(unsigned short)); object->link = CtrStdCType_ushort; break;
    case CTR_CTYPE_SSHORT: object->value.rvalue->ptr = ctr_heap_allocate(sizeof(short)); object->link = CtrStdCType_sshort; break;
    case CTR_CTYPE_UINT: object->value.rvalue->ptr = ctr_heap_allocate(sizeof(unsigned int)); object->link = CtrStdCType_uint; break;
    case CTR_CTYPE_SINT: object->value.rvalue->ptr = ctr_heap_allocate(sizeof(int)); object->link = CtrStdCType_sint; break;
    case CTR_CTYPE_ULONG: object->value.rvalue->ptr = ctr_heap_allocate(sizeof(unsigned long)); object->link = CtrStdCType_ulong; break;
    case CTR_CTYPE_SLONG: object->value.rvalue->ptr = ctr_heap_allocate(sizeof(long)); object->link = CtrStdCType_slong; break;
    case CTR_CTYPE_LONGDOUBLE: object->value.rvalue->ptr = ctr_heap_allocate(sizeof(long double)); object->link = CtrStdCType_longdouble; break;
    case CTR_CTYPE_POINTER: object->link = CtrStdCType_pointer; break;
    case CTR_CTYPE_DYN_LIB: object->link = CtrStdCType_dynamic_lib; break;
    default: return;
  }
}
int ctr_is_valid_ctype(ctr_object* object) {
  return (ctr_internal_object_find_property(object, ctr_build_string_from_cstring(":cType"), 0) != NULL);
}
ctr_ctype ctr_ctypes_typeof(ctr_object* object) {
  ctr_object* ctype = (ctr_internal_object_find_property(object, ctr_build_string_from_cstring(":cType"), 0));
  if(ctype != NULL) return (ctr_ctype)(int)(ctype->value.nvalue);
  return CTR_CTYPE_INVALID;
}
ctr_ctype ctr_ctypes_rtypeof(ctr_object* object) {
  ctr_object* ctype = (ctr_internal_object_find_property(object, ctr_build_string_from_cstring(":crType"), 0));
  if(ctype != NULL) return (ctr_ctype)(int)(ctype->value.nvalue);
  return CTR_CTYPE_INVALID;
}

//Void
CTR_CT_SIMPLE_TYPE_FUNC_UNMAKE(void) {return myself;}
CTR_CT_SIMPLE_TYPE_FUNC_MAKE(void) {
  return ctr_build_nil();
}
CTR_CT_SIMPLE_TYPE_FUNC_SET(void) {
  CtrStdFlow = ctr_build_string_from_cstring("Cannot assign to void");
  return ctr_build_nil();
}
CTR_CT_SIMPLE_TYPE_FUNC_GET(void) {
  return ctr_build_nil();
}

//Unsigned Int 8
CTR_CT_SIMPLE_TYPE_FUNC_UNMAKE(uint8) {
  ctr_heap_free(myself->value.rvalue->ptr); //Free our resource, leave the rest to the GC
	return myself;
}
CTR_CT_SIMPLE_TYPE_FUNC_MAKE(uint8) {
  CTR_CREATE_CTOBJECT(object);
  ctr_ctypes_set_type(object, CTR_CTYPE_UINT8);
  return object;
}
CTR_CT_SIMPLE_TYPE_FUNC_SET(uint8) {
  *(uint8_t*)(myself->value.rvalue->ptr) = (uint8_t)(ctr_internal_cast2number(argumentList->object)->value.nvalue);
  return myself;
}
CTR_CT_SIMPLE_TYPE_FUNC_GET(uint8) {
  return ctr_build_number_from_float((double)(*((uint8_t*)(myself->value.rvalue->ptr)))); //kinda derpy in bigger values...meh will see if it derps.
  //TODO: Check if derps
}

//Signed Int 8
CTR_CT_SIMPLE_TYPE_FUNC_UNMAKE(sint8) {
  ctr_heap_free(myself->value.rvalue->ptr); //Free our resource, leave the rest to the GC
	return myself;
}
CTR_CT_SIMPLE_TYPE_FUNC_MAKE(sint8) {
  CTR_CREATE_CTOBJECT(object);
  ctr_ctypes_set_type(object, CTR_CTYPE_SINT8);
  return object;
}
CTR_CT_SIMPLE_TYPE_FUNC_SET(sint8) {
  *(int8_t*)(myself->value.rvalue->ptr) = (int8_t)(ctr_internal_cast2number(argumentList->object)->value.nvalue);
  return myself;
}
CTR_CT_SIMPLE_TYPE_FUNC_GET(sint8) {
  return ctr_build_number_from_float((double)(*((int8_t*)(myself->value.rvalue->ptr))));
}

//Unsigned Int 16
CTR_CT_SIMPLE_TYPE_FUNC_UNMAKE(uint16) {
  ctr_heap_free(myself->value.rvalue->ptr); //Free our resource, leave the rest to the GC
	return myself;
}
CTR_CT_SIMPLE_TYPE_FUNC_MAKE(uint16) {
  CTR_CREATE_CTOBJECT(object);
  ctr_ctypes_set_type(object, CTR_CTYPE_UINT16); //HALP
  return object;
}
CTR_CT_SIMPLE_TYPE_FUNC_SET(uint16) {
  *(uint16_t*)(myself->value.rvalue->ptr) = (uint16_t)(ctr_internal_cast2number(argumentList->object)->value.nvalue);
  return myself;
}
CTR_CT_SIMPLE_TYPE_FUNC_GET(uint16) {
  return ctr_build_number_from_float((double)(*((uint16_t*)(myself->value.rvalue->ptr))));
}

//Signed Int 16
CTR_CT_SIMPLE_TYPE_FUNC_UNMAKE(sint16) {
  ctr_heap_free(myself->value.rvalue->ptr); //Free our resource, leave the rest to the GC
	return myself;
}
CTR_CT_SIMPLE_TYPE_FUNC_MAKE(sint16) {
  CTR_CREATE_CTOBJECT(object);
  ctr_ctypes_set_type(object, CTR_CTYPE_SINT16); //HALP
  return object;
}
CTR_CT_SIMPLE_TYPE_FUNC_SET(sint16) {
  *(int16_t*)(myself->value.rvalue->ptr) = (int16_t)(ctr_internal_cast2number(argumentList->object)->value.nvalue);
  return myself;
}
CTR_CT_SIMPLE_TYPE_FUNC_GET(sint16) {
  return ctr_build_number_from_float((double)(*((int16_t*)(myself->value.rvalue->ptr))));
}

//Unsigned Int 32
CTR_CT_SIMPLE_TYPE_FUNC_UNMAKE(uint32) {
  ctr_heap_free(myself->value.rvalue->ptr); //Free our resource, leave the rest to the GC
	return myself;
}
CTR_CT_SIMPLE_TYPE_FUNC_MAKE(uint32) {
  CTR_CREATE_CTOBJECT(object);
  ctr_ctypes_set_type(object, CTR_CTYPE_UINT32); //HALP
  return object;
}
CTR_CT_SIMPLE_TYPE_FUNC_SET(uint32) {
  *(uint32_t*)(myself->value.rvalue->ptr) = (uint32_t)(ctr_internal_cast2number(argumentList->object)->value.nvalue);
  return myself;
}
CTR_CT_SIMPLE_TYPE_FUNC_GET(uint32) {
  return ctr_build_number_from_float((double)(*((uint32_t*)(myself->value.rvalue->ptr))));
}

//Signed Int 32
CTR_CT_SIMPLE_TYPE_FUNC_UNMAKE(sint32) {
  ctr_heap_free(myself->value.rvalue->ptr); //Free our resource, leave the rest to the GC
	return myself;
}
CTR_CT_SIMPLE_TYPE_FUNC_MAKE(sint32) {
  CTR_CREATE_CTOBJECT(object);
  ctr_ctypes_set_type(object, CTR_CTYPE_SINT32); //HALP
  return object;
}
CTR_CT_SIMPLE_TYPE_FUNC_SET(sint32) {
  *(int32_t*)(myself->value.rvalue->ptr) = (int32_t)(ctr_internal_cast2number(argumentList->object)->value.nvalue);
  return myself;
}
CTR_CT_SIMPLE_TYPE_FUNC_GET(sint32) {
  return ctr_build_number_from_float((double)(*((int32_t*)(myself->value.rvalue->ptr))));
}

//Unsigned Int 64
CTR_CT_SIMPLE_TYPE_FUNC_UNMAKE(uint64) {
  ctr_heap_free(myself->value.rvalue->ptr); //Free our resource, leave the rest to the GC
	return myself;
}
CTR_CT_SIMPLE_TYPE_FUNC_MAKE(uint64) {
  CTR_CREATE_CTOBJECT(object);
  ctr_ctypes_set_type(object, CTR_CTYPE_UINT64); //HALP
  return object;
}
CTR_CT_SIMPLE_TYPE_FUNC_SET(uint64) {
  *(uint64_t*)(myself->value.rvalue->ptr) = (uint64_t)(ctr_internal_cast2number(argumentList->object)->value.nvalue);
  return myself;
}
CTR_CT_SIMPLE_TYPE_FUNC_GET(uint64) {
  return ctr_build_number_from_float((double)(*((uint64_t*)(myself->value.rvalue->ptr))));
}

//Signed Int 64
CTR_CT_SIMPLE_TYPE_FUNC_UNMAKE(sint64) {
  ctr_heap_free(myself->value.rvalue->ptr); //Free our resource, leave the rest to the GC
	return myself;
}
CTR_CT_SIMPLE_TYPE_FUNC_MAKE(sint64) {
  CTR_CREATE_CTOBJECT(object);
  ctr_ctypes_set_type(object, CTR_CTYPE_SINT64); //HALP
  return object;
}
CTR_CT_SIMPLE_TYPE_FUNC_SET(sint64) {
  *(int64_t*)(myself->value.rvalue->ptr) = (int64_t)(ctr_internal_cast2number(argumentList->object)->value.nvalue);
  return myself;
}
CTR_CT_SIMPLE_TYPE_FUNC_GET(sint64) {
  return ctr_build_number_from_float((double)(*((int64_t*)(myself->value.rvalue->ptr))));
}

//Float
CTR_CT_SIMPLE_TYPE_FUNC_UNMAKE(float) {
  ctr_heap_free(myself->value.rvalue->ptr); //Free our resource, leave the rest to the GC
	return myself;
}
CTR_CT_SIMPLE_TYPE_FUNC_MAKE(float) {
  CTR_CREATE_CTOBJECT(object);
  ctr_ctypes_set_type(object, CTR_CTYPE_FLOAT); //HALP
  return object;
}
CTR_CT_SIMPLE_TYPE_FUNC_SET(float) {
  *(float*)(myself->value.rvalue->ptr) = (float)(ctr_internal_cast2number(argumentList->object)->value.nvalue);
  return myself;
}
CTR_CT_SIMPLE_TYPE_FUNC_GET(float) {
  return ctr_build_number_from_float((double)(*((float*)(myself->value.rvalue->ptr))));
}

//Double
CTR_CT_SIMPLE_TYPE_FUNC_UNMAKE(double) {
  ctr_heap_free(myself->value.rvalue->ptr); //Free our resource, leave the rest to the GC
	return myself;
}
CTR_CT_SIMPLE_TYPE_FUNC_MAKE(double) {
  CTR_CREATE_CTOBJECT(object);
  ctr_ctypes_set_type(object, CTR_CTYPE_DOUBLE); //HALP
  return object;
}
CTR_CT_SIMPLE_TYPE_FUNC_SET(double) {
  *(double*)(myself->value.rvalue->ptr) = (double)(ctr_internal_cast2number(argumentList->object)->value.nvalue);
  return myself;
}
CTR_CT_SIMPLE_TYPE_FUNC_GET(double) {
  return ctr_build_number_from_float((*((double*)(myself->value.rvalue->ptr))));
}

//Unsigned Char
CTR_CT_SIMPLE_TYPE_FUNC_UNMAKE(uchar) {
  ctr_heap_free(myself->value.rvalue->ptr); //Free our resource, leave the rest to the GC
	return myself;
}
CTR_CT_SIMPLE_TYPE_FUNC_MAKE(uchar) {
  CTR_CREATE_CTOBJECT(object);
  ctr_ctypes_set_type(object, CTR_CTYPE_UCHAR); //HALP
  return object;
}
CTR_CT_SIMPLE_TYPE_FUNC_SET(uchar) {
  *(unsigned char*)(myself->value.rvalue->ptr) = (unsigned char)(*(ctr_internal_cast2string(argumentList->object)->value.svalue->value)); //ONE char! lol
  return myself;
}
CTR_CT_SIMPLE_TYPE_FUNC_GET(uchar) {
  return ctr_build_string(((char*)(unsigned char*)(myself->value.rvalue->ptr)), 1);
}

//Signed Char
CTR_CT_SIMPLE_TYPE_FUNC_UNMAKE(schar) {
  ctr_heap_free(myself->value.rvalue->ptr); //Free our resource, leave the rest to the GC
	return myself;
}
CTR_CT_SIMPLE_TYPE_FUNC_MAKE(schar) {
  CTR_CREATE_CTOBJECT(object);
  ctr_ctypes_set_type(object, CTR_CTYPE_SCHAR); //HALP
  return object;
}
CTR_CT_SIMPLE_TYPE_FUNC_SET(schar) {
  *(char*)(myself->value.rvalue->ptr) = (char)(*(ctr_internal_cast2string(argumentList->object)->value.svalue->value)); //ONE char! lol
  return myself;
}
CTR_CT_SIMPLE_TYPE_FUNC_GET(schar) {
  return ctr_build_string(((char*)(myself->value.rvalue->ptr)), 1);
}

//Unsigned Short
CTR_CT_SIMPLE_TYPE_FUNC_UNMAKE(ushort) {
  ctr_heap_free(myself->value.rvalue->ptr); //Free our resource, leave the rest to the GC
	return myself;
}
CTR_CT_SIMPLE_TYPE_FUNC_MAKE(ushort) {
  CTR_CREATE_CTOBJECT(object);
  ctr_ctypes_set_type(object, CTR_CTYPE_USHORT); //HALP
  return object;
}
CTR_CT_SIMPLE_TYPE_FUNC_SET(ushort) {
  *(unsigned short*)(myself->value.rvalue->ptr) = (unsigned short)(ctr_internal_cast2number(argumentList->object)->value.nvalue);
  return myself;
}
CTR_CT_SIMPLE_TYPE_FUNC_GET(ushort) {
  return ctr_build_number_from_float((double)(*((unsigned short*)(myself->value.rvalue->ptr))));
}

//Signed Short
CTR_CT_SIMPLE_TYPE_FUNC_UNMAKE(sshort) {
  ctr_heap_free(myself->value.rvalue->ptr); //Free our resource, leave the rest to the GC
	return myself;
}
CTR_CT_SIMPLE_TYPE_FUNC_MAKE(sshort) {
  CTR_CREATE_CTOBJECT(object);
  ctr_ctypes_set_type(object, CTR_CTYPE_SSHORT); //HALP
  return object;
}
CTR_CT_SIMPLE_TYPE_FUNC_SET(sshort) {
  *(short*)(myself->value.rvalue->ptr) = (short)(ctr_internal_cast2number(argumentList->object)->value.nvalue);
  return myself;
}
CTR_CT_SIMPLE_TYPE_FUNC_GET(sshort) {
  return ctr_build_number_from_float((double)(*((short*)(myself->value.rvalue->ptr))));
}

//Unsigned Int
CTR_CT_SIMPLE_TYPE_FUNC_UNMAKE(uint) {
  ctr_heap_free(myself->value.rvalue->ptr); //Free our resource, leave the rest to the GC
	return myself;
}
CTR_CT_SIMPLE_TYPE_FUNC_MAKE(uint) {
  CTR_CREATE_CTOBJECT(object);
  ctr_ctypes_set_type(object, CTR_CTYPE_UINT); //HALP
  return object;
}
CTR_CT_SIMPLE_TYPE_FUNC_SET(uint) {
  *(unsigned int*)(myself->value.rvalue->ptr) = (unsigned int)(ctr_internal_cast2number(argumentList->object)->value.nvalue);
  return myself;
}
CTR_CT_SIMPLE_TYPE_FUNC_GET(uint) {
  return ctr_build_number_from_float((double)(*((unsigned int*)(myself->value.rvalue->ptr))));
}

//Signed Int
CTR_CT_SIMPLE_TYPE_FUNC_UNMAKE(sint) {
  ctr_heap_free(myself->value.rvalue->ptr); //Free our resource, leave the rest to the GC
	return myself;
}
CTR_CT_SIMPLE_TYPE_FUNC_MAKE(sint) {
  CTR_CREATE_CTOBJECT(object);
  ctr_ctypes_set_type(object, CTR_CTYPE_SINT); //HALP
  return object;
}
CTR_CT_SIMPLE_TYPE_FUNC_SET(sint) {
  *(int*)(myself->value.rvalue->ptr) = (int)(ctr_internal_cast2number(argumentList->object)->value.nvalue);
  return myself;
}
CTR_CT_SIMPLE_TYPE_FUNC_GET(sint) {
  return ctr_build_number_from_float((double)(*((int*)(myself->value.rvalue->ptr))));
}

//Unsigned Long
CTR_CT_SIMPLE_TYPE_FUNC_UNMAKE(ulong) {
  ctr_heap_free(myself->value.rvalue->ptr); //Free our resource, leave the rest to the GC
	return myself;
}
CTR_CT_SIMPLE_TYPE_FUNC_MAKE(ulong) {
  CTR_CREATE_CTOBJECT(object);
  ctr_ctypes_set_type(object, CTR_CTYPE_ULONG); //HALP
  return object;
}
CTR_CT_SIMPLE_TYPE_FUNC_SET(ulong) {
  *(unsigned long*)(myself->value.rvalue->ptr) = (unsigned long)(ctr_internal_cast2number(argumentList->object)->value.nvalue);
  return myself;
}
CTR_CT_SIMPLE_TYPE_FUNC_GET(ulong) {
  return ctr_build_number_from_float((double)(*((unsigned long*)(myself->value.rvalue->ptr))));
}

//Signed Long
CTR_CT_SIMPLE_TYPE_FUNC_UNMAKE(slong) {
  ctr_heap_free(myself->value.rvalue->ptr); //Free our resource, leave the rest to the GC
	return myself;
}
CTR_CT_SIMPLE_TYPE_FUNC_MAKE(slong) {
  CTR_CREATE_CTOBJECT(object);
  ctr_ctypes_set_type(object, CTR_CTYPE_SLONG); //HALP
  return object;
}
CTR_CT_SIMPLE_TYPE_FUNC_SET(slong) {
  *(long*)(myself->value.rvalue->ptr) = (long)(ctr_internal_cast2number(argumentList->object)->value.nvalue);
  return myself;
}
CTR_CT_SIMPLE_TYPE_FUNC_GET(slong) {
  return ctr_build_number_from_float((double)(*((long*)(myself->value.rvalue->ptr))));
}

//Long Double
CTR_CT_SIMPLE_TYPE_FUNC_UNMAKE(longdouble) {
  ctr_heap_free(myself->value.rvalue->ptr); //Free our resource, leave the rest to the GC
	return myself;
}
CTR_CT_SIMPLE_TYPE_FUNC_MAKE(longdouble) { //OUCH, too big for literal citron number. consider bigger number?
  CTR_CREATE_CTOBJECT(object);
  ctr_ctypes_set_type(object, CTR_CTYPE_LONGDOUBLE); //HALP
  return object;
}
CTR_CT_SIMPLE_TYPE_FUNC_SET(longdouble) {
  *(long double*)(myself->value.rvalue->ptr) = (long double)(ctr_internal_cast2number(argumentList->object)->value.nvalue);
  return myself;
}
CTR_CT_SIMPLE_TYPE_FUNC_GET(longdouble) {
  return ctr_build_number_from_float((double)(*((long double*)(myself->value.rvalue->ptr)))); //Actual downcast, will lose accuracy
}

//Citron object Pointer
CTR_CT_SIMPLE_TYPE_FUNC_UNMAKE(pointer) {
  //no touching the pointee
	return myself;
}
CTR_CT_SIMPLE_TYPE_FUNC_MAKE(pointer) {
  CTR_CREATE_CTOBJECT(object);
  ctr_ctypes_set_type(object, CTR_CTYPE_POINTER); //HALP
  return object;
}
CTR_CT_SIMPLE_TYPE_FUNC_SET(pointer) {
  if(argumentList->object->info.type == CTR_OBJECT_TYPE_OTEX) {
    myself->value.rvalue->ptr = argumentList->object->value.rvalue->ptr;
  } else
    (myself->value.rvalue->ptr) = (void*)(argumentList->object);
  return myself;
}
CTR_CT_SIMPLE_TYPE_FUNC_STR(pointer) {
  ctr_object* str_rep = ctr_build_string_from_cstring("<CType Pointer ");
  char repr[128];
  sprintf(repr, "%p", myself->value.rvalue->ptr);
  ctr_argument* args = ctr_heap_allocate(sizeof(ctr_argument));
  args->object = ctr_build_string_from_cstring(repr);
  ctr_string_append(str_rep, args);
  args->object = ctr_build_string_from_cstring(">");
  ctr_string_append(str_rep, args);
  ctr_heap_free(args);
  return str_rep;
}

CTR_CT_SIMPLE_TYPE_FUNC_UNMAKE(dynamic_lib) {
  dlclose(myself->value.rvalue->ptr); //close the handle
	return myself;
}
CTR_CT_SIMPLE_TYPE_FUNC_MAKE(dynamic_lib) {
  CTR_CREATE_CTOBJECT(object);
  ctr_ctypes_set_type(object, CTR_CTYPE_DYN_LIB);
  return object;
}
CTR_CT_SIMPLE_TYPE_FUNC_STR(dynamic_lib) {
  ctr_object* str_rep = ctr_build_string_from_cstring("<CType Dynamic Library at ");
  char repr[128];
  sprintf(repr, "%p", myself->value.rvalue->ptr);
  ctr_argument* args = ctr_heap_allocate(sizeof(ctr_argument));
  args->object = ctr_build_string_from_cstring(repr);
  ctr_string_append(str_rep, args);
  args->object = ctr_build_string_from_cstring(">");
  ctr_string_append(str_rep, args);
  ctr_heap_free(args);
  return str_rep;
}

CTR_CT_SIMPLE_TYPE_FUNC_GET(dynamic_lib) {
  char* symbol = ctr_heap_allocate_cstring(argumentList->object);
  void(*fn)(void) = dlsym(myself->value.rvalue->ptr, symbol);
  ctr_object* funptr = ctr_ctypes_make_pointer(NULL,NULL);
  funptr->value.rvalue->ptr = (void*) fn;
  ctr_heap_free(symbol);
  return funptr;
}

ctr_object* ctr_ctypes_get_first_meta(ctr_object* object, ctr_object* last) {
  ctr_object* link = object;
  while(link->link != NULL && link->link != last) {
    link = link->link;
  }
  return link;
}
//FFI Bindings  TODO: Implement simple structs
ffi_type* ctr_ctypes_ffi_convert_to_ffi_type(ctr_object* type) {
    ctr_object* initial = ctr_ctypes_get_first_meta(type->link, CtrStdCType);
    if((initial) == CtrStdCType_void)             return &ffi_type_void;
    else if((initial) == CtrStdCType_uint8)       return &ffi_type_uint8;
    else if((initial) == CtrStdCType_sint8)       return &ffi_type_sint8;
    else if((initial) == CtrStdCType_uint16)      return &ffi_type_uint16;
    else if((initial) == CtrStdCType_sint16)      return &ffi_type_sint16;
    else if((initial) == CtrStdCType_uint32)      return &ffi_type_uint32;
    else if((initial) == CtrStdCType_sint32)      return &ffi_type_sint32;
    else if((initial) == CtrStdCType_uint64)      return &ffi_type_uint64;
    else if((initial) == CtrStdCType_sint64)      return &ffi_type_sint64;
    else if((initial) == CtrStdCType_float)       return &ffi_type_float;
    else if((initial) == CtrStdCType_double)      return &ffi_type_double;
    else if((initial) == CtrStdCType_uchar)       return &ffi_type_uchar;
    else if((initial) == CtrStdCType_schar)       return &ffi_type_schar;
    else if((initial) == CtrStdCType_ushort)      return &ffi_type_ushort;
    else if((initial) == CtrStdCType_sshort)      return &ffi_type_sshort;
    else if((initial) == CtrStdCType_uint)        return &ffi_type_uint;
    else if((initial) == CtrStdCType_sint)        return &ffi_type_sint;
    else if((initial) == CtrStdCType_ulong)       return &ffi_type_ulong;
    else if((initial) == CtrStdCType_slong)       return &ffi_type_slong;
    else if((initial) == CtrStdCType_longdouble)  return &ffi_type_longdouble;
    else if((initial) == CtrStdCType_pointer ||
             initial == CtrStdCType_dynamic_lib)  return &ffi_type_pointer;

    else return &ffi_type_void;
}
ctr_ctype ctr_ctypes_ffi_convert_to_citron_ctype(ctr_object* type) {
    ctr_object* initial = ctr_ctypes_get_first_meta(type, CtrStdCType);
    if((initial) == CtrStdCType_void)             return CTR_CTYPE_VOID;
    else if((initial) == CtrStdCType_uint8)       return CTR_CTYPE_UINT8;
    else if((initial) == CtrStdCType_sint8)       return CTR_CTYPE_SINT8;
    else if((initial) == CtrStdCType_uint16)      return CTR_CTYPE_UINT16;
    else if((initial) == CtrStdCType_sint16)      return CTR_CTYPE_SINT16;
    else if((initial) == CtrStdCType_uint32)      return CTR_CTYPE_UINT32;
    else if((initial) == CtrStdCType_sint32)      return CTR_CTYPE_SINT32;
    else if((initial) == CtrStdCType_uint64)      return CTR_CTYPE_UINT64;
    else if((initial) == CtrStdCType_sint64)      return CTR_CTYPE_SINT64;
    else if((initial) == CtrStdCType_float)       return CTR_CTYPE_FLOAT;
    else if((initial) == CtrStdCType_double)      return CTR_CTYPE_DOUBLE;
    else if((initial) == CtrStdCType_uchar)       return CTR_CTYPE_UCHAR;
    else if((initial) == CtrStdCType_schar)       return CTR_CTYPE_SCHAR;
    else if((initial) == CtrStdCType_ushort)      return CTR_CTYPE_USHORT;
    else if((initial) == CtrStdCType_sshort)      return CTR_CTYPE_SSHORT;
    else if((initial) == CtrStdCType_uint)        return CTR_CTYPE_UINT;
    else if((initial) == CtrStdCType_sint)        return CTR_CTYPE_SINT;
    else if((initial) == CtrStdCType_ulong)       return CTR_CTYPE_ULONG;
    else if((initial) == CtrStdCType_slong)       return CTR_CTYPE_SLONG;
    else if((initial) == CtrStdCType_longdouble)  return CTR_CTYPE_LONGDOUBLE;
    else if((initial) == CtrStdCType_pointer ||
             initial == CtrStdCType_dynamic_lib)  return CTR_CTYPE_POINTER;

    else return CTR_CTYPE_VOID;
}
ctr_object* ctr_ctypes_convert_ffi_type_to_citron(ffi_arg* value, ctr_ctype type) {
  CTR_CREATE_CTOBJECT(object);
  object->value.rvalue = ctr_heap_allocate_tracked(sizeof(ctr_resource));
  switch(type) {
    case CTR_CTYPE_VOID: object->link = CtrStdCType_void; break;
    case CTR_CTYPE_UINT8: object->value.rvalue->ptr = (void*)(uint8_t*)value; object->link = CtrStdCType_uint8; break;
    case CTR_CTYPE_SINT8: object->value.rvalue->ptr = (void*)(int8_t*)value; object->link = CtrStdCType_sint8; break;
    case CTR_CTYPE_UINT16: object->value.rvalue->ptr = (void*)(uint16_t*)value; object->link = CtrStdCType_uint16; break;
    case CTR_CTYPE_SINT16: object->value.rvalue->ptr = (void*)(int16_t*)value; object->link = CtrStdCType_sint16; break;
    case CTR_CTYPE_UINT32: object->value.rvalue->ptr = (void*)(uint32_t*)value; object->link = CtrStdCType_uint32; break;
    case CTR_CTYPE_SINT32: object->value.rvalue->ptr = (void*)(int32_t*)value; object->link = CtrStdCType_sint32; break;
    case CTR_CTYPE_UINT64: object->value.rvalue->ptr = (void*)(uint64_t*)value; object->link = CtrStdCType_uint64; break;
    case CTR_CTYPE_SINT64: object->value.rvalue->ptr = (void*)(int64_t*)value; object->link = CtrStdCType_sint64; break;
    case CTR_CTYPE_FLOAT: object->value.rvalue->ptr = (void*)(float*)value; object->link = CtrStdCType_float; break;
    case CTR_CTYPE_DOUBLE: object->value.rvalue->ptr = (void*)(double*)value; object->link = CtrStdCType_double; break;
    case CTR_CTYPE_UCHAR: object->value.rvalue->ptr = (void*)(unsigned char*)value; object->link = CtrStdCType_uchar; break;
    case CTR_CTYPE_SCHAR: object->value.rvalue->ptr = (void*)(char*)value; object->link = CtrStdCType_schar; break;
    case CTR_CTYPE_USHORT: object->value.rvalue->ptr = (void*)(unsigned short*)value; object->link = CtrStdCType_ushort; break;
    case CTR_CTYPE_SSHORT: object->value.rvalue->ptr = (void*)(short*)value; object->link = CtrStdCType_sshort; break;
    case CTR_CTYPE_UINT: object->value.rvalue->ptr = (void*)(unsigned int*)value; object->link = CtrStdCType_uint; break;
    case CTR_CTYPE_SINT: object->value.rvalue->ptr = (void*)(int*)value; object->link = CtrStdCType_sint; break;
    case CTR_CTYPE_ULONG: object->value.rvalue->ptr = (void*)(unsigned long*)value; object->link = CtrStdCType_ulong; break;
    case CTR_CTYPE_SLONG: object->value.rvalue->ptr = (void*)(long*)value; object->link = CtrStdCType_slong; break;
    case CTR_CTYPE_LONGDOUBLE: object->value.rvalue->ptr = (void*)(long double*)value; object->link = CtrStdCType_longdouble; break;
    case CTR_CTYPE_POINTER: object->value.rvalue->ptr = (void*)value; object->link = CtrStdCType_pointer; break;
    case CTR_CTYPE_DYN_LIB: object->link = CtrStdCType_dynamic_lib; break;//Ha?
    default: object = CtrStdNil;
  }
  return object;
}
CTR_CT_FFI_BIND(cif_destruct) {//free the cif
  ctr_heap_free(myself->value.rvalue->ptr); //Free the cif, the resource will be collected by the GC
  return myself;
}
CTR_CT_FFI_BIND(cif_new) { //^External CIF ptr
  ffi_cif* cif = ctr_heap_allocate(sizeof(ffi_cif));
  ctr_object* cifobj = ctr_internal_create_object(CTR_OBJECT_TYPE_OTEX);
  ctr_ctypes_set_type(cifobj, CTR_CTYPE_CIF);
  cifobj->link = CtrStdCType_ffi_cif;
  cifobj->value.rvalue->ptr = (void*)cif;
  return cifobj;
}

CTR_CT_FFI_BIND(prep_cif) { //cif*, int<abi>, type* rtype, type** atypes
  ffi_cif*      cif_res = (ffi_cif*)(myself->value.rvalue->ptr);
  int           abi_mask= (int)(argumentList->object->value.nvalue);
  ffi_type*     rtype   = (ctr_ctypes_ffi_convert_to_ffi_type(argumentList->next->object));
  ctr_object*   atypes_ = argumentList->next->next->object;
  int           asize   = (int)(ctr_array_count(atypes_, NULL)->value.nvalue);
  ffi_type**    atypes  = ctr_heap_allocate_tracked(sizeof(ffi_type) * asize);
  ctr_argument* args    = ctr_heap_allocate(sizeof(ctr_argument));
  for(int i = 0; i<asize; i++) {
    args->object = ctr_build_number_from_float(i);
    atypes[i] = ctr_ctypes_ffi_convert_to_ffi_type(ctr_array_get(atypes_, args));
  }
  ffi_abi abi;
  switch(abi_mask) {
    default: abi=FFI_DEFAULT_ABI; break;
  }
  ffi_status status = ffi_prep_cif(cif_res, abi, asize, rtype, atypes);
  ctr_heap_free(args);
  if (status != FFI_OK) CtrStdFlow = ctr_build_string_from_cstring((status==FFI_BAD_ABI?"FFI_BAD_ABI":"FFI_ERROR"));
  else {
    ctr_internal_object_set_property(myself, ctr_build_string_from_cstring(":crType"), ctr_build_number_from_float(ctr_ctypes_ffi_convert_to_citron_ctype(argumentList->next->object)), 0);
  }
  return myself;
}

CTR_CT_FFI_BIND(call) { //<cif, CTypes pointer (fn), Array avalues; ^CTypes type* rtype
  ffi_arg*      result    = ctr_heap_allocate(sizeof(ffi_arg));
  ffi_cif*      cif       = (ffi_cif*)(myself->value.rvalue->ptr);
  ctr_object*   avals_    = argumentList->next->object;
  int           asize     = (int)(ctr_array_count(avals_, NULL)->value.nvalue);
  void**        avals     = ctr_heap_allocate(sizeof(void*) * asize);
  ctr_argument* args      = ctr_heap_allocate(sizeof(ctr_argument));
  void        (*fn)(void) = (void (*)(void)) (argumentList->object->value.rvalue->ptr);

  for(int i = 0; i<asize; i++) {
    args->object = ctr_build_number_from_float(i);
    avals[i] = (ctr_array_get(avals_, args)->value.rvalue->ptr);
    printf("assigning pointer %p to argument %d\n", avals[i], i);
  }
  ffi_call(cif, fn, result, avals);
  ctr_heap_free(avals);
  return ctr_ctypes_convert_ffi_type_to_citron(result, (ctr_ctypes_rtypeof(myself)));
}

CTR_CT_FFI_BIND(ll) {
  char* name;
  if(argumentList->object==CtrStdNil) name = NULL;
  else name = ctr_heap_allocate_cstring(argumentList->object);
  int mode = RTLD_NOW | RTLD_GLOBAL; //TODO Implement selecting modes
  //if(argumentList->next->object != CtrStdNil) {
  //  mode =
  //}
  void* handle = dlopen(name, mode);
  CTR_CREATE_CTOBJECT(object);
  ctr_ctypes_set_type(object, CTR_CTYPE_DYN_LIB);
  object->value.rvalue->ptr = handle;
  if(name != NULL) ctr_heap_free(name);
  return object;
}

CTR_CT_FFI_BIND(malloc) {
  int size = ctr_internal_cast2number(argumentList->object)->value.nvalue;
  void* resource = ctr_heap_allocate(size);
  ctr_object* object = ctr_ctypes_make_pointer(NULL,NULL);
  object->value.rvalue->ptr = resource;
  return object;
}
CTR_CT_FFI_BIND(memcpy) {//dest, source, count
  void *r0 = argumentList->object->value.rvalue->ptr, *r1 = argumentList->next->object->value.rvalue->ptr;
  int count = argumentList->next->next->object->value.nvalue;
  memcpy(r0, r1, count);
  return myself;
}
CTR_CT_FFI_BIND(fill_buf) {//dest, source
  void *r0 = argumentList->object->value.rvalue->ptr;
  ctr_object *c1 = ctr_internal_cast2string(argumentList->next->object);
  char* cstring = ctr_heap_allocate_cstring(c1);
  size_t* block_width;
  int q = sizeof( size_t );
  size_t size;

  /* find the correct size of this memory block and move pointer back */
  r0 = (void*) ((char*) r0 - q);
  block_width = (size_t*) r0;
  size = *(block_width);
  /* Then just put the thing back where it belongs */
  r0 = (void*) ((char*) r0 + q);
  int count = fmin(strlen(cstring), (int)size) - 1;
  for (int i =0; i<count; i++) *((char*)r0+i) = cstring[i];
  *(((char*)r0)+count+1) = '\0'; //NULL byte for proper c-string
  ctr_heap_free(cstring);
  return myself;
}
CTR_CT_FFI_BIND(buf_from_str) {
  ctr_object* str = ctr_internal_cast2string(argumentList->object);
  ctr_object* object = ctr_ctypes_make_pointer(NULL,NULL);
  object->value.rvalue->ptr = ctr_heap_allocate_cstring_shared(str);
  return object;
}
CTR_CT_FFI_BIND(free) {
  if ((argumentList->object->info.type) != CTR_OBJECT_TYPE_OTEX) {CtrStdFlow = ctr_build_string_from_cstring("Can only free resources."); return CtrStdNil;}
  ctr_heap_free(argumentList->object->value.rvalue->ptr);
  return myself;
}
void begin() {
  CtrStdCType = ctr_internal_create_object(CTR_OBJECT_TYPE_OTOBJECT);
  CtrStdCType->link = CtrStdObject;
  CtrStdCType_ffi_cif = ctr_internal_create_object(CTR_OBJECT_TYPE_OTOBJECT);
  CtrStdCType_ffi_cif->link = CtrStdObject;

  //Void
  CTR_CT_INTRODUCE_TYPE(void);
  CTR_CT_INTRODUCE_SET(void);
  CTR_CT_INTRODUCE_GET(void);
	CTR_CT_INTRODUCE_MAKE(void);
	CTR_CT_INTRODUCE_UNMAKE(void);
  //Unsigned Int 8
  CTR_CT_INTRODUCE_TYPE(uint8);
  CTR_CT_INTRODUCE_SET(uint8);
  CTR_CT_INTRODUCE_GET(uint8);
	CTR_CT_INTRODUCE_MAKE(uint8);
	CTR_CT_INTRODUCE_UNMAKE(uint8);
  //Signed Int 8
  CTR_CT_INTRODUCE_TYPE(sint8);
  CTR_CT_INTRODUCE_SET(sint8);
  CTR_CT_INTRODUCE_GET(sint8);
	CTR_CT_INTRODUCE_MAKE(sint8);
	CTR_CT_INTRODUCE_UNMAKE(sint8);
  //Unsigned Int 16
  CTR_CT_INTRODUCE_TYPE(uint16);
  CTR_CT_INTRODUCE_SET(uint16);
  CTR_CT_INTRODUCE_GET(uint16);
	CTR_CT_INTRODUCE_MAKE(uint16);
	CTR_CT_INTRODUCE_UNMAKE(uint16);
  //Signed Int 16
  CTR_CT_INTRODUCE_TYPE(sint16);
  CTR_CT_INTRODUCE_SET(sint16);
  CTR_CT_INTRODUCE_GET(sint16);
	CTR_CT_INTRODUCE_MAKE(sint16);
	CTR_CT_INTRODUCE_UNMAKE(sint16);
  //Unsigned Int 32
  CTR_CT_INTRODUCE_TYPE(uint32);
  CTR_CT_INTRODUCE_SET(uint32);
  CTR_CT_INTRODUCE_GET(uint32);
	CTR_CT_INTRODUCE_MAKE(uint32);
	CTR_CT_INTRODUCE_UNMAKE(uint32);
  //Signed Int 32
  CTR_CT_INTRODUCE_TYPE(sint32);
  CTR_CT_INTRODUCE_SET(sint32);
  CTR_CT_INTRODUCE_GET(sint32);
	CTR_CT_INTRODUCE_MAKE(sint32);
	CTR_CT_INTRODUCE_UNMAKE(sint32);
  //Unsigned Int 64
  CTR_CT_INTRODUCE_TYPE(uint64);
  CTR_CT_INTRODUCE_SET(uint64);
  CTR_CT_INTRODUCE_GET(uint64);
	CTR_CT_INTRODUCE_MAKE(uint64);
	CTR_CT_INTRODUCE_UNMAKE(uint64);
  //Signed Int 64
  CTR_CT_INTRODUCE_TYPE(sint64);
  CTR_CT_INTRODUCE_SET(sint64);
  CTR_CT_INTRODUCE_GET(sint64);
	CTR_CT_INTRODUCE_MAKE(sint64);
	CTR_CT_INTRODUCE_UNMAKE(sint64);
  //Float
  CTR_CT_INTRODUCE_TYPE(float);
  CTR_CT_INTRODUCE_SET(float);
  CTR_CT_INTRODUCE_GET(float);
	CTR_CT_INTRODUCE_MAKE(float);
	CTR_CT_INTRODUCE_UNMAKE(float);
  //Double
  CTR_CT_INTRODUCE_TYPE(double);
  CTR_CT_INTRODUCE_SET(double);
  CTR_CT_INTRODUCE_GET(double);
	CTR_CT_INTRODUCE_MAKE(double);
	CTR_CT_INTRODUCE_UNMAKE(double);
  //Signed Char
  CTR_CT_INTRODUCE_TYPE(schar);
  CTR_CT_INTRODUCE_SET(schar);
  CTR_CT_INTRODUCE_GET(schar);
	CTR_CT_INTRODUCE_MAKE(schar);
	CTR_CT_INTRODUCE_UNMAKE(schar);
  //Unsigned Char
  CTR_CT_INTRODUCE_TYPE(uchar);
  CTR_CT_INTRODUCE_SET(uchar);
  CTR_CT_INTRODUCE_GET(uchar);
	CTR_CT_INTRODUCE_MAKE(uchar);
	CTR_CT_INTRODUCE_UNMAKE(uchar);
  //Signed Short
  CTR_CT_INTRODUCE_TYPE(sshort);
  CTR_CT_INTRODUCE_SET(sshort);
  CTR_CT_INTRODUCE_GET(sshort);
	CTR_CT_INTRODUCE_MAKE(sshort);
	CTR_CT_INTRODUCE_UNMAKE(sshort);
  //Unsigned Short
  CTR_CT_INTRODUCE_TYPE(ushort);
  CTR_CT_INTRODUCE_SET(ushort);
  CTR_CT_INTRODUCE_GET(ushort);
	CTR_CT_INTRODUCE_MAKE(ushort);
	CTR_CT_INTRODUCE_UNMAKE(ushort);
  //Signed Int
  CTR_CT_INTRODUCE_TYPE(sint);
  CTR_CT_INTRODUCE_SET(sint);
  CTR_CT_INTRODUCE_GET(sint);
	CTR_CT_INTRODUCE_MAKE(sint);
	CTR_CT_INTRODUCE_UNMAKE(sint);
  //Unsigned Int
  CTR_CT_INTRODUCE_TYPE(uint);
  CTR_CT_INTRODUCE_SET(uint);
  CTR_CT_INTRODUCE_GET(uint);
	CTR_CT_INTRODUCE_MAKE(uint);
	CTR_CT_INTRODUCE_UNMAKE(uint);
  //Signed Long
  CTR_CT_INTRODUCE_TYPE(slong);
  CTR_CT_INTRODUCE_SET(slong);
  CTR_CT_INTRODUCE_GET(slong);
	CTR_CT_INTRODUCE_MAKE(slong);
	CTR_CT_INTRODUCE_UNMAKE(slong);
  //Unsigned Long
  CTR_CT_INTRODUCE_TYPE(ulong);
  CTR_CT_INTRODUCE_SET(ulong);
  CTR_CT_INTRODUCE_GET(ulong);
	CTR_CT_INTRODUCE_MAKE(ulong);
	CTR_CT_INTRODUCE_UNMAKE(ulong);
  //Long Double
  CTR_CT_INTRODUCE_TYPE(longdouble);
  CTR_CT_INTRODUCE_SET(longdouble);
  CTR_CT_INTRODUCE_GET(longdouble);
	CTR_CT_INTRODUCE_MAKE(longdouble);
	CTR_CT_INTRODUCE_UNMAKE(longdouble);
  //Pointer
  CTR_CT_INTRODUCE_TYPE(pointer);
  CTR_CT_INTRODUCE_SET(pointer);
  ctr_internal_create_func(CtrStdCType_pointer, ctr_build_string_from_cstring("toString"), &ctr_ctypes_str_pointer);
  CTR_CT_INTRODUCE_MAKE(pointer);
	CTR_CT_INTRODUCE_UNMAKE(pointer);
  //Dynamic Library
  CTR_CT_INTRODUCE_TYPE(dynamic_lib);
  ctr_internal_create_func(CtrStdCType_dynamic_lib, ctr_build_string_from_cstring("respondTo:"), &ctr_ctypes_get_dynamic_lib);
  ctr_internal_create_func(CtrStdCType_dynamic_lib, ctr_build_string_from_cstring("toString"), &ctr_ctypes_str_dynamic_lib);
  CTR_CT_INTRODUCE_MAKE(dynamic_lib);
	CTR_CT_INTRODUCE_UNMAKE(dynamic_lib);

  ctr_internal_create_func(CtrStdCType_ffi_cif, ctr_build_string_from_cstring("new"), &ctr_ctype_ffi_cif_new);
  ctr_internal_create_func(CtrStdCType_ffi_cif, ctr_build_string_from_cstring("destruct"), &ctr_ctype_ffi_cif_destruct);
  ctr_internal_create_func(CtrStdCType_ffi_cif, ctr_build_string_from_cstring("setABI:return:argTypes:"), &ctr_ctype_ffi_prep_cif);
  ctr_internal_create_func(CtrStdCType_ffi_cif, ctr_build_string_from_cstring("call:withArgs:"), &ctr_ctype_ffi_call);

  ctr_internal_create_func(CtrStdCType, ctr_build_string_from_cstring("allocateBytes:"), &ctr_ctype_ffi_malloc);
  ctr_internal_create_func(CtrStdCType, ctr_build_string_from_cstring("copyTo:from:numBytes:"), &ctr_ctype_ffi_memcpy);
  ctr_internal_create_func(CtrStdCType, ctr_build_string_from_cstring("fill:withString:"), &ctr_ctype_ffi_fill_buf);
  ctr_internal_create_func(CtrStdCType, ctr_build_string_from_cstring("free:"), &ctr_ctype_ffi_free);
  ctr_internal_create_func(CtrStdCType, ctr_build_string_from_cstring("fromString:"), &ctr_ctype_ffi_buf_from_str);
  ctr_internal_create_func(CtrStdCType, ctr_build_string_from_cstring("loadLibrary:"), &ctr_ctype_ffi_ll);

  ctr_internal_object_add_property(CtrStdWorld, ctr_build_string_from_cstring("CIF"), CtrStdCType_ffi_cif, 0);
  ctr_internal_object_add_property(CtrStdWorld, ctr_build_string_from_cstring("CTypes"), CtrStdCType, 0);
  //bind initial->lexical_name->value.svalue->value
}
