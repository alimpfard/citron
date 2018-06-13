#include "symbol.h"
#include <stdio.h>

ctr_object * ctr_create_symbol(const char* s, ctr_size length) {
  ctr_object *ptr = ctr_build_string ((char*)s, length);
  ctr_set_link_all (ptr, CtrStdSymbol);
  ptr->info.type = CTR_OBJECT_TYPE_OTMISC;
  return ptr;
}

ctr_object *
ctr_symbol_to_string (ctr_object * myself, ctr_argument * argumentList)
{
  if (unlikely (myself == CtrStdSymbol))
    return ctr_build_string_from_cstring ("Symbol#");
  char *name = ctr_heap_allocate (sizeof (char) * (myself->value.svalue->vlen + 1));
  ctr_size len = sprintf (name, "\\%.*s", myself->value.svalue->vlen, myself->value.svalue->value);
  ctr_object *nameS = ctr_build_string (name, len);
  ctr_heap_free (name);
  return nameS;
}

ctr_object *
ctr_symbol_to_string_s (ctr_object * myself, ctr_argument * argumentList)
{
  return ctr_get_or_create_symbol_table_entry_s(myself->value.svalue->value, myself->value.svalue->vlen);
}

ctr_object *
ctr_symbol_as_string (ctr_object * symbol)
{
  if (unlikely (symbol == CtrStdSymbol))
    return ctr_build_string_from_cstring ("Symbol#");
  ctr_object * nameS = ctr_internal_create_object(CTR_OBJECT_TYPE_OTSTRING);
  nameS->value.svalue = symbol->value.svalue;
  ctr_set_link_all(nameS, CtrStdString);
  return nameS;
}

ctr_object *
ctr_symbol_type (ctr_object * myself, ctr_argument * argumentList)
{
  return ctr_build_string_from_cstring ("Symbol");
}

ctr_object *
ctr_symbol_equals (ctr_object * myself, ctr_argument * argumentList)
{
  ctr_object *other = argumentList->object;
  return ctr_build_bool (myself == other);
}

ctr_object *
ctr_symbol_ihash (ctr_object * myself, ctr_argument * argumentList)
{
  if (myself == CtrStdSymbol)
    return ctr_build_number_from_float (ctr_internal_index_hash (myself));
  else
    return ctr_build_number_from_float ((uintptr_t) (myself));
}

ctr_object *
ctr_symbol_unpack (ctr_object * myself, ctr_argument * argumentList)
{
  if (myself == CtrStdSymbol)
    {
      CtrStdFlow = ctr_build_string_from_cstring ("binding a null symbol");
      return CtrStdNil;
    }
  if (argumentList->object->info.type == CTR_OBJECT_TYPE_OTSTRING)
    {
      return ctr_assign_value (myself, argumentList->object);
    }
  if (ctr_reflect_get_primitive_link (argumentList->object) == CtrStdSymbol)
    {
      if (argumentList->object != myself)
	{
	  CtrStdFlow = ctr_build_string_from_cstring ("Cannot bind symbols with different values");
	}
    }
  return myself;
}
