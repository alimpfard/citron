#include "citron.h"
#ifdef withInjectNative

#include "tcc/tcc.h"
#include <ffi.h>

void *
ctr_inject_compiled_state_release_hook (void *state)
{
  if (state)
    tcc_delete ((struct TCCState *) state);
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
ctr_object *
ctr_inject_make (ctr_object * myself, ctr_argument * argumentList)
{
  int debug = 0;
  int imode = TCC_OUTPUT_MEMORY;
  if (argumentList && argumentList->object && argumentList->object->info.type != CTR_OBJECT_TYPE_OTNIL)
    {
      char *mode = ctr_heap_allocate_cstring (ctr_internal_cast2string (argumentList->object));
      if (strcasecmp (mode, "mem") == 0)
	imode = TCC_OUTPUT_MEMORY;
      else if (strcasecmp (mode, "exe") == 0)
	imode = TCC_OUTPUT_EXE;
      else if (strcasecmp (mode, "dyn") == 0)
	imode = TCC_OUTPUT_DLL;
      else if (strcasecmp (mode, "obj") == 0)
	imode = TCC_OUTPUT_OBJ;
      else
	{
	  CtrStdFlow = ctr_format_str ("EInvalid output mode `%s', expecting any of mem, exe, dyn or obj", mode);
	  ctr_heap_free (mode);
	  return myself;
	}
      ctr_heap_free (mode);
    }
  TCCState *s = tcc_new ();
  if (!s)
    {
      CtrStdFlow = ctr_build_string_from_cstring ("Could not create compiler state");
      return CtrStdNil;
    }
  tcc_set_output_type (s, imode);
  static char buf[10240];
  sprintf (buf, "%s/tcc", ctr_file_stdext_path_raw ());
  tcc_set_lib_path (s, buf);
  // add compiler-specific include path
  sprintf (buf, "%s/include", buf);
  tcc_add_include_path (s, buf);
  tcc_define_symbol (s, "CTR_INJECT", "");
  // TODO: Add meaningful symbols
  // if (debug)
  //     tcc_enable_debug(s);
  ctr_object *handle = ctr_internal_create_object (CTR_OBJECT_TYPE_OTEX);
  ctr_set_link_all (handle, CtrStdInject);
  handle->release_hook = ctr_inject_compiled_state_release_hook;
  ctr_inject_data_t *ds = ctr_heap_allocate (sizeof *ds);
  ds->info.relocated = 0;
  ds->state = s;
  handle->value.rvalue = ctr_heap_allocate (sizeof (ctr_resource));
  handle->value.rvalue->ptr = ds;
  return handle;
}

typedef struct
{
  size_t member_count;
  size_t max_alignment;
  pad_info_node_t **pad_structure;
  size_t format_length;
  ctr_object *names;
  char *format;
} struct_member_desc_complement_t;

static inline struct_member_desc_complement_t
new_struct_member_desc_complement ()
{
  struct_member_desc_complement_t obj = { };
  obj.names = ctr_array_new (CtrStdArray, NULL);
  obj.format = ctr_heap_allocate (obj.format_length = 1024);
  obj.pad_structure = ctr_heap_allocate (sizeof (*obj.pad_structure));
  return obj;
}

typedef struct ctr_inferred_ctype_type_t
{
  int kind;
  int nb_args;
  int fnptrq;
  union
  {
    ffi_type *return_type;
    ffi_type *vtype;
    ctr_object *const_value;
  };
  union
  {
    struct ctr_inferred_ctype_type_t **arguments;
    struct_member_desc_complement_t struct_member_desc;
    struct ctr_inferred_ctype_type_t *contained;
  };
} ctr_inferred_ctype_type_t;
extern TokenSym **table_ident;
enum ctr_inferred_ctype_type_kind
{
  KIND_BASIC,
  KIND_FUNCTION,
  KIND_STRUCT,
  KIND_CONSTANT,
  KIND_ARRAY
};
extern int type_size (CType * type, int *a);
static inline CType* pointed_type(CType *type) {
  return &type->ref->type;
}
ctr_object *
ctr_inject_generate_ctype (ctr_inferred_ctype_type_t ty);

ctr_inferred_ctype_type_t
ctr_inject_type_to_ctype (Sym * svv, CType * type)
{
  int bt, t;
  Sym *s, *sa;
  int is_unsigned = 0;
  ffi_type *fty = NULL;

  t = type->t & VT_TYPE;
  bt = t & VT_BTYPE;

  if (t & VT_CONSTANT)
    // pstrcat(buf, buf_size, "const ");
    ;				// can't do anything to this
  if (t & VT_VOLATILE)
    // pstrcat(buf, buf_size, "volatile ");
    ;				// can't do anything to this
  if (t & VT_UNSIGNED)
    is_unsigned = 1;

  if ((t & VT_ARRAY) && (bt == VT_PTR)) {
    // it be an array?
    s = type->ref;
    long array_count = s->c;
    void type_to_str(char*,int,CType*,char*);
    char fbuf[1024];
    char nbuf[1024];
    int tok;
    tok = svv->v&~SYM_STRUCT;
    tok -= TOK_IDENT;
	  TokenSym *token = table_ident[tok];
	  sprintf (nbuf, "%.*s", token->len, token->str);
    (void)type_to_str(fbuf, 1024, pointed_type(type), nbuf);
    printf("%s[%lu]\n", fbuf, array_count);
    ctr_inferred_ctype_type_t contained = ctr_inject_type_to_ctype (svv, pointed_type(type));
    ctr_inferred_ctype_type_t *inner = ctr_heap_allocate(sizeof*inner);
    memcpy(inner, &contained, sizeof*inner);

    return (ctr_inferred_ctype_type_t) {
      .kind = KIND_ARRAY,
      .nb_args = array_count,
      .contained = inner
    };
    // ctr_send_message_variadic (ns, "initWith:names:", 15, 2, cifobj, de.names);
  }

  switch (bt)
    {
    case VT_VOID:
      fty = &ffi_type_void;
      goto add_tstr;
    case VT_BOOL:
      /* die */
      CtrStdFlow = ctr_build_string_from_cstring ("_Bool is not supported with automatic inference");
      return (ctr_inferred_ctype_type_t)
      {
      .vtype = NULL};
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
      fty = &ffi_type_ulong;	// ?
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
      return (ctr_inferred_ctype_type_t)
      {
      .vtype = fty};
    case VT_ENUM:
      return (ctr_inferred_ctype_type_t)
      {
      .const_value = ctr_build_number_from_float (svv->c),.kind = KIND_CONSTANT};
    case VT_STRUCT:
      /* TODO: fix structs */
      // return (ctr_inferred_ctype_type_t){.vtype=NULL};
      // v = type->ref->v & ~SYM_STRUCT;
      // if (v >= SYM_FIRST_ANOM)
      //     pstrcat(buf, buf_size, "<anonymous>");
      // else
      //     pstrcat(buf, buf_size, get_tok_str(v, NULL));
      // break;
      s = svv;
      fty = ctr_heap_allocate (sizeof (ffi_type));
      fty->size = 0;
      fty->alignment = 0;
      fty->type = FFI_TYPE_STRUCT;
      struct_member_desc_complement_t descriptor = new_struct_member_desc_complement ();
      int offset = 0;

      ffi_type **elems = ctr_heap_allocate (sizeof (ffi_type *));	//plus one for the terminating NULL
      size_t this_size = 0;
      int this_alignment = 0;
      int max_alignment = 0;
      char *buf = descriptor.format;
      size_t remaining_buf_len = descriptor.format_length;

      while ((s = s->next) != NULL)
	{
	  // really shitty, but it's not exactly a hot-spot
	  descriptor.pad_structure =
	    ctr_heap_reallocate (descriptor.pad_structure, sizeof (*descriptor.pad_structure) * (descriptor.member_count + 1));
	  elems = ctr_heap_reallocate (elems, (descriptor.member_count + 1) * sizeof (*elems));

	  pad_info_node_t *current_padinfo = ctr_heap_allocate (sizeof (pad_info_node_t *));
	  descriptor.pad_structure[descriptor.member_count++] = current_padinfo;
	  current_padinfo->offset = offset;
	  offset += type_size (&s->type, &this_alignment);
	  max_alignment = fmax (max_alignment, this_alignment);
	  int pad = 0;
	  int tok = s->v & ~SYM_FIELD;
	  if (tok > SYM_FIRST_ANOM)
	    pad = 1;
	  if (!pad)
	    {
	      tok -= TOK_IDENT;
	      TokenSym *token = table_ident[tok];
	      printf ("FIELD %.*s\n", token->len, token->str);
	      // todo get field name
	      ctr_array_push (descriptor.names, &(ctr_argument)
			      {
			      ctr_build_string (token->str, token->len)}
	      );
	    }
	  current_padinfo->pad = pad;
	  ctr_inferred_ctype_type_t innerty = ctr_inject_type_to_ctype (s, &s->type);
	  if (!pad)
	    {
	      if (ctr_create_ffi_str_descriptor (innerty.vtype, NULL) > remaining_buf_len)
		{
		  remaining_buf_len += descriptor.format_length;
		  buf = descriptor.format = ctr_heap_reallocate (descriptor.format, descriptor.format_length += descriptor.format_length);
		}
	      int sz = ctr_create_ffi_str_descriptor (innerty.vtype, buf);
	      buf += sz;
	      remaining_buf_len -= sz;
	    }
	  elems[descriptor.member_count - 1] = innerty.vtype;
	}
      elems[descriptor.member_count] = NULL;
      fty->elements = elems;
      fty->size = offset;
      descriptor.max_alignment = max_alignment;
      buf[0] = 0;
      ctr_inferred_ctype_type_t ftype = {
	.kind = KIND_STRUCT,
	.vtype = fty,
	.struct_member_desc = descriptor
      };
      return ftype;
    case VT_FUNC:
      s = type->ref;
      ctr_inferred_ctype_type_t fntype = {
	.kind = KIND_FUNCTION
      };
      ctr_inferred_ctype_type_t rtype = ctr_inject_type_to_ctype (s, &s->type);
      if (!rtype.vtype)
	{
	  // cascade error down
	  return rtype;
	}
      fntype.return_type = rtype.vtype;
      sa = s->next;
      while (sa != NULL)
	{
	  ctr_inferred_ctype_type_t argtype = ctr_inject_type_to_ctype (sa, &sa->type);
	  if (!argtype.vtype)
	    {
	      // dynarray_reset((void***)&fntype.arguments, &fntype.nb_args);
	      return (ctr_inferred_ctype_type_t)
	      {
	      .kind = KIND_FUNCTION,.return_type = NULL};
	    }
	  ctr_inferred_ctype_type_t *argtype_p = ctr_heap_allocate (sizeof (argtype));
	  *argtype_p = argtype;
	  dynarray_add ((void ***) &fntype.arguments, &fntype.nb_args, argtype_p);
	  sa = sa->next;
	  if (sa == s)
	    break;		// wtf
	}
      return fntype;
    case VT_PTR:
      if ((type->ref->type.t & VT_TYPE & VT_BTYPE) == VT_FUNC) {
        void type_to_str(char*,int,CType*,char*);
        char fbuf[1024];
        (void)type_to_str(fbuf, 1024, type, NULL);
        // printf("Function pointer %s\n", fbuf);
        ctr_inferred_ctype_type_t val =  ctr_inject_type_to_ctype(svv, &type->ref->type);
        val.fnptrq = 1;
        return val;
      }
      // :shrug:
      return (ctr_inferred_ctype_type_t)
      {
      .vtype = &ffi_type_pointer};
    }
  return (ctr_inferred_ctype_type_t)
  {
  .vtype = &ffi_type_sint};	// good guess!
}

void
ctr_inject_free_function_type (ctr_inferred_ctype_type_t ty)
{
  for (size_t i = 0; i < ty.nb_args; i++)
    {
      ctr_inferred_ctype_type_t *aty = ty.arguments[i];
      if (aty->kind == KIND_FUNCTION)
	ctr_inject_free_function_type (*aty);
    }
  // dynarray_reset((void***)&ty.arguments, &ty.nb_args);
}

extern void ctr_ctypes_set_type (ctr_object * object, ctr_ctype type);
extern ctr_ctype ctr_ctypes_ffi_convert_ffi_type_to_ctype (ffi_type * type);
ffi_type *ctr_ctypes_ffi_convert_to_ffi_type (ctr_object * type);
ssize_t ctr_ctype_get_c_size (ctr_object * obj);

ctr_object *
ctr_inject_generate_ctype (ctr_inferred_ctype_type_t ty)
{
  if (ty.kind == KIND_FUNCTION)
    {
      ffi_cif *cif_res = ctr_heap_allocate (sizeof (ffi_cif));
      ffi_type *rtype = ty.return_type;
      int asize = ty.nb_args;
      ffi_type **atypes = ctr_heap_allocate (sizeof (ffi_type) * asize);
      for (int i = 0; i < asize; i++)
	{
	  ctr_inferred_ctype_type_t t = *ty.arguments[i];
	  atypes[i] = t.kind == KIND_FUNCTION ? &ffi_type_pointer : t.vtype;	// can't be anything else, but just in case
	}
      ffi_abi abi = FFI_DEFAULT_ABI;
      ffi_status status = ffi_prep_cif (cif_res, abi, asize, rtype, atypes);
      if (status != FFI_OK)
	CtrStdFlow = ctr_build_string_from_cstring ((status == FFI_BAD_ABI ? "FFI_BAD_ABI" : "FFI_ERROR"));
      ctr_object *cifobj = ctr_internal_create_object (CTR_OBJECT_TYPE_OTEX);
      ctr_internal_object_set_property (cifobj,
					ctr_build_string_from_cstring (":crType"),
					ctr_build_number_from_float (ctr_ctypes_ffi_convert_ffi_type_to_ctype (ty.vtype)), 0);
      ctr_ctypes_set_type (cifobj, CTR_CTYPE_CIF);
      ctr_set_link_all (cifobj, CtrStdCType_ffi_cif);
      ctr_internal_object_set_property(cifobj, ctr_build_string_from_cstring(":cfnptr"), ctr_build_bool(ty.fnptrq), 0);
      cifobj->value.rvalue->ptr = (void *) cif_res;
      return cifobj;
    }
  else if (ty.kind == KIND_STRUCT)
    {
      ctr_object *cifobj = ctr_internal_create_object (CTR_OBJECT_TYPE_OTEX);
      ctr_ctypes_set_type (cifobj, CTR_CTYPE_STRUCT);

      ctr_ctypes_ffi_struct_value *ptr = ctr_heap_allocate (sizeof (ctr_ctypes_ffi_struct_value));
      struct_member_desc_complement_t de = ty.struct_member_desc;

      ptr->member_count = de.member_count;
      ptr->type = ty.vtype;
      ptr->size = ty.vtype->size;
      ptr->padinfo = de.pad_structure;
      ptr->value = NULL;
      ptr->original_format = de.format;
      cifobj->value.rvalue->ptr = (void *) ptr;
      ctr_send_message_variadic (ctr_find (ctr_build_string_from_cstring ("import")), "Library/Foreign/C/NamedStruct", 29, 0);
      ctr_object *ns = ctr_send_message_variadic (ctr_find (ctr_build_string_from_cstring ("NamedStruct")),
						  "new",
						  3,
						  0);
      ctr_send_message_variadic (ns, "initWith:names:", 15, 2, cifobj, de.names);
      return ns;
    }
  else if (ty.kind == KIND_CONSTANT)
    {
      return ty.const_value;
    }
  else if (ty.kind == KIND_ARRAY)
    {
      int array_count = ty.nb_args;
      // now go ahead and get the contained type
      ctr_object* ctype = ctr_inject_generate_ctype(*ty.contained);
      // ctr_send_message_variadic (ctr_find (ctr_build_string_from_cstring ("import")), "Library/Foreign/C/PackedArray", 29, 0);
      // ctr_object *ns = ctr_find (ctr_build_string_from_cstring ("PackedArray"));
      size_t size = ctr_ctype_get_c_size(ctype);
      void *storage = ctr_heap_allocate(array_count * size);
      ctr_ctypes_cont_array_t *arr = ctr_heap_allocate(sizeof (ctr_ctypes_cont_array_t));
      arr->storage = storage;
      arr->count = array_count;
      arr->esize = size;
      arr->etype = ctr_ctypes_ffi_convert_to_ffi_type(ctype);
      ctr_object *obj = ctr_ctypes_make_cont_pointer (NULL, NULL);
      obj->value.rvalue->ptr = arr;
      // ns = ctr_send_message_variadic(ns, "initWithCType:count:", 20, 2, obj, ctr_build_number_from_float(array_count));
      return obj;
    }
  else
    {
      // just a normal value type
      ctr_object *cifobj = ctr_internal_create_object (CTR_OBJECT_TYPE_OTEX);
      ctr_ctypes_set_type (cifobj, ctr_ctypes_ffi_convert_ffi_type_to_ctype (ty.vtype));
      return cifobj;
    }
}


ctr_object *
ctr_inject_defined_functions (ctr_object * myself, ctr_argument * argumentList)
{
  ctr_resource *r = myself->value.rvalue;
  ctr_inject_data_t *ds;
  if (!r || !(ds = r->ptr))
    {
      CtrStdFlow = ctr_build_string_from_cstring ("request to uninitialized Inject object");
      return CtrStdNil;
    }
  ctr_object *all = NULL;
  if (argumentList && argumentList->object)
    all = argumentList->object;
  all = all == CtrStdNil ? NULL : all;
  TCCState *state = ds->state;
  Sym *symbols = state->global_stack;
  ctr_object *type_map = ctr_map_new (CtrStdMap, NULL);
  ctr_argument *map_put_arg = &(ctr_argument) {.next = &(ctr_argument) {}
  };

  Sym *s;
  TokenSym *ts;
  int v;

  s = symbols;
  typeof(ctr_map_contains)* fn_all_contains_val =
    all->info.type == CTR_OBJECT_TYPE_OTARRAY ?
      &ctr_array_contains :
      &ctr_map_contains;
  while (s != NULL)
    {
      v = s->v;
      /* remove symbol in token array */
      /* XXX: simplify */
      if (!(v & SYM_FIELD) && (v & ~SYM_STRUCT) < SYM_FIRST_ANOM)
	{
	  ts = table_ident[(v & ~SYM_STRUCT) - TOK_IDENT];
	  int tok;
    tok = v&~SYM_STRUCT;
	  if (tok < TOK_IDENT)
	    goto NOT_THIS_ONE;
	  tok -= TOK_IDENT;
	  TokenSym *tokdata = ts; //table_ident[tok];

	  ctr_object *svname = ctr_build_string (tokdata->str, tokdata->len);

	  if (!all)
	    goto go_on;
	  if (!fn_all_contains_val (all, &(ctr_argument)
				   {
				   .object = svname}
	      )->value.bvalue)
	    goto NOT_THIS_ONE;

	go_on:;
	  ctr_inferred_ctype_type_t ty = ctr_inject_type_to_ctype (s, &s->type);
	  if (!ty.vtype)
	    {
	      if (ty.kind == KIND_FUNCTION)
		ctr_inject_free_function_type (ty);
        if (ty.kind != KIND_ARRAY)
	  goto NOT_THIS_ONE;
	    }

	  map_put_arg->object = ctr_inject_generate_ctype (ty);
	  map_put_arg->next->object = svname;

	  ctr_map_put (type_map, map_put_arg);
	}
    NOT_THIS_ONE:
      s = s->prev;
    }

  return type_map;
}

ctr_object *
ctr_inject_finish(ctr_object * myself, ctr_argument * argumentList)
{
  ctr_resource *r = myself->value.rvalue;
  ctr_inject_data_t *ds;
  if (!r || !(ds = r->ptr))
    {
      CtrStdFlow = ctr_build_string_from_cstring ("deinit request to uninitialized Inject object");
      return CtrStdNil;
    }
    TCCState* s = ds->state;
    tcc_delete(s);
    ctr_heap_free(ds);
    r->ptr = NULL;
    return myself;
}

/**
 * [Inject] addLibraryPath: [String]
 *
 */
ctr_object *
ctr_inject_add_libp (ctr_object * myself, ctr_argument * argumentList)
{
  ctr_resource *r = myself->value.rvalue;
  ctr_inject_data_t *ds;
  if (!r || !(ds = r->ptr))
    {
      CtrStdFlow = ctr_build_string_from_cstring ("compile request to uninitialized Inject object");
      return CtrStdNil;
    }
  ctr_object *l = argumentList->object;
  CTR_ENSURE_TYPE_STRING (l);
  char *ls = ctr_heap_allocate_cstring (l);
  TCCState *s = ds->state;
  int status = tcc_add_library_path (s, ls);
  ctr_heap_free (ls);
  return ctr_build_bool (status != -1);
}

/**
 * [Inject] libraryPaths
 *
 */
ctr_object *
ctr_inject_get_libp (ctr_object * myself, ctr_argument * argumentList)
{
  ctr_resource *r = myself->value.rvalue;
  ctr_inject_data_t *ds;
  if (!r || !(ds = r->ptr))
    {
      CtrStdFlow = ctr_build_string_from_cstring ("compile request to uninitialized Inject object");
      return CtrStdNil;
    }
  TCCState *s = ds->state;
  ctr_argument arg = { NULL, NULL };
  ctr_object *res = ctr_array_new (CtrStdArray, NULL);
  for (size_t i = 0; i < s->nb_library_paths; i++)
    {
      arg.object = ctr_build_string_from_cstring (s->library_paths[i]);
      ctr_array_push (res, &arg);
    }
  return res;
}

/**
 * [Inject] addIncludePath: [String]
 *
 */
ctr_object *
ctr_inject_add_inclp (ctr_object * myself, ctr_argument * argumentList)
{
  ctr_resource *r = myself->value.rvalue;
  ctr_inject_data_t *ds;
  if (!r || !(ds = r->ptr))
    {
      CtrStdFlow = ctr_build_string_from_cstring ("compile request to uninitialized Inject object");
      return CtrStdNil;
    }
  ctr_object *l = argumentList->object;
  CTR_ENSURE_TYPE_STRING (l);
  char *ls = ctr_heap_allocate_cstring (l);
  TCCState *s = ds->state;
  int status = tcc_add_include_path (s, ls);
  ctr_heap_free (ls);
  return ctr_build_bool (status != -1);
}

/**
 * [Inject] includePaths
 *
 */
ctr_object *
ctr_inject_get_inclp (ctr_object * myself, ctr_argument * argumentList)
{
  ctr_resource *r = myself->value.rvalue;
  ctr_inject_data_t *ds;
  if (!r || !(ds = r->ptr))
    {
      CtrStdFlow = ctr_build_string_from_cstring ("compile request to uninitialized Inject object");
      return CtrStdNil;
    }
  TCCState *s = ds->state;
  ctr_argument arg = { NULL, NULL };
  ctr_object *res = ctr_array_new (CtrStdArray, NULL);
  for (size_t i = 0; i < s->nb_include_paths; i++)
    {
      arg.object = ctr_build_string_from_cstring (s->include_paths[i]);
      ctr_array_push (res, &arg);
    }
  return res;
}

/**
 *[Inject] compile: [String]
 *
 * Compile a C program into memory
 */
ctr_object *
ctr_inject_compile (ctr_object * myself, ctr_argument * argumentList)
{
  if (ctr_check_permission_internal (CTR_SECPRO_NO_SHELL))
    return myself;
  ctr_resource *r = myself->value.rvalue;
  ctr_inject_data_t *ds;
  if (!r || !(ds = r->ptr))
    {
      CtrStdFlow = ctr_build_string_from_cstring ("compile request to uninitialized Inject object");
      return CtrStdNil;
    }
  ctr_object *prg = argumentList->object;
  CTR_ENSURE_TYPE_STRING (prg);
  char *program = ctr_heap_allocate_cstring (prg);
  TCCState *s = ds->state;
  int status = tcc_compile_string (s, program);
  ctr_heap_free (program);
  if (status == -1)
    CtrStdFlow = ctr_build_string_from_cstring ("Compilation failed");
  return myself;
}

ctr_object *
ctr_inject_generate_output (ctr_object * myself, ctr_argument * argumentList)
{
  if (ctr_check_permission_internal (CTR_SECPRO_NO_SHELL))
    return myself;
  ctr_resource *r = myself->value.rvalue;
  ctr_inject_data_t *ds;
  if (!r || !(ds = r->ptr) || ds->info.relocated)
    {
      CtrStdFlow = ctr_build_string_from_cstring ("Inject object is in an invalid state (not fit for compilation output)");
      return CtrStdNil;
    }
  TCCState *s = ds->state;
  char const *filename = "a.out";
  int alloc = 0;
  if (argumentList && argumentList->object)
    {
      filename = ctr_heap_allocate_cstring (ctr_internal_cast2string (argumentList->object));
      alloc = 1;
    }
  int res = tcc_output_file (s, filename);
  if (alloc)
    ctr_heap_free (filename);
  if (res)
    CtrStdFlow = ctr_build_string_from_cstring ("Failed to generate output");
  return myself;
}

/**
 *[Inject] run: [String] arguments: [[String]]
 *
 * Run a complete C program's main function
 */
ctr_object *
ctr_inject_run (ctr_object * myself, ctr_argument * argumentList)
{
  if (ctr_check_permission_internal (CTR_SECPRO_NO_SHELL))
    return myself;
  ctr_resource *r = myself->value.rvalue;
  ctr_inject_data_t *ds;
  if (!r || !(ds = r->ptr))
    {
      CtrStdFlow = ctr_build_string_from_cstring ("compile request to uninitialized Inject object");
      return CtrStdNil;
    }
  if (ds->info.relocated)
    {
      CtrStdFlow = ctr_format_str ("ESymbols in this context have already been relocated");
      return CtrStdNil;
    }
  ctr_object *prg = argumentList->object;
  CTR_ENSURE_TYPE_STRING (prg);
  char *program = ctr_heap_allocate_cstring (prg);
  TCCState *s = ds->state;
  int status = tcc_compile_string (s, program);
  ctr_heap_free (program);
  if (CtrStdFlow)
    return CtrStdNil;		// stop exec in case of any errors in the handler
  ctr_object *argl = argumentList->next->object;
  CTR_ENSURE_TYPE_ARRAY (argl);
  int length = ctr_array_count (argl, NULL)->value.nvalue;
  char **argv = malloc (sizeof (char *) * (length + 1));
  argv[length] = NULL;
  ctr_collection *arr = argl->value.avalue;
  for (int i = 0; i < length; i++)
    argv[i] = ctr_heap_allocate_cstring (ctr_internal_cast2string (arr->elements[arr->tail + i]));
  int result = tcc_run (s, length, argv);
  ds->info.relocated = 1;
  for (int i = 0; i < length; i++)
    ctr_heap_free (argv[i]);
  free (argv);
  return ctr_build_number_from_float (result);
}

/**
 *[Inject] run: [String] arguments: [[String]] function: [String]
 *
 * Run a complete C program's given function
 */
ctr_object *
ctr_inject_run_named (ctr_object * myself, ctr_argument * argumentList)
{
  if (ctr_check_permission_internal (CTR_SECPRO_NO_SHELL))
    return myself;
  ctr_resource *r = myself->value.rvalue;
  ctr_inject_data_t *ds;
  if (!r || !(ds = r->ptr))
    {
      CtrStdFlow = ctr_build_string_from_cstring ("compile request to uninitialized Inject object");
      return CtrStdNil;
    }
  if (ds->info.relocated)
    {
      CtrStdFlow = ctr_format_str ("ESymbols in this context have already been relocated");
      return CtrStdNil;
    }
  ctr_object *prg = argumentList->object;
  CTR_ENSURE_TYPE_STRING (prg);
  char *program = ctr_heap_allocate_cstring (prg);
  TCCState *s = ds->state;
  int status = tcc_compile_string (s, program);
  ctr_heap_free (program);
  if (CtrStdFlow)
    return CtrStdNil;		// stop exec in case of any errors in the handler
  ctr_object *argl = argumentList->next->object;
  CTR_ENSURE_TYPE_ARRAY (argl);
  int length = ctr_array_count (argl, NULL)->value.nvalue;
  char **argv = malloc (sizeof (char *) * (length + 1));
  argv[length] = NULL;
  ctr_collection *arr = argl->value.avalue;
  for (int i = 0; i < length; i++)
    argv[i] = ctr_heap_allocate_cstring (ctr_internal_cast2string (arr->elements[arr->tail + i]));
  int (*fn) (int, char **);
  ssize_t size = tcc_relocate (s, NULL);
  if (size <= 0)
    {
      CtrStdFlow = ctr_build_string_from_cstring ("Relocation failed");
      return CtrStdNil;
    }
  void *mem = ctr_heap_allocate (size);
  tcc_relocate (s, mem);
  ds->info.relocated = 1;
  ctr_object *sym = argumentList->next->next->object;
  if (sym->info.type != CTR_OBJECT_TYPE_OTSTRING)
    {
      CtrStdFlow = ctr_format_str ("EExpected String but got %S", ctr_internal_cast2string (ctr_send_message_blocking (sym, "type", 4, NULL)));
      return CtrStdNil;
    }
  char *sym_s = ctr_heap_allocate_cstring (sym);
  fn = tcc_get_symbol (s, sym_s);
  if (!fn)
    {
      ctr_heap_free (sym_s);
      CtrStdFlow = ctr_format_str ("ENo such function %s", sym_s);
      return CtrStdNil;
    }
  int result = fn (length, argv);
  for (int i = 0; i < length; i++)
    ctr_heap_free (argv[i]);
  free (argv);
  return ctr_build_number_from_float (result);
}

/**
 * [Inject] symbol: [String|Symbol]
 *
 */
ctr_object *
ctr_inject_get_symbol (ctr_object * myself, ctr_argument * argumentList)
{
  ctr_resource *r = myself->value.rvalue;
  ctr_inject_data_t *ds;
  if (!r || !(ds = r->ptr))
    {
      CtrStdFlow = ctr_build_string_from_cstring ("compile request to uninitialized Inject object");
      return CtrStdNil;
    }
  ctr_object *sym = argumentList->object;
  if (sym->info.type != CTR_OBJECT_TYPE_OTSTRING)
    {
      CtrStdFlow = ctr_format_str ("EExpected String but got %S", ctr_internal_cast2string (ctr_send_message_blocking (sym, "type", 4, NULL)));
      return CtrStdNil;
    }
  char *sym_s = ctr_heap_allocate_cstring (sym);
  TCCState *s = ds->state;
  if (!ds->info.relocated)
    {
      ssize_t size = tcc_relocate (s, NULL);
      if (size <= 0)
	{
	  CtrStdFlow = ctr_build_string_from_cstring ("Relocation failed");
	  return CtrStdNil;
	}
      void *mem = ctr_heap_allocate (size);
      tcc_relocate (s, mem);
      ds->info.relocated = 1;
    }
  ctr_object *ptr = ctr_ctypes_make_pointer (NULL, NULL);
  ptr->value.rvalue->ptr = tcc_get_symbol (s, sym_s);
  ctr_heap_free (sym_s);
  return ptr;
}

/**
 * [Inject] linkInLibrary: [String]
 *
 */
ctr_object *
ctr_inject_add_lib (ctr_object * myself, ctr_argument * argumentList)
{
  if (ctr_check_permission_internal (CTR_SECPRO_NO_SHELL))
    return myself;
  ctr_resource *r = myself->value.rvalue;
  ctr_inject_data_t *ds;
  if (!r || !(ds = r->ptr))
    {
      CtrStdFlow = ctr_build_string_from_cstring ("link request to uninitialized Inject object");
      return CtrStdNil;
    }
  ctr_object *l = argumentList->object;
  CTR_ENSURE_TYPE_STRING (l);
  char *ls = ctr_heap_allocate_cstring (l);
  TCCState *s = ds->state;
  int status = tcc_add_library (s, ls);
  ctr_heap_free (ls);
  if (status == -1)
    CtrStdFlow = ctr_build_string_from_cstring ("Linking failed");
  return myself;
}

void
ctr_inject_error_handler (void *_blk, char *msg)
{
  ctr_object *blk = _blk;
  if (!(blk->info.type == CTR_OBJECT_TYPE_OTBLOCK || blk->info.type == CTR_OBJECT_TYPE_OTNATFUNC))
    return;
  ctr_argument arg;
  arg.next = NULL;
  arg.object = ctr_build_string_from_cstring (msg);
  ctr_block_run (blk, &arg, blk);
  ctr_heap_free (arg.object);
}

/**
 * [Inject] errorHandler: [Block<String>]
 *
 */
ctr_object *
ctr_inject_set_error_handler (ctr_object * myself, ctr_argument * argumentList)
{
  ctr_resource *r = myself->value.rvalue;
  ctr_inject_data_t *ds;
  if (!r || !(ds = r->ptr))
    {
      CtrStdFlow = ctr_build_string_from_cstring ("set request to uninitialized Inject object");
      return CtrStdNil;
    }
  ctr_object *l = argumentList->object;
  TCCState *s = ds->state;
  tcc_set_error_func (s, l, ctr_inject_error_handler);
  return myself;
}
#endif // withInjectNative
