#ifndef CTR_STRUCTMEMBER_H
#define CTR_STRUCTMEMBER_H

#include <limits.h>
#include <float.h>
#include <stdio.h>
#include "citron.h"
#include "ctypes.h"
#include "reverse_sizes.h"

ctr_object* nuchar(char* p);
ctr_object* nuuchar(char* p);
ctr_object* nubyte(char* p);
ctr_object* nuubyte(char* p);
ctr_object* nushort(char* p);
ctr_object* nuushort(char* p);
ctr_object* nuint(char* p);
ctr_object* nuuint(char* p);
ctr_object* nuint8(char* p);
ctr_object* nuuint8(char* p);
ctr_object* nuint16(char* p);
ctr_object* nuuint16(char* p);
ctr_object* nuint32(char* p);
ctr_object* nuuint32(char* p);
ctr_object* nuint64(char* p);
ctr_object* nuuint64(char* p);
ctr_object* nulong(char* p);
ctr_object* nuulong(char* p);
ctr_object* nulonglong(char* p);
ctr_object* nuulonglong(char* p);
ctr_object* nubool(char* p);
ctr_object* nufloat(char* p);
ctr_object* nudouble(char* p);
ctr_object* nupointer(char* p);
int npdispatch(char* p, ctr_object* o, ffi_type* type);
ctr_object* nudispatch(char* p, ffi_type* type);
int npbyte(char* p, ctr_object* o);
int npubyte(char* p, ctr_object* o);
int npchar(char* p, ctr_object* o);
int npuchar(char* p, ctr_object* o);
int npshort(char* p, ctr_object* o);
int npushort(char* p, ctr_object* o);
int npint(char* p, ctr_object* o);
int npuint(char* p, ctr_object* o);
int npint8(char* p, ctr_object* o);
int npuint8(char* p, ctr_object* o);
int npint16(char* p, ctr_object* o);
int npuint16(char* p, ctr_object* o);
int npint32(char* p, ctr_object* o);
int npuint32(char* p, ctr_object* o);
int npint64(char* p, ctr_object* o);
int npuint64(char* p, ctr_object* o);
int nplong(char* p, ctr_object* o);
int nplonglong(char* p, ctr_object* o);
int npbool(char* p, ctr_object* o);
int npfloat(char* p, ctr_object* o);
int npdouble(char* p, ctr_object* o);
int nppointer(char* p, ctr_object* o);

#endif /* end of include guard: CTR_STRUCTMEMBER_H */
