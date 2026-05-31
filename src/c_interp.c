#include "c_interp.h"

#include <ctype.h>
#include <dlfcn.h>
#include <errno.h>
#include <float.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern void ctr_ctypes_set_type(ctr_object* object, ctr_ctype type);
extern ctr_ctype ctr_ctypes_ffi_convert_ffi_type_to_ctype(ffi_type* type);
extern wrapped_ffi_type* ctr_ctypes_ffi_convert_ctype_to_ffi_type(ctr_ctype type);
extern ctr_object* ctr_ctypes_struct_make_internal(ctr_object* myself, char* fmt);
extern ctr_object* ctr_ctypes_make_cont_pointer(ctr_object* myself, ctr_argument* argumentList);

typedef enum ctr_cinterp_op {
    CI_OP_NOP,
    CI_OP_CONST,
    CI_OP_ARG,
    CI_OP_LOCAL,
    CI_OP_STORE_LOCAL,
    CI_OP_GLOBAL_ADDR,
    CI_OP_LOAD,
    CI_OP_STORE,
    CI_OP_ADD,
    CI_OP_SUB,
    CI_OP_MUL,
    CI_OP_DIV,
    CI_OP_MOD,
    CI_OP_NEG,
    CI_OP_EQ,
    CI_OP_NE,
    CI_OP_LT,
    CI_OP_LE,
    CI_OP_GT,
    CI_OP_GE,
    CI_OP_AND,
    CI_OP_OR,
    CI_OP_XOR,
    CI_OP_SHL,
    CI_OP_SHR,
    CI_OP_NOT,
    CI_OP_JMP,
    CI_OP_JZ,
    CI_OP_JNZ,
    CI_OP_CALL,
    CI_OP_CALL_EXT,
    CI_OP_RET,
    CI_OP_POP,
    CI_OP_DUP,
    CI_OP_CAST,
    CI_OP_PTR_ADD,
    CI_OP_MEMCPY,
    CI_OP_MEMSET
} ctr_cinterp_op;

typedef struct ctr_cinterp_instr {
    ctr_cinterp_op op;
    ctr_cinterp_type type;
    ctr_cinterp_type type2;
    ctr_cinterp_value imm;
    int a;
    int b;
    char* name;
} ctr_cinterp_instr;

struct ctr_cinterp_function {
    uint64_t magic;
    ctr_cinterp* owner;
    char* name;
    size_t argc;
    size_t nlocals;
    ctr_cinterp_type ret_type;
    ctr_cinterp_instr* code;
    size_t code_count;
    size_t code_cap;
};

#define CTR_CINTERP_FUNCTION_MAGIC UINT64_C(0x4354524349464e31)

typedef struct ctr_cinterp_external {
    char* name;
    void* ptr;
    ctr_cinterp_type ret_type;
    ctr_cinterp_type* arg_types;
    size_t argc;
    ffi_cif cif;
    ffi_type** ffi_arg_types;
    int cif_ready;
} ctr_cinterp_external;

typedef struct ctr_cinterp_global {
    char* name;
    void* data;
    size_t size;
} ctr_cinterp_global;

typedef struct ctr_cinterp_macro {
    char* name;
    char* value;
    char* param;
    int function_like;
} ctr_cinterp_macro;

typedef struct ctr_cinterp_ctype {
    char* name;
    char* format;
    char** field_names;
    size_t field_count;
    size_t array_count;
} ctr_cinterp_ctype;

#define CI_STR2(x) #x
#define CI_STR(x) CI_STR2(x)

struct ctr_cinterp {
    ctr_cinterp_function* functions;
    size_t function_count;
    size_t function_cap;
    ctr_cinterp_external* externals;
    size_t external_count;
    size_t external_cap;
    ctr_cinterp_global* globals;
    size_t global_count;
    size_t global_cap;
    ctr_cinterp_macro* macros;
    size_t macro_count;
    size_t macro_cap;
    ctr_cinterp_ctype* ctypes;
    size_t ctype_count;
    size_t ctype_cap;
    char** library_paths;
    size_t library_path_count;
    size_t library_path_cap;
    void** library_handles;
    size_t library_handle_count;
    size_t library_handle_cap;
    ctr_cinterp_error_fn error_fn;
    void* error_userdata;
};

static char* ci_strdup(char const* s)
{
    if (!s)
        return NULL;
    size_t len = strlen(s) + 1;
    char* out = malloc(len);
    if (out)
        memcpy(out, s, len);
    return out;
}

static void ci_error(ctr_cinterp* interp, char const* fmt, ...)
{
    char buf[1024];
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(buf, sizeof(buf), fmt, ap);
    va_end(ap);
    if (interp && interp->error_fn)
        interp->error_fn(interp->error_userdata, buf);
}

static int ci_grow(void** ptr, size_t* cap, size_t count, size_t elem_size)
{
    if (count < *cap)
        return 1;
    size_t new_cap = *cap ? *cap * 2 : 8;
    void* p = realloc(*ptr, new_cap * elem_size);
    if (!p)
        return 0;
    *ptr = p;
    *cap = new_cap;
    return 1;
}

ffi_type* ctr_cinterp_ffi_type(ctr_cinterp_type type)
{
    switch (type) {
    case CTR_CINTERP_T_VOID:
        return &ffi_type_void;
    case CTR_CINTERP_T_I8:
        return &ffi_type_sint8;
    case CTR_CINTERP_T_U8:
        return &ffi_type_uint8;
    case CTR_CINTERP_T_I16:
        return &ffi_type_sint16;
    case CTR_CINTERP_T_U16:
        return &ffi_type_uint16;
    case CTR_CINTERP_T_I32:
        return &ffi_type_sint32;
    case CTR_CINTERP_T_U32:
        return &ffi_type_uint32;
    case CTR_CINTERP_T_I64:
        return &ffi_type_sint64;
    case CTR_CINTERP_T_U64:
        return &ffi_type_uint64;
    case CTR_CINTERP_T_F32:
        return &ffi_type_float;
    case CTR_CINTERP_T_F64:
        return &ffi_type_double;
    case CTR_CINTERP_T_PTR:
        return &ffi_type_pointer;
    }
    return &ffi_type_void;
}

size_t ctr_cinterp_type_size(ctr_cinterp_type type)
{
    switch (type) {
    case CTR_CINTERP_T_VOID:
        return 0;
    case CTR_CINTERP_T_I8:
    case CTR_CINTERP_T_U8:
        return 1;
    case CTR_CINTERP_T_I16:
    case CTR_CINTERP_T_U16:
        return 2;
    case CTR_CINTERP_T_I32:
    case CTR_CINTERP_T_U32:
    case CTR_CINTERP_T_F32:
        return 4;
    case CTR_CINTERP_T_I64:
    case CTR_CINTERP_T_U64:
    case CTR_CINTERP_T_F64:
        return 8;
    case CTR_CINTERP_T_PTR:
        return sizeof(void*);
    }
    return 0;
}

int ctr_cinterp_parse_type(char const* name, ctr_cinterp_type* out)
{
    struct type_name {
        char const* name;
        ctr_cinterp_type type;
    };
    static struct type_name names[] = {
        { "void", CTR_CINTERP_T_VOID }, { "i8", CTR_CINTERP_T_I8 },
        { "u8", CTR_CINTERP_T_U8 }, { "i16", CTR_CINTERP_T_I16 },
        { "u16", CTR_CINTERP_T_U16 }, { "i32", CTR_CINTERP_T_I32 },
        { "u32", CTR_CINTERP_T_U32 }, { "i64", CTR_CINTERP_T_I64 },
        { "u64", CTR_CINTERP_T_U64 }, { "f32", CTR_CINTERP_T_F32 },
        { "f64", CTR_CINTERP_T_F64 }, { "ptr", CTR_CINTERP_T_PTR },
        { "int", CTR_CINTERP_T_I64 }, { "uint", CTR_CINTERP_T_U64 },
        { "double", CTR_CINTERP_T_F64 }, { "float", CTR_CINTERP_T_F32 },
        { "pointer", CTR_CINTERP_T_PTR }, { NULL, CTR_CINTERP_T_VOID }
    };
    for (size_t i = 0; names[i].name; i++) {
        if (strcasecmp(name, names[i].name) == 0) {
            *out = names[i].type;
            return 1;
        }
    }
    return 0;
}

ctr_cinterp* ctr_cinterp_new(void)
{
    return calloc(1, sizeof(ctr_cinterp));
}

void ctr_cinterp_set_error_handler(ctr_cinterp* interp, ctr_cinterp_error_fn fn, void* userdata)
{
    if (!interp)
        return;
    interp->error_fn = fn;
    interp->error_userdata = userdata;
}

static void ci_free_function(ctr_cinterp_function* fn)
{
    if (!fn)
        return;
    free(fn->name);
    for (size_t i = 0; i < fn->code_count; i++)
        free(fn->code[i].name);
    free(fn->code);
}

static void ci_free_external(ctr_cinterp_external* ext)
{
    free(ext->name);
    free(ext->arg_types);
    free(ext->ffi_arg_types);
}

void ctr_cinterp_free(ctr_cinterp* interp)
{
    if (!interp)
        return;
    for (size_t i = 0; i < interp->function_count; i++)
        ci_free_function(&interp->functions[i]);
    for (size_t i = 0; i < interp->external_count; i++)
        ci_free_external(&interp->externals[i]);
    for (size_t i = 0; i < interp->global_count; i++) {
        free(interp->globals[i].name);
        free(interp->globals[i].data);
    }
    for (size_t i = 0; i < interp->macro_count; i++) {
        free(interp->macros[i].name);
        free(interp->macros[i].value);
        free(interp->macros[i].param);
    }
    for (size_t i = 0; i < interp->ctype_count; i++) {
        free(interp->ctypes[i].name);
        free(interp->ctypes[i].format);
        for (size_t j = 0; j < interp->ctypes[i].field_count; j++)
            free(interp->ctypes[i].field_names[j]);
        free(interp->ctypes[i].field_names);
    }
    for (size_t i = 0; i < interp->library_path_count; i++)
        free(interp->library_paths[i]);
    for (size_t i = 0; i < interp->library_handle_count; i++)
        dlclose(interp->library_handles[i]);
    free(interp->functions);
    free(interp->externals);
    free(interp->globals);
    free(interp->macros);
    free(interp->ctypes);
    free(interp->library_paths);
    free(interp->library_handles);
    free(interp);
}

int ctr_cinterp_add_library_path(ctr_cinterp* interp, char const* path)
{
    if (!interp || !path)
        return 0;
    if (!ci_grow((void**)&interp->library_paths, &interp->library_path_cap, interp->library_path_count, sizeof(char*)))
        return 0;
    interp->library_paths[interp->library_path_count++] = ci_strdup(path);
    return 1;
}

size_t ctr_cinterp_library_path_count(ctr_cinterp const* interp)
{
    return interp ? interp->library_path_count : 0;
}

char const* ctr_cinterp_library_path_at(ctr_cinterp const* interp, size_t index)
{
    if (!interp || index >= interp->library_path_count)
        return NULL;
    return interp->library_paths[index];
}

static ctr_cinterp_external* ci_find_external(ctr_cinterp* interp, char const* name)
{
    for (size_t i = 0; i < interp->external_count; i++) {
        if (strcmp(interp->externals[i].name, name) == 0)
            return &interp->externals[i];
    }
    return NULL;
}

static ctr_cinterp_external* ci_add_external(ctr_cinterp* interp, char const* name, ctr_cinterp_type ret_type, ctr_cinterp_type* arg_types, size_t argc)
{
    ctr_cinterp_external* ext = ci_find_external(interp, name);
    if (!ext) {
        if (!ci_grow((void**)&interp->externals, &interp->external_cap, interp->external_count, sizeof(ctr_cinterp_external)))
            return NULL;
        ext = &interp->externals[interp->external_count++];
        memset(ext, 0, sizeof(*ext));
        ext->name = ci_strdup(name);
    } else {
        free(ext->arg_types);
        free(ext->ffi_arg_types);
        ext->ffi_arg_types = NULL;
        ext->cif_ready = 0;
    }
    ext->ret_type = ret_type;
    ext->argc = argc;
    ext->arg_types = calloc(argc ? argc : 1, sizeof(ctr_cinterp_type));
    if (!ext->arg_types)
        return NULL;
    if (argc)
        memcpy(ext->arg_types, arg_types, argc * sizeof(ctr_cinterp_type));
    ext->ptr = dlsym(RTLD_DEFAULT, name);
    return ext;
}

static ctr_cinterp_global* ci_find_global(ctr_cinterp* interp, char const* name)
{
    for (size_t i = 0; i < interp->global_count; i++) {
        if (strcmp(interp->globals[i].name, name) == 0)
            return &interp->globals[i];
    }
    return NULL;
}

static ctr_cinterp_ctype* ci_find_ctype(ctr_cinterp* interp, char const* name)
{
    for (size_t i = interp->ctype_count; i > 0; i--) {
        if (strcmp(interp->ctypes[i - 1].name, name) == 0)
            return &interp->ctypes[i - 1];
    }
    return NULL;
}

static int ci_type_format(ctr_cinterp_type type, char* out, size_t out_size)
{
    char const* fmt = "si";
    switch (type) {
    case CTR_CINTERP_T_VOID:
        fmt = "v";
        break;
    case CTR_CINTERP_T_I8:
        fmt = "3si";
        break;
    case CTR_CINTERP_T_U8:
        fmt = "3ui";
        break;
    case CTR_CINTERP_T_I16:
        fmt = "4si";
        break;
    case CTR_CINTERP_T_U16:
        fmt = "4ui";
        break;
    case CTR_CINTERP_T_I32:
        fmt = "si";
        break;
    case CTR_CINTERP_T_U32:
        fmt = "ui";
        break;
    case CTR_CINTERP_T_I64:
        fmt = "sl";
        break;
    case CTR_CINTERP_T_U64:
        fmt = "ul";
        break;
    case CTR_CINTERP_T_F32:
        fmt = "f";
        break;
    case CTR_CINTERP_T_F64:
        fmt = "d";
        break;
    case CTR_CINTERP_T_PTR:
        fmt = "p";
        break;
    }
    return snprintf(out, out_size, "%s", fmt) < (int)out_size;
}

