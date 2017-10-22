#ifndef CTR_ENSURE_TYPE_H_GUARD
#define CTR_ENSURE_TYPE_H_GUARD

#define NEW_TYPE_TYPE(TYPE) {TYPE, #TYPE}
typedef struct {
  int typecode;
  const char* typestring;
} typeType_t;


static const typeType_t typeType[] = {
	NEW_TYPE_TYPE(CTR_OBJECT_TYPE_OTNIL),
	NEW_TYPE_TYPE(CTR_OBJECT_TYPE_OTBOOL),
	NEW_TYPE_TYPE(CTR_OBJECT_TYPE_OTNUMBER),
	NEW_TYPE_TYPE(CTR_OBJECT_TYPE_OTSTRING),
	NEW_TYPE_TYPE(CTR_OBJECT_TYPE_OTBLOCK),
	NEW_TYPE_TYPE(CTR_OBJECT_TYPE_OTOBJECT),
	NEW_TYPE_TYPE(CTR_OBJECT_TYPE_OTNATFUNC),
	NEW_TYPE_TYPE(CTR_OBJECT_TYPE_OTARRAY),
	NEW_TYPE_TYPE(CTR_OBJECT_TYPE_OTMISC),
	NEW_TYPE_TYPE(CTR_OBJECT_TYPE_OTEX)
};

#define CTR_TYPE_ERR_0(type) "No direct conversion to type " #type ", nor any implicit cast to it arises from the use of type "

#define CTR_ENSURE_TYPE_NUMBER(object)    do { if ( object->info.type != CTR_OBJECT_TYPE_OTNUMBER ) {CtrStdFlow = ctr_build_string_from_cstring(strcat(CTR_TYPE_ERR_0(Number), typeType[object->info.type].typestring)); return CtrStdNil;} } while (0)
#define CTR_ENSURE_TYPE_NIL(object)    do { if ( object->info.type != CTR_OBJECT_TYPE_OTNIL ) {CtrStdFlow = ctr_build_string_from_cstring(strcat(CTR_TYPE_ERR_0(Nil), typeType[object->info.type].typestring)); return CtrStdNil;} } while (0)
#define CTR_ENSURE_TYPE_ARRAY(object)    do { if ( object->info.type != CTR_OBJECT_TYPE_OTARRAY ) {CtrStdFlow = ctr_build_string_from_cstring(strcat(CTR_TYPE_ERR_0(Array), typeType[object->info.type].typestring)); return CtrStdNil;} } while (0)
#define CTR_ENSURE_TYPE_STRING(object)    do { if ( object->info.type != CTR_OBJECT_TYPE_OTSTRING ) {CtrStdFlow = ctr_build_string_from_cstring(strcat(CTR_TYPE_ERR_0(String), typeType[object->info.type].typestring)); return CtrStdNil;} } while (0)
#define CTR_ENSURE_TYPE_BOOL(object)    do { if ( object->info.type != CTR_OBJECT_TYPE_OTBOOL ) {CtrStdFlow = ctr_build_string_from_cstring(strcat(CTR_TYPE_ERR_0(Boolean), typeType[object->info.type].typestring)); return CtrStdNil;} } while (0)
#define CTR_ENSURE_TYPE_BLOCK(object)    do { if ( object->info.type != CTR_OBJECT_TYPE_OTBLOCK ) {CtrStdFlow = ctr_build_string_from_cstring(strcat(CTR_TYPE_ERR_0(CodeBlock), typeType[object->info.type].typestring)); return CtrStdNil;} } while (0)
#define CTR_ENSURE_TYPE_NATFUNC(object)    do { if ( object->info.type != CTR_OBJECT_TYPE_OTNATFUNC ) {CtrStdFlow = ctr_build_string_from_cstring(strcat(CTR_TYPE_ERR_0(NativeFunction), typeType[object->info.type].typestring)); return CtrStdNil;} } while (0)
#define CTR_ENSURE_TYPE_EXTERN(object)    do { if ( object->info.type != CTR_OBJECT_TYPE_OTEX ) {CtrStdFlow = ctr_build_string_from_cstring(strcat(CTR_TYPE_ERR_0(ExternalResource), typeType[object->info.type].typestring)); return CtrStdNil;} } while (0)
#define CTR_ENSURE_TYPE_OBJECT(object)    do { if ( object->info.type != CTR_OBJECT_TYPE_OTOBJECT ) {CtrStdFlow = ctr_build_string_from_cstring(strcat(CTR_TYPE_ERR_0(object), typeType[object->info.type].typestring)); return CtrStdNil;} } while (0)
#define CTR_ENSURE_TYPE_MISC(object)    do { if ( object->info.type != CTR_OBJECT_TYPE_OTMISC ) {CtrStdFlow = ctr_build_string_from_cstring(strcat(CTR_TYPE_ERR_0(Miscellaneous), typeType[object->info.type].typestring)); return CtrStdNil;} } while (0)

#endif
