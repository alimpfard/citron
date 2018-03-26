#include "citron.h"
#include "generator.h"
#include <stdlib.h>

#define CTR_STEP_GENNY   1
#define CTR_REPEAT_GENNY 2
#define CTR_E_OF_S_GENNY 3
#define CTR_E_OF_A_GENNY 4
#define CTR_E_OF_M_GENNY 5

typedef struct {
  ctr_size seq_index;
  void* data;
  void* sequence;
  int finished;
} ctr_generator;

typedef struct {
  ctr_number current, end, step;
} ctr_step_generator;

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

ctr_object* ctr_generator_next(ctr_object* myself, ctr_argument* argumentList) {
  ctr_resource* res = myself->value.rvalue;
  ctr_generator* genny = res->ptr;
  if(!genny) {
    errd:;
    CtrStdFlow = ctr_build_string_from_cstring("#next on uninitialized generator");
    return CtrStdNil;
  }
  switch(res->type) {
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
    	memcpy(dest, (myself->value.svalue->value) + ua, ub);
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
    default: goto errd;
  }
}

ctr_object* ctr_generator_tostr(ctr_object* myself, ctr_argument* argumentList) {
  if(!myself->value.rvalue)
    return ctr_build_string_from_cstring("[UninitializedGenerator]");
    switch(myself->value.rvalue->type) {
      case CTR_REPEAT_GENNY: return ctr_build_string_from_cstring("[RepeatGenerator]");
      case CTR_E_OF_S_GENNY: return ctr_build_string_from_cstring("[StringGenerator]");
      case CTR_E_OF_A_GENNY: return ctr_build_string_from_cstring("[ArrayGenerator]");
      case CTR_STEP_GENNY  : return ctr_build_string_from_cstring("[StepGenerator]");
      case CTR_E_OF_M_GENNY: return ctr_build_string_from_cstring("[MapGenerator]");
      default: return ctr_build_string_from_cstring("[Generator]");
    }
}

void* ctr_generator_free(void* res_) {
  ctr_resource* res = res_;
  switch(res->type) {
    case CTR_REPEAT_GENNY:  /* fall through */
    case CTR_E_OF_S_GENNY:  /* fall through */
    case CTR_E_OF_A_GENNY:  ctr_heap_free(res->ptr); return NULL;
    case CTR_STEP_GENNY  :  /* fall through */
    case CTR_E_OF_M_GENNY:  ctr_heap_free(((ctr_generator*)res->ptr)->data); ctr_heap_free(res->ptr); return NULL;
  }
  return res_;
}