static int ci_add_ctype_ex(ctr_cinterp* interp, char const* name, char const* format, char** field_names, size_t field_count, size_t array_count)
{
    ctr_cinterp_ctype* ctype = ci_find_ctype(interp, name);
    if (!ctype) {
        if (!ci_grow((void**)&interp->ctypes, &interp->ctype_cap, interp->ctype_count, sizeof(ctr_cinterp_ctype)))
            return 0;
        ctype = &interp->ctypes[interp->ctype_count++];
        memset(ctype, 0, sizeof(*ctype));
        ctype->name = ci_strdup(name);
    } else {
        free(ctype->format);
        for (size_t i = 0; i < ctype->field_count; i++)
            free(ctype->field_names[i]);
        free(ctype->field_names);
    }
    ctype->format = ci_strdup(format);
    ctype->field_count = field_count;
    ctype->array_count = array_count;
    ctype->field_names = calloc(field_count ? field_count : 1, sizeof(char*));
    if (!ctype->field_names)
        return 0;
    for (size_t i = 0; i < field_count; i++)
        ctype->field_names[i] = field_names[i] ? ci_strdup(field_names[i]) : ci_strdup("");
    return 1;
}

static int ci_add_ctype(ctr_cinterp* interp, char const* name, char const* format, char** field_names, size_t field_count)
{
    return ci_add_ctype_ex(interp, name, format, field_names, field_count, 0);
}

static int ci_format_to_ctype(char const* format, ctr_ctype* out, size_t* size)
{
    if (strcmp(format, "v") == 0) {
        *out = CTR_CTYPE_VOID;
        *size = 0;
    } else if (strcmp(format, "3ui") == 0) {
        *out = CTR_CTYPE_UINT8;
        *size = 1;
    } else if (strcmp(format, "3si") == 0) {
        *out = CTR_CTYPE_SINT8;
        *size = 1;
    } else if (strcmp(format, "4ui") == 0) {
        *out = CTR_CTYPE_UINT16;
        *size = 2;
    } else if (strcmp(format, "4si") == 0) {
        *out = CTR_CTYPE_SINT16;
        *size = 2;
    } else if (strcmp(format, "ui") == 0) {
        *out = CTR_CTYPE_UINT;
        *size = sizeof(unsigned int);
    } else if (strcmp(format, "si") == 0) {
        *out = CTR_CTYPE_SINT;
        *size = sizeof(int);
    } else if (strcmp(format, "ul") == 0) {
        *out = CTR_CTYPE_ULONG;
        *size = sizeof(unsigned long);
    } else if (strcmp(format, "sl") == 0) {
        *out = CTR_CTYPE_SLONG;
        *size = sizeof(long);
    } else if (strcmp(format, "f") == 0) {
        *out = CTR_CTYPE_FLOAT;
        *size = sizeof(float);
    } else if (strcmp(format, "d") == 0) {
        *out = CTR_CTYPE_DOUBLE;
        *size = sizeof(double);
    } else if (strcmp(format, "p") == 0) {
        *out = CTR_CTYPE_POINTER;
        *size = sizeof(void*);
    } else {
        return 0;
    }
    return 1;
}

static ctr_cinterp_macro* ci_find_macro_entry(ctr_cinterp* interp, char const* name)
{
    for (size_t i = interp->macro_count; i > 0; i--) {
        if (strcmp(interp->macros[i - 1].name, name) == 0)
            return &interp->macros[i - 1];
    }
    return NULL;
}

static char const* ci_find_macro(ctr_cinterp* interp, char const* name)
{
    ctr_cinterp_macro* macro = ci_find_macro_entry(interp, name);
    if (macro && !macro->function_like)
        return macro->value;
    if (strcmp(name, "__FLT_MIN__") == 0)
        return CI_STR(__FLT_MIN__);
    return NULL;
}

static int ci_set_macro_ex(ctr_cinterp* interp, char const* name, char const* value, char const* param, int function_like)
{
    for (size_t i = 0; i < interp->macro_count; i++) {
        if (strcmp(interp->macros[i].name, name) == 0) {
            free(interp->macros[i].value);
            free(interp->macros[i].param);
            interp->macros[i].value = ci_strdup(value);
            interp->macros[i].param = param ? ci_strdup(param) : NULL;
            interp->macros[i].function_like = function_like;
            return 1;
        }
    }
    if (!ci_grow((void**)&interp->macros, &interp->macro_cap, interp->macro_count, sizeof(ctr_cinterp_macro)))
        return 0;
    interp->macros[interp->macro_count].name = ci_strdup(name);
    interp->macros[interp->macro_count].value = ci_strdup(value);
    interp->macros[interp->macro_count].param = param ? ci_strdup(param) : NULL;
    interp->macros[interp->macro_count].function_like = function_like;
    interp->macro_count++;
    return 1;
}

static int ci_set_macro(ctr_cinterp* interp, char const* name, char const* value)
{
    return ci_set_macro_ex(interp, name, value, NULL, 0);
}

ctr_cinterp_function* ctr_cinterp_find_function(ctr_cinterp* interp, char const* name)
{
    if (!interp || !name)
        return NULL;
    for (size_t i = 0; i < interp->function_count; i++) {
        if (strcmp(interp->functions[i].name, name) == 0)
            return &interp->functions[i];
    }
    return NULL;
}

int ctr_cinterp_link_symbol(ctr_cinterp* interp, char const* name, void* ptr)
{
    if (!interp || !name || !ptr)
        return 0;
    ctr_cinterp_external* ext = ci_find_external(interp, name);
    if (!ext) {
        if (!ci_grow((void**)&interp->externals, &interp->external_cap, interp->external_count, sizeof(ctr_cinterp_external)))
            return 0;
        ext = &interp->externals[interp->external_count++];
        memset(ext, 0, sizeof(*ext));
        ext->name = ci_strdup(name);
        ext->ret_type = CTR_CINTERP_T_I64;
    }
    ext->ptr = ptr;
    return 1;
}

void* ctr_cinterp_get_symbol(ctr_cinterp* interp, char const* name)
{
    if (!interp || !name)
        return NULL;
    ctr_cinterp_external* ext = ci_find_external(interp, name);
    if (ext && ext->ptr)
        return ext->ptr;
    ctr_cinterp_function* fn = ctr_cinterp_find_function(interp, name);
    if (fn)
        return fn;
    ctr_cinterp_global* global = ci_find_global(interp, name);
    if (global)
        return global->data;
    void* ptr = dlsym(RTLD_DEFAULT, name);
    if (ptr)
        ctr_cinterp_link_symbol(interp, name, ptr);
    return ptr;
}

int ctr_cinterp_link_library(ctr_cinterp* interp, char const* name)
{
    if (!interp || !name)
        return 0;
    char soname[1024];
    char versioned_soname[1024];
    int has_slash = strchr(name, '/') != NULL;
    int has_lib_prefix = strncmp(name, "lib", 3) == 0;
    int has_so_suffix = strstr(name, ".so") != NULL;
    if (!has_slash && !has_lib_prefix && !has_so_suffix)
        snprintf(soname, sizeof(soname), "lib%s.so", name);
    else
        snprintf(soname, sizeof(soname), "%s", name);
    if (!has_slash && !has_lib_prefix && !has_so_suffix)
        snprintf(versioned_soname, sizeof(versioned_soname), "lib%s.so.6", name);
    else
        snprintf(versioned_soname, sizeof(versioned_soname), "%s.6", name);

    void* handle = dlopen(name, RTLD_NOW | RTLD_GLOBAL);
    char const* last_error = dlerror();
    if (!handle && strcmp(soname, name) != 0) {
        handle = dlopen(soname, RTLD_NOW | RTLD_GLOBAL);
        last_error = dlerror();
    }
    if (!handle && strcmp(versioned_soname, soname) != 0 && strcmp(versioned_soname, name) != 0) {
        handle = dlopen(versioned_soname, RTLD_NOW | RTLD_GLOBAL);
        last_error = dlerror();
    }
    for (size_t i = 0; i < interp->library_path_count && !handle; i++) {
        char full[4096];
        snprintf(full, sizeof(full), "%s/%s", interp->library_paths[i], name);
        handle = dlopen(full, RTLD_NOW | RTLD_GLOBAL);
        last_error = dlerror();
        if (!handle && strcmp(soname, name) != 0) {
            snprintf(full, sizeof(full), "%s/%s", interp->library_paths[i], soname);
            handle = dlopen(full, RTLD_NOW | RTLD_GLOBAL);
            last_error = dlerror();
        }
        if (!handle && strcmp(versioned_soname, soname) != 0 && strcmp(versioned_soname, name) != 0) {
            snprintf(full, sizeof(full), "%s/%s", interp->library_paths[i], versioned_soname);
            handle = dlopen(full, RTLD_NOW | RTLD_GLOBAL);
            last_error = dlerror();
        }
    }
    if (!handle) {
        ci_error(interp, "Could not link library `%s': %s", name, last_error ? last_error : "unknown dlopen error");
        return 0;
    }
    if (!ci_grow((void**)&interp->library_handles, &interp->library_handle_cap, interp->library_handle_count, sizeof(void*))) {
        ci_error(interp, "Could not link library `%s': Not enough space", name);
        dlclose(handle);
        return 0;
    }
    interp->library_handles[interp->library_handle_count++] = handle;
    return 1;
}

static int ci_prepare_cif(ctr_cinterp_external* ext)
{
    if (ext->cif_ready)
        return 1;
    ext->ffi_arg_types = calloc(ext->argc ? ext->argc : 1, sizeof(ffi_type*));
    if (!ext->ffi_arg_types)
        return 0;
    for (size_t i = 0; i < ext->argc; i++)
        ext->ffi_arg_types[i] = ctr_cinterp_ffi_type(ext->arg_types[i]);
    ffi_status status = ffi_prep_cif(&ext->cif, FFI_DEFAULT_ABI, ext->argc, ctr_cinterp_ffi_type(ext->ret_type), ext->ffi_arg_types);
    ext->cif_ready = status == FFI_OK;
    return ext->cif_ready;
}

static char* ci_read_file(char const* path)
{
    FILE* fp = fopen(path, "rb");
    if (!fp)
        return NULL;
    fseek(fp, 0, SEEK_END);
    long size = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    if (size < 0) {
        fclose(fp);
        return NULL;
    }
    char* buf = malloc((size_t)size + 1);
    if (!buf) {
        fclose(fp);
        return NULL;
    }
    fread(buf, 1, (size_t)size, fp);
    buf[size] = 0;
    fclose(fp);
    return buf;
}

static char* ci_trim(char* s);
static void ci_collect_typedefs(ctr_cinterp* interp, char const* source);

static int ci_is_identifier_token(char const* value)
{
    if (!value || !(isalpha((unsigned char)value[0]) || value[0] == '_'))
        return 0;
    for (size_t i = 1; value[i]; i++) {
        if (!(isalnum((unsigned char)value[i]) || value[i] == '_'))
            return 0;
    }
    return 1;
}

static int ci_substitute_macro_param(char const* body, char const* param, char const* arg, char* out, size_t out_size)
{
    size_t pos = 0;
    for (size_t i = 0; body[i];) {
        if (isalpha((unsigned char)body[i]) || body[i] == '_') {
            size_t start = i++;
            while (isalnum((unsigned char)body[i]) || body[i] == '_')
                i++;
            size_t len = i - start;
            char const* text = body + start;
            size_t text_len = len;
            if (strlen(param) == len && strncmp(text, param, len) == 0) {
                text = arg;
                text_len = strlen(arg);
            }
            if (pos + text_len >= out_size)
                return 0;
            memcpy(out + pos, text, text_len);
            pos += text_len;
            continue;
        }
        if (pos + 1 >= out_size)
            return 0;
        out[pos++] = body[i++];
    }
    out[pos] = 0;
    return 1;
}

static int ci_expand_macro_text(ctr_cinterp* interp, char const* text, char* out, size_t out_size, int depth)
{
    if (depth > 32)
        return 0;
    char work[4096];
    snprintf(work, sizeof(work), "%s", text ? text : "");
    char* trimmed = ci_trim(work);
    if (ci_is_identifier_token(trimmed)) {
        char const* value = ci_find_macro(interp, trimmed);
        if (value)
            return ci_expand_macro_text(interp, value, out, out_size, depth + 1);
        snprintf(out, out_size, "%s", trimmed);
        return 1;
    }

    char* open = strchr(trimmed, '(');
    if (open) {
        char macro_name[256];
        size_t name_len = (size_t)(open - trimmed);
        while (name_len > 0 && isspace((unsigned char)trimmed[name_len - 1]))
            name_len--;
        if (name_len > 0 && name_len < sizeof(macro_name)) {
            memcpy(macro_name, trimmed, name_len);
            macro_name[name_len] = 0;
            if (ci_is_identifier_token(macro_name)) {
                ctr_cinterp_macro* macro = ci_find_macro_entry(interp, macro_name);
                if (macro && macro->function_like && macro->param) {
                    char* close = strrchr(open + 1, ')');
                    if (close) {
                        *close = 0;
                        char* arg = ci_trim(open + 1);
                        char substituted[4096];
                        if (ci_substitute_macro_param(macro->value, macro->param, arg, substituted, sizeof(substituted)))
                            return ci_expand_macro_text(interp, substituted, out, out_size, depth + 1);
                    }
                }
            }
        }
    }

    snprintf(out, out_size, "%s", trimmed);
    return 1;
}

static char const* ci_expand_macro(ctr_cinterp* interp, char const* name, char* out, size_t out_size)
{
    char const* value = ci_find_macro(interp, name);
    if (!value)
        return NULL;
    if (!ci_expand_macro_text(interp, value, out, out_size, 0))
        return value;
    return out;
}

