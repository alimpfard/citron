#include "ctypes.h"
#include <dlfcn.h>
#include <inttypes.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

ctr_object *ctr_ctypes_struct_get_size(ctr_object *myself,
                                       ctr_argument *argumentList);
ctr_object *ctr_ctypes_packed_size(ctr_object *myself,
                                   ctr_argument *argumentList);
// Common
#define CTR_CREATE_CTOBJECT(name)                                              \
  ctr_object *name = ctr_internal_create_object(CTR_OBJECT_TYPE_OTEX)

/**@I_OBJ_DEF CTypes*/
/**
 * CTypes
 *
 * An interface to the native "foreign" functions (AKA C-style functions)
 * Also a collection of C types
 */
/**@I_OBJ_DEF CIF*/
/**
 * CIF
 *
 * An object that contains and specifies the types of a foreign function/symbol
 */
ffi_type *ctr_ctype_citron_object_try_infer_type(ctr_object *object) {
  switch (object->info.type) {
  case CTR_OBJECT_TYPE_OTNIL:
    return &ffi_type_void;
  case CTR_OBJECT_TYPE_OTBOOL:
    return &ffi_type_uint;
  case CTR_OBJECT_TYPE_OTNUMBER: {
    double intpart;
    if (modf(object->value.nvalue, &intpart) == 0)
      return &ffi_type_sint64;
    return &ffi_type_double;
  }
  case CTR_OBJECT_TYPE_OTARRAY: // if an array, pass array
    return &ffi_type_pointer;   // only doubles for now
  case CTR_OBJECT_TYPE_OTSTRING:
    return &ffi_type_pointer;
  case CTR_OBJECT_TYPE_OTEX:
    return ctr_ctypes_ffi_convert_to_ffi_type(object);
  case CTR_OBJECT_TYPE_OTNATFUNC: // Do not pass around native functions, bad
                                  // kid.
  case CTR_OBJECT_TYPE_OTARRAY: // these do not make sense, C has no notion of
                                // them
  case CTR_OBJECT_TYPE_OTOBJECT:
  case CTR_OBJECT_TYPE_OTMISC:
  case CTR_OBJECT_TYPE_OTBLOCK:
  default:
    return NULL;
  }
}

void ctr_run_function_ptr(ffi_cif *cif, void *ret, void *args[],
                          void *function) {
  if (!(function && cif))
    return;
  ctr_argument *ctrargs = NULL, *argumentList;
  ffi_type *return_t = cif->rtype;
  int argc = cif->nargs;
  if (argc)
    ctrargs = ctr_heap_allocate(sizeof(ctr_argument));
  argumentList = ctrargs;
  for (int i = 0; i < argc; i++) {
    argumentList->object = nudispatch(args[i], cif->arg_types[i]);
    if (i < argc - 1)
      argumentList->next = ctr_heap_allocate(sizeof(ctr_argument));
    argumentList = argumentList->next;
  }
  argumentList = ctrargs;
  ctr_object *result = ctr_block_run(
      (ctr_object *)function, argumentList,
      (ctr_object *)function); // No type information for args, so we can't
                               // deconstruct them unless provided with type info
  ffi_type *type =
      return_t ? return_t : ctr_ctype_citron_object_try_infer_type(result);
  if (type)
    npdispatch((char *)ret, result, /*Inferred type */ type);
}

void ctr_ctypes_set_type(ctr_object *object, ctr_ctype type) {
  ctr_internal_object_set_property(object,
                                   ctr_build_string_from_cstring(":cType"),
                                   ctr_build_number_from_float((int)type), 0);
  object->value.rvalue = ctr_heap_allocate(sizeof(ctr_resource));
  object->info.sticky = 1;
  object->value.rvalue->type = (int)(type);
  switch (type) {
  case CTR_CTYPE_VOID:
    ctr_set_link_all(object, CtrStdCType_void);
    break;
  case CTR_CTYPE_UINT8:
    object->value.rvalue->ptr = ctr_heap_allocate(sizeof(uint8_t));
    ctr_set_link_all(object, CtrStdCType_uint8);
    break;
  case CTR_CTYPE_SINT8:
    object->value.rvalue->ptr = ctr_heap_allocate(sizeof(int8_t));
    ctr_set_link_all(object, CtrStdCType_sint8);
    break;
  case CTR_CTYPE_UINT16:
    object->value.rvalue->ptr = ctr_heap_allocate(sizeof(uint16_t));
    ctr_set_link_all(object, CtrStdCType_uint16);
    break;
  case CTR_CTYPE_SINT16:
    object->value.rvalue->ptr = ctr_heap_allocate(sizeof(int16_t));
    ctr_set_link_all(object, CtrStdCType_sint16);
    break;
  case CTR_CTYPE_UINT32:
    object->value.rvalue->ptr = ctr_heap_allocate(sizeof(uint32_t));
    ctr_set_link_all(object, CtrStdCType_uint32);
    break;
  case CTR_CTYPE_SINT32:
    object->value.rvalue->ptr = ctr_heap_allocate(sizeof(int32_t));
    ctr_set_link_all(object, CtrStdCType_sint32);
    break;
  case CTR_CTYPE_UINT64:
    object->value.rvalue->ptr = ctr_heap_allocate(sizeof(uint64_t));
    ctr_set_link_all(object, CtrStdCType_uint64);
    break;
  case CTR_CTYPE_SINT64:
    object->value.rvalue->ptr = ctr_heap_allocate(sizeof(int64_t));
    ctr_set_link_all(object, CtrStdCType_sint64);
    break;
  case CTR_CTYPE_FLOAT:
    object->value.rvalue->ptr = ctr_heap_allocate(sizeof(float));
    ctr_set_link_all(object, CtrStdCType_float);
    break;
  case CTR_CTYPE_DOUBLE:
    object->value.rvalue->ptr = ctr_heap_allocate(sizeof(double));
    ctr_set_link_all(object, CtrStdCType_double);
    break;
  case CTR_CTYPE_UCHAR:
    object->value.rvalue->ptr = ctr_heap_allocate(sizeof(unsigned char));
    ctr_set_link_all(object, CtrStdCType_uchar);
    break;
  case CTR_CTYPE_SCHAR:
    object->value.rvalue->ptr = ctr_heap_allocate(sizeof(char));
    ctr_set_link_all(object, CtrStdCType_schar);
    break;
  case CTR_CTYPE_USHORT:
    object->value.rvalue->ptr = ctr_heap_allocate(sizeof(unsigned short));
    ctr_set_link_all(object, CtrStdCType_ushort);
    break;
  case CTR_CTYPE_SSHORT:
    object->value.rvalue->ptr = ctr_heap_allocate(sizeof(short));
    ctr_set_link_all(object, CtrStdCType_sshort);
    break;
  case CTR_CTYPE_UINT:
    object->value.rvalue->ptr = ctr_heap_allocate(sizeof(unsigned int));
    ctr_set_link_all(object, CtrStdCType_uint);
    break;
  case CTR_CTYPE_SINT:
    object->value.rvalue->ptr = ctr_heap_allocate(sizeof(int));
    ctr_set_link_all(object, CtrStdCType_sint);
    break;
  case CTR_CTYPE_ULONG:
    object->value.rvalue->ptr = ctr_heap_allocate(sizeof(unsigned long));
    ctr_set_link_all(object, CtrStdCType_ulong);
    break;
  case CTR_CTYPE_SLONG:
    object->value.rvalue->ptr = ctr_heap_allocate(sizeof(long));
    ctr_set_link_all(object, CtrStdCType_slong);
    break;
  case CTR_CTYPE_LONGDOUBLE:
    object->value.rvalue->ptr = ctr_heap_allocate(sizeof(long double));
    ctr_set_link_all(object, CtrStdCType_longdouble);
    break;
  case CTR_CTYPE_POINTER:
    ctr_set_link_all(object, CtrStdCType_pointer);
    break;
  case CTR_CTYPE_DYN_LIB:
    ctr_set_link_all(object, CtrStdCType_dynamic_lib);
    break;
  case CTR_CTYPE_STRUCT:
    object->value.rvalue->ptr =
        ctr_heap_allocate(sizeof(ctr_ctypes_ffi_struct_value));
    memset(object->value.rvalue->ptr, 0, sizeof(ctr_ctypes_ffi_struct_value));
    ctr_set_link_all(object, CtrStdCType_struct);
    break;
  case CTR_CTYPE_STRING:
    ctr_set_link_all(object, CtrStdCType_string);
    break;
  case CTR_CTYPE_FUNCTION_POINTER:
    ctr_set_link_all(object, CtrStdCType_functionptr);
    break;
  case CTR_CTYPE_CONTIGUOUS_ARRAY:
    object->value.rvalue->ptr =
        ctr_heap_allocate(sizeof(ctr_ctypes_cont_array_t));
    memset(object->value.rvalue->ptr, 0, sizeof(ctr_ctypes_cont_array_t));
    ctr_set_link_all(object, CtrStdCType_cont_pointer);
    break;
  default:
    return;
  }
}

int ctr_is_valid_ctype(ctr_object *object) {
  return (ctr_internal_object_find_property(
              object, ctr_build_string_from_cstring(":cType"), 0) != NULL);
}

ctr_ctype ctr_ctypes_typeof(ctr_object *object) {
  ctr_object *ctype = (ctr_internal_object_find_property(
      object, ctr_build_string_from_cstring(":cType"), 0));
  if (ctype != NULL)
    return (ctr_ctype)(int)(ctype->value.nvalue);
  return CTR_CTYPE_INVALID;
}

ctr_ctype ctr_ctypes_rtypeof(ctr_object *object) {
  ctr_object *ctype = (ctr_internal_object_find_property(
      object, ctr_build_string_from_cstring(":crType"), 0));
  if (ctype != NULL)
    return (ctr_ctype)(int)(ctype->value.nvalue);
  return CTR_CTYPE_INVALID;
}

ctr_object *ctr_ctype_generic_copy(ctr_object *myself,
                                   ctr_argument *argumentList) {
  CTR_CREATE_CTOBJECT(object);
  if (!myself->value.rvalue) {
    CtrStdFlow = ctr_build_string_from_cstring(
        "Cannon instantiate invalid CType object");
  }
  ctr_ctypes_set_type(object, myself->value.rvalue->type);
  return object;
}

// Void
CTR_CT_SIMPLE_TYPE_FUNC_UNMAKE(void) { return myself; }

CTR_CT_SIMPLE_TYPE_FUNC_TSTR(void) {
  return ctr_build_string_from_cstring("<CTypes void>");
}

CTR_CT_SIMPLE_TYPE_FUNC_MAKE(void) { return ctr_build_nil(); }

CTR_CT_SIMPLE_TYPE_FUNC_SET(void) {
  CtrStdFlow = ctr_build_string_from_cstring("Cannot assign to void");
  return ctr_build_nil();
}

CTR_CT_SIMPLE_TYPE_FUNC_GET(void) { return ctr_build_nil(); }

// Unsigned Int 8
CTR_CT_SIMPLE_TYPE_FUNC_UNMAKE(uint8) {
  ctr_heap_free(
      myself->value.rvalue->ptr); // Free our resource, leave the rest to the GC
  myself->value.rvalue->ptr = NULL;
  return myself;
}

CTR_CT_SIMPLE_TYPE_FUNC_TSTR(uint8) {
  char buf[1024];
  sprintf(buf, "<CTypes uint8 at: %p value: %d>", myself->value.rvalue->ptr,
          *(uint8_t *)myself->value.rvalue->ptr);
  return ctr_build_string_from_cstring(buf);
}

CTR_CT_SIMPLE_TYPE_FUNC_MAKE(uint8) {
  CTR_CREATE_CTOBJECT(object);
  ctr_ctypes_set_type(object, CTR_CTYPE_UINT8);
  return object;
}

CTR_CT_SIMPLE_TYPE_FUNC_SET(uint8) {
  *(uint8_t *)(myself->value.rvalue->ptr) =
      (uint8_t)(ctr_internal_cast2number(argumentList->object)->value.nvalue);
  return myself;
}

CTR_CT_SIMPLE_TYPE_FUNC_GET(uint8) {
  return ctr_build_number_from_float(
      (double)(*((uint8_t *)(myself->value.rvalue
                                 ->ptr)))); // kinda derpy in bigger
                                            // values...meh will see if it derps.
  // TODO: Check if derps
}

// Signed Int 8
CTR_CT_SIMPLE_TYPE_FUNC_UNMAKE(sint8) {
  ctr_heap_free(
      myself->value.rvalue->ptr); // Free our resource, leave the rest to the GC
  myself->value.rvalue->ptr = NULL;
  return myself;
}

CTR_CT_SIMPLE_TYPE_FUNC_TSTR(sint8) {
  char buf[1024];
  sprintf(buf, "<CTypes sint8 at: %p value: %d>", myself->value.rvalue->ptr,
          *(int8_t *)myself->value.rvalue->ptr);
  return ctr_build_string_from_cstring(buf);
}

CTR_CT_SIMPLE_TYPE_FUNC_MAKE(sint8) {
  CTR_CREATE_CTOBJECT(object);
  ctr_ctypes_set_type(object, CTR_CTYPE_SINT8);
  return object;
}

CTR_CT_SIMPLE_TYPE_FUNC_SET(sint8) {
  *(int8_t *)(myself->value.rvalue->ptr) =
      (int8_t)(ctr_internal_cast2number(argumentList->object)->value.nvalue);
  return myself;
}

CTR_CT_SIMPLE_TYPE_FUNC_GET(sint8) {
  return ctr_build_number_from_float(
      (double)(*((int8_t *)(myself->value.rvalue->ptr))));
}

// Unsigned Int 16
CTR_CT_SIMPLE_TYPE_FUNC_UNMAKE(uint16) {
  ctr_heap_free(
      myself->value.rvalue->ptr); // Free our resource, leave the rest to the GC
  myself->value.rvalue->ptr = NULL;
  return myself;
}

CTR_CT_SIMPLE_TYPE_FUNC_TSTR(uint16) {
  char buf[1024];
  sprintf(buf, "<CTypes uint16 at: %p value: %d>", myself->value.rvalue->ptr,
          *(uint16_t *)myself->value.rvalue->ptr);
  return ctr_build_string_from_cstring(buf);
}

CTR_CT_SIMPLE_TYPE_FUNC_MAKE(uint16) {
  CTR_CREATE_CTOBJECT(object);
  ctr_ctypes_set_type(object, CTR_CTYPE_UINT16); // HALP
  return object;
}

CTR_CT_SIMPLE_TYPE_FUNC_SET(uint16) {
  *(uint16_t *)(myself->value.rvalue->ptr) =
      (uint16_t)(ctr_internal_cast2number(argumentList->object)->value.nvalue);
  return myself;
}

CTR_CT_SIMPLE_TYPE_FUNC_GET(uint16) {
  return ctr_build_number_from_float(
      (double)(*((uint16_t *)(myself->value.rvalue->ptr))));
}

