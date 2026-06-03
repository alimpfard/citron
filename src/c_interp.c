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
    CI_OP_SWAP,
    CI_OP_CAST,
    CI_OP_PTR_ADD,
    CI_OP_MEMCPY,
    CI_OP_MEMSET,
    CI_OP_FRAME_ADDR,
    CI_OP_STORE_R,
    CI_OP_FUNC_ADDR,
    CI_OP_CALL_PTR
} ctr_cinterp_op;

typedef struct ctr_cinterp_instr {
    ctr_cinterp_op op;
    ctr_cinterp_type type;
    ctr_cinterp_type type2;
    ctr_cinterp_value imm;
    int a;
    int b;
    char* name;
    ctr_cinterp_type* arg_types;
} ctr_cinterp_instr;

typedef struct ci_dtype {
    ctr_cinterp_type prim;
    ctr_cinterp_type pointee;
    char* tag;
    int is_array;
    size_t array_len;
} ci_dtype;

typedef struct ci_struct_member {
    char* name;
    ci_dtype type;
    size_t offset;
    size_t size;
} ci_struct_member;

typedef struct ci_struct_def {
    char* name;
    ci_struct_member* members;
    size_t member_count;
    size_t size;
    size_t align;
} ci_struct_def;

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
    size_t frame_size;
    size_t* arg_offset;
    ctr_cinterp_type* arg_slot_type;
    size_t* arg_copy_size;
    void* closure_code;
};

#define CTR_CINTERP_FUNCTION_MAGIC UINT64_C(0x4354524349464e31)

typedef struct ctr_cinterp_external {
    char* name;
    void* ptr;
    ctr_cinterp_type ret_type;
    ctr_cinterp_type* arg_types;
    size_t argc;
    int variadic;
    ffi_cif cif;
    ffi_type** ffi_arg_types;
    int cif_ready;
} ctr_cinterp_external;

typedef struct ctr_cinterp_global {
    char* name;
    void* data;
    size_t size;
    ci_dtype dtype;
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
    ci_struct_def* structs;
    size_t struct_count;
    size_t struct_cap;
    struct ci_typedef_entry {
        char* name;
        ci_dtype type;
    }* typedefs;
    size_t typedef_count;
    size_t typedef_cap;
    struct ci_enumconst_entry {
        char* name;
        int64_t value;
    }* enum_consts;
    size_t enum_const_count;
    size_t enum_const_cap;
    struct ci_closure_entry {
        void* code;
        ctr_cinterp_function* fn;
        ffi_closure* closure;
        ffi_type** atypes;
        ffi_cif cif;
    }* closures;
    size_t closure_count;
    size_t closure_cap;
    char** library_paths;
    size_t library_path_count;
    size_t library_path_cap;
    void** library_handles;
    size_t library_handle_count;
    size_t library_handle_cap;
    ctr_cinterp_error_fn error_fn;
    void* error_userdata;
    char* last_error;
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
    if (interp) {
        free(interp->last_error);
        interp->last_error = ci_strdup(buf);
        if (interp->error_fn)
            interp->error_fn(interp->error_userdata, buf);
    }
}