static void ci_preprocess_directives(ctr_cinterp* interp, char const* source, int depth)
{
    if (depth > 16)
        return;
    char* copy = ci_strdup(source);
    char* saveptr = NULL;
    for (char* line = strtok_r(copy, "\n", &saveptr); line; line = strtok_r(NULL, "\n", &saveptr)) {
        char* s = ci_trim(line);
        if (strncmp(s, "#define", 7) == 0 && isspace((unsigned char)s[7])) {
            s = ci_trim(s + 7);
            char* name = s;
            while (*s && !isspace((unsigned char)*s) && *s != '(')
                s++;
            char saved = *s;
            *s = 0;
            if (*name && saved == '(') {
                char* param = s + 1;
                char* end = strchr(param, ')');
                if (end) {
                    *end = 0;
                    ci_set_macro_ex(interp, name, ci_trim(end + 1), ci_trim(param), 1);
                    *end = ')';
                }
            } else {
                char* value = saved ? ci_trim(s + 1) : "";
                char const* expanded = ci_find_macro(interp, value);
                ci_set_macro(interp, name, expanded ? expanded : value);
            }
            *s = saved;
        } else if ((strncmp(s, "#include", 8) == 0 && isspace((unsigned char)s[8]))
            || (strncmp(s, "#include_next", 13) == 0 && isspace((unsigned char)s[13]))) {
            s = ci_trim(s + (s[8] == '_' ? 13 : 8));
            char end = *s == '<' ? '>' : *s == '"' ? '"' : 0;
            if (!end)
                continue;
            s++;
            char* e = strchr(s, end);
            if (!e)
                continue;
            *e = 0;
            char full[4096];
            char* contents = NULL;
            if (end == '"')
                contents = ci_read_file(s);
            for (size_t i = 0; i < interp->library_path_count && !contents; i++) {
                snprintf(full, sizeof(full), "%s/%s", interp->library_paths[i], s);
                contents = ci_read_file(full);
            }
            if (!contents && end == '>')
                contents = ci_read_file((snprintf(full, sizeof(full), "/usr/include/%s", s), full));
            if (contents) {
                ci_collect_typedefs(interp, contents);
                ci_preprocess_directives(interp, contents, depth + 1);
                free(contents);
            }
        }
    }
    free(copy);
}

static int ci_source_is_preprocessor_only(char const* source)
{
    char* copy = ci_strdup(source);
    if (!copy)
        return 0;
    int only_directives = 1;
    char* saveptr = NULL;
    for (char* line = strtok_r(copy, "\n", &saveptr); line; line = strtok_r(NULL, "\n", &saveptr)) {
        char* s = ci_trim(line);
        if (!*s || *s == '#')
            continue;
        only_directives = 0;
        break;
    }
    free(copy);
    return only_directives;
}

static int ci_collect_include_only_macros(ctr_cinterp* interp, char const* source)
{
    ci_preprocess_directives(interp, source, 0);
    return ci_source_is_preprocessor_only(source);
}

static int ci_typedef_base_format(ctr_cinterp* interp, char const* base, char* out, size_t out_size)
{
    char tmp[1024];
    snprintf(tmp, sizeof(tmp), "%s", base);
    char* s = ci_trim(tmp);
    if (strchr(s, '*'))
        return snprintf(out, out_size, "p") < (int)out_size;
    int is_unsigned = strstr(s, "unsigned") != NULL;
    if (strstr(s, "float"))
        return snprintf(out, out_size, "f") < (int)out_size;
    if (strstr(s, "double"))
        return snprintf(out, out_size, "d") < (int)out_size;
    if (strstr(s, "char"))
        return snprintf(out, out_size, "%s", is_unsigned ? "3ui" : "3si") < (int)out_size;
    if (strstr(s, "short"))
        return snprintf(out, out_size, "%s", is_unsigned ? "4ui" : "4si") < (int)out_size;
    if (strstr(s, "long"))
        return snprintf(out, out_size, "%s", is_unsigned ? "ul" : "sl") < (int)out_size;
    if (strstr(s, "int"))
        return snprintf(out, out_size, "%s", is_unsigned ? "ui" : "si") < (int)out_size;
    char* last = s + strlen(s);
    while (last > s && isspace((unsigned char)last[-1]))
        *--last = 0;
    while (last > s && (isalnum((unsigned char)last[-1]) || last[-1] == '_'))
        last--;
    if (*last) {
        ctr_cinterp_ctype* alias = ci_find_ctype(interp, last);
        if (alias)
            return snprintf(out, out_size, "%s", alias->format) < (int)out_size;
    }
    return 0;
}

static void ci_collect_typedefs(ctr_cinterp* interp, char const* source)
{
    char* copy = ci_strdup(source);
    char* saveptr = NULL;
    for (char* line = strtok_r(copy, "\n", &saveptr); line; line = strtok_r(NULL, "\n", &saveptr)) {
        char* s = ci_trim(line);
        if (strncmp(s, "LINMATH_H_DEFINE_VEC(", 21) == 0) {
            char* n = s + 21;
            int count = atoi(n);
            if (count > 0) {
                char name[32];
                snprintf(name, sizeof(name), "vec%d", count);
                ci_add_ctype_ex(interp, name, "f", NULL, 0, (size_t)count);
            }
            continue;
        }
        if (strncmp(s, "typedef", 7) != 0 || !isspace((unsigned char)s[7]) || strchr(s, '('))
            continue;
        s = ci_trim(s + 7);
        char* semi = strchr(s, ';');
        if (!semi)
            continue;
        *semi = 0;
        size_t array_count = 0;
        char* rb = strrchr(s, ']');
        if (rb) {
            char* lb = rb;
            while (lb > s && *lb != '[')
                lb--;
            if (*lb == '[') {
                *rb = 0;
                array_count = (size_t)strtoull(lb + 1, NULL, 0);
                *lb = 0;
            }
        }
        char* end = s + strlen(s);
        while (end > s && isspace((unsigned char)end[-1]))
            *--end = 0;
        char* alias = end;
        while (alias > s && (isalnum((unsigned char)alias[-1]) || alias[-1] == '_'))
            alias--;
        if (alias == end || alias == s)
            continue;
        char alias_name[256];
        size_t alias_len = (size_t)(end - alias);
        if (alias_len >= sizeof(alias_name))
            continue;
        memcpy(alias_name, alias, alias_len);
        alias_name[alias_len] = 0;
        char saved = *alias;
        *alias = 0;
        char format[16];
        if (ci_typedef_base_format(interp, s, format, sizeof(format)))
            ci_add_ctype_ex(interp, alias_name, format, NULL, 0, array_count);
        *alias = saved;
    }
    free(copy);
}

static int ci_builtin_signature(char const* name, ctr_cinterp_type* ret_type, ctr_cinterp_type* arg_types, size_t* argc)
{
    if (strcmp(name, "atof") == 0) {
        *ret_type = CTR_CINTERP_T_F64;
        arg_types[0] = CTR_CINTERP_T_PTR;
        *argc = 1;
        return 1;
    }
    if (strcmp(name, "fmaxf") == 0 || strcmp(name, "fminf") == 0) {
        *ret_type = CTR_CINTERP_T_F32;
        arg_types[0] = CTR_CINTERP_T_F32;
        arg_types[1] = CTR_CINTERP_T_F32;
        *argc = 2;
        return 1;
    }
    if (strcmp(name, "fmax") == 0 || strcmp(name, "fmin") == 0) {
        *ret_type = CTR_CINTERP_T_F64;
        arg_types[0] = CTR_CINTERP_T_F64;
        arg_types[1] = CTR_CINTERP_T_F64;
        *argc = 2;
        return 1;
    }
    return 0;
}

static ctr_object* ci_make_cif_object(ctr_cinterp_type ret_type, ctr_cinterp_type* arg_types, size_t argc)
{
    ffi_cif* cif_res = ctr_heap_allocate(sizeof(ffi_cif));
    ffi_type** atypes = ctr_heap_allocate(sizeof(ffi_type*) * (argc ? argc : 1));
    for (size_t i = 0; i < argc; i++)
        atypes[i] = ctr_cinterp_ffi_type(arg_types[i]);
    ffi_type* rtype = ctr_cinterp_ffi_type(ret_type);
    ffi_status status = ffi_prep_cif(cif_res, FFI_DEFAULT_ABI, argc, rtype, atypes);
    if (status != FFI_OK) {
        CtrStdFlow = ctr_build_string_from_cstring(status == FFI_BAD_ABI ? "FFI_BAD_ABI" : "FFI_ERROR");
        return CtrStdNil;
    }
    ctr_object* cifobj = ctr_internal_create_object(CTR_OBJECT_TYPE_OTEX);
    ctr_ctypes_set_type(cifobj, CTR_CTYPE_CIF);
    ctr_set_link_all(cifobj, CtrStdCType_ffi_cif);
    cifobj->value.rvalue->ptr = (void*)cif_res;
    ctr_internal_object_set_property(cifobj, ctr_build_string_from_cstring(":cfnptr"), ctr_build_bool(0), 0);
    ctr_internal_object_set_property(
        cifobj, ctr_build_string_from_cstring(":crType"),
        ctr_build_number_from_float(ctr_ctypes_ffi_convert_ffi_type_to_ctype(rtype)), 0);
    return cifobj;
}

static ctr_object* ci_make_ctype_object(ctr_cinterp_ctype* ctype)
{
    if (!ctype->field_count) {
        ctr_ctype ctype_id;
        size_t size;
        if (!ci_format_to_ctype(ctype->format, &ctype_id, &size))
            return CtrStdNil;
        ctr_object* obj = ctr_internal_create_object(CTR_OBJECT_TYPE_OTEX);
        ctr_ctypes_set_type(obj, ctype_id);
        if (ctype->array_count) {
            ctr_ctypes_cont_array_t* arr = ctr_heap_allocate(sizeof(*arr));
            arr->count = ctype->array_count;
            arr->esize = size;
            arr->storage = ctr_heap_allocate(size * ctype->array_count);
            arr->etype = (ffi_type*)ctr_ctypes_ffi_convert_ctype_to_ffi_type(ctype_id);
            ctr_object* array = ctr_ctypes_make_cont_pointer(NULL, NULL);
            array->value.rvalue->ptr = arr;
            return array;
        }
        return obj;
    }
    size_t fmt_len = strlen(ctype->format) + 1;
    char* fmt = ctr_heap_allocate(fmt_len);
    memcpy(fmt, ctype->format, fmt_len);
    ctr_object* struct_obj = ctr_ctypes_struct_make_internal(CtrStdCType_struct, fmt);
    ctr_send_message_variadic(ctr_find(ctr_build_string_from_cstring("import")),
        "Library/Foreign/C/NamedStruct", 29, 0);
    ctr_object* ns = ctr_send_message_variadic(
        ctr_find(ctr_build_string_from_cstring("NamedStruct")), "new", 3, 0);
    ctr_object* names = ctr_array_new(CtrStdArray, NULL);
    for (size_t i = 0; i < ctype->field_count; i++)
        ctr_array_push(names, &(ctr_argument) { ctr_build_string_from_cstring(ctype->field_names[i]), NULL });
    return ctr_send_message_variadic(ns, "initWith:names:", 15, 2, struct_obj, names);
}

static int ci_push_instr(ctr_cinterp_function* fn, ctr_cinterp_instr instr)
{
    if (!ci_grow((void**)&fn->code, &fn->code_cap, fn->code_count, sizeof(ctr_cinterp_instr)))
        return 0;
    fn->code[fn->code_count++] = instr;
    return 1;
}

static char* ci_trim(char* s)
{
    while (isspace((unsigned char)*s))
        s++;
    char* end = s + strlen(s);
    while (end > s && isspace((unsigned char)end[-1]))
        *--end = 0;
    return s;
}

static char* ci_next(char** cursor)
{
    char* s = *cursor;
    while (isspace((unsigned char)*s))
        s++;
    if (!*s) {
        *cursor = s;
        return NULL;
    }
    char* out = s;
    while (*s && !isspace((unsigned char)*s))
        s++;
    if (*s)
        *s++ = 0;
    *cursor = s;
    return out;
}

static int ci_parse_instr(ctr_cinterp* interp, ctr_cinterp_function* fn, char* line)
{
    char* cursor = line;
    char* op = ci_next(&cursor);
    if (!op)
        return 1;
    ctr_cinterp_instr ins;
    memset(&ins, 0, sizeof(ins));
    ins.type = CTR_CINTERP_T_I64;
    if (strcmp(op, "nop") == 0)
        ins.op = CI_OP_NOP;
    else if (strcmp(op, "iconst") == 0 || strcmp(op, "const.i64") == 0) {
        ins.op = CI_OP_CONST;
        ins.type = CTR_CINTERP_T_I64;
        ins.imm.i = strtoll(ci_next(&cursor), NULL, 0);
    } else if (strcmp(op, "uconst") == 0 || strcmp(op, "const.u64") == 0) {
        ins.op = CI_OP_CONST;
        ins.type = CTR_CINTERP_T_U64;
        ins.imm.u = strtoull(ci_next(&cursor), NULL, 0);
    } else if (strcmp(op, "fconst") == 0 || strcmp(op, "const.f64") == 0) {
        ins.op = CI_OP_CONST;
        ins.type = CTR_CINTERP_T_F64;
        ins.imm.f = strtod(ci_next(&cursor), NULL);
    } else if (strcmp(op, "pconst") == 0 || strcmp(op, "const.ptr") == 0) {
        ins.op = CI_OP_CONST;
        ins.type = CTR_CINTERP_T_PTR;
        ins.imm.p = (void*)(uintptr_t)strtoull(ci_next(&cursor), NULL, 0);
    } else if (strcmp(op, "arg") == 0) {
        ins.op = CI_OP_ARG;
        ins.a = atoi(ci_next(&cursor));
    } else if (strcmp(op, "local") == 0) {
        ins.op = CI_OP_LOCAL;
        ins.a = atoi(ci_next(&cursor));
    } else if (strcmp(op, "store.local") == 0) {
        ins.op = CI_OP_STORE_LOCAL;
        ins.a = atoi(ci_next(&cursor));
    } else if (strcmp(op, "global.addr") == 0) {
        ins.op = CI_OP_GLOBAL_ADDR;
        ins.name = ci_strdup(ci_next(&cursor));
    } else if (strncmp(op, "load.", 5) == 0) {
        ins.op = CI_OP_LOAD;
        if (!ctr_cinterp_parse_type(op + 5, &ins.type))
            goto bad_type;
    } else if (strncmp(op, "store.", 6) == 0) {
        ins.op = CI_OP_STORE;
        if (!ctr_cinterp_parse_type(op + 6, &ins.type))
            goto bad_type;
    } else if (strncmp(op, "add.", 4) == 0 || strcmp(op, "add") == 0) {
        ins.op = CI_OP_ADD;
        ctr_cinterp_parse_type(op[3] ? op + 4 : "i64", &ins.type);
    } else if (strncmp(op, "sub.", 4) == 0 || strcmp(op, "sub") == 0) {
        ins.op = CI_OP_SUB;
        ctr_cinterp_parse_type(op[3] ? op + 4 : "i64", &ins.type);
    } else if (strncmp(op, "mul.", 4) == 0 || strcmp(op, "mul") == 0) {
        ins.op = CI_OP_MUL;
        ctr_cinterp_parse_type(op[3] ? op + 4 : "i64", &ins.type);
    } else if (strncmp(op, "div.", 4) == 0 || strcmp(op, "div") == 0) {
        ins.op = CI_OP_DIV;
        ctr_cinterp_parse_type(op[3] ? op + 4 : "i64", &ins.type);
    } else if (strcmp(op, "mod") == 0)
        ins.op = CI_OP_MOD;
    else if (strcmp(op, "neg") == 0)
        ins.op = CI_OP_NEG;
    else if (strcmp(op, "eq") == 0)
        ins.op = CI_OP_EQ;
    else if (strcmp(op, "ne") == 0)
        ins.op = CI_OP_NE;
    else if (strcmp(op, "lt") == 0)
        ins.op = CI_OP_LT;
    else if (strcmp(op, "le") == 0)
        ins.op = CI_OP_LE;
    else if (strcmp(op, "gt") == 0)
        ins.op = CI_OP_GT;
    else if (strcmp(op, "ge") == 0)
        ins.op = CI_OP_GE;
    else if (strcmp(op, "and") == 0)
        ins.op = CI_OP_AND;
    else if (strcmp(op, "or") == 0)
        ins.op = CI_OP_OR;
    else if (strcmp(op, "xor") == 0)
        ins.op = CI_OP_XOR;
    else if (strcmp(op, "shl") == 0)
        ins.op = CI_OP_SHL;
    else if (strcmp(op, "shr") == 0)
        ins.op = CI_OP_SHR;
    else if (strcmp(op, "not") == 0)
        ins.op = CI_OP_NOT;
    else if (strcmp(op, "jmp") == 0) {
        ins.op = CI_OP_JMP;
        ins.a = atoi(ci_next(&cursor));
    } else if (strcmp(op, "jz") == 0) {
        ins.op = CI_OP_JZ;
        ins.a = atoi(ci_next(&cursor));
    } else if (strcmp(op, "jnz") == 0) {
        ins.op = CI_OP_JNZ;
        ins.a = atoi(ci_next(&cursor));
    } else if (strcmp(op, "call") == 0) {
        ins.op = CI_OP_CALL;
        ins.name = ci_strdup(ci_next(&cursor));
        ins.a = atoi(ci_next(&cursor));
    } else if (strcmp(op, "call.ext") == 0) {
        ins.op = CI_OP_CALL_EXT;
        ins.name = ci_strdup(ci_next(&cursor));
        ins.a = atoi(ci_next(&cursor));
    } else if (strcmp(op, "ret") == 0)
        ins.op = CI_OP_RET;
    else if (strcmp(op, "pop") == 0)
        ins.op = CI_OP_POP;
    else if (strcmp(op, "dup") == 0)
        ins.op = CI_OP_DUP;
    else if (strncmp(op, "cast.", 5) == 0) {
        ins.op = CI_OP_CAST;
        if (!ctr_cinterp_parse_type(op + 5, &ins.type))
            goto bad_type;
    } else if (strcmp(op, "ptr.add") == 0)
        ins.op = CI_OP_PTR_ADD;
    else if (strcmp(op, "memcpy") == 0)
        ins.op = CI_OP_MEMCPY;
    else if (strcmp(op, "memset") == 0)
        ins.op = CI_OP_MEMSET;
    else {
        ci_error(interp, "Unknown bytecode op `%s' in `%s'", op, fn->name);
        return 0;
    }
    return ci_push_instr(fn, ins);
bad_type:
    ci_error(interp, "Unknown bytecode type in op `%s'", op);
    return 0;
}

