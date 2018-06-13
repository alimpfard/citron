#include "symbol.hpp"

struct eqstr
{
  bool operator()(std::string s1, std::string s2) const
  {
    return s1 == s2;
  }
};

static dense_hash_map<std::string, ctr_object*, hash<std::string>, eqstr> symbols;
static bool initialized = 0;

inline ctr_object* get_or_create(const char* name, ctr_size length) {
    std::string s (name, length);
    ctr_object* sym;
    if (symbols.count(s) == 0) {
        sym = ctr_create_symbol(name, length);
        symbols[s] = sym;
    } else {
        sym = symbols[s];
    }
    return sym;
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
    return get_or_create(name, length);
}
