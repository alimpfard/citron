#include "citron.h"
#include "generator.h"
#include <stdlib.h>

#define CTR_STEP_GENNY   1
#define CTR_REPEAT_GENNY 2
#define CTR_E_OF_S_GENNY 3
#define CTR_E_OF_A_GENNY 4
#define CTR_E_OF_M_GENNY 5
#define CTR_FN_OF_GENNY  6

typedef struct {
  ctr_size seq_index;
  void* data;
  void* sequence;
  int finished;
} ctr_generator;

typedef struct {
  ctr_number current, end, step;
} ctr_step_generator;

typedef struct {
  unsigned int i_type;
  ctr_generator* genny;
  ctr_object* fn;
} ctr_mapping_generator;

ctr_object* ctr_generator_make(ctr_object* myself, ctr_argument* argumentList) {
  ctr_object* inst = ctr_internal_create_object(CTR_OBJECT_TYPE_OTEX);
  ctr_set_link_all(inst, myself);
  inst->release_hook = ctr_generator_free;
  inst->value.rvalue = ctr_heap_allocate(sizeof(ctr_resource));
  ctr_resource* res = inst->value.rvalue;
  ctr_object* arg = argumentList->object;
  switch (arg->info.type) {
    case CTR_OBJECT_TYPE_OTNUMBER: {
      ctr_object *end, *step;
      if((end = argumentList->next->object)) {//from_to_
        res->type = CTR_STEP_GENNY;
        if((step = argumentList->next->next->object)) {//from_to_step
          ctr_generator* gen = ctr_heap_allocate(sizeof(*gen));
          gen->seq_index = 0;
          ctr_step_generator* sg = ctr_heap_allocate(sizeof(ctr_step_generator));
          sg->current = arg->value.nvalue;
          sg->end = end->value.nvalue;
          sg->step = step->value.nvalue;
          gen->data = NULL;
          gen->finished = 0;
          gen->sequence = sg;
          res->ptr = gen;
        } else {//from_to_
          ctr_generator* gen = ctr_heap_allocate(sizeof(*gen));
          gen->seq_index = 0;
          ctr_step_generator* sg = ctr_heap_allocate(sizeof(ctr_step_generator));
          sg->current = arg->value.nvalue;
          sg->end = end->value.nvalue;
          sg->step = (ctr_number)1;
          gen->data = NULL;
          gen->finished = 0;
          gen->sequence = sg;
          res->ptr = gen;
        }
      } else {//repeat
        res->type = CTR_REPEAT_GENNY;
        ctr_generator* gen = ctr_heap_allocate(sizeof(*gen));
        gen->finished = 0;
        gen->sequence = arg;
        res->ptr = gen;
      }
      break;
    }
    case CTR_OBJECT_TYPE_OTSTRING: {//elements_of
      res->type = CTR_E_OF_S_GENNY;
      ctr_generator* gen = ctr_heap_allocate(sizeof(*gen));
      gen->finished = 0;
      gen->seq_index = 0;
      gen->sequence = arg->value.svalue;
      res->ptr = gen;
      break;
    }
    case CTR_OBJECT_TYPE_OTARRAY: {//elements_of
      res->type = CTR_E_OF_A_GENNY;
      ctr_generator* gen = ctr_heap_allocate(sizeof(*gen));
      gen->finished = 0;
      gen->seq_index = 0;
      gen->sequence = arg->value.avalue;
      res->ptr = gen;
      break;
    }
    default: {//repeat | map
      if (ctr_reflect_get_primitive_link(arg) == CtrStdMap) {//map
        res->type = CTR_E_OF_M_GENNY;
        ctr_generator* gen = ctr_heap_allocate(sizeof(*gen));
        gen->finished = 0;
        gen->seq_index = 0;
        gen->sequence = arg->properties;
        res->ptr = gen;
      } else {//simply repeat
        res->type = CTR_REPEAT_GENNY;
        ctr_generator* gen = ctr_heap_allocate(sizeof(*gen));
        gen->finished = 0;
        gen->sequence = arg;
        res->ptr = gen;
      }
    }
  }
  return inst;
}