int ctr_cinterp_compile_text(ctr_cinterp* interp, char const* source)
{
    if (!interp || !source)
        return 0;
    char* copy = ci_strdup(source);
    if (!copy)
        return 0;
    ctr_cinterp_function* current = NULL;
    int ok = 1;
    for (char* line = strtok(copy, "\n"); line; line = strtok(NULL, "\n")) {
        char* hash = strchr(line, '#');
        if (hash)
            *hash = 0;
        line = ci_trim(line);
        if (!*line)
            continue;
        char* parse_line = ci_strdup(line);
        if (!parse_line) {
            ok = 0;
            break;
        }
        char* cursor = parse_line;
        char* first = ci_next(&cursor);
        if (!first) {
            free(parse_line);
            continue;
        }
        if (strcmp(first, ".function") == 0) {
            char* name = ci_next(&cursor);
            char* argc_s = ci_next(&cursor);
            char* nlocals_s = ci_next(&cursor);
            char* ret_s = ci_next(&cursor);
            if (!name || !argc_s || !nlocals_s || !ret_s) {
                ci_error(interp, "Malformed .function directive");
                ok = 0;
                free(parse_line);
                break;
            }
            if (!ci_grow((void**)&interp->functions, &interp->function_cap, interp->function_count, sizeof(ctr_cinterp_function))) {
                ok = 0;
                free(parse_line);
                break;
            }
            current = &interp->functions[interp->function_count++];
            memset(current, 0, sizeof(*current));
            current->magic = CTR_CINTERP_FUNCTION_MAGIC;
            current->owner = interp;
            current->name = ci_strdup(name);
            current->argc = strtoull(argc_s, NULL, 0);
            current->nlocals = strtoull(nlocals_s, NULL, 0);
            if (!ctr_cinterp_parse_type(ret_s, &current->ret_type)) {
                ci_error(interp, "Unknown return type `%s'", ret_s);
                ok = 0;
                free(parse_line);
                break;
            }
        } else if (strcmp(first, ".end") == 0) {
            current = NULL;
        } else if (strcmp(first, ".extern") == 0) {
            char* name = ci_next(&cursor);
            char* ret_s = ci_next(&cursor);
            char* argc_s = ci_next(&cursor);
            ctr_cinterp_type ret_type;
            if (!name || !ret_s || !argc_s) {
                ci_error(interp, "Malformed .extern directive");
                ok = 0;
                free(parse_line);
                break;
            }
            size_t argc = strtoull(argc_s, NULL, 0);
            ctr_cinterp_type* arg_types = calloc(argc ? argc : 1, sizeof(ctr_cinterp_type));
            if (!arg_types || !ctr_cinterp_parse_type(ret_s, &ret_type)) {
                ci_error(interp, "Unknown extern return type `%s'", ret_s);
                free(arg_types);
                ok = 0;
                free(parse_line);
                break;
            }
            for (size_t i = 0; i < argc; i++) {
                char* arg_s = ci_next(&cursor);
                if (!arg_s || !ctr_cinterp_parse_type(arg_s, &arg_types[i])) {
                    ci_error(interp, "Bad extern argument type for `%s'", name);
                    ok = 0;
                    break;
                }
            }
            if (!ok) {
                free(arg_types);
                free(parse_line);
                break;
            }
            if (!ci_add_external(interp, name, ret_type, arg_types, argc))
                ok = 0;
            free(arg_types);
        } else if (strcmp(first, ".global") == 0) {
            char* name = ci_next(&cursor);
            char* size_s = ci_next(&cursor);
            if (!name || !size_s) {
                ci_error(interp, "Malformed .global directive");
                ok = 0;
                free(parse_line);
                break;
            }
            if (!ci_grow((void**)&interp->globals, &interp->global_cap, interp->global_count, sizeof(ctr_cinterp_global))) {
                ok = 0;
                free(parse_line);
                break;
            }
            ctr_cinterp_global* global = &interp->globals[interp->global_count++];
            global->name = ci_strdup(name);
            global->size = strtoull(size_s, NULL, 0);
            global->data = calloc(1, global->size ? global->size : 1);
        } else if (current) {
            ok = ci_parse_instr(interp, current, line);
            if (!ok) {
                free(parse_line);
                break;
            }
        } else {
            ci_error(interp, "Instruction outside .function: `%s'", first);
            ok = 0;
            free(parse_line);
            break;
        }
        free(parse_line);
    }
    free(copy);
    return ok;
}

typedef enum ci_ctok_kind {
    CI_CTOK_EOF = 0,
    CI_CTOK_IDENT,
    CI_CTOK_NUM,
    CI_CTOK_STR,
    CI_CTOK_PUNCT,
    CI_CTOK_RETURN,
    CI_CTOK_IF,
    CI_CTOK_ELSE,
    CI_CTOK_WHILE,
    CI_CTOK_FOR,
    CI_CTOK_INT,
    CI_CTOK_LONG,
    CI_CTOK_SHORT,
    CI_CTOK_CHAR,
    CI_CTOK_VOID,
    CI_CTOK_FLOAT,
    CI_CTOK_DOUBLE,
    CI_CTOK_UNSIGNED,
    CI_CTOK_SIGNED,
    CI_CTOK_CONST,
    CI_CTOK_EXTERN,
    CI_CTOK_STATIC,
    CI_CTOK_STRUCT
} ci_ctok_kind;

typedef struct ci_ctok {
    ci_ctok_kind kind;
    char* text;
    int64_t num;
    int line;
} ci_ctok;

typedef struct ci_cvar {
    char* name;
    int index;
    int is_arg;
    ctr_cinterp_type type;
} ci_cvar;

typedef struct ci_cparser {
    ctr_cinterp* interp;
    ci_ctok* toks;
    size_t pos;
    size_t count;
    ctr_cinterp_function* fn;
    ci_cvar* vars;
    size_t var_count;
    size_t var_cap;
    int string_id;
    int ok;
} ci_cparser;

static void ci_free_tokens(ci_ctok* toks, size_t count)
{
    for (size_t i = 0; i < count; i++)
        free(toks[i].text);
    free(toks);
}

static int ci_tok_push(ci_ctok** toks, size_t* count, size_t* cap, ci_ctok tok)
{
    if (!ci_grow((void**)toks, cap, *count, sizeof(ci_ctok)))
        return 0;
    (*toks)[(*count)++] = tok;
    return 1;
}

static ci_ctok_kind ci_keyword_kind(char const* text)
{
    if (strcmp(text, "return") == 0)
        return CI_CTOK_RETURN;
    if (strcmp(text, "if") == 0)
        return CI_CTOK_IF;
    if (strcmp(text, "else") == 0)
        return CI_CTOK_ELSE;
    if (strcmp(text, "while") == 0)
        return CI_CTOK_WHILE;
    if (strcmp(text, "for") == 0)
        return CI_CTOK_FOR;
    if (strcmp(text, "int") == 0)
        return CI_CTOK_INT;
    if (strcmp(text, "long") == 0)
        return CI_CTOK_LONG;
    if (strcmp(text, "short") == 0)
        return CI_CTOK_SHORT;
    if (strcmp(text, "char") == 0)
        return CI_CTOK_CHAR;
    if (strcmp(text, "void") == 0)
        return CI_CTOK_VOID;
    if (strcmp(text, "float") == 0)
        return CI_CTOK_FLOAT;
    if (strcmp(text, "double") == 0)
        return CI_CTOK_DOUBLE;
    if (strcmp(text, "unsigned") == 0)
        return CI_CTOK_UNSIGNED;
    if (strcmp(text, "signed") == 0)
        return CI_CTOK_SIGNED;
    if (strcmp(text, "const") == 0)
        return CI_CTOK_CONST;
    if (strcmp(text, "extern") == 0)
        return CI_CTOK_EXTERN;
    if (strcmp(text, "static") == 0)
        return CI_CTOK_STATIC;
    if (strcmp(text, "struct") == 0)
        return CI_CTOK_STRUCT;
    return CI_CTOK_IDENT;
}

static int ci_lex_c(ctr_cinterp* interp, char const* source, ci_ctok** out, size_t* out_count)
{
    ci_ctok* toks = NULL;
    size_t count = 0;
    size_t cap = 0;
    int line = 1;
    char const* p = source;
    while (*p) {
        if (isspace((unsigned char)*p)) {
            if (*p == '\n')
                line++;
            p++;
            continue;
        }
        if (p[0] == '/' && p[1] == '/') {
            while (*p && *p != '\n')
                p++;
            continue;
        }
        if (p[0] == '/' && p[1] == '*') {
            p += 2;
            while (*p && !(p[0] == '*' && p[1] == '/')) {
                if (*p == '\n')
                    line++;
                p++;
            }
            if (*p)
                p += 2;
            continue;
        }
        if (*p == '#') {
            while (*p && *p != '\n')
                p++;
            continue;
        }
        ci_ctok tok;
        memset(&tok, 0, sizeof(tok));
        tok.line = line;
        if (isalpha((unsigned char)*p) || *p == '_') {
            char const* start = p++;
            while (isalnum((unsigned char)*p) || *p == '_')
                p++;
            size_t len = p - start;
            tok.text = malloc(len + 1);
            memcpy(tok.text, start, len);
            tok.text[len] = 0;
            tok.kind = ci_keyword_kind(tok.text);
            if (!ci_tok_push(&toks, &count, &cap, tok))
                goto oom;
            continue;
        }
        if (isdigit((unsigned char)*p)) {
            char* end = NULL;
            tok.kind = CI_CTOK_NUM;
            tok.num = strtoll(p, &end, 0);
            tok.text = ci_strdup("");
            p = end;
            if (!ci_tok_push(&toks, &count, &cap, tok))
                goto oom;
            continue;
        }
        if (*p == '"') {
            p++;
            char* buf = NULL;
            size_t len = 0;
            size_t bcap = 0;
            while (*p && *p != '"') {
                char c = *p++;
                if (c == '\\') {
                    c = *p++;
                    if (c == 'n')
                        c = '\n';
                    else if (c == 't')
                        c = '\t';
                    else if (c == 'r')
                        c = '\r';
                    else if (c == '0')
                        c = '\0';
                }
                if (!ci_grow((void**)&buf, &bcap, len + 1, 1))
                    goto oom;
                buf[len++] = c;
            }
            if (*p == '"')
                p++;
            if (!ci_grow((void**)&buf, &bcap, len + 1, 1))
                goto oom;
            buf[len++] = 0;
            tok.kind = CI_CTOK_STR;
            tok.text = buf;
            if (!ci_tok_push(&toks, &count, &cap, tok))
                goto oom;
            continue;
        }
        char two[3] = { p[0], p[1], 0 };
        if (strcmp(two, "==") == 0 || strcmp(two, "!=") == 0 || strcmp(two, "<=") == 0 || strcmp(two, ">=") == 0 || strcmp(two, "&&") == 0 || strcmp(two, "||") == 0 || strcmp(two, "<<") == 0 || strcmp(two, ">>") == 0) {
            tok.kind = CI_CTOK_PUNCT;
            tok.text = ci_strdup(two);
            p += 2;
        } else {
            char one[2] = { *p++, 0 };
            tok.kind = CI_CTOK_PUNCT;
            tok.text = ci_strdup(one);
        }
        if (!ci_tok_push(&toks, &count, &cap, tok))
            goto oom;
    }
    ci_ctok eof = { .kind = CI_CTOK_EOF, .text = ci_strdup(""), .line = line };
    if (!ci_tok_push(&toks, &count, &cap, eof))
        goto oom;
    *out = toks;
    *out_count = count;
    return 1;
oom:
    ci_error(interp, "Out of memory while lexing C source");
    ci_free_tokens(toks, count);
    return 0;
}

