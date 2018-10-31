#include "Python.h"

#ifdef existing
#include <Citron/citron.h>
#else
#include "../../citron.h"
#endif

#define ctrraise(STR, myself) {(*get_CtrStdFlow()) = ctr_build_string_from_cstring(STR); return myself;}
#define CTR_PY_GETOBJECT(obj) ((PyObject*)obj->value.rvalue->ptr)

static ctr_object* ctr_bound_fn_map = NULL;
static ctr_object* ctrpy_interpobj = NULL;
static ctr_object* ctr_py_code_obj = NULL;
static ctr_object* ctr_py_proxy_obj = NULL;
static ctr_object* ctr_py_codegstr = NULL;
static ctr_object* ctr_py_codemstr = NULL;
static ctr_object* ctr_py_codelstr = NULL;
static ctr_object* ctr_py_codeistr = NULL;
static ctr_object* ctr_py_fnpstr = NULL;

void ctr_py_create() {
  if(!Py_IsInitialized()) {
    ctr_bound_fn_map = ctr_internal_create_object(CTR_OBJECT_TYPE_OTMISC);//tfp nonlocal
    ctrpy_interpobj = ctr_internal_create_object(CTR_OBJECT_TYPE_OTEX);
    ctr_py_code_obj = ctr_internal_create_object(CTR_OBJECT_TYPE_OTOBJECT);
    ctr_py_proxy_obj = ctr_internal_create_object(CTR_OBJECT_TYPE_OTOBJECT);
    ctr_set_link_all(ctrpy_interpobj, CtrStdObject);
    ctr_set_link_all(ctr_py_code_obj, CtrStdObject);
    ctr_set_link_all(ctr_py_proxy_obj, CtrStdObject);
    ctr_py_codegstr = ctr_build_string_from_cstring(":pyInterpGlob");
    ctr_py_codelstr = ctr_build_string_from_cstring(":pyInterpLoc");
    ctr_py_codeistr = ctr_build_string_from_cstring(":pyInterp");
    ctr_py_codemstr = ctr_build_string_from_cstring(":pyInterpModule");
    ctr_py_fnpstr   = ctr_build_string_from_cstring(":pyCallbackFunc");
  }
}

ctr_object* ctr_py_code_create(PyObject* rc) {
  ctr_object* obj = ctr_internal_create_object(CTR_OBJECT_TYPE_OTEX);
  ctr_set_link_all(obj, ctr_py_code_obj);
  obj->value.rvalue = ctr_heap_allocate(sizeof(ctr_resource));
  obj->value.rvalue->ptr = rc;
  return obj;
}

ctr_object* ctr_py_proxy(PyObject* rc) {
  ctr_object* obj = ctr_internal_create_object(CTR_OBJECT_TYPE_OTEX);
  ctr_set_link_all(obj, ctr_py_proxy_obj);
  obj->value.rvalue = ctr_heap_allocate(sizeof(ctr_resource));
  obj->value.rvalue->ptr = rc;
  return obj;
}

static PyObject* ctr_py_as_pyobject(ctr_object* ctrobj) {
  if (ctrobj->interfaces->link == ctr_py_code_obj) return CTR_PY_GETOBJECT(ctrobj);
  switch(ctrobj->info.type) {
    case CTR_OBJECT_TYPE_OTNIL:       Py_RETURN_NONE;
    case CTR_OBJECT_TYPE_OTBOOL:      return PyBool_FromLong(ctrobj->value.bvalue);
    case CTR_OBJECT_TYPE_OTNUMBER:    {
      double intpart;
      if(modf(ctrobj->value.nvalue, &intpart) == 0) {//int
        if(intpart < LONG_MAX && intpart > LONG_MIN) {
          return PyLong_FromLong(intpart);
        }
      }
      return PyFloat_FromDouble(ctrobj->value.nvalue);
    }
    case CTR_OBJECT_TYPE_OTSTRING: return PyUnicode_FromStringAndSize(ctrobj->value.svalue->value, ctrobj->value.svalue->vlen);
    case CTR_OBJECT_TYPE_OTARRAY: {
      ctr_collection* coll = ctrobj->value.avalue;
      if (coll->immutable) {
        PyObject* list = PyTuple_New(coll->head - coll->tail);
        for (int i=coll->tail; i<coll->head; i++)
          PyTuple_SetItem(list, i, ctr_py_as_pyobject(coll->elements[i]));
        return list;
      } else {
        PyObject* list = PyList_New(coll->head - coll->tail);
        for (int i=coll->tail; i<coll->head; i++)
          PyList_SetItem(list, i, ctr_py_as_pyobject(coll->elements[i]));
        return list;
      }
    }
    case CTR_OBJECT_TYPE_OTOBJECT:
    case CTR_OBJECT_TYPE_OTMISC: {
      // just pass it off as a dict
      PyObject* dict = PyDict_New();
      ctr_map* coll = ctrobj->properties;
      ctr_mapitem* mapitem = coll->head;
      for (int i = 0; i<coll->size; i++,mapitem=mapitem->next)
        PyDict_SetItem(dict, ctr_py_as_pyobject(mapitem->key), ctr_py_as_pyobject(mapitem->value));
      return dict;
    }
    case CTR_OBJECT_TYPE_OTEX:
    case CTR_OBJECT_TYPE_OTNATFUNC:
    case CTR_OBJECT_TYPE_OTBLOCK:
    default: /* todo */ Py_RETURN_NONE;
  }
}

