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

static std::map<std::string, ctr_object> symbols;
static int initialized = 0;

static std::map<std::string, fixity_ind> fixity_map;

static fixity_lookup_rv basic_fixity = {1, 2};

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
//        symbols.set_empty_key("");
        initialized = 1;
    }
    ctr_object* s = get_or_create(name, length);
    return s;
}

extern "C"
ctr_object *ctr_get_or_create_symbol_table_entry (const char *name, ctr_size length) {
    if(!initialized) {
        initialized = 1;
    }
    ctr_object* s = get_or_create(name, length);
    return s;
}

extern "C" fixity_lookup_rv ctr_lookup_fix(const char* name, int length) {
    return lookup_fix(std::string(name, length));
}
extern "C" void ctr_set_fix(const char* name, int length, int fix, int prec) {
    if (length == 0) return;
    auto s = std::string(name, length);
    eFixity fix_;
    if (fix == 0)
        fix_ = eFixity::LEFT;
    else
        fix_ = eFixity::RIGHT;
    fixity_ind ind = {fix_, prec};
    fixity_map[s] = ind;
}
inline fixity_lookup_rv lookup_fix(std::string s) {
    if (fixity_map.count(s) == 0 || s.length() == 0)
        return basic_fixity;
    auto v = fixity_map[s];
    fixity_lookup_rv rv;
    rv.fix = v.fix == eFixity::RIGHT;
    rv.prec = v.prec;
    return rv;
}

extern "C" void clear_fixity_map() {
    fixity_map.clear();
}
