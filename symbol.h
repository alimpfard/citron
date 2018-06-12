#ifndef CTR_C_SYMBOL
#define CTR_C_SYMBOL

#include "citron.h"

ctr_object* ctr_get_or_create_symbol_table_entry(char* name, ctr_size length);
ctr_object* ctr_symbol_as_string (ctr_object* symbol);
ctr_object* ctr_symbol_to_string (ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_symbol_type (ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_symbol_equals (ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_symbol_ihash (ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_symbol_unpack (ctr_object* myself, ctr_argument* argumentList);

#endif /* end of include guard: CTR_C_SYMBOL */