// Signed Int 16
CTR_CT_SIMPLE_TYPE_FUNC_UNMAKE(sint16) {
  ctr_heap_free(
      myself->value.rvalue->ptr); // Free our resource, leave the rest to the GC
  myself->value.rvalue->ptr = NULL;
  return myself;
}

CTR_CT_SIMPLE_TYPE_FUNC_TSTR(sint16) {
  char buf[1024];
  sprintf(buf, "<CTypes sint16 at: %p value: %d>", myself->value.rvalue->ptr,
          *(int16_t *)myself->value.rvalue->ptr);
  return ctr_build_string_from_cstring(buf);
}

CTR_CT_SIMPLE_TYPE_FUNC_MAKE(sint16) {
  CTR_CREATE_CTOBJECT(object);
  ctr_ctypes_set_type(object, CTR_CTYPE_SINT16); // HALP
  return object;
}

CTR_CT_SIMPLE_TYPE_FUNC_SET(sint16) {
  *(int16_t *)(myself->value.rvalue->ptr) =
      (int16_t)(ctr_internal_cast2number(argumentList->object)->value.nvalue);
  return myself;
}

CTR_CT_SIMPLE_TYPE_FUNC_GET(sint16) {
  return ctr_build_number_from_float(
      (double)(*((int16_t *)(myself->value.rvalue->ptr))));
}

// Unsigned Int 32
CTR_CT_SIMPLE_TYPE_FUNC_UNMAKE(uint32) {
  ctr_heap_free(
      myself->value.rvalue->ptr); // Free our resource, leave the rest to the GC
  myself->value.rvalue->ptr = NULL;
  return myself;
}

CTR_CT_SIMPLE_TYPE_FUNC_TSTR(uint32) {
  char buf[1024];
  sprintf(buf, "<CTypes uint32 at: %p value: %d>", myself->value.rvalue->ptr,
          *(uint32_t *)myself->value.rvalue->ptr);
  return ctr_build_string_from_cstring(buf);
}

CTR_CT_SIMPLE_TYPE_FUNC_MAKE(uint32) {
  CTR_CREATE_CTOBJECT(object);
  ctr_ctypes_set_type(object, CTR_CTYPE_UINT32); // HALP
  return object;
}

CTR_CT_SIMPLE_TYPE_FUNC_SET(uint32) {
  *(uint32_t *)(myself->value.rvalue->ptr) =
      (uint32_t)(ctr_internal_cast2number(argumentList->object)->value.nvalue);
  return myself;
}

CTR_CT_SIMPLE_TYPE_FUNC_GET(uint32) {
  return ctr_build_number_from_float(
      (double)(*((uint32_t *)(myself->value.rvalue->ptr))));
}

// Signed Int 32
CTR_CT_SIMPLE_TYPE_FUNC_UNMAKE(sint32) {
  ctr_heap_free(
      myself->value.rvalue->ptr); // Free our resource, leave the rest to the GC
  myself->value.rvalue->ptr = NULL;
  return myself;
}

CTR_CT_SIMPLE_TYPE_FUNC_MAKE(sint32) {
  CTR_CREATE_CTOBJECT(object);
  ctr_ctypes_set_type(object, CTR_CTYPE_SINT32); // HALP
  return object;
}

CTR_CT_SIMPLE_TYPE_FUNC_TSTR(sint32) {
  char buf[1024];
  sprintf(buf, "<CTypes sint32 at: %p value: %d>", myself->value.rvalue->ptr,
          *(int32_t *)myself->value.rvalue->ptr);
  return ctr_build_string_from_cstring(buf);
}

CTR_CT_SIMPLE_TYPE_FUNC_SET(sint32) {
  *(int32_t *)(myself->value.rvalue->ptr) =
      (int32_t)(ctr_internal_cast2number(argumentList->object)->value.nvalue);
  return myself;
}

CTR_CT_SIMPLE_TYPE_FUNC_GET(sint32) {
  return ctr_build_number_from_float(
      (double)(*((int32_t *)(myself->value.rvalue->ptr))));
}

// Unsigned Int 64
CTR_CT_SIMPLE_TYPE_FUNC_UNMAKE(uint64) {
  ctr_heap_free(
      myself->value.rvalue->ptr); // Free our resource, leave the rest to the GC
  myself->value.rvalue->ptr = NULL;
  return myself;
}

CTR_CT_SIMPLE_TYPE_FUNC_TSTR(uint64) {
  char buf[1024];
  sprintf(buf, "<CTypes uint64 at: %p value: %d>", myself->value.rvalue->ptr,
          *(uint64_t *)myself->value.rvalue->ptr);
  return ctr_build_string_from_cstring(buf);
}

CTR_CT_SIMPLE_TYPE_FUNC_MAKE(uint64) {
  CTR_CREATE_CTOBJECT(object);
  ctr_ctypes_set_type(object, CTR_CTYPE_UINT64); // HALP
  return object;
}

CTR_CT_SIMPLE_TYPE_FUNC_SET(uint64) {
  *(uint64_t *)(myself->value.rvalue->ptr) =
      (uint64_t)(ctr_internal_cast2number(argumentList->object)->value.nvalue);
  return myself;
}

CTR_CT_SIMPLE_TYPE_FUNC_GET(uint64) {
  return ctr_build_number_from_float(
      (double)(*((uint64_t *)(myself->value.rvalue->ptr))));
}

// Signed Int 64
CTR_CT_SIMPLE_TYPE_FUNC_UNMAKE(sint64) {
  ctr_heap_free(
      myself->value.rvalue->ptr); // Free our resource, leave the rest to the GC
  myself->value.rvalue->ptr = NULL;
  return myself;
}

CTR_CT_SIMPLE_TYPE_FUNC_TSTR(sint64) {
  char buf[1024];
  sprintf(buf, "<CTypes sint64 at: %p value: %ld>", myself->value.rvalue->ptr,
          *(int64_t *)myself->value.rvalue->ptr);
  return ctr_build_string_from_cstring(buf);
}

CTR_CT_SIMPLE_TYPE_FUNC_MAKE(sint64) {
  CTR_CREATE_CTOBJECT(object);
  ctr_ctypes_set_type(object, CTR_CTYPE_SINT64); // HALP
  return object;
}

CTR_CT_SIMPLE_TYPE_FUNC_SET(sint64) {
  *(int64_t *)(myself->value.rvalue->ptr) =
      (int64_t)(ctr_internal_cast2number(argumentList->object)->value.nvalue);
  return myself;
}

CTR_CT_SIMPLE_TYPE_FUNC_GET(sint64) {
  return ctr_build_number_from_float(
      (double)(*((int64_t *)(myself->value.rvalue->ptr))));
}

// Float
CTR_CT_SIMPLE_TYPE_FUNC_UNMAKE(float) {
  ctr_heap_free(
      myself->value.rvalue->ptr); // Free our resource, leave the rest to the GC
  myself->value.rvalue->ptr = NULL;
  return myself;
}

CTR_CT_SIMPLE_TYPE_FUNC_TSTR(float) {
  char buf[1024];
  sprintf(buf, "<CTypes float at: %p value: %f>", myself->value.rvalue->ptr,
          *(float *)myself->value.rvalue->ptr);
  return ctr_build_string_from_cstring(buf);
}

CTR_CT_SIMPLE_TYPE_FUNC_MAKE(float) {
  CTR_CREATE_CTOBJECT(object);
  ctr_ctypes_set_type(object, CTR_CTYPE_FLOAT); // HALP
  return object;
}

CTR_CT_SIMPLE_TYPE_FUNC_SET(float) {
  *(float *)(myself->value.rvalue->ptr) =
      (float)(ctr_internal_cast2number(argumentList->object)->value.nvalue);
  return myself;
}

CTR_CT_SIMPLE_TYPE_FUNC_GET(float) {
  return ctr_build_number_from_float(
      (double)(*((float *)(myself->value.rvalue->ptr))));
}

// Double
CTR_CT_SIMPLE_TYPE_FUNC_UNMAKE(double) {
  ctr_heap_free(
      myself->value.rvalue->ptr); // Free our resource, leave the rest to the GC
  myself->value.rvalue->ptr = NULL;
  return myself;
}

CTR_CT_SIMPLE_TYPE_FUNC_TSTR(double) {
  char buf[1024];
  sprintf(buf, "<CTypes double at: %p value: %lf>", myself->value.rvalue->ptr,
          *(double *)myself->value.rvalue->ptr);
  return ctr_build_string_from_cstring(buf);
}

CTR_CT_SIMPLE_TYPE_FUNC_MAKE(double) {
  CTR_CREATE_CTOBJECT(object);
  ctr_ctypes_set_type(object, CTR_CTYPE_DOUBLE); // HALP
  return object;
}

CTR_CT_SIMPLE_TYPE_FUNC_SET(double) {
  *(double *)(myself->value.rvalue->ptr) =
      (double)(ctr_internal_cast2number(argumentList->object)->value.nvalue);
  return myself;
}

CTR_CT_SIMPLE_TYPE_FUNC_GET(double) {
  return ctr_build_number_from_float(
      (*((double *)(myself->value.rvalue->ptr))));
}

// Unsigned Char
CTR_CT_SIMPLE_TYPE_FUNC_UNMAKE(uchar) {
  ctr_heap_free(
      myself->value.rvalue->ptr); // Free our resource, leave the rest to the GC
  myself->value.rvalue->ptr = NULL;
  return myself;
}

CTR_CT_SIMPLE_TYPE_FUNC_TSTR(uchar) {
  char buf[1024];
  sprintf(buf, "<CTypes uchar at: %p value: %ud>", myself->value.rvalue->ptr,
          *(uint8_t *)myself->value.rvalue->ptr);
  return ctr_build_string_from_cstring(buf);
}

CTR_CT_SIMPLE_TYPE_FUNC_MAKE(uchar) {
  CTR_CREATE_CTOBJECT(object);
  ctr_ctypes_set_type(object, CTR_CTYPE_UCHAR); // HALP
  return object;
}

CTR_CT_SIMPLE_TYPE_FUNC_SET(uchar) {
  *(unsigned char *)(myself->value.rvalue->ptr) =
      (unsigned char)(*(ctr_internal_cast2string(argumentList->object)
                            ->value.svalue->value)); // ONE char! lol
  return myself;
}

CTR_CT_SIMPLE_TYPE_FUNC_GET(uchar) {
  return ctr_build_string(
      ((char *)(unsigned char *)(myself->value.rvalue->ptr)), 1);
}

// Signed Char
CTR_CT_SIMPLE_TYPE_FUNC_UNMAKE(schar) {
  ctr_heap_free(
      myself->value.rvalue->ptr); // Free our resource, leave the rest to the GC
  myself->value.rvalue->ptr = NULL;
  return myself;
}

CTR_CT_SIMPLE_TYPE_FUNC_MAKE(schar) {
  CTR_CREATE_CTOBJECT(object);
  ctr_ctypes_set_type(object, CTR_CTYPE_SCHAR); // HALP
  return object;
}

CTR_CT_SIMPLE_TYPE_FUNC_TSTR(schar) {
  char buf[1024];
  sprintf(buf, "<CTypes schar at: %p value: %d>", myself->value.rvalue->ptr,
          *(int8_t *)myself->value.rvalue->ptr);
  return ctr_build_string_from_cstring(buf);
}

CTR_CT_SIMPLE_TYPE_FUNC_SET(schar) {
  *(char *)(myself->value.rvalue->ptr) =
      (char)(*(ctr_internal_cast2string(argumentList->object)
                   ->value.svalue->value)); // ONE char! lol
  return myself;
}

CTR_CT_SIMPLE_TYPE_FUNC_GET(schar) {
  return ctr_build_string(((char *)(myself->value.rvalue->ptr)), 1);
}

// Unsigned Short
CTR_CT_SIMPLE_TYPE_FUNC_UNMAKE(ushort) {
  ctr_heap_free(
      myself->value.rvalue->ptr); // Free our resource, leave the rest to the GC
  myself->value.rvalue->ptr = NULL;
  return myself;
}

CTR_CT_SIMPLE_TYPE_FUNC_TSTR(ushort) {
  char buf[1024];
  sprintf(buf, "<CTypes ushort at: %p value: %ui>", myself->value.rvalue->ptr,
          *(ushort *)myself->value.rvalue->ptr);
  return ctr_build_string_from_cstring(buf);
}

CTR_CT_SIMPLE_TYPE_FUNC_MAKE(ushort) {
  CTR_CREATE_CTOBJECT(object);
  ctr_ctypes_set_type(object, CTR_CTYPE_USHORT); // HALP
  return object;
}

CTR_CT_SIMPLE_TYPE_FUNC_SET(ushort) {
  *(unsigned short *)(myself->value.rvalue->ptr) =
      (unsigned short)(ctr_internal_cast2number(argumentList->object)
                           ->value.nvalue);
  return myself;
}

CTR_CT_SIMPLE_TYPE_FUNC_GET(ushort) {
  return ctr_build_number_from_float(
      (double)(*((unsigned short *)(myself->value.rvalue->ptr))));
}

// Signed Short
CTR_CT_SIMPLE_TYPE_FUNC_UNMAKE(sshort) {
  ctr_heap_free(
      myself->value.rvalue->ptr); // Free our resource, leave the rest to the GC
  myself->value.rvalue->ptr = NULL;
  return myself;
}

CTR_CT_SIMPLE_TYPE_FUNC_TSTR(sshort) {
  char buf[1024];
  sprintf(buf, "<CTypes sshort at: %p value: %i>", myself->value.rvalue->ptr,
          *(short *)myself->value.rvalue->ptr);
  return ctr_build_string_from_cstring(buf);
}

CTR_CT_SIMPLE_TYPE_FUNC_MAKE(sshort) {
  CTR_CREATE_CTOBJECT(object);
  ctr_ctypes_set_type(object, CTR_CTYPE_SSHORT); // HALP
  return object;
}

CTR_CT_SIMPLE_TYPE_FUNC_SET(sshort) {
  *(short *)(myself->value.rvalue->ptr) =
      (short)(ctr_internal_cast2number(argumentList->object)->value.nvalue);
  return myself;
}

CTR_CT_SIMPLE_TYPE_FUNC_GET(sshort) {
  return ctr_build_number_from_float(
      (double)(*((short *)(myself->value.rvalue->ptr))));
}

// Unsigned Int
CTR_CT_SIMPLE_TYPE_FUNC_UNMAKE(uint) {
  ctr_heap_free(
      myself->value.rvalue->ptr); // Free our resource, leave the rest to the GC
  myself->value.rvalue->ptr = NULL;
  return myself;
}

CTR_CT_SIMPLE_TYPE_FUNC_TSTR(uint) {
  char buf[1024];
  sprintf(buf, "<CTypes uint at: %p value: %ui>", myself->value.rvalue->ptr,
          *(uint *)myself->value.rvalue->ptr);
  return ctr_build_string_from_cstring(buf);
}

CTR_CT_SIMPLE_TYPE_FUNC_MAKE(uint) {
  CTR_CREATE_CTOBJECT(object);
  ctr_ctypes_set_type(object, CTR_CTYPE_UINT); // HALP
  return object;
}