ctr_object* ctr_generator_map(ctr_object* myself, ctr_argument* argumentList) {
  ctr_object* blk = argumentList->object;
  ctr_object* inst = ctr_internal_create_object(CTR_OBJECT_TYPE_OTEX);
  ctr_set_link_all(inst, ctr_std_generator);
  ctr_generator* under = myself->value.rvalue->ptr;
  inst->release_hook = ctr_generator_free;
  inst->value.rvalue = ctr_heap_allocate(sizeof(ctr_resource));
  ctr_resource* res = inst->value.rvalue;
  res->type = CTR_FN_OF_GENNY;
  ctr_mapping_generator* genny = ctr_heap_allocate(sizeof(*genny));
  genny->i_type = myself->value.rvalue->type;
  genny->genny  = myself->value.rvalue->ptr;
  genny->fn = blk;
  ctr_generator* generator = ctr_heap_allocate(sizeof(*generator));
  generator->seq_index = under->seq_index;
  generator->sequence = genny;
  ctr_argument* argm = ctr_heap_allocate(sizeof(ctr_argument));
  argm->next = ctr_heap_allocate(sizeof(ctr_argument));
  generator->data = argm;
  generator->finished = under->finished;
  res->ptr = generator;
  return inst;
}

ctr_object* ctr_generator_internal_next(ctr_generator* genny, int gtype) {
  switch(gtype) {
    case CTR_STEP_GENNY: {
      if(genny->finished) return CtrStdNil;
      genny->seq_index++;
      ctr_step_generator* sg = genny->sequence;
      ctr_object* num = ctr_build_number_from_float(sg->current);
      sg->current += sg->step;
      if(sg->end <= sg->current) { genny->finished = 1; }
      return num;
    }
    case CTR_REPEAT_GENNY: {
      return genny->sequence;
    }
    case CTR_E_OF_S_GENNY: {
      ctr_string* str = genny->sequence;
      if(genny->finished) return CtrStdNil;
      if(genny->seq_index >= str->vlen) { genny->finished = 1; return CtrStdNil; }
    	long ua = getBytesUtf8(str->value, 0, genny->seq_index);
    	long ub = getBytesUtf8(str->value, ua, 1);
    	ctr_object *newString;
    	char *dest = ctr_heap_allocate(ub * sizeof(char));
    	memcpy(dest, (str->value) + ua, ub);
    	newString = ctr_build_string(dest, ub);
    	ctr_heap_free(dest);
      genny->seq_index++;
      return newString;
    }
    case CTR_E_OF_A_GENNY: {
      ctr_collection* coll = genny->sequence;
      if(genny->finished) return CtrStdNil;
      if(genny->seq_index >= coll->head-coll->tail) { genny->finished = 1; return CtrStdNil; }
      ctr_object* elem = coll->elements[genny->seq_index];
      genny->seq_index++;
      return elem;
    }
    case CTR_E_OF_M_GENNY: {
      ctr_map* map = genny->sequence;
      if(genny->finished) return CtrStdNil;
      if(genny->seq_index >= map->size) { genny->finished = 1; return CtrStdNil; }
      if(!genny->data) {
        genny->data = ctr_heap_allocate(sizeof(ctr_argument));
      }
      ctr_size c = genny->seq_index++;
      ctr_mapitem* head = map->head;
      while(--c > 0) {
        head = head->next;
      }
      ctr_object* tup = ctr_array_new(CtrStdArray, NULL);
      ctr_argument* argm = genny->data;
      argm->object = head->key;
      ctr_array_push(tup, argm);
      argm->object = head->value;
      ctr_array_push(tup, argm);
      tup->value.avalue->immutable = 1;
      return tup;
    }
    case CTR_FN_OF_GENNY: {
      ctr_mapping_generator* mgen = genny->sequence;
      ctr_argument* argm = genny->data;
      ctr_generator* igen = mgen->genny;
      int igen_type = mgen->i_type;
      ctr_object* fn = mgen->fn;
      argm->object = ctr_build_number_from_float(genny->seq_index++);
      argm->next->object = ctr_generator_internal_next(igen, igen_type);
      genny->finished = igen->finished;
      if(genny->finished) return CtrStdNil;
      return ctr_block_run(fn, argm, fn);
    }
    default: return NULL;
  }
}

