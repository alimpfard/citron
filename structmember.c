#include "structmember.h"
#include <stdint.h>

#warning "Inner structs are not fully available, they are ignored in unpacks"

static int
get_double (ctr_object * o, double *p)
{				//TODO: Error handling
  double x;
  if (o->info.type != CTR_OBJECT_TYPE_OTNUMBER)
    return -1;
  x = o->value.nvalue;
  *p = x;
  return 0;
}

/* Native routines */
/* ctr_object* n[up]<type> */

ctr_object *
nuchar (char *p)
{
  return ctr_build_string (p, 1);
}

ctr_object *
nuuchar (char *p)
{
  unsigned char c = *(unsigned char *) p;
  return ctr_build_string (&c, 1);
}

ctr_object *
nubyte (char *p)
{
  return ctr_build_number_from_float ((double) *(signed char *) p);
}

ctr_object *
nuubyte (char *p)
{
  return ctr_build_number_from_float ((double) *(unsigned char *) p);
}

ctr_object *
nushort (char *p)
{
  short x;
  memcpy ((char *) &x, p, sizeof (x));
  return ctr_build_number_from_float ((double) x);
}

ctr_object *
nuushort (char *p)
{
  unsigned short x;
  memcpy ((char *) &x, p, sizeof (x));
  return ctr_build_number_from_float ((double) x);
}

ctr_object *
nuint (char *p)
{
  int x;
  memcpy ((char *) &x, p, sizeof (x));
  return ctr_build_number_from_float ((double) x);
}

ctr_object *
nuuint (char *p)
{
  unsigned int x;
  memcpy ((char *) &x, p, sizeof (x));
  return ctr_build_number_from_float ((double) x);
}

ctr_object *
nuint8 (char *p)
{
  int8_t x;
  memcpy ((char *) &x, p, sizeof (x));
  return ctr_build_number_from_float ((double) x);
}

ctr_object *
nuint16 (char *p)
{
  int16_t x;
  memcpy ((char *) &x, p, sizeof (x));
  return ctr_build_number_from_float ((double) x);
}

ctr_object *
nuint32 (char *p)
{
  int32_t x;
  memcpy ((char *) &x, p, sizeof (x));
  return ctr_build_number_from_float ((double) x);
}

ctr_object *
nuint64 (char *p)
{
  int64_t x;
  memcpy ((char *) &x, p, sizeof (x));
  return ctr_build_number_from_float ((double) x);
}

ctr_object *
nuuint8 (char *p)
{
  uint8_t x;
  memcpy ((char *) &x, p, sizeof (x));
  return ctr_build_number_from_float ((double) x);
}

ctr_object *
nuuint16 (char *p)
{
  uint16_t x;
  memcpy ((char *) &x, p, sizeof (x));
  return ctr_build_number_from_float ((double) x);
}

ctr_object *
nuuint32 (char *p)
{
  uint32_t x;
  memcpy ((char *) &x, p, sizeof (x));
  return ctr_build_number_from_float ((double) x);
}

ctr_object *
nuuint64 (char *p)
{
  uint64_t x;
  memcpy ((char *) &x, p, sizeof (x));
  return ctr_build_number_from_float ((double) x);
}

ctr_object *
nulong (char *p)
{
  long x;
  memcpy ((char *) &x, p, sizeof (x));
  return ctr_build_number_from_float ((double) x);
}

ctr_object *
nuulong (char *p)
{
  unsigned long x;
  memcpy ((char *) &x, p, sizeof (x));
  return ctr_build_number_from_float ((double) x);
}

ctr_object *
nulonglong (char *p)
{
  long long x;
  memcpy ((char *) &x, p, sizeof (x));
  return ctr_build_number_from_float ((double) x);
}

ctr_object *
nuulonglong (char *p)
{
  unsigned long long x;
  memcpy ((char *) &x, p, sizeof (x));
  return ctr_build_number_from_float ((double) x);
}

ctr_object *
nubool (char *p)
{
  _Bool x;
  memcpy ((char *) &x, p, sizeof (x));
  return ctr_build_bool (x != 0);
}