CTR_CT_SIMPLE_TYPE_FUNC_SET(uint) {
  *(unsigned int *)(myself->value.rvalue->ptr) =
      (unsigned int)(ctr_internal_cast2number(argumentList->object)
                         ->value.nvalue);
  return myself;
}

CTR_CT_SIMPLE_TYPE_FUNC_GET(uint) {
  return ctr_build_number_from_float(
      (double)(*((unsigned int *)(myself->value.rvalue->ptr))));
}

// Signed Int
CTR_CT_SIMPLE_TYPE_FUNC_UNMAKE(sint) {
  ctr_heap_free(
      myself->value.rvalue->ptr); // Free our resource, leave the rest to the GC
  myself->value.rvalue->ptr = NULL;
  return myself;
}

CTR_CT_SIMPLE_TYPE_FUNC_TSTR(sint) {
  char buf[1024];
  sprintf(buf, "<CTypes sint at: %p value: %i>", myself->value.rvalue->ptr,
          *(int *)myself->value.rvalue->ptr);
  return ctr_build_string_from_cstring(buf);
}

CTR_CT_SIMPLE_TYPE_FUNC_MAKE(sint) {
  CTR_CREATE_CTOBJECT(object);
  ctr_ctypes_set_type(object, CTR_CTYPE_SINT); // HALP
  return object;
}

CTR_CT_SIMPLE_TYPE_FUNC_SET(sint) {
  *(int *)(myself->value.rvalue->ptr) =
      (int)(ctr_internal_cast2number(argumentList->object)->value.nvalue);
  return myself;
}

CTR_CT_SIMPLE_TYPE_FUNC_GET(sint) {
  return ctr_build_number_from_float(
      (double)(*((int *)(myself->value.rvalue->ptr))));
}

// Unsigned Long
CTR_CT_SIMPLE_TYPE_FUNC_UNMAKE(ulong) {
  ctr_heap_free(
      myself->value.rvalue->ptr); // Free our resource, leave the rest to the GC
  myself->value.rvalue->ptr = NULL;
  return myself;
}

CTR_CT_SIMPLE_TYPE_FUNC_TSTR(ulong) {
  char buf[1024];
  sprintf(buf, "<CTypes ulong at: %p value: %lu>", myself->value.rvalue->ptr,
          *(unsigned long *)myself->value.rvalue->ptr);
  return ctr_build_string_from_cstring(buf);
}

CTR_CT_SIMPLE_TYPE_FUNC_MAKE(ulong) {
  CTR_CREATE_CTOBJECT(object);
  ctr_ctypes_set_type(object, CTR_CTYPE_ULONG); // HALP
  return object;
}

CTR_CT_SIMPLE_TYPE_FUNC_SET(ulong) {
  *(unsigned long *)(myself->value.rvalue->ptr) =
      (unsigned long)(ctr_internal_cast2number(argumentList->object)
                          ->value.nvalue);
  return myself;
}

CTR_CT_SIMPLE_TYPE_FUNC_GET(ulong) {
  return ctr_build_number_from_float(
      (double)(*((unsigned long *)(myself->value.rvalue->ptr))));
}

// Signed Long
CTR_CT_SIMPLE_TYPE_FUNC_UNMAKE(slong) {
  ctr_heap_free(
      myself->value.rvalue->ptr); // Free our resource, leave the rest to the GC
  myself->value.rvalue->ptr = NULL;
  return myself;
}

CTR_CT_SIMPLE_TYPE_FUNC_TSTR(slong) {
  char buf[1024];
  sprintf(buf, "<CTypes slong at: %p value: %li>", myself->value.rvalue->ptr,
          *(long *)myself->value.rvalue->ptr);
  return ctr_build_string_from_cstring(buf);
}

CTR_CT_SIMPLE_TYPE_FUNC_MAKE(slong) {
  CTR_CREATE_CTOBJECT(object);
  ctr_ctypes_set_type(object, CTR_CTYPE_SLONG); // HALP
  return object;
}

CTR_CT_SIMPLE_TYPE_FUNC_SET(slong) {
  *(long *)(myself->value.rvalue->ptr) =
      (long)(ctr_internal_cast2number(argumentList->object)->value.nvalue);
  return myself;
}

CTR_CT_SIMPLE_TYPE_FUNC_GET(slong) {
  return ctr_build_number_from_float(
      (double)(*((long *)(myself->value.rvalue->ptr))));
}

// Long Double
CTR_CT_SIMPLE_TYPE_FUNC_UNMAKE(longdouble) {
  ctr_heap_free(
      myself->value.rvalue->ptr); // Free our resource, leave the rest to the GC
  myself->value.rvalue->ptr = NULL;
  return myself;
}

CTR_CT_SIMPLE_TYPE_FUNC_TSTR(longdouble) {
  char buf[1024];
  sprintf(buf, "<CTypes ulong at: %p value: %Lf>", myself->value.rvalue->ptr,
          *(long double *)myself->value.rvalue->ptr);
  return ctr_build_string_from_cstring(buf);
}

CTR_CT_SIMPLE_TYPE_FUNC_MAKE(longdouble) { // OUCH, too big for literal citron
                                           // number. consider bigger number?
  CTR_CREATE_CTOBJECT(object);
  ctr_ctypes_set_type(object, CTR_CTYPE_LONGDOUBLE); // HALP
  return object;
}

CTR_CT_SIMPLE_TYPE_FUNC_SET(longdouble) {
  *(long double *)(myself->value.rvalue->ptr) =
      (long double)(ctr_internal_cast2number(argumentList->object)
                        ->value.nvalue);
  return myself;
}

CTR_CT_SIMPLE_TYPE_FUNC_GET(longdouble) {
  return ctr_build_number_from_float((double)(*(
      (long double *)(myself->value.rvalue
                          ->ptr)))); // Actual downcast, will lose accuracy
}

// Citron object Pointer
CTR_CT_SIMPLE_TYPE_FUNC_UNMAKE(pointer) {
  // no touching the pointee
  return myself;
}

CTR_CT_SIMPLE_TYPE_FUNC_MAKE(pointer) {
  CTR_CREATE_CTOBJECT(object);
  ctr_ctypes_set_type(object, CTR_CTYPE_POINTER); // HALP
  return object;
}

CTR_CT_SIMPLE_TYPE_FUNC_SET(pointer) {
  if (argumentList->object->info.type == CTR_OBJECT_TYPE_OTEX) {
    myself->value.rvalue->ptr = argumentList->object->value.rvalue->ptr;
  } else
    (myself->value.rvalue->ptr) =
        (void *)((intptr_t)argumentList->object->value.nvalue);
  return myself;
}

CTR_CT_SIMPLE_TYPE_FUNC_GET(pointer) {
  return ctr_build_number_from_float((intptr_t)myself->value.rvalue->ptr);
}

CTR_CT_SIMPLE_TYPE_FUNC_STR(pointer) {
  ctr_object *str_rep = ctr_build_string_from_cstring("<CType Pointer ");
  char repr[128];
  sprintf(repr, "%p", myself->value.rvalue->ptr);
  ctr_argument *args = ctr_heap_allocate(sizeof(ctr_argument));
  args->object = ctr_build_string_from_cstring(repr);
  ctr_string_append(str_rep, args);
  args->object = ctr_build_string_from_cstring(">");
  ctr_string_append(str_rep, args);
  ctr_heap_free(args);
  return str_rep;
}

CTR_CT_SIMPLE_TYPE_FUNC_MAKE(cont_pointer) {
  CTR_CREATE_CTOBJECT(object);
  ctr_ctypes_set_type(object, CTR_CTYPE_CONTIGUOUS_ARRAY);
  return object;
}
CTR_CT_SIMPLE_TYPE_FUNC_TSTR(cont_pointer) {
  char buf[1024];
  ctr_ctypes_cont_array_t *arr = myself->value.rvalue->ptr;
  sprintf(buf, "<CTypes contiguous pointer at: %p count: %zu>", arr->storage,
          arr->count);
  return ctr_build_string_from_cstring(buf);
}
/*
ffi_type_uint8
ffi_type_sint8
ffi_type_uint16
ffi_type_sint16
ffi_type_uint32
ffi_type_sint32
ffi_type_uint64
ffi_type_sint64
ffi_type_uchar
ffi_type_schar
ffi_type_ushort
ffi_type_sshort
ffi_type_uint
ffi_type_sint
ffi_type_ulong
ffi_type_slong
ffi_type_float
ffi_type_double
ffi_type_longdouble
*/
ctr_object *ctr_ctypes_make_packed(
    ctr_object *myself,
    ctr_argument *argumentList) { // packed: [int] count: [int]
  int typeid = ctr_internal_cast2number(argumentList->object)->value.nvalue;
  ffi_type *type_ = 0;
  size_t size = 0;
  size_t count =
      ctr_internal_cast2number(argumentList->next->object)->value.nvalue;
  switch (typeid) {
  case 0:
    size = sizeof(uint8_t);
    type_ = &ffi_type_uint8;
    break;
  case 1:
    size = sizeof(char);
    type_ = &ffi_type_sint8;
    break;
  case 2:
    size = sizeof(uint16_t);
    type_ = &ffi_type_uint16;
    break;
  case 3:
    size = sizeof(int16_t);
    type_ = &ffi_type_sint16;
    break;
  case 4:
    size = sizeof(uint32_t);
    type_ = &ffi_type_uint32;
    break;
  case 5:
    size = sizeof(int32_t);
    type_ = &ffi_type_sint32;
    break;
  case 6:
    size = sizeof(uint64_t);
    type_ = &ffi_type_uint64;
    break;
  case 7:
    size = sizeof(int64_t);
    type_ = &ffi_type_sint64;
    break;
  case 8:
    size = sizeof(unsigned char);
    type_ = &ffi_type_uchar;
    break;
  case 9:
    size = sizeof(signed char);
    type_ = &ffi_type_schar;
    break;
  case 10:
    size = sizeof(unsigned short);
    type_ = &ffi_type_ushort;
    break;
  case 11:
    size = sizeof(signed short);
    type_ = &ffi_type_sshort;
    break;
  case 12:
    size = sizeof(unsigned int);
    type_ = &ffi_type_uint;
    break;
  case 13:
    size = sizeof(signed int);
    type_ = &ffi_type_sint;
    break;
  case 14:
    size = sizeof(unsigned long);
    type_ = &ffi_type_ulong;
    break;
  case 15:
    size = sizeof(signed long);
    type_ = &ffi_type_slong;
    break;
  case 16:
    size = sizeof(float);
    type_ = &ffi_type_float;
    break;
  case 17:
    size = sizeof(double);
    type_ = &ffi_type_double;
    break;
  case 18:
    size = sizeof(long double);
    type_ = &ffi_type_longdouble;
    break;
  case 19:
    size = sizeof(void *);
    type_ = &ffi_type_pointer;
    break;
  default:
    size = 0;
    type_ = 0;
  }
  if (!type_) {
    CtrStdFlow = ctr_build_string_from_cstring("Incorrect type id for array");
    return CtrStdNil;
  }
  void *storage = ctr_heap_allocate(size * count);
  ctr_ctypes_cont_array_t *arr = ctr_heap_allocate(sizeof(*arr));
  arr->storage = storage;
  arr->count = count;
  arr->esize = size;
  arr->etype = type_;
  ctr_object *obj = ctr_ctypes_make_cont_pointer(myself, NULL);
  obj->value.rvalue->ptr = arr;
  return obj;
}

ctr_object *ctr_ctypes_packed_from(ctr_object *myself,
                                   ctr_argument *argumentList) {
  ctr_ctypes_cont_array_t *ptr = myself->value.rvalue->ptr;
  if (!ptr) {
    CtrStdFlow = ctr_build_string_from_cstring("Inavlid packed array");
    return ctr_build_nil();
  }
  size_t size = ptr->esize;
  size_t count = ptr->count;
  void *storage = ctr_heap_allocate(size * count);
  ctr_ctypes_cont_array_t *arr = ctr_heap_allocate(sizeof(*arr));
  arr->etype = ptr->etype;
  arr->esize = size;
  arr->count = count;
  arr->storage = storage;
  ctr_object *obj = ctr_ctypes_make_cont_pointer(myself, NULL);
  obj->value.rvalue->ptr = arr;
  return obj;
}

ctr_object *ctr_ctypes_packed_set(ctr_object *myself,
                                  ctr_argument *argumentList) {
  ctr_ctypes_cont_array_t *ptr = myself->value.rvalue->ptr;
  if (!ptr)
    return myself;
  char *storage = ptr->storage;
  ffi_type *etype = ptr->etype;
  size_t count = ptr->count, esize = ptr->esize;
  int elem = ctr_internal_cast2number(argumentList->next->object)->value.nvalue;
  if (elem >= count) {
    CtrStdFlow = ctr_build_string_from_cstring("Index out of bounds");
    return myself;
  }
  ctr_object *obj = argumentList->object;
  npdispatch(storage + (esize * elem), obj, etype);
  return myself;
}

ctr_object *ctr_ctypes_packed_get(ctr_object *myself,
                                  ctr_argument *argumentList) {
  ctr_ctypes_cont_array_t *ptr = myself->value.rvalue->ptr;
  if (!ptr)
    return myself;
  char *storage = ptr->storage;
  ffi_type *etype = ptr->etype;
  size_t count = ptr->count, esize = ptr->esize;
  int elem = ctr_internal_cast2number(argumentList->object)->value.nvalue;
  if (elem >= count) {
    CtrStdFlow = ctr_build_string_from_cstring("Index out of bounds");
    return CtrStdNil;
  }
  ctr_object *obj;
  obj = nudispatch(storage + (esize * elem), etype);
  if (!obj) {
    CtrStdFlow = ctr_build_string_from_cstring("Could not unpack the array");
    return CtrStdNil;
  }
  return obj;
}

ctr_object *ctr_ctypes_packed_count(ctr_object *myself,
                                    ctr_argument *argumentList) {
  ctr_ctypes_cont_array_t *ptr = myself->value.rvalue->ptr;
  if (!ptr)
    return ctr_build_number_from_float(0);
  size_t count = ptr->count;
  return ctr_build_number_from_float(count);
}

ctr_object *ctr_ctypes_packed_size(ctr_object *myself,
                                   ctr_argument *argumentList) {
  ctr_ctypes_cont_array_t *ptr = myself->value.rvalue->ptr;
  if (!ptr)
    return ctr_build_number_from_float(0);
  size_t count = ptr->count, esize = ptr->esize;
  return ctr_build_number_from_float(count * esize);
}