static ci_ctok* cp_peek(ci_cparser* p)
{
    return &p->toks[p->pos];
}

static int cp_is_punct(ci_cparser* p, char const* s)
{
    ci_ctok* tok = cp_peek(p);
    return tok->kind == CI_CTOK_PUNCT && strcmp(tok->text, s) == 0;
}

static int cp_match_punct(ci_cparser* p, char const* s)
{
    if (!cp_is_punct(p, s))
        return 0;
    p->pos++;
    return 1;
}

static int cp_match_kind(ci_cparser* p, ci_ctok_kind kind)
{
    if (cp_peek(p)->kind != kind)
        return 0;
    p->pos++;
    return 1;
}

static void cp_error(ci_cparser* p, char const* fmt, ...)
{
    char detail[512];
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(detail, sizeof(detail), fmt, ap);
    va_end(ap);
    ci_error(p->interp, "C frontend line %d: %s", cp_peek(p)->line, detail);
    p->ok = 0;
}

static int cp_expect_punct(ci_cparser* p, char const* s)
{
    if (cp_match_punct(p, s))
        return 1;
    cp_error(p, "expected `%s'", s);
    return 0;
}

static char* cp_expect_ident(ci_cparser* p)
{
    if (cp_peek(p)->kind != CI_CTOK_IDENT) {
        cp_error(p, "expected identifier");
        return NULL;
    }
    return ci_strdup(p->toks[p->pos++].text);
}

static int cp_is_type_start(ci_cparser* p)
{
    switch (cp_peek(p)->kind) {
    case CI_CTOK_INT:
    case CI_CTOK_LONG:
    case CI_CTOK_SHORT:
    case CI_CTOK_CHAR:
    case CI_CTOK_VOID:
    case CI_CTOK_FLOAT:
    case CI_CTOK_DOUBLE:
    case CI_CTOK_UNSIGNED:
    case CI_CTOK_SIGNED:
    case CI_CTOK_CONST:
    case CI_CTOK_EXTERN:
    case CI_CTOK_STATIC:
    case CI_CTOK_STRUCT:
        return 1;
    default:
        return 0;
    }
}

static int cp_parse_type(ci_cparser* p, ctr_cinterp_type* out, int* storage_extern)
{
    int is_unsigned = 0;
    int saw = 0;
    int pointer = 0;
    ctr_cinterp_type type = CTR_CINTERP_T_I64;
    if (storage_extern)
        *storage_extern = 0;
    while (cp_match_kind(p, CI_CTOK_CONST) || cp_match_kind(p, CI_CTOK_STATIC))
        ;
    if (cp_match_kind(p, CI_CTOK_EXTERN) && storage_extern)
        *storage_extern = 1;
    while (cp_match_kind(p, CI_CTOK_CONST))
        ;
    if (cp_match_kind(p, CI_CTOK_STRUCT)) {
        if (cp_peek(p)->kind == CI_CTOK_IDENT)
            p->pos++;
        type = CTR_CINTERP_T_PTR;
        saw = 1;
    } else
    if (cp_match_kind(p, CI_CTOK_UNSIGNED))
        is_unsigned = 1;
    else
        cp_match_kind(p, CI_CTOK_SIGNED);
    if (cp_match_kind(p, CI_CTOK_VOID)) {
        type = CTR_CINTERP_T_VOID;
        saw = 1;
    } else if (cp_match_kind(p, CI_CTOK_CHAR)) {
        type = is_unsigned ? CTR_CINTERP_T_U8 : CTR_CINTERP_T_I8;
        saw = 1;
    } else if (cp_match_kind(p, CI_CTOK_SHORT)) {
        type = is_unsigned ? CTR_CINTERP_T_U32 : CTR_CINTERP_T_I32;
        saw = 1;
    } else if (cp_match_kind(p, CI_CTOK_INT)) {
        type = is_unsigned ? CTR_CINTERP_T_U64 : CTR_CINTERP_T_I64;
        saw = 1;
    } else if (cp_match_kind(p, CI_CTOK_LONG)) {
        cp_match_kind(p, CI_CTOK_LONG);
        type = is_unsigned ? CTR_CINTERP_T_U64 : CTR_CINTERP_T_I64;
        saw = 1;
    } else if (cp_match_kind(p, CI_CTOK_FLOAT)) {
        type = CTR_CINTERP_T_F32;
        saw = 1;
    } else if (cp_match_kind(p, CI_CTOK_DOUBLE)) {
        type = CTR_CINTERP_T_F64;
        saw = 1;
    }
    while (cp_match_kind(p, CI_CTOK_CONST))
        ;
    while (cp_match_punct(p, "*")) {
        pointer = 1;
        while (cp_match_kind(p, CI_CTOK_CONST))
            ;
    }
    if (!saw) {
        cp_error(p, "expected C type");
        return 0;
    }
    *out = pointer ? CTR_CINTERP_T_PTR : type;
    return 1;
}

static int cp_emit(ci_cparser* p, ctr_cinterp_instr ins)
{
    if (!p->fn) {
        cp_error(p, "internal frontend error: no active function");
        return 0;
    }
    return ci_push_instr(p->fn, ins);
}

static size_t cp_emit_jump(ci_cparser* p, ctr_cinterp_op op)
{
    ctr_cinterp_instr ins;
    memset(&ins, 0, sizeof(ins));
    ins.op = op;
    ins.a = 0;
    if (!cp_emit(p, ins))
        return 0;
    return p->fn->code_count - 1;
}

static void cp_patch_jump(ci_cparser* p, size_t at, size_t target)
{
    if (at < p->fn->code_count)
        p->fn->code[at].a = (int)target;
}

static ci_cvar* cp_find_var(ci_cparser* p, char const* name)
{
    for (size_t i = p->var_count; i > 0; i--) {
        if (strcmp(p->vars[i - 1].name, name) == 0)
            return &p->vars[i - 1];
    }
    return NULL;
}

static ci_cvar* cp_add_var(ci_cparser* p, char const* name, ctr_cinterp_type type, int is_arg, int index)
{
    if (!ci_grow((void**)&p->vars, &p->var_cap, p->var_count, sizeof(ci_cvar)))
        return NULL;
    ci_cvar* var = &p->vars[p->var_count++];
    var->name = ci_strdup(name);
    var->type = type;
    var->is_arg = is_arg;
    var->index = index;
    return var;
}

static void cp_free_vars(ci_cparser* p)
{
    for (size_t i = 0; i < p->var_count; i++)
        free(p->vars[i].name);
    free(p->vars);
    p->vars = NULL;
    p->var_count = 0;
    p->var_cap = 0;
}

static void cp_emit_load_var(ci_cparser* p, ci_cvar* var)
{
    ctr_cinterp_instr ins;
    memset(&ins, 0, sizeof(ins));
    ins.op = var->is_arg ? CI_OP_ARG : CI_OP_LOCAL;
    ins.a = var->index;
    cp_emit(p, ins);
}

static void cp_emit_store_var(ci_cparser* p, ci_cvar* var)
{
    ctr_cinterp_instr ins;
    memset(&ins, 0, sizeof(ins));
    ins.op = CI_OP_STORE_LOCAL;
    ins.a = var->index;
    if (var->is_arg)
        cp_error(p, "assignment to function arguments is not supported yet");
    else
        cp_emit(p, ins);
}

static int cp_add_string_global(ci_cparser* p, char const* value)
{
    char name[64];
    snprintf(name, sizeof(name), ".str%d", p->string_id++);
    if (!ci_grow((void**)&p->interp->globals, &p->interp->global_cap, p->interp->global_count, sizeof(ctr_cinterp_global)))
        return 0;
    ctr_cinterp_global* global = &p->interp->globals[p->interp->global_count++];
    global->name = ci_strdup(name);
    global->size = strlen(value) + 1;
    global->data = malloc(global->size);
    memcpy(global->data, value, global->size);
    ctr_cinterp_instr ins;
    memset(&ins, 0, sizeof(ins));
    ins.op = CI_OP_GLOBAL_ADDR;
    ins.name = ci_strdup(name);
    return cp_emit(p, ins);
}

static void cp_parse_expr(ci_cparser* p);
static void cp_parse_stmt(ci_cparser* p);

static void cp_parse_primary(ci_cparser* p)
{
    if (cp_match_punct(p, "(")) {
        cp_parse_expr(p);
        cp_expect_punct(p, ")");
        return;
    }
    if (cp_peek(p)->kind == CI_CTOK_NUM) {
        ctr_cinterp_instr ins;
        memset(&ins, 0, sizeof(ins));
        ins.op = CI_OP_CONST;
        ins.type = CTR_CINTERP_T_I64;
        ins.imm.i = p->toks[p->pos++].num;
        cp_emit(p, ins);
        return;
    }
    if (cp_peek(p)->kind == CI_CTOK_STR) {
        cp_add_string_global(p, p->toks[p->pos++].text);
        return;
    }
    if (cp_peek(p)->kind == CI_CTOK_IDENT) {
        char* name = ci_strdup(cp_peek(p)->text);
        p->pos++;
        if (cp_match_punct(p, "(")) {
            size_t argc = 0;
            if (!cp_is_punct(p, ")")) {
                do {
                    cp_parse_expr(p);
                    argc++;
                } while (cp_match_punct(p, ","));
            }
            cp_expect_punct(p, ")");
            ctr_cinterp_instr ins;
            memset(&ins, 0, sizeof(ins));
            ins.op = ctr_cinterp_find_function(p->interp, name) ? CI_OP_CALL : CI_OP_CALL_EXT;
            ins.name = name;
            ins.a = (int)argc;
            cp_emit(p, ins);
            return;
        }
        ci_cvar* var = cp_find_var(p, name);
        if (var)
            cp_emit_load_var(p, var);
        else {
            ctr_cinterp_instr ins;
            memset(&ins, 0, sizeof(ins));
            ins.op = CI_OP_GLOBAL_ADDR;
            ins.name = name;
            cp_emit(p, ins);
            return;
        }
        free(name);
        return;
    }
    cp_error(p, "expected expression");
}

static void cp_parse_unary(ci_cparser* p)
{
    if (cp_is_punct(p, "(") && cp_is_type_start(&(ci_cparser) { .toks = p->toks, .pos = p->pos + 1, .count = p->count, .interp = p->interp, .ok = p->ok })) {
        size_t save = p->pos;
        ctr_cinterp_type cast_type;
        int ignored_storage = 0;
        cp_match_punct(p, "(");
        if (cp_parse_type(p, &cast_type, &ignored_storage) && cp_match_punct(p, ")")) {
            cp_parse_unary(p);
            ctr_cinterp_instr ins;
            memset(&ins, 0, sizeof(ins));
            ins.op = CI_OP_CAST;
            ins.type = cast_type;
            cp_emit(p, ins);
            return;
        }
        p->pos = save;
    }
    if (cp_match_punct(p, "-")) {
        cp_parse_unary(p);
        ctr_cinterp_instr ins = { .op = CI_OP_NEG };
        cp_emit(p, ins);
    } else if (cp_match_punct(p, "!")) {
        cp_parse_unary(p);
        ctr_cinterp_instr zero;
        memset(&zero, 0, sizeof(zero));
        zero.op = CI_OP_CONST;
        zero.imm.i = 0;
        cp_emit(p, zero);
        ctr_cinterp_instr eq = { .op = CI_OP_EQ };
        cp_emit(p, eq);
    } else if (cp_match_punct(p, "&")) {
        if (cp_peek(p)->kind != CI_CTOK_IDENT) {
            cp_error(p, "address-of currently expects a local/global name");
            return;
        }
        char* name = ci_strdup(cp_peek(p)->text);
        p->pos++;
        ci_cvar* var = cp_find_var(p, name);
        if (var)
            cp_error(p, "address-of locals is not supported by this bytecode layout yet");
        else {
            ctr_cinterp_instr ins;
            memset(&ins, 0, sizeof(ins));
            ins.op = CI_OP_GLOBAL_ADDR;
            ins.name = name;
            cp_emit(p, ins);
            return;
        }
        free(name);
    } else if (cp_match_punct(p, "*")) {
        cp_parse_unary(p);
        ctr_cinterp_instr ins = { .op = CI_OP_LOAD, .type = CTR_CINTERP_T_I64 };
        cp_emit(p, ins);
    } else {
        cp_parse_primary(p);
    }
}

static void cp_parse_mul(ci_cparser* p)
{
    cp_parse_unary(p);
    while (cp_is_punct(p, "*") || cp_is_punct(p, "/") || cp_is_punct(p, "%")) {
        char op = cp_peek(p)->text[0];
        p->pos++;
        cp_parse_unary(p);
        ctr_cinterp_instr ins;
        memset(&ins, 0, sizeof(ins));
        ins.op = op == '*' ? CI_OP_MUL : op == '/' ? CI_OP_DIV
                                                   : CI_OP_MOD;
        cp_emit(p, ins);
    }
}

static void cp_parse_add(ci_cparser* p)
{
    cp_parse_mul(p);
    while (cp_is_punct(p, "+") || cp_is_punct(p, "-")) {
        char op = cp_peek(p)->text[0];
        p->pos++;
        cp_parse_mul(p);
        ctr_cinterp_instr ins;
        memset(&ins, 0, sizeof(ins));
        ins.op = op == '+' ? CI_OP_ADD : CI_OP_SUB;
        cp_emit(p, ins);
    }
}

static void cp_parse_shift(ci_cparser* p)
{
    cp_parse_add(p);
    while (cp_is_punct(p, "<<") || cp_is_punct(p, ">>")) {
        int left = cp_is_punct(p, "<<");
        p->pos++;
        cp_parse_add(p);
        ctr_cinterp_instr ins = { .op = left ? CI_OP_SHL : CI_OP_SHR };
        cp_emit(p, ins);
    }
}

static void cp_parse_rel(ci_cparser* p)
{
    cp_parse_shift(p);
    while (cp_is_punct(p, "<") || cp_is_punct(p, "<=") || cp_is_punct(p, ">") || cp_is_punct(p, ">=")) {
        char const* op = cp_peek(p)->text;
        p->pos++;
        cp_parse_shift(p);
        ctr_cinterp_instr ins;
        memset(&ins, 0, sizeof(ins));
        ins.op = strcmp(op, "<") == 0 ? CI_OP_LT : strcmp(op, "<=") == 0 ? CI_OP_LE
            : strcmp(op, ">") == 0                                       ? CI_OP_GT
                                                                         : CI_OP_GE;
        cp_emit(p, ins);
    }
}