ctr_object *
nufloat (char *p)
{
  float x;
  memcpy ((char *) &x, p, sizeof (x));
  return ctr_build_number_from_float ((double) x);
}

ctr_object *
nudouble (char *p)
{
  double x;
  memcpy ((char *) &x, p, sizeof (x));
  return ctr_build_number_from_float (x);
}

ctr_object *
nupointer (char *p)
{
  void *x = ctr_heap_allocate (sizeof (void *));
  memcpy ((char *) &x, p, sizeof (x));
  ctr_object *ptr = ctr_ctypes_make_pointer (NULL, NULL);
  ptr->value.rvalue->ptr = x;
  return ptr;
}

ctr_object *
nuvoid (char *p)
{
  return ctr_build_nil ();
}

struct npdt_t
{
  ffi_type *type;
  int (*fn) (char *, ctr_object *);
};
struct nudt_t
{
  ffi_type *type;
  ctr_object *(*fn) (char *);
};

static struct npdt_t npdtable[] = {	//TODO: Fix mappings
  {&ffi_type_uchar, npuchar},
  {&ffi_type_schar, npchar},
  {&ffi_type_uint8, npuint8},
  {&ffi_type_sint8, npint8},
  {&ffi_type_uint16, npuint16},
  {&ffi_type_sint16, npint16},
  {&ffi_type_uint32, npuint32},
  {&ffi_type_sint32, npint32},
  {&ffi_type_uint64, npuint64},
  {&ffi_type_sint64, npint64},
  {&ffi_type_float, npfloat},
  {&ffi_type_double, npdouble},
  {&ffi_type_ushort, npushort},
  {&ffi_type_sshort, npshort},
  {&ffi_type_uint, npuint},
  {&ffi_type_sint, npint},
  {&ffi_type_ulong, nplong},
  {&ffi_type_slong, nplong},
  {&ffi_type_longdouble, nplonglong},
  {&ffi_type_pointer, nppointer},
  {NULL, NULL}
};

static struct nudt_t nudtable[] = {	//TODO: Fix mappings
  {&ffi_type_void, nuvoid},
  {&ffi_type_uchar, nuuchar},
  {&ffi_type_schar, nuchar},
  {&ffi_type_uint8, nuuint8},
  {&ffi_type_sint8, nuint8},
  {&ffi_type_uint16, nuuint16},
  {&ffi_type_sint16, nuint16},
  {&ffi_type_uint32, nuuint32},
  {&ffi_type_sint32, nuint32},
  {&ffi_type_uint64, nuuint64},
  {&ffi_type_sint64, nuint64},
  {&ffi_type_float, nufloat},
  {&ffi_type_double, nudouble},
  {&ffi_type_ushort, nuushort},
  {&ffi_type_sshort, nushort},
  {&ffi_type_uint, nuuint},
  {&ffi_type_sint, nuint},
  {&ffi_type_ulong, nulong},
  {&ffi_type_slong, nulong},
  {&ffi_type_longdouble, nulonglong},
  {&ffi_type_pointer, nupointer},
  {NULL, NULL}
};

struct npdt_t
get_pentry (ffi_type * type)
{
  int i = 0;
  while (npdtable[i].type != NULL)
    {
      if (npdtable[i].type == type)
	return npdtable[i];
      i++;
    }
  struct npdt_t ret = { NULL, NULL };
  return ret;
}

int
npdispatch (char *p, ctr_object * o, ffi_type * type)
{
  if (type == &ffi_type_pointer && o->info.type == CTR_OBJECT_TYPE_OTSTRING)
    {
      char *buf = ctr_heap_allocate_cstring (o);
      strncpy (p, &buf, sizeof (void *));
      return 0;
    }
  int (*fn) (char *, ctr_object *) = get_pentry (type).fn;
  if (fn != NULL)
    {
      return fn (p, o);
    }
  return -2;
}

struct nudt_t
get_uentry (ffi_type * type)
{
  int i = 0;
  while (nudtable[i].type != NULL)
    {
      if (nudtable[i].type == type)
	return nudtable[i];
      i++;
    }
  struct nudt_t ret = { NULL, NULL };
  return ret;
}