ctr_object *ctr_ctypes_packed_type(ctr_object *myself,
                                   ctr_argument *argumentList) {
  ctr_ctypes_cont_array_t *ptr = myself->value.rvalue->ptr;
  if (!ptr) {
    CtrStdFlow = ctr_format_str(
        "Uninitialized PackedArray, has not been assigned a type");
    return ctr_build_number_from_float(-1);
  }
  ffi_type *etype = ptr->etype;

  if (etype == &ffi_type_uint8)
    return ctr_build_number_from_float(0);
  else if (etype == &ffi_type_sint8)
    return ctr_build_number_from_float(1);
  else if (etype == &ffi_type_uint16)
    return ctr_build_number_from_float(2);
  else if (etype == &ffi_type_sint16)
    return ctr_build_number_from_float(3);
  else if (etype == &ffi_type_uint32)
    return ctr_build_number_from_float(4);
  else if (etype == &ffi_type_sint32)
    return ctr_build_number_from_float(5);
  else if (etype == &ffi_type_uint64)
    return ctr_build_number_from_float(6);
  else if (etype == &ffi_type_sint64)
    return ctr_build_number_from_float(7);
  else if (etype == &ffi_type_uchar)
    return ctr_build_number_from_float(8);
  else if (etype == &ffi_type_schar)
    return ctr_build_number_from_float(9);
  else if (etype == &ffi_type_ushort)
    return ctr_build_number_from_float(10);
  else if (etype == &ffi_type_sshort)
    return ctr_build_number_from_float(11);
  else if (etype == &ffi_type_uint)
    return ctr_build_number_from_float(12);
  else if (etype == &ffi_type_sint)
    return ctr_build_number_from_float(13);
  else if (etype == &ffi_type_ulong)
    return ctr_build_number_from_float(14);
  else if (etype == &ffi_type_slong)
    return ctr_build_number_from_float(15);
  else if (etype == &ffi_type_float)
    return ctr_build_number_from_float(16);
  else if (etype == &ffi_type_double)
    return ctr_build_number_from_float(17);
  else if (etype == &ffi_type_longdouble)
    return ctr_build_number_from_float(18);
  return ctr_build_number_from_float(-1);
}

ssize_t ctr_ctype_get_c_size(ctr_object *obj) {
  ctr_object *meta = ctr_ctypes_get_first_meta(obj, CtrStdCType);
  if (meta->info.type == CTR_OBJECT_TYPE_OTSTRING)
    return sizeof(char) * (meta->value.svalue->vlen + 1);
  if ((meta) == CtrStdCType_void)
    return sizeof(void);
  else if ((meta) == CtrStdCType_uint8)
    return sizeof(uint8_t);
  else if ((meta) == CtrStdCType_sint8)
    return sizeof(int8_t);
  else if ((meta) == CtrStdCType_uint16)
    return sizeof(uint16_t);
  else if ((meta) == CtrStdCType_sint16)
    return sizeof(int16_t);
  else if ((meta) == CtrStdCType_uint32)
    return sizeof(uint32_t);
  else if ((meta) == CtrStdCType_sint32)
    return sizeof(int32_t);
  else if ((meta) == CtrStdCType_uint64)
    return sizeof(uint64_t);
  else if ((meta) == CtrStdCType_sint64)
    return sizeof(int64_t);
  else if ((meta) == CtrStdCType_float)
    return sizeof(float);
  else if ((meta) == CtrStdCType_double)
    return sizeof(double);
  else if ((meta) == CtrStdCType_uchar)
    return sizeof(unsigned char);
  else if ((meta) == CtrStdCType_schar)
    return sizeof(char);
  else if ((meta) == CtrStdCType_ushort)
    return sizeof(unsigned short);
  else if ((meta) == CtrStdCType_sshort)
    return sizeof(short);
  else if ((meta) == CtrStdCType_uint)
    return sizeof(unsigned int);
  else if ((meta) == CtrStdCType_sint)
    return sizeof(signed int);
  else if ((meta) == CtrStdCType_ulong)
    return sizeof(unsigned long);
  else if ((meta) == CtrStdCType_slong)
    return sizeof(long);
  else if ((meta) == CtrStdCType_longdouble)
    return sizeof(long double);
  else if ((meta) == CtrStdCType_cont_pointer)
    return ctr_ctypes_packed_size(obj, NULL)->value.nvalue;
  else if (meta == CtrStdCType_struct)
    return ctr_ctypes_struct_get_size(obj, NULL)
        ->value.nvalue; // signal for struct
  // else if((meta) == CtrStdCType_pointer ||
  // meta == CtrStdCType_dynamic_lib ||
  // meta == CtrStdCType_string)     return sizeof(void*);
  else
    return sizeof(void *);
}

ctr_object *ctr_ctypes_addr_of(ctr_object *myself, ctr_argument *argumentList) {
  ctr_object *ret = ctr_ctypes_make_pointer(NULL, NULL);
  if (myself->info.type != CTR_OBJECT_TYPE_OTEX) {
    ret->value.rvalue->ptr = &(myself);
    return ret;
  } else {
    ret->value.rvalue->ptr = &(myself->value.rvalue->ptr);
    return ret;
  }
}

ctr_object *ctr_ctypes_addr_of_raw(ctr_object *myself,
                                   ctr_argument *argumentList) {
  ctr_object *ret = ctr_ctypes_make_pointer(NULL, NULL);
  if (myself->info.type != CTR_OBJECT_TYPE_OTEX) {
    ret->value.rvalue->ptr = (myself);
    return ret;
  } else {
    ctr_object *meta = ctr_ctypes_get_first_meta(myself, CtrStdCType);
    ret->value.rvalue->ptr =
        (meta == CtrStdCType_struct)
            ? ((ctr_ctypes_ffi_struct_value *)(myself->value.rvalue->ptr))
                  ->value
            : (meta == CtrStdCType_cont_pointer)
                  ? ((ctr_ctypes_cont_array_t *)(myself->value.rvalue->ptr))
                        ->storage
                  : (myself->value.rvalue->ptr);
    return ret;
  }
}

ctr_object *ctr_ctypes_pointer_set_obj(ctr_object *myself,
                                       ctr_argument *argumentList) {
  ctr_object *ret = ctr_ctypes_make_pointer(NULL, NULL);
  ret->value.rvalue->ptr = &(myself);
  return ret;
}

ctr_object *ctr_ctypes_pointer_get_obj(ctr_object *myself,
                                       ctr_argument *argumentList) {
  return myself->value.rvalue->ptr;
}

ctr_object *ctr_ctypes_pointer_arith_add(ctr_object *myself,
                                         ctr_argument *argumentList) {
  void *ptr = myself->value.rvalue->ptr;
  ctr_object *newp = ctr_ctypes_make_pointer(NULL, NULL);
  newp->value.rvalue->ptr =
      ptr + ((intptr_t)argumentList->object->value.nvalue);
  return newp;
}

ctr_object *ctr_ctypes_pointer_arith_sub(ctr_object *myself,
                                         ctr_argument *argumentList) {
  void *ptr = myself->value.rvalue->ptr;
  ctr_object *newp = ctr_ctypes_make_pointer(NULL, NULL);
  newp->value.rvalue->ptr =
      ptr - ((intptr_t)argumentList->object->value.nvalue);
  return newp;
}

ctr_object *ctr_ctypes_to_bytes(ctr_object *myself,
                                ctr_argument *argumentList) {
  ctr_object *ret = ctr_array_new(CtrStdArray, NULL);
  char *buf = (void *)myself->value.rvalue->ptr;
  int count = ctr_internal_cast2number(argumentList->object)->value.nvalue;
  ctr_argument *arg = ctr_heap_allocate(sizeof(ctr_argument));
  for (; count; count--) {
    arg->object = ctr_build_number_from_float((double)(*buf++));
    ctr_array_push(ret, arg);
  }
  return ret;
}

ctr_object *ctr_ctypes_to_bytes_string(ctr_object *myself,
                                       ctr_argument *argumentList) {
  char *buf = (void *)myself->value.rvalue->ptr;
  int count = ctr_internal_cast2number(argumentList->object)->value.nvalue;
  ctr_object *ret = ctr_build_string(buf, count);
  return ret;
}

ctr_object *ctr_ctypes_read_string(ctr_object *myself,
                                   ctr_argument *argumentList) {
  char *buf = (void *)myself->value.rvalue->ptr;
  ctr_object *ret = ctr_build_string_from_cstring(buf);
  return ret;
}

ctr_object *ctr_ctype_new_instance(ctr_object *type) {
  if (type == CtrStdCType_void)
    return ctr_ctypes_make_void(CtrStdCType_void, NULL);
  else if (type == CtrStdCType_uint8)
    return ctr_ctypes_make_uint8(CtrStdCType_uint8, NULL);
  else if (type == CtrStdCType_sint8)
    return ctr_ctypes_make_sint8(CtrStdCType_sint8, NULL);
  else if (type == CtrStdCType_uint16)
    return ctr_ctypes_make_uint16(CtrStdCType_uint16, NULL);
  else if (type == CtrStdCType_sint16)
    return ctr_ctypes_make_sint16(CtrStdCType_sint16, NULL);
  else if (type == CtrStdCType_uint32)
    return ctr_ctypes_make_uint32(CtrStdCType_uint32, NULL);
  else if (type == CtrStdCType_sint32)
    return ctr_ctypes_make_sint32(CtrStdCType_sint32, NULL);
  else if (type == CtrStdCType_uint64)
    return ctr_ctypes_make_uint64(CtrStdCType_uint64, NULL);
  else if (type == CtrStdCType_sint64)
    return ctr_ctypes_make_sint64(CtrStdCType_sint64, NULL);
  else if (type == CtrStdCType_float)
    return ctr_ctypes_make_float(CtrStdCType_float, NULL);
  else if (type == CtrStdCType_double)
    return ctr_ctypes_make_double(CtrStdCType_double, NULL);
  else if (type == CtrStdCType_uchar)
    return ctr_ctypes_make_uchar(CtrStdCType_uchar, NULL);
  else if (type == CtrStdCType_schar)
    return ctr_ctypes_make_schar(CtrStdCType_schar, NULL);
  else if (type == CtrStdCType_ushort)
    return ctr_ctypes_make_ushort(CtrStdCType_ushort, NULL);
  else if (type == CtrStdCType_sshort)
    return ctr_ctypes_make_sshort(CtrStdCType_sshort, NULL);
  else if (type == CtrStdCType_uint)
    return ctr_ctypes_make_uint(CtrStdCType_uint, NULL);
  else if (type == CtrStdCType_sint)
    return ctr_ctypes_make_sint(CtrStdCType_sint, NULL);
  else if (type == CtrStdCType_ulong)
    return ctr_ctypes_make_ulong(CtrStdCType_ulong, NULL);
  else if (type == CtrStdCType_slong)
    return ctr_ctypes_make_slong(CtrStdCType_slong, NULL);
  else if (type == CtrStdCType_longdouble)
    return ctr_ctypes_make_longdouble(CtrStdCType_longdouble, NULL);
  else if (type == CtrStdCType_pointer)
    return ctr_ctypes_make_pointer(CtrStdCType_pointer, NULL);
  else if (type == CtrStdCType_struct)
    return (ctr_object *)0x100; // signal for struct
  else
    return NULL;
}

ctr_object *
ctr_ctype_struct_copy(ctr_object *str) { // TODO: Write copy method for struct
  return str;
}

ctr_object *ctr_ctypes_deref_pointer(ctr_object *myself,
                                     ctr_argument *argumentList) {
  if (argumentList->object->info.type != CTR_OBJECT_TYPE_OTEX) {
    CtrStdFlow =
        ctr_build_string_from_cstring("Attempt to deref as a non-ctype object");
    return CtrStdNil;
  }
  void *ptr = myself->value.rvalue->ptr;
  ctr_object *meta =
      ctr_ctypes_get_first_meta(argumentList->object, CtrStdCType);
  ctr_object *new_obj = ctr_ctype_new_instance(meta);
  int isstruct = 0;
  if (!new_obj) {
    CtrStdFlow = ctr_build_string_from_cstring("Could not instantiate a CType");
    return CtrStdNil;
  }
  if (new_obj == (ctr_object *)0x100) {
    isstruct = 1;
    new_obj = ctr_ctype_struct_copy(argumentList->object);
  }
  ssize_t csize = ctr_ctype_get_c_size(argumentList->object);
  if (csize == -1)
    csize = ((ctr_ctypes_ffi_struct_value *)(argumentList->object->value.rvalue
                                                 ->ptr))
                ->size;
  if (!isstruct)
    memcpy(&new_obj->value.rvalue->ptr, ptr, csize);
  else
    ((ctr_ctypes_ffi_struct_value *)(new_obj->value.rvalue->ptr))->value = ptr;

  return new_obj;
}

ctr_object *ctr_ctypes_set_internal_pointer(ctr_object *myself,
                                            ctr_argument *argumentList) {
  if (argumentList->object->info.type != CTR_OBJECT_TYPE_OTEX) {
    CtrStdFlow = ctr_build_string_from_cstring(
        "Attempt to deref and set as a non-ctype object");
    return CtrStdNil;
  }
  void *ptr = myself->value.rvalue->ptr;
  void *res = argumentList->object->value.rvalue->ptr;
  ctr_object *meta =
      ctr_ctypes_get_first_meta(argumentList->object, CtrStdCType);
  ssize_t csize = ctr_ctype_get_c_size(argumentList->object);
  if (csize == -1)
    csize = ((ctr_ctypes_ffi_struct_value *)(argumentList->object->value.rvalue
                                                 ->ptr))
                ->size;
  memcpy(ptr, res, csize);

  return myself;
}

CTR_CT_SIMPLE_TYPE_FUNC_UNMAKE(dynamic_lib) {
  dlclose(myself->value.rvalue->ptr); // close the handle
  myself->value.rvalue->ptr = NULL;
  return myself;
}

CTR_CT_SIMPLE_TYPE_FUNC_MAKE(dynamic_lib) {
  CTR_CREATE_CTOBJECT(object);
  ctr_ctypes_set_type(object, CTR_CTYPE_DYN_LIB);
  return object;
}

CTR_CT_SIMPLE_TYPE_FUNC_STR(dynamic_lib) {
  ctr_object *str_rep =
      ctr_build_string_from_cstring("<CType Dynamic Library at ");
  char repr[128];
  sprintf(repr, "%p", myself->value.rvalue->ptr);
  ctr_argument *args = ctr_heap_allocate(sizeof(ctr_argument));
  args->object = ctr_build_string_from_cstring(repr);
  ctr_string_append(str_rep, args);
  args->object = ctr_build_string_from_cstring(">");
  ctr_string_append(str_rep, args);
  ctr_heap_free(args);
  return str_rep;
}

CTR_CT_SIMPLE_TYPE_FUNC_GET(dynamic_lib) {
  char *symbol = ctr_heap_allocate_cstring(argumentList->object);
  void (*fn)(void) = dlsym(myself->value.rvalue->ptr, symbol);
  ctr_object *funptr = ctr_ctypes_make_pointer(NULL, NULL);
  funptr->value.rvalue->ptr = (void *)fn;
  ctr_heap_free(symbol);
  return funptr;
}

void *ctr_struct_destruct(void *res) {
  ctr_ctypes_ffi_struct_value *ptr = res;
  ctr_heap_free(ptr->original_format);
  if (ptr->value)
    ctr_heap_free(ptr->value);
  return 0;
}