static void ci_clear_error(ctr_cinterp* interp)
{
    if (!interp)
        return;
    free(interp->last_error);
    interp->last_error = NULL;
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

static void ci_seed_base_types(ctr_cinterp* interp);

ctr_cinterp* ctr_cinterp_new(void)
{
    ctr_cinterp* interp = calloc(1, sizeof(ctr_cinterp));
    if (interp)
        ci_seed_base_types(interp);
    return interp;
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
    for (size_t i = 0; i < fn->code_count; i++) {
        free(fn->code[i].name);
        free(fn->code[i].arg_types);
    }
    free(fn->code);
    free(fn->arg_offset);
    free(fn->arg_slot_type);
    free(fn->arg_copy_size);
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
        free(interp->globals[i].dtype.tag);
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
    for (size_t i = 0; i < interp->struct_count; i++) {
        free(interp->structs[i].name);
        for (size_t j = 0; j < interp->structs[i].member_count; j++) {
            free(interp->structs[i].members[j].name);
            free(interp->structs[i].members[j].type.tag);
        }
        free(interp->structs[i].members);
    }
    for (size_t i = 0; i < interp->typedef_count; i++) {
        free(interp->typedefs[i].name);
        free(interp->typedefs[i].type.tag);
    }
    for (size_t i = 0; i < interp->enum_const_count; i++)
        free(interp->enum_consts[i].name);
    for (size_t i = 0; i < interp->closure_count; i++) {
        if (interp->closures[i].closure)
            ffi_closure_free(interp->closures[i].closure);
        free(interp->closures[i].atypes);
    }
    free(interp->closures);
    for (size_t i = 0; i < interp->library_path_count; i++)
        free(interp->library_paths[i]);
    for (size_t i = 0; i < interp->library_handle_count; i++)
        dlclose(interp->library_handles[i]);
    free(interp->functions);
    free(interp->externals);
    free(interp->globals);
    free(interp->macros);
    free(interp->ctypes);
    free(interp->structs);
    free(interp->typedefs);
    free(interp->enum_consts);
    free(interp->library_paths);
    free(interp->library_handles);
    free(interp->last_error);
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

static ctr_cinterp_external* ci_add_external_ex(ctr_cinterp* interp, char const* name, ctr_cinterp_type ret_type, ctr_cinterp_type* arg_types, size_t argc, int variadic)
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
    ext->variadic = variadic;
    ext->arg_types = calloc(argc ? argc : 1, sizeof(ctr_cinterp_type));
    if (!ext->arg_types)
        return NULL;
    if (argc)
        memcpy(ext->arg_types, arg_types, argc * sizeof(ctr_cinterp_type));
    ext->ptr = dlsym(RTLD_DEFAULT, name);
    return ext;
}

static ctr_cinterp_external* ci_add_external(ctr_cinterp* interp, char const* name, ctr_cinterp_type ret_type, ctr_cinterp_type* arg_types, size_t argc)
{
    return ci_add_external_ex(interp, name, ret_type, arg_types, argc, 0);
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

static ci_struct_def* ci_find_struct(ctr_cinterp* interp, char const* name)
{
    if (!name)
        return NULL;
    for (size_t i = interp->struct_count; i > 0; i--) {
        if (strcmp(interp->structs[i - 1].name, name) == 0)
            return &interp->structs[i - 1];
    }
    return NULL;
}

static ci_struct_def* ci_add_struct(ctr_cinterp* interp, char const* name)
{
    if (!ci_grow((void**)&interp->structs, &interp->struct_cap, interp->struct_count, sizeof(ci_struct_def)))
        return NULL;
    ci_struct_def* def = &interp->structs[interp->struct_count++];
    memset(def, 0, sizeof(*def));
    def->name = ci_strdup(name);
    def->align = 1;
    return def;
}

static int ci_find_typedef(ctr_cinterp* interp, char const* name, ci_dtype* out)
{
    if (!name)
        return 0;
    for (size_t i = interp->typedef_count; i > 0; i--) {
        if (strcmp(interp->typedefs[i - 1].name, name) == 0) {
            *out = interp->typedefs[i - 1].type;
            return 1;
        }
    }
    return 0;
}

static void ci_add_typedef(ctr_cinterp* interp, char const* name, ci_dtype type)
{
    for (size_t i = 0; i < interp->typedef_count; i++) {
        if (strcmp(interp->typedefs[i].name, name) == 0)
            return;
    }
    if (!ci_grow((void**)&interp->typedefs, &interp->typedef_cap, interp->typedef_count, sizeof(*interp->typedefs)))
        return;
    interp->typedefs[interp->typedef_count].name = ci_strdup(name);
    interp->typedefs[interp->typedef_count].type = type;
    interp->typedefs[interp->typedef_count].type.tag = type.tag ? ci_strdup(type.tag) : NULL;
    interp->typedef_count++;
}

/* These match the LP64 ABI used by the headers this frontend scans. */
static void ci_seed_base_types(ctr_cinterp* interp)
{
    struct {
        char const* name;
        ctr_cinterp_type prim;
    } base[] = {
        { "size_t", CTR_CINTERP_T_U64 },
        { "ssize_t", CTR_CINTERP_T_I64 },
        { "ptrdiff_t", CTR_CINTERP_T_I64 },
        { "intptr_t", CTR_CINTERP_T_I64 },
        { "uintptr_t", CTR_CINTERP_T_U64 },
        { "wchar_t", CTR_CINTERP_T_I32 },
        { "wint_t", CTR_CINTERP_T_I32 },
        { "int8_t", CTR_CINTERP_T_I8 },
        { "uint8_t", CTR_CINTERP_T_U8 },
        { "int16_t", CTR_CINTERP_T_I16 },
        { "uint16_t", CTR_CINTERP_T_U16 },
        { "int32_t", CTR_CINTERP_T_I32 },
        { "uint32_t", CTR_CINTERP_T_U32 },
        { "int64_t", CTR_CINTERP_T_I64 },
        { "uint64_t", CTR_CINTERP_T_U64 },
        { "intmax_t", CTR_CINTERP_T_I64 },
        { "uintmax_t", CTR_CINTERP_T_U64 },
        { "off_t", CTR_CINTERP_T_I64 },
        { "mode_t", CTR_CINTERP_T_U32 },
        { "pid_t", CTR_CINTERP_T_I32 },
        { "uid_t", CTR_CINTERP_T_U32 },
        { "gid_t", CTR_CINTERP_T_U32 },
        { "time_t", CTR_CINTERP_T_I64 },
        { "clock_t", CTR_CINTERP_T_I64 },
        { "useconds_t", CTR_CINTERP_T_U32 },
        { "socklen_t", CTR_CINTERP_T_U32 },
        { "FILE", CTR_CINTERP_T_I32 },
        { "va_list", CTR_CINTERP_T_PTR },
        { "_Bool", CTR_CINTERP_T_U8 },
        { "bool", CTR_CINTERP_T_U8 },
    };
    for (size_t i = 0; i < sizeof(base) / sizeof(base[0]); i++) {
        ci_dtype dt;
        memset(&dt, 0, sizeof(dt));
        dt.prim = base[i].prim;
        ci_add_typedef(interp, base[i].name, dt);
    }
}

static int ci_find_enum_const(ctr_cinterp* interp, char const* name, int64_t* out)
{
    for (size_t i = interp->enum_const_count; i > 0; i--) {
        if (strcmp(interp->enum_consts[i - 1].name, name) == 0) {
            *out = interp->enum_consts[i - 1].value;
            return 1;
        }
    }
    return 0;
}

static void ci_add_enum_const(ctr_cinterp* interp, char const* name, int64_t value)
{
    if (!ci_grow((void**)&interp->enum_consts, &interp->enum_const_cap, interp->enum_const_count, sizeof(*interp->enum_consts)))
        return;
    interp->enum_consts[interp->enum_const_count].name = ci_strdup(name);
    interp->enum_consts[interp->enum_const_count].value = value;
    interp->enum_const_count++;
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
    if (ext->variadic)
        return 0;
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

static int ci_prepare_call_cif(ctr_cinterp_external* ext, ffi_cif* cif, ffi_type*** ffi_arg_types_out, ctr_cinterp_type* call_arg_types, size_t call_argc)
{
    if (!ext->variadic) {
        if (call_argc != ext->argc)
            return 0;
        *ffi_arg_types_out = NULL;
        return ci_prepare_cif(ext);
    }
    if (call_argc < ext->argc)
        return 0;
    ffi_type** ffi_arg_types = calloc(call_argc ? call_argc : 1, sizeof(ffi_type*));
    if (!ffi_arg_types)
        return 0;
    for (size_t i = 0; i < call_argc; i++) {
        ctr_cinterp_type type = i < ext->argc ? ext->arg_types[i] : call_arg_types[i];
        if (type == CTR_CINTERP_T_F32)
            type = CTR_CINTERP_T_F64;
        ffi_arg_types[i] = ctr_cinterp_ffi_type(type);
    }
    ffi_status status = ffi_prep_cif_var(cif, FFI_DEFAULT_ABI, ext->argc, call_argc, ctr_cinterp_ffi_type(ext->ret_type), ffi_arg_types);
    if (status != FFI_OK) {
        free(ffi_arg_types);
        return 0;
    }
    *ffi_arg_types_out = ffi_arg_types;
    return 1;
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
static void ci_collect_function_declarations(ctr_cinterp* interp, char const* source);

static int ci_append_text(char* out, size_t out_size, size_t* pos, char const* text, size_t text_len)
{
    if (*pos + text_len >= out_size)
        return 0;
    memcpy(out + *pos, text, text_len);
    *pos += text_len;
    out[*pos] = 0;
    return 1;
}

static int ci_split_macro_list(char* text, char** items, size_t* count, size_t max_count)
{
    *count = 0;
    int depth = 0;
    char* start = text;
    for (char* p = text;; p++) {
        if (*p == '(')
            depth++;
        else if (*p == ')' && depth > 0)
            depth--;
        if ((*p == ',' && depth == 0) || *p == 0) {
            if (*count >= max_count)
                return 0;
            char saved = *p;
            *p = 0;
            items[(*count)++] = ci_trim(start);
            if (!saved)
                break;
            start = p + 1;
        }
    }
    return 1;
}

static int ci_find_macro_arg(char const* name, size_t name_len, char** params, char** args, size_t count, char const** value, size_t* value_len)
{
    for (size_t i = 0; i < count; i++) {
        if (strlen(params[i]) == name_len && strncmp(params[i], name, name_len) == 0) {
            *value = args[i];
            *value_len = strlen(args[i]);
            return 1;
        }
    }
    return 0;
}

static void ci_collapse_token_paste(char* text)
{
    char* r = text;
    char* w = text;
    while (*r) {
        if (r[0] == '#' && r[1] == '#') {
            while (w > text && isspace((unsigned char)w[-1]))
                w--;
            r += 2;
            while (isspace((unsigned char)*r))
                r++;
            continue;
        }
        *w++ = *r++;
    }
    *w = 0;
}

static int ci_substitute_macro_params(ctr_cinterp* interp, char const* body, char const* param_list, char const* arg_list, char* out, size_t out_size)
{
    char params_copy[2048];
    char args_copy[4096];
    snprintf(params_copy, sizeof(params_copy), "%s", param_list ? param_list : "");
    snprintf(args_copy, sizeof(args_copy), "%s", arg_list ? arg_list : "");
    char* params[64];
    char* args[64];
    size_t param_count = 0;
    size_t arg_count = 0;
    if (!ci_split_macro_list(params_copy, params, &param_count, 64)
        || !ci_split_macro_list(args_copy, args, &arg_count, 64)
        || param_count != arg_count)
        return 0;

    size_t pos = 0;
    for (size_t i = 0; body[i];) {
        if (isalpha((unsigned char)body[i]) || body[i] == '_') {
            size_t start = i++;
            while (isalnum((unsigned char)body[i]) || body[i] == '_')
                i++;
            size_t len = i - start;
            char const* text = body + start;
            size_t text_len = len;
            char const* arg_value = NULL;
            size_t arg_value_len = 0;
            if (ci_find_macro_arg(text, len, params, args, param_count, &arg_value, &arg_value_len)) {
                text = arg_value;
                text_len = arg_value_len;
            } else {
                char name[256];
                if (len < sizeof(name)) {
                    memcpy(name, text, len);
                    name[len] = 0;
                    char const* macro_value = ci_find_macro(interp, name);
                    if (macro_value) {
                        text = macro_value;
                        text_len = strlen(macro_value);
                    }
                }
            }
            if (!ci_append_text(out, out_size, &pos, text, text_len))
                return 0;
            continue;
        }
        if (!ci_append_text(out, out_size, &pos, body + i, 1))
            return 0;
        i++;
    }
    out[pos] = 0;
    ci_collapse_token_paste(out);
    return 1;
}

static int ci_expand_macro_text(ctr_cinterp* interp, char const* text, char* out, size_t out_size, int depth)
{
    if (depth > 32)
        return 0;
    char work[16384];
    snprintf(work, sizeof(work), "%s", text ? text : "");
    char* trimmed = ci_trim(work);

    for (char* scan = trimmed; *scan; scan++) {
        if (!(isalpha((unsigned char)*scan) || *scan == '_'))
            continue;
        char* name_start = scan;
        scan++;
        while (isalnum((unsigned char)*scan) || *scan == '_')
            scan++;
        size_t name_len = (size_t)(scan - name_start);
        char macro_name[256];
        if (name_len >= sizeof(macro_name))
            continue;
        memcpy(macro_name, name_start, name_len);
        macro_name[name_len] = 0;

        ctr_cinterp_macro* macro = ci_find_macro_entry(interp, macro_name);
        if (!macro)
            continue;

        if (!macro->function_like) {
            char replaced[16384] = "";
            size_t pos = 0;
            if (!ci_append_text(replaced, sizeof(replaced), &pos, trimmed, (size_t)(name_start - trimmed))
                || !ci_append_text(replaced, sizeof(replaced), &pos, macro->value, strlen(macro->value))
                || !ci_append_text(replaced, sizeof(replaced), &pos, scan, strlen(scan)))
                return 0;
            return ci_expand_macro_text(interp, replaced, out, out_size, depth + 1);
        }

        char* open = scan;
        while (isspace((unsigned char)*open))
            open++;
        if (*open != '(')
            continue;
        int paren_depth = 1;
        char* close = open + 1;
        while (*close && paren_depth) {
            if (*close == '(')
                paren_depth++;
            else if (*close == ')')
                paren_depth--;
            close++;
        }
        if (paren_depth)
            continue;
        close--;
        char saved = *close;
        *close = 0;
        char* args = open + 1;
        char substituted[16384];
        int ok = ci_substitute_macro_params(interp, macro->value, macro->param, args, substituted, sizeof(substituted));
        *close = saved;
        if (ok) {
            char replaced[16384] = "";
            size_t pos = 0;
            if (!ci_append_text(replaced, sizeof(replaced), &pos, trimmed, (size_t)(name_start - trimmed))
                || !ci_append_text(replaced, sizeof(replaced), &pos, substituted, strlen(substituted))
                || !ci_append_text(replaced, sizeof(replaced), &pos, close + 1, strlen(close + 1)))
                return 0;
            return ci_expand_macro_text(interp, replaced, out, out_size, depth + 1);
        }
    }

    snprintf(out, out_size, "%s", trimmed);
    return 1;
}

static char* ci_join_line_continuations(char const* source)
{
    size_t len = strlen(source);
    char* out = malloc(len + 1);
    if (!out)
        return NULL;
    size_t pos = 0;
    for (size_t i = 0; source[i]; i++) {
        if (source[i] == '\\' && source[i + 1] == '\n') {
            i++;
            continue;
        }
        out[pos++] = source[i];
    }
    out[pos] = 0;
    return out;
}

static int ci_parse_directive(char* s, char** directive, char** rest)
{
    s = ci_trim(s);
    if (*s != '#')
        return 0;
    s++;
    while (isspace((unsigned char)*s))
        s++;
    *directive = s;
    while (*s && !isspace((unsigned char)*s))
        s++;
    if (*s)
        *s++ = 0;
    *rest = ci_trim(s);
    return **directive != 0;
}

static void ci_remove_macro(ctr_cinterp* interp, char const* name)
{
    for (size_t i = 0; i < interp->macro_count; i++) {
        if (strcmp(interp->macros[i].name, name) == 0) {
            free(interp->macros[i].name);
            free(interp->macros[i].value);
            free(interp->macros[i].param);
            memmove(&interp->macros[i], &interp->macros[i + 1],
                (interp->macro_count - i - 1) * sizeof(interp->macros[i]));
            interp->macro_count--;
            return;
        }
    }
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

static void ci_pp_skip(char const** s)
{
    while (isspace((unsigned char)**s))
        (*s)++;
}

static long ci_pp_expr(char const** s, int min_prec);

static long ci_pp_unary(char const** s)
{
    ci_pp_skip(s);
    char c = **s;
    if (c == '(') {
        (*s)++;
        long v = ci_pp_expr(s, 0);
        ci_pp_skip(s);
        if (**s == ')')
            (*s)++;
        return v;
    }
    if (c == '!') {
        (*s)++;
        return !ci_pp_unary(s);
    }
    if (c == '~') {
        (*s)++;
        return ~ci_pp_unary(s);
    }
    if (c == '-') {
        (*s)++;
        return -ci_pp_unary(s);
    }
    if (c == '+') {
        (*s)++;
        return ci_pp_unary(s);
    }
    if (isdigit((unsigned char)c)) {
        char* e = NULL;
        long v = strtol(*s, &e, 0);
        *s = e;
        while (**s == 'u' || **s == 'U' || **s == 'l' || **s == 'L')
            (*s)++;
        return v;
    }
    if (isalpha((unsigned char)c) || c == '_') {
        while (isalnum((unsigned char)**s) || **s == '_')
            (*s)++;
        return 0;
    }
    if (c)
        (*s)++;
    return 0;
}

static int ci_pp_binop(char const* s, int* len)
{
    if (s[0] == '|' && s[1] == '|') {
        *len = 2;
        return 1;
    }
    if (s[0] == '&' && s[1] == '&') {
        *len = 2;
        return 2;
    }
    if (s[0] == '|') {
        *len = 1;
        return 3;
    }
    if (s[0] == '^') {
        *len = 1;
        return 4;
    }
    if (s[0] == '&') {
        *len = 1;
        return 5;
    }
    if (s[0] == '=' && s[1] == '=') {
        *len = 2;
        return 6;
    }
    if (s[0] == '!' && s[1] == '=') {
        *len = 2;
        return 6;
    }
    if (s[0] == '<' && s[1] == '=') {
        *len = 2;
        return 7;
    }
    if (s[0] == '>' && s[1] == '=') {
        *len = 2;
        return 7;
    }
    if (s[0] == '<' && s[1] == '<') {
        *len = 2;
        return 8;
    }
    if (s[0] == '>' && s[1] == '>') {
        *len = 2;
        return 8;
    }
    if (s[0] == '<') {
        *len = 1;
        return 7;
    }
    if (s[0] == '>') {
        *len = 1;
        return 7;
    }
    if (s[0] == '+') {
        *len = 1;
        return 9;
    }
    if (s[0] == '-') {
        *len = 1;
        return 9;
    }
    if (s[0] == '*') {
        *len = 1;
        return 10;
    }
    if (s[0] == '/') {
        *len = 1;
        return 10;
    }
    if (s[0] == '%') {
        *len = 1;
        return 10;
    }
    return 0;
}

static long ci_pp_apply(long a, long b, char const* op, int len)
{
    if (len == 2) {
        if (op[0] == '|' && op[1] == '|')
            return a || b;
        if (op[0] == '&' && op[1] == '&')
            return a && b;
        if (op[0] == '=' && op[1] == '=')
            return a == b;
        if (op[0] == '!' && op[1] == '=')
            return a != b;
        if (op[0] == '<' && op[1] == '=')
            return a <= b;
        if (op[0] == '>' && op[1] == '=')
            return a >= b;
        if (op[0] == '<' && op[1] == '<')
            return a << b;
        if (op[0] == '>' && op[1] == '>')
            return a >> b;
    }
    switch (op[0]) {
    case '|':
        return a | b;
    case '^':
        return a ^ b;
    case '&':
        return a & b;
    case '<':
        return a < b;
    case '>':
        return a > b;
    case '+':
        return a + b;
    case '-':
        return a - b;
    case '*':
        return a * b;
    case '/':
        return b ? a / b : 0;
    case '%':
        return b ? a % b : 0;
    }
    return 0;
}

static long ci_pp_expr(char const** s, int min_prec)
{
    long lhs = ci_pp_unary(s);
    for (;;) {
        ci_pp_skip(s);
        int len = 0;
        int prec = ci_pp_binop(*s, &len);
        if (!prec || prec < min_prec)
            break;
        char op[3] = { (*s)[0], len == 2 ? (*s)[1] : (char)0, 0 };
        *s += len;
        long rhs = ci_pp_expr(s, prec + 1);
        lhs = ci_pp_apply(lhs, rhs, op, len);
    }
    return lhs;
}

static int ci_pp_eval(ctr_cinterp* interp, char const* expr)
{
    char resolved[8192];
    size_t o = 0;
    for (char const* s = expr; *s && o + 2 < sizeof(resolved);) {
        if (strncmp(s, "defined", 7) == 0 && !(isalnum((unsigned char)s[7]) || s[7] == '_')) {
            s += 7;
            while (isspace((unsigned char)*s))
                s++;
            int paren = 0;
            if (*s == '(') {
                paren = 1;
                s++;
                while (isspace((unsigned char)*s))
                    s++;
            }
            char nm[256];
            size_t n = 0;
            while ((isalnum((unsigned char)*s) || *s == '_') && n + 1 < sizeof(nm))
                nm[n++] = *s++;
            nm[n] = 0;
            if (paren) {
                while (isspace((unsigned char)*s))
                    s++;
                if (*s == ')')
                    s++;
            }
            resolved[o++] = ci_find_macro_entry(interp, nm) ? '1' : '0';
            continue;
        }
        resolved[o++] = *s++;
    }
    resolved[o] = 0;
    char expanded[16384];
    if (!ci_expand_macro_text(interp, resolved, expanded, sizeof(expanded), 0))
        snprintf(expanded, sizeof(expanded), "%s", resolved);
    char const* cur = expanded;
    return ci_pp_expr(&cur, 0) != 0;
}

typedef struct ci_strbuf {
    char* data;
    size_t len;
    size_t cap;
} ci_strbuf;

static void ci_sb_append(ci_strbuf* b, char const* s, size_t n)
{
    if (b->len + n + 1 > b->cap) {
        size_t nc = b->cap ? b->cap * 2 : 1024;
        while (nc < b->len + n + 1)
            nc *= 2;
        char* nd = realloc(b->data, nc);
        if (!nd)
            return;
        b->data = nd;
        b->cap = nc;
    }
    memcpy(b->data + b->len, s, n);
    b->len += n;
    b->data[b->len] = 0;
}

static void ci_preprocess(ctr_cinterp* interp, char const* source, ci_strbuf* out, int depth)
{
    if (depth > 24)
        return;
    char* copy = ci_join_line_continuations(source);
    if (!copy)
        return;
    struct {
        int parent_emit;
        int active;
        int taken;
    } cond[64];
    int ctop = 0;
    char* saveptr = NULL;
    for (char* line = strtok_r(copy, "\n", &saveptr); line; line = strtok_r(NULL, "\n", &saveptr)) {
        int emit = ctop == 0 ? 1 : cond[ctop - 1].active;
        char* s = ci_trim(line);
        char* directive = NULL;
        char* rest = NULL;
        char tmp[8192];
        snprintf(tmp, sizeof(tmp), "%s", s);
        if (ci_parse_directive(tmp, &directive, &rest)) {
            if (strcmp(directive, "if") == 0 || strcmp(directive, "ifdef") == 0 || strcmp(directive, "ifndef") == 0) {
                int cond_val = 0;
                if (emit) {
                    if (strcmp(directive, "if") == 0)
                        cond_val = ci_pp_eval(interp, rest);
                    else {
                        char nm[256];
                        sscanf(rest, "%255s", nm);
                        int def = ci_find_macro_entry(interp, nm) != NULL;
                        cond_val = strcmp(directive, "ifdef") == 0 ? def : !def;
                    }
                }
                if (ctop < 64) {
                    cond[ctop].parent_emit = emit;
                    cond[ctop].active = emit && cond_val;
                    cond[ctop].taken = emit && cond_val;
                    ctop++;
                }
            } else if (strcmp(directive, "elif") == 0) {
                if (ctop > 0) {
                    int pe = cond[ctop - 1].parent_emit;
                    if (!pe || cond[ctop - 1].taken) {
                        cond[ctop - 1].active = 0;
                    } else {
                        int cv = ci_pp_eval(interp, rest);
                        cond[ctop - 1].active = cv;
                        cond[ctop - 1].taken = cv;
                    }
                }
            } else if (strcmp(directive, "else") == 0) {
                if (ctop > 0) {
                    cond[ctop - 1].active = cond[ctop - 1].parent_emit && !cond[ctop - 1].taken;
                    cond[ctop - 1].taken = 1;
                }
            } else if (strcmp(directive, "endif") == 0) {
                if (ctop > 0)
                    ctop--;
            } else if (!emit) {
            } else if (strcmp(directive, "define") == 0) {
                char* d = rest;
                char* name = d;
                while (*d && !isspace((unsigned char)*d) && *d != '(')
                    d++;
                char saved = *d;
                *d = 0;
                if (*name && saved == '(') {
                    char* param = d + 1;
                    char* end = strchr(param, ')');
                    if (end) {
                        *end = 0;
                        ci_set_macro_ex(interp, name, ci_trim(end + 1), ci_trim(param), 1);
                    }
                } else {
                    char* value = saved ? ci_trim(d + 1) : "";
                    ci_set_macro(interp, name, value);
                }
            } else if (strcmp(directive, "undef") == 0) {
                char nm[256];
                sscanf(rest, "%255s", nm);
                ci_remove_macro(interp, nm);
            } else if (strcmp(directive, "include") == 0 || strcmp(directive, "include_next") == 0) {
                char* inc = rest;
                char end = *inc == '<' ? '>' : *inc == '"' ? '"'
                                                           : 0;
                if (end) {
                    inc++;
                    char* e = strchr(inc, end);
                    if (e) {
                        *e = 0;
                        char full[4096];
                        char* contents = NULL;
                        if (end == '"')
                            contents = ci_read_file(inc);
                        for (size_t i = 0; i < interp->library_path_count && !contents; i++) {
                            snprintf(full, sizeof(full), "%s/%s", interp->library_paths[i], inc);
                            contents = ci_read_file(full);
                        }
                        if (!contents && end == '>')
                            contents = ci_read_file((snprintf(full, sizeof(full), "/usr/include/%s", inc), full));
                        if (contents) {
                            ci_collect_typedefs(interp, contents);
                            ci_collect_function_declarations(interp, contents);
                            ci_strbuf sink = { 0 };
                            ci_preprocess(interp, contents, &sink, depth + 1);
                            free(sink.data);
                            free(contents);
                        }
                    }
                }
            }
            continue;
        }
        if (!emit)
            continue;
        char expanded[16384];
        if (ci_expand_macro_text(interp, s, expanded, sizeof(expanded), 0))
            ci_sb_append(out, expanded, strlen(expanded));
        else
            ci_sb_append(out, s, strlen(s));
        ci_sb_append(out, "\n", 1);
    }
    free(copy);
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
    CI_CTOK_FNUM,
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
    CI_CTOK_STRUCT,
    CI_CTOK_UNION,
    CI_CTOK_ENUM,
    CI_CTOK_TYPEDEF,
    CI_CTOK_DO,
    CI_CTOK_SWITCH,
    CI_CTOK_CASE,
    CI_CTOK_DEFAULT,
    CI_CTOK_BREAK,
    CI_CTOK_CONTINUE,
    CI_CTOK_GOTO,
    CI_CTOK_SIZEOF
} ci_ctok_kind;

typedef struct ci_ctok {
    ci_ctok_kind kind;
    char* text;
    int64_t num;
    double fnum;
    int line;
} ci_ctok;

typedef struct ci_cvar {
    char* name;
    size_t offset;
    ci_dtype type;
    int is_global;
    char* global_name;
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
    ctr_cinterp_type last_type;
    ctr_cinterp_type last_pointee;
    char* last_tag;
    int last_is_array;
    size_t last_array_len;
    struct ci_jlist* brk_ctx;
    struct ci_jlist* cont_ctx;
    struct ci_swctx* sw_ctx;
    int anon_struct_id;
    struct ci_label {
        char* name;
        size_t pos;
    }* labels;
    size_t label_count;
    size_t label_cap;
    struct ci_goto {
        char* name;
        size_t at;
        int line;
    }* gotos;
    size_t goto_count;
    size_t goto_cap;
    int ok;
} ci_cparser;

typedef struct ci_jlist {
    size_t* items;
    size_t count;
    size_t cap;
} ci_jlist;

typedef struct ci_swctx {
    struct {
        int64_t v;
        size_t pos;
    } cases[256];
    size_t ncase;
    size_t default_pos;
    int has_default;
} ci_swctx;

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
    if (strcmp(text, "union") == 0)
        return CI_CTOK_UNION;
    if (strcmp(text, "enum") == 0)
        return CI_CTOK_ENUM;
    if (strcmp(text, "typedef") == 0)
        return CI_CTOK_TYPEDEF;
    if (strcmp(text, "do") == 0)
        return CI_CTOK_DO;
    if (strcmp(text, "switch") == 0)
        return CI_CTOK_SWITCH;
    if (strcmp(text, "case") == 0)
        return CI_CTOK_CASE;
    if (strcmp(text, "default") == 0)
        return CI_CTOK_DEFAULT;
    if (strcmp(text, "break") == 0)
        return CI_CTOK_BREAK;
    if (strcmp(text, "continue") == 0)
        return CI_CTOK_CONTINUE;
    if (strcmp(text, "goto") == 0)
        return CI_CTOK_GOTO;
    if (strcmp(text, "sizeof") == 0)
        return CI_CTOK_SIZEOF;
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
        if (isdigit((unsigned char)*p) || (p[0] == '.' && isdigit((unsigned char)p[1]))) {
            char const* start = p;
            int is_float = 0;
            if (p[0] == '0' && (p[1] == 'x' || p[1] == 'X')) {
                p += 2;
                while (isxdigit((unsigned char)*p))
                    p++;
            } else {
                while (isdigit((unsigned char)*p))
                    p++;
                if (*p == '.') {
                    is_float = 1;
                    p++;
                    while (isdigit((unsigned char)*p))
                        p++;
                }
                if (*p == 'e' || *p == 'E') {
                    is_float = 1;
                    p++;
                    if (*p == '+' || *p == '-')
                        p++;
                    while (isdigit((unsigned char)*p))
                        p++;
                }
            }
            if (is_float) {
                tok.kind = CI_CTOK_FNUM;
                tok.fnum = strtod(start, NULL);
            } else {
                tok.kind = CI_CTOK_NUM;
                tok.num = (int64_t)strtoull(start, NULL, 0);
            }
            while (*p == 'u' || *p == 'U' || *p == 'l' || *p == 'L' || *p == 'f' || *p == 'F')
                p++;
            tok.text = ci_strdup("");
            if (!ci_tok_push(&toks, &count, &cap, tok))
                goto oom;
            continue;
        }
        if (*p == '\'') {
            p++;
            int64_t value = 0;
            if (*p == '\\') {
                p++;
                switch (*p) {
                case 'n':
                    value = '\n';
                    break;
                case 't':
                    value = '\t';
                    break;
                case 'r':
                    value = '\r';
                    break;
                case '0':
                    value = '\0';
                    break;
                case 'b':
                    value = '\b';
                    break;
                case 'f':
                    value = '\f';
                    break;
                case 'v':
                    value = '\v';
                    break;
                case 'a':
                    value = '\a';
                    break;
                default:
                    value = (unsigned char)*p;
                    break;
                }
                if (*p)
                    p++;
            } else if (*p) {
                value = (unsigned char)*p++;
            }
            if (*p == '\'')
                p++;
            tok.kind = CI_CTOK_NUM;
            tok.num = value;
            tok.text = ci_strdup("");
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
        {
            static char const* const three[] = { "<<=", ">>=", "...", NULL };
            static char const* const two[] = { "==", "!=", "<=", ">=", "&&", "||",
                "<<", ">>", "->", "++", "--", "+=", "-=", "*=", "/=", "%=",
                "&=", "|=", "^=", NULL };
            char const* matched = NULL;
            for (size_t i = 0; three[i]; i++) {
                if (p[0] == three[i][0] && p[1] == three[i][1] && p[2] == three[i][2]) {
                    matched = three[i];
                    p += 3;
                    break;
                }
            }
            for (size_t i = 0; !matched && two[i]; i++) {
                if (p[0] == two[i][0] && p[1] == two[i][1]) {
                    matched = two[i];
                    p += 2;
                    break;
                }
            }
            tok.kind = CI_CTOK_PUNCT;
            if (matched) {
                tok.text = ci_strdup(matched);
            } else {
                char one[2] = { *p++, 0 };
                tok.text = ci_strdup(one);
            }
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
    if (!p->ok)
        return;
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

static char* cp_parse_declarator(ci_cparser* p, ci_dtype base, ci_dtype* out);
static void cp_define_struct_body(ci_cparser* p, char const* name, int is_union);
static int64_t cp_eval_const(ci_cparser* p);
static void cp_parse_enum_body(ci_cparser* p);

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
    case CI_CTOK_UNION:
    case CI_CTOK_ENUM:
    case CI_CTOK_TYPEDEF:
        return 1;
    case CI_CTOK_IDENT: {
        ci_dtype td;
        return ci_find_typedef(p->interp, cp_peek(p)->text, &td);
    }
    default:
        return 0;
    }
}

static void cp_apply_stars(ci_cparser* p, ci_dtype* dt)
{
    while (cp_match_kind(p, CI_CTOK_CONST))
        ;
    while (cp_match_punct(p, "*")) {
        if (dt->prim != CTR_CINTERP_T_PTR)
            dt->pointee = dt->prim;
        else
            dt->pointee = CTR_CINTERP_T_VOID;
        dt->prim = CTR_CINTERP_T_PTR;
        dt->is_array = 0;
        while (cp_match_kind(p, CI_CTOK_CONST))
            ;
    }
}

static int cp_parse_dtype_ex(ci_cparser* p, ci_dtype* out, int* storage_extern, int consume_stars)
{
    int is_unsigned = 0;
    int saw = 0;
    ci_dtype dt;
    memset(&dt, 0, sizeof(dt));
    dt.prim = CTR_CINTERP_T_I32;
    if (storage_extern)
        *storage_extern = 0;
    for (;;) {
        if (cp_match_kind(p, CI_CTOK_CONST) || cp_match_kind(p, CI_CTOK_STATIC))
            continue;
        if (cp_match_kind(p, CI_CTOK_EXTERN)) {
            if (storage_extern)
                *storage_extern = 1;
            continue;
        }
        if (cp_match_kind(p, CI_CTOK_TYPEDEF)) {
            if (storage_extern)
                *storage_extern = 2;
            continue;
        }
        break;
    }
    int is_union_kw = cp_peek(p)->kind == CI_CTOK_UNION;
    if (cp_match_kind(p, CI_CTOK_STRUCT) || cp_match_kind(p, CI_CTOK_UNION)) {
        if (cp_peek(p)->kind == CI_CTOK_IDENT) {
            free(dt.tag);
            dt.tag = ci_strdup(cp_peek(p)->text);
            p->pos++;
        }
        if (cp_is_punct(p, "{")) {
            if (!dt.tag) {
                char anon[64];
                snprintf(anon, sizeof(anon), ".anon%d", p->anon_struct_id++);
                dt.tag = ci_strdup(anon);
            }
            cp_define_struct_body(p, dt.tag, is_union_kw);
        }
        dt.prim = CTR_CINTERP_T_VOID;
        saw = 1;
    } else if (cp_match_kind(p, CI_CTOK_ENUM)) {
        if (cp_peek(p)->kind == CI_CTOK_IDENT)
            p->pos++;
        if (cp_is_punct(p, "{"))
            cp_parse_enum_body(p);
        dt.prim = CTR_CINTERP_T_I32;
        saw = 1;
    } else {
        if (cp_match_kind(p, CI_CTOK_UNSIGNED))
            is_unsigned = 1;
        else if (cp_match_kind(p, CI_CTOK_SIGNED))
            is_unsigned = 0;
        if (cp_match_kind(p, CI_CTOK_VOID)) {
            dt.prim = CTR_CINTERP_T_VOID;
            saw = 1;
        } else if (cp_match_kind(p, CI_CTOK_CHAR)) {
            dt.prim = is_unsigned ? CTR_CINTERP_T_U8 : CTR_CINTERP_T_I8;
            saw = 1;
        } else if (cp_match_kind(p, CI_CTOK_SHORT)) {
            cp_match_kind(p, CI_CTOK_INT);
            dt.prim = is_unsigned ? CTR_CINTERP_T_U16 : CTR_CINTERP_T_I16;
            saw = 1;
        } else if (cp_match_kind(p, CI_CTOK_LONG)) {
            cp_match_kind(p, CI_CTOK_LONG);
            cp_match_kind(p, CI_CTOK_INT);
            dt.prim = is_unsigned ? CTR_CINTERP_T_U64 : CTR_CINTERP_T_I64;
            saw = 1;
        } else if (cp_match_kind(p, CI_CTOK_INT)) {
            dt.prim = is_unsigned ? CTR_CINTERP_T_U32 : CTR_CINTERP_T_I32;
            saw = 1;
        } else if (cp_match_kind(p, CI_CTOK_FLOAT)) {
            dt.prim = CTR_CINTERP_T_F32;
            saw = 1;
        } else if (cp_match_kind(p, CI_CTOK_DOUBLE)) {
            dt.prim = CTR_CINTERP_T_F64;
            saw = 1;
        } else if (is_unsigned) {
            dt.prim = CTR_CINTERP_T_U32;
            saw = 1;
        } else if (cp_peek(p)->kind == CI_CTOK_IDENT) {
            ci_dtype td;
            if (ci_find_typedef(p->interp, cp_peek(p)->text, &td)) {
                free(dt.tag);
                dt = td;
                dt.tag = td.tag ? ci_strdup(td.tag) : NULL;
                p->pos++;
                saw = 1;
            }
        }
    }
    if (!saw) {
        free(dt.tag);
        cp_error(p, "expected C type");
        return 0;
    }
    if (consume_stars)
        cp_apply_stars(p, &dt);
    *out = dt;
    return 1;
}

static int cp_parse_dtype(ci_cparser* p, ci_dtype* out, int* storage_extern)
{
    return cp_parse_dtype_ex(p, out, storage_extern, 1);
}

static int cp_parse_type(ci_cparser* p, ctr_cinterp_type* out, int* storage_extern)
{
    ci_dtype dt;
    if (!cp_parse_dtype(p, &dt, storage_extern))
        return 0;
    ctr_cinterp_type t = (dt.prim == CTR_CINTERP_T_VOID && dt.tag) ? CTR_CINTERP_T_PTR : dt.prim;
    free(dt.tag);
    *out = t;
    return 1;
}

static int ci_token_can_start_decl(ci_ctok* tok)
{
    switch (tok->kind) {
    case CI_CTOK_EXTERN:
    case CI_CTOK_STATIC:
    case CI_CTOK_CONST:
    case CI_CTOK_INT:
    case CI_CTOK_LONG:
    case CI_CTOK_SHORT:
    case CI_CTOK_CHAR:
    case CI_CTOK_VOID:
    case CI_CTOK_FLOAT:
    case CI_CTOK_DOUBLE:
    case CI_CTOK_UNSIGNED:
    case CI_CTOK_SIGNED:
    case CI_CTOK_STRUCT:
        return 1;
    default:
        return 0;
    }
}

static int ci_collect_one_function_declaration(ctr_cinterp* interp, char const* decl)
{
    if (strstr(decl, "typedef") || strstr(decl, "(*"))
        return 0;

    ci_ctok* toks = NULL;
    size_t count = 0;
    if (!ci_lex_c(interp, decl, &toks, &count))
        return 0;

    ci_cparser p;
    memset(&p, 0, sizeof(p));
    p.interp = interp;
    p.toks = toks;
    p.count = count;
    p.ok = 1;

    if (!ci_token_can_start_decl(cp_peek(&p))) {
        ci_free_tokens(toks, count);
        return 0;
    }

    int storage_extern = 0;
    ctr_cinterp_type ret_type;
    if (!cp_parse_type(&p, &ret_type, &storage_extern) || !p.ok) {
        ci_free_tokens(toks, count);
        return 0;
    }
    if (cp_peek(&p)->kind != CI_CTOK_IDENT) {
        ci_free_tokens(toks, count);
        return 0;
    }
    char* name = ci_strdup(cp_peek(&p)->text);
    p.pos++;
    if (!cp_match_punct(&p, "(")) {
        free(name);
        ci_free_tokens(toks, count);
        return 0;
    }

    ctr_cinterp_type arg_types[128];
    size_t argc = 0;
    int variadic = 0;
    if (!cp_is_punct(&p, ")")) {
        do {
            if (cp_match_punct(&p, "...")) {
                variadic = 1;
                break;
            }
            if (cp_peek(&p)->kind == CI_CTOK_VOID && p.toks[p.pos + 1].kind == CI_CTOK_PUNCT && strcmp(p.toks[p.pos + 1].text, ")") == 0) {
                p.pos++;
                break;
            }
            if (argc >= 128) {
                p.ok = 0;
                break;
            }
            int ignored_storage = 0;
            if (!cp_parse_type(&p, &arg_types[argc], &ignored_storage) || !p.ok)
                break;
            while (cp_peek(&p)->kind == CI_CTOK_IDENT || cp_is_punct(&p, "*"))
                p.pos++;
            while (cp_match_punct(&p, "[")) {
                while (!cp_is_punct(&p, "]") && cp_peek(&p)->kind != CI_CTOK_EOF)
                    p.pos++;
                cp_match_punct(&p, "]");
                arg_types[argc] = CTR_CINTERP_T_PTR;
            }
            argc++;
        } while (cp_match_punct(&p, ","));
    }

    int ok = p.ok && cp_match_punct(&p, ")");
    if (ok)
        ci_add_external_ex(interp, name, ret_type, arg_types, argc, variadic);
    free(name);
    ci_free_tokens(toks, count);
    return ok;
}

static void ci_collect_function_declarations(ctr_cinterp* interp, char const* source)
{
    char* copy = ci_strdup(source);
    if (!copy)
        return;
    char* start = copy;
    for (char* p = copy; *p; p++) {
        if (*p != ';')
            continue;
        char saved = p[1];
        p[1] = 0;
        char* decl = ci_trim(start);
        if (strchr(decl, '(') && strchr(decl, ')'))
            ci_collect_one_function_declaration(interp, decl);
        p[1] = saved;
        start = p + 1;
    }
    free(copy);
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

static size_t ci_align_up(size_t v, size_t a)
{
    if (a < 1)
        a = 1;
    return (v + a - 1) / a * a;
}

static size_t ci_prim_size(ctr_cinterp_type t)
{
    size_t s = ctr_cinterp_type_size(t);
    return s ? s : 1;
}

static int ci_type_is_float(ctr_cinterp_type t)
{
    return t == CTR_CINTERP_T_F32 || t == CTR_CINTERP_T_F64;
}

static int ci_type_is_void(ctr_cinterp_type t)
{
    return t == CTR_CINTERP_T_VOID;
}

static size_t ci_struct_size_of(ctr_cinterp* interp, char const* tag)
{
    ci_struct_def* def = ci_find_struct(interp, tag);
    return def ? def->size : 0;
}

static size_t ci_elem_size(ctr_cinterp* interp, ci_dtype dt)
{
    if (dt.pointee == CTR_CINTERP_T_VOID && dt.tag)
        return ci_struct_size_of(interp, dt.tag);
    if (dt.pointee == CTR_CINTERP_T_VOID)
        return 1;
    return ci_prim_size(dt.pointee);
}

static size_t ci_storage_size(ctr_cinterp* interp, ci_dtype dt)
{
    if (dt.is_array)
        return dt.array_len * ci_elem_size(interp, dt);
    if (dt.prim == CTR_CINTERP_T_VOID && dt.tag)
        return ci_struct_size_of(interp, dt.tag);
    return ci_prim_size(dt.prim);
}

static size_t ci_storage_align(ctr_cinterp* interp, ci_dtype dt)
{
    if (dt.is_array || (dt.prim == CTR_CINTERP_T_PTR)) {
        if (dt.is_array)
            return ci_elem_size(interp, dt) > 8 ? 8 : ci_elem_size(interp, dt);
        return sizeof(void*);
    }
    if (dt.prim == CTR_CINTERP_T_VOID && dt.tag) {
        ci_struct_def* def = ci_find_struct(interp, dt.tag);
        return def && def->align ? def->align : 1;
    }
    return ci_prim_size(dt.prim);
}

static int ci_dt_is_struct(ci_dtype dt)
{
    return dt.prim == CTR_CINTERP_T_VOID && dt.tag && !dt.is_array;
}

static void cp_set_scalar(ci_cparser* p, ctr_cinterp_type prim)
{
    p->last_type = prim;
    p->last_pointee = CTR_CINTERP_T_VOID;
    p->last_tag = NULL;
    p->last_is_array = 0;
    p->last_array_len = 0;
}

static void cp_set_dtype(ci_cparser* p, ci_dtype dt)
{
    p->last_type = dt.prim;
    p->last_pointee = dt.pointee;
    p->last_tag = dt.tag;
    p->last_is_array = dt.is_array;
    p->last_array_len = dt.array_len;
}

static ci_dtype cp_get_dtype(ci_cparser* p)
{
    ci_dtype dt;
    memset(&dt, 0, sizeof(dt));
    dt.prim = p->last_type;
    dt.pointee = p->last_pointee;
    dt.tag = p->last_tag;
    dt.is_array = p->last_is_array;
    dt.array_len = p->last_array_len;
    return dt;
}

static ci_cvar* cp_add_var(ci_cparser* p, char const* name, ci_dtype type)
{
    if (!ci_grow((void**)&p->vars, &p->var_cap, p->var_count, sizeof(ci_cvar)))
        return NULL;
    ci_cvar* var = &p->vars[p->var_count++];
    memset(var, 0, sizeof(*var));
    var->name = ci_strdup(name);
    var->type = type;
    var->type.tag = type.tag ? ci_strdup(type.tag) : NULL;
    if (p->fn) {
        size_t sz = ci_storage_size(p->interp, type);
        size_t al = ci_storage_align(p->interp, type);
        size_t off = ci_align_up(p->fn->frame_size, al ? al : 1);
        var->offset = off;
        p->fn->frame_size = off + (sz ? sz : 1);
    }
    return var;
}

static void cp_free_vars(ci_cparser* p)
{
    for (size_t i = 0; i < p->var_count; i++) {
        free(p->vars[i].name);
        free(p->vars[i].global_name);
        free(p->vars[i].type.tag);
    }
    free(p->vars);
    p->vars = NULL;
    p->var_count = 0;
    p->var_cap = 0;
}

static void cp_emit_var_addr(ci_cparser* p, ci_cvar* var)
{
    ctr_cinterp_instr ins;
    memset(&ins, 0, sizeof(ins));
    if (var->is_global) {
        ins.op = CI_OP_GLOBAL_ADDR;
        ins.name = ci_strdup(var->global_name);
    } else {
        ins.op = CI_OP_FRAME_ADDR;
        ins.a = (int)var->offset;
    }
    cp_emit(p, ins);
}

static void cp_emit_load(ci_cparser* p, ci_dtype dt)
{
    if (dt.is_array) {
        ci_dtype res;
        memset(&res, 0, sizeof(res));
        res.prim = CTR_CINTERP_T_PTR;
        res.pointee = dt.pointee;
        res.tag = dt.tag;
        cp_set_dtype(p, res);
        return;
    }
    if (ci_dt_is_struct(dt)) {
        cp_set_dtype(p, dt);
        return;
    }
    ctr_cinterp_instr ins;
    memset(&ins, 0, sizeof(ins));
    ins.op = CI_OP_LOAD;
    ins.type = dt.prim;
    cp_emit(p, ins);
    cp_set_dtype(p, dt);
}

static void cp_emit_store(ci_cparser* p, ctr_cinterp_type prim, int keep_value)
{
    ctr_cinterp_instr ins;
    memset(&ins, 0, sizeof(ins));
    ins.op = keep_value ? CI_OP_STORE_R : CI_OP_STORE;
    ins.type = prim;
    cp_emit(p, ins);
}

static void cp_emit_const_i64(ci_cparser* p, int64_t v)
{
    ctr_cinterp_instr ins;
    memset(&ins, 0, sizeof(ins));
    ins.op = CI_OP_CONST;
    ins.type = CTR_CINTERP_T_I64;
    ins.imm.i = v;
    cp_emit(p, ins);
}

static void cp_emit_pop_if_value(ci_cparser* p)
{
    if (ci_type_is_void(p->last_type))
        return;
    ctr_cinterp_instr pop = { .op = CI_OP_POP };
    cp_emit(p, pop);
}

static void cp_coerce(ci_cparser* p, ctr_cinterp_type target)
{
    if (target == CTR_CINTERP_T_VOID || target == p->last_type)
        return;
    int tf = ci_type_is_float(target);
    int sf = ci_type_is_float(p->last_type);
    if (tf || sf) {
        ctr_cinterp_instr ins;
        memset(&ins, 0, sizeof(ins));
        ins.op = CI_OP_CAST;
        ins.type = target;
        ins.type2 = p->last_type;
        cp_emit(p, ins);
        cp_set_scalar(p, target);
    }
}

static ctr_cinterp_type ci_promote_binary_type(ctr_cinterp_type a, ctr_cinterp_type b)
{
    if (a == CTR_CINTERP_T_F64 || b == CTR_CINTERP_T_F64)
        return CTR_CINTERP_T_F64;
    if (a == CTR_CINTERP_T_F32 || b == CTR_CINTERP_T_F32)
        return CTR_CINTERP_T_F32;
    if (a == CTR_CINTERP_T_PTR || b == CTR_CINTERP_T_PTR)
        return CTR_CINTERP_T_PTR;
    return CTR_CINTERP_T_I64;
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
static void cp_parse_assign(ci_cparser* p);
static void cp_parse_cond(ci_cparser* p);
static void cp_parse_unary(ci_cparser* p);
static void cp_parse_stmt(ci_cparser* p);
static int cp_parse_lvalue_addr(ci_cparser* p);
static int64_t cp_eval_const(ci_cparser* p);

static void cp_truncate_code(ci_cparser* p, size_t n)
{
    if (!p->fn)
        return;
    for (size_t i = n; i < p->fn->code_count; i++) {
        free(p->fn->code[i].name);
        free(p->fn->code[i].arg_types);
    }
    if (n < p->fn->code_count)
        p->fn->code_count = n;
}

static ci_dtype ci_pointee_object(ci_dtype ptr)
{
    ci_dtype obj;
    memset(&obj, 0, sizeof(obj));
    if (ptr.pointee == CTR_CINTERP_T_VOID && ptr.tag) {
        obj.prim = CTR_CINTERP_T_VOID;
        obj.tag = ptr.tag;
    } else {
        obj.prim = ptr.pointee ? ptr.pointee : CTR_CINTERP_T_I64;
    }
    return obj;
}

static ci_dtype ci_pointer_to(ci_dtype obj)
{
    ci_dtype ptr;
    memset(&ptr, 0, sizeof(ptr));
    ptr.prim = CTR_CINTERP_T_PTR;
    if (ci_dt_is_struct(obj)) {
        ptr.pointee = CTR_CINTERP_T_VOID;
        ptr.tag = obj.tag;
    } else if (obj.is_array) {
        ptr.pointee = obj.pointee;
        ptr.tag = obj.tag;
    } else {
        ptr.pointee = obj.prim;
    }
    return ptr;
}

static void cp_emit_call(ci_cparser* p, char* name)
{
    ctr_cinterp_function* declfn = ctr_cinterp_find_function(p->interp, name);
    ctr_cinterp_external* declext = ci_find_external(p->interp, name);
    ci_cvar* fpvar = (!declfn && !declext) ? cp_find_var(p, name) : NULL;
    ctr_cinterp_global* fpglobal = (!declfn && !declext && !fpvar) ? ci_find_global(p->interp, name) : NULL;
    int indirect = fpvar || fpglobal;
    ctr_cinterp_type const* param_types = declfn ? NULL : (declext ? declext->arg_types : NULL);
    size_t param_count = declfn ? declfn->argc : (declext ? declext->argc : 0);
    int variadic = declext ? declext->variadic : 0;
    ctr_cinterp_type arg_types[128];
    size_t argc = 0;
    if (!cp_is_punct(p, ")")) {
        do {
            if (argc >= 128) {
                cp_error(p, "too many function call arguments");
                break;
            }
            cp_parse_assign(p);
            if (param_types && argc < param_count && !(variadic && argc >= param_count))
                cp_coerce(p, param_types[argc]);
            arg_types[argc] = p->last_type;
            argc++;
        } while (cp_match_punct(p, ","));
    }
    cp_expect_punct(p, ")");
    if (indirect) {
        if (fpvar)
            cp_emit_var_addr(p, fpvar);
        else {
            ctr_cinterp_instr g;
            memset(&g, 0, sizeof(g));
            g.op = CI_OP_GLOBAL_ADDR;
            g.name = ci_strdup(name);
            cp_emit(p, g);
        }
        cp_emit(p, (ctr_cinterp_instr) { .op = CI_OP_LOAD, .type = CTR_CINTERP_T_PTR });
        ctr_cinterp_instr ci;
        memset(&ci, 0, sizeof(ci));
        ci.op = CI_OP_CALL_PTR;
        ci.a = (int)argc;
        if (argc) {
            ci.arg_types = calloc(argc, sizeof(ctr_cinterp_type));
            if (ci.arg_types)
                memcpy(ci.arg_types, arg_types, argc * sizeof(ctr_cinterp_type));
        }
        cp_emit(p, ci);
        cp_set_scalar(p, CTR_CINTERP_T_I64);
        free(name);
        return;
    }
    ctr_cinterp_instr ins;
    memset(&ins, 0, sizeof(ins));
    ctr_cinterp_function* fn = declfn;
    ctr_cinterp_external* ext = declext;
    ctr_cinterp_type ret = CTR_CINTERP_T_I64;
    if (fn) {
        ins.op = CI_OP_CALL;
        ret = fn->ret_type;
    } else if (ext) {
        ins.op = CI_OP_CALL_EXT;
        ret = ext->ret_type;
    } else {
        cp_error(p, "call to unknown C function `%s' (include or declare it first)", name);
        free(name);
        return;
    }
    ins.name = name;
    ins.a = (int)argc;
    if (argc) {
        ins.arg_types = calloc(argc, sizeof(ctr_cinterp_type));
        if (!ins.arg_types) {
            cp_error(p, "out of memory recording call argument types");
            free(name);
            return;
        }
        memcpy(ins.arg_types, arg_types, argc * sizeof(ctr_cinterp_type));
    }
    cp_emit(p, ins);
    cp_set_scalar(p, ret);
}

static void cp_parse_primary(ci_cparser* p)
{
    if (cp_match_punct(p, "(")) {
        cp_parse_expr(p);
        cp_expect_punct(p, ")");
        return;
    }
    if (cp_peek(p)->kind == CI_CTOK_NUM) {
        int64_t n = p->toks[p->pos++].num;
        cp_emit_const_i64(p, n);
        cp_set_scalar(p, (n > INT32_MAX || n < INT32_MIN) ? CTR_CINTERP_T_I64 : CTR_CINTERP_T_I32);
        return;
    }
    if (cp_peek(p)->kind == CI_CTOK_FNUM) {
        ctr_cinterp_instr ins;
        memset(&ins, 0, sizeof(ins));
        ins.op = CI_OP_CONST;
        ins.type = CTR_CINTERP_T_F64;
        ins.imm.f = p->toks[p->pos++].fnum;
        cp_emit(p, ins);
        cp_set_scalar(p, CTR_CINTERP_T_F64);
        return;
    }
    if (cp_peek(p)->kind == CI_CTOK_STR) {
        cp_add_string_global(p, p->toks[p->pos++].text);
        ci_dtype d;
        memset(&d, 0, sizeof(d));
        d.prim = CTR_CINTERP_T_PTR;
        d.pointee = CTR_CINTERP_T_I8;
        cp_set_dtype(p, d);
        return;
    }
    if (cp_peek(p)->kind == CI_CTOK_IDENT) {
        char* name = ci_strdup(cp_peek(p)->text);
        p->pos++;
        if (cp_match_punct(p, "(")) {
            cp_emit_call(p, name);
            return;
        }
        int64_t ev;
        if (ci_find_enum_const(p->interp, name, &ev)) {
            cp_emit_const_i64(p, ev);
            cp_set_scalar(p, CTR_CINTERP_T_I32);
            free(name);
            return;
        }
        ctr_cinterp_instr ins;
        memset(&ins, 0, sizeof(ins));
        if (ctr_cinterp_find_function(p->interp, name) || ci_find_external(p->interp, name))
            ins.op = CI_OP_FUNC_ADDR;
        else
            ins.op = CI_OP_GLOBAL_ADDR;
        ins.name = name;
        cp_emit(p, ins);
        cp_set_scalar(p, CTR_CINTERP_T_PTR);
        return;
    }
    cp_error(p, "expected expression");
}

static int cp_resolve_ident_addr(ci_cparser* p, char const* name, ci_dtype* out)
{
    ci_cvar* var = cp_find_var(p, name);
    if (var) {
        cp_emit_var_addr(p, var);
        *out = var->type;
        return 1;
    }
    ctr_cinterp_global* g = ci_find_global(p->interp, name);
    if (g) {
        ctr_cinterp_instr ins;
        memset(&ins, 0, sizeof(ins));
        ins.op = CI_OP_GLOBAL_ADDR;
        ins.name = ci_strdup(name);
        cp_emit(p, ins);
        *out = g->dtype;
        if (out->prim == CTR_CINTERP_T_VOID && !out->tag)
            out->prim = CTR_CINTERP_T_I64;
        return 1;
    }
    return 0;
}

static int cp_lvalue_postfix(ci_cparser* p)
{
    for (;;) {
        if (cp_match_punct(p, "[")) {
            ci_dtype base = cp_get_dtype(p);
            ci_dtype elem;
            size_t esz;
            if (base.is_array) {
                elem = ci_pointee_object(base);
                esz = ci_elem_size(p->interp, base);
            } else if (base.prim == CTR_CINTERP_T_PTR) {
                cp_emit_load(p, base);
                base = cp_get_dtype(p);
                elem = ci_pointee_object(base);
                esz = ci_elem_size(p->interp, base);
            } else {
                cp_error(p, "subscripted value is not an array or pointer");
                return 1;
            }
            cp_parse_expr(p);
            cp_expect_punct(p, "]");
            if (esz != 1) {
                cp_emit_const_i64(p, (int64_t)esz);
                ctr_cinterp_instr mul = { .op = CI_OP_MUL, .type = CTR_CINTERP_T_I64 };
                cp_emit(p, mul);
            }
            ctr_cinterp_instr add = { .op = CI_OP_PTR_ADD };
            cp_emit(p, add);
            cp_set_dtype(p, elem);
        } else if (cp_match_punct(p, ".") || cp_match_punct(p, "->")) {
            int arrow = strcmp(p->toks[p->pos - 1].text, "->") == 0;
            ci_dtype base = cp_get_dtype(p);
            char const* tag = base.tag;
            if (arrow) {
                if (base.prim != CTR_CINTERP_T_PTR || !base.tag) {
                    cp_error(p, "`->' used on non-pointer-to-struct");
                    return 1;
                }
                cp_emit_load(p, base);
            } else if (!ci_dt_is_struct(base)) {
                cp_error(p, "`.' used on non-struct value");
                return 1;
            }
            char* field = cp_expect_ident(p);
            if (!field)
                return 1;
            ci_struct_def* def = ci_find_struct(p->interp, tag);
            ci_struct_member* m = NULL;
            for (size_t i = 0; def && i < def->member_count; i++) {
                if (strcmp(def->members[i].name, field) == 0) {
                    m = &def->members[i];
                    break;
                }
            }
            if (!m) {
                cp_error(p, "no member `%s' in struct `%s'", field, tag ? tag : "?");
                free(field);
                return 1;
            }
            if (m->offset) {
                cp_emit_const_i64(p, (int64_t)m->offset);
                ctr_cinterp_instr add = { .op = CI_OP_PTR_ADD };
                cp_emit(p, add);
            }
            cp_set_dtype(p, m->type);
            free(field);
        } else {
            break;
        }
    }
    return 1;
}

static int cp_parse_lvalue_addr(ci_cparser* p)
{
    if (cp_match_punct(p, "*")) {
        cp_parse_unary(p);
        ci_dtype ptr = cp_get_dtype(p);
        cp_set_dtype(p, ci_pointee_object(ptr));
        return cp_lvalue_postfix(p);
    }
    if (cp_is_punct(p, "(")) {
        size_t spos = p->pos;
        size_t scode = p->fn->code_count;
        p->pos++;
        if (cp_parse_lvalue_addr(p) && cp_match_punct(p, ")"))
            return cp_lvalue_postfix(p);
        cp_truncate_code(p, scode);
        p->pos = spos;
        return 0;
    }
    if (cp_peek(p)->kind == CI_CTOK_IDENT
        && !(p->toks[p->pos + 1].kind == CI_CTOK_PUNCT && strcmp(p->toks[p->pos + 1].text, "(") == 0)) {
        char* name = ci_strdup(cp_peek(p)->text);
        ci_dtype dt;
        if (!cp_resolve_ident_addr(p, name, &dt)) {
            free(name);
            return 0;
        }
        free(name);
        p->pos++;
        cp_set_dtype(p, dt);
        return cp_lvalue_postfix(p);
    }
    return 0;
}

static void cp_emit_incdec(ci_cparser* p, ci_dtype lvt, int delta, int is_postfix)
{
    int isf = lvt.prim == CTR_CINTERP_T_F32 || lvt.prim == CTR_CINTERP_T_F64;
    size_t esz = lvt.prim == CTR_CINTERP_T_PTR ? ci_elem_size(p->interp, lvt) : 1;
    int64_t step = (int64_t)esz * delta;
    ctr_cinterp_instr add = { .op = CI_OP_ADD,
        .type = isf ? lvt.prim : CTR_CINTERP_T_I64,
        .a = isf ? (int)lvt.prim : CTR_CINTERP_T_I64,
        .b = CTR_CINTERP_T_I64 };
    if (is_postfix) {
        cp_emit(p, (ctr_cinterp_instr) { .op = CI_OP_DUP });
        cp_emit_load(p, lvt);
        cp_emit(p, (ctr_cinterp_instr) { .op = CI_OP_SWAP });
        cp_emit(p, (ctr_cinterp_instr) { .op = CI_OP_DUP });
        cp_emit_load(p, lvt);
        cp_emit_const_i64(p, step);
        cp_emit(p, add);
        cp_emit_store(p, lvt.prim, 0);
    } else {
        cp_emit(p, (ctr_cinterp_instr) { .op = CI_OP_DUP });
        cp_emit_load(p, lvt);
        cp_emit_const_i64(p, step);
        cp_emit(p, add);
        cp_emit_store(p, lvt.prim, 1);
    }
}

static void cp_parse_unary(ci_cparser* p)
{
    if (cp_is_punct(p, "(") && cp_is_type_start(&(ci_cparser) { .toks = p->toks, .pos = p->pos + 1, .count = p->count, .interp = p->interp, .ok = p->ok })) {
        size_t save = p->pos;
        ci_dtype cast_dt;
        int ignored_storage = 0;
        cp_match_punct(p, "(");
        if (cp_parse_dtype(p, &cast_dt, &ignored_storage) && cp_match_punct(p, ")")) {
            cp_parse_unary(p);
            ctr_cinterp_instr ins;
            memset(&ins, 0, sizeof(ins));
            ins.op = CI_OP_CAST;
            ins.type = cast_dt.prim;
            ins.type2 = p->last_type;
            cp_emit(p, ins);
            cp_set_dtype(p, cast_dt);
            free(cast_dt.tag);
            return;
        }
        free(cast_dt.tag);
        p->pos = save;
    }
    if (cp_match_kind(p, CI_CTOK_SIZEOF)) {
        size_t scode = p->fn->code_count;
        size_t spos = p->pos;
        ci_dtype dt;
        memset(&dt, 0, sizeof(dt));
        int got = 0;
        if (cp_is_punct(p, "(") && cp_is_type_start(&(ci_cparser) { .toks = p->toks, .pos = p->pos + 1, .count = p->count, .interp = p->interp, .ok = p->ok })) {
            cp_match_punct(p, "(");
            int ig = 0;
            if (cp_parse_dtype(p, &dt, &ig)) {
                while (cp_match_punct(p, "[")) {
                    if (cp_peek(p)->kind == CI_CTOK_NUM) {
                        dt.is_array = 1;
                        dt.pointee = dt.prim;
                        dt.array_len = (size_t)p->toks[p->pos++].num;
                    }
                    cp_expect_punct(p, "]");
                }
                cp_expect_punct(p, ")");
                got = 1;
            }
        }
        if (!got) {
            free(dt.tag);
            memset(&dt, 0, sizeof(dt));
            p->pos = spos;
            if (cp_parse_lvalue_addr(p))
                dt = cp_get_dtype(p);
            else {
                cp_truncate_code(p, scode);
                p->pos = spos;
                cp_parse_unary(p);
                dt = cp_get_dtype(p);
            }
        }
        size_t sz = ci_storage_size(p->interp, dt);
        if (got)
            free(dt.tag);
        cp_truncate_code(p, scode);
        cp_emit_const_i64(p, (int64_t)(sz ? sz : 1));
        cp_set_scalar(p, CTR_CINTERP_T_U64);
        return;
    }
    if (cp_is_punct(p, "++") || cp_is_punct(p, "--")) {
        int delta = cp_is_punct(p, "++") ? 1 : -1;
        p->pos++;
        if (!cp_parse_lvalue_addr(p)) {
            cp_error(p, "operand of prefix ++/-- is not an lvalue");
            return;
        }
        ci_dtype lvt = cp_get_dtype(p);
        cp_emit_incdec(p, lvt, delta, 0);
        cp_set_dtype(p, lvt);
        return;
    }
    if (cp_match_punct(p, "-")) {
        cp_parse_unary(p);
        ctr_cinterp_instr ins = { .op = CI_OP_NEG, .type = p->last_type };
        cp_emit(p, ins);
    } else if (cp_match_punct(p, "+")) {
        cp_parse_unary(p);
    } else if (cp_match_punct(p, "~")) {
        cp_parse_unary(p);
        ctr_cinterp_instr ins = { .op = CI_OP_NOT };
        cp_emit(p, ins);
        cp_set_scalar(p, CTR_CINTERP_T_I64);
    } else if (cp_match_punct(p, "!")) {
        cp_parse_unary(p);
        cp_emit_const_i64(p, 0);
        ctr_cinterp_instr eq = { .op = CI_OP_EQ, .type = p->last_type, .a = p->last_type, .b = CTR_CINTERP_T_I64 };
        cp_emit(p, eq);
        cp_set_scalar(p, CTR_CINTERP_T_I64);
    } else if (cp_match_punct(p, "&")) {
        if (cp_peek(p)->kind == CI_CTOK_IDENT && !cp_find_var(p, cp_peek(p)->text)
            && (ctr_cinterp_find_function(p->interp, cp_peek(p)->text) || ci_find_external(p->interp, cp_peek(p)->text))) {
            ctr_cinterp_instr ins;
            memset(&ins, 0, sizeof(ins));
            ins.op = CI_OP_FUNC_ADDR;
            ins.name = ci_strdup(cp_peek(p)->text);
            p->pos++;
            cp_emit(p, ins);
            cp_set_scalar(p, CTR_CINTERP_T_PTR);
            return;
        }
        if (!cp_parse_lvalue_addr(p)) {
            cp_error(p, "operand of unary `&' is not an lvalue");
            return;
        }
        cp_set_dtype(p, ci_pointer_to(cp_get_dtype(p)));
    } else if (cp_match_punct(p, "*")) {
        cp_parse_unary(p);
        ci_dtype obj = ci_pointee_object(cp_get_dtype(p));
        cp_emit_load(p, obj);
    } else {
        size_t spos = p->pos;
        size_t scode = p->fn->code_count;
        if (cp_parse_lvalue_addr(p)) {
            ci_dtype dt = cp_get_dtype(p);
            if (cp_is_punct(p, "++") || cp_is_punct(p, "--")) {
                int delta = cp_is_punct(p, "++") ? 1 : -1;
                p->pos++;
                cp_emit_incdec(p, dt, delta, 1);
                cp_set_dtype(p, dt);
                return;
            }
            cp_emit_load(p, dt);
            return;
        }
        cp_truncate_code(p, scode);
        p->pos = spos;
        cp_parse_primary(p);
    }
}

static void cp_parse_mul(ci_cparser* p)
{
    cp_parse_unary(p);
    while (cp_is_punct(p, "*") || cp_is_punct(p, "/") || cp_is_punct(p, "%")) {
        ctr_cinterp_type lhs_type = p->last_type;
        char op = cp_peek(p)->text[0];
        p->pos++;
        cp_parse_unary(p);
        ctr_cinterp_type rhs_type = p->last_type;
        ctr_cinterp_instr ins;
        memset(&ins, 0, sizeof(ins));
        ins.op = op == '*' ? CI_OP_MUL : op == '/' ? CI_OP_DIV
                                                   : CI_OP_MOD;
        ins.type = ci_promote_binary_type(lhs_type, rhs_type);
        ins.a = lhs_type;
        ins.b = rhs_type;
        cp_emit(p, ins);
        cp_set_scalar(p, op == '%' ? CTR_CINTERP_T_I64 : ins.type);
    }
}

static void cp_parse_add(ci_cparser* p)
{
    cp_parse_mul(p);
    while (cp_is_punct(p, "+") || cp_is_punct(p, "-")) {
        ci_dtype lt = cp_get_dtype(p);
        int sub = cp_is_punct(p, "-");
        p->pos++;
        cp_parse_mul(p);
        ci_dtype rt = cp_get_dtype(p);
        int lptr = lt.prim == CTR_CINTERP_T_PTR;
        int rptr = rt.prim == CTR_CINTERP_T_PTR;
        if (lptr && rptr && sub) {
            ctr_cinterp_instr s = { .op = CI_OP_SUB, .type = CTR_CINTERP_T_I64, .a = CTR_CINTERP_T_I64, .b = CTR_CINTERP_T_I64 };
            cp_emit(p, s);
            size_t esz = ci_elem_size(p->interp, lt);
            if (esz > 1) {
                cp_emit_const_i64(p, (int64_t)esz);
                ctr_cinterp_instr d = { .op = CI_OP_DIV, .type = CTR_CINTERP_T_I64, .a = CTR_CINTERP_T_I64, .b = CTR_CINTERP_T_I64 };
                cp_emit(p, d);
            }
            cp_set_scalar(p, CTR_CINTERP_T_I64);
        } else if (lptr || rptr) {
            ci_dtype pt = lptr ? lt : rt;
            size_t esz = ci_elem_size(p->interp, pt);
            if (!lptr) {
                ctr_cinterp_instr sw = { .op = CI_OP_SWAP };
                cp_emit(p, sw);
            }
            if (esz > 1) {
                cp_emit_const_i64(p, (int64_t)esz);
                ctr_cinterp_instr m = { .op = CI_OP_MUL, .type = CTR_CINTERP_T_I64, .a = CTR_CINTERP_T_I64, .b = CTR_CINTERP_T_I64 };
                cp_emit(p, m);
            }
            if (sub) {
                ctr_cinterp_instr neg = { .op = CI_OP_NEG, .type = CTR_CINTERP_T_I64 };
                cp_emit(p, neg);
            }
            ctr_cinterp_instr add = { .op = CI_OP_PTR_ADD };
            cp_emit(p, add);
            cp_set_dtype(p, pt);
        } else {
            ctr_cinterp_instr ins;
            memset(&ins, 0, sizeof(ins));
            ins.op = sub ? CI_OP_SUB : CI_OP_ADD;
            ins.type = ci_promote_binary_type(lt.prim, rt.prim);
            ins.a = lt.prim;
            ins.b = rt.prim;
            cp_emit(p, ins);
            cp_set_scalar(p, ins.type);
        }
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
        cp_set_scalar(p, CTR_CINTERP_T_I64);
    }
}

static void cp_parse_rel(ci_cparser* p)
{
    cp_parse_shift(p);
    while (cp_is_punct(p, "<") || cp_is_punct(p, "<=") || cp_is_punct(p, ">") || cp_is_punct(p, ">=")) {
        ctr_cinterp_type lhs_type = p->last_type;
        char const* op = cp_peek(p)->text;
        p->pos++;
        cp_parse_shift(p);
        ctr_cinterp_instr ins;
        memset(&ins, 0, sizeof(ins));
        ins.op = strcmp(op, "<") == 0 ? CI_OP_LT : strcmp(op, "<=") == 0 ? CI_OP_LE
            : strcmp(op, ">") == 0                                       ? CI_OP_GT
                                                                         : CI_OP_GE;
        ins.type = ci_promote_binary_type(lhs_type, p->last_type);
        ins.a = lhs_type;
        ins.b = p->last_type;
        cp_emit(p, ins);
        cp_set_scalar(p, CTR_CINTERP_T_I64);
    }
}

static void cp_parse_eq(ci_cparser* p)
{
    cp_parse_rel(p);
    while (cp_is_punct(p, "==") || cp_is_punct(p, "!=")) {
        ctr_cinterp_type lhs_type = p->last_type;
        int eq = cp_is_punct(p, "==");
        p->pos++;
        cp_parse_rel(p);
        ctr_cinterp_instr ins = { .op = eq ? CI_OP_EQ : CI_OP_NE, .type = ci_promote_binary_type(lhs_type, p->last_type), .a = lhs_type, .b = p->last_type };
        cp_emit(p, ins);
        cp_set_scalar(p, CTR_CINTERP_T_I64);
    }
}

static void cp_parse_bitand(ci_cparser* p)
{
    cp_parse_eq(p);
    while (cp_match_punct(p, "&")) {
        cp_parse_eq(p);
        ctr_cinterp_instr ins = { .op = CI_OP_AND };
        cp_emit(p, ins);
        cp_set_scalar(p, CTR_CINTERP_T_I64);
    }
}

static void cp_parse_bitxor(ci_cparser* p)
{
    cp_parse_bitand(p);
    while (cp_match_punct(p, "^")) {
        cp_parse_bitand(p);
        ctr_cinterp_instr ins = { .op = CI_OP_XOR };
        cp_emit(p, ins);
        cp_set_scalar(p, CTR_CINTERP_T_I64);
    }
}

static void cp_parse_bitor(ci_cparser* p)
{
    cp_parse_bitxor(p);
    while (cp_match_punct(p, "|")) {
        cp_parse_bitxor(p);
        ctr_cinterp_instr ins = { .op = CI_OP_OR };
        cp_emit(p, ins);
        cp_set_scalar(p, CTR_CINTERP_T_I64);
    }
}

static void cp_parse_logand(ci_cparser* p)
{
    cp_parse_bitor(p);
    if (!cp_is_punct(p, "&&"))
        return;
    size_t false_jumps[64];
    size_t nfalse = 0;
    while (cp_match_punct(p, "&&")) {
        if (nfalse < 64)
            false_jumps[nfalse++] = cp_emit_jump(p, CI_OP_JZ);
        cp_parse_bitor(p);
    }
    if (nfalse < 64)
        false_jumps[nfalse++] = cp_emit_jump(p, CI_OP_JZ);
    cp_emit_const_i64(p, 1);
    size_t end = cp_emit_jump(p, CI_OP_JMP);
    for (size_t i = 0; i < nfalse; i++)
        cp_patch_jump(p, false_jumps[i], p->fn->code_count);
    cp_emit_const_i64(p, 0);
    cp_patch_jump(p, end, p->fn->code_count);
    cp_set_scalar(p, CTR_CINTERP_T_I64);
}

static void cp_parse_logor(ci_cparser* p)
{
    cp_parse_logand(p);
    if (!cp_is_punct(p, "||"))
        return;
    size_t true_jumps[64];
    size_t ntrue = 0;
    while (cp_match_punct(p, "||")) {
        if (ntrue < 64)
            true_jumps[ntrue++] = cp_emit_jump(p, CI_OP_JNZ);
        cp_parse_logand(p);
    }
    if (ntrue < 64)
        true_jumps[ntrue++] = cp_emit_jump(p, CI_OP_JNZ);
    cp_emit_const_i64(p, 0);
    size_t end = cp_emit_jump(p, CI_OP_JMP);
    for (size_t i = 0; i < ntrue; i++)
        cp_patch_jump(p, true_jumps[i], p->fn->code_count);
    cp_emit_const_i64(p, 1);
    cp_patch_jump(p, end, p->fn->code_count);
    cp_set_scalar(p, CTR_CINTERP_T_I64);
}

static void cp_parse_cond(ci_cparser* p)
{
    cp_parse_logor(p);
    if (!cp_match_punct(p, "?"))
        return;
    size_t else_jmp = cp_emit_jump(p, CI_OP_JZ);
    cp_parse_assign(p);
    ci_dtype tt = cp_get_dtype(p);
    size_t end_jmp = cp_emit_jump(p, CI_OP_JMP);
    cp_patch_jump(p, else_jmp, p->fn->code_count);
    cp_expect_punct(p, ":");
    cp_parse_cond(p);
    cp_patch_jump(p, end_jmp, p->fn->code_count);
    if (tt.prim == CTR_CINTERP_T_PTR || tt.prim == CTR_CINTERP_T_F64 || tt.prim == CTR_CINTERP_T_F32)
        cp_set_dtype(p, tt);
}

static int cp_assign_op(ci_cparser* p, ctr_cinterp_op* compound)
{
    static const struct {
        char const* tok;
        ctr_cinterp_op op;
    } ops[] = {
        { "+=", CI_OP_ADD }, { "-=", CI_OP_SUB }, { "*=", CI_OP_MUL },
        { "/=", CI_OP_DIV }, { "%=", CI_OP_MOD }, { "&=", CI_OP_AND },
        { "|=", CI_OP_OR }, { "^=", CI_OP_XOR }, { "<<=", CI_OP_SHL },
        { ">>=", CI_OP_SHR }
    };
    if (cp_is_punct(p, "=")) {
        *compound = CI_OP_NOP;
        return 1;
    }
    for (size_t i = 0; i < sizeof(ops) / sizeof(ops[0]); i++) {
        if (cp_is_punct(p, ops[i].tok)) {
            *compound = ops[i].op;
            return 1;
        }
    }
    return 0;
}

static void cp_parse_assign(ci_cparser* p)
{
    size_t spos = p->pos;
    size_t scode = p->fn->code_count;
    if (cp_parse_lvalue_addr(p)) {
        ctr_cinterp_op compound;
        if (cp_assign_op(p, &compound)) {
            ci_dtype lvt = cp_get_dtype(p);
            p->pos++;
            if (compound == CI_OP_NOP && ci_dt_is_struct(lvt)) {
                size_t sz = ci_storage_size(p->interp, lvt);
                cp_emit(p, (ctr_cinterp_instr) { .op = CI_OP_DUP });
                cp_parse_assign(p);
                cp_emit_const_i64(p, (int64_t)sz);
                cp_emit(p, (ctr_cinterp_instr) { .op = CI_OP_MEMCPY });
                cp_set_dtype(p, lvt);
                return;
            }
            if (compound == CI_OP_NOP) {
                cp_parse_assign(p);
                cp_coerce(p, lvt.prim);
                cp_emit_store(p, lvt.prim, 1);
            } else {
                cp_emit(p, (ctr_cinterp_instr) { .op = CI_OP_DUP });
                cp_emit_load(p, lvt);
                ci_dtype old = cp_get_dtype(p);
                cp_parse_assign(p);
                ctr_cinterp_instr ins;
                memset(&ins, 0, sizeof(ins));
                ins.op = compound;
                ins.type = old.prim;
                ins.a = old.prim;
                ins.b = p->last_type;
                cp_emit(p, ins);
                cp_set_scalar(p, old.prim);
                cp_coerce(p, lvt.prim);
                cp_emit_store(p, lvt.prim, 1);
            }
            cp_set_scalar(p, lvt.prim);
            return;
        }
        cp_truncate_code(p, scode);
        p->pos = spos;
    } else {
        cp_truncate_code(p, scode);
        p->pos = spos;
    }
    cp_parse_cond(p);
}

static void cp_parse_expr(ci_cparser* p)
{
    cp_parse_assign(p);
    while (cp_is_punct(p, ",")) {
        p->pos++;
        cp_emit_pop_if_value(p);
        cp_parse_assign(p);
    }
}

static char* cp_parse_declarator(ci_cparser* p, ci_dtype base, ci_dtype* out)
{
    ci_dtype dt = base;
    dt.tag = base.tag ? ci_strdup(base.tag) : NULL;
    cp_apply_stars(p, &dt);
    char* name = NULL;
    int parenthesized = 0;
    if (cp_is_punct(p, "(")) {
        p->pos++;
        ci_dtype inner;
        name = cp_parse_declarator(p, dt, &inner);
        cp_expect_punct(p, ")");
        free(dt.tag);
        dt = inner;
        parenthesized = 1;
    } else {
        name = cp_expect_ident(p);
    }
    if (!name)
        return NULL;
    if (parenthesized && cp_is_punct(p, "(")) {
        int depth = 0;
        do {
            if (cp_is_punct(p, "("))
                depth++;
            else if (cp_is_punct(p, ")"))
                depth--;
            p->pos++;
        } while (depth && cp_peek(p)->kind != CI_CTOK_EOF);
    }
    size_t total_len = 1;
    int saw_array = 0;
    int unsupported_array = 0;
    ctr_cinterp_type el_prim = dt.prim;
    ctr_cinterp_type el_pointee = dt.pointee;
    while (cp_match_punct(p, "[")) {
        size_t len = 0;
        if (cp_peek(p)->kind == CI_CTOK_NUM)
            len = (size_t)p->toks[p->pos++].num;
        cp_expect_punct(p, "]");
        if (saw_array) {
            cp_error(p, "multidimensional arrays are not supported");
            unsupported_array = 1;
        }
        total_len = saw_array ? total_len * (len ? len : 1) : len;
        saw_array = 1;
    }
    if (unsupported_array) {
        free(dt.tag);
        free(name);
        return NULL;
    }
    if (saw_array) {
        dt.is_array = 1;
        dt.array_len = total_len;
        dt.pointee = (el_prim == CTR_CINTERP_T_VOID && dt.tag) ? CTR_CINTERP_T_VOID : el_prim;
        if (el_prim == CTR_CINTERP_T_PTR)
            dt.pointee = el_pointee;
        dt.prim = CTR_CINTERP_T_PTR;
    }
    *out = dt;
    return name;
}

static void cp_emit_init(ci_cparser* p, ci_dtype dt)
{
    if (cp_is_punct(p, "{")) {
        p->pos++;
        if (dt.is_array) {
            ci_dtype el = ci_pointee_object(dt);
            size_t esz = ci_elem_size(p->interp, dt);
            size_t idx = 0;
            while (p->ok && !cp_is_punct(p, "}") && cp_peek(p)->kind != CI_CTOK_EOF) {
                if (cp_match_punct(p, "[")) {
                    idx = (size_t)cp_eval_const(p);
                    cp_expect_punct(p, "]");
                    cp_match_punct(p, "=");
                }
                cp_emit(p, (ctr_cinterp_instr) { .op = CI_OP_DUP });
                if (idx) {
                    cp_emit_const_i64(p, (int64_t)(idx * esz));
                    cp_emit(p, (ctr_cinterp_instr) { .op = CI_OP_PTR_ADD });
                }
                cp_emit_init(p, el);
                idx++;
                if (!cp_match_punct(p, ","))
                    break;
            }
            cp_expect_punct(p, "}");
            cp_emit(p, (ctr_cinterp_instr) { .op = CI_OP_POP });
        } else if (ci_dt_is_struct(dt)) {
            ci_struct_def* def = ci_find_struct(p->interp, dt.tag);
            size_t i = 0;
            while (p->ok && !cp_is_punct(p, "}") && cp_peek(p)->kind != CI_CTOK_EOF) {
                if (cp_match_punct(p, ".")) {
                    char* fn = cp_expect_ident(p);
                    cp_match_punct(p, "=");
                    for (size_t k = 0; def && fn && k < def->member_count; k++)
                        if (strcmp(def->members[k].name, fn) == 0) {
                            i = k;
                            break;
                        }
                    free(fn);
                }
                if (!def || i >= def->member_count) {
                    cp_error(p, "too many struct initializers");
                    break;
                }
                cp_emit(p, (ctr_cinterp_instr) { .op = CI_OP_DUP });
                if (def->members[i].offset) {
                    cp_emit_const_i64(p, (int64_t)def->members[i].offset);
                    cp_emit(p, (ctr_cinterp_instr) { .op = CI_OP_PTR_ADD });
                }
                cp_emit_init(p, def->members[i].type);
                i++;
                if (!cp_match_punct(p, ","))
                    break;
            }
            cp_expect_punct(p, "}");
            cp_emit(p, (ctr_cinterp_instr) { .op = CI_OP_POP });
        } else {
            cp_error(p, "brace initializer for scalar");
        }
        return;
    }
    if (dt.is_array && (dt.pointee == CTR_CINTERP_T_I8 || dt.pointee == CTR_CINTERP_T_U8)
        && cp_peek(p)->kind == CI_CTOK_STR) {
        char const* s = p->toks[p->pos++].text;
        size_t len = strlen(s) + 1;
        if (dt.array_len && len > dt.array_len)
            len = dt.array_len;
        cp_add_string_global(p, s);
        cp_emit_const_i64(p, (int64_t)len);
        cp_emit(p, (ctr_cinterp_instr) { .op = CI_OP_MEMCPY });
        return;
    }
    cp_parse_assign(p);
    cp_coerce(p, dt.prim);
    cp_emit_store(p, dt.prim, 0);
}

static void cp_parse_decl_stmt(ci_cparser* p)
{
    ci_dtype base;
    int storage = 0;
    if (!cp_parse_dtype_ex(p, &base, &storage, 0))
        return;
    if (cp_is_punct(p, ";")) {
        p->pos++;
        free(base.tag);
        return;
    }
    if (storage == 2) {
        do {
            ci_dtype dt;
            char* name = cp_parse_declarator(p, base, &dt);
            if (!name)
                break;
            ci_add_typedef(p->interp, name, dt);
            free(name);
            free(dt.tag);
        } while (cp_match_punct(p, ","));
        cp_expect_punct(p, ";");
        free(base.tag);
        return;
    }
    do {
        ci_dtype dt;
        char* name = cp_parse_declarator(p, base, &dt);
        if (!name)
            break;
        ci_cvar* var = cp_add_var(p, name, dt);
        free(name);
        if (cp_match_punct(p, "=") && var) {
            cp_emit_var_addr(p, var);
            cp_emit_init(p, var->type);
        }
        free(dt.tag);
    } while (cp_match_punct(p, ","));
    cp_expect_punct(p, ";");
    free(base.tag);
}

static void cp_jlist_add(ci_jlist* l, size_t at)
{
    if (ci_grow((void**)&l->items, &l->cap, l->count, sizeof(size_t)))
        l->items[l->count++] = at;
}

static void cp_jlist_patch(ci_cparser* p, ci_jlist* l, size_t target)
{
    for (size_t i = 0; i < l->count; i++)
        cp_patch_jump(p, l->items[i], target);
}

static int64_t cp_eval_const(ci_cparser* p);

static int64_t cp_eval_const_primary(ci_cparser* p)
{
    if (cp_match_punct(p, "(")) {
        int64_t v = cp_eval_const(p);
        cp_expect_punct(p, ")");
        return v;
    }
    if (cp_match_punct(p, "-"))
        return -cp_eval_const_primary(p);
    if (cp_match_punct(p, "+"))
        return cp_eval_const_primary(p);
    if (cp_match_punct(p, "~"))
        return ~cp_eval_const_primary(p);
    if (cp_match_punct(p, "!"))
        return !cp_eval_const_primary(p);
    if (cp_peek(p)->kind == CI_CTOK_NUM)
        return p->toks[p->pos++].num;
    if (cp_peek(p)->kind == CI_CTOK_IDENT) {
        int64_t v = 0;
        ci_find_enum_const(p->interp, cp_peek(p)->text, &v);
        p->pos++;
        return v;
    }
    cp_error(p, "expected integer constant");
    return 0;
}

static int64_t cp_eval_const(ci_cparser* p)
{
    int64_t v = cp_eval_const_primary(p);
    for (;;) {
        if (cp_match_punct(p, "+"))
            v += cp_eval_const_primary(p);
        else if (cp_match_punct(p, "-"))
            v -= cp_eval_const_primary(p);
        else if (cp_match_punct(p, "*"))
            v *= cp_eval_const_primary(p);
        else if (cp_match_punct(p, "/")) {
            int64_t d = cp_eval_const_primary(p);
            v = d ? v / d : 0;
        } else if (cp_match_punct(p, "<<"))
            v <<= cp_eval_const_primary(p);
        else if (cp_match_punct(p, ">>"))
            v >>= cp_eval_const_primary(p);
        else if (cp_match_punct(p, "|"))
            v |= cp_eval_const_primary(p);
        else
            break;
    }
    return v;
}

static void cp_parse_block(ci_cparser* p)
{
    size_t sstruct = p->interp->struct_count;
    size_t stypedef = p->interp->typedef_count;
    size_t senum = p->interp->enum_const_count;
    size_t svar = p->var_count;
    cp_expect_punct(p, "{");
    while (p->ok && !cp_is_punct(p, "}") && cp_peek(p)->kind != CI_CTOK_EOF)
        cp_parse_stmt(p);
    cp_expect_punct(p, "}");
    for (size_t i = svar; i < p->var_count; i++) {
        free(p->vars[i].name);
        free(p->vars[i].global_name);
        free(p->vars[i].type.tag);
    }
    p->var_count = svar;
    for (size_t i = sstruct; i < p->interp->struct_count; i++) {
        free(p->interp->structs[i].name);
        for (size_t j = 0; j < p->interp->structs[i].member_count; j++) {
            free(p->interp->structs[i].members[j].name);
            free(p->interp->structs[i].members[j].type.tag);
        }
        free(p->interp->structs[i].members);
    }
    p->interp->struct_count = sstruct;
    for (size_t i = stypedef; i < p->interp->typedef_count; i++) {
        free(p->interp->typedefs[i].name);
        free(p->interp->typedefs[i].type.tag);
    }
    p->interp->typedef_count = stypedef;
    for (size_t i = senum; i < p->interp->enum_const_count; i++)
        free(p->interp->enum_consts[i].name);
    p->interp->enum_const_count = senum;
}

static void cp_parse_stmt(ci_cparser* p)
{
    if (!p->ok)
        return;
    if (cp_peek(p)->kind == CI_CTOK_IDENT && p->toks[p->pos + 1].kind == CI_CTOK_PUNCT
        && strcmp(p->toks[p->pos + 1].text, ":") == 0) {
        if (ci_grow((void**)&p->labels, &p->label_cap, p->label_count, sizeof(*p->labels))) {
            p->labels[p->label_count].name = ci_strdup(cp_peek(p)->text);
            p->labels[p->label_count].pos = p->fn->code_count;
            p->label_count++;
        }
        p->pos += 2;
        if (!cp_is_punct(p, "}"))
            cp_parse_stmt(p);
        return;
    }
    if (cp_match_kind(p, CI_CTOK_CASE)) {
        int64_t v = cp_eval_const(p);
        cp_expect_punct(p, ":");
        if (p->sw_ctx && p->sw_ctx->ncase < 256) {
            p->sw_ctx->cases[p->sw_ctx->ncase].v = v;
            p->sw_ctx->cases[p->sw_ctx->ncase].pos = p->fn->code_count;
            p->sw_ctx->ncase++;
        }
        if (!cp_is_punct(p, "}"))
            cp_parse_stmt(p);
        return;
    }
    if (cp_match_kind(p, CI_CTOK_DEFAULT)) {
        cp_expect_punct(p, ":");
        if (p->sw_ctx) {
            p->sw_ctx->default_pos = p->fn->code_count;
            p->sw_ctx->has_default = 1;
        }
        if (!cp_is_punct(p, "}"))
            cp_parse_stmt(p);
        return;
    }
    if (cp_is_punct(p, "{")) {
        cp_parse_block(p);
        return;
    }
    if (cp_match_kind(p, CI_CTOK_RETURN)) {
        if (!cp_is_punct(p, ";")) {
            cp_parse_expr(p);
            cp_coerce(p, p->fn->ret_type);
        } else {
            cp_emit_const_i64(p, 0);
        }
        ctr_cinterp_instr ins = { .op = CI_OP_RET };
        cp_emit(p, ins);
        cp_expect_punct(p, ";");
        return;
    }
    if (cp_match_kind(p, CI_CTOK_BREAK)) {
        if (p->brk_ctx)
            cp_jlist_add(p->brk_ctx, cp_emit_jump(p, CI_OP_JMP));
        else
            cp_error(p, "`break' outside loop or switch");
        cp_expect_punct(p, ";");
        return;
    }
    if (cp_match_kind(p, CI_CTOK_CONTINUE)) {
        if (p->cont_ctx)
            cp_jlist_add(p->cont_ctx, cp_emit_jump(p, CI_OP_JMP));
        else
            cp_error(p, "`continue' outside loop");
        cp_expect_punct(p, ";");
        return;
    }
    if (cp_match_kind(p, CI_CTOK_GOTO)) {
        char* name = cp_expect_ident(p);
        size_t at = cp_emit_jump(p, CI_OP_JMP);
        if (name && ci_grow((void**)&p->gotos, &p->goto_cap, p->goto_count, sizeof(*p->gotos))) {
            p->gotos[p->goto_count].name = name;
            p->gotos[p->goto_count].at = at;
            p->gotos[p->goto_count].line = cp_peek(p)->line;
            p->goto_count++;
        } else {
            free(name);
        }
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
        ci_jlist brk = { 0 }, cont = { 0 };
        ci_jlist* sb = p->brk_ctx;
        ci_jlist* sc = p->cont_ctx;
        p->brk_ctx = &brk;
        p->cont_ctx = &cont;
        size_t start = p->fn->code_count;
        cp_expect_punct(p, "(");
        cp_parse_expr(p);
        cp_expect_punct(p, ")");
        size_t jz = cp_emit_jump(p, CI_OP_JZ);
        cp_parse_stmt(p);
        ctr_cinterp_instr back = { .op = CI_OP_JMP, .a = (int)start };
        cp_emit(p, back);
        cp_patch_jump(p, jz, p->fn->code_count);
        cp_jlist_patch(p, &cont, start);
        cp_jlist_patch(p, &brk, p->fn->code_count);
        free(brk.items);
        free(cont.items);
        p->brk_ctx = sb;
        p->cont_ctx = sc;
        return;
    }
    if (cp_match_kind(p, CI_CTOK_DO)) {
        ci_jlist brk = { 0 }, cont = { 0 };
        ci_jlist* sb = p->brk_ctx;
        ci_jlist* sc = p->cont_ctx;
        p->brk_ctx = &brk;
        p->cont_ctx = &cont;
        size_t start = p->fn->code_count;
        cp_parse_stmt(p);
        cp_jlist_patch(p, &cont, p->fn->code_count);
        if (!cp_match_kind(p, CI_CTOK_WHILE))
            cp_error(p, "expected `while' after do-body");
        cp_expect_punct(p, "(");
        cp_parse_expr(p);
        cp_expect_punct(p, ")");
        cp_expect_punct(p, ";");
        ctr_cinterp_instr jnz = { .op = CI_OP_JNZ, .a = (int)start };
        cp_emit(p, jnz);
        cp_jlist_patch(p, &brk, p->fn->code_count);
        free(brk.items);
        free(cont.items);
        p->brk_ctx = sb;
        p->cont_ctx = sc;
        return;
    }
    if (cp_match_kind(p, CI_CTOK_FOR)) {
        ci_jlist brk = { 0 }, cont = { 0 };
        ci_jlist* sb = p->brk_ctx;
        ci_jlist* sc = p->cont_ctx;
        cp_expect_punct(p, "(");
        if (!cp_is_punct(p, ";")) {
            if (cp_is_type_start(p)) {
                cp_parse_decl_stmt(p);
            } else {
                cp_parse_expr(p);
                cp_emit_pop_if_value(p);
                cp_expect_punct(p, ";");
            }
        } else {
            cp_expect_punct(p, ";");
        }
        size_t start = p->fn->code_count;
        if (!cp_is_punct(p, ";"))
            cp_parse_expr(p);
        else
            cp_emit_const_i64(p, 1);
        cp_expect_punct(p, ";");
        size_t jz = cp_emit_jump(p, CI_OP_JZ);
        size_t iter_start = p->pos;
        int depth = 0;
        while ((depth || !cp_is_punct(p, ")")) && cp_peek(p)->kind != CI_CTOK_EOF) {
            if (cp_is_punct(p, "("))
                depth++;
            else if (cp_is_punct(p, ")"))
                depth--;
            p->pos++;
        }
        size_t iter_end = p->pos;
        cp_expect_punct(p, ")");
        p->brk_ctx = &brk;
        p->cont_ctx = &cont;
        cp_parse_stmt(p);
        cp_jlist_patch(p, &cont, p->fn->code_count);
        if (iter_end > iter_start) {
            size_t save = p->pos;
            p->pos = iter_start;
            cp_parse_expr(p);
            cp_emit_pop_if_value(p);
            p->pos = save;
        }
        ctr_cinterp_instr back = { .op = CI_OP_JMP, .a = (int)start };
        cp_emit(p, back);
        cp_patch_jump(p, jz, p->fn->code_count);
        cp_jlist_patch(p, &brk, p->fn->code_count);
        free(brk.items);
        free(cont.items);
        p->brk_ctx = sb;
        p->cont_ctx = sc;
        return;
    }
    if (cp_match_kind(p, CI_CTOK_SWITCH)) {
        cp_expect_punct(p, "(");
        cp_parse_expr(p);
        cp_expect_punct(p, ")");
        size_t tmp_off = ci_align_up(p->fn->frame_size, 8);
        p->fn->frame_size = tmp_off + 8;
        cp_emit(p, (ctr_cinterp_instr) { .op = CI_OP_FRAME_ADDR, .a = (int)tmp_off });
        cp_emit(p, (ctr_cinterp_instr) { .op = CI_OP_SWAP });
        cp_emit_store(p, CTR_CINTERP_T_I64, 0);
        size_t dispatch_jmp = cp_emit_jump(p, CI_OP_JMP);
        ci_jlist brk = { 0 };
        ci_swctx sw = { 0 };
        ci_jlist* sb = p->brk_ctx;
        ci_swctx* ss = p->sw_ctx;
        p->brk_ctx = &brk;
        p->sw_ctx = &sw;
        cp_parse_stmt(p);
        p->sw_ctx = ss;
        size_t body_end = cp_emit_jump(p, CI_OP_JMP);
        cp_patch_jump(p, dispatch_jmp, p->fn->code_count);
        for (size_t i = 0; i < sw.ncase; i++) {
            cp_emit(p, (ctr_cinterp_instr) { .op = CI_OP_FRAME_ADDR, .a = (int)tmp_off });
            cp_emit(p, (ctr_cinterp_instr) { .op = CI_OP_LOAD, .type = CTR_CINTERP_T_I64 });
            cp_emit_const_i64(p, sw.cases[i].v);
            cp_emit(p, (ctr_cinterp_instr) { .op = CI_OP_EQ, .type = CTR_CINTERP_T_I64, .a = CTR_CINTERP_T_I64, .b = CTR_CINTERP_T_I64 });
            cp_emit(p, (ctr_cinterp_instr) { .op = CI_OP_JNZ, .a = (int)sw.cases[i].pos });
        }
        if (sw.has_default)
            cp_emit(p, (ctr_cinterp_instr) { .op = CI_OP_JMP, .a = (int)sw.default_pos });
        cp_patch_jump(p, body_end, p->fn->code_count);
        cp_jlist_patch(p, &brk, p->fn->code_count);
        free(brk.items);
        p->brk_ctx = sb;
        return;
    }
    if (cp_is_type_start(p)) {
        cp_parse_decl_stmt(p);
        return;
    }
    if (!cp_is_punct(p, ";")) {
        cp_parse_expr(p);
        cp_emit_pop_if_value(p);
    }
    cp_expect_punct(p, ";");
}

static void cp_define_struct_body(ci_cparser* p, char const* name, int is_union)
{
    char format[4096] = "";
    size_t format_len = 0;
    char** names = NULL;
    size_t name_count = 0;
    size_t name_cap = 0;
    ci_struct_member* mem = NULL;
    size_t nmem = 0;
    size_t mem_cap = 0;
    size_t offset = 0;
    size_t max_align = 1;
    cp_expect_punct(p, "{");
    while (p->ok && !cp_is_punct(p, "}") && cp_peek(p)->kind != CI_CTOK_EOF) {
        ci_dtype base;
        int ignored_storage = 0;
        if (!cp_parse_dtype_ex(p, &base, &ignored_storage, 0))
            break;
        if (cp_is_punct(p, ";") && ci_dt_is_struct(base)) {
            ci_struct_def* inner = ci_find_struct(p->interp, base.tag);
            size_t al = inner && inner->align ? inner->align : 1;
            size_t off = is_union ? 0 : ci_align_up(offset, al);
            size_t sz = inner ? inner->size : 0;
            for (size_t i = 0; inner && i < inner->member_count; i++) {
                if (!ci_grow((void**)&mem, &mem_cap, nmem, sizeof(ci_struct_member)))
                    break;
                ci_struct_member* m = &mem[nmem++];
                memset(m, 0, sizeof(*m));
                m->name = ci_strdup(inner->members[i].name);
                m->type = inner->members[i].type;
                m->type.tag = inner->members[i].type.tag ? ci_strdup(inner->members[i].type.tag) : NULL;
                m->offset = off + inner->members[i].offset;
                m->size = inner->members[i].size;
            }
            if (is_union) {
                if (sz > offset)
                    offset = sz;
            } else
                offset = off + sz;
            if (al > max_align)
                max_align = al;
            cp_expect_punct(p, ";");
            free(base.tag);
            continue;
        }
        do {
            ci_dtype mt;
            char* field = cp_parse_declarator(p, base, &mt);
            if (!field)
                break;
            size_t sz = ci_storage_size(p->interp, mt);
            size_t al = ci_storage_align(p->interp, mt);
            if (al < 1)
                al = 1;
            size_t off = is_union ? 0 : ci_align_up(offset, al);
            if (ci_grow((void**)&mem, &mem_cap, nmem, sizeof(ci_struct_member))) {
                ci_struct_member* m = &mem[nmem++];
                memset(m, 0, sizeof(*m));
                m->name = ci_strdup(field);
                m->type = mt;
                m->type.tag = mt.tag ? ci_strdup(mt.tag) : NULL;
                m->offset = off;
                m->size = sz;
            }
            if (is_union) {
                if (sz > offset)
                    offset = sz;
            } else {
                offset = off + sz;
            }
            if (al > max_align)
                max_align = al;
            char tfmt[16];
            ctr_cinterp_type ftype = mt.is_array ? CTR_CINTERP_T_PTR : mt.prim;
            if (ftype == CTR_CINTERP_T_VOID && mt.tag)
                ftype = CTR_CINTERP_T_PTR;
            if (ci_type_format(ftype, tfmt, sizeof(tfmt))) {
                size_t tl = strlen(tfmt);
                if (format_len + tl < sizeof(format)) {
                    memcpy(format + format_len, tfmt, tl + 1);
                    format_len += tl;
                }
            }
            if (ci_grow((void**)&names, &name_cap, name_count, sizeof(char*)))
                names[name_count++] = ci_strdup(field);
            free(mt.tag);
            free(field);
        } while (cp_match_punct(p, ","));
        cp_expect_punct(p, ";");
        free(base.tag);
    }
    cp_expect_punct(p, "}");
    ci_struct_def* def = ci_add_struct(p->interp, name);
    if (def) {
        def->members = mem;
        def->member_count = nmem;
        def->align = max_align;
        def->size = ci_align_up(offset ? offset : 1, max_align);
    } else {
        for (size_t i = 0; i < nmem; i++) {
            free(mem[i].name);
            free(mem[i].type.tag);
        }
        free(mem);
    }
    if (p->ok)
        ci_add_ctype(p->interp, name, format, names, name_count);
    for (size_t i = 0; i < name_count; i++)
        free(names[i]);
    free(names);
}

static void cp_parse_enum_body(ci_cparser* p)
{
    cp_expect_punct(p, "{");
    int64_t next = 0;
    while (p->ok && !cp_is_punct(p, "}") && cp_peek(p)->kind != CI_CTOK_EOF) {
        char* cname = cp_expect_ident(p);
        if (!cname)
            break;
        if (cp_match_punct(p, "="))
            next = cp_eval_const(p);
        ci_add_enum_const(p->interp, cname, next);
        free(cname);
        next++;
        if (!cp_match_punct(p, ","))
            break;
    }
    cp_expect_punct(p, "}");
}

static void cp_write_global_init(ci_cparser* p, void* data, ci_dtype dt);

static ctr_cinterp_global* cp_new_anon_global(ci_cparser* p, ci_dtype dt)
{
    if (!ci_grow((void**)&p->interp->globals, &p->interp->global_cap, p->interp->global_count, sizeof(ctr_cinterp_global)))
        return NULL;
    ctr_cinterp_global* g = &p->interp->globals[p->interp->global_count++];
    memset(g, 0, sizeof(*g));
    char nm[64];
    snprintf(nm, sizeof(nm), ".cl%d", p->string_id++);
    g->name = ci_strdup(nm);
    g->size = ci_storage_size(p->interp, dt);
    if (!g->size)
        g->size = 1;
    g->data = calloc(1, g->size);
    g->dtype = dt;
    g->dtype.tag = dt.tag ? ci_strdup(dt.tag) : NULL;
    return g;
}

static void cp_write_global_init(ci_cparser* p, void* data, ci_dtype dt)
{
    if (cp_is_punct(p, "(") && cp_is_type_start(&(ci_cparser) { .toks = p->toks, .pos = p->pos + 1, .count = p->count, .interp = p->interp, .ok = p->ok })) {
        p->pos++;
        ci_dtype lt;
        int ig = 0;
        if (cp_parse_dtype(p, &lt, &ig) && cp_match_punct(p, ")") && cp_is_punct(p, "{")) {
            cp_write_global_init(p, data, lt);
            free(lt.tag);
            return;
        }
        free(lt.tag);
    }
    if (cp_is_punct(p, "{")) {
        p->pos++;
        if (dt.is_array) {
            size_t esz = ci_elem_size(p->interp, dt);
            ci_dtype el = ci_pointee_object(dt);
            size_t idx = 0;
            while (p->ok && !cp_is_punct(p, "}") && cp_peek(p)->kind != CI_CTOK_EOF) {
                if (cp_match_punct(p, "[")) {
                    idx = (size_t)cp_eval_const(p);
                    cp_expect_punct(p, "]");
                    cp_match_punct(p, "=");
                }
                cp_write_global_init(p, (char*)data + idx * esz, el);
                idx++;
                if (!cp_match_punct(p, ","))
                    break;
            }
            cp_expect_punct(p, "}");
        } else if (ci_dt_is_struct(dt)) {
            ci_struct_def* def = ci_find_struct(p->interp, dt.tag);
            size_t i = 0;
            while (p->ok && !cp_is_punct(p, "}") && cp_peek(p)->kind != CI_CTOK_EOF) {
                if (cp_match_punct(p, ".")) {
                    char* fn = cp_expect_ident(p);
                    cp_match_punct(p, "=");
                    for (size_t k = 0; def && fn && k < def->member_count; k++)
                        if (strcmp(def->members[k].name, fn) == 0) {
                            i = k;
                            break;
                        }
                    free(fn);
                }
                if (!def || i >= def->member_count)
                    break;
                cp_write_global_init(p, (char*)data + def->members[i].offset, def->members[i].type);
                i++;
                if (!cp_match_punct(p, ","))
                    break;
            }
            cp_expect_punct(p, "}");
        } else {
            cp_expect_punct(p, "}");
        }
        return;
    }
    if (dt.is_array && (dt.pointee == CTR_CINTERP_T_I8 || dt.pointee == CTR_CINTERP_T_U8)
        && cp_peek(p)->kind == CI_CTOK_STR) {
        char const* s = p->toks[p->pos++].text;
        size_t len = strlen(s) + 1;
        if (dt.array_len && len > dt.array_len)
            len = dt.array_len;
        memcpy(data, s, len);
        return;
    }
    if (dt.prim == CTR_CINTERP_T_PTR && cp_is_punct(p, "&")) {
        p->pos++;
        if (cp_is_punct(p, "(") && cp_is_type_start(&(ci_cparser) { .toks = p->toks, .pos = p->pos + 1, .count = p->count, .interp = p->interp, .ok = p->ok })) {
            p->pos++;
            ci_dtype lt;
            int ig = 0;
            if (cp_parse_dtype(p, &lt, &ig) && cp_match_punct(p, ")")) {
                ctr_cinterp_global* g = cp_new_anon_global(p, lt);
                if (g) {
                    void* addr = g->data; /* capture before recursion may grow globals */
                    cp_write_global_init(p, addr, lt);
                    memcpy(data, &addr, sizeof(void*));
                }
            }
            free(lt.tag);
            return;
        }
        char* id = cp_expect_ident(p);
        ctr_cinterp_global* g = id ? ci_find_global(p->interp, id) : NULL;
        if (g) {
            void* addr = g->data;
            memcpy(data, &addr, sizeof(void*));
        }
        free(id);
        return;
    }
    if (dt.prim == CTR_CINTERP_T_PTR && cp_peek(p)->kind == CI_CTOK_STR) {
        char const* s = p->toks[p->pos++].text;
        size_t n = strlen(s) + 1;
        char* buf = malloc(n);
        if (buf) {
            memcpy(buf, s, n);
            memcpy(data, &buf, sizeof(char*)); /* leaked intentionally for program lifetime */
        }
        return;
    }
    if (ci_type_is_float(dt.prim)) {
        double v = 0;
        if (cp_peek(p)->kind == CI_CTOK_FNUM)
            v = p->toks[p->pos++].fnum;
        else if (cp_peek(p)->kind == CI_CTOK_NUM)
            v = (double)p->toks[p->pos++].num;
        else
            v = (double)cp_eval_const(p);
        if (dt.prim == CTR_CINTERP_T_F32) {
            float f = (float)v;
            memcpy(data, &f, sizeof(f));
        } else {
            memcpy(data, &v, sizeof(v));
        }
        return;
    }
    int64_t value = cp_eval_const(p);
    size_t sz = ci_prim_size(dt.prim);
    memcpy(data, &value, sz < sizeof(value) ? sz : sizeof(value));
}

static ctr_cinterp_global* cp_define_global(ci_cparser* p, char const* name, ci_dtype dt)
{
    ctr_cinterp_global* g = ci_find_global(p->interp, name);
    if (!g) {
        if (!ci_grow((void**)&p->interp->globals, &p->interp->global_cap, p->interp->global_count, sizeof(ctr_cinterp_global)))
            return NULL;
        g = &p->interp->globals[p->interp->global_count++];
        memset(g, 0, sizeof(*g));
        g->name = ci_strdup(name);
        g->size = ci_storage_size(p->interp, dt);
        if (!g->size)
            g->size = 1;
        g->data = calloc(1, g->size);
    }
    free(g->dtype.tag);
    g->dtype = dt;
    g->dtype.tag = dt.tag ? ci_strdup(dt.tag) : NULL;
    return g;
}

static int cp_parse_toplevel(ci_cparser* p)
{
    int is_typedef = 0;
    if (cp_peek(p)->kind == CI_CTOK_TYPEDEF && p->toks[p->pos + 1].kind == CI_CTOK_ENUM) {
        is_typedef = 1;
        p->pos++;
    }
    if (cp_peek(p)->kind == CI_CTOK_ENUM
        && (p->toks[p->pos + 1].kind == CI_CTOK_PUNCT
            || (p->toks[p->pos + 1].kind == CI_CTOK_IDENT && p->toks[p->pos + 2].kind == CI_CTOK_PUNCT))) {
        p->pos++;
        if (cp_peek(p)->kind == CI_CTOK_IDENT)
            p->pos++;
        if (cp_is_punct(p, "{"))
            cp_parse_enum_body(p);
        ci_dtype edt;
        memset(&edt, 0, sizeof(edt));
        edt.prim = CTR_CINTERP_T_I32;
        if (cp_peek(p)->kind == CI_CTOK_IDENT) {
            char* tn = cp_expect_ident(p);
            if (is_typedef)
                ci_add_typedef(p->interp, tn, edt);
            else
                cp_define_global(p, tn, edt);
            free(tn);
        }
        cp_expect_punct(p, ";");
        return p->ok;
    }

    int storage = 0;
    ci_dtype base;
    if (!cp_parse_dtype_ex(p, &base, &storage, 0))
        return 0;
    is_typedef = is_typedef || storage == 2;

    if (cp_is_punct(p, ";")) {
        p->pos++;
        free(base.tag);
        return p->ok;
    }

    ci_dtype dt;
    char* name = cp_parse_declarator(p, base, &dt);
    if (!name) {
        free(base.tag);
        return 0;
    }

    if (is_typedef) {
        ci_add_typedef(p->interp, name, dt);
        while (cp_match_punct(p, ",")) {
            ci_dtype dt2;
            char* n2 = cp_parse_declarator(p, base, &dt2);
            if (!n2)
                break;
            ci_add_typedef(p->interp, n2, dt2);
            free(dt2.tag);
            free(n2);
        }
        cp_expect_punct(p, ";");
        free(name);
        free(dt.tag);
        free(base.tag);
        return p->ok;
    }

    if (!cp_is_punct(p, "(")) {
        for (;;) {
            ctr_cinterp_global* g = cp_define_global(p, name, dt);
            if (g && cp_match_punct(p, "="))
                cp_write_global_init(p, g->data, dt);
            free(name);
            free(dt.tag);
            if (!cp_match_punct(p, ","))
                break;
            name = cp_parse_declarator(p, base, &dt);
            if (!name)
                break;
        }
        cp_expect_punct(p, ";");
        free(base.tag);
        return p->ok;
    }
    free(base.tag);

    p->pos++;
    ctr_cinterp_type ret_type = (dt.prim == CTR_CINTERP_T_VOID && dt.tag) ? CTR_CINTERP_T_PTR : dt.prim;
    free(dt.tag);
    ci_dtype arg_dtypes[128];
    char* arg_names[128];
    ctr_cinterp_type arg_prims[128];
    size_t argc = 0;
    int variadic = 0;
    memset(arg_names, 0, sizeof(arg_names));
    if (!cp_is_punct(p, ")")) {
        do {
            if (cp_match_punct(p, "...")) {
                variadic = 1;
                break;
            }
            if (cp_peek(p)->kind == CI_CTOK_VOID && p->toks[p->pos + 1].kind == CI_CTOK_PUNCT && strcmp(p->toks[p->pos + 1].text, ")") == 0) {
                p->pos++;
                break;
            }
            if (argc >= 128) {
                cp_error(p, "too many function arguments");
                break;
            }
            ci_dtype abase;
            int ig = 0;
            if (!cp_parse_dtype_ex(p, &abase, &ig, 0))
                break;
            ci_dtype adt;
            char* aname = NULL;
            if (cp_peek(p)->kind == CI_CTOK_IDENT
                || cp_is_punct(p, "*") || cp_is_punct(p, "[")) {
                aname = cp_parse_declarator(p, abase, &adt);
            } else {
                adt = abase;
                adt.tag = abase.tag ? ci_strdup(abase.tag) : NULL;
            }
            free(abase.tag);
            arg_dtypes[argc] = adt;
            arg_prims[argc] = (adt.prim == CTR_CINTERP_T_VOID && adt.tag) ? CTR_CINTERP_T_PTR : adt.prim;
            arg_names[argc] = aname ? aname : ci_strdup("");
            argc++;
        } while (cp_match_punct(p, ","));
    }
    cp_expect_punct(p, ")");

    if (cp_match_punct(p, ";")) {
        ci_add_external_ex(p->interp, name, ret_type, arg_prims, argc, variadic);
        for (size_t i = 0; i < argc; i++) {
            free(arg_names[i]);
            free(arg_dtypes[i].tag);
        }
        free(name);
        return p->ok;
    }
    if (variadic) {
        cp_error(p, "interpreted C function definitions cannot be variadic");
        for (size_t i = 0; i < argc; i++) {
            free(arg_names[i]);
            free(arg_dtypes[i].tag);
        }
        free(name);
        return 0;
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
    p->fn->arg_offset = calloc(argc ? argc : 1, sizeof(size_t));
    p->fn->arg_slot_type = calloc(argc ? argc : 1, sizeof(ctr_cinterp_type));
    p->fn->arg_copy_size = calloc(argc ? argc : 1, sizeof(size_t));
    p->label_count = 0;
    p->goto_count = 0;
    for (size_t i = 0; i < argc; i++) {
        char tmpname[32];
        char const* vn = arg_names[i] && *arg_names[i] ? arg_names[i] : (snprintf(tmpname, sizeof(tmpname), ".arg%zu", i), tmpname);
        ci_cvar* v = cp_add_var(p, vn, arg_dtypes[i]);
        if (v && p->fn->arg_offset) {
            p->fn->arg_offset[i] = v->offset;
            if (ci_dt_is_struct(arg_dtypes[i])) {
                p->fn->arg_slot_type[i] = CTR_CINTERP_T_VOID;
                p->fn->arg_copy_size[i] = ci_storage_size(p->interp, arg_dtypes[i]);
            } else {
                p->fn->arg_slot_type[i] = arg_prims[i];
            }
        }
        free(arg_names[i]);
        free(arg_dtypes[i].tag);
    }
    cp_parse_block(p);
    if (p->ok && (p->fn->code_count == 0 || p->fn->code[p->fn->code_count - 1].op != CI_OP_RET)) {
        cp_emit_const_i64(p, 0);
        ctr_cinterp_instr ret = { .op = CI_OP_RET };
        cp_emit(p, ret);
    }
    for (size_t i = 0; i < p->goto_count; i++) {
        size_t target = 0;
        int found = 0;
        for (size_t j = 0; j < p->label_count; j++) {
            if (strcmp(p->labels[j].name, p->gotos[i].name) == 0) {
                target = p->labels[j].pos;
                found = 1;
                break;
            }
        }
        if (found)
            cp_patch_jump(p, p->gotos[i].at, target);
        else
            cp_error(p, "goto to undefined label `%s'", p->gotos[i].name);
        free(p->gotos[i].name);
    }
    for (size_t j = 0; j < p->label_count; j++)
        free(p->labels[j].name);
    p->goto_count = 0;
    p->label_count = 0;
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
    for (size_t i = 0; i < p.goto_count; i++)
        free(p.gotos[i].name);
    for (size_t j = 0; j < p.label_count; j++)
        free(p.labels[j].name);
    free(p.gotos);
    free(p.labels);
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

static ctr_cinterp_value ci_cast_value(ctr_cinterp_value v, ctr_cinterp_type target, ctr_cinterp_type source)
{
    int src_float = ci_type_is_float(source);
    int dst_float = ci_type_is_float(target);
    ctr_cinterp_value out;
    memset(&out, 0, sizeof(out));
    if (dst_float) {
        out.f = src_float ? v.f : (double)v.i;
        if (target == CTR_CINTERP_T_F32)
            out.f = (float)out.f;
        return out;
    }
    int64_t iv = src_float ? (int64_t)v.f : v.i;
    switch (target) {
    case CTR_CINTERP_T_I8:
        out.i = (int8_t)iv;
        break;
    case CTR_CINTERP_T_U8:
        out.u = (uint8_t)iv;
        break;
    case CTR_CINTERP_T_I16:
        out.i = (int16_t)iv;
        break;
    case CTR_CINTERP_T_U16:
        out.u = (uint16_t)iv;
        break;
    case CTR_CINTERP_T_I32:
        out.i = (int32_t)iv;
        break;
    case CTR_CINTERP_T_U32:
        out.u = (uint32_t)iv;
        break;
    case CTR_CINTERP_T_PTR:
        out.p = (void*)(uintptr_t)iv;
        break;
    default:
        out.i = iv;
        break;
    }
    return out;
}

static void ci_store_value(void* ptr, ctr_cinterp_type type, ctr_cinterp_value a)
{
    switch (type) {
    case CTR_CINTERP_T_I8:
    case CTR_CINTERP_T_U8:
        *(uint8_t*)ptr = (uint8_t)a.u;
        break;
    case CTR_CINTERP_T_I16:
    case CTR_CINTERP_T_U16:
        *(uint16_t*)ptr = (uint16_t)a.u;
        break;
    case CTR_CINTERP_T_I32:
    case CTR_CINTERP_T_U32:
        *(uint32_t*)ptr = (uint32_t)a.u;
        break;
    case CTR_CINTERP_T_F32:
        *(float*)ptr = (float)a.f;
        break;
    case CTR_CINTERP_T_F64:
        *(double*)ptr = a.f;
        break;
    case CTR_CINTERP_T_PTR:
        *(void**)ptr = a.p;
        break;
    default:
        *(uint64_t*)ptr = a.u;
        break;
    }
}

static int ci_call_function(ctr_cinterp* interp, ctr_cinterp_function* fn, ctr_cinterp_value* argv, size_t argc, ctr_cinterp_value* ret, unsigned depth);

static void ci_closure_handler(ffi_cif* cif, void* ret, void** args, void* user)
{
    ctr_cinterp_function* fn = user;
    size_t argc = cif->nargs;
    ctr_cinterp_value stackv[16];
    ctr_cinterp_value* iargs = argc <= 16 ? stackv : calloc(argc ? argc : 1, sizeof(ctr_cinterp_value));
    for (size_t i = 0; i < argc; i++) {
        ffi_type* t = cif->arg_types[i];
        memset(&iargs[i], 0, sizeof(iargs[i]));
        if (t == &ffi_type_float)
            iargs[i].f = *(float*)args[i];
        else if (t == &ffi_type_double)
            iargs[i].f = *(double*)args[i];
        else if (t == &ffi_type_pointer)
            iargs[i].p = *(void**)args[i];
        else if (t->size <= 4)
            iargs[i].i = *(int32_t*)args[i];
        else
            iargs[i].i = *(int64_t*)args[i];
    }
    ctr_cinterp_value iret;
    memset(&iret, 0, sizeof(iret));
    ctr_cinterp_call_function_pointer(fn, iargs, argc, &iret);
    if (cif->rtype == &ffi_type_float)
        *(float*)ret = (float)iret.f;
    else if (cif->rtype == &ffi_type_double)
        *(double*)ret = iret.f;
    else if (cif->rtype == &ffi_type_pointer)
        *(void**)ret = iret.p;
    else if (cif->rtype != &ffi_type_void)
        *(ffi_arg*)ret = (ffi_arg)iret.i;
    if (iargs != stackv)
        free(iargs);
}

static void* ci_function_code(ctr_cinterp* interp, ctr_cinterp_function* fn)
{
    if (!fn)
        return NULL;
    if (fn->closure_code)
        return fn->closure_code;
    if (!ci_grow((void**)&interp->closures, &interp->closure_cap, interp->closure_count, sizeof(*interp->closures)))
        return NULL;
    struct ci_closure_entry* e = &interp->closures[interp->closure_count];
    memset(e, 0, sizeof(*e));
    e->atypes = calloc(fn->argc ? fn->argc : 1, sizeof(ffi_type*));
    if (!e->atypes)
        return NULL;
    for (size_t i = 0; i < fn->argc; i++) {
        ctr_cinterp_type t = fn->arg_slot_type ? fn->arg_slot_type[i] : CTR_CINTERP_T_I64;
        if (t == CTR_CINTERP_T_VOID)
            t = CTR_CINTERP_T_PTR;
        e->atypes[i] = ctr_cinterp_ffi_type(t);
    }
    void* code = NULL;
    e->closure = ffi_closure_alloc(sizeof(ffi_closure), &code);
    if (!e->closure) {
        free(e->atypes);
        return NULL;
    }
    if (ffi_prep_cif(&e->cif, FFI_DEFAULT_ABI, fn->argc, ctr_cinterp_ffi_type(fn->ret_type), e->atypes) != FFI_OK
        || ffi_prep_closure_loc(e->closure, &e->cif, ci_closure_handler, fn, code) != FFI_OK) {
        ffi_closure_free(e->closure);
        free(e->atypes);
        return NULL;
    }
    e->code = code;
    e->fn = fn;
    interp->closure_count++;
    fn->closure_code = code;
    return code;
}

static ctr_cinterp_function* ci_function_from_code(ctr_cinterp* interp, void* code)
{
    for (size_t i = 0; i < interp->closure_count; i++)
        if (interp->closures[i].code == code)
            return interp->closures[i].fn;
    return NULL;
}

typedef union ci_ffi_value {
    int8_t i8;
    uint8_t u8;
    int16_t i16;
    uint16_t u16;
    int32_t i32;
    uint32_t u32;
    int64_t i64;
    uint64_t u64;
    float f32;
    double f64;
    void* ptr;
    ffi_arg ffi;
} ci_ffi_value;

static void* ci_ffi_arg_ptr(ci_ffi_value* storage, ctr_cinterp_type type, ctr_cinterp_value value)
{
    memset(storage, 0, sizeof(*storage));
    switch (type) {
    case CTR_CINTERP_T_I8:
        storage->i8 = (int8_t)value.i;
        return &storage->i8;
    case CTR_CINTERP_T_U8:
        storage->u8 = (uint8_t)value.u;
        return &storage->u8;
    case CTR_CINTERP_T_I16:
        storage->i16 = (int16_t)value.i;
        return &storage->i16;
    case CTR_CINTERP_T_U16:
        storage->u16 = (uint16_t)value.u;
        return &storage->u16;
    case CTR_CINTERP_T_I32:
        storage->i32 = (int32_t)value.i;
        return &storage->i32;
    case CTR_CINTERP_T_U32:
        storage->u32 = (uint32_t)value.u;
        return &storage->u32;
    case CTR_CINTERP_T_F32:
        storage->f32 = (float)value.f;
        return &storage->f32;
    case CTR_CINTERP_T_F64:
        storage->f64 = value.f;
        return &storage->f64;
    case CTR_CINTERP_T_PTR:
        storage->ptr = value.p;
        return &storage->ptr;
    case CTR_CINTERP_T_U64:
        storage->u64 = value.u;
        return &storage->u64;
    case CTR_CINTERP_T_I64:
    default:
        storage->i64 = value.i;
        return &storage->i64;
    }
}

static void ci_ffi_read_ret(ci_ffi_value* storage, ctr_cinterp_type type, ctr_cinterp_value* ret)
{
    if (!ret)
        return;
    memset(ret, 0, sizeof(*ret));
    switch (type) {
    case CTR_CINTERP_T_VOID:
        break;
    case CTR_CINTERP_T_I8:
        ret->i = storage->i8;
        break;
    case CTR_CINTERP_T_U8:
        ret->u = storage->u8;
        break;
    case CTR_CINTERP_T_I16:
        ret->i = storage->i16;
        break;
    case CTR_CINTERP_T_U16:
        ret->u = storage->u16;
        break;
    case CTR_CINTERP_T_I32:
        ret->i = storage->i32;
        break;
    case CTR_CINTERP_T_U32:
        ret->u = storage->u32;
        break;
    case CTR_CINTERP_T_I64:
        ret->i = storage->i64;
        break;
    case CTR_CINTERP_T_U64:
        ret->u = storage->u64;
        break;
    case CTR_CINTERP_T_F32:
        ret->f = storage->f32;
        break;
    case CTR_CINTERP_T_F64:
        ret->f = storage->f64;
        break;
    case CTR_CINTERP_T_PTR:
        ret->p = storage->ptr;
        break;
    }
}

static int ci_call_external(ctr_cinterp* interp, ctr_cinterp_external* ext, ctr_cinterp_value* args, size_t argc, ctr_cinterp_type* arg_types, ctr_cinterp_value* ret)
{
    if (!ext->ptr)
        ext->ptr = dlsym(RTLD_DEFAULT, ext->name);
    if (!ext->ptr) {
        ci_error(interp, "External symbol `%s' is not linked", ext->name);
        return 0;
    }
    if ((!ext->variadic && argc != ext->argc) || (ext->variadic && argc < ext->argc)) {
        ci_error(interp, "Bad call to external `%s': expected %s%zu argument(s), got %zu", ext->name, ext->variadic ? "at least " : "", ext->argc, argc);
        return 0;
    }
    ffi_cif var_cif;
    ffi_type** var_arg_types = NULL;
    ffi_cif* cif = ext->variadic ? &var_cif : &ext->cif;
    if (!ci_prepare_call_cif(ext, cif, &var_arg_types, arg_types, argc)) {
        ci_error(interp, "Could not prepare FFI call for `%s'", ext->name);
        return 0;
    }
    void** ffi_args = calloc(argc ? argc : 1, sizeof(void*));
    if (!ffi_args) {
        free(var_arg_types);
        return 0;
    }
    ci_ffi_value* ffi_values = calloc(argc ? argc : 1, sizeof(ci_ffi_value));
    if (!ffi_values) {
        free(ffi_args);
        free(var_arg_types);
        return 0;
    }
    for (size_t i = 0; i < argc; i++) {
        ctr_cinterp_type type = i < ext->argc ? ext->arg_types[i] : (arg_types ? arg_types[i] : CTR_CINTERP_T_I64);
        if (ext->variadic && i >= ext->argc && type == CTR_CINTERP_T_F32)
            type = CTR_CINTERP_T_F64;
        ffi_args[i] = ci_ffi_arg_ptr(&ffi_values[i], type, args[i]);
    }
    ci_ffi_value ret_storage;
    memset(&ret_storage, 0, sizeof(ret_storage));
    ffi_call(cif, FFI_FN(ext->ptr), &ret_storage, ffi_args);
    ci_ffi_read_ret(&ret_storage, ext->ret_type, ret);
    free(ffi_values);
    free(ffi_args);
    free(var_arg_types);
    return 1;
}

static int ci_call_function(ctr_cinterp* interp, ctr_cinterp_function* fn, ctr_cinterp_value* argv, size_t argc, ctr_cinterp_value* ret, unsigned depth)
{
    if (!fn) {
        ci_error(interp, "Missing bytecode function");
        return 0;
    }
    if (argc != fn->argc) {
        ci_error(interp, "Bad call to bytecode function `%s': expected %zu argument(s), got %zu", fn->name, fn->argc, argc);
        return 0;
    }
    if (depth > 1024) {
        ci_error(interp, "Call depth exceeded while calling bytecode function `%s'", fn->name);
        return 0;
    }
    enum { STACK_MAX = 4096 };
    ctr_cinterp_value stack[STACK_MAX];
    ctr_cinterp_value locals_stack[256];
    ctr_cinterp_value* locals = fn->nlocals <= 256 ? locals_stack : calloc(fn->nlocals, sizeof(ctr_cinterp_value));
    unsigned char frame_stack[2048];
    unsigned char* frame = NULL;
    int frame_heap = 0;
    if (fn->frame_size) {
        if (fn->frame_size <= sizeof(frame_stack)) {
            frame = frame_stack;
            memset(frame, 0, fn->frame_size);
        } else {
            frame = calloc(1, fn->frame_size);
            frame_heap = 1;
        }
        for (size_t i = 0; i < argc && fn->arg_offset; i++) {
            unsigned char* dst = frame + fn->arg_offset[i];
            ctr_cinterp_value av = argv[i];
            if (fn->arg_copy_size && fn->arg_copy_size[i]) {
                if (av.p)
                    memcpy(dst, av.p, fn->arg_copy_size[i]);
                continue;
            }
            switch (fn->arg_slot_type[i]) {
            case CTR_CINTERP_T_I8:
            case CTR_CINTERP_T_U8:
                *(uint8_t*)dst = (uint8_t)av.u;
                break;
            case CTR_CINTERP_T_I16:
            case CTR_CINTERP_T_U16:
                *(uint16_t*)dst = (uint16_t)av.u;
                break;
            case CTR_CINTERP_T_I32:
            case CTR_CINTERP_T_U32:
                *(uint32_t*)dst = (uint32_t)av.u;
                break;
            case CTR_CINTERP_T_F32:
                *(float*)dst = (float)av.f;
                break;
            case CTR_CINTERP_T_F64:
                *(double*)dst = av.f;
                break;
            case CTR_CINTERP_T_PTR:
                *(void**)dst = av.p;
                break;
            default:
                *(uint64_t*)dst = av.u;
                break;
            }
        }
    }
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
            if (ok) {
                v.p = global->data;
                ok = ci_push(stack, &sp, STACK_MAX, v);
            } else {
                ci_error(interp, "Unknown C global `%s'", ins->name);
            }
            break;
        }
        case CI_OP_FUNC_ADDR: {
            ctr_cinterp_function* tf = ctr_cinterp_find_function(interp, ins->name);
            v.p = tf ? ci_function_code(interp, tf) : ctr_cinterp_get_symbol(interp, ins->name);
            ok = ci_push(stack, &sp, STACK_MAX, v);
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
            if (ok)
                ci_store_value(b.p, ins->type, a);
            break;
        case CI_OP_STORE_R:
            ok = ci_pop(stack, &sp, &a) && ci_pop(stack, &sp, &b);
            if (ok) {
                ci_store_value(b.p, ins->type, a);
                ok = ci_push(stack, &sp, STACK_MAX, a);
            }
            break;
        case CI_OP_SWAP:
            ok = ci_pop(stack, &sp, &a) && ci_pop(stack, &sp, &b);
            ok = ok && ci_push(stack, &sp, STACK_MAX, a) && ci_push(stack, &sp, STACK_MAX, b);
            break;
        case CI_OP_FRAME_ADDR:
            v.p = frame + ins->a;
            ok = ci_push(stack, &sp, STACK_MAX, v);
            break;
        case CI_OP_ADD:
        case CI_OP_SUB:
        case CI_OP_MUL:
        case CI_OP_DIV:
            ok = ci_pop(stack, &sp, &b) && ci_pop(stack, &sp, &a);
            if (ok) {
                if (ins->type == CTR_CINTERP_T_F32 || ins->type == CTR_CINTERP_T_F64) {
                    double af = ci_type_is_float((ctr_cinterp_type)ins->a) ? a.f : (double)a.i;
                    double bf = ci_type_is_float((ctr_cinterp_type)ins->b) ? b.f : (double)b.i;
                    v.f = ins->op == CI_OP_ADD ? af + bf : ins->op == CI_OP_SUB ? af - bf
                        : ins->op == CI_OP_MUL                                  ? af * bf
                                                                                : af / bf;
                } else if (ins->op == CI_OP_DIV && b.i == 0) {
                    ci_error(interp, "Integer division by zero in `%s'", fn->name);
                    ok = 0;
                } else {
                    v.i = ins->op == CI_OP_ADD ? a.i + b.i : ins->op == CI_OP_SUB ? a.i - b.i
                        : ins->op == CI_OP_MUL                                    ? a.i * b.i
                                                                                  : a.i / b.i;
                }
                ok = ok && ci_push(stack, &sp, STACK_MAX, v);
            }
            break;
        case CI_OP_MOD:
            ok = ci_pop(stack, &sp, &b) && ci_pop(stack, &sp, &a);
            if (ok && b.i == 0) {
                ci_error(interp, "Integer modulo by zero in `%s'", fn->name);
                ok = 0;
            } else if (ok) {
                v.i = a.i % b.i;
                ok = ci_push(stack, &sp, STACK_MAX, v);
            }
            break;
        case CI_OP_NEG:
            ok = ci_pop(stack, &sp, &a);
            if (ins->type == CTR_CINTERP_T_F32 || ins->type == CTR_CINTERP_T_F64)
                v.f = -a.f;
            else
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
            if (ins->type == CTR_CINTERP_T_F32 || ins->type == CTR_CINTERP_T_F64) {
                double af = ci_type_is_float((ctr_cinterp_type)ins->a) ? a.f : (double)a.i;
                double bf = ci_type_is_float((ctr_cinterp_type)ins->b) ? b.f : (double)b.i;
                v.i = ins->op == CI_OP_EQ ? af == bf : ins->op == CI_OP_NE ? af != bf
                    : ins->op == CI_OP_LT                                  ? af < bf
                    : ins->op == CI_OP_LE                                  ? af <= bf
                    : ins->op == CI_OP_GT                                  ? af > bf
                                                                           : af >= bf;
            } else {
                v.i = ins->op == CI_OP_EQ ? a.i == b.i : ins->op == CI_OP_NE ? a.i != b.i
                    : ins->op == CI_OP_LT                                    ? a.i < b.i
                    : ins->op == CI_OP_LE                                    ? a.i <= b.i
                    : ins->op == CI_OP_GT                                    ? a.i > b.i
                                                                             : a.i >= b.i;
            }
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
                if (ok && callee && ci_type_is_void(callee->ret_type))
                    goto call_done;
            } else if (ok) {
                ctr_cinterp_external* ext = ci_find_external(interp, ins->name);
                if (ext) {
                    ok = ci_call_external(interp, ext, call_args, call_argc, ins->arg_types, &v);
                    if (ok && ci_type_is_void(ext->ret_type))
                        goto call_done;
                } else {
                    ci_error(interp, "Call to external `%s' has no declaration", ins->name);
                    ok = 0;
                }
            }
            ok = ok && ci_push(stack, &sp, STACK_MAX, v);
call_done:
            free(call_args);
            break;
        }
        case CI_OP_CALL_PTR: {
            size_t call_argc = (size_t)ins->a;
            ctr_cinterp_value fp;
            ok = ci_pop(stack, &sp, &fp);
            ctr_cinterp_value* call_args = calloc(call_argc ? call_argc : 1, sizeof(ctr_cinterp_value));
            ok = ok && call_args != NULL && sp >= call_argc;
            for (size_t i = call_argc; ok && i > 0; i--)
                ok = ci_pop(stack, &sp, &call_args[i - 1]);
            memset(&v, 0, sizeof(v));
            ctr_cinterp_function* target = ok ? ci_function_from_code(interp, fp.p) : NULL;
            if (ok && !target && ctr_cinterp_is_function_pointer(fp.p))
                target = (ctr_cinterp_function*)fp.p;
            if (ok && target) {
                ok = ci_call_function(interp, target, call_args, call_argc, &v, depth + 1);
            } else if (ok && fp.p) {
                ci_error(interp, "Indirect native function pointer calls require a declared signature and are not supported yet");
                ok = 0;
            } else if (ok) {
                ci_error(interp, "Indirect call through a null function pointer");
                ok = 0;
            }
            ok = ok && ci_push(stack, &sp, STACK_MAX, v);
            free(call_args);
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
            v = ci_cast_value(a, ins->type, ins->type2);
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
    if (!ok && (!interp || !interp->last_error))
        ci_error(interp, "Runtime error in bytecode function `%s'", fn->name);
    if (locals != locals_stack)
        free(locals);
    if (frame_heap)
        free(frame);
    return ok;
}

int ctr_cinterp_call(ctr_cinterp* interp, char const* name, ctr_cinterp_value* argv, size_t argc, ctr_cinterp_value* ret)
{
    ci_clear_error(interp);
    ctr_cinterp_function* fn = ctr_cinterp_find_function(interp, name);
    if (!fn) {
        ci_error(interp, "No such bytecode function `%s'", name);
        return 0;
    }
    return ci_call_function(interp, fn, argv, argc, ret, 0);
}

char const* ctr_cinterp_last_error(ctr_cinterp const* interp)
{
    return interp ? interp->last_error : NULL;
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
    ci_clear_error(fn->owner);
    return ci_call_function(fn->owner, fn, argv, argc, ret, 0);
}

char const* ctr_cinterp_last_error_for_function_pointer(void* ptr)
{
    ctr_cinterp_function* fn = ptr;
    if (!ctr_cinterp_is_function_pointer(ptr))
        return NULL;
    return ctr_cinterp_last_error(fn->owner);
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
    ci_clear_error(interp);
    int is_asm = strncmp(trimmed, ".function", 9) == 0 || strncmp(trimmed, ".extern", 7) == 0 || strncmp(trimmed, ".global", 7) == 0;
    int ok;
    if (is_asm) {
        ok = ctr_cinterp_compile_text(interp, trimmed);
    } else {
        ci_strbuf pp = { 0 };
        ci_preprocess(interp, trimmed, &pp, 0);
        if (pp.data)
            ci_collect_function_declarations(interp, pp.data);
        int preprocessor_only = ci_source_is_preprocessor_only(trimmed);
        ok = preprocessor_only ? 1 : ctr_cinterp_compile_c(interp, pp.data ? pp.data : "");
        free(pp.data);
    }
    ctr_heap_free(program);
    if (!ok) {
        char const* reason = ctr_cinterp_last_error(interp);
        CtrStdFlow = reason && *reason
            ? ctr_format_str("EC bytecode compilation failed: %s", reason)
            : ctr_build_string_from_cstring("C bytecode compilation failed");
    }
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
        char const* reason = ctr_cinterp_last_error(interp);
        CtrStdFlow = reason && *reason
            ? ctr_format_str("EC bytecode call to `%s' failed: %s", name, reason)
            : ctr_format_str("EC bytecode call to `%s' failed", name);
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
