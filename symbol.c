#include "symbol.h"
#include <stdio.h>

static ctr_object** ctr_static_symbol_table = 0;
static ctr_size ctr_static_symbol_table_count = 0;
ctr_object* ctr_get_or_create_symbol_table_entry(char* name, ctr_size length) {
	ctr_initialize_world();
	if (!ctr_static_symbol_table) {
		ctr_static_symbol_table = ctr_heap_allocate(sizeof(ctr_object*));
    ctr_object* ptr = (ctr_static_symbol_table[0] = ctr_build_string(name, length));
		ctr_static_symbol_table_count = 1;
    ctr_set_link_all(ptr, CtrStdSymbol);
    ptr->info.type = CTR_OBJECT_TYPE_OTMISC;
    return ptr;
	}
	for(ctr_size i=0; i<ctr_static_symbol_table_count; i++) {
    ctr_object* ptr = ctr_static_symbol_table[i];
		if(!ptr) continue;
		ctr_size len = ptr->value.svalue->vlen;
		if (len == length
        &&
        strncmp(ptr->value.svalue->value, name, length) == 0
    )
      return ptr;
	}
	ctr_static_symbol_table = ctr_heap_reallocate(ctr_static_symbol_table, (ctr_static_symbol_table_count+1)*sizeof(ctr_object*));
	ctr_object* ptr = (ctr_static_symbol_table[ctr_static_symbol_table_count++] = ctr_build_string(name, length));
  ctr_set_link_all(ptr, CtrStdSymbol);
  ptr->info.type = CTR_OBJECT_TYPE_OTMISC;
  return ptr;
}

ctr_object* ctr_symbol_to_string(ctr_object* myself, ctr_argument* argumentList) {
	if (unlikely(myself == CtrStdSymbol)) return ctr_build_string_from_cstring("#Symbol");
	char* name = ctr_heap_allocate(sizeof(char)*(myself->value.svalue->vlen+1));
	ctr_size len = sprintf(name, "\\%.*s", myself->value.svalue->vlen, myself->value.svalue->value);
	ctr_object* nameS = ctr_build_string(name, len);
	ctr_heap_free(name);
	return nameS;
}

ctr_object* ctr_symbol_type(ctr_object* myself, ctr_argument* argumentList) {
	return ctr_build_string_from_cstring("Symbol");
}

ctr_object* ctr_symbol_equals(ctr_object* myself, ctr_argument* argumentList) {
	ctr_object* other = argumentList->object;
	return ctr_build_bool(myself == other);
}

ctr_object* ctr_symbol_ihash(ctr_object* myself, ctr_argument* argumentList) {
	if(myself == CtrStdSymbol) return ctr_build_number_from_float(ctr_internal_index_hash(myself));
	else return ctr_build_number_from_float((uintptr_t)(myself));
}

ctr_object* ctr_symbol_unpack(ctr_object* myself, ctr_argument* argumentList) {
	if(myself == CtrStdSymbol) { CtrStdFlow = ctr_build_string_from_cstring("binding a null symbol"); return CtrStdNil; }
	if (argumentList->object->info.type == CTR_OBJECT_TYPE_OTSTRING) {
		return ctr_assign_value(myself, argumentList->object);
	}
	if (ctr_reflect_get_primitive_link(argumentList->object) == CtrStdSymbol) {
		if(argumentList->object != myself) {
			CtrStdFlow = ctr_build_string_from_cstring("Cannot bind symbols with different values");
		}
	}
	return myself;
}