static void cp_parse_eq(ci_cparser* p)
{
    cp_parse_rel(p);
    while (cp_is_punct(p, "==") || cp_is_punct(p, "!=")) {
        int eq = cp_is_punct(p, "==");
        p->pos++;
        cp_parse_rel(p);
        ctr_cinterp_instr ins = { .op = eq ? CI_OP_EQ : CI_OP_NE };
        cp_emit(p, ins);
    }
}

static void cp_parse_bitand(ci_cparser* p)
{
    cp_parse_eq(p);
    while (cp_match_punct(p, "&")) {
        cp_parse_eq(p);
        ctr_cinterp_instr ins = { .op = CI_OP_AND };
        cp_emit(p, ins);
    }
}

static void cp_parse_bitxor(ci_cparser* p)
{
    cp_parse_bitand(p);
    while (cp_match_punct(p, "^")) {
        cp_parse_bitand(p);
        ctr_cinterp_instr ins = { .op = CI_OP_XOR };
        cp_emit(p, ins);
    }
}

static void cp_parse_bitor(ci_cparser* p)
{
    cp_parse_bitxor(p);
    while (cp_match_punct(p, "|")) {
        cp_parse_bitxor(p);
        ctr_cinterp_instr ins = { .op = CI_OP_OR };
        cp_emit(p, ins);
    }
}

static void cp_parse_logand(ci_cparser* p)
{
    cp_parse_bitor(p);
    while (cp_match_punct(p, "&&")) {
        cp_parse_bitor(p);
        ctr_cinterp_instr ins = { .op = CI_OP_AND };
        cp_emit(p, ins);
    }
}

static void cp_parse_logor(ci_cparser* p)
{
    cp_parse_logand(p);
    while (cp_match_punct(p, "||")) {
        cp_parse_logand(p);
        ctr_cinterp_instr ins = { .op = CI_OP_OR };
        cp_emit(p, ins);
    }
}

static void cp_parse_expr(ci_cparser* p)
{
    if (cp_peek(p)->kind == CI_CTOK_IDENT && p->toks[p->pos + 1].kind == CI_CTOK_PUNCT && strcmp(p->toks[p->pos + 1].text, "=") == 0) {
        char* name = ci_strdup(cp_peek(p)->text);
        p->pos += 2;
        cp_parse_expr(p);
        ci_cvar* var = cp_find_var(p, name);
        if (!var)
            cp_error(p, "unknown assignment target `%s'", name);
        else {
            ctr_cinterp_instr dup = { .op = CI_OP_DUP };
            cp_emit(p, dup);
            cp_emit_store_var(p, var);
        }
        free(name);
        return;
    }
    cp_parse_logor(p);
}

static void cp_parse_decl_stmt(ci_cparser* p)
{
    ctr_cinterp_type type;
    int storage_extern = 0;
    if (!cp_parse_type(p, &type, &storage_extern))
        return;
    char* name = cp_expect_ident(p);
    if (!name)
        return;
    int local_index = (int)p->fn->nlocals++;
    ci_cvar* var = cp_add_var(p, name, type, 0, local_index);
    if (cp_match_punct(p, "=")) {
        cp_parse_expr(p);
        if (var)
            cp_emit_store_var(p, var);
    }
    while (cp_match_punct(p, ",")) {
        char* extra = cp_expect_ident(p);
        if (!extra)
            break;
        local_index = (int)p->fn->nlocals++;
        var = cp_add_var(p, extra, type, 0, local_index);
        if (cp_match_punct(p, "=")) {
            cp_parse_expr(p);
            if (var)
                cp_emit_store_var(p, var);
        }
        free(extra);
    }
    free(name);
    cp_expect_punct(p, ";");
}

static void cp_parse_block(ci_cparser* p)
{
    cp_expect_punct(p, "{");
    while (p->ok && !cp_is_punct(p, "}") && cp_peek(p)->kind != CI_CTOK_EOF)
        cp_parse_stmt(p);
    cp_expect_punct(p, "}");
}

static void cp_parse_stmt(ci_cparser* p)
{
    if (!p->ok)
        return;
    if (cp_is_punct(p, "{")) {
        cp_parse_block(p);
        return;
    }
    if (cp_match_kind(p, CI_CTOK_RETURN)) {
        if (!cp_is_punct(p, ";"))
            cp_parse_expr(p);
        ctr_cinterp_instr ins = { .op = CI_OP_RET };
        cp_emit(p, ins);
        cp_expect_punct(p, ";");
        return;
    }
    if (cp_match_kind(p, CI_CTOK_IF)) {
        cp_expect_punct(p, "(");
        cp_parse_expr(p);
        cp_expect_punct(p, ")");
        size_t jz = cp_emit_jump(p, CI_OP_JZ);
        cp_parse_stmt(p);
        if (cp_match_kind(p, CI_CTOK_ELSE)) {
            size_t jmp = cp_emit_jump(p, CI_OP_JMP);
            cp_patch_jump(p, jz, p->fn->code_count);
            cp_parse_stmt(p);
            cp_patch_jump(p, jmp, p->fn->code_count);
        } else {
            cp_patch_jump(p, jz, p->fn->code_count);
        }
        return;
    }
    if (cp_match_kind(p, CI_CTOK_WHILE)) {
        size_t start = p->fn->code_count;
        cp_expect_punct(p, "(");
        cp_parse_expr(p);
        cp_expect_punct(p, ")");
        size_t jz = cp_emit_jump(p, CI_OP_JZ);
        cp_parse_stmt(p);
        ctr_cinterp_instr back = { .op = CI_OP_JMP, .a = (int)start };
        cp_emit(p, back);
        cp_patch_jump(p, jz, p->fn->code_count);
        return;
    }
    if (cp_match_kind(p, CI_CTOK_FOR)) {
        cp_expect_punct(p, "(");
        if (!cp_is_punct(p, ";"))
            cp_parse_expr(p);
        cp_expect_punct(p, ";");
        size_t start = p->fn->code_count;
        if (!cp_is_punct(p, ";"))
            cp_parse_expr(p);
        else {
            ctr_cinterp_instr one = { .op = CI_OP_CONST, .imm.i = 1 };
            cp_emit(p, one);
        }
        cp_expect_punct(p, ";");
        size_t jz = cp_emit_jump(p, CI_OP_JZ);
        size_t iter_start = p->pos;
        while (!cp_is_punct(p, ")") && cp_peek(p)->kind != CI_CTOK_EOF)
            p->pos++;
        size_t iter_end = p->pos;
        cp_expect_punct(p, ")");
        cp_parse_stmt(p);
        if (iter_end > iter_start) {
            size_t save = p->pos;
            p->pos = iter_start;
            cp_parse_expr(p);
            p->pos = save;
        }
        ctr_cinterp_instr back = { .op = CI_OP_JMP, .a = (int)start };
        cp_emit(p, back);
        cp_patch_jump(p, jz, p->fn->code_count);
        return;
    }
    if (cp_is_type_start(p)) {
        cp_parse_decl_stmt(p);
        return;
    }
    if (!cp_is_punct(p, ";")) {
        cp_parse_expr(p);
        ctr_cinterp_instr pop = { .op = CI_OP_POP };
        cp_emit(p, pop);
    }
    cp_expect_punct(p, ";");
}

static int cp_parse_struct_decl(ci_cparser* p)
{
    if (!cp_match_kind(p, CI_CTOK_STRUCT))
        return 0;
    char* name = cp_expect_ident(p);
    if (!name)
        return 0;
    if (!cp_expect_punct(p, "{")) {
        free(name);
        return 0;
    }
    char format[4096] = "";
    size_t format_len = 0;
    char** names = NULL;
    size_t name_count = 0;
    size_t name_cap = 0;
    while (p->ok && !cp_is_punct(p, "}") && cp_peek(p)->kind != CI_CTOK_EOF) {
        ctr_cinterp_type type;
        int ignored_storage = 0;
        if (!cp_parse_type(p, &type, &ignored_storage))
            break;
        char* field = cp_expect_ident(p);
        if (!field)
            break;
        while (cp_match_punct(p, "[")) {
            if (cp_peek(p)->kind == CI_CTOK_NUM)
                p->pos++;
            else
                cp_error(p, "array member length must be numeric");
            cp_expect_punct(p, "]");
            type = CTR_CINTERP_T_PTR;
        }
        char tfmt[16];
        if (!ci_type_format(type, tfmt, sizeof(tfmt))) {
            cp_error(p, "could not encode struct member type");
            free(field);
            break;
        }
        size_t tfmt_len = strlen(tfmt);
        if (format_len + tfmt_len >= sizeof(format)) {
            cp_error(p, "struct format too large");
            free(field);
            break;
        }
        memcpy(format + format_len, tfmt, tfmt_len + 1);
        format_len += tfmt_len;
        if (!ci_grow((void**)&names, &name_cap, name_count, sizeof(char*))) {
            free(field);
            p->ok = 0;
            break;
        }
        names[name_count++] = field;
        cp_expect_punct(p, ";");
    }
    cp_expect_punct(p, "}");
    cp_expect_punct(p, ";");
    if (p->ok)
        p->ok = ci_add_ctype(p->interp, name, format, names, name_count);
    for (size_t i = 0; i < name_count; i++)
        free(names[i]);
    free(names);
    free(name);
    return p->ok;
}

static int cp_parse_toplevel(ci_cparser* p)
{
    if (cp_peek(p)->kind == CI_CTOK_STRUCT && p->toks[p->pos + 2].kind == CI_CTOK_PUNCT && strcmp(p->toks[p->pos + 2].text, "{") == 0)
        return cp_parse_struct_decl(p);
    int storage_extern = 0;
    ctr_cinterp_type ret_type;
    if (!cp_parse_type(p, &ret_type, &storage_extern))
        return 0;
    char* name = cp_expect_ident(p);
    if (!name)
        return 0;
    if (!cp_match_punct(p, "(")) {
        size_t size = ctr_cinterp_type_size(ret_type);
        if (!ci_grow((void**)&p->interp->globals, &p->interp->global_cap, p->interp->global_count, sizeof(ctr_cinterp_global))) {
            free(name);
            return 0;
        }
        ctr_cinterp_global* global = &p->interp->globals[p->interp->global_count++];
        global->name = name;
        global->size = size ? size : 1;
        global->data = calloc(1, global->size);
        if (cp_match_punct(p, "=")) {
            if (cp_peek(p)->kind == CI_CTOK_NUM) {
                int64_t value = p->toks[p->pos++].num;
                memcpy(global->data, &value, global->size < sizeof(value) ? global->size : sizeof(value));
            } else if (cp_peek(p)->kind == CI_CTOK_STR && ret_type == CTR_CINTERP_T_PTR) {
                char* dup = ci_strdup(cp_peek(p)->text);
                p->pos++;
                memcpy(global->data, &dup, sizeof(dup));
            } else {
                cp_error(p, "global initializers currently support numbers and pointer strings");
            }
        }
        cp_expect_punct(p, ";");
        return p->ok;
    }
    if (!p->ok) {
        free(name);
        return 0;
    }
    ctr_cinterp_type arg_types[128];
    char* arg_names[128];
    size_t argc = 0;
    memset(arg_names, 0, sizeof(arg_names));
    if (!cp_is_punct(p, ")")) {
        do {
            if (cp_peek(p)->kind == CI_CTOK_VOID && p->toks[p->pos + 1].kind == CI_CTOK_PUNCT && strcmp(p->toks[p->pos + 1].text, ")") == 0) {
                p->pos++;
                break;
            }
            if (argc >= 128) {
                cp_error(p, "too many function arguments");
                break;
            }
            int ignored_storage = 0;
            if (!cp_parse_type(p, &arg_types[argc], &ignored_storage))
                break;
            if (cp_peek(p)->kind == CI_CTOK_IDENT)
                arg_names[argc] = cp_expect_ident(p);
            else
                arg_names[argc] = ci_strdup("");
            argc++;
        } while (cp_match_punct(p, ","));
    }
    cp_expect_punct(p, ")");
    if (cp_match_punct(p, ";")) {
        ci_add_external(p->interp, name, ret_type, arg_types, argc);
        for (size_t i = 0; i < argc; i++)
            free(arg_names[i]);
        free(name);
        return p->ok;
    }
    if (!ci_grow((void**)&p->interp->functions, &p->interp->function_cap, p->interp->function_count, sizeof(ctr_cinterp_function))) {
        free(name);
        return 0;
    }
    p->fn = &p->interp->functions[p->interp->function_count++];
    memset(p->fn, 0, sizeof(*p->fn));
    p->fn->magic = CTR_CINTERP_FUNCTION_MAGIC;
    p->fn->owner = p->interp;
    p->fn->name = name;
    p->fn->argc = argc;
    p->fn->ret_type = ret_type;
    for (size_t i = 0; i < argc; i++) {
        if (arg_names[i] && *arg_names[i])
            cp_add_var(p, arg_names[i], arg_types[i], 1, (int)i);
        free(arg_names[i]);
    }
    cp_parse_block(p);
    if (p->ok && (p->fn->code_count == 0 || p->fn->code[p->fn->code_count - 1].op != CI_OP_RET)) {
        ctr_cinterp_instr zero;
        memset(&zero, 0, sizeof(zero));
        zero.op = CI_OP_CONST;
        zero.imm.i = 0;
        cp_emit(p, zero);
        ctr_cinterp_instr ret = { .op = CI_OP_RET };
        cp_emit(p, ret);
    }
    p->fn = NULL;
    cp_free_vars(p);
    return p->ok;
}

int ctr_cinterp_compile_c(ctr_cinterp* interp, char const* source)
{
    ci_cparser p;
    memset(&p, 0, sizeof(p));
    p.interp = interp;
    p.ok = 1;
    if (!ci_lex_c(interp, source, &p.toks, &p.count))
        return 0;
    while (p.ok && cp_peek(&p)->kind != CI_CTOK_EOF)
        cp_parse_toplevel(&p);
    cp_free_vars(&p);
    ci_free_tokens(p.toks, p.count);
    return p.ok;
}

static int ci_pop(ctr_cinterp_value* stack, size_t* sp, ctr_cinterp_value* out)
{
    if (!*sp)
        return 0;
    *out = stack[--*sp];
    return 1;
}