ctr_object *
nudispatch (char *p, ffi_type * type)
{
  ctr_object *(*fn) (char *) = get_uentry (type).fn;
  if (fn != NULL)
    {
      ctr_object *r = fn (p);
      if (r)
	r->info.sticky = 1;
      return r;
    }
  sttrace_print (NULL);
  return NULL;
}

int
npbyte (char *p, ctr_object * o)
{
  double x;
  if (get_double (o, &x) < 0)
    return -1;
  if (x < -128 || x > 127)
    {
      char err[1024];
      int errlen = sprintf (err, "Byte requires [-128,127] but value was %d", x);
      CtrStdFlow = ctr_build_string (err, errlen);
      return -1;
    }
  *p = (char) x;
  return 0;
}

int
npubyte (char *p, ctr_object * o)
{
  double x;
  if (get_double (o, &x) < 0)
    return -1;
  if (x < 0 || x > 255)
    {
      char err[1024];
      int errlen = sprintf (err, "unsigned byte requires [0,255] but value was %d", x);
      CtrStdFlow = ctr_build_string (err, errlen);
      return -1;
    }
  *p = (unsigned char) x;
  return 0;
}

int
npchar (char *p, ctr_object * o)
{
  if (o->info.type != CTR_OBJECT_TYPE_OTSTRING)
    return -1;
  if (o->value.svalue->vlen != 1)
    {
      char err[2048];
      int errlen =
	sprintf (err, "char requires string of length 1 but we were passed %.*s (%d chars)", o->value.svalue->vlen, o->value.svalue->value,
		 o->value.svalue->vlen);
      CtrStdFlow = ctr_build_string (err, errlen);
      return -1;
    }
  *p = *(o->value.svalue->value);
  return 0;
}

int
npuchar (char *p, ctr_object * o)
{
  if (o->info.type != CTR_OBJECT_TYPE_OTSTRING)
    return -1;
  if (o->value.svalue->vlen != 1)
    {
      char err[2048];
      int errlen =
	sprintf (err, "uchar requires string of length 1 but we were passed %.*s (%d chars)", o->value.svalue->vlen, o->value.svalue->value,
		 o->value.svalue->vlen);
      CtrStdFlow = ctr_build_string (err, errlen);
      return -1;
    }
  *((unsigned char *) p) = *((unsigned char *) o->value.svalue->value);
  return 0;
}

int
npshort (char *p, ctr_object * o)
{
  short y;
  double x;
  if (get_double (o, &x) < 0)
    return -1;
  if (x < SHRT_MIN || x > SHRT_MAX)
    {
      char err[1024];
      int errlen = sprintf (err, "short requires [%d,%d] but value was %d", SHRT_MIN, SHRT_MAX, x);
      CtrStdFlow = ctr_build_string (err, errlen);
      return -1;
    }
  y = (short) x;
  memcpy (p, (char *) &y, sizeof (y));
  return 0;
}

int
npushort (char *p, ctr_object * o)
{
  unsigned short y;
  double x;
  if (get_double (o, &x) < 0)
    return -1;
  if (x < 0 || x > USHRT_MAX)
    {
      char err[1024];
      int errlen = sprintf (err, "ushort requires [%d,%d] but value was %d", 0, USHRT_MAX, x);
      CtrStdFlow = ctr_build_string (err, errlen);
      return -1;
    }
  y = (unsigned short) x;
  memcpy (p, (char *) &y, sizeof (y));
  return 0;
}

int
npint (char *p, ctr_object * o)
{
  int y;
  double x;
  if (get_double (o, &x) < 0)
    return -1;

  //#if (sizeof(double) > sizeof(int))
  if (x < (double) INT_MIN || x > (double) INT_MAX)
    {
      char err[1024];
      int errlen = sprintf (err, "int requires [%d,%d] but value was %d", INT_MIN, INT_MAX, x);
      CtrStdFlow = ctr_build_string (err, errlen);
      return -1;
    }
  //#endif

  y = (int) x;
  memcpy (p, (char *) &y, sizeof (y));
  return 0;
}