ctr_object *ctr_ctypes_struct_make_internal(ctr_object *myself, char *_fmt) {
  CTR_CREATE_CTOBJECT(object);
  ctr_ctypes_set_type(object, CTR_CTYPE_STRUCT);
  char *fmt = _fmt;
  char *fmtp = fmt;
  size_t size;
  struct_member_desc_t desc;
  desc = ctr_ffi_type_get_member_count(fmt, &size, 1);
  int member_count = desc.member_count;
  ffi_type *type = ctr_create_ffi_type_descriptor_(fmt, member_count);
  ctr_ctypes_ffi_struct_value *ptr =
      ctr_heap_allocate(sizeof(ctr_ctypes_ffi_struct_value));
  ptr->member_count = member_count;
  ptr->type = type;
  ptr->size = size;
  ptr->padinfo = desc.pad_structure;
  ptr->value = NULL;
  ptr->original_format = fmtp;
  object->value.rvalue->ptr = (void *)ptr;
  // object->release_hook = &ctr_struct_destruct;
  return object;
}

ctr_object *ctr_ctypes_make_struct(ctr_object *myself,
                                   ctr_argument *argumentList) {
  char *fmt = ctr_heap_allocate_cstring(argumentList->object);
  return ctr_ctypes_struct_make_internal(myself, fmt);
}

/* ptr is CTypes fromString: 'test'.
 * struct is CTypes structWithFormat: 'sip'. #signed int, pointer
 * struct pack: [4, ptr]. #Fills the struct <-
 */
ctr_object *ctr_ctypes_pack_struct(
    ctr_object *myself,
    ctr_argument *argumentList) { //<struct> pack: [member, ...]
  if (myself->info.type != CTR_OBJECT_TYPE_OTEX ||
      myself->value.rvalue->type != CTR_CTYPE_STRUCT) {
    // TODO: Error out
    printf("Invalid object\n");
    return myself;
  }
  ctr_ctypes_ffi_struct_value *structptr = myself->value.rvalue->ptr;
  ffi_type *structtype = structptr->type;
  ffi_type **struct_fields = structtype->elements;
  // int member_count = structptr->member_count;
  size_t size = structptr->size;
  ctr_object *data = argumentList->object;
  pad_info_node_t **padinfo = structptr->padinfo;
  char *fields = structptr->value ? structptr->value
                                  : ctr_heap_allocate(sizeof(char) * size);
  char *fieldsp = fields;
  CTR_ENSURE_TYPE_ARRAY(data);
  //----
  int init = 0;
  for (int i = data->value.avalue->tail; i < data->value.avalue->head; i++) {
    size_t this_size = reverse_ffi_type_size_map_lookup(struct_fields[init]);
    if (this_size == 0)
      printf("\n");
    while (padinfo[init]->pad) { // skip them pads
      fieldsp += 1;
      // ctr_heap_free(padinfo[init]); //we don't need this anymoar
      init++;
      if (!padinfo[init])
        break;
    }
    ctr_object *fld = *(data->value.avalue->elements + i);
    if (fld->info.type == CTR_OBJECT_TYPE_OTNIL) {
      memset(fieldsp, 0, this_size);
    } else if (fld->info.type == CTR_OBJECT_TYPE_OTEX) {
      nppointer(fieldsp, fld);
    } else {
      int ret = npdispatch(fieldsp, fld, struct_fields[init]);
    }
    fieldsp += this_size;
    // ctr_heap_free(padinfo[init]); //we don't need this anymoar either
    init++;
  }
  structptr->value = fields;
  //----
  return myself;
}

ctr_object *ctr_ctypes_pack_struct_at(
    ctr_object *myself,
    ctr_argument *argumentList) { //<struct> put: [value] at: [offset]
  if (myself->info.type != CTR_OBJECT_TYPE_OTEX ||
      myself->value.rvalue->type != CTR_CTYPE_STRUCT) {
    // TODO: Error out
    printf("Invalid object\n");
    return myself;
  }
  ctr_ctypes_ffi_struct_value *structptr = myself->value.rvalue->ptr;
  ffi_type *structtype = structptr->type;
  ffi_type **struct_fields = structtype->elements;
  int member_count = structptr->member_count;
  size_t size = structptr->size;
  ctr_object *fld = argumentList->object;
  size_t offset =
      ctr_internal_cast2number(argumentList->next->object)->value.nvalue;
  pad_info_node_t **padinfo = structptr->padinfo;
  char *fields = structptr->value ? structptr->value
                                  : ctr_heap_allocate(sizeof(char) * size);
  char *fieldsp = fields;
  //----
  for (int i = 0; i < member_count; i++) {
    if (padinfo[i]->offset == offset) {
      size_t this_size = reverse_ffi_type_size_map_lookup(struct_fields[i]);
      if (fld->info.type == CTR_OBJECT_TYPE_OTNIL) {
        memset(fieldsp + offset, 0, this_size);
      } else if (fld->info.type == CTR_OBJECT_TYPE_OTEX) {
        nppointer(fieldsp + offset, fld);
      } else {
        (void)npdispatch(fieldsp + offset, fld, struct_fields[i]);
      }
      structptr->value = fields;
    }
  }
  //----
  return myself;
}

ctr_object *
ctr_ctypes_unpack_struct(ctr_object *myself,
                         ctr_argument *argumentList) { //<struct> unpack
  if (myself->info.type != CTR_OBJECT_TYPE_OTEX ||
      myself->value.rvalue->type != CTR_CTYPE_STRUCT) {
    // TODO: Error out
    printf("Invalid object\n");
    return myself;
  }
  ctr_ctypes_ffi_struct_value *structptr = myself->value.rvalue->ptr;
  if (!structptr->value)
    return CtrStdNil;
  ffi_type *structtype = structptr->type;
  ffi_type **struct_fields = structtype->elements;
  int member_count = structptr->member_count;
  size_t size = structptr->size;
  ctr_object *data = ctr_array_new(CtrStdArray, NULL);
  pad_info_node_t **padinfo = structptr->padinfo;
  char *fieldsp = structptr->value;
  ctr_argument *arglist = ctr_heap_allocate(sizeof(ctr_argument));
  //----
  int init = 0;
  int i = 0;
  int ignore_pads = 0;
  // while(struct_fields[i]) {
  for (; i < member_count; i++) {
    if (!struct_fields[init])
      break;
    // size_t this_size = reverse_ffi_type_size_map_lookup(struct_fields[init]);
    if (!padinfo[init]) {
      // printf("no more pad info. dumping the rest");
      // ignore_pads = 1;
      return data;
    }
    while (!ignore_pads && padinfo[init]->pad) { // skip them pads
      // fieldsp+=1;
      // ctr_heap_free(padino[init]); //we don't need this anymoar
      init++;
      i++;
    }
    ctr_object *fld =
        nudispatch(fieldsp + padinfo[init]->offset, struct_fields[init]);
    if (fld) {
      arglist->object = fld;
      ctr_array_push(data, arglist);
    }
    // fieldsp += this_size;
    // ctr_heap_free(padinfo[init]); //we don't need this anymoar either
    init++;
  }
  //----
  return data;
}

ctr_object *
ctr_ctypes_unpack_struct_at(ctr_object *myself,
                            ctr_argument *argumentList) { //<struct> atOffset:
  // Will throw if offset is not found in padinfo
  if (myself->info.type != CTR_OBJECT_TYPE_OTEX ||
      myself->value.rvalue->type != CTR_CTYPE_STRUCT) {
    // TODO: Error out
    printf("Invalid object\n");
    return myself;
  }
  ctr_ctypes_ffi_struct_value *structptr = myself->value.rvalue->ptr;
  if (!structptr->value)
    return CtrStdNil;
  ffi_type *structtype = structptr->type;
  ffi_type **struct_fields = structtype->elements;
  int member_count = structptr->member_count;
  size_t offset = ctr_internal_cast2number(argumentList->object)->value.nvalue;
  pad_info_node_t **padinfo = structptr->padinfo;
  char *fieldsp = structptr->value;
  //----
  int i = 0;
  for (; i < member_count; i++) {
    if (padinfo[i]->offset == offset) {
      ctr_object *fld = nudispatch(fieldsp + offset, struct_fields[i]);
      if (fld)
        return fld;
      return CtrStdNil;
    }
  }
  return CtrStdNil;
}

ctr_object *ctr_ctypes_struct_to_string(ctr_object *myself,
                                        ctr_argument *argumentList) {
  ctr_ctypes_ffi_struct_value *ptr = myself->value.rvalue->ptr;
  ffi_type *type = ptr->type;
  int len = ctr_create_ffi_str_descriptor(type, NULL);
  char *buf = ctr_heap_allocate(len * sizeof(char) + 1); // one lucky byte, pls
  ctr_create_ffi_str_descriptor(type, buf);
  char *nbf = ctr_heap_allocate((len + 32) * sizeof(char));
  len = sprintf(nbf, "CTypes structWithFormat: '%.*s'", len, buf);
  return ctr_build_string(nbf, len);
}

ctr_object *ctr_ctypes_struct_new(ctr_object *myself,
                                  ctr_argument *argumentList) {
  ctr_ctypes_ffi_struct_value *ptr = myself->value.rvalue->ptr;
  ctr_object *new =
      ctr_ctypes_struct_make_internal(myself, ptr->original_format);
  return new;
}

ctr_object *ctr_ctypes_unmake_struct(ctr_object *myself,
                                     ctr_argument *argumentList) {
  argumentList->object = myself;
  ctr_gc_sweep_this(CtrStdGC, argumentList);
  return ctr_build_nil();
}

ctr_object *ctr_ctypes_struct_allocate(ctr_object *myself,
                                       ctr_argument *argumentList) {
  size_t size = ctr_ctype_get_c_size(myself);
  if (((ctr_ctypes_ffi_struct_value *)(myself->value.rvalue->ptr))->value)
    return myself;
  ((ctr_ctypes_ffi_struct_value *)(myself->value.rvalue->ptr))->value =
      ctr_heap_allocate(size);
  return myself;
}

ctr_object *ctr_ctypes_struct_set_from(ctr_object *myself,
                                       ctr_argument *argumentList) {
  if (ctr_ctypes_get_first_meta(argumentList->object, CtrStdCType) !=
      CtrStdCType_pointer) {
    CtrStdFlow =
        ctr_build_string_from_cstring("Expected to get a CTypes_pointer.");
    return myself;
  }
  void *ptr = argumentList->object->value.rvalue->ptr;
  if (!ptr) {
    CtrStdFlow = ctr_build_string_from_cstring("Tried to deref NULL");
    return myself;
  }
  ((ctr_ctypes_ffi_struct_value *)(myself->value.rvalue->ptr))->value = ptr;
  return myself;
}

ctr_object *ctr_ctypes_struct_get_size(ctr_object *myself,
                                       ctr_argument *argumentList) {
  ctr_ctypes_ffi_struct_value *ptr = myself->value.rvalue->ptr;
  size_t size = ptr->size;
  return ctr_build_number_from_float(size);
}

ctr_object *ctr_ctypes_struct_get_member_count(ctr_object *myself,
                                               ctr_argument *argumentList) {
  ctr_ctypes_ffi_struct_value *ptr = myself->value.rvalue->ptr;
  size_t size = ptr->member_count;
  return ctr_build_number_from_float(size);
}

ctr_object *ctr_ctypes_struct_get_padding_format(ctr_object *myself,
                                                 ctr_argument *argumentList) {
  ctr_ctypes_ffi_struct_value *ptr = myself->value.rvalue->ptr;
  ctr_object *arr = ctr_array_new(CtrStdArray, NULL);
  ctr_argument *args = ctr_heap_allocate(sizeof(ctr_argument));
  for (int i = 0; i < ptr->member_count; i++) {
    ctr_object *elem = ctr_array_new(CtrStdArray, NULL);
    args->object = ctr_build_bool(ptr->padinfo[i]->pad);
    ctr_array_push(elem, args);
    args->object = ctr_build_number_from_float(ptr->padinfo[i]->offset);
    ctr_array_push(elem, args);
    args->object = elem;
    ctr_array_push(arr, args);
  }
  ctr_heap_free(args);
  return arr;
}

ctr_object *ctr_ctypes_struct_get_offset_format(ctr_object *myself,
                                                ctr_argument *argumentList) {
  ctr_ctypes_ffi_struct_value *ptr = myself->value.rvalue->ptr;
  ctr_object *arr = ctr_array_new(CtrStdArray, NULL);
  ctr_argument *args = ctr_heap_allocate(sizeof(ctr_argument));
  for (int i = 0; i < ptr->member_count; i++) {
    if (ptr->padinfo[i]->pad)
      continue;
    args->object = ctr_build_number_from_float(ptr->padinfo[i]->offset);
    ctr_array_push(arr, args);
  }
  ctr_heap_free(args);
  return arr;
}

ctr_object *ctr_ctypes_get_first_meta(ctr_object *object, ctr_object *last) {
  ctr_object *link = object;
  while (link->interfaces->link != NULL && link->interfaces->link != last) {
    link = link->interfaces->link;
  }
  return link;
}

ffi_type *ctr_ctypes_ffi_convert_to_ffi_type(ctr_object *type) {
  ctr_object *initial =
      ctr_ctypes_get_first_meta(type->interfaces->link, CtrStdCType);
  if ((initial) == CtrStdCType_void)
    return &ffi_type_void;
  else if ((initial) == CtrStdCType_uint8)
    return &ffi_type_uint8;
  else if ((initial) == CtrStdCType_sint8)
    return &ffi_type_sint8;
  else if ((initial) == CtrStdCType_uint16)
    return &ffi_type_uint16;
  else if ((initial) == CtrStdCType_sint16)
    return &ffi_type_sint16;
  else if ((initial) == CtrStdCType_uint32)
    return &ffi_type_uint32;
  else if ((initial) == CtrStdCType_sint32)
    return &ffi_type_sint32;
  else if ((initial) == CtrStdCType_uint64)
    return &ffi_type_uint64;
  else if ((initial) == CtrStdCType_sint64)
    return &ffi_type_sint64;
  else if ((initial) == CtrStdCType_float)
    return &ffi_type_float;
  else if ((initial) == CtrStdCType_double)
    return &ffi_type_double;
  else if ((initial) == CtrStdCType_uchar)
    return &ffi_type_uchar;
  else if ((initial) == CtrStdCType_schar)
    return &ffi_type_schar;
  else if ((initial) == CtrStdCType_ushort)
    return &ffi_type_ushort;
  else if ((initial) == CtrStdCType_sshort)
    return &ffi_type_sshort;
  else if ((initial) == CtrStdCType_uint)
    return &ffi_type_uint;
  else if ((initial) == CtrStdCType_sint)
    return &ffi_type_sint;
  else if ((initial) == CtrStdCType_ulong)
    return &ffi_type_ulong;
  else if ((initial) == CtrStdCType_slong)
    return &ffi_type_slong;
  else if ((initial) == CtrStdCType_longdouble)
    return &ffi_type_longdouble;
  else if ((initial) == CtrStdCType_pointer ||
           initial == CtrStdCType_dynamic_lib ||
           initial == CtrStdCType_string ||
           initial == CtrStdCType_functionptr ||
           initial == CtrStdCType_cont_pointer)
    return &ffi_type_pointer;
  else if (initial == CtrStdCType_struct)
    return ((ctr_ctypes_ffi_struct_value *)(type->value.rvalue->ptr))->type;
  else
    return &ffi_type_void;
}

