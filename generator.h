#ifndef CTR_GENERATOR
#define CTR_GENERATOR
#include "citron.h"

ctr_object* ctr_std_generator;
ctr_object* ctr_generator_make(ctr_object*, ctr_argument*);
ctr_object* ctr_generator_next(ctr_object*, ctr_argument*);
ctr_object* ctr_generator_inext(ctr_object*, ctr_argument*);
ctr_object* ctr_generator_each(ctr_object*, ctr_argument*);
ctr_object* ctr_generator_ieach(ctr_object*, ctr_argument*);
ctr_object* ctr_generator_imap(ctr_object*, ctr_argument*);
ctr_object* ctr_generator_fmap(ctr_object*, ctr_argument*);
ctr_object* ctr_generator_tostr(ctr_object*, ctr_argument*);
ctr_object* ctr_generator_copy(ctr_object*, ctr_argument*);
ctr_object* ctr_generator_isfin(ctr_object*, ctr_argument*);
ctr_object* ctr_generator_toarray(ctr_object*, ctr_argument*);
void* ctr_generator_free(void*);

#endif /* end of include guard: CTR_GENERATOR */
