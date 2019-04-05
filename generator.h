#ifndef CTR_GENERATOR
#define CTR_GENERATOR
#include "citron.h"

ctr_object* ctr_std_generator;
ctr_object* ctr_generator_make(ctr_object*, ctr_argument*);
ctr_object* ctr_generator_make_rept (ctr_object*, ctr_argument*);
ctr_object* ctr_generator_next(ctr_object*, ctr_argument*);
ctr_object* ctr_generator_inext(ctr_object*, ctr_argument*);
ctr_object* ctr_generator_each(ctr_object*, ctr_argument*);
ctr_object* ctr_generator_eachv(ctr_object*, ctr_argument*);
ctr_object* ctr_generator_ieach(ctr_object*, ctr_argument*);
ctr_object* ctr_generator_ieachv(ctr_object*, ctr_argument*);
ctr_object* ctr_generator_imap(ctr_object*, ctr_argument*);
ctr_object* ctr_generator_fmap(ctr_object*, ctr_argument*);
ctr_object* ctr_generator_ifmap(ctr_object*, ctr_argument*);
ctr_object* ctr_generator_tostr(ctr_object*, ctr_argument*);
ctr_object* ctr_generator_copy(ctr_object*, ctr_argument*);
ctr_object* ctr_generator_isfin(ctr_object*, ctr_argument*);
ctr_object* ctr_generator_toarray(ctr_object*, ctr_argument*);
ctr_object* ctr_generator_foldl(ctr_object*, ctr_argument*);
ctr_object* ctr_generator_foldl0(ctr_object*, ctr_argument*);
ctr_object* ctr_generator_filter(ctr_object*, ctr_argument*);
ctr_object* ctr_generator_underlaying(ctr_object*, ctr_argument*);
void* ctr_generator_free(void*);

#endif /* end of include guard: CTR_GENERATOR */
