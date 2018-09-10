#pragma once

#include "citron.h"
//#include <google/dense_hash_map>
#include <functional>

//using google::dense_hash_map;
using std::hash;

extern "C" ctr_object *ctr_get_or_create_symbol_table_entry (const char *name, ctr_size length);
extern "C" ctr_object *ctr_create_symbol(const char* s, ctr_size length);
inline ctr_object* get_or_create(const char* name, ctr_size length);

enum class eFixity {
        LEFT,
        RIGHT
};

struct fixity_ind
{
    eFixity fix;
    int prec;
    int lazy;
};

extern "C" fixity_lookup_rv ctr_lookup_fix(const char* name, int length);
extern "C" void ctr_set_fix(const char* name, int length, int fix, int prec, int lazy);
inline fixity_lookup_rv lookup_fix(std::string s);
