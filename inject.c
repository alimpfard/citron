#include "citron.h"
#ifdef withInjectNative

#include "tcc/tcc.h"
#include <ffi.h>

void *ctr_inject_compiled_state_release_hook(void *state)
{
    if (state)
        tcc_delete((struct TCCState *)state);
    return NULL;
}

struct ctr_inject_data_t
{
    struct
    {
        int relocated;
    } info;
    TCCState *state;
};

typedef struct ctr_inject_data_t ctr_inject_data_t;

/**
 * [Inject] newWithOutputMode: [outmode:String]
 * [Inject] new (no debug syms)
 *
 * Generate a new context to compile C programs into
 *
 * outmode: any of
 *  - 'mem' into mem(default)
 *  - 'exe' executable
 *  - 'dyn' dynamic library
 *  - 'obj' object file
 */
ctr_object *ctr_inject_make(ctr_object *myself, ctr_argument *argumentList)
{
    int debug = 0;
    int imode = TCC_OUTPUT_MEMORY;
    if (argumentList && argumentList->object && argumentList->object->info.type != CTR_OBJECT_TYPE_OTNIL) {
      const char* mode = ctr_heap_allocate_cstring(ctr_internal_cast2string(argumentList->object));
      if (strcasecmp(mode, "mem") == 0)
        imode = TCC_OUTPUT_MEMORY;
      else if (strcasecmp(mode, "exe") == 0)
        imode = TCC_OUTPUT_EXE;
      else if (strcasecmp(mode, "dyn") == 0)
        imode = TCC_OUTPUT_DLL;
      else if (strcasecmp(mode, "obj") == 0)
        imode = TCC_OUTPUT_OBJ;
      else {
        CtrStdFlow = ctr_format_str("EInvalid output mode `%s', expecting any of mem, exe, dyn or obj", mode);
        ctr_heap_free(mode);
        return myself;
      }
      ctr_heap_free(mode);
    }
    TCCState *s = tcc_new();
    if (!s)
    {
        CtrStdFlow = ctr_build_string_from_cstring("Could not create compiler state");
        return CtrStdNil;
    }
    tcc_set_output_type(s, imode);
    static char buf[10240];
    sprintf(buf, "%s/tcc", ctr_file_stdext_path_raw());
    tcc_set_lib_path(s, buf);
    // add compiler-specific include path
    sprintf(buf, "%s/include", buf);
    tcc_add_include_path(s, buf);
    tcc_define_symbol(s, "CTR_INJECT", "");
    // TODO: Add meaningful symbols
    // if (debug)
    //     tcc_enable_debug(s);
    ctr_object *handle = ctr_internal_create_object(CTR_OBJECT_TYPE_OTEX);
    ctr_set_link_all(handle, CtrStdInject);
    handle->release_hook = ctr_inject_compiled_state_release_hook;
    ctr_inject_data_t *ds = ctr_heap_allocate(sizeof *ds);
    ds->info.relocated = 0;
    ds->state = s;
    handle->value.rvalue = ctr_heap_allocate(sizeof(ctr_resource));
    handle->value.rvalue->ptr = ds;
    return handle;
}

typedef struct ctr_inferred_ctype_type_t {
  int is_function;
  int nb_args;
  union {
    ffi_type* return_type;
    ffi_type* vtype;
  };
  struct ctr_inferred_ctype_type_t** arguments;
} ctr_inferred_ctype_type_t;