ctr_object* ctr_generator_next(ctr_object* myself, ctr_argument* argumentList) {
  ctr_resource* res = myself->value.rvalue;
  ctr_generator* genny = res->ptr;
  int gtype = res->type;
  if(!genny) {
    CtrStdFlow = ctr_build_string_from_cstring("#next on uninitialized generator");
    return CtrStdNil;
  }
  ctr_object* next = ctr_generator_internal_next(genny, gtype);
  if(!next) {
    CtrStdFlow = ctr_build_string_from_cstring("Invalid generator type(probably)");
    return CtrStdNil;
  }
  return next;
}
ctr_object* ctr_generator_each(ctr_object* myself, ctr_argument* argumentList) {
  ctr_object* blk = argumentList->object;
  ctr_resource* res = myself->value.rvalue;
  ctr_generator* genny = res->ptr;
  int gtype = res->type;
  if(!genny) {
    CtrStdFlow = ctr_build_string_from_cstring("#next on uninitialized generator");
    return CtrStdNil;
  }
  ctr_argument* argm = ctr_heap_allocate(sizeof(*argm));
  argm->next = ctr_heap_allocate(sizeof(*argm));
  while(1) {
    ctr_object* next = ctr_generator_internal_next(genny, gtype);
    if(genny->finished) break;
    argm->object = ctr_build_number_from_float(genny->seq_index - 1);
    argm->next->object = next;
    ctr_block_run(blk, argm, blk);
    if(CtrStdFlow) {
      if(CtrStdFlow == CtrStdContinue) { CtrStdFlow = NULL; continue; }
      break;
    }
  }
  if(CtrStdFlow == CtrStdBreak) CtrStdFlow = NULL;
  ctr_heap_free(argm);
  return myself;
}
ctr_object* ctr_generator_internal_tostr(ctr_generator* gen, int gtype) {
  switch(gtype) {
    case CTR_REPEAT_GENNY: return ctr_build_string_from_cstring("[RepeatGenerator]");
    case CTR_E_OF_S_GENNY: return ctr_build_string_from_cstring("[StringGenerator]");
    case CTR_E_OF_A_GENNY: return ctr_build_string_from_cstring("[ArrayGenerator]");
    case CTR_STEP_GENNY  : return ctr_build_string_from_cstring("[StepGenerator]");
    case CTR_E_OF_M_GENNY: return ctr_build_string_from_cstring("[MapGenerator]");
    case CTR_FN_OF_GENNY: {
      ctr_object* str = ctr_build_string_from_cstring("[MappedGenerator<");
      ctr_mapping_generator* mgen = gen->sequence;
      ctr_object* inner = ctr_generator_internal_tostr(mgen->genny, mgen->i_type);
      ctr_argument* arg = gen->data;
      arg->object = inner;
      ctr_string_append(str, arg);
      arg->object = ctr_build_string_from_cstring(">]");
      ctr_string_append(str, arg);
      return str;
    }
    default: return ctr_build_string_from_cstring("[Generator]");
  }
}

ctr_object* ctr_generator_tostr(ctr_object* myself, ctr_argument* argumentList) {
  if(!myself->value.rvalue)
    return ctr_build_string_from_cstring("[UninitializedGenerator]");
  return ctr_generator_internal_tostr(myself->value.rvalue->ptr, myself->value.rvalue->type);
}

void* ctr_generator_free(void* res_) {
  ctr_resource* res = res_;
  switch(res->type) {
    case CTR_REPEAT_GENNY:  /* fall through */
    case CTR_E_OF_S_GENNY:  /* fall through */
    case CTR_E_OF_A_GENNY:  ctr_heap_free(res->ptr); return NULL;
    case CTR_STEP_GENNY  :  /* fall through */
    case CTR_E_OF_M_GENNY:  ctr_heap_free(((ctr_generator*)res->ptr)->data); ctr_heap_free(res->ptr); return NULL;
    case CTR_FN_OF_GENNY :  {
      ctr_heap_free(((ctr_generator*)res->ptr)->sequence);
      ctr_argument* argm = ((ctr_generator*)res->ptr)->data;
      ctr_heap_free(argm->next);
      ctr_heap_free(argm);
      ctr_heap_free(res->ptr);
      return NULL;
    }
  }
  return res_;
}