ctr_ctype ctr_ctypes_ffi_convert_to_citron_ctype(ctr_object *type) {
  ctr_object *initial = ctr_ctypes_get_first_meta(type, CtrStdCType);
  if ((initial) == CtrStdCType_void)
    return CTR_CTYPE_VOID;
  else if ((initial) == CtrStdCType_uint8)
    return CTR_CTYPE_UINT8;
  else if ((initial) == CtrStdCType_sint8)
    return CTR_CTYPE_SINT8;
  else if ((initial) == CtrStdCType_uint16)
    return CTR_CTYPE_UINT16;
  else if ((initial) == CtrStdCType_sint16)
    return CTR_CTYPE_SINT16;
  else if ((initial) == CtrStdCType_uint32)
    return CTR_CTYPE_UINT32;
  else if ((initial) == CtrStdCType_sint32)
    return CTR_CTYPE_SINT32;
  else if ((initial) == CtrStdCType_uint64)
    return CTR_CTYPE_UINT64;
  else if ((initial) == CtrStdCType_sint64)
    return CTR_CTYPE_SINT64;
  else if ((initial) == CtrStdCType_float)
    return CTR_CTYPE_FLOAT;
  else if ((initial) == CtrStdCType_double)
    return CTR_CTYPE_DOUBLE;
  else if ((initial) == CtrStdCType_uchar)
    return CTR_CTYPE_UCHAR;
  else if ((initial) == CtrStdCType_schar)
    return CTR_CTYPE_SCHAR;
  else if ((initial) == CtrStdCType_ushort)
    return CTR_CTYPE_USHORT;
  else if ((initial) == CtrStdCType_sshort)
    return CTR_CTYPE_SSHORT;
  else if ((initial) == CtrStdCType_uint)
    return CTR_CTYPE_UINT;
  else if ((initial) == CtrStdCType_sint)
    return CTR_CTYPE_SINT;
  else if ((initial) == CtrStdCType_ulong)
    return CTR_CTYPE_ULONG;
  else if ((initial) == CtrStdCType_slong)
    return CTR_CTYPE_SLONG;
  else if ((initial) == CtrStdCType_longdouble)
    return CTR_CTYPE_LONGDOUBLE;
  else if ((initial) == CtrStdCType_pointer ||
           initial == CtrStdCType_dynamic_lib)
    return CTR_CTYPE_POINTER;
  else if (initial == CtrStdCType_struct)
    return CTR_CTYPE_STRUCT;
  else
    return CTR_CTYPE_VOID;
}

ffi_type *ctr_ctypes_ffi_convert_ctype_to_ffi_type(ctr_ctype type) {
  switch (type) {
  case CTR_CTYPE_VOID:
    return &ffi_type_void;
  case CTR_CTYPE_UINT8:
    return &ffi_type_uint8;
  case CTR_CTYPE_SINT8:
    return &ffi_type_sint8;
  case CTR_CTYPE_UINT16:
    return &ffi_type_uint16;
  case CTR_CTYPE_SINT16:
    return &ffi_type_sint16;
  case CTR_CTYPE_UINT32:
    return &ffi_type_uint32;
  case CTR_CTYPE_SINT32:
    return &ffi_type_sint32;
  case CTR_CTYPE_UINT64:
    return &ffi_type_uint64;
  case CTR_CTYPE_SINT64:
    return &ffi_type_sint64;
  case CTR_CTYPE_FLOAT:
    return &ffi_type_float;
  case CTR_CTYPE_DOUBLE:
    return &ffi_type_double;
  case CTR_CTYPE_UCHAR:
    return &ffi_type_uchar;
  case CTR_CTYPE_SCHAR:
    return &ffi_type_schar;
  case CTR_CTYPE_USHORT:
    return &ffi_type_ushort;
  case CTR_CTYPE_SSHORT:
    return &ffi_type_sshort;
  case CTR_CTYPE_UINT:
    return &ffi_type_uint;
  case CTR_CTYPE_SINT:
    return &ffi_type_sint;
  case CTR_CTYPE_ULONG:
    return &ffi_type_ulong;
  case CTR_CTYPE_SLONG:
    return &ffi_type_slong;
  case CTR_CTYPE_LONGDOUBLE:
    return &ffi_type_longdouble;
  case CTR_CTYPE_STRING:
  case CTR_CTYPE_FUNCTION_POINTER:
  case CTR_CTYPE_DYN_LIB:
  case CTR_CTYPE_POINTER:
    return &ffi_type_pointer;

  case CTR_CTYPE_STRUCT:
  default:
    return NULL;
  }
}

ctr_ctype ctr_ctypes_ffi_convert_ffi_type_to_ctype(ffi_type *type) {
  if (type == &ffi_type_void)
    return CTR_CTYPE_VOID;
  else if (type == &ffi_type_uint8)
    return CTR_CTYPE_UINT8;
  else if (type == &ffi_type_sint8)
    return CTR_CTYPE_SINT8;
  else if (type == &ffi_type_uint16)
    return CTR_CTYPE_UINT16;
  else if (type == &ffi_type_sint16)
    return CTR_CTYPE_SINT16;
  else if (type == &ffi_type_uint32)
    return CTR_CTYPE_UINT32;
  else if (type == &ffi_type_sint32)
    return CTR_CTYPE_SINT32;
  else if (type == &ffi_type_uint64)
    return CTR_CTYPE_UINT64;
  else if (type == &ffi_type_sint64)
    return CTR_CTYPE_SINT64;
  else if (type == &ffi_type_float)
    return CTR_CTYPE_FLOAT;
  else if (type == &ffi_type_double)
    return CTR_CTYPE_DOUBLE;
  else if (type == &ffi_type_uchar)
    return CTR_CTYPE_UCHAR;
  else if (type == &ffi_type_schar)
    return CTR_CTYPE_SCHAR;
  else if (type == &ffi_type_ushort)
    return CTR_CTYPE_USHORT;
  else if (type == &ffi_type_sshort)
    return CTR_CTYPE_SSHORT;
  else if (type == &ffi_type_uint)
    return CTR_CTYPE_UINT;
  else if (type == &ffi_type_sint)
    return CTR_CTYPE_SINT;
  else if (type == &ffi_type_ulong)
    return CTR_CTYPE_ULONG;
  else if (type == &ffi_type_slong)
    return CTR_CTYPE_SLONG;
  else if (type == &ffi_type_longdouble)
    return CTR_CTYPE_LONGDOUBLE;
  else if (type == &ffi_type_pointer)
    return CTR_CTYPE_POINTER;
  return CTR_CTYPE_INVALID;
}

ctr_object *ctr_ctypes_convert_ffi_type_to_citron(ffi_arg *value,
                                                  ctr_ctype type) {
  // printf("p %d\n", type);
  ctr_object *object =
      nudispatch((char *)value, ctr_ctypes_ffi_convert_ctype_to_ffi_type(type));
  return object;
}

CTR_CT_FFI_BIND(cif_destruct) { // free the cif
  ctr_heap_free(
      myself->value.rvalue
          ->ptr); // Free the cif, the resource will be collected by the GC
  myself->value.rvalue->ptr = NULL;
  return myself;
}

CTR_CT_FFI_BIND(cif_new) { //^External CIF ptr
  ffi_cif *cif = ctr_heap_allocate(sizeof(ffi_cif));
  ctr_object *cifobj = ctr_internal_create_object(CTR_OBJECT_TYPE_OTEX);
  ctr_ctypes_set_type(cifobj, CTR_CTYPE_CIF);
  ctr_set_link_all(cifobj, CtrStdCType_ffi_cif);
  cifobj->value.rvalue->ptr = (void *)cif;
  ctr_internal_object_set_property(
      cifobj, ctr_build_string_from_cstring(":cfnptr"), ctr_build_bool(0), 0);
  return cifobj;
}

CTR_CT_FFI_BIND(prep_cif) { // cif*, int<abi>, type* rtype, type** atypes
  ffi_cif *cif_res = (ffi_cif *)(myself->value.rvalue->ptr);
  int abi_mask = (int)(argumentList->object->value.nvalue);
  ffi_type *rtype =
      (ctr_ctypes_ffi_convert_to_ffi_type(argumentList->next->object));
  ctr_object *atypes_ = argumentList->next->next->object;
  int asize = (int)(ctr_array_count(atypes_, NULL)->value.nvalue);
  ffi_type **atypes = ctr_heap_allocate(sizeof(ffi_type) * asize);
  ctr_argument *args = ctr_heap_allocate(sizeof(ctr_argument));
  for (int i = 0; i < asize; i++) {
    args->object = ctr_build_number_from_float(i);
    atypes[i] =
        ctr_ctypes_ffi_convert_to_ffi_type(ctr_array_get(atypes_, args));
  }
  ffi_abi abi;
  switch (abi_mask) {
  default:
    abi = FFI_DEFAULT_ABI;
    break;
  }
  ffi_status status = ffi_prep_cif(cif_res, abi, asize, rtype, atypes);
  ctr_heap_free(args);
  if (status != FFI_OK)
    CtrStdFlow = ctr_build_string_from_cstring(
        (status == FFI_BAD_ABI ? "FFI_BAD_ABI" : "FFI_ERROR"));
  else {
    ctr_internal_object_set_property(
        myself, ctr_build_string_from_cstring(":crType"),
        ctr_build_number_from_float(
            ctr_ctypes_ffi_convert_to_citron_ctype(argumentList->next->object)),
        0);
  }
  return myself;
}

CTR_CT_FFI_BIND(prep_cif_inferred) { //[*atypes, rtype]
  ctr_object *cifobj = ctr_internal_create_object(CTR_OBJECT_TYPE_OTEX);
  ffi_cif *cif_res = ctr_heap_allocate(sizeof(ffi_cif));
  ctr_object *atypes_ = argumentList->object;
  int asize = ((int)(ctr_array_count(atypes_, NULL)->value.nvalue)) - 1;
  ffi_type **atypes = ctr_heap_allocate(sizeof(ffi_type) * asize);
  ctr_argument *args = ctr_heap_allocate(sizeof(ctr_argument));
  int i = 0;
  ffi_type *rtype;
  ffi_type *ty;
  ctr_ctypes_set_type(cifobj, CTR_CTYPE_CIF);
  ctr_set_link_all(cifobj, CtrStdCType_ffi_cif);
  cifobj->value.rvalue->ptr = cif_res;
  int error = 0;
  for (i = 0; i < asize; i++) {
    args->object = ctr_build_number_from_float(i);
    ty = ctr_ctype_citron_object_try_infer_type(ctr_array_get(atypes_, args));
    if (ty == NULL) {
      error = 5; // ERROR_INFERRED
      goto error_out_inf;
    }
    atypes[i] = ty;
  }
  args->object = ctr_build_number_from_float(i);
  ty = ctr_ctype_citron_object_try_infer_type(ctr_array_get(atypes_, args));
  if (ty == NULL) {
    error = 5; // ERROR_INFERRED
    goto error_out_inf;
  }
  rtype = ty;
  ffi_abi abi = FFI_DEFAULT_ABI;
  ffi_status status = ffi_prep_cif(cif_res, abi, asize, rtype, atypes);
error_out_inf:;
  ctr_heap_free(args);
  if (error == 5) {
    static char ERROR_STR[1024];
    size_t slen =
        sprintf(ERROR_STR, "Type inferrence failed for %s (argument#%d)",
                i == asize ? "the return value" : "an argument", i);
    CtrStdFlow = ctr_build_string(ERROR_STR, slen);
    return myself;
  }
  if (status != FFI_OK)
    CtrStdFlow = ctr_build_string_from_cstring(
        (status == FFI_BAD_ABI ? "FFI_BAD_ABI" : "FFI_ERROR"));
  else {
    ctr_internal_object_set_property(
        cifobj, ctr_build_string_from_cstring(":crType"),
        ctr_build_number_from_float(
            ctr_ctypes_ffi_convert_ffi_type_to_ctype(rtype)),
        0);
  }
  return cifobj;
}

CTR_CT_FFI_BIND(cif_arg_count) { //<cif; ^Nil|Number
  ffi_cif *cif = (ffi_cif *)(myself->value.rvalue->ptr);
  if (!cif) {
    CtrStdFlow =
        ctr_build_string_from_cstring("Invalid request, CIF does not exist");
    return CtrStdNil;
  }
  return ctr_build_number_from_float(cif->nargs);
}

CTR_CT_FFI_BIND(cif_is_fnptr) { //<cif; ^Bool
  ffi_cif *cif = (ffi_cif *)(myself->value.rvalue->ptr);
  if (!cif) {
    CtrStdFlow =
        ctr_build_string_from_cstring("Invalid request, CIF does not exist");
    return CtrStdNil;
  }
  return ctr_internal_object_find_property(
      myself, ctr_build_string_from_cstring(":cfnptr"), 0);
}

CTR_CT_FFI_BIND(cif_arg_at) { //<cif, Number; ^CType
  ffi_cif *cif = (ffi_cif *)(myself->value.rvalue->ptr);
  if (!cif) {
    CtrStdFlow =
        ctr_build_string_from_cstring("Invalid request, CIF does not exist");
    return CtrStdNil;
  }
  int i = argumentList->object->value.nvalue;
  if (i >= cif->nargs || i < 0) {
    CtrStdFlow = ctr_build_string_from_cstring("argument index out of bounds");
    return CtrStdNil;
  }
  return ctr_build_number_from_float(
      ctr_ctypes_ffi_convert_ffi_type_to_ctype(cif->arg_types[i]));
}

