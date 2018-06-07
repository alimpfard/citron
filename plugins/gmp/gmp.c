#define DEBUG

#ifdef existing
#include <Citron/citron.h>
#else
#warning "We don't have Citron installed"
#include "../../citron.h"
#endif
#include <gmp.h>

#ifdef DEBUG
#include <stdio.h>
#endif

ctr_object* CtrStdBigInt;

ctr_object* ctr_gmp_make(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_gmp_to_gmp(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_gmp_add(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_gmp_sub(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_gmp_mul(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_gmp_div(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_gmp_pow(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_gmp_mod(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_gmp_lt(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_gmp_gt(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_gmp_eq(ctr_object* myself, ctr_argument* argumentList);
//ctr_object* ctr_gmp_(ctr_object* myself, ctr_argument* argumentList);

ctr_object* ctr_gmp_make(ctr_object* myself, ctr_argument* argumentList) {
  mpz_t* num = ctr_heap_allocate(sizeof(mpz_t));
  int number;
  if (argumentList != NULL && argumentList->object != NULL)
    number = ctr_internal_cast2number(argumentList->object)->value.nvalue;
  else
  number = 0;
  mpz_init(*num);
  mpz_set_ui(*num, number);
  ctr_object* numobj = ctr_internal_create_object(CTR_OBJECT_TYPE_OTEX);
  ctr_set_link_all(numobj, CtrStdBigInt);
  numobj->value.rvalue = ctr_heap_allocate_tracked(sizeof(ctr_resource));
  numobj->value.rvalue->ptr = (void*)num;
  return numobj;
}
ctr_object* ctr_gmp_to_gmp(ctr_object* myself, ctr_argument* argumentList) { //Number to BigInt
  ctr_object* gmpobj = ctr_internal_create_object(CTR_OBJECT_TYPE_OTEX);
  unsigned long int number = ctr_internal_cast2number(myself)->value.nvalue;
  ctr_set_link_all(gmpobj, CtrStdBigInt);
  gmpobj->value.rvalue = ctr_heap_allocate_tracked(sizeof(ctr_resource));
  gmpobj->value.rvalue->ptr = (void*)ctr_heap_allocate_tracked(sizeof(mpz_t));
  mpz_init_set_ui(*(mpz_t*)(gmpobj->value.rvalue->ptr), number);
  return gmpobj;
}

ctr_object* ctr_gmp_add(ctr_object* myself, ctr_argument* argumentList) {
  switch(argumentList->object->info.type) {
    case CTR_OBJECT_TYPE_OTEX: {
      mpz_add((*(mpz_t*)(myself->value.rvalue->ptr)), (*(mpz_t*)(myself->value.rvalue->ptr)), (*(mpz_t*)(argumentList->object->value.rvalue->ptr)));
      return myself;
    }
    default: {
      mpz_add_ui((*(mpz_t*)(myself->value.rvalue->ptr)), (*(mpz_t*)(myself->value.rvalue->ptr)), (ctr_internal_cast2number(argumentList->object)->value.nvalue));
      return myself;
    }
  }
  return myself;
}
ctr_object* ctr_gmp_sub(ctr_object* myself, ctr_argument* argumentList) {
  switch(argumentList->object->info.type) {
    case CTR_OBJECT_TYPE_OTEX: {
      mpz_sub((*(mpz_t*)(myself->value.rvalue->ptr)), (*(mpz_t*)(myself->value.rvalue->ptr)), (*(mpz_t*)(argumentList->object->value.rvalue->ptr)));
      return myself;
    }
    default: {
      mpz_sub_ui((*(mpz_t*)(myself->value.rvalue->ptr)), (*(mpz_t*)(myself->value.rvalue->ptr)), (ctr_internal_cast2number(argumentList->object)->value.nvalue));
      return myself;
    }
  }
  return myself;
}
ctr_object* ctr_gmp_mul(ctr_object* myself, ctr_argument* argumentList) {
  switch(argumentList->object->info.type) {
    case CTR_OBJECT_TYPE_OTEX: {
      mpz_mul((*(mpz_t*)(myself->value.rvalue->ptr)), (*(mpz_t*)(myself->value.rvalue->ptr)), (*(mpz_t*)(argumentList->object->value.rvalue->ptr)));
      return myself;
    }
    default: {
      mpz_mul_ui((*(mpz_t*)(myself->value.rvalue->ptr)), (*(mpz_t*)(myself->value.rvalue->ptr)), (ctr_internal_cast2number(argumentList->object)->value.nvalue));
      return myself;
    }
  }
  return myself;
}

ctr_object* ctr_gmp_nadd(ctr_object* myself, ctr_argument* argumentList) {
  ctr_object* res = ctr_gmp_make(NULL,NULL);
  switch(argumentList->object->info.type) {
    case CTR_OBJECT_TYPE_OTEX: {
      mpz_add((*(mpz_t*)(res->value.rvalue->ptr)), (*(mpz_t*)(myself->value.rvalue->ptr)), (*(mpz_t*)(argumentList->object->value.rvalue->ptr)));
      return res;
    }
    default: {
      mpz_add_ui((*(mpz_t*)(res->value.rvalue->ptr)), (*(mpz_t*)(myself->value.rvalue->ptr)), (ctr_internal_cast2number(argumentList->object)->value.nvalue));
      return res;
    }
  }
  return res;
}
ctr_object* ctr_gmp_nsub(ctr_object* myself, ctr_argument* argumentList) {
  ctr_object* res = ctr_gmp_make(NULL,NULL);
  switch(argumentList->object->info.type) {
    case CTR_OBJECT_TYPE_OTEX: {
      mpz_sub((*(mpz_t*)(res->value.rvalue->ptr)), (*(mpz_t*)(myself->value.rvalue->ptr)), (*(mpz_t*)(argumentList->object->value.rvalue->ptr)));
      return res;
    }
    default: {
      mpz_sub_ui((*(mpz_t*)(res->value.rvalue->ptr)), (*(mpz_t*)(myself->value.rvalue->ptr)), (ctr_internal_cast2number(argumentList->object)->value.nvalue));
      return res;
    }
  }
  return res;
}
ctr_object* ctr_gmp_nmul(ctr_object* myself, ctr_argument* argumentList) {
  ctr_object* res = ctr_gmp_make(NULL,NULL);
  switch(argumentList->object->info.type) {
    case CTR_OBJECT_TYPE_OTEX: {
      mpz_mul((*(mpz_t*)(res->value.rvalue->ptr)), (*(mpz_t*)(myself->value.rvalue->ptr)), (*(mpz_t*)(argumentList->object->value.rvalue->ptr)));
      return res;
    }
    default: {
      mpz_mul_ui((*(mpz_t*)(res->value.rvalue->ptr)), (*(mpz_t*)(myself->value.rvalue->ptr)), (ctr_internal_cast2number(argumentList->object)->value.nvalue));
      return res;
    }
  }
  return res;
}

ctr_object* ctr_gmp_to_string(ctr_object* myself, ctr_argument* argumentList) {
  if (myself->value.rvalue == NULL || myself->value.rvalue->ptr == NULL) return ctr_build_string_from_cstring("[Uninitialized Big-Integer]");
  char* str = mpz_get_str(NULL, 10/*(int)(ctr_internal_cast2string(argumentList->object)->value.nvalue)*/, *(mpz_t*)(myself->value.rvalue->ptr));
  return ctr_build_string_from_cstring(str);
}

ctr_object* ctr_gmp_div(ctr_object* myself, ctr_argument* argumentList) { //will return floor, get quotient with mod
  ctr_object* div_ = ctr_gmp_make(NULL,NULL);
  switch(argumentList->object->info.type) {
    case CTR_OBJECT_TYPE_OTEX: {
      mpz_fdiv_q((*(mpz_t*)(div_->value.rvalue->ptr)), (*(mpz_t*)(myself->value.rvalue->ptr)), (*(mpz_t*)(argumentList->object->value.rvalue->ptr)));
      return div_;
    }
    default: {
      mpz_fdiv_q_ui((*(mpz_t*)(div_->value.rvalue->ptr)), (*(mpz_t*)(myself->value.rvalue->ptr)), (ctr_internal_cast2number(argumentList->object)->value.nvalue));
      return div_;
    }
  }
  return div_;
}
ctr_object* ctr_gmp_mod(ctr_object* myself, ctr_argument* argumentList) {
  ctr_object* mod_ = ctr_gmp_make(NULL,NULL);
  switch(argumentList->object->info.type) {
    case CTR_OBJECT_TYPE_OTEX: {
      mpz_fdiv_r((*(mpz_t*)(mod_->value.rvalue->ptr)), (*(mpz_t*)(myself->value.rvalue->ptr)), (*(mpz_t*)(argumentList->object->value.rvalue->ptr)));
      return mod_;
    }
    default: {
      mpz_fdiv_r_ui((*(mpz_t*)(mod_->value.rvalue->ptr)), (*(mpz_t*)(myself->value.rvalue->ptr)), (ctr_internal_cast2number(argumentList->object)->value.nvalue));
      return mod_;
    }
  }
  return mod_;
}

int ctr_gmp__compare(ctr_object* one, ctr_object* other) {
  switch(other->info.type) {
    case CTR_OBJECT_TYPE_OTEX: {
      return mpz_cmp((*(mpz_t*)(one->value.rvalue->ptr)), (*(mpz_t*)(other->value.rvalue->ptr)));

    }
    default: {
      return mpz_cmp_d((*(mpz_t*)(one->value.rvalue->ptr)), (double)(other->value.nvalue));
    }
  }
  return 0; //dud
}

// ctr_object* ctr_gmp_pow(ctr_object* myself, ctr_argument* argumentList) {
//
// }

ctr_object* ctr_gmp_lt(ctr_object* myself, ctr_argument* argumentList) {
  return ctr_build_bool(ctr_gmp__compare(myself, ctr_internal_cast2number(argumentList->object))<0?1:0);
}
ctr_object* ctr_gmp_gt(ctr_object* myself, ctr_argument* argumentList) {
  return ctr_build_bool(ctr_gmp__compare(myself, ctr_internal_cast2number(argumentList->object))>0?1:0);
}
ctr_object* ctr_gmp_eq(ctr_object* myself, ctr_argument* argumentList) {
  return ctr_build_bool(ctr_gmp__compare(myself, ctr_internal_cast2number(argumentList->object))==0?1:0);
}
ctr_object* ctr_gmp_lte(ctr_object* myself, ctr_argument* argumentList) {
  return ctr_build_bool(ctr_gmp__compare(myself, ctr_internal_cast2number(argumentList->object))<=0?1:0);
}
ctr_object* ctr_gmp_gte(ctr_object* myself, ctr_argument* argumentList) {
  return ctr_build_bool(ctr_gmp__compare(myself, ctr_internal_cast2number(argumentList->object))>=0?1:0);
}

ctr_object* ctr_gmp_times(ctr_object* myself, ctr_argument* argumentList) {
  ctr_object* blk = argumentList->object;
  if (blk == NULL || blk->info.type != CTR_OBJECT_TYPE_OTBLOCK) {
    CtrStdFlow = ctr_build_string_from_cstring("Expected code block.");
    return myself;
  }
  ctr_object* indexNumber = ctr_gmp_make(NULL,NULL);
  ctr_argument* arguments = ctr_heap_allocate( sizeof( ctr_argument ) );
  arguments->object = myself;
  ctr_gmp_add(indexNumber, arguments);
  blk->info.sticky = 1;
  while (ctr_gmp__compare(indexNumber, ctr_build_number_from_float(0))>0) {
    arguments->object = ctr_build_number_from_float(1);
    ctr_gmp_sub(indexNumber, arguments);
    arguments->object = indexNumber;
    ctr_block_run(blk, arguments, NULL);
    if (CtrStdFlow == CtrStdContinue) {CtrStdFlow = NULL; arguments->object=ctr_build_number_from_float(1); ctr_gmp_sub(indexNumber, arguments); continue;}
    if (CtrStdFlow) break;
  }
  ctr_heap_free( arguments );
  if (CtrStdFlow == CtrStdBreak) CtrStdFlow = NULL; /* consume break */
  blk->info.mark = 0;
  blk->info.sticky = 0;
  return myself;
}


void begin() {
  //mp_set_memory_functions(&ctr_heap_allocate, &ctr_heap_reallocate, &ctr_heap_free);
  CtrStdBigInt = ctr_internal_create_object(CTR_OBJECT_TYPE_OTOBJECT);
  ctr_set_link_all(CtrStdBigInt, CtrStdObject);
  ctr_internal_create_func(CtrStdNumber, ctr_build_string_from_cstring("toBigInt"), &ctr_gmp_to_gmp);
  ctr_internal_create_func(CtrStdBigInt, ctr_build_string_from_cstring("+=:"), &ctr_gmp_add);
  ctr_internal_create_func(CtrStdBigInt, ctr_build_string_from_cstring("-=:"), &ctr_gmp_sub);
  ctr_internal_create_func(CtrStdBigInt, ctr_build_string_from_cstring("*=:"), &ctr_gmp_mul);

  ctr_internal_create_func(CtrStdBigInt, ctr_build_string_from_cstring("+"), &ctr_gmp_nadd);
  ctr_internal_create_func(CtrStdBigInt, ctr_build_string_from_cstring("-"), &ctr_gmp_nsub);
  ctr_internal_create_func(CtrStdBigInt, ctr_build_string_from_cstring("*"), &ctr_gmp_nmul);

  ctr_internal_create_func(CtrStdBigInt, ctr_build_string_from_cstring("div:"), &ctr_gmp_div);
  ctr_internal_create_func(CtrStdBigInt, ctr_build_string_from_cstring("mod:"), &ctr_gmp_mod);

  ctr_internal_create_func(CtrStdBigInt, ctr_build_string_from_cstring(">"), &ctr_gmp_gt);
  ctr_internal_create_func(CtrStdBigInt, ctr_build_string_from_cstring("<"), &ctr_gmp_lt);
  ctr_internal_create_func(CtrStdBigInt, ctr_build_string_from_cstring("="), &ctr_gmp_eq);
  ctr_internal_create_func(CtrStdBigInt, ctr_build_string_from_cstring(">=:"), &ctr_gmp_gte);
  ctr_internal_create_func(CtrStdBigInt, ctr_build_string_from_cstring("<=:"), &ctr_gmp_lte);
  ctr_internal_create_func(CtrStdBigInt, ctr_build_string_from_cstring("times:"), &ctr_gmp_times);
  ctr_internal_create_func(CtrStdBigInt, ctr_build_string_from_cstring("toString"), &ctr_gmp_to_string);
  ctr_internal_object_add_property(CtrStdWorld, ctr_build_string_from_cstring("BigInteger"), CtrStdBigInt, 0);
}
