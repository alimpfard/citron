#ifdef existing
#include <Citron/citron.h>
#else
#warning "We don't have Citron installed"
#include "../../citron.h"
#endif
#include <tcl.h>
#include <tk.h>
#include <limits.h>

/* libs
-I/software/tcl-7.4/include -I/software/tk-4.0/include -I/software/x11r5_dev/Include -L/software/tcl-7.4/lib -L/software/tk-4.0/lib -L/software/x11r5_dev/lib -ltk -ltcl -lX11
*/

#define ctrraise(STR, myself) {(*get_CtrStdFlow()) = ctr_build_string_from_cstring(STR); return myself;}
#define ctrfraise(STR, myself, ...) {(*get_CtrStdFlow()) = ctr_format_str(STR, __VA_ARGS__); return myself;}

static ctr_object* ctrtcl_interpobj;
static ctr_object* CSTR_INTERP = NULL;
static ctr_object* ctr_bound_fn_map = NULL;

typedef struct {
  Tcl_Interp* interp;
  int initialized;
} ctr_tcl_interp;

static ctr_tcl_interp ctrtcl_interp = {0, 0};

void ctr_tcl_create() {
  if(!ctrtcl_interp.interp) {
    ctr_bound_fn_map = ctr_internal_create_object(CTR_OBJECT_TYPE_OTMISC);//tfp nonlocal

    CSTR_INTERP = ctr_build_string_from_cstring(":interp");
    ctrtcl_interp.interp = Tcl_CreateInterp();
    if(!ctrtcl_interp.interp) ctrraise("Could not create Tcl interpreter",);
    ctrtcl_interpobj = ctr_internal_create_object(CTR_OBJECT_TYPE_OTEX);
    ctr_set_link_all(ctrtcl_interpobj, CtrStdObject);
    ctr_internal_object_set_property(ctrtcl_interpobj, CSTR_INTERP, ctr_build_number_from_float((intptr_t)&ctrtcl_interp), 0);
  }
}

ctr_object* ctr_tcl_init(ctr_object* myself, ctr_argument* _ign) {
  ctr_tcl_interp* interp;
  ctr_object* interps = ctr_internal_object_find_property(myself, CSTR_INTERP, 0);
  if (interps == NULL) ctrraise("Tcl object not created", myself);
  interp = (void*)(intptr_t)interps->value.nvalue;

  if (interp->initialized) ctrraise("Already initialized", myself);

  if(Tcl_Init(interp->interp) != TCL_OK) ctrfraise("EFailed to initialize: %s", myself, Tcl_GetStringResult(interp->interp));

  interp->initialized = 1;

  return myself;
}

ctr_object* ctr_tcl_eval(ctr_object* myself, ctr_argument* argumentList) {
  ctr_tcl_interp* interp;
  ctr_object* interps = ctr_internal_object_find_property(myself, CSTR_INTERP, 0);
  if (interps == NULL) ctrraise("Tcl object not initialized", myself);
  interp = (void*)(intptr_t)interps->value.nvalue;
  char* s = ctr_heap_allocate_cstring(ctr_internal_cast2string(argumentList->object));
  int rc = Tcl_Eval(interp->interp, s);
  ctr_heap_free(s);
  if(rc != TCL_OK) ctrraise((char*)Tcl_GetStringResult(interp->interp), myself);
  return myself;
}

static Tcl_Obj* ctr_tcl_as_obj(ctr_object* ctrobj, void* interp) {
  switch(ctrobj->info.type) {
    case CTR_OBJECT_TYPE_OTNIL: return Tcl_NewBooleanObj(0); //emulate false
    case CTR_OBJECT_TYPE_OTNUMBER: {
      double intpart;
      if(modf(ctrobj->value.nvalue, &intpart) == 0) {//int
#ifdef TCL_WIDE_INT_TYPE
        Tcl_WideInt wideValue;
#endif
        if(intpart < LONG_MAX && intpart > LONG_MIN) {
          return Tcl_NewLongObj(intpart);
        }
#ifdef TCL_WIDE_INT_TYPE
        wideValue = intpart;
        return Tcl_NewWideIntObj(wideValue);
#endif
      }
      return Tcl_NewDoubleObj(ctrobj->value.nvalue);
    }
    case CTR_OBJECT_TYPE_OTBOOL: Tcl_NewBooleanObj(ctrobj->value.bvalue);
    case CTR_OBJECT_TYPE_OTSTRING: {
      size_t slen = ctr_getutf8len(ctrobj->value.svalue->value, ctrobj->value.svalue->vlen);
      Tcl_UniChar* inbuf = ctr_heap_allocate(sizeof(Tcl_UniChar)*slen);
      for(size_t i=0; i<slen; i++) {
        long ua = getBytesUtf8(ctrobj->value.svalue->value, 0, i);
        long ub = getBytesUtf8(ctrobj->value.svalue->value, i, 1);
        memcpy(inbuf+i, ctrobj->value.svalue->value+ua, ub);
      }
      return Tcl_NewUnicodeObj(inbuf, slen);
    }
    case CTR_OBJECT_TYPE_OTARRAY: {
      Tcl_Obj *argv = Tcl_NewListObj(1, NULL);
      size_t i;
      for(i=ctrobj->value.avalue->tail;i<ctrobj->value.avalue->head;i++) {
        Tcl_ListObjAppendElement(interp, argv, ctr_tcl_as_obj(ctrobj->value.avalue->elements[i], interp));
      }
      return argv;
    }
    default: return ctr_tcl_as_obj(ctr_internal_cast2string(ctrobj), interp);
  }
}

struct tcl_run_data {
  ctr_object* blk;
  ctr_argument* argumentList;
};