CTR_CT_FFI_BIND(
    call) { //<cif, CTypes pointer (fn), Array avalues; ^Citron object
  ffi_arg *result = ctr_heap_allocate(sizeof(ffi_arg));
  ffi_cif *cif = (ffi_cif *)(myself->value.rvalue->ptr);
  ctr_object *avals_ = argumentList->next->object;
  int asize = (int)(ctr_array_count(avals_, NULL)->value.nvalue);
  void **avals = ctr_heap_allocate(sizeof(void *) * asize);
  ctr_argument *args = ctr_heap_allocate(sizeof(ctr_argument));
  void (*fn)(void) = (void (*)(void))(argumentList->object->value.rvalue->ptr);
  char **buffers = ctr_heap_allocate(sizeof(char *) * asize);
  void **aval_fnptrs = ctr_heap_allocate(sizeof(void *) * asize);
  for (int i = 0; i < asize; i++) {
    aval_fnptrs[i] = NULL;
    args->object = ctr_build_number_from_float(i);
    ctr_object *obj = ctr_array_get(avals_, args);
    ffi_type *type = cif->arg_types[i];
    if (type->type == FFI_TYPE_STRUCT)
      avals[i] =
          ((ctr_ctypes_ffi_struct_value *)(obj->value.rvalue->ptr))->value;
    else if (type == &ffi_type_pointer &&
             obj->info.type == CTR_OBJECT_TYPE_OTBLOCK) {
      void **bound_f = ctr_heap_allocate(sizeof(void *));
      ffi_closure *closure = ffi_closure_alloc(sizeof(ffi_closure), bound_f);
      ctr_object *typeinfo = ctr_internal_object_find_property(
          (ctr_object *)obj, ctr_build_string_from_cstring(":cCallInterface"),
          0);                                     // CIF
      ffi_cif *cif = typeinfo->value.rvalue->ptr; // TODO Check for type
      if (!cif)
        goto error_out;
      if (ffi_prep_closure_loc(closure, cif, ctr_run_function_ptr, obj,
                               bound_f) == FFI_OK) {
        aval_fnptrs[i] = closure;
        avals[i] = bound_f;
      } else {
      error_out:;
        for (int j = 0; j < asize; j++) {
          if (aval_fnptrs[j])
            ffi_closure_free(aval_fnptrs[j]);
        }
        ctr_heap_free(avals);
        for (int i = 0; i < asize; i++) {
          if (buffers[i])
            ctr_heap_free_shared(buffers[i]);
        }
        ctr_heap_free(buffers);
        CtrStdFlow = ctr_build_string_from_cstring("Could not create closure");
        return CtrStdNil;
      }
    } else {
      char *buf = ctr_heap_allocate_shared(ctr_ctype_get_c_size(obj));
      buffers[i] = buf;
      npdispatch(buf, obj, type);
      avals[i] = (void *)buf;
    }
  }
  ffi_call(cif, FFI_FN(fn), result, avals);
  for (int j = 0; j < asize; j++) {
    if (aval_fnptrs[j])
      ffi_closure_free(aval_fnptrs[j]);
  }
  ctr_heap_free(avals);
  for (int i = 0; i < asize; i++) {
    if (buffers[i])
      ctr_heap_free_shared(buffers[i]);
  }
  ctr_heap_free(buffers);
  return ctr_ctypes_convert_ffi_type_to_citron(result,
                                               (ctr_ctypes_rtypeof(myself)));
}

CTR_CT_FFI_BIND(ll) {
  char *name;
  if (argumentList->object == CtrStdNil)
    name = NULL;
  else
    name = ctr_heap_allocate_cstring(argumentList->object);
  int mode = RTLD_NOW | RTLD_GLOBAL; // TODO Implement selecting modes
  // if(argumentList->next->object != CtrStdNil) {
  //  mode =
  //}
  void *handle = dlopen(name, mode);
  if (!handle) {
    CtrStdFlow = ctr_build_string_from_cstring(dlerror());
    return ctr_build_nil();
  }
  CTR_CREATE_CTOBJECT(object);
  ctr_ctypes_set_type(object, CTR_CTYPE_DYN_LIB);
  object->value.rvalue->ptr = handle;
  if (name != NULL)
    ctr_heap_free(name);
  return object;
}

CTR_CT_FFI_BIND(malloc) {
  int size = ctr_internal_cast2number(argumentList->object)->value.nvalue;
  void *resource = malloc(size);
  ctr_object *object = ctr_ctypes_make_pointer(NULL, NULL);
  object->value.rvalue->ptr = resource;
  return object;
}

CTR_CT_FFI_BIND(shmalloc) {
  int size = ctr_internal_cast2number(argumentList->object)->value.nvalue;
  void *resource = ctr_heap_allocate_shared(size);
  ctr_object *object = ctr_ctypes_make_pointer(NULL, NULL);
  object->value.rvalue->ptr = resource;
  return object;
}

CTR_CT_FFI_BIND(memcpy) { // dest, source, count
  void *r0 = argumentList->object->value.rvalue->ptr,
       *r1 = argumentList->next->object->value.rvalue->ptr;
  int count = argumentList->next->next->object->value.nvalue;
  memcpy(r0, r1, count);
  return myself;
}

CTR_CT_FFI_BIND(fill_buf) { // dest, source
  void *r0 = argumentList->object->value.rvalue->ptr;
  ctr_object *c1 = ctr_internal_cast2string(argumentList->next->object);
  char *cstring = ctr_heap_allocate_cstring(c1);
  size_t *block_width;
  int q = sizeof(size_t);
  size_t size;

  /* find the correct size of this memory block and move pointer back */
  r0 = (void *)((char *)r0 - q);
  block_width = (size_t *)r0;
  size = *(block_width);
  /* Then just put the thing back where it belongs */
  r0 = (void *)((char *)r0 + q);
  int count = fmin(strlen(cstring), (int)size) - 1;
  for (int i = 0; i < count; i++)
    *((char *)r0 + i) = cstring[i];
  *(((char *)r0) + count + 1) = '\0'; // NULL byte for proper c-string
  ctr_heap_free(cstring);
  return myself;
}

CTR_CT_FFI_BIND(buf_from_str) {
  ctr_object *str = ctr_internal_cast2string(argumentList->object);
  ctr_object *object = ctr_ctypes_make_pointer(NULL, NULL);
  object->value.rvalue->ptr = ctr_heap_allocate_cstring_shared(str);
  return object;
}

CTR_CT_FFI_BIND(free) {
  if ((argumentList->object->info.type) != CTR_OBJECT_TYPE_OTEX) {
    CtrStdFlow = ctr_build_string_from_cstring("Can only free resources.");
    return CtrStdNil;
  }
  free(argumentList->object->value.rvalue->ptr);
  argumentList->object->value.rvalue->ptr = NULL;
  return myself;
}

CTR_CT_FFI_BIND(shfree) {
  if ((argumentList->object->info.type) != CTR_OBJECT_TYPE_OTEX) {
    CtrStdFlow = ctr_build_string_from_cstring("Can only free resources.");
    return CtrStdNil;
  }
  ctr_heap_free_shared(argumentList->object->value.rvalue->ptr);
  argumentList->object->value.rvalue->ptr = NULL;
  return myself;
}

// TODO Fix memory leak
ctr_object *ctr_ctype_ffi_closure_cif(ctr_object *myself,
                                      ctr_argument *argumentList) {
  ctr_object *fun = argumentList->object;
  ffi_cif *cif = argumentList->next->object->value.rvalue->ptr;
  if (!(fun && cif)) {
    CtrStdFlow = ctr_build_string_from_cstring("NULL function or CIF");
    return CtrStdNil;
  }
  void **bound_f = ctr_heap_allocate(sizeof(void *));
  ffi_closure *closure = ffi_closure_alloc(sizeof(ffi_closure), bound_f);

  if (ffi_prep_closure_loc(closure, cif, ctr_run_function_ptr, fun, bound_f) !=
      FFI_OK) {
    CtrStdFlow = ctr_build_string_from_cstring("Could not create closure");
    return CtrStdNil;
  }

  ctr_gc_pin(fun);
  ctr_gc_pin(cif);
  ctr_gc_pin(bound_f);

  ctr_object *fn = ctr_ctypes_make_pointer(CtrStdCType_pointer, NULL);
  fn->value.rvalue->ptr = *bound_f;
  return fn;
}

// Citron object String (pretty pointer)
CTR_CT_SIMPLE_TYPE_FUNC_UNMAKE(string) {
  // no touching the pointee
  return myself;
}

CTR_CT_SIMPLE_TYPE_FUNC_MAKE(string) {
  CTR_CREATE_CTOBJECT(object);
  ctr_ctypes_set_type(object, CTR_CTYPE_STRING); // HALP
  return object;
}

CTR_CT_SIMPLE_TYPE_FUNC_GET(string) {
  char *value = myself->value.rvalue->ptr;
  ctr_object *str = ctr_build_string(value, strlen(value));
  return str;
}

CTR_CT_SIMPLE_TYPE_FUNC_SET(string) {
  if (argumentList->object->info.type == CTR_OBJECT_TYPE_OTEX &&
      (argumentList->object->value.rvalue->type == CTR_CTYPE_POINTER ||
       argumentList->object->value.rvalue->type == CTR_CTYPE_STRING)) {
    myself->value.rvalue->ptr = argumentList->object->value.rvalue->ptr;
  } else if (argumentList->object->info.type == CTR_OBJECT_TYPE_OTSTRING) {
    ctr_object *buf = ctr_ctype_ffi_buf_from_str(CtrStdCType, argumentList);
    myself->value.rvalue->ptr = buf->value.rvalue->ptr;
  }

  return myself;
}

CTR_CT_SIMPLE_TYPE_FUNC_STR(string) {
  ctr_object *str_rep = ctr_build_string_from_cstring("<CType string at: ");
  char repr[128];
  sprintf(repr, "%p", myself->value.rvalue->ptr);
  ctr_argument *args = ctr_heap_allocate(sizeof(ctr_argument));
  args->object = ctr_build_string_from_cstring(repr);
  ctr_string_append(str_rep, args);
  args->object = ctr_build_string_from_cstring(">");
  ctr_string_append(str_rep, args);
  ctr_heap_free(args);
  return str_rep;
}

// Citron object Function (pretty pointer)
CTR_CT_SIMPLE_TYPE_FUNC_UNMAKE(functionptr) {
  // no touching the pointee
  return myself;
}

CTR_CT_SIMPLE_TYPE_FUNC_MAKE(functionptr) {
  CTR_CREATE_CTOBJECT(object);
  ctr_ctypes_set_type(object, CTR_CTYPE_FUNCTION_POINTER); // HALP
  return object;
}

CTR_CT_SIMPLE_TYPE_FUNC_GET(functionptr) {
  if (myself->value.rvalue->ptr)
    return (ctr_object *)(myself->value.rvalue->ptr);
  else
    return CtrStdNil;
}

CTR_CT_SIMPLE_TYPE_FUNC_SET(functionptr) {
  if (argumentList->object->info.type == CTR_OBJECT_TYPE_OTEX &&
      argumentList->object->value.rvalue->type == CTR_CTYPE_FUNCTION_POINTER) {
    myself->value.rvalue->ptr = argumentList->object->value.rvalue->ptr;
  } else if (argumentList->object->info.type == CTR_OBJECT_TYPE_OTBLOCK) {
    myself->value.rvalue->ptr = argumentList->object;
    argumentList->object->info.mark = 1;
  }

  return myself;
}

CTR_CT_SIMPLE_TYPE_FUNC_STR(functionptr) {
  ctr_object *str_rep =
      ctr_build_string_from_cstring("<CType function pointer at: ");
  char repr[128];
  sprintf(repr, "%p", myself->value.rvalue->ptr);
  ctr_argument *args = ctr_heap_allocate(sizeof(ctr_argument));
  args->object = ctr_build_string_from_cstring(repr);
  ctr_string_append(str_rep, args);
  args->object = ctr_build_string_from_cstring(">");
  ctr_string_append(str_rep, args);
  ctr_heap_free(args);
  return str_rep;
}