ctr_inferred_ctype_type_t ctr_inject_type_to_ctype(CType *type)
{
    int bt, t;
    Sym *s, *sa;
    int is_unsigned = 0;
    ffi_type* fty = NULL;

    t = type->t & VT_TYPE;
    bt = t & VT_BTYPE;

    if (t & VT_CONSTANT)
        // pstrcat(buf, buf_size, "const ");
        ; // can't do anything to this
    if (t & VT_VOLATILE)
        // pstrcat(buf, buf_size, "volatile ");
        ; // can't do anything to this
    if (t & VT_UNSIGNED)
        is_unsigned = 1;

    switch(bt) {
    case VT_VOID:
        fty = &ffi_type_void;
        goto add_tstr;
    case VT_BOOL:
      /* die */
        CtrStdFlow = ctr_build_string_from_cstring("_Bool is not supported with automatic inference");
        return (ctr_inferred_ctype_type_t){.vtype=NULL};
    case VT_BYTE:
        fty = &ffi_type_sint8;
        goto add_tstr;
    case VT_SHORT:
        fty = &ffi_type_sshort;
        goto add_tstr;
    case VT_INT:
        fty = &ffi_type_sint;
        goto add_tstr;
    case VT_LONG:
        fty = &ffi_type_slong;
        goto add_tstr;
    case VT_LLONG:
        fty = &ffi_type_ulong; // ?
        goto add_tstr;
    case VT_FLOAT:
        fty = &ffi_type_float;
        goto add_tstr;
    case VT_DOUBLE:
        fty = &ffi_type_double;
        goto add_tstr;
    case VT_LDOUBLE:
        fty = &ffi_type_longdouble;
    add_tstr:
        return (ctr_inferred_ctype_type_t){.vtype=fty};
    case VT_ENUM:
    case VT_STRUCT:
        if (bt != VT_STRUCT) {
          CtrStdFlow = ctr_build_string_from_cstring("Enums are not supported as automatic inference targets");
          return (ctr_inferred_ctype_type_t){.vtype=NULL};
        }
        /* TODO: fix structs */
        return (ctr_inferred_ctype_type_t){.vtype=NULL};
        // v = type->ref->v & ~SYM_STRUCT;
        // if (v >= SYM_FIRST_ANOM)
        //     pstrcat(buf, buf_size, "<anonymous>");
        // else
        //     pstrcat(buf, buf_size, get_tok_str(v, NULL));
        // break;
    case VT_FUNC:
        s = type->ref;
        ctr_inferred_ctype_type_t fntype = {
          .is_function = 1
        };
        ctr_inferred_ctype_type_t rtype = ctr_inject_type_to_ctype(&s->type);
        if (!rtype.vtype) {
          // cascade error down
          return rtype;
        }
        fntype.return_type = rtype.vtype;
        sa = s->next;
        while (sa != NULL) {
            ctr_inferred_ctype_type_t argtype = ctr_inject_type_to_ctype(&sa->type);
            if (!argtype.vtype) {
              // dynarray_reset((void***)&fntype.arguments, &fntype.nb_args);
              return (ctr_inferred_ctype_type_t){.is_function=1, .return_type=NULL};
            }
            ctr_inferred_ctype_type_t* argtype_p = ctr_heap_allocate(sizeof(argtype));
            *argtype_p = argtype;
            dynarray_add((void***)&fntype.arguments, &fntype.nb_args, argtype_p);
            sa = sa->next;
            if (sa == s) break; // wtf
        }
        return fntype;
    case VT_PTR:
        // s = type->ref;
        // :shrug:
        return (ctr_inferred_ctype_type_t){.vtype = &ffi_type_pointer};
    }
    return (ctr_inferred_ctype_type_t){.vtype = &ffi_type_sint}; // good guess!
}

void ctr_inject_free_function_type(ctr_inferred_ctype_type_t ty) {
  for (size_t i = 0; i < ty.nb_args; i++) {
    ctr_inferred_ctype_type_t* aty = ty.arguments[i];
    if (aty->is_function)
      ctr_inject_free_function_type(*aty);
  }
  // dynarray_reset((void***)&ty.arguments, &ty.nb_args);
}
extern void ctr_ctypes_set_type(ctr_object* object, ctr_ctype type);
extern ctr_ctype ctr_ctypes_ffi_convert_ffi_type_to_ctype(ffi_type* type);

ctr_object *ctr_inject_generate_ctype(ctr_inferred_ctype_type_t ty) {
  if (ty.is_function) {
    ffi_cif*      cif_res = ctr_heap_allocate(sizeof (ffi_cif));
    ffi_type*     rtype   = ty.return_type;
    int           asize   = ty.nb_args;
    ffi_type**    atypes  = ctr_heap_allocate(sizeof(ffi_type) * asize);
    for(int i = 0; i<asize; i++) {
      ctr_inferred_ctype_type_t t = *ty.arguments[i];
      atypes[i] = t.is_function ? &ffi_type_pointer : t.vtype; // can't be anything else, but just in case
    }
    ffi_abi abi = FFI_DEFAULT_ABI;
    ffi_status status = ffi_prep_cif(cif_res, abi, asize, rtype, atypes);
    if (status != FFI_OK)
      CtrStdFlow = ctr_build_string_from_cstring((status==FFI_BAD_ABI?"FFI_BAD_ABI":"FFI_ERROR"));
    ctr_object* cifobj = ctr_internal_create_object(CTR_OBJECT_TYPE_OTEX);
    ctr_internal_object_set_property (
      cifobj,
      ctr_build_string_from_cstring(":crType"),
      ctr_build_number_from_float(ctr_ctypes_ffi_convert_ffi_type_to_ctype(ty.vtype)),
      0
    );
    ctr_ctypes_set_type(cifobj, CTR_CTYPE_CIF);
    ctr_set_link_all(cifobj, CtrStdCType_ffi_cif);
    cifobj->value.rvalue->ptr = (void*)cif_res;
    return cifobj;
  } else {
    // just a normal value type
    ctr_object* cifobj = ctr_internal_create_object(CTR_OBJECT_TYPE_OTEX);
    ctr_ctypes_set_type(cifobj, ctr_ctypes_ffi_convert_ffi_type_to_ctype(ty.vtype));
    return cifobj;
  }
}