ctr_object* ctr_py_run_pyfunc(ctr_object* myself, ctr_argument* argumentList);

static ctr_object* ctr_py_from_pyobject(PyObject* pyobj) {
  if (!pyobj || pyobj == Py_None) return CtrStdNil;
  if (PyLong_Check(pyobj)) return ctr_build_number_from_float(PyLong_AsLong(pyobj));
  if (PyFloat_Check(pyobj)) return ctr_build_number_from_float(PyFloat_AsDouble(pyobj));
  if (PyBool_Check(pyobj)) return ctr_build_bool(Py_True == pyobj);
  if (PyUnicode_Check(pyobj)) return ctr_build_string(PyUnicode_DATA(pyobj), PyUnicode_GET_LENGTH(pyobj)); // experimental [maybe disallow this]
  if (PyBytes_Check(pyobj)) return ctr_build_string(PyBytes_AS_STRING(pyobj), PyBytes_GET_SIZE(pyobj));
  if (PyTuple_Check(pyobj)) {
    ctr_size tlength = PyTuple_Size(pyobj);
    ctr_object* idx = ctr_build_number_from_float(tlength);
    ctr_argument arg = {
      idx,
      NULL
    },
    arg1 = {
      NULL,
      &arg
    };
    ctr_object* tuple = ctr_array_new(CtrStdArray, &arg);
    for (int i = 0; i<tlength; i++) {
      idx->value.nvalue = i;
      arg1.object = ctr_py_from_pyobject(PyTuple_GetItem(pyobj, i));
      ctr_array_put(tuple, &arg1);
    }
    tuple->value.avalue->immutable = 1;
    return tuple;
  }
  if (PyList_Check(pyobj)) {
    ctr_size tlength = PyList_Size(pyobj);
    ctr_object* idx = ctr_build_number_from_float(tlength);
    ctr_argument arg = {
      idx,
      NULL
    },
    arg1 = {
      NULL,
      &arg
    };
    ctr_object* tuple = ctr_array_new(CtrStdArray, &arg);
    for (int i = 0; i<tlength; i++) {
      idx->value.nvalue = i;
      arg1.object = ctr_py_from_pyobject(PyList_GetItem(pyobj, i));
      ctr_array_put(tuple, &arg1);
    }
    tuple->value.avalue->immutable = 0;
    return tuple;
  }
  if (PyDict_Check(pyobj)) {
    PyObject *key, *value;
    Py_ssize_t pos = 0;
    ctr_object* map = ctr_map_new(CtrStdMap, NULL);
    ctr_argument arg = {
      NULL,
      NULL
    },
    arg0 = {
      NULL,
      &arg
    };
    while (PyDict_Next(pyobj, &pos, &key, &value)) {
      arg.object = ctr_py_from_pyobject(key);
      arg0.object = ctr_py_from_pyobject(value);
      ctr_map_put(map, &arg0);
    }
    return map;
  }
  if (PyCallable_Check(pyobj)) {
    ctr_object* fn = ctr_internal_create_object(CTR_OBJECT_TYPE_OTNATFUNC);
    ctr_set_link_all(fn, CtrStdBlock);
    fn->value.fvalue = &ctr_py_run_pyfunc;
    Py_XINCREF(pyobj);
    ctr_internal_object_set_property(fn, ctr_py_fnpstr, ctr_py_code_create(pyobj), 0);
    return fn;
  }
  return CtrStdNil;
}

ctr_object* ctr_py_run_pyfunc(ctr_object* myself, ctr_argument* argumentList) {
  PyObject* pyobj = CTR_PY_GETOBJECT(ctr_internal_object_find_property(myself, ctr_py_fnpstr, 0));
  if (!pyobj) ctrraise("Invalid python function object", myself);
  int i;
  ctr_argument* arg = argumentList->next;
  for (i=0;arg;i++,arg=arg->next);
  arg = argumentList;
  PyObject* args = PyTuple_New(i);
  PyObject* dict = ctr_py_as_pyobject(arg->object);
  arg = arg->next;
  for (int x=0;x<i;x++,arg=arg->next) {
    PyTuple_SetItem(args, x, ctr_py_as_pyobject(arg->object));
  }
  return ctr_py_from_pyobject(PyEval_CallObjectWithKeywords(pyobj, args, dict));
}

ctr_object* ctr_py_init(ctr_object* myself, ctr_argument* _ign) {
  if (Py_IsInitialized()) ctrraise("Already initialized", myself);
  Py_InitializeEx(0);
  PyObject* mod = PyImport_AddModule("__main__");
  ctr_internal_object_set_property(myself, ctr_py_codemstr, ctr_py_code_create(mod), 0); //module
  ctr_internal_object_set_property(myself, ctr_py_codegstr, ctr_py_code_create(PyModule_GetDict(mod)), 0); //globals
  ctr_internal_object_set_property(myself, ctr_py_codelstr, ctr_py_code_create(PyDict_New()), 0); //locals
  return myself;
}
ctr_object* ctr_py_shutdown(ctr_object* myself, ctr_argument* _ign) {
  if (!Py_IsInitialized()) ctrraise("not initialized", myself);
  Py_FinalizeEx();
  return myself;
}


