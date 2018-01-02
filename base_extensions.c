#include "citron.h"

/**
 * [Array|String] letEqual: [Array|Object] in: [Block]
 *
 * Assigns local values to the block, and runs it as a closure
 */
ctr_object *
ctr_block_let (ctr_object * myself, ctr_argument * argumentList)
{
  ctr_object *defs = argumentList->object, *block = argumentList->next->object;
  ctr_object *result;
  ctr_open_context ();
  if (myself->info.type == CTR_OBJECT_TYPE_OTARRAY)
    {
      for (ctr_size i = 0;
	   i < myself->value.avalue->head - myself->value.avalue->tail
	   && (defs->info.type == CTR_OBJECT_TYPE_OTARRAY
	       && i < defs->value.avalue->head - defs->value.avalue->tail); i++)
	{
	  switch (defs->info.type)
	    {
	    case CTR_OBJECT_TYPE_OTARRAY:
	      ctr_assign_value_to_local (myself->value.avalue->elements[i],
					 defs->value.avalue->elements[i]);
	      break;
	    default:
	      ctr_assign_value_to_local (myself->value.avalue->elements[i], defs);
	      break;
	    }
	}
      result = ctr_block_run_here (block, NULL, block);
    }
  else
    {
      myself = ctr_internal_cast2string (myself);
      ctr_assign_value_to_local (myself, defs);
      result = ctr_block_run_here (block, NULL, block);
    }
  ctr_close_context ();
  return result;
}

void
initiailize_base_extensions ()
{
  ctr_internal_create_func (CtrStdObject,
			    ctr_build_string_from_cstring ("letEqual:in:"), &ctr_block_let);
}