extern TokenSym **table_ident;

ctr_object *ctr_inject_defined_functions(ctr_object* myself, ctr_argument* argumentList)
{
  ctr_resource *r = myself->value.rvalue;
  ctr_inject_data_t *ds;
  if (!r || !(ds = r->ptr))
  {
      CtrStdFlow = ctr_build_string_from_cstring("request to uninitialized Inject object");
      return CtrStdNil;
  }
  TCCState *state = ds->state;
  Sym* symbols = state->global_stack;
  ctr_object *type_map = ctr_map_new(CtrStdMap, NULL);
  ctr_argument* map_put_arg = &(ctr_argument){ .next = &(ctr_argument){} };

  Sym *s, *ss, **ps;
  TokenSym *ts;
  int v;

  s = symbols;
  while(s != NULL) {
      ss = s->prev;
      v = s->v;
      /* remove symbol in token array */
      /* XXX: simplify */
      if (!(v & SYM_FIELD) && (v & ~SYM_STRUCT) < SYM_FIRST_ANOM) {
          ts = table_ident[(v & ~SYM_STRUCT) - TOK_IDENT];
          if (v & SYM_STRUCT)
              // ps = &ts->sym_struct;
              goto NOT_THIS_ONE;
          else
              ps = &ts->sym_identifier;
          ctr_inferred_ctype_type_t ty = ctr_inject_type_to_ctype(&s->type);
          if (!ty.vtype) {
            if (ty.is_function)
              ctr_inject_free_function_type(ty);
            goto NOT_THIS_ONE;
          }
          int tok = s->v;
          if (tok < TOK_IDENT)
            goto NOT_THIS_ONE;
          tok -= TOK_IDENT;
          TokenSym *tokdata = table_ident[tok];

          map_put_arg->object = ctr_inject_generate_ctype(ty);
          map_put_arg->next->object = ctr_build_string(tokdata->str, tokdata->len);

          ctr_map_put(type_map, map_put_arg);
      }
      NOT_THIS_ONE:
      s = ss;
  }

  return type_map;
}

/**
 * [Inject] addIncludePath: [String]
 *
 */
ctr_object *ctr_inject_add_inclp(ctr_object* myself, ctr_argument* argumentList)
{
    ctr_resource *r = myself->value.rvalue;
    ctr_inject_data_t *ds;
    if (!r || !(ds = r->ptr))
    {
        CtrStdFlow = ctr_build_string_from_cstring("compile request to uninitialized Inject object");
        return CtrStdNil;
    }
    ctr_object *l = argumentList->object;
    CTR_ENSURE_TYPE_STRING(l);
    char *ls = ctr_heap_allocate_cstring(l);
    TCCState *s = ds->state;
    int status = tcc_add_include_path(s, ls);
    ctr_heap_free(ls);
    return ctr_build_bool(status != -1);
}
/**
 *[Inject] compile: [String]
 *
 * Compile a C program into memory
 */
ctr_object *ctr_inject_compile(ctr_object *myself, ctr_argument *argumentList)
{
    if (ctr_check_permission_internal (CTR_SECPRO_NO_SHELL))
      return myself;
    ctr_resource *r = myself->value.rvalue;
    ctr_inject_data_t *ds;
    if (!r || !(ds = r->ptr))
    {
        CtrStdFlow = ctr_build_string_from_cstring("compile request to uninitialized Inject object");
        return CtrStdNil;
    }
    ctr_object *prg = argumentList->object;
    CTR_ENSURE_TYPE_STRING(prg);
    char *program = ctr_heap_allocate_cstring(prg);
    TCCState *s = ds->state;
    int status = tcc_compile_string(s, program);
    ctr_heap_free(program);
    if (status == -1)
        CtrStdFlow = ctr_build_string_from_cstring("Compilation failed");
    return myself;
}

