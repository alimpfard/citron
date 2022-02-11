#ifndef CTR_ENSURE_TYPE_H_GUARD
#define CTR_ENSURE_TYPE_H_GUARD
#ifdef CTR_H_GUARD

#    define NEW_TYPE_TYPE(TYPE, TYPENAME) \
        {                                 \
            TYPE, #TYPENAME               \
        }
typedef struct {
    int typecode;
    const char* typestring;
} typeType_t;

static const typeType_t typeType[] = {
    NEW_TYPE_TYPE(CTR_OBJECT_TYPE_OTNIL, Nil),
    NEW_TYPE_TYPE(CTR_OBJECT_TYPE_OTBOOL, Bool),
    NEW_TYPE_TYPE(CTR_OBJECT_TYPE_OTNUMBER, Number),
    NEW_TYPE_TYPE(CTR_OBJECT_TYPE_OTSTRING, String),
    NEW_TYPE_TYPE(CTR_OBJECT_TYPE_OTBLOCK, Block),
    NEW_TYPE_TYPE(CTR_OBJECT_TYPE_OTOBJECT, Object),
    NEW_TYPE_TYPE(CTR_OBJECT_TYPE_OTNATFUNC, NativeFunction),
    NEW_TYPE_TYPE(CTR_OBJECT_TYPE_OTARRAY, Array),
    NEW_TYPE_TYPE(CTR_OBJECT_TYPE_OTMISC, Miscellaneous),
    NEW_TYPE_TYPE(CTR_OBJECT_TYPE_OTEX, ExternalResource)
};

static inline ctr_object* ctr_can_cast_to_string(ctr_object* object)
{
    ctr_object* out = ctr_internal_cast2string(object);
    if (CtrStdFlow) {
        CtrStdFlow = NULL;
        out = NULL;
    }
    return out;
}
static inline ctr_object* ctr_can_cast_to_number(ctr_object* object)
{
    ctr_object* out = ctr_internal_cast2number(object);
    if (CtrStdFlow) {
        CtrStdFlow = NULL;
        out = NULL;
    }
    return out;
}
static inline ctr_object* ctr_can_cast_to_bool(ctr_object* object)
{
    ctr_object* out = ctr_internal_cast2bool(object);
    if (CtrStdFlow) {
        CtrStdFlow = NULL;
        out = NULL;
    }
    return out;
}

#    define CTR_TYPE_ERR_0(type)               \
        ("No direct conversion to type " #type \
         ", nor any implicit cast to it arises from the use of type ")

#    define CTR_TYPE_ERR(object, _type)                                       \
        do {                                                                  \
            char CTR_TYPE_ERR_STR[1000] = CTR_TYPE_ERR_0(_type);              \
            strcat(CTR_TYPE_ERR_STR, typeType[object->info.type].typestring); \
            CtrStdFlow = ctr_build_string_from_cstring(CTR_TYPE_ERR_STR);     \
            return CtrStdNil;                                                 \
        } while (0)
#    define CTR_ERR(message)                                     \
        do {                                                     \
            CtrStdFlow = ctr_build_string_from_cstring(message); \
            return CtrStdNil;                                    \
        } while (0)

#    define CTR_ENSURE_NON_NULL(object)                           \
        do {                                                      \
            if (object == NULL) {                                 \
                CtrStdFlow = ctr_build_string_from_cstring(       \
                    "Object was null when we expected nonnull."); \
                return CtrStdFlow;                                \
            }                                                     \
        } while (0)

#    define CTR_ENSURE_TYPE_NUMBER(object)                                                               \
        do {                                                                                             \
            CTR_ENSURE_NON_NULL(object);                                                                 \
            if (object->info.type != CTR_OBJECT_TYPE_OTNUMBER && ctr_can_cast_to_number(object) == NULL) \
                CTR_TYPE_ERR(object, Number);                                                            \
        } while (0)
#    define CTR_ENSURE_TYPE_NIL(object)                     \
        do {                                                \
            CTR_ENSURE_NON_NULL(object);                    \
            if (object->info.type != CTR_OBJECT_TYPE_OTNIL) \
                CTR_TYPE_ERR(object, Nil);                  \
        } while (0)
#    define CTR_ENSURE_TYPE_ARRAY(object)                     \
        do {                                                  \
            CTR_ENSURE_NON_NULL(object);                      \
            if (object->info.type != CTR_OBJECT_TYPE_OTARRAY) \
                CTR_TYPE_ERR(object, Array);                  \
        } while (0)
#    define CTR_ENSURE_TYPE_STRING(object)                                                               \
        do {                                                                                             \
            CTR_ENSURE_NON_NULL(object);                                                                 \
            if (object->info.type != CTR_OBJECT_TYPE_OTSTRING && ctr_can_cast_to_string(object) == NULL) \
                CTR_TYPE_ERR(object, String);                                                            \
        } while (0)
#    define CTR_ENSURE_TYPE_BOOL(object)                                                                                                       \
        do {                                                                                                                                   \
            CTR_ENSURE_NON_NULL(object);                                                                                                       \
            if (object->info.type != CTR_OBJECT_TYPE_OTBOOL && ctr_can_cast_to_bool(object) == NULL && ctr_can_cast_to_number(object) != NULL) \
                CTR_TYPE_ERR(object, Boolean);                                                                                                 \
        } while (0)
#    define CTR_ENSURE_TYPE_BLOCK(object)                                                                       \
        do {                                                                                                    \
            CTR_ENSURE_NON_NULL(object);                                                                        \
            if (object->info.type != CTR_OBJECT_TYPE_OTBLOCK && object->info.type != CTR_OBJECT_TYPE_OTNATFUNC) \
                CTR_TYPE_ERR(object, CodeBlock);                                                                \
        } while (0)
#    define CTR_ENSURE_TYPE_NATFUNC(object)                     \
        do {                                                    \
            CTR_ENSURE_NON_NULL(object);                        \
            if (object->info.type != CTR_OBJECT_TYPE_OTNATFUNC) \
                CTR_TYPE_ERR(object, NativeFunction);           \
        } while (0)
#    define CTR_ENSURE_TYPE_EXTERN(object)                 \
        do {                                               \
            CTR_ENSURE_NON_NULL(object);                   \
            if (object->info.type != CTR_OBJECT_TYPE_OTEX) \
                CTR_TYPE_ERR(object, ExternalResource);    \
        } while (0)
#    define CTR_ENSURE_TYPE_OBJECT(object)                     \
        do {                                                   \
            CTR_ENSURE_NON_NULL(object);                       \
            if (object->info.type != CTR_OBJECT_TYPE_OTOBJECT) \
                CTR_TYPE_ERR(object, Object);                  \
        } while (0)
#    define CTR_ENSURE_TYPE_MISC(object)                     \
        do {                                                 \
            CTR_ENSURE_NON_NULL(object);                     \
            if (object->info.type != CTR_OBJECT_TYPE_OTMISC) \
                CTR_TYPE_ERR(object, Miscellaneous);         \
        } while (0)

#endif
#endif