void ctr_ffi_begin() {
  CtrStdCType = ctr_internal_create_object(CTR_OBJECT_TYPE_OTOBJECT);
  ctr_set_link_all(CtrStdCType, CtrStdObject);
  CtrStdCType->info.sticky = 1;
  CtrStdCType_ffi_cif = ctr_internal_create_object(CTR_OBJECT_TYPE_OTOBJECT);
  ctr_set_link_all(CtrStdCType_ffi_cif, CtrStdObject);
  CtrStdCType_ffi_cif->info.sticky = 1;
  ctr_internal_create_func(CtrStdCType, ctr_build_string_from_cstring("newIns"),
                           &ctr_ctype_generic_copy);
  // Void
  CTR_CT_INTRODUCE_TYPE(void);
  CTR_CT_INTRODUCE_TSTR(void);
  CTR_CT_INTRODUCE_SET(void);
  CTR_CT_INTRODUCE_GET(void);
  CTR_CT_INTRODUCE_MAKE(void);
  CTR_CT_INTRODUCE_UNMAKE(void);
  // Unsigned Int 8
  CTR_CT_INTRODUCE_TYPE(uint8);
  CTR_CT_INTRODUCE_TSTR(uint8);
  CTR_CT_INTRODUCE_SET(uint8);
  CTR_CT_INTRODUCE_GET(uint8);
  CTR_CT_INTRODUCE_MAKE(uint8);
  CTR_CT_INTRODUCE_UNMAKE(uint8);
  // Signed Int 8
  CTR_CT_INTRODUCE_TYPE(sint8);
  CTR_CT_INTRODUCE_TSTR(sint8);
  CTR_CT_INTRODUCE_SET(sint8);
  CTR_CT_INTRODUCE_GET(sint8);
  CTR_CT_INTRODUCE_MAKE(sint8);
  CTR_CT_INTRODUCE_UNMAKE(sint8);
  // Unsigned Int 16
  CTR_CT_INTRODUCE_TYPE(uint16);
  CTR_CT_INTRODUCE_TSTR(uint16);
  CTR_CT_INTRODUCE_SET(uint16);
  CTR_CT_INTRODUCE_GET(uint16);
  CTR_CT_INTRODUCE_MAKE(uint16);
  CTR_CT_INTRODUCE_UNMAKE(uint16);
  // Signed Int 16
  CTR_CT_INTRODUCE_TYPE(sint16);
  CTR_CT_INTRODUCE_TSTR(sint16);
  CTR_CT_INTRODUCE_SET(sint16);
  CTR_CT_INTRODUCE_GET(sint16);
  CTR_CT_INTRODUCE_MAKE(sint16);
  CTR_CT_INTRODUCE_UNMAKE(sint16);
  // Unsigned Int 32
  CTR_CT_INTRODUCE_TYPE(uint32);
  CTR_CT_INTRODUCE_TSTR(uint32);
  CTR_CT_INTRODUCE_SET(uint32);
  CTR_CT_INTRODUCE_GET(uint32);
  CTR_CT_INTRODUCE_MAKE(uint32);
  CTR_CT_INTRODUCE_UNMAKE(uint32);
  // Signed Int 32
  CTR_CT_INTRODUCE_TYPE(sint32);
  CTR_CT_INTRODUCE_TSTR(sint32);
  CTR_CT_INTRODUCE_SET(sint32);
  CTR_CT_INTRODUCE_GET(sint32);
  CTR_CT_INTRODUCE_MAKE(sint32);
  CTR_CT_INTRODUCE_UNMAKE(sint32);
  // Unsigned Int 64
  CTR_CT_INTRODUCE_TYPE(uint64);
  CTR_CT_INTRODUCE_TSTR(uint64);
  CTR_CT_INTRODUCE_SET(uint64);
  CTR_CT_INTRODUCE_GET(uint64);
  CTR_CT_INTRODUCE_MAKE(uint64);
  CTR_CT_INTRODUCE_UNMAKE(uint64);
  // Signed Int 64
  CTR_CT_INTRODUCE_TYPE(sint64);
  CTR_CT_INTRODUCE_TSTR(sint64);
  CTR_CT_INTRODUCE_SET(sint64);
  CTR_CT_INTRODUCE_GET(sint64);
  CTR_CT_INTRODUCE_MAKE(sint64);
  CTR_CT_INTRODUCE_UNMAKE(sint64);
  // Float
  CTR_CT_INTRODUCE_TYPE(float);
  CTR_CT_INTRODUCE_TSTR(float);
  CTR_CT_INTRODUCE_SET(float);
  CTR_CT_INTRODUCE_GET(float);
  CTR_CT_INTRODUCE_MAKE(float);
  CTR_CT_INTRODUCE_UNMAKE(float);
  // Double
  CTR_CT_INTRODUCE_TYPE(double);
  CTR_CT_INTRODUCE_TSTR(double);
  CTR_CT_INTRODUCE_SET(double);
  CTR_CT_INTRODUCE_GET(double);
  CTR_CT_INTRODUCE_MAKE(double);
  CTR_CT_INTRODUCE_UNMAKE(double);
  // Signed Char
  CTR_CT_INTRODUCE_TYPE(schar);
  CTR_CT_INTRODUCE_TSTR(schar);
  CTR_CT_INTRODUCE_SET(schar);
  CTR_CT_INTRODUCE_GET(schar);
  CTR_CT_INTRODUCE_MAKE(schar);
  CTR_CT_INTRODUCE_UNMAKE(schar);
  // Unsigned Char
  CTR_CT_INTRODUCE_TYPE(uchar);
  CTR_CT_INTRODUCE_TSTR(uchar);
  CTR_CT_INTRODUCE_SET(uchar);
  CTR_CT_INTRODUCE_GET(uchar);
  CTR_CT_INTRODUCE_MAKE(uchar);
  CTR_CT_INTRODUCE_UNMAKE(uchar);
  // Signed Short
  CTR_CT_INTRODUCE_TYPE(sshort);
  CTR_CT_INTRODUCE_TSTR(sshort);
  CTR_CT_INTRODUCE_SET(sshort);
  CTR_CT_INTRODUCE_GET(sshort);
  CTR_CT_INTRODUCE_MAKE(sshort);
  CTR_CT_INTRODUCE_UNMAKE(sshort);
  // Unsigned Short
  CTR_CT_INTRODUCE_TYPE(ushort);
  CTR_CT_INTRODUCE_TSTR(ushort);
  CTR_CT_INTRODUCE_SET(ushort);
  CTR_CT_INTRODUCE_GET(ushort);
  CTR_CT_INTRODUCE_MAKE(ushort);
  CTR_CT_INTRODUCE_UNMAKE(ushort);
  // Signed Int
  CTR_CT_INTRODUCE_TYPE(sint);
  CTR_CT_INTRODUCE_TSTR(sint);
  CTR_CT_INTRODUCE_SET(sint);
  CTR_CT_INTRODUCE_GET(sint);
  CTR_CT_INTRODUCE_MAKE(sint);
  CTR_CT_INTRODUCE_UNMAKE(sint);
  // Unsigned Int
  CTR_CT_INTRODUCE_TYPE(uint);
  CTR_CT_INTRODUCE_TSTR(uint);
  CTR_CT_INTRODUCE_SET(uint);
  CTR_CT_INTRODUCE_GET(uint);
  CTR_CT_INTRODUCE_MAKE(uint);
  CTR_CT_INTRODUCE_UNMAKE(uint);
  // Signed Long
  CTR_CT_INTRODUCE_TYPE(slong);
  CTR_CT_INTRODUCE_TSTR(slong);
  CTR_CT_INTRODUCE_SET(slong);
  CTR_CT_INTRODUCE_GET(slong);
  CTR_CT_INTRODUCE_MAKE(slong);
  CTR_CT_INTRODUCE_UNMAKE(slong);
  // Unsigned Long
  CTR_CT_INTRODUCE_TYPE(ulong);
  CTR_CT_INTRODUCE_TSTR(ulong);
  CTR_CT_INTRODUCE_SET(ulong);
  CTR_CT_INTRODUCE_GET(ulong);
  CTR_CT_INTRODUCE_MAKE(ulong);
  CTR_CT_INTRODUCE_UNMAKE(ulong);
  // Long Double
  CTR_CT_INTRODUCE_TYPE(longdouble);

  CTR_CT_INTRODUCE_SET(longdouble);
  CTR_CT_INTRODUCE_GET(longdouble);
  CTR_CT_INTRODUCE_MAKE(longdouble);
  CTR_CT_INTRODUCE_UNMAKE(longdouble);
  // Pointer
  CTR_CT_INTRODUCE_TYPE(pointer);
  CTR_CT_INTRODUCE_SET(pointer);
  CTR_CT_INTRODUCE_GET(pointer);
  ctr_internal_create_func(CtrStdCType_pointer,
                           ctr_build_string_from_cstring("toString"),
                           &ctr_ctypes_str_pointer);
  ctr_internal_create_func(CtrStdCType_pointer,
                           ctr_build_string_from_cstring("derefAs:"),
                           &ctr_ctypes_deref_pointer);
  ctr_internal_create_func(CtrStdCType_pointer,
                           ctr_build_string_from_cstring("derefSet:"),
                           &ctr_ctypes_set_internal_pointer);
  ctr_internal_create_func(CtrStdCType_pointer,
                           ctr_build_string_from_cstring("readBytes:"),
                           &ctr_ctypes_to_bytes);
  ctr_internal_create_func(CtrStdCType_pointer,
                           ctr_build_string_from_cstring("readBytesAsString:"),
                           &ctr_ctypes_to_bytes_string);
  ctr_internal_create_func(CtrStdCType_pointer,
                           ctr_build_string_from_cstring("readCString"),
                           &ctr_ctypes_read_string);
  ctr_internal_create_func(CtrStdCType,
                           ctr_build_string_from_cstring("getAddress"),
                           &ctr_ctypes_addr_of);
  ctr_internal_create_func(CtrStdCType,
                           ctr_build_string_from_cstring("getRawAddress"),
                           &ctr_ctypes_addr_of_raw);
  ctr_internal_create_func(CtrStdCType_pointer,
                           ctr_build_string_from_cstring("addressOfObject:"),
                           &ctr_ctypes_pointer_set_obj);
  ctr_internal_create_func(CtrStdCType_pointer,
                           ctr_build_string_from_cstring("asObject"),
                           &ctr_ctypes_pointer_get_obj);
  ctr_internal_create_func(CtrStdCType_pointer,
                           ctr_build_string_from_cstring("+"),
                           &ctr_ctypes_pointer_arith_add);
  ctr_internal_create_func(CtrStdCType_pointer,
                           ctr_build_string_from_cstring("-"),
                           &ctr_ctypes_pointer_arith_sub);
  CTR_CT_INTRODUCE_MAKE(pointer);
  CTR_CT_INTRODUCE_UNMAKE(pointer);

  CTR_CT_INTRODUCE_TYPE(string);
  CTR_CT_INTRODUCE_SET(string);
  CTR_CT_INTRODUCE_GET(string);
  CTR_CT_INTRODUCE_MAKE(string);
  ctr_internal_create_func(CtrStdCType_string,
                           ctr_build_string_from_cstring("toString"),
                           &ctr_ctypes_str_string);

  // Dynamic Library
  CTR_CT_INTRODUCE_TYPE(dynamic_lib);
  ctr_internal_create_func(CtrStdCType_dynamic_lib,
                           ctr_build_string_from_cstring("respondTo:"),
                           &ctr_ctypes_get_dynamic_lib);
  ctr_internal_create_func(CtrStdCType_dynamic_lib,
                           ctr_build_string_from_cstring("toString"),
                           &ctr_ctypes_str_dynamic_lib);
  CTR_CT_INTRODUCE_MAKE(dynamic_lib);
  CTR_CT_INTRODUCE_UNMAKE(dynamic_lib);

  // C Struct
  CTR_CT_INTRODUCE_TYPE(struct);
  ctr_internal_create_func(CtrStdCType,
                           ctr_build_string_from_cstring("structWithFormat:"),
                           &ctr_ctypes_make_struct);
  ctr_internal_create_func(CtrStdCType_struct,
                           ctr_build_string_from_cstring("toString"),
                           &ctr_ctypes_struct_to_string);
  ctr_internal_create_func(CtrStdCType_struct,
                           ctr_build_string_from_cstring("new"),
                           &ctr_ctypes_struct_new);
  ctr_internal_create_func(CtrStdCType_struct,
                           ctr_build_string_from_cstring("newIns"),
                           &ctr_ctypes_struct_new);
  ctr_internal_create_func(CtrStdCType_struct,
                           ctr_build_string_from_cstring("getSize"),
                           &ctr_ctypes_struct_get_size);
  ctr_internal_create_func(CtrStdCType_struct,
                           ctr_build_string_from_cstring("memberCount"),
                           &ctr_ctypes_struct_get_member_count);
  ctr_internal_create_func(CtrStdCType_struct,
                           ctr_build_string_from_cstring("padInfo"),
                           &ctr_ctypes_struct_get_padding_format);
  ctr_internal_create_func(CtrStdCType_struct,
                           ctr_build_string_from_cstring("offsets"),
                           &ctr_ctypes_struct_get_offset_format);
  ctr_internal_create_func(CtrStdCType_struct,
                           ctr_build_string_from_cstring("pack:"),
                           &ctr_ctypes_pack_struct);
  ctr_internal_create_func(CtrStdCType_struct,
                           ctr_build_string_from_cstring("unpack"),
                           &ctr_ctypes_unpack_struct);
  ctr_internal_create_func(CtrStdCType_struct,
                           ctr_build_string_from_cstring("at:"),
                           &ctr_ctypes_unpack_struct_at);
  ctr_internal_create_func(CtrStdCType_struct,
                           ctr_build_string_from_cstring("put:at:"),
                           &ctr_ctypes_pack_struct_at);
  ctr_internal_create_func(CtrStdCType_struct,
                           ctr_build_string_from_cstring("destruct"),
                           &ctr_ctypes_unmake_struct);
  ctr_internal_create_func(CtrStdCType_struct,
                           ctr_build_string_from_cstring("allocate"),
                           &ctr_ctypes_struct_allocate);
  ctr_internal_create_func(CtrStdCType_struct,
                           ctr_build_string_from_cstring("setFrom:"),
                           &ctr_ctypes_struct_set_from);

  ctr_internal_create_func(CtrStdCType_ffi_cif,
                           ctr_build_string_from_cstring("new"),
                           &ctr_ctype_ffi_cif_new);
  ctr_internal_create_func(CtrStdCType_ffi_cif,
                           ctr_build_string_from_cstring("destruct"),
                           &ctr_ctype_ffi_cif_destruct);
  ctr_internal_create_func(
      CtrStdCType_ffi_cif,
      ctr_build_string_from_cstring("setABI:return:argTypes:"),
      &ctr_ctype_ffi_prep_cif);
  ctr_internal_create_func(
      CtrStdCType_ffi_cif,
      ctr_build_string_from_cstring("newByInferringTypes:"),
      &ctr_ctype_ffi_prep_cif_inferred);
  ctr_internal_create_func(CtrStdCType_ffi_cif,
                           ctr_build_string_from_cstring("call:withArgs:"),
                           &ctr_ctype_ffi_call);
  ctr_internal_create_func(CtrStdCType_ffi_cif,
                           ctr_build_string_from_cstring("argumentCount"),
                           &ctr_ctype_ffi_cif_arg_count);
  ctr_internal_create_func(CtrStdCType_ffi_cif,
                           ctr_build_string_from_cstring("isFunctionPointer"),
                           &ctr_ctype_ffi_cif_is_fnptr);
  ctr_internal_create_func(CtrStdCType_ffi_cif,
                           ctr_build_string_from_cstring("argumentTypeOfIdx:"),
                           &ctr_ctype_ffi_cif_arg_at);
  CTR_CT_INTRODUCE_TYPE(cont_pointer);
  CTR_CT_INTRODUCE_TSTR(cont_pointer);
  ctr_internal_create_func(CtrStdCType,
                           ctr_build_string_from_cstring("packed:count:"),
                           &ctr_ctypes_make_packed);
  ctr_internal_create_func(CtrStdCType_cont_pointer,
                           ctr_build_string_from_cstring("at:"),
                           &ctr_ctypes_packed_get);
  ctr_internal_create_func(CtrStdCType_cont_pointer,
                           ctr_build_string_from_cstring("put:at:"),
                           &ctr_ctypes_packed_set);
  ctr_internal_create_func(CtrStdCType_cont_pointer,
                           ctr_build_string_from_cstring("count"),
                           &ctr_ctypes_packed_count);
  ctr_internal_create_func(CtrStdCType_cont_pointer,
                           ctr_build_string_from_cstring("_type"),
                           &ctr_ctypes_packed_type);
  ctr_internal_create_func(CtrStdCType_cont_pointer,
                           ctr_build_string_from_cstring("getSize"),
                           &ctr_ctypes_packed_size);
  ctr_internal_create_func(CtrStdCType_cont_pointer,
                           ctr_build_string_from_cstring("new"),
                           &ctr_ctypes_packed_from);
  ctr_internal_create_func(CtrStdCType_cont_pointer,
                           ctr_build_string_from_cstring("newIns"),
                           &ctr_ctypes_packed_from);

  ctr_internal_create_func(CtrStdCType,
                           ctr_build_string_from_cstring("allocateBytes:"),
                           &ctr_ctype_ffi_malloc);
  ctr_internal_create_func(
      CtrStdCType, ctr_build_string_from_cstring("allocateBytesShared:"),
      &ctr_ctype_ffi_shmalloc);
  ctr_internal_create_func(
      CtrStdCType, ctr_build_string_from_cstring("copyTo:from:numBytes:"),
      &ctr_ctype_ffi_memcpy);
  ctr_internal_create_func(CtrStdCType,
                           ctr_build_string_from_cstring("fill:withString:"),
                           &ctr_ctype_ffi_fill_buf);
  ctr_internal_create_func(CtrStdCType, ctr_build_string_from_cstring("free:"),
                           &ctr_ctype_ffi_free);
  ctr_internal_create_func(CtrStdCType,
                           ctr_build_string_from_cstring("freeShared:"),
                           &ctr_ctype_ffi_shfree);
  ctr_internal_create_func(CtrStdCType,
                           ctr_build_string_from_cstring("fromString:"),
                           &ctr_ctype_ffi_buf_from_str);
  ctr_internal_create_func(CtrStdCType,
                           ctr_build_string_from_cstring("loadLibrary:"),
                           &ctr_ctype_ffi_ll);
  ctr_internal_create_func(CtrStdCType,
                           ctr_build_string_from_cstring("closureOf:withCIF:"),
                           &ctr_ctype_ffi_closure_cif);

  ctr_internal_object_add_property(CtrStdWorld,
                                   ctr_build_string_from_cstring("CIF"),
                                   CtrStdCType_ffi_cif, 0);
  ctr_internal_object_add_property(
      CtrStdWorld, ctr_build_string_from_cstring("CTypes"), CtrStdCType, 0);
  // bind initial->lexical_name->value.svalue->value
}