ctr_object *ctr_inject_generate_output(ctr_object *myself, ctr_argument *argumentList)
{
    if (ctr_check_permission_internal (CTR_SECPRO_NO_SHELL))
      return myself;
    ctr_resource *r = myself->value.rvalue;
    ctr_inject_data_t *ds;
    if (!r || !(ds = r->ptr) || ds->info.relocated)
    {
        CtrStdFlow = ctr_build_string_from_cstring("Inject object is in an invalid state (not fit for compilation output)");
        return CtrStdNil;
    }
    TCCState *s = ds->state;
    char const* filename = "a.out";
    int alloc = 0;
    if (argumentList && argumentList->object) {
      filename = ctr_heap_allocate_cstring(ctr_internal_cast2string(argumentList->object));
      alloc = 1;
    }
    int res = tcc_output_file(s, filename);
    if (alloc)
      ctr_heap_free(filename);
    if (res)
      CtrStdFlow = ctr_build_string_from_cstring("Failed to generate output");
    return myself;
}

/**
 *[Inject] run: [String] arguments: [[String]]
 *
 * Run a complete C program's main function
 */
ctr_object *ctr_inject_run(ctr_object *myself, ctr_argument *argumentList)
{
    if (ctr_check_permission_internal (CTR_SECPRO_NO_SHELL))
      return myself;
    ctr_resource *r = myself->value.rvalue;
    ctr_inject_data_t *ds;
    if (!r || !(ds = r->ptr))
    {
        CtrStdFlow = ctr_build_string_from_cstring("compile request to uninitialized Inject object");
        return CtrStdNil;
    }
    if (ds->info.relocated)
    {
      CtrStdFlow = ctr_format_str("ESymbols in this context have already been relocated");
      return CtrStdNil;
    }
    ctr_object *prg = argumentList->object;
    CTR_ENSURE_TYPE_STRING(prg);
    char *program = ctr_heap_allocate_cstring(prg);
    TCCState *s = ds->state;
    int status = tcc_compile_string(s, program);
    ctr_heap_free(program);
    if (CtrStdFlow) return CtrStdNil; // stop exec in case of any errors in the handler
    ctr_object* argl = argumentList->next->object;
    CTR_ENSURE_TYPE_ARRAY(argl);
    int length = ctr_array_count(argl, NULL)->value.nvalue;
    char** argv = malloc(sizeof(char*) * (length+1));
    argv[length] = NULL;
    ctr_collection* arr = argl->value.avalue;
    for(int i=0; i<length; i++)
        argv[i] = ctr_heap_allocate_cstring(ctr_internal_cast2string(arr->elements[arr->tail+i]));
    int result = tcc_run(s, length, argv);
    ds->info.relocated = 1;
    for(int i=0; i<length; i++)
        ctr_heap_free(argv[i]);
    free(argv);
    return ctr_build_number_from_float(result);
}

/**
 *[Inject] run: [String] arguments: [[String]] function: [String]
 *
 * Run a complete C program's given function
 */
ctr_object *ctr_inject_run_named(ctr_object *myself, ctr_argument *argumentList)
{
    if (ctr_check_permission_internal (CTR_SECPRO_NO_SHELL))
      return myself;
    ctr_resource *r = myself->value.rvalue;
    ctr_inject_data_t *ds;
    if (!r || !(ds = r->ptr))
    {
        CtrStdFlow = ctr_build_string_from_cstring("compile request to uninitialized Inject object");
        return CtrStdNil;
    }
    if (ds->info.relocated)
    {
      CtrStdFlow = ctr_format_str("ESymbols in this context have already been relocated");
      return CtrStdNil;
    }
    ctr_object *prg = argumentList->object;
    CTR_ENSURE_TYPE_STRING(prg);
    char *program = ctr_heap_allocate_cstring(prg);
    TCCState *s = ds->state;
    int status = tcc_compile_string(s, program);
    ctr_heap_free(program);
    if (CtrStdFlow) return CtrStdNil; // stop exec in case of any errors in the handler
    ctr_object* argl = argumentList->next->object;
    CTR_ENSURE_TYPE_ARRAY(argl);
    int length = ctr_array_count(argl, NULL)->value.nvalue;
    char** argv = malloc(sizeof(char*) * (length+1));
    argv[length] = NULL;
    ctr_collection* arr = argl->value.avalue;
    for(int i=0; i<length; i++)
        argv[i] = ctr_heap_allocate_cstring(ctr_internal_cast2string(arr->elements[arr->tail+i]));
    int (*fn)(int,char**);
    ssize_t size = tcc_relocate(s, NULL);
    if (size <= 0) {
        CtrStdFlow = ctr_build_string_from_cstring("Relocation failed");
    return CtrStdNil;
    }
    void *mem = ctr_heap_allocate(size);
    tcc_relocate(s, mem);
    ds->info.relocated = 1;
    ctr_object *sym = argumentList->next->next->object;
    if (sym->info.type != CTR_OBJECT_TYPE_OTSTRING)
    {
      CtrStdFlow = ctr_format_str("EExpected String but got %S", ctr_internal_cast2string(ctr_send_message_blocking(sym, "type", 4, NULL)));
      return CtrStdNil;
    }
    char *sym_s = ctr_heap_allocate_cstring(sym);
    fn = tcc_get_symbol(s, sym_s);
    if (!fn) {
      ctr_heap_free(sym_s);
      CtrStdFlow = ctr_format_str("ENo such function %s", sym_s);
      return CtrStdNil;
    }
    int result = fn(length, argv);
    for(int i=0; i<length; i++)
        ctr_heap_free(argv[i]);
    free(argv);
    return ctr_build_number_from_float(result);
}