int
npuint (char *p, ctr_object * o)
{
  unsigned int y;
  double x;
  if (get_double (o, &x) < 0)
    return -1;

  //#if (sizeof(double) > sizeof(int))
  if (x < (double) 0 || x > (double) UINT8_MAX)
    {
      char err[1024];
      int errlen = sprintf (err, "uint8 requires [%d,%d] but value was %d", 0, UINT8_MAX, x);
      CtrStdFlow = ctr_build_string (err, errlen);
      return -1;
    }
  //#endif

  y = (unsigned int) x;
  memcpy (p, (char *) &y, sizeof (y));
  return 0;
}

int
npint8 (char *p, ctr_object * o)
{
  int8_t y;
  double x;
  if (get_double (o, &x) < 0)
    return -1;

  //#if (sizeof(double) > sizeof(int))
  if (x < (double) INT8_MIN || x > (double) INT8_MAX)
    {
      char err[1024];
      int errlen = sprintf (err, "int8 requires [%d,%d] but value was %d", INT8_MIN, INT8_MAX, x);
      CtrStdFlow = ctr_build_string (err, errlen);
      return -1;
    }
  //#endif

  y = (int8_t) x;
  memcpy (p, (char *) &y, sizeof (y));
  return 0;
}

int
npuint8 (char *p, ctr_object * o)
{
  uint8_t y;
  double x;
  if (get_double (o, &x) < 0)
    return -1;

  //#if (sizeof(double) > sizeof(int))
  if (x < (double) 0 || x > (double) UINT8_MAX)
    {
      char err[1024];
      int errlen = sprintf (err, "uint requires [%d,%d] but value was %d", 0, UINT8_MAX, x);
      CtrStdFlow = ctr_build_string (err, errlen);
      return -1;
    }
  //#endif

  y = (uint8_t) x;
  memcpy (p, (char *) &y, sizeof (y));
  return 0;
}

int
npint16 (char *p, ctr_object * o)
{
  int16_t y;
  double x;
  if (get_double (o, &x) < 0)
    return -1;

  //#if (sizeof(double) > sizeof(int))
  if (x < (double) INT16_MIN || x > (double) INT16_MAX)
    {
      char err[1024];
      int errlen = sprintf (err, "int8 requires [%d,%d] but value was %d", INT16_MIN, INT16_MAX, x);
      CtrStdFlow = ctr_build_string (err, errlen);
      return -1;
    }
  //#endif

  y = (int16_t) x;
  memcpy (p, (char *) &y, sizeof (y));
  return 0;
}

int
npuint16 (char *p, ctr_object * o)
{
  uint16_t y;
  double x;
  if (get_double (o, &x) < 0)
    return -1;

  //#if (sizeof(double) > sizeof(int))
  if (x < (double) 0 || x > (double) UINT8_MAX)
    {
      char err[1024];
      int errlen = sprintf (err, "uint requires [%d,%d] but value was %d", 0, UINT16_MAX, x);
      CtrStdFlow = ctr_build_string (err, errlen);
      return -1;
    }
  //#endif

  y = (uint16_t) x;
  memcpy (p, (char *) &y, sizeof (y));
  return 0;
}

int
npint32 (char *p, ctr_object * o)
{
  int32_t y;
  double x;
  if (get_double (o, &x) < 0)
    return -1;

  //#if (sizeof(double) > sizeof(int))
  if (x < (double) INT32_MIN || x > (double) INT32_MAX)
    {
      char err[1024];
      int errlen = sprintf (err, "int8 requires [%d,%d] but value was %d", INT32_MIN, INT32_MAX, x);
      CtrStdFlow = ctr_build_string (err, errlen);
      return -1;
    }
  //#endif

  y = (int32_t) x;
  memcpy (p, (char *) &y, sizeof (y));
  return 0;
}

int
npuint32 (char *p, ctr_object * o)
{
  uint32_t y;
  double x;
  if (get_double (o, &x) < 0)
    return -1;

  //#if (sizeof(double) > sizeof(int))
  if (x < (double) 0 || x > (double) UINT8_MAX)
    {
      char err[1024];
      int errlen = sprintf (err, "uint requires [%d,%d] but value was %d", 0, UINT32_MAX, x);
      CtrStdFlow = ctr_build_string (err, errlen);
      return -1;
    }
  //#endif

  y = (uint32_t) x;
  memcpy (p, (char *) &y, sizeof (y));
  return 0;
}