ctr_object* ctr_py_compile(ctr_object* myself, ctr_argument* argumentList) {
  if (!Py_IsInitialized()) ctrraise("Python interp object not initialized", myself);
  char* s = ctr_heap_allocate_cstring(ctr_internal_cast2string(argumentList->object));
  PyObject* rc = Py_CompileString(s, "citron-py-bridge", Py_file_input);
  ctr_heap_free(s);
  if (!rc||PyErr_Occurred()) {
    PyObject *pt, *pv, *ptb;
    PyErr_Fetch(&pt, &pv, &ptb);
    (*get_CtrStdFlow()) = ctr_py_from_pyobject(PyObject_Repr(pv));
    return CtrStdNil;
  }
  ctr_object* code = ctr_py_code_create(rc);
  ctr_internal_object_set_property(code, ctr_py_codeistr, myself, 0);
  return code;
}

ctr_object* ctr_py_eval(ctr_object* myself, ctr_argument* argumentList) {
  if (!Py_IsInitialized()) ctrraise("Python interp object not initialized", myself);
  char* s = ctr_heap_allocate_cstring(ctr_internal_cast2string(argumentList->object));
  PyObject* code = Py_CompileString(s, "citron-py-bridge", Py_eval_input);
  ctr_heap_free(s);
  if (!code||PyErr_Occurred()) {
    PyObject *pt, *pv, *ptb;
    PyErr_Fetch(&pt, &pv, &ptb);
    (*get_CtrStdFlow()) = ctr_py_from_pyobject(PyObject_Repr(pv));
    return CtrStdNil;
  }
  ctr_object *locs = ctr_internal_object_find_property(myself, ctr_py_codelstr, 0),
             *glob = ctr_internal_object_find_property(myself, ctr_py_codegstr, 0);

  PyObject* res  = PyEval_EvalCode(code, CTR_PY_GETOBJECT(glob), CTR_PY_GETOBJECT(locs));
  if (PyErr_Occurred()) {
    PyObject *pt, *pv, *ptb;
    PyErr_Fetch(&pt, &pv, &ptb);
    (*get_CtrStdFlow()) = ctr_py_from_pyobject(PyObject_Repr(pv));
    return CtrStdNil;
  }
  return ctr_py_from_pyobject(res);
}

ctr_object* ctr_py_exec(ctr_object* myself, ctr_argument* argumentList) {
  PyObject* code = CTR_PY_GETOBJECT(myself);
  ctr_object *intp = ctr_internal_object_find_property(myself, ctr_py_codeistr, 0),
             *locs = ctr_internal_object_find_property(intp, ctr_py_codelstr, 0),
             *glob = ctr_internal_object_find_property(intp, ctr_py_codegstr, 0);

  PyObject* res  = PyEval_EvalCode(code, CTR_PY_GETOBJECT(glob), CTR_PY_GETOBJECT(locs));
  if (PyErr_Occurred()) {
    PyObject *pt, *pv, *ptb;
    PyErr_Fetch(&pt, &pv, &ptb);
    (*get_CtrStdFlow()) = ctr_py_from_pyobject(PyObject_Repr(pv));
    return CtrStdNil;
  }
  return ctr_py_from_pyobject(res);
}

ctr_object* ctr_py_valueof(ctr_object* myself, ctr_argument* _ign) {
  PyObject* obj = CTR_PY_GETOBJECT(myself);
  return ctr_py_from_pyobject(obj);
}

ctr_object* ctr_py_as(ctr_object* myself, ctr_argument* argumentList) {
  PyObject* obj = ctr_py_as_pyobject(argumentList->object);
  return ctr_py_code_create(obj);
}

__attribute__((constructor))
void begin() {
  ctr_py_create();
  ctr_internal_create_func(ctrpy_interpobj, ctr_build_string_from_cstring("init"), &ctr_py_init);
  ctr_internal_create_func(ctrpy_interpobj, ctr_build_string_from_cstring("compile:"), &ctr_py_compile);
  ctr_internal_create_func(ctrpy_interpobj, ctr_build_string_from_cstring("eval:"), &ctr_py_eval);
  ctr_internal_create_func(ctrpy_interpobj, ctr_build_string_from_cstring("shutdown"), &ctr_py_shutdown);
  ctr_internal_create_func(ctrpy_interpobj, ctr_build_string_from_cstring("asPyObject:"), &ctr_py_as);
  ctr_internal_create_func(ctr_py_code_obj, ctr_build_string_from_cstring("value"), &ctr_py_valueof);
  ctr_internal_create_func(ctr_py_code_obj, ctr_build_string_from_cstring("exec"), &ctr_py_exec);
  ctr_internal_object_add_property(CtrStdWorld, ctr_build_string_from_cstring("Python"), ctrpy_interpobj, 0);
}