/**
 * [Inject] symbol: [String|Symbol]
 *
 */
ctr_object *ctr_inject_get_symbol(ctr_object *myself, ctr_argument *argumentList)
{
    ctr_resource *r = myself->value.rvalue;
    ctr_inject_data_t *ds;
    if (!r || !(ds = r->ptr))
    {
        CtrStdFlow = ctr_build_string_from_cstring("compile request to uninitialized Inject object");
        return CtrStdNil;
    }
    ctr_object *sym = argumentList->object;
    if (sym->info.type != CTR_OBJECT_TYPE_OTSTRING)
    {
        CtrStdFlow = ctr_format_str("EExpected String but got %S", ctr_internal_cast2string(ctr_send_message_blocking(sym, "type", 4, NULL)));
        return CtrStdNil;
    }
    char *sym_s = ctr_heap_allocate_cstring(sym);
    TCCState *s = ds->state;
    if (!ds->info.relocated)
    {
        ssize_t size = tcc_relocate(s, NULL);
        if (size <= 0) {
            CtrStdFlow = ctr_build_string_from_cstring("Relocation failed");
        return CtrStdNil;
        }
        void *mem = ctr_heap_allocate(size);
        tcc_relocate(s, mem);
        ds->info.relocated = 1;
    }
    ctr_object *ptr = ctr_ctypes_make_pointer(NULL, NULL);
    ptr->value.rvalue->ptr = tcc_get_symbol(s, sym_s);
    ctr_heap_free(sym_s);
    return ptr;
}

/**
 * [Inject] linkInLibrary: [String]
 *
 */
ctr_object *ctr_inject_add_lib(ctr_object* myself, ctr_argument* argumentList)
{
    if (ctr_check_permission_internal (CTR_SECPRO_NO_SHELL))
      return myself;
    ctr_resource *r = myself->value.rvalue;
    ctr_inject_data_t *ds;
    if (!r || !(ds = r->ptr))
    {
        CtrStdFlow = ctr_build_string_from_cstring("link request to uninitialized Inject object");
        return CtrStdNil;
    }
    ctr_object *l = argumentList->object;
    CTR_ENSURE_TYPE_STRING(l);
    char *ls = ctr_heap_allocate_cstring(l);
    TCCState *s = ds->state;
    int status = tcc_add_library(s, ls);
    ctr_heap_free(ls);
    if (status == -1)
        CtrStdFlow = ctr_build_string_from_cstring("Linking failed");
    return myself;
}

void ctr_inject_error_handler(void* _blk, char* msg)
{
    ctr_object* blk = _blk;
    if (!(blk->info.type == CTR_OBJECT_TYPE_OTBLOCK || blk->info.type == CTR_OBJECT_TYPE_OTNATFUNC)) return;
    ctr_argument arg;
    arg.next = NULL;
    arg.object = ctr_build_string_from_cstring(msg);
    ctr_block_run(blk, &arg, blk);
    ctr_heap_free(arg.object);
}

/**
 * [Inject] errorHandler: [Block<String>]
 *
 */
ctr_object *ctr_inject_set_error_handler(ctr_object* myself, ctr_argument* argumentList)
{
    ctr_resource *r = myself->value.rvalue;
    ctr_inject_data_t *ds;
    if (!r || !(ds = r->ptr))
    {
        CtrStdFlow = ctr_build_string_from_cstring("set request to uninitialized Inject object");
        return CtrStdNil;
    }
    ctr_object *l = argumentList->object;
    TCCState *s = ds->state;
    tcc_set_error_func(s, l, ctr_inject_error_handler);
    return myself;
}
#endif // withInjectNative