int
npint64 (char *p, ctr_object * o)
{
  int64_t y;
  double x;
  if (get_double (o, &x) < 0)
    return -1;

  //#if (sizeof(double) > sizeof(int))
  if (x < (double) INT64_MIN || x > (double) INT64_MAX)
    {
      char err[1024];
      int errlen = sprintf (err, "int8 requires [%d,%d] but value was %d", INT64_MIN, INT64_MAX, x);
      CtrStdFlow = ctr_build_string (err, errlen);
      return -1;
    }
  //#endif

  y = (int64_t) x;
  memcpy (p, (char *) &y, sizeof (y));
  return 0;
}

int
npuint64 (char *p, ctr_object * o)
{
  uint64_t y;
  double x;
  if (get_double (o, &x) < 0)
    return -1;

  //#if (sizeof(double) > sizeof(int))
  if (x < (double) 0 || x > (double) UINT64_MAX)
    {
      char err[1024];
      int errlen = sprintf (err, "uint requires [%d,%d] but value was %d", 0, UINT8_MAX, x);
      CtrStdFlow = ctr_build_string (err, errlen);
      return -1;
    }
  //#endif

  y = (uint64_t) x;
  memcpy (p, (char *) &y, sizeof (y));
  return 0;
}

int
nplong (char *p, ctr_object * o)
{
  long y;
  double x;
  if (get_double (o, &x) < 0)
    return -1;
  if (x < (double) LONG_MIN || x > (double) LONG_MAX)
    {
      char err[1024];
      int errlen = sprintf (err, "long requires [%d,%d] but value was %d", LONG_MIN, LONG_MAX, x);
      CtrStdFlow = ctr_build_string (err, errlen);
      return -1;
    }
  y = (long) x;
  memcpy (p, (char *) &y, sizeof (y));
  return 0;
}

int
nplonglong (char *p, ctr_object * o)
{
  long long y;
  double x;
  if (get_double (o, &x) < 0)
    return -1;
  if (x < (double) LLONG_MIN || x > (double) LLONG_MAX)
    {
      char err[1024];
      int errlen = sprintf (err, "long long requires [%d,%d] but value was %d", LLONG_MIN, LLONG_MAX, x);
      CtrStdFlow = ctr_build_string (err, errlen);
      return -1;
    }
  y = (long long) x;
  memcpy (p, (char *) &y, sizeof (y));
  return 0;
}

int
npbool (char *p, ctr_object * o)
{
  _Bool x;
  x = ctr_internal_cast2bool (o)->value.bvalue != 0;
  memcpy (p, (char *) &x, sizeof (x));
  return 0;
}

int
npfloat (char *p, ctr_object * o)
{
  float y;
  double x;
  if (get_double (o, &x) < 0)
    return -1;
  if (x < (double) FLT_MIN || x > (double) FLT_MAX)
    {
      char err[1024];
      int errlen = sprintf (err, "uint requires [%d,%d] but value was %d", FLT_MIN, FLT_MAX, x);
      CtrStdFlow = ctr_build_string (err, errlen);
      return -1;
    }
  y = (float) x;
  memcpy (p, (char *) &y, sizeof (y));
  return 0;
}

int
npdouble (char *p, ctr_object * o)
{
  double x;
  if (get_double (o, &x) < 0)
    return -1;
  memcpy (p, (char *) &x, sizeof (x));
  return 0;
}

int
nppointer (char *p, ctr_object * o)
{
  void *y;
  if (o->info.type != CTR_OBJECT_TYPE_OTEX)
    y = o;
  else if (o->value.rvalue->type == CTR_CTYPE_STRUCT)
    y = ((ctr_ctypes_ffi_struct_value *) (o->value.rvalue->ptr))->value;
  else
    y = o->value.rvalue->ptr;
  memcpy (p, &y, sizeof (void *));
  return 0;
}
