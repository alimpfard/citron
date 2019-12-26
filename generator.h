#ifndef CTR_GENERATOR
#define CTR_GENERATOR
#include "citron.h"

#define CTR_STEP_GENNY 1
#define CTR_REPEAT_GENNY 2
#define CTR_E_OF_S_GENNY 3
#define CTR_E_OF_A_GENNY 4
#define CTR_E_OF_M_GENNY 5
#define CTR_FN_OF_GENNY 6
#define CTR_IFN_OF_GENNY 7
#define CTR_XFN_OF_GENNY 8
#define CTR_FIL_OF_GENNY 9
struct ctr_generator
{
  ctr_size seq_index;
  void *data;
  void *sequence;
  ctr_object *current;
  int finished;
};
typedef struct ctr_generator ctr_generator;

struct ctr_step_generator
{
  ctr_number current, end, step;
};
typedef struct ctr_step_generator ctr_step_generator;

struct ctr_mapping_generator
{
  unsigned int i_type;
  ctr_generator *genny;
  ctr_object *fn;
};
typedef struct ctr_mapping_generator ctr_mapping_generator;


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
ctr_object* ctr_generator_fin(ctr_object*, ctr_argument*);
ctr_object* ctr_generator_toarray(ctr_object*, ctr_argument*);
ctr_object* ctr_generator_foldl(ctr_object*, ctr_argument*);
ctr_object* ctr_generator_foldl0(ctr_object*, ctr_argument*);
ctr_object* ctr_generator_filter(ctr_object*, ctr_argument*);
ctr_object* ctr_generator_underlaying(ctr_object*, ctr_argument*);
void* ctr_generator_free(void*);

#endif /* end of include guard: CTR_GENERATOR */
