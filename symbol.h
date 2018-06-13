#ifndef CTR_C_SYMBOL
#define CTR_C_SYMBOL

#include "citron.h"

inline ctr_object* ctr_create_symbol(const char*, ctr_size);
ctr_object* ctr_symbol_as_string (ctr_object* symbol);
ctr_object* ctr_symbol_to_string (ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_symbol_type (ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_symbol_equals (ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_symbol_ihash (ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_symbol_unpack (ctr_object* myself, ctr_argument* argumentList);

#endif /* end of include guard: CTR_C_SYMBOL */
