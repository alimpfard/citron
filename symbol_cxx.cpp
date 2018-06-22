#include "symbol.hpp"
#include <map>
#include <iostream>

struct eqstr
{
  bool operator()(std::string s1, std::string s2) const
  {
    return s1 == s2;
  }
};

static google::dense_hash_map<std::string, ctr_object, std::hash<std::string>, eqstr> symbols;
static int initialized = 0;

inline ctr_object* get_or_create(const char* name, ctr_size length) {
    std::string s (name, length);
    if (symbols.count(s) == 0) {
        ctr_object* spp = ctr_create_symbol(name, length);
        symbols[s] = *spp;
    }
    auto& p = symbols[s];
    ctr_object* op = ctr_internal_create_object(CTR_OBJECT_TYPE_OTSTRING);
    ctr_set_link_all(op, CtrStdSymbol);
    op->value.svalue = p.value.svalue;
    op->info.type = CTR_OBJECT_TYPE_OTMISC;
    return op;
}

extern "C"
ctr_object* ctr_get_or_create_symbol_table_entry_s (const char* name, ctr_size length) {
    if(!initialized) {
        symbols.set_empty_key("");
        initialized = 1;
    }
    ctr_object* s = get_or_create(name, length);
    return s;
}

extern "C"
ctr_object *ctr_get_or_create_symbol_table_entry (const char *name, ctr_size length) {
    if(!initialized) {
        symbols.set_empty_key("");
        initialized = 1;
    }
    ctr_object* s = get_or_create(name, length);
    return s;
}