int ctr_tcl_run_blk(ClientData clientData, Tcl_Interp* interp, int objc, Tcl_Obj *const objv[]) {
  ctr_argument* argumentList = ctr_heap_allocate(sizeof(*argumentList));
  ctr_argument* argm = argumentList;
  int namelen;
  char* name = Tcl_GetStringFromObj(objv[0], &namelen);
  ctr_object* sym = ctr_get_or_create_symbol_table_entry(name, namelen);
  ctr_object* blk = ctr_internal_object_find_property(ctr_bound_fn_map, sym, 0);
  if(!blk) {
    (*get_CtrStdFlow()) = ctr_build_string_from_cstring("No such bound function");
    Tcl_SetObjResult(interp, ctr_tcl_as_obj(*get_CtrStdFlow(), interp));
    return TCL_ERROR;
  }
  int len;
  for(int i=1; i<objc; i++) {
    char* str = Tcl_GetStringFromObj(objv[i], &len);
    argm->object = ctr_build_string(str, len);
    argm->next = ctr_heap_allocate(sizeof(*argumentList));
    argm = argm->next;
  }
  argm->next = NULL;

  ctr_object* res = ctr_block_run(blk, argumentList, blk);
  ctr_free_argumentList(argumentList);
  if((*get_CtrStdFlow()) == CtrStdBreak) {
    (*get_CtrStdFlow()) = NULL;
    return TCL_BREAK;
  } else if ((*get_CtrStdFlow()) == CtrStdContinue) {
    (*get_CtrStdFlow()) = NULL;
    return TCL_CONTINUE;
  } else if ((*get_CtrStdFlow()) != NULL) {
    Tcl_SetObjResult(interp, ctr_tcl_as_obj(*get_CtrStdFlow(), interp));
    return TCL_ERROR;
  }
  if(res != blk) Tcl_SetObjResult(interp, ctr_tcl_as_obj(res, interp));
  return TCL_OK;
}

void kill_argml(ClientData clientData) {
  struct tcl_run_data* data = clientData;
  if(data->argumentList) ctr_free_argumentList(data->argumentList);
  ctr_heap_free(data);
}

ctr_object* ctr_tcl_fnof(ctr_object* myself, ctr_argument* argumentList) {
  ctr_tcl_interp* interp;
  ctr_object* blk = argumentList->object;
  if(blk->info.type != CTR_OBJECT_TYPE_OTBLOCK) ctrraise("Expected a block", myself);
  argumentList->object = blk;
  // blk = ctr_reflect_fn_copy(CtrStdReflect, argumentList);
  ctr_object* interps = ctr_internal_object_find_property(myself, CSTR_INTERP, 0);
  if (interps == NULL) ctrraise("Tcl object not initialized", myself);
  interp = (void*)(intptr_t)interps->value.nvalue;
  static char fnname[1024];
  size_t len = sprintf(fnname, "CtrBlock%ld", (intptr_t)blk);
  ctr_object* sym = ctr_get_or_create_symbol_table_entry(fnname, len);
  ctr_internal_object_set_property(ctr_bound_fn_map, sym, blk, 0);
  Tcl_Command cmd = Tcl_CreateObjCommand(interp->interp, fnname, &ctr_tcl_run_blk, NULL, NULL);
  if(!cmd) ctrraise((char*)Tcl_GetStringResult(interp->interp), myself);
  ctr_object* blk_ren = ctr_build_string_from_cstring(fnname);
  ctr_internal_object_add_property(blk_ren, ctr_build_string_from_cstring(":function"), blk, 0);
  return blk_ren;
}

ctr_object*
ctr_tcl_valof (ctr_object* myself, ctr_argument* argumentList) {
  ctr_tcl_interp* interp;
  ctr_object* interps = ctr_internal_object_find_property(myself, CSTR_INTERP, 0);
  if (interps == NULL) ctrraise("Tcl object not initialized", myself);
  interp = (void*)(intptr_t)interps->value.nvalue;
  ctr_string* s = ctr_internal_cast2string(argumentList->object)->value.svalue;
  int rc = Tcl_EvalEx(interp->interp, s->value, s->vlen, 0);
  ctr_heap_free(s);
  char* res = (char*)Tcl_GetStringResult(interp->interp);
  if(rc != TCL_OK) ctrraise(res, myself);
  return ctr_build_string_from_cstring(res);
}

ctr_object* ctr_tcl_shutdown(ctr_object* myself, ctr_argument* argumentList) {
  ctr_tcl_interp* interp;
  ctr_object* interps = ctr_internal_object_find_property(myself, CSTR_INTERP, 0);
  if (interps == NULL) ctrraise("Tcl object not created", myself);
  interp = (void*)(intptr_t)interps->value.nvalue;
  Tcl_Finalize();
  interp->initialized = -1;
  return myself;
}

void begin() {
  ctr_tcl_create();
  ctr_internal_create_func(ctrtcl_interpobj, ctr_build_string_from_cstring("init"), &ctr_tcl_init);
  ctr_internal_create_func(ctrtcl_interpobj, ctr_build_string_from_cstring("eval:"), &ctr_tcl_eval);
  ctr_internal_create_func(ctrtcl_interpobj, ctr_build_string_from_cstring("shutdown"), &ctr_tcl_shutdown);
  ctr_internal_create_func(ctrtcl_interpobj, ctr_build_string_from_cstring("fnOf:"), &ctr_tcl_fnof);
  ctr_internal_create_func(ctrtcl_interpobj, ctr_build_string_from_cstring("valueOf:"), &ctr_tcl_valof);
  ctr_internal_object_add_property(CtrStdWorld, ctr_build_string_from_cstring("Tcl"), ctrtcl_interpobj, 0);
}