static int ci_push(ctr_cinterp_value* stack, size_t* sp, size_t cap, ctr_cinterp_value value)
{
    if (*sp >= cap)
        return 0;
    stack[(*sp)++] = value;
    return 1;
}

static ctr_cinterp_value ci_cast_value(ctr_cinterp_value v, ctr_cinterp_type type)
{
    switch (type) {
    case CTR_CINTERP_T_F32:
    case CTR_CINTERP_T_F64:
        v.f = v.f;
        return v;
    case CTR_CINTERP_T_PTR:
        v.p = (void*)(uintptr_t)v.u;
        return v;
    default:
        v.i = (int64_t)v.u;
        return v;
    }
}

static int ci_call_function(ctr_cinterp* interp, ctr_cinterp_function* fn, ctr_cinterp_value* argv, size_t argc, ctr_cinterp_value* ret, unsigned depth);

static int ci_call_external(ctr_cinterp* interp, ctr_cinterp_external* ext, ctr_cinterp_value* args, ctr_cinterp_value* ret)
{
    if (!ext->ptr)
        ext->ptr = dlsym(RTLD_DEFAULT, ext->name);
    if (!ext->ptr) {
        ci_error(interp, "External symbol `%s' is not linked", ext->name);
        return 0;
    }
    if (!ci_prepare_cif(ext)) {
        ci_error(interp, "Could not prepare FFI call for `%s'", ext->name);
        return 0;
    }
    void** ffi_args = calloc(ext->argc ? ext->argc : 1, sizeof(void*));
    if (!ffi_args)
        return 0;
    for (size_t i = 0; i < ext->argc; i++)
        ffi_args[i] = &args[i];
    ffi_call(&ext->cif, FFI_FN(ext->ptr), ret, ffi_args);
    free(ffi_args);
    return 1;
}

static int ci_call_function(ctr_cinterp* interp, ctr_cinterp_function* fn, ctr_cinterp_value* argv, size_t argc, ctr_cinterp_value* ret, unsigned depth)
{
    if (!fn || argc != fn->argc || depth > 1024) {
        ci_error(interp, "Bad call to bytecode function");
        return 0;
    }
    enum { STACK_MAX = 4096 };
    ctr_cinterp_value stack[STACK_MAX];
    ctr_cinterp_value locals_stack[256];
    ctr_cinterp_value* locals = fn->nlocals <= 256 ? locals_stack : calloc(fn->nlocals, sizeof(ctr_cinterp_value));
    size_t sp = 0;
    size_t pc = 0;
    memset(stack, 0, sizeof(stack));
    if (fn->nlocals <= 256)
        memset(locals_stack, 0, sizeof(locals_stack));
    int ok = 1;
    while (pc < fn->code_count && ok) {
        ctr_cinterp_instr* ins = &fn->code[pc++];
        ctr_cinterp_value a, b, v;
        memset(&v, 0, sizeof(v));
        switch (ins->op) {
        case CI_OP_NOP:
            break;
        case CI_OP_CONST:
            ok = ci_push(stack, &sp, STACK_MAX, ins->imm);
            break;
        case CI_OP_ARG:
            ok = ins->a >= 0 && (size_t)ins->a < argc && ci_push(stack, &sp, STACK_MAX, argv[ins->a]);
            break;
        case CI_OP_LOCAL:
            ok = ins->a >= 0 && (size_t)ins->a < fn->nlocals && ci_push(stack, &sp, STACK_MAX, locals[ins->a]);
            break;
        case CI_OP_STORE_LOCAL:
            ok = ins->a >= 0 && (size_t)ins->a < fn->nlocals && ci_pop(stack, &sp, &locals[ins->a]);
            break;
        case CI_OP_GLOBAL_ADDR: {
            ctr_cinterp_global* global = ci_find_global(interp, ins->name);
            ok = global != NULL;
            v.p = ok ? global->data : NULL;
            ok = ok && ci_push(stack, &sp, STACK_MAX, v);
            break;
        }
        case CI_OP_LOAD:
            ok = ci_pop(stack, &sp, &a);
            if (ok) {
                switch (ins->type) {
                case CTR_CINTERP_T_I8:
                    v.i = *(int8_t*)a.p;
                    break;
                case CTR_CINTERP_T_U8:
                    v.u = *(uint8_t*)a.p;
                    break;
                case CTR_CINTERP_T_I16:
                    v.i = *(int16_t*)a.p;
                    break;
                case CTR_CINTERP_T_U16:
                    v.u = *(uint16_t*)a.p;
                    break;
                case CTR_CINTERP_T_I32:
                    v.i = *(int32_t*)a.p;
                    break;
                case CTR_CINTERP_T_U32:
                    v.u = *(uint32_t*)a.p;
                    break;
                case CTR_CINTERP_T_F32:
                    v.f = *(float*)a.p;
                    break;
                case CTR_CINTERP_T_F64:
                    v.f = *(double*)a.p;
                    break;
                case CTR_CINTERP_T_PTR:
                    v.p = *(void**)a.p;
                    break;
                default:
                    v.i = *(int64_t*)a.p;
                    break;
                }
                ok = ci_push(stack, &sp, STACK_MAX, v);
            }
            break;
        case CI_OP_STORE:
            ok = ci_pop(stack, &sp, &a) && ci_pop(stack, &sp, &b);
            if (ok) {
                switch (ins->type) {
                case CTR_CINTERP_T_I8:
                case CTR_CINTERP_T_U8:
                    *(uint8_t*)b.p = (uint8_t)a.u;
                    break;
                case CTR_CINTERP_T_I16:
                case CTR_CINTERP_T_U16:
                    *(uint16_t*)b.p = (uint16_t)a.u;
                    break;
                case CTR_CINTERP_T_I32:
                case CTR_CINTERP_T_U32:
                    *(uint32_t*)b.p = (uint32_t)a.u;
                    break;
                case CTR_CINTERP_T_F32:
                    *(float*)b.p = (float)a.f;
                    break;
                case CTR_CINTERP_T_F64:
                    *(double*)b.p = a.f;
                    break;
                case CTR_CINTERP_T_PTR:
                    *(void**)b.p = a.p;
                    break;
                default:
                    *(uint64_t*)b.p = a.u;
                    break;
                }
            }
            break;
        case CI_OP_ADD:
        case CI_OP_SUB:
        case CI_OP_MUL:
        case CI_OP_DIV:
            ok = ci_pop(stack, &sp, &b) && ci_pop(stack, &sp, &a);
            if (ok) {
                if (ins->type == CTR_CINTERP_T_F32 || ins->type == CTR_CINTERP_T_F64) {
                    v.f = ins->op == CI_OP_ADD ? a.f + b.f : ins->op == CI_OP_SUB ? a.f - b.f
                        : ins->op == CI_OP_MUL                                    ? a.f * b.f
                                                                                  : a.f / b.f;
                } else {
                    v.i = ins->op == CI_OP_ADD ? a.i + b.i : ins->op == CI_OP_SUB ? a.i - b.i
                        : ins->op == CI_OP_MUL                                    ? a.i * b.i
                                                                                  : a.i / b.i;
                }
                ok = ci_push(stack, &sp, STACK_MAX, v);
            }
            break;
        case CI_OP_MOD:
            ok = ci_pop(stack, &sp, &b) && ci_pop(stack, &sp, &a);
            v.i = a.i % b.i;
            ok = ok && ci_push(stack, &sp, STACK_MAX, v);
            break;
        case CI_OP_NEG:
            ok = ci_pop(stack, &sp, &a);
            v.i = -a.i;
            ok = ok && ci_push(stack, &sp, STACK_MAX, v);
            break;
        case CI_OP_EQ:
        case CI_OP_NE:
        case CI_OP_LT:
        case CI_OP_LE:
        case CI_OP_GT:
        case CI_OP_GE:
            ok = ci_pop(stack, &sp, &b) && ci_pop(stack, &sp, &a);
            v.i = ins->op == CI_OP_EQ ? a.i == b.i : ins->op == CI_OP_NE ? a.i != b.i
                : ins->op == CI_OP_LT                                    ? a.i < b.i
                : ins->op == CI_OP_LE                                    ? a.i <= b.i
                : ins->op == CI_OP_GT                                    ? a.i > b.i
                                                                         : a.i >= b.i;
            ok = ok && ci_push(stack, &sp, STACK_MAX, v);
            break;
        case CI_OP_AND:
        case CI_OP_OR:
        case CI_OP_XOR:
        case CI_OP_SHL:
        case CI_OP_SHR:
            ok = ci_pop(stack, &sp, &b) && ci_pop(stack, &sp, &a);
            v.u = ins->op == CI_OP_AND ? a.u & b.u : ins->op == CI_OP_OR ? a.u | b.u
                : ins->op == CI_OP_XOR                                   ? a.u ^ b.u
                : ins->op == CI_OP_SHL                                   ? a.u << b.u
                                                                         : a.u >> b.u;
            ok = ok && ci_push(stack, &sp, STACK_MAX, v);
            break;
        case CI_OP_NOT:
            ok = ci_pop(stack, &sp, &a);
            v.u = ~a.u;
            ok = ok && ci_push(stack, &sp, STACK_MAX, v);
            break;
        case CI_OP_JMP:
            ok = ins->a >= 0 && (size_t)ins->a <= fn->code_count;
            pc = ins->a;
            break;
        case CI_OP_JZ:
            ok = ci_pop(stack, &sp, &a);
            if (ok && !a.i) {
                ok = ins->a >= 0 && (size_t)ins->a <= fn->code_count;
                pc = ins->a;
            }
            break;
        case CI_OP_JNZ:
            ok = ci_pop(stack, &sp, &a);
            if (ok && a.i) {
                ok = ins->a >= 0 && (size_t)ins->a <= fn->code_count;
                pc = ins->a;
            }
            break;
        case CI_OP_CALL:
        case CI_OP_CALL_EXT: {
            size_t call_argc = (size_t)ins->a;
            ctr_cinterp_value* call_args = calloc(call_argc ? call_argc : 1, sizeof(ctr_cinterp_value));
            ok = call_args != NULL && sp >= call_argc;
            for (size_t i = call_argc; ok && i > 0; i--)
                ok = ci_pop(stack, &sp, &call_args[i - 1]);
            memset(&v, 0, sizeof(v));
            if (ok && ins->op == CI_OP_CALL) {
                ctr_cinterp_function* callee = ctr_cinterp_find_function(interp, ins->name);
                ok = ci_call_function(interp, callee, call_args, call_argc, &v, depth + 1);
            } else if (ok) {
                ctr_cinterp_external* ext = ci_find_external(interp, ins->name);
                if (!ext)
                    ctr_cinterp_link_symbol(interp, ins->name, dlsym(RTLD_DEFAULT, ins->name));
                ext = ci_find_external(interp, ins->name);
                ok = ext && ci_call_external(interp, ext, call_args, &v);
            }
            free(call_args);
            ok = ok && ci_push(stack, &sp, STACK_MAX, v);
            break;
        }
        case CI_OP_RET:
            if (ret) {
                if (fn->ret_type == CTR_CINTERP_T_VOID)
                    memset(ret, 0, sizeof(*ret));
                else
                    ok = ci_pop(stack, &sp, ret);
            }
            goto done;
        case CI_OP_POP:
            ok = ci_pop(stack, &sp, &a);
            break;
        case CI_OP_DUP:
            ok = sp > 0 && ci_push(stack, &sp, STACK_MAX, stack[sp - 1]);
            break;
        case CI_OP_CAST:
            ok = ci_pop(stack, &sp, &a);
            v = ci_cast_value(a, ins->type);
            ok = ok && ci_push(stack, &sp, STACK_MAX, v);
            break;
        case CI_OP_PTR_ADD:
            ok = ci_pop(stack, &sp, &b) && ci_pop(stack, &sp, &a);
            v.p = (char*)a.p + b.i;
            ok = ok && ci_push(stack, &sp, STACK_MAX, v);
            break;
        case CI_OP_MEMCPY:
            ok = ci_pop(stack, &sp, &v) && ci_pop(stack, &sp, &b) && ci_pop(stack, &sp, &a);
            if (ok)
                memcpy(a.p, b.p, (size_t)v.u);
            break;
        case CI_OP_MEMSET:
            ok = ci_pop(stack, &sp, &v) && ci_pop(stack, &sp, &b) && ci_pop(stack, &sp, &a);
            if (ok)
                memset(a.p, (int)b.i, (size_t)v.u);
            break;
        }
    }
done:
    if (!ok)
        ci_error(interp, "Runtime error in bytecode function `%s'", fn->name);
    if (locals != locals_stack)
        free(locals);
    return ok;
}

int ctr_cinterp_call(ctr_cinterp* interp, char const* name, ctr_cinterp_value* argv, size_t argc, ctr_cinterp_value* ret)
{
    ctr_cinterp_function* fn = ctr_cinterp_find_function(interp, name);
    if (!fn) {
        ci_error(interp, "No such bytecode function `%s'", name);
        return 0;
    }
    return ci_call_function(interp, fn, argv, argc, ret, 0);
}

int ctr_cinterp_is_function_pointer(void* ptr)
{
    ctr_cinterp_function* fn = ptr;
    return fn && fn->magic == CTR_CINTERP_FUNCTION_MAGIC && fn->owner;
}

int ctr_cinterp_call_function_pointer(void* ptr, ctr_cinterp_value* argv, size_t argc, ctr_cinterp_value* ret)
{
    ctr_cinterp_function* fn = ptr;
    if (!ctr_cinterp_is_function_pointer(ptr))
        return 0;
    return ci_call_function(fn->owner, fn, argv, argc, ret, 0);
}

void* ctr_cinterp_resource_release_hook(void* state)
{
    ctr_cinterp_free(state);
    return NULL;
}

static ctr_cinterp* ci_from_ctr(ctr_object* myself, char const* action)
{
    ctr_resource* r = myself ? myself->value.rvalue : NULL;
    ctr_cinterp* interp = r ? r->ptr : NULL;
    if (!interp)
        CtrStdFlow = ctr_format_str("CInterpreter %s request to uninitialized object", action);
    return interp;
}

ctr_object* ctr_cinterp_make(ctr_object* myself, ctr_argument* argumentList)
{
    ctr_object* handle = ctr_internal_create_object(CTR_OBJECT_TYPE_OTEX);
    ctr_set_link_all(handle, myself ? myself : (CtrStdCInterp ? CtrStdCInterp : CtrStdObject));
    handle->release_hook = ctr_cinterp_resource_release_hook;
    handle->value.rvalue = ctr_heap_allocate(sizeof(ctr_resource));
    handle->value.rvalue->ptr = ctr_cinterp_new();
    return handle;
}

