#include "citron.h"
#include "tcc/tcc.h"

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
 * [Inject] newWithDebugSyms: [Boolean]
 * [Inject] new (no debug syms)
 *
 * Generate a new context to compile C programs into
 */
ctr_object *ctr_inject_make(ctr_object *myself, ctr_argument *argumentList)
{
    int debug = argumentList && ctr_internal_cast2bool(argumentList->object)->value.bvalue;
    TCCState *s = tcc_new();
    if (!s)
    {
        CtrStdFlow = ctr_build_string_from_cstring("Could not create compiler state");
        return CtrStdNil;
    }
    tcc_set_output_type(s, TCC_OUTPUT_MEMORY);
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

/**
 *[Inject] run: [String] arguments: [[String]]
 *
 * Run a complete C program's main function
 */
ctr_object *ctr_inject_run(ctr_object *myself, ctr_argument *argumentList)
{
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
