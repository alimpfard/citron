#ifndef CTR_C_INTERP_H
#define CTR_C_INTERP_H

#include <ffi.h>
#include <stddef.h>
#include <stdint.h>

typedef enum ctr_cinterp_type {
    CTR_CINTERP_T_VOID = 0,
    CTR_CINTERP_T_I8,
    CTR_CINTERP_T_U8,
    CTR_CINTERP_T_I16,
    CTR_CINTERP_T_U16,
    CTR_CINTERP_T_I32,
    CTR_CINTERP_T_U32,
    CTR_CINTERP_T_I64,
    CTR_CINTERP_T_U64,
    CTR_CINTERP_T_F32,
    CTR_CINTERP_T_F64,
    CTR_CINTERP_T_PTR
} ctr_cinterp_type;

typedef union ctr_cinterp_value {
    int64_t i;
    uint64_t u;
    double f;
    void* p;
} ctr_cinterp_value;

typedef struct ctr_cinterp ctr_cinterp;
typedef struct ctr_cinterp_function ctr_cinterp_function;

typedef void (*ctr_cinterp_error_fn)(void* userdata, char const* message);

ctr_cinterp* ctr_cinterp_new(void);
void ctr_cinterp_free(ctr_cinterp* interp);
void ctr_cinterp_set_error_handler(ctr_cinterp* interp, ctr_cinterp_error_fn fn, void* userdata);

int ctr_cinterp_add_library_path(ctr_cinterp* interp, char const* path);
size_t ctr_cinterp_library_path_count(ctr_cinterp const* interp);
char const* ctr_cinterp_library_path_at(ctr_cinterp const* interp, size_t index);
int ctr_cinterp_link_library(ctr_cinterp* interp, char const* name);
int ctr_cinterp_link_symbol(ctr_cinterp* interp, char const* name, void* ptr);
void* ctr_cinterp_get_symbol(ctr_cinterp* interp, char const* name);

int ctr_cinterp_compile_text(ctr_cinterp* interp, char const* source);
int ctr_cinterp_compile_c(ctr_cinterp* interp, char const* source);
ctr_cinterp_function* ctr_cinterp_find_function(ctr_cinterp* interp, char const* name);
int ctr_cinterp_call(ctr_cinterp* interp, char const* name, ctr_cinterp_value* argv, size_t argc, ctr_cinterp_value* ret);
char const* ctr_cinterp_last_error(ctr_cinterp const* interp);
int ctr_cinterp_is_function_pointer(void* ptr);
int ctr_cinterp_call_function_pointer(void* ptr, ctr_cinterp_value* argv, size_t argc, ctr_cinterp_value* ret);
char const* ctr_cinterp_last_error_for_function_pointer(void* ptr);

ffi_type* ctr_cinterp_ffi_type(ctr_cinterp_type type);
size_t ctr_cinterp_type_size(ctr_cinterp_type type);
int ctr_cinterp_parse_type(char const* name, ctr_cinterp_type* out);

#include "citron.h"

CTR_H_DECLSPEC ctr_object* CtrStdCInterp;
void* ctr_cinterp_resource_release_hook(void* state);
ctr_object* ctr_cinterp_make(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_cinterp_compile(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_cinterp_defined_functions(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_cinterp_defined_macros_ctr(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_cinterp_get_symbol_ctr(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_cinterp_run(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_cinterp_run_named(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_cinterp_add_libp_ctr(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_cinterp_get_libp_ctr(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_cinterp_add_inclp_ctr(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_cinterp_get_inclp_ctr(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_cinterp_add_lib_ctr(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_cinterp_set_error_handler_ctr(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_cinterp_generate_output_ctr(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_cinterp_finish(ctr_object* myself, ctr_argument* argumentList);

#endif
