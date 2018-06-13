#pragma once

#include "citron.h"
#include <google/dense_hash_map>
#include <functional>

using google::dense_hash_map;
using std::hash;

extern "C" ctr_object *ctr_get_or_create_symbol_table_entry (const char *name, ctr_size length);
extern "C" ctr_object *ctr_create_symbol(const char* s, ctr_size length);
inline ctr_object* get_or_create(const char* name, ctr_size length);
