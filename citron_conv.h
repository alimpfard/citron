#ifndef CTR_CONV_H
#define CTR_CONV_H

#define CTR_MAKE_INTERMEDIATE(type) ctr_internal_create_standalone_object(CTR_OBJECT_TYPE_OT##type)
#define CTR_CREATE_INTERMEDIATE(name, type) name = CTR_MAKE_INTERMEDIATE(type)
#define CTR_DEFINE_INTERMEDIATE(name, type) ctr_object* name = CTR_MAKE_INTERMEDIATE(type)
#define CTR_DELETE_INTERMEDIATE(name) ctr_internal_delete_standalone_object(name)

#endif /* end of include guard: CTR_CONV_H */