ctr_object* ctr_cinterp_compile(ctr_object* myself, ctr_argument* argumentList)
{
    ctr_cinterp* interp = ci_from_ctr(myself, "compile");
    if (!interp)
        return CtrStdNil;
    ctr_object* prg = argumentList->object;
    CTR_ENSURE_TYPE_STRING(prg);
    char* program = ctr_heap_allocate_cstring(prg);
    char* trimmed = ci_trim(program);
    int preprocessor_only = ci_collect_include_only_macros(interp, trimmed);
    int ok = preprocessor_only ? 1
        : (strncmp(trimmed, ".function", 9) == 0 || strncmp(trimmed, ".extern", 7) == 0 || strncmp(trimmed, ".global", 7) == 0
                ? ctr_cinterp_compile_text(interp, trimmed)
                : ctr_cinterp_compile_c(interp, trimmed));
    ctr_heap_free(program);
    if (!ok)
        CtrStdFlow = ctr_build_string_from_cstring("C bytecode compilation failed");
    return myself;
}

ctr_object* ctr_cinterp_defined_functions(ctr_object* myself, ctr_argument* argumentList)
{
    ctr_cinterp* interp = ci_from_ctr(myself, "definedFunctions");
    if (!interp)
        return CtrStdNil;
    if (argumentList && argumentList->object && argumentList->object != CtrStdNil) {
        ctr_object* map = ctr_map_new(CtrStdMap, NULL);
        ctr_argument put_arg = { 0 };
        ctr_argument key_arg = { 0 };
        put_arg.next = &key_arg;
        ctr_object* filter = argumentList->object;
        if (filter->info.type == CTR_OBJECT_TYPE_OTARRAY) {
            ctr_collection* arr = filter->value.avalue;
            for (ctr_size i = arr->tail; i < arr->head; i++) {
                ctr_object* key = ctr_internal_cast2string(arr->elements[i]);
                char* name = ctr_heap_allocate_cstring(key);
                ctr_cinterp_type arg_types[16];
                ctr_cinterp_type ret_type = CTR_CINTERP_T_I64;
                size_t argc = 0;
                ctr_cinterp_external* ext = ci_find_external(interp, name);
                ctr_cinterp_ctype* ctype = ci_find_ctype(interp, name);
                if (ctype) {
                    put_arg.object = ci_make_ctype_object(ctype);
                    key_arg.object = key;
                    ctr_map_put(map, &put_arg);
                    ctr_heap_free(name);
                    continue;
                } else if (ext) {
                    ret_type = ext->ret_type;
                    argc = ext->argc;
                    for (size_t j = 0; j < argc && j < 16; j++)
                        arg_types[j] = ext->arg_types[j];
                } else if (ci_builtin_signature(name, &ret_type, arg_types, &argc)) {
                    ci_add_external(interp, name, ret_type, arg_types, argc);
                } else {
                    ctr_cinterp_get_symbol(interp, name);
                }
                put_arg.object = ci_make_cif_object(ret_type, arg_types, argc);
                key_arg.object = key;
                ctr_map_put(map, &put_arg);
                ctr_heap_free(name);
            }
            return map;
        }
        if (filter->properties) {
            for (ctr_mapitem* item = filter->properties->head; item; item = item->next) {
                ctr_object* key = ctr_internal_cast2string(item->key);
                char* name = ctr_heap_allocate_cstring(key);
                ctr_cinterp_type arg_types[16];
                ctr_cinterp_type ret_type = CTR_CINTERP_T_I64;
                size_t argc = 0;
                ctr_cinterp_external* ext = ci_find_external(interp, name);
                ctr_cinterp_ctype* ctype = ci_find_ctype(interp, name);
                if (ctype) {
                    put_arg.object = ci_make_ctype_object(ctype);
                    key_arg.object = key;
                    ctr_map_put(map, &put_arg);
                    ctr_heap_free(name);
                    continue;
                } else if (ext) {
                    ret_type = ext->ret_type;
                    argc = ext->argc;
                    for (size_t j = 0; j < argc && j < 16; j++)
                        arg_types[j] = ext->arg_types[j];
                } else if (ci_builtin_signature(name, &ret_type, arg_types, &argc)) {
                    ci_add_external(interp, name, ret_type, arg_types, argc);
                } else {
                    ctr_cinterp_get_symbol(interp, name);
                }
                put_arg.object = ci_make_cif_object(ret_type, arg_types, argc);
                key_arg.object = key;
                ctr_map_put(map, &put_arg);
                ctr_heap_free(name);
            }
            return map;
        }
    }
    ctr_object* arr = ctr_array_new(CtrStdArray, NULL);
    for (size_t i = 0; i < interp->function_count; i++)
        ctr_array_push(arr, &(ctr_argument) { ctr_build_string_from_cstring(interp->functions[i].name), NULL });
    return arr;
}

ctr_object* ctr_cinterp_defined_macros_ctr(ctr_object* myself, ctr_argument* argumentList)
{
    ctr_cinterp* interp = ci_from_ctr(myself, "definedMacros");
    if (!interp)
        return CtrStdNil;
    ctr_object* map = ctr_map_new(CtrStdMap, NULL);
    if (!argumentList || !argumentList->object || argumentList->object == CtrStdNil)
        return map;
    ctr_object* filter = argumentList->object;
    ctr_argument put_arg = { 0 };
    ctr_argument key_arg = { 0 };
    put_arg.next = &key_arg;
    if (filter->info.type == CTR_OBJECT_TYPE_OTARRAY) {
        ctr_collection* arr = filter->value.avalue;
        for (ctr_size i = arr->tail; i < arr->head; i++) {
            ctr_object* key = ctr_internal_cast2string(arr->elements[i]);
            char* name = ctr_heap_allocate_cstring(key);
            char expanded[4096];
            char const* value = ci_expand_macro(interp, name, expanded, sizeof(expanded));
            put_arg.object = ctr_build_string_from_cstring((char*)(value ? value : name));
            key_arg.object = key;
            ctr_map_put(map, &put_arg);
            ctr_heap_free(name);
        }
        return map;
    }
    if (filter->properties) {
        for (ctr_mapitem* item = filter->properties->head; item; item = item->next) {
            ctr_object* key = ctr_internal_cast2string(item->key);
            char* name = ctr_heap_allocate_cstring(key);
            char expanded[4096];
            char const* value = ci_expand_macro(interp, name, expanded, sizeof(expanded));
            put_arg.object = ctr_build_string_from_cstring((char*)(value ? value : name));
            key_arg.object = key;
            ctr_map_put(map, &put_arg);
            ctr_heap_free(name);
        }
    }
    return map;
}

ctr_object* ctr_cinterp_get_symbol_ctr(ctr_object* myself, ctr_argument* argumentList)
{
    ctr_cinterp* interp = ci_from_ctr(myself, "symbol");
    if (!interp)
        return CtrStdNil;
    ctr_object* sym = argumentList->object;
    CTR_ENSURE_TYPE_STRING(sym);
    char* sym_s = ctr_heap_allocate_cstring(sym);
    void* ptr_v = ctr_cinterp_get_symbol(interp, sym_s);
    ctr_heap_free(sym_s);
    ctr_object* ptr = ctr_ctypes_make_pointer(NULL, NULL);
    ptr->value.rvalue->ptr = ptr_v;
    return ptr;
}

static ctr_cinterp_value ci_value_from_ctr_arg(ctr_object* obj)
{
    ctr_cinterp_value v;
    memset(&v, 0, sizeof(v));
    if (obj->info.type == CTR_OBJECT_TYPE_OTEX && obj->value.rvalue)
        v.p = obj->value.rvalue->ptr;
    else if (obj->info.type == CTR_OBJECT_TYPE_OTSTRING)
        v.p = ctr_heap_allocate_cstring(obj);
    else
        v.i = (int64_t)ctr_internal_cast2number(obj)->value.nvalue;
    return v;
}

static ctr_object* ci_call_from_ctr(ctr_cinterp* interp, char const* name, ctr_object* argl)
{
    CTR_ENSURE_TYPE_ARRAY(argl);
    int length = ctr_array_count(argl, NULL)->value.nvalue;
    ctr_cinterp_value* argv = ctr_heap_allocate(sizeof(ctr_cinterp_value) * (length ? length : 1));
    ctr_collection* arr = argl->value.avalue;
    for (int i = 0; i < length; i++)
        argv[i] = ci_value_from_ctr_arg(arr->elements[arr->tail + i]);
    ctr_cinterp_value ret;
    memset(&ret, 0, sizeof(ret));
    int ok = ctr_cinterp_call(interp, name, argv, length, &ret);
    for (int i = 0; i < length; i++) {
        ctr_object* obj = arr->elements[arr->tail + i];
        if (obj->info.type == CTR_OBJECT_TYPE_OTSTRING && argv[i].p)
            ctr_heap_free(argv[i].p);
    }
    ctr_heap_free(argv);
    if (!ok) {
        CtrStdFlow = ctr_format_str("C bytecode call to `%s' failed", name);
        return CtrStdNil;
    }
    return ctr_build_number_from_float((double)ret.i);
}

ctr_object* ctr_cinterp_run(ctr_object* myself, ctr_argument* argumentList)
{
    ctr_cinterp* interp = ci_from_ctr(myself, "run");
    if (!interp)
        return CtrStdNil;
    ctr_object* prg = argumentList->object;
    CTR_ENSURE_TYPE_STRING(prg);
    ctr_cinterp_compile(myself, argumentList);
    if (CtrStdFlow)
        return CtrStdNil;
    return ci_call_from_ctr(interp, "main", argumentList->next->object);
}

ctr_object* ctr_cinterp_run_named(ctr_object* myself, ctr_argument* argumentList)
{
    ctr_cinterp* interp = ci_from_ctr(myself, "run");
    if (!interp)
        return CtrStdNil;
    ctr_cinterp_compile(myself, argumentList);
    if (CtrStdFlow)
        return CtrStdNil;
    ctr_object* sym = argumentList->next->next->object;
    CTR_ENSURE_TYPE_STRING(sym);
    char* name = ctr_heap_allocate_cstring(sym);
    ctr_object* out = ci_call_from_ctr(interp, name, argumentList->next->object);
    ctr_heap_free(name);
    return out;
}

ctr_object* ctr_cinterp_add_libp_ctr(ctr_object* myself, ctr_argument* argumentList)
{
    ctr_cinterp* interp = ci_from_ctr(myself, "addLibraryPath");
    if (!interp)
        return CtrStdNil;
    ctr_object* l = argumentList->object;
    CTR_ENSURE_TYPE_STRING(l);
    char* ls = ctr_heap_allocate_cstring(l);
    int ok = ctr_cinterp_add_library_path(interp, ls);
    ctr_heap_free(ls);
    return ctr_build_bool(ok);
}

ctr_object* ctr_cinterp_get_libp_ctr(ctr_object* myself, ctr_argument* argumentList)
{
    ctr_cinterp* interp = ci_from_ctr(myself, "libraryPaths");
    if (!interp)
        return CtrStdNil;
    ctr_object* res = ctr_array_new(CtrStdArray, NULL);
    for (size_t i = 0; i < ctr_cinterp_library_path_count(interp); i++)
        ctr_array_push(res, &(ctr_argument) { ctr_build_string_from_cstring((char*)ctr_cinterp_library_path_at(interp, i)), NULL });
    return res;
}

ctr_object* ctr_cinterp_add_inclp_ctr(ctr_object* myself, ctr_argument* argumentList)
{
    ctr_cinterp* interp = ci_from_ctr(myself, "addIncludePath");
    if (!interp)
        return CtrStdNil;
    ctr_object* l = argumentList->object;
    CTR_ENSURE_TYPE_STRING(l);
    char* ls = ctr_heap_allocate_cstring(l);
    int ok = ctr_cinterp_add_library_path(interp, ls);
    ctr_heap_free(ls);
    return ctr_build_bool(ok);
}

ctr_object* ctr_cinterp_get_inclp_ctr(ctr_object* myself, ctr_argument* argumentList)
{
    return ctr_cinterp_get_libp_ctr(myself, argumentList);
}

ctr_object* ctr_cinterp_add_lib_ctr(ctr_object* myself, ctr_argument* argumentList)
{
    ctr_cinterp* interp = ci_from_ctr(myself, "linkInLibrary");
    if (!interp)
        return CtrStdNil;
    ctr_object* l = argumentList->object;
    CTR_ENSURE_TYPE_STRING(l);
    char* ls = ctr_heap_allocate_cstring(l);
    int ok = ctr_cinterp_link_library(interp, ls);
    ctr_heap_free(ls);
    if (!ok)
        CtrStdFlow = ctr_build_string_from_cstring("C bytecode library link failed");
    return myself;
}

static void ci_ctr_error_handler(void* userdata, char const* msg)
{
    ctr_object* blk = userdata;
    if (!blk || !(blk->info.type == CTR_OBJECT_TYPE_OTBLOCK || blk->info.type == CTR_OBJECT_TYPE_OTNATFUNC))
        return;
    ctr_argument arg = { ctr_build_string_from_cstring((char*)msg), NULL };
    ctr_block_run(blk, &arg, blk);
    ctr_heap_free(arg.object);
}

ctr_object* ctr_cinterp_set_error_handler_ctr(ctr_object* myself, ctr_argument* argumentList)
{
    ctr_cinterp* interp = ci_from_ctr(myself, "errorHandler");
    if (!interp)
        return CtrStdNil;
    ctr_cinterp_set_error_handler(interp, ci_ctr_error_handler, argumentList->object);
    return myself;
}

ctr_object* ctr_cinterp_generate_output_ctr(ctr_object* myself, ctr_argument* argumentList)
{
    if (!ci_from_ctr(myself, "outputTo"))
        return CtrStdNil;
    CtrStdFlow = ctr_build_string_from_cstring("CInterpreter does not generate native object/executable output");
    return CtrStdNil;
}

ctr_object* ctr_cinterp_finish(ctr_object* myself, ctr_argument* argumentList)
{
    ctr_resource* r = myself ? myself->value.rvalue : NULL;
    if (!r || !r->ptr) {
        CtrStdFlow = ctr_build_string_from_cstring("deinit request to uninitialized CInterpreter object");
        return CtrStdNil;
    }
    ctr_cinterp_free(r->ptr);
    r->ptr = NULL;
    return myself;
}
