#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdarg.h>
#include <math.h>
#include <unistd.h>
#include <stdint.h>
#include <time.h>

#include "citron.h"
#include "siphash.h"
#include "symbol.h"
/**@I_OBJ_DEF Array*/

/**
 * [Array] new
 * [Array] new: [Number:space]
 * Creates a new Array.
 *
 * Usage:
 *
 * a := Array new.
 *
 * or, the short form:
 *
 * a := Array ← 1 ; 2 ; 3.
 *
 * or (if you prefer ASCII only symbols):
 *
 * a := Array < 1 ; 2 ; 3.
 *
 */
ctr_object *
ctr_array_new (ctr_object * myclass, ctr_argument * argumentList)
{
  ctr_object *s = ctr_internal_create_object (CTR_OBJECT_TYPE_OTARRAY);
  ctr_set_link_all (s, myclass);
  int initial_length = 2;
  if (argumentList && argumentList->object)
    {
      int len = ctr_internal_cast2number (argumentList->object)->value.nvalue;
      if (len > 1)
	initial_length = len;
    }
  s->value.avalue = (ctr_collection *) ctr_heap_allocate (sizeof (ctr_collection));
  s->value.avalue->immutable = 0;
  s->value.avalue->length = initial_length;
  s->value.avalue->elements = (ctr_object **) ctr_heap_allocate (sizeof (ctr_object *) * initial_length);
  s->value.avalue->head = 0;
  s->value.avalue->tail = 0;
  return s;
}

/**
 * [Array] copy
 *
 * shallow copy of the array
 **/
ctr_object *
ctr_array_copy (ctr_object * myself, ctr_argument * argumentList)
{
  ctr_object *arr = ctr_array_new (CtrStdArray, NULL);
  ctr_argument parg = {0}, *pushArg = &parg;
  for (ctr_size i = myself->value.avalue->tail; i < myself->value.avalue->head; i++)
    {
      pushArg->object = *(myself->value.avalue->elements + i);
      ctr_array_push (arr, pushArg);
    }
  return arr;
}

/**
 * [Array] type
 *
 * Returns the string 'Array'.
 *
 **/
ctr_object *
ctr_array_type (ctr_object * myself, ctr_argument * argumentList)
{
  return ctr_build_string_from_cstring ("Array");
}

/**
 * [Array] push: [Element]
 *
 * Pushes an element on top of the array.
 *
 * Usage:
 *
 * numbers := Array new.
 * numbers push: 3.
 */
ctr_object *
ctr_array_push (ctr_object * myself, ctr_argument * argumentList)
{
  if (myself->value.avalue->immutable)
    {
      CtrStdFlow = ctr_build_string_from_cstring ("Cannot change immutable array's structure");
      return myself;
    }
  ctr_object *pushValue;
  if (myself->value.avalue->length <= (myself->value.avalue->head + 1))
    {
      myself->value.avalue->length = myself->value.avalue->length * 2;
      myself->value.avalue->elements =
	(ctr_object **) ctr_heap_reallocate (myself->value.avalue->elements, (sizeof (ctr_object *) * (myself->value.avalue->length)));
    }
  pushValue = argumentList->object;
  *(myself->value.avalue->elements + myself->value.avalue->head) = pushValue;
  myself->value.avalue->head++;
  return myself;
}

ctr_object *
ctr_array_push_imm (ctr_object * myself, ctr_argument * argumentList)
{
  ctr_object *pushValue;
  if (myself->value.avalue->length <= (myself->value.avalue->head + 1))
    {
      myself->value.avalue->length = myself->value.avalue->length * 2;
      myself->value.avalue->elements =
	(ctr_object **) ctr_heap_reallocate (myself->value.avalue->elements, (sizeof (ctr_object *) * (myself->value.avalue->length)));
    }
  pushValue = argumentList->object;
  *(myself->value.avalue->elements + myself->value.avalue->head) = pushValue;
  myself->value.avalue->head++;
  return myself;
}

/**
 * [Array] min
 *
 * Returns the minimum value from an array.
 *
 * Usage:
 *
 * a := Array ← 8 ; 4 ; 2 ; 16.
 * m := a min. #2
 *
 */
ctr_object *
ctr_array_min (ctr_object * myself, ctr_argument * argumentList)
{
  double min = 0;
  double v = 0;
  ctr_object *el;
  ctr_size i = 0;
  for (i = myself->value.avalue->tail; i < myself->value.avalue->head; i++)
    {
      el = *(myself->value.avalue->elements + i);
      v = ctr_internal_cast2number (el)->value.nvalue;
      if (i == 0 || v < min)
	{
	  min = v;
	}
    }
  return ctr_build_number_from_float (min);
}

/**
 * [Array] all [: [Block]]
 *
 * Returns whether all the objects inside the array are truth-y.
 *
 * Usage:
 *
 * a := Array ← True ; False ; False.
 * m := a all. #False
 *
 */
ctr_object *
ctr_array_all (ctr_object * myself, ctr_argument * argumentList)
{
  ctr_object *el;
  ctr_size i = 0;
  ctr_object *fn;
  if ((fn = argumentList->object))
    {
      ctr_open_context ();
      for (i = myself->value.avalue->tail; i < myself->value.avalue->head; i++)
	{
	  argumentList->object = *(myself->value.avalue->elements + i);
	  el = ctr_block_run_here (fn, argumentList, fn);
	  if ((el->info.type == CTR_OBJECT_TYPE_OTBOOL && el->value.bvalue) || (ctr_internal_cast2bool (el)->value.bvalue));
	  else
	    {
	      ctr_close_context ();
	      return ctr_build_bool (0);
	    }
	}
      ctr_close_context ();
      return ctr_build_bool (1);
    }
  else
    {
      for (i = myself->value.avalue->tail; i < myself->value.avalue->head; i++)
	{
	  el = *(myself->value.avalue->elements + i);
	  if ((el->info.type == CTR_OBJECT_TYPE_OTBOOL && el->value.bvalue) || (ctr_internal_cast2bool (el)->value.bvalue));
	  else
	    return ctr_build_bool (0);
	}
      return ctr_build_bool (1);
    }
}

/**
 * [Array] any [: [Block]]
 *
 * Returns whether any of the objects inside the array are truth-y.
 *
 * Usage:
 *
 * a := Array ← True ; False ; False.
 * m := a any. #True
 *
 */
ctr_object *
ctr_array_any (ctr_object * myself, ctr_argument * argumentList)
{
  ctr_object *el;
  ctr_size i = 0;
  ctr_object *fn;
  if ((fn = argumentList->object))
    {
      ctr_open_context ();
      for (i = myself->value.avalue->tail; i < myself->value.avalue->head; i++)
	{
	  argumentList->object = *(myself->value.avalue->elements + i);
	  el = ctr_block_run_here (fn, argumentList, fn);
	  if ((el->info.type == CTR_OBJECT_TYPE_OTBOOL && el->value.bvalue) || (ctr_internal_cast2bool (el)->value.bvalue))
	    {
	      ctr_close_context ();
	      return ctr_build_bool (1);
	    }
	}
      ctr_close_context ();
      return ctr_build_bool (0);
    }
  else
    {
      for (i = myself->value.avalue->tail; i < myself->value.avalue->head; i++)
	{
	  el = *(myself->value.avalue->elements + i);
	  if ((el->info.type == CTR_OBJECT_TYPE_OTBOOL && el->value.bvalue) || (ctr_internal_cast2bool (el)->value.bvalue))
	    {
	      return ctr_build_bool (1);
	    }
	}
      return ctr_build_bool (0);
    }
}

/**
 * [Array] max
 *
 * Returns the maximum value from an array.
 *
 * Usage:
 *
 * a := Array ← 8 ; 4 ; 2 ; 16.
 * m := a max. #16
 *
 */
ctr_object *
ctr_array_max (ctr_object * myself, ctr_argument * argumentList)
{
  double max = 0;
  double v = 0;
  ctr_object *el;
  ctr_size i = 0;
  for (i = myself->value.avalue->tail; i < myself->value.avalue->head; i++)
    {
      el = *(myself->value.avalue->elements + i);
      v = ctr_internal_cast2number (el)->value.nvalue;
      if (i == 0 || max < v)
	{
	  max = v;
	}
    }
  return ctr_build_number_from_float (max);
}

/**
 * [Array] sum
 *
 * Takes the sum of an array. This message will calculate the
 * sum of the elements in the array.
 *
 * Usage:
 *
 * a := Array ← 1 ; 2 ; 3.
 * s := a sum. #6
 *
 * In the example above, the sum of array will be stored in s and
 * it's value will be 6.
 */
ctr_object *
ctr_array_sum (ctr_object * myself, ctr_argument * argumentList)
{
  ctr_object *sum = CtrStdNil;
  ctr_object *el;
  ctr_size i = 0;
  for (i = myself->value.avalue->tail; i < myself->value.avalue->head; i++)
    {
      if (i == myself->value.avalue->tail)
	{
	  sum = *myself->value.avalue->elements;
	  continue;
	}
      el = *(myself->value.avalue->elements + i);
      sum = ctr_send_message_variadic (sum, "+", 1, 1, el);
    }
  return (sum);
}

/**
 * [Array] product
 *
 * Takes the product of an array. On receiving this message, the
 * Array recipient object will calculate the product of its
 * numerical elements.
 *
 * Usage:
 *
 * a := Array ← 2 ; 4 ; 8.
 * p := a product. #64
 *
 * In the example above, the product of the array will be calculated
 * because the array receives the message 'product'. The product of the elements
 * ( 2 * 4 * 8 = 64 ) will be stored in p.
 */
ctr_object *
ctr_array_product (ctr_object * myself, ctr_argument * argumentList)
{
  ctr_object *prod = CtrStdNil;
  ctr_object *el;
  ctr_size i = 0;
  for (i = myself->value.avalue->tail; i < myself->value.avalue->head; i++)
    {
      if (i == myself->value.avalue->tail)
	{
	  prod = *myself->value.avalue->elements;
	  continue;
	}
      el = *(myself->value.avalue->elements + i);
      prod = ctr_send_message_variadic (prod, "*", 1, 1, el);
    }
  return (prod);
}

/**
 * [Array] * [o:Number | Array]
 *
 * Repeats the array o times if o is a number, and generates an array multiplication
 * for myself and o if o is an array
 */
ctr_object *
ctr_array_multiply (ctr_object * myself, ctr_argument * argumentList)
{
  ctr_object *mand = argumentList->object;
  if (mand->info.type == CTR_OBJECT_TYPE_OTNUMBER)
    {
      int rep = (int) mand->value.nvalue;
      if (rep == 0)
	return ctr_array_new (CtrStdArray, NULL);
      else if (rep == 1)
	return myself;
      else
	{
	  rep--;
	  ctr_object *newArr = ctr_array_new (CtrStdArray, NULL);
	  ctr_argument varg = {0}, *arg = &varg;
	  for (; rep >= 0; rep--)
	    {
	      arg->object = myself;
	      newArr = ctr_array_add (newArr, arg);
	    }
	  return newArr;
	}
    }
  if (mand->info.type == CTR_OBJECT_TYPE_OTARRAY)
    {
      ctr_object *newArr = ctr_array_new (CtrStdArray, NULL);
      ctr_argument arg = {0}, *args = &arg;
      for (ctr_size i = myself->value.avalue->tail; i < myself->value.avalue->head; i++)
	{
	  ctr_object *elem0 = *(myself->value.avalue->elements + i);
	  for (ctr_size j = mand->value.avalue->tail; j < mand->value.avalue->head; j++)
	    {
	      ctr_object *lst = ctr_array_new (CtrStdArray, NULL);
	      args->object = elem0;
	      ctr_array_push (lst, args);
	      args->object = *(mand->value.avalue->elements + j);
	      ctr_array_push (lst, args);
	      lst->value.avalue->immutable = 1;
	      args->object = lst;
	      ctr_array_push (newArr, args);
	    }
	}
      return newArr;
    }
}

/**
 * [Array] intersperse: [o: Object]
 *
 * places an 'o' between all array elements
 * returns an array
 **/
ctr_object *
ctr_array_intersperse (ctr_object * myself, ctr_argument * argumentList)
{
  ctr_object *newArr = ctr_array_new (CtrStdArray, NULL);
  ctr_size i = 0;
  ctr_argument parg = {0}, *pushArg = &parg;
  for (i = myself->value.avalue->tail; i < myself->value.avalue->head; i++)
    {
      pushArg->object = *(myself->value.avalue->elements + i);
      newArr = ctr_array_push (newArr, pushArg);
      if (i < myself->value.avalue->head - 1)
	{
	  pushArg->object = argumentList->object;
	  newArr = ctr_array_push (newArr, pushArg);
	}
    }
  return newArr;
}

/**
 * [Array] map: [Block].
 *
 * Iterates over the array. Passing each element as a key-value pair to the
 * specified block.
 * The map message will pass the following arguments to the block, the key,
 * the value and a reference to the array itself. The last argument might seem
 * redundant but allows for a more functional programming style.
 *
 * Usage:
 *
 * files map: showName.
 * files map: {
 *   :key :filename :files
 *   Pen write: filename, brk.
 * }.
 */
ctr_object *
ctr_array_map (ctr_object * myself, ctr_argument * argumentList)
{
  ctr_object *block = argumentList->object;
  ctr_size i = 0;
  if (block->info.type != CTR_OBJECT_TYPE_OTBLOCK)
    {
      CtrStdFlow = ctr_build_string_from_cstring ("Expected Block.");
      CtrStdFlow->info.sticky = 1;
      return myself;
    }
  block->info.sticky = 1;
  ctr_argument a0 = {0}, *arguments = &a0;
  ctr_argument a1 = {0}, *argument2 = &a1;
  ctr_argument a2 = {0}, *argument3 = NULL;

  if (!myself->value.avalue->immutable)
    argument3 = &a2;

  for (i = myself->value.avalue->tail; i < myself->value.avalue->head; i++)
    {
      arguments->object = ctr_build_number_from_float ((double) i);
      argument2->object = *(myself->value.avalue->elements + i);
      if (!myself->value.avalue->immutable)
	argument3->object = myself;
      arguments->next = argument2;
      if (!myself->value.avalue->immutable)
	argument2->next = argument3;
      ctr_block_run (block, arguments, NULL);
      if (CtrStdFlow == CtrStdContinue)
	CtrStdFlow = NULL;
      if (CtrStdFlow)
	break;
    }
  if (CtrStdFlow == CtrStdBreak)
    CtrStdFlow = NULL;		/* consume break */
  block->info.mark = 0;
  block->info.sticky = 0;
  return myself;
}

/**
 * [Array] each: [Block].
 *
 *  Alias for [Array] map: [Block].
 */

/**
 * [Array] map_v: [Block].
 *
 * Iterates over the array. Passing each element as a value to the
 * specified block.
 * The map message will pass only the value.
 *
 * Usage:
 *
 * files map_v: showName.
 * files map_v: {
 *   :filename
 *   Pen write: filename, brk.
 * }.
 */
 /**
  * [Array] each_v: [Block].
  *
  *  Alias for [Array] map_v: [Block].
  */
ctr_object *
ctr_array_map_v (ctr_object * myself, ctr_argument * argumentList)
{
  ctr_object *func = argumentList->object;
  CTR_ENSURE_TYPE_BLOCK (func);

  ctr_argument parg = {0}, *pushArg = &parg;
  ctr_argument earg = {0}, *elnumArg = &earg;
  ctr_size i;
  for (i = 0; i < myself->value.avalue->head - myself->value.avalue->tail; i++)
    {
      ctr_object *elnum = ctr_build_number_from_float ((ctr_number) i);
      elnumArg->object = elnum;
      pushArg->object = ctr_array_get (myself, elnumArg);
      ctr_block_run (func, pushArg, func);
      if (CtrStdFlow == CtrStdContinue)
	{
	  CtrStdFlow = NULL;
	  continue;
	}
      if (CtrStdFlow)
	break;
    }
  if (CtrStdFlow == CtrStdBreak)
    CtrStdFlow = NULL;
  return myself;
}

/**
 * [Array] from: [Number] lengthMax: [Number]
 *
 *  makes n-tuple-wise iterations
 * If `fill` is specified, that will be used to fill when we are elements short
 * Else Nil will be passed
 */
ctr_object *
ctr_array_slice (ctr_object * myself, ctr_argument * argumentList)
{
  ctr_argument varg = {0}, *arg = &varg;
  ctr_object *elnum;
  ctr_object *startElement = ctr_internal_cast2number (argumentList->object);
  ctr_object *count = ctr_internal_cast2number (argumentList->next->object);
  ctr_size start = (ctr_size) startElement->value.nvalue;
  if (start >= myself->value.avalue->head - myself->value.avalue->tail)
    return ctr_array_new (CtrStdArray, NULL);
  ctr_size len = (ctr_size) count->value.nvalue;
  len = fmin (myself->value.avalue->head - myself->value.avalue->tail - start, len);
  ctr_size i = 0;
  ctr_object *newArray = ctr_array_new (CtrStdArray, NULL);
  for (i = start; i < start + len; i++)
    {
      elnum = ctr_build_number_from_float ((ctr_number) i);
      arg->object = elnum;
      arg->object = ctr_array_get (myself, arg);
      ctr_array_push (newArray, arg);
    }
  return newArray;
}

/**
 * [Array] every: [Number] do: [Block] fill: [Object].
 * [Array] every: [Number] do: [Block].
 *
 *  makes n-tuple-wise iterations
 * If `fill` is specified, that will be used to fill when we are elements short
 * Else Nil will be passed
 */
ctr_object *
ctr_array_every_do_fill (ctr_object * myself, ctr_argument * argumentList)
{
  ctr_size icount = ctr_internal_cast2number (argumentList->object)->value.nvalue;
  ctr_object *func = argumentList->next->object;
  CTR_ENSURE_TYPE_BLOCK (func);
  ctr_object *fill = ctr_build_nil ();
  if (argumentList->next->next && argumentList->next->next->object)
    fill = argumentList->next->next->object;
  ctr_object *parr;		//= ctr_array_new(CtrStdArray, NULL);
  ctr_argument varg = {0}, *args = &varg;
  ctr_argument vcaa = {0}, *caa = &vcaa;
  for (ctr_size s = ceil ((myself->value.avalue->head - myself->value.avalue->tail) / (double) icount), i = 0; i < s; i++)
    {
      args->object = ctr_build_number_from_float (i * icount);
      args->next = caa;
      args->next->object = ctr_build_number_from_float (icount);
      parr = ctr_array_slice (myself, args);
      while (parr->value.avalue->head - parr->value.avalue->tail < icount)
	{
	  args->object = fill;
	  ctr_array_push (parr, args);
	}
      args->object = parr;
      args->next = NULL;
      ctr_block_run (func, args, parr);
    }
  return myself;
}

/**
 * [Array] chunks: [Number]
 * [Array] chunks: [Number] fill: [Object]
 *  makes a bunch of n-tuples
 * If `fill` is specified, that will be used to fill when we are elements short
 * Else Nil will be passed
 */
ctr_object *
ctr_array_chunks (ctr_object * myself, ctr_argument * argumentList)
{
  ctr_size icount = ctr_internal_cast2number (argumentList->object)->value.nvalue;
  ctr_object *fill = CtrStdNil;
  if (argumentList->next && argumentList->next->object)
    fill = argumentList->next->object;
  ctr_object *parr;		//= ctr_array_new(CtrStdArray, NULL);
  ctr_object *ret = ctr_array_new (CtrStdArray, NULL);
  ctr_argument varg = {0}, *args = &varg;
  ctr_argument vcaa = {0}, *caa = &vcaa;
  for (ctr_size s = ceil ((myself->value.avalue->head - myself->value.avalue->tail) / (double) icount), i = 0; i < s; i++)
    {
      args->object = ctr_build_number_from_float (i * icount);
      args->next = caa;
      args->next->object = ctr_build_number_from_float (icount);
      parr = ctr_array_slice (myself, args);
      while (parr->value.avalue->head - parr->value.avalue->tail < icount)
	{
	  args->object = fill;
	  parr = ctr_array_push (parr, args);
	}
      args->object = parr;
      args->next = NULL;
      ctr_array_push (ret, args);
    }
  return ret;
}

/**
 * [Array] ← [Element1] ; [Element2] ; ...
 *
 * Creates a new instance of an array and initializes this
 * array with a first element, useful for literal-like Array
 * notations.
 *
 * Usage:
 *
 * a := Array ← 1 ; 2 ; 3.
 *
 * or if ASCII-only is preferred:
 *
 * a := Array < 1 ; 2 ; 3.
 *
 * Note that the ; symbol here is an alias for 'push:'.
 */
ctr_object *
ctr_array_new_and_push (ctr_object * myclass, ctr_argument * argumentList)
{
  ctr_object *s = ctr_array_new (myclass, NULL);
  return ctr_array_push (s, argumentList);
}

/**
 * [Array] unshift: [Element].
 *
 * Unshift operation for array.
 * Adds the specified element to the beginning of the array.
 *
 * Usage:
 *
 * a := Array new.
 * a push: 1.
 * a unshift: 3. #now contains: 3,1
 */
ctr_object *
ctr_array_unshift (ctr_object * myself, ctr_argument * argumentList)
{
  if (myself->value.avalue->immutable)
    {
      CtrStdFlow = ctr_build_string_from_cstring ("Cannot change immutable array's structure");
      return myself;
    }
  ctr_object *pushValue = argumentList->object;
  if (myself->value.avalue->tail > 0)
    {
      myself->value.avalue->tail--;
    }
  else
    {
      if (myself->value.avalue->length <= (myself->value.avalue->head + 1))
	{
	  myself->value.avalue->length = myself->value.avalue->length * 3;
	  myself->value.avalue->elements =
	    (ctr_object **) ctr_heap_reallocate (myself->value.avalue->elements, (sizeof (ctr_object *) * (myself->value.avalue->length)));
	}
      myself->value.avalue->head++;
      memmove (myself->value.avalue->elements + 1, myself->value.avalue->elements, myself->value.avalue->head * sizeof (ctr_object *));
    }
  *(myself->value.avalue->elements + myself->value.avalue->tail) = pushValue;
  return myself;
}

/**
 * [Array] reverse
 */
ctr_object *
ctr_array_reverse (ctr_object * myself, ctr_argument * argumentList)
{
  ctr_object *newArr = ctr_array_new (CtrStdArray, NULL);
  ctr_size i = ctr_array_count (myself, NULL)->value.nvalue;
  ctr_argument varg = {0}, *args = &varg;
  for (; i > 0; i--)
    {
      args->object = ctr_build_number_from_float (i - 1);
      args->object = ctr_array_get (myself, args);
      ctr_array_push (newArr, args);
    }
  return newArr;
}

/**
 * [Array] join: [Glue].
 *
 * Joins the elements of an array together in a string
 * separated by a specified glue string.
 *
 * Usage:
 *
 * collection := Array new.
 * collection push: 1, push: 2, push 3.
 * collection join: ','. # results in string: '1,2,3'
 */
ctr_object *
ctr_array_join (ctr_object * myself, ctr_argument * argumentList)
{
  ctr_size i;
  char *result;
  ctr_size len = 0;
  ctr_size pos;
  ctr_object *o;
  ctr_object *str;
  ctr_object *resultStr;
  ctr_object *glue = ctr_internal_cast2string (argumentList->object);
  ctr_size glen = glue->value.svalue->vlen;
  for (i = myself->value.avalue->tail; i < myself->value.avalue->head; i++)
    {
      o = *(myself->value.avalue->elements + i);
      str = ctr_internal_cast2string (o);
      pos = len;
      if (len == 0)
	{
	  len = str->value.svalue->vlen;
	  result = ctr_heap_allocate (sizeof (char) * len);
	}
      else
	{
	  len += str->value.svalue->vlen + glen;
	  result = ctr_heap_reallocate (result, sizeof (char) * len);
	  memcpy (result + pos, glue->value.svalue->value, glen);
	  pos += glen;
	}
      memcpy (result + pos, str->value.svalue->value, str->value.svalue->vlen);
    }
  resultStr = ctr_build_string (result, len);
  if (len > 0)
    ctr_heap_free (result);
  return resultStr;
}

/**
 * [Array] at: [Index]
 *
 * Returns the element in the array at the specified index.
 * Note that the fisrt index of the array is index 0.
 *
 * Usage:
 *
 * fruits := Array ← 'apples' ; 'oranges' ; 'bananas'.
 * fruits at: 1. #returns 'oranges'
 */
ctr_object *
ctr_array_get (ctr_object * myself, ctr_argument * argumentList)
{
  ctr_object *getIndex = argumentList->object;
  long long i;
  if (getIndex->info.type != CTR_OBJECT_TYPE_OTNUMBER)
    {
      //printf("Index must be number.\n"); exit(1);
      char buf[1024];
      char *typename_ =
	ctr_heap_allocate_cstring (ctr_internal_cast2string
				   (ctr_send_message
				    (getIndex, "type", 4, NULL))), *value = ctr_heap_allocate_cstring (ctr_internal_cast2string (getIndex));
      sprintf (buf, "Array index must be a number (not %s type %d(%s)).", value, getIndex->info.type, typename_);
      CtrStdFlow = ctr_build_string_from_cstring (buf);
      ctr_heap_free (typename_);
      ctr_heap_free (value);
      return CtrStdNil;
    }
  i = (int) getIndex->value.nvalue;
  if (i < 0)
    i += myself->value.avalue->head - myself->value.avalue->tail;
  if (myself->value.avalue->head <= (i + myself->value.avalue->tail) || i < 0)
    {
      CtrStdFlow = ctr_build_string_from_cstring ("Index out of bounds.");
      CtrStdFlow->info.sticky = 1;
      return CtrStdNil;
    }
  return *(myself->value.avalue->elements + myself->value.avalue->tail + i);
}

/**
 * [Array] indexOf: [Object]
 *
 * Returns the index of the first occurence of object
 * -1 if not found
 */
ctr_object *
ctr_array_index (ctr_object * myself, ctr_argument * argumentList)
{
  ctr_size i;
  ctr_object *o;
  ctr_object *needle = argumentList->object;
  for (i = myself->value.avalue->tail; i < myself->value.avalue->head; i++)
    {
      o = *(myself->value.avalue->elements + i);
      if (ctr_internal_object_is_equal (o, needle))
	return ctr_build_number_from_float (i);
    }
  return ctr_build_number_from_float (-1);
}

/**
 * [Array] contains: [Object]
 *
 * Returns whether the array contains the object or not
 */
ctr_object *
ctr_array_contains (ctr_object * myself, ctr_argument * argumentList)
{
  ctr_size i;
  ctr_object *o;
  ctr_object *needle = argumentList->object;
  for (i = myself->value.avalue->tail; i < myself->value.avalue->head; i++)
    {
      o = *(myself->value.avalue->elements + i);
      if (ctr_internal_object_is_equal (o, needle))
	return ctr_build_bool (1);
    }
  return ctr_build_bool (0);
}

/**
 * [Array] @ [Index]
 *
 * Alias for [Array] at: [Index]
 */

/**
 * [Array] put: [Element] at: [Index]
 *
 * Puts a value in the array at the specified index.
 * Array will be automatically expanded if the index is higher than
 * the maximum index of the array, unless the array is immutable
 *
 * Usage:
 *
 * fruits := Array new.
 * fruits put: 'apples' at: 5.
 */
ctr_object *
ctr_array_put (ctr_object * myself, ctr_argument * argumentList)
{
  if (myself->value.avalue->immutable)
    {
      CtrStdFlow = ctr_build_string_from_cstring ("Cannot change immutable array's structure");
      return myself;
    }
  ctr_object *putValue = argumentList->object;
  ctr_object *putIndex = ctr_internal_cast2number (argumentList->next->object);
  ctr_size putIndexNumber;
  ctr_size head;
  ctr_size tail;
  ctr_argument arg = {0}, *argument = &arg;
  if (putIndex->value.nvalue < 0)
    {
      CtrStdFlow = ctr_build_string_from_cstring ("Index out of bounds.");
      CtrStdFlow->info.sticky = 1;
      return myself;
    }

  head = (ctr_size) myself->value.avalue->head;
  tail = (ctr_size) myself->value.avalue->tail;
  putIndexNumber = (ctr_size) putIndex->value.nvalue;
  if (head <= putIndexNumber)
    {
      if (myself->value.avalue->immutable)
	{
	  CtrStdFlow = ctr_build_string_from_cstring ("Cannot change immutable array's structure");
	  return myself;
	}
      ctr_size j;
      for (j = head; j <= putIndexNumber; j++)
	{
	  argument->object = CtrStdNil;
	  ctr_array_push (myself, argument);
	}
      myself->value.avalue->head = putIndexNumber + 1;
    }
  if (putIndexNumber < tail)
    {
      ctr_size j;
      for (j = tail; j > putIndexNumber; j--)
	{
	  *(myself->value.avalue->elements + j) = CtrStdNil;
	}
      myself->value.avalue->tail = putIndexNumber;
    }
  *(myself->value.avalue->elements + putIndexNumber) = putValue;
  return myself;
}

/**
 * [Array] pop [: [idx]]
 *
 * Pops off the last element of the array.
 */
ctr_object *
ctr_array_pop (ctr_object * myself, ctr_argument * argumentList)
{
  if (myself->value.avalue->immutable)
    {
      CtrStdFlow = ctr_build_string_from_cstring ("Cannot change immutable array's structure");
      return myself;
    }
  if (myself->value.avalue->tail >= myself->value.avalue->head)
    {
      return CtrStdNil;
    }
  if (argumentList->object && argumentList->object->info.type == CTR_OBJECT_TYPE_OTNUMBER)
    {
      ctr_object *val = *(myself->value.avalue->elements + (ctr_size) (argumentList->object->value.nvalue));
      myself->value.avalue->head--;
      for (int i = (ctr_size) (argumentList->object->value.nvalue); i < myself->value.avalue->head - myself->value.avalue->tail; i++)
	myself->value.avalue->elements[i] = myself->value.avalue->elements[i + 1];
      return val;
    }
  else
    {
      myself->value.avalue->head--;
      return *(myself->value.avalue->elements + myself->value.avalue->head);
    }
}

/**
 * [Array] shift
 *
 * Shifts off the first element of the array.
 */
ctr_object *
ctr_array_shift (ctr_object * myself, ctr_argument * argumentList)
{
  if (myself->value.avalue->immutable)
    {
      CtrStdFlow = ctr_build_string_from_cstring ("Cannot change immutable array's structure");
      return myself;
    }
  ctr_object *shiftedOff;
  if (myself->value.avalue->tail >= myself->value.avalue->head)
    {
      return CtrStdNil;
    }
  shiftedOff = *(myself->value.avalue->elements + myself->value.avalue->tail);
  myself->value.avalue->tail++;
  return shiftedOff;
}

/**
 * [Array] count
 *
 * Returns the number of elements in the array.
 */
ctr_object *
ctr_array_count (ctr_object * myself, ctr_argument * argumentList)
{
  ctr_number d = 0;
  d = (ctr_number) myself->value.avalue->head - myself->value.avalue->tail;
  return ctr_build_number_from_float ((ctr_number) d);
}

/**
 * [Array] from: [Begin] length: [End]
 *
 * Copies part of an array indicated by from and to and
 * returns a new array consisting of a copy of this region.
 */
ctr_object *
ctr_array_from_length (ctr_object * myself, ctr_argument * argumentList)
{
  ctr_object *elnum;
  ctr_object *startElement = ctr_internal_cast2number (argumentList->object);
  ctr_object *count = ctr_internal_cast2number (argumentList->next->object);
  ctr_size start = (int) startElement->value.nvalue;
  ctr_size len = (int) count->value.nvalue;
  ctr_size i = 0;
  ctr_object *newArray = ctr_array_new (CtrStdArray, NULL);
  ctr_argument parg = {0}, *pushArg = &parg;
  ctr_argument earg = {0}, *elnumArg = &earg;
  for (i = start; i < start + len; i++)
    {
      elnum = ctr_build_number_from_float ((ctr_number) i);
      elnumArg->object = elnum;
      pushArg->object = ctr_array_get (myself, elnumArg);
      ctr_array_push (newArray, pushArg);
    }
  return newArray;
}

/**
 * [Array] skip: [n:Number]
 *
 * Skips the first n elements of the array
 */
ctr_object *
ctr_array_skip (ctr_object * myself, ctr_argument * argumentList)
{
  ctr_object *elnum;
  ctr_object *startElement = ctr_internal_cast2number (argumentList->object);
  ctr_size start = (int) startElement->value.nvalue;
  ctr_size len = (int) myself->value.avalue->length - 1;
  ctr_size i = 0;
  ctr_object *newArray = ctr_array_new (CtrStdArray, NULL);
  ctr_argument parg = {0}, *pushArg = &parg;
  ctr_argument earg = {0}, *elnumArg = &earg;
  for (i = start; i < start + len; i++)
    {
      elnum = ctr_build_number_from_float ((ctr_number) i);
      elnumArg->object = elnum;
      pushArg->object = ctr_array_get (myself, elnumArg);
      ctr_array_push (newArray, pushArg);
    }
  return newArray;
}

/**
 * [Array] zip
 *
 * zip array.
 * Works on the inner elements of the array.
 * If the element is an array, it is expanded exactly once.
 * And if the element is not an array, it is used as-is
 *
 */
ctr_object *
ctr_array_zip (ctr_object * myself, ctr_argument * argumentList)
{
  ctr_object* res = ctr_array_new(CtrStdArray, NULL);
  for (int i = 0;;i++) {
    ctr_object* tarr = ctr_array_new(CtrStdArray, NULL);
    for (int x = myself->value.avalue->tail; x < myself->value.avalue->head; x++) {
      ctr_object* element = myself->value.avalue->elements[x];
      if (element->info.type != CTR_OBJECT_TYPE_OTARRAY)
        ctr_array_push(
          tarr,
          &(ctr_argument){
            element,
            NULL
          }
        );
      else if (element->value.avalue->head - element->value.avalue->tail > i)
        ctr_array_push(
          tarr,
          &(ctr_argument){
            element->value.avalue->elements[i+element->value.avalue->tail],
            NULL
          }
        );
      else goto thatsenough;
    }
    ctr_array_push (res, &(ctr_argument){tarr});
  }
  thatsenough:;
  return res;
}

/**
 * [Array] zipWith: [Block]
 *
 * zip array according to the return of the block
 * Works on the inner elements of the array.
 * If the element is an array, it is expanded exactly once.
 * And if the element is not an array, it is used as-is
 *
 */
ctr_object *
ctr_array_zip_with (ctr_object * myself, ctr_argument * argumentList)
{
  ctr_object *blk = argumentList->object;
  ctr_object* res = ctr_array_new(CtrStdArray, NULL);
  for (int i = 0;;i++) {
    ctr_object* tarr = ctr_array_new(CtrStdArray, NULL);
    for (int x = myself->value.avalue->tail; x < myself->value.avalue->head; x++) {
      ctr_object* element = myself->value.avalue->elements[x];
      if (element->info.type != CTR_OBJECT_TYPE_OTARRAY)
        ctr_array_push(
          tarr,
          &(ctr_argument){
            element,
            NULL
          }
        );
      else if (element->value.avalue->head - element->value.avalue->tail > i)
        ctr_array_push(
          tarr,
          &(ctr_argument){
            element->value.avalue->elements[i+element->value.avalue->tail],
            NULL
          }
        );
      else goto thatsenough;
    }
    ctr_array_push (res, &(ctr_argument){ctr_block_runall(blk, &(ctr_argument){tarr})});
  }
  thatsenough:;
  return res;
}

/**
 * [Array] head
 * see also (tail, init, last)
 *
 * returns the first element of the array
 * Does generate exceptions when array is empty
 */
ctr_object *
ctr_array_head (ctr_object * myself, ctr_argument * argumentList)
{
  return ctr_send_message_variadic (myself, "at:", 3, 1, ctr_build_number_from_float (0));
}

/**
 * [Array] tail
 * see also (head, init, last)
 *
 * returns all of the array sans the first element
 * Does not generate exceptions when array is empty
 */
ctr_object *
ctr_array_tail (ctr_object * myself, ctr_argument * argumentList)
{
  ctr_object *arr = ctr_array_new (CtrStdArray, NULL);
  if (myself->value.avalue->head == myself->value.avalue->tail)
    return arr;
  else
    {
      ctr_argument varg = {0}, *arg = &varg;
      for (ctr_size i = 1; i < myself->value.avalue->head - myself->value.avalue->tail; i++)
	{
	  arg->object = ctr_build_number_from_float (i);
	  arg->object = ctr_send_message (myself, "at:", 3, arg);
	  ctr_send_message (arr, "push:", 5, arg);
	}
    }
  arr->value.avalue->immutable = myself->value.avalue->immutable;
  return arr;
}

/**
 * [Array] init
 * see also (head, tail, last)
 *
 * returns all of the array sans the last element
 * Does not generate exceptions when array is empty
 */
ctr_object *
ctr_array_init (ctr_object * myself, ctr_argument * argumentList)
{
  ctr_object *arr = ctr_array_new (CtrStdArray, NULL);
  if (myself->value.avalue->head == myself->value.avalue->tail)
    return arr;
  else
    {
      ctr_argument varg = {0}, *arg = &varg;
      for (ctr_size i = 0; i < myself->value.avalue->head - myself->value.avalue->tail - 1; i++)
	{
	  arg->object = ctr_build_number_from_float (i);
	  arg->object = ctr_send_message (myself, "at:", 3, arg);
	  ctr_send_message (arr, "push:", 5, arg);
	}
    }
  arr->value.avalue->immutable = myself->value.avalue->immutable;
  return arr;
}

/**
 * [Array] last
 * see also (head, tail, init)
 *
 * returns all of the array sans the last element
 * Does generate exceptions when array is empty
 */
ctr_object *
ctr_array_last (ctr_object * myself, ctr_argument * argumentList)
{
  return ctr_send_message_variadic (myself, "at:", 3, 1, ctr_build_number_from_float (myself->value.avalue->head - myself->value.avalue->tail - 1));
}

/**
 * [Array] + [Array]
 *
 * Returns a new array, containing elements of itself and the other
 * array.
 */
ctr_object *
ctr_array_add (ctr_object * myself, ctr_argument * argumentList)
{
  ctr_object *otherArray = argumentList->object;
  ctr_object *newArray = ctr_array_new (CtrStdArray, NULL);
  ctr_argument parg = {0}, *pushArg = &parg;
  ctr_argument earg = {0}, *elnumArg = &earg;
  ctr_size i;
  for (i = 0; i < myself->value.avalue->head - myself->value.avalue->tail; i++)
    {
      ctr_object *elnum = ctr_build_number_from_float ((ctr_number) i);
      elnumArg->object = elnum;
      pushArg->object = ctr_array_get (myself, elnumArg);
      ctr_array_push (newArray, pushArg);
    }
  if (otherArray->info.type == CTR_OBJECT_TYPE_OTARRAY)
    {
      for (i = otherArray->value.avalue->tail; i < otherArray->value.avalue->head; i++)
	{
	  ctr_object *elnum = ctr_build_number_from_float ((ctr_number) i);
	  elnumArg->object = elnum;
	  pushArg->object = ctr_array_get (otherArray, elnumArg);
	  ctr_array_push (newArray, pushArg);
	}
    }
  return newArray;
}

/**
 * Array fmap: [Block]
 *
 * Maps a function over the block. this function should accept a single value.
 *
 * [1,2,3,4] fmap: {:v ^v + 1.}. #=> Array new < 2 ; 3 ; 4 ; 5
 */
ctr_object *
ctr_array_fmap (ctr_object * myself, ctr_argument * argumentList)
{
  ctr_object *func = argumentList->object;
  CTR_ENSURE_TYPE_BLOCK (func);

  ctr_object *newArray = ctr_array_new (CtrStdArray, NULL);
  ctr_argument varg = {0}, *arg = &varg;
  ctr_size i;
  ctr_object** elems = myself->value.avalue->elements;
  for (i = 0; i < myself->value.avalue->head - myself->value.avalue->tail; i++)
    {
      arg->object = elems[i];
      arg->object = ctr_block_run (func, arg, func);
      if (CtrStdFlow)
	{
	  if (CtrStdFlow == CtrStdContinue)
	    {
	      CtrStdFlow = NULL;
	      continue;
	    }
	  if (CtrStdFlow == CtrStdBreak)
	    CtrStdFlow = NULL;
	  break;
	}
      ctr_array_push (newArray, arg);
    }
  return newArray;
}

/**
 * Array imap: [Block]
 *
 * Maps a function over the block. this function should accept an index and a value.
 *
 * [1,2,3,4] imap: {:i:v ^v + i.}. #=> Array new < 1 ; 3 ; 5 ; 7
 */
ctr_object *
ctr_array_imap (ctr_object * myself, ctr_argument * argumentList)
{
  ctr_object *func = argumentList->object;
  CTR_ENSURE_TYPE_BLOCK (func);

  ctr_object *newArray = ctr_array_new (CtrStdArray, NULL);
  ctr_argument parg = {0}, *pushArg = &parg, pnext = {0};
  pushArg->next = &pnext;
  ctr_size i;
  ctr_object** elements = myself->value.avalue->elements;
  for (i = 0; i < myself->value.avalue->head - myself->value.avalue->tail; i++)
    {
      ctr_object *elnum = ctr_build_number_from_float ((ctr_number) i);
      pushArg->next->object = elements[i];
      pushArg->object = elnum;
      pushArg->object = ctr_block_run (func, pushArg, func);
      ctr_array_push (newArray, pushArg);
    }
  return newArray;
}

/**
 * Array fmap!: [Block]
 *
 * Maps a function over the block. this function should accept a single value.
 * Modifies the collection inplace
 *
 * [1,2,3,4] fmap!: {:v ^v + 1.}. #=> [2,3,4,5]
 */
ctr_object *
ctr_array_fmap_inp (ctr_object * myself, ctr_argument * argumentList)
{
  ctr_object *func = argumentList->object;
  CTR_ENSURE_TYPE_BLOCK (func);

  ctr_argument varg = {0}, *arg = &varg;
  ctr_size i;
  ctr_object** elements = myself->value.avalue->elements;
  for (i = 0; i < myself->value.avalue->head - myself->value.avalue->tail; i++)
    {
      arg->object = elements[i];
      ctr_object* value = ctr_block_run (func, arg, func);
      if (CtrStdFlow)
	{
	  if (CtrStdFlow == CtrStdContinue)
	    {
	      CtrStdFlow = NULL;
	      continue;
	    }
	  if (CtrStdFlow == CtrStdBreak)
	    CtrStdFlow = NULL;
	  break;
	}
    elements[i + myself->value.avalue->tail] = value;
    }
  return myself;
}

/**
 * Array imap!: [Block]
 *
 * Maps a function over the block. this function should accept an index and a value.
 * Modifies the collection inplace
 *
 * [1,2,3,4] imap!: {:i:v ^v + i.}. #=> [1,3,5,7]
 */
ctr_object *
ctr_array_imap_inp (ctr_object * myself, ctr_argument * argumentList)
{
  ctr_object *func = argumentList->object;
  CTR_ENSURE_TYPE_BLOCK (func);

  ctr_argument parg = {0}, *pushArg = &parg, pargnext = {0}, earg, *elnumArg = &earg;
  pushArg->next = &pargnext;
  ctr_size i;
  ctr_object** elements = myself->value.avalue->elements;
  for (i = 0; i < myself->value.avalue->head - myself->value.avalue->tail; i++)
    {
      ctr_object *elnum = ctr_build_number_from_float ((ctr_number) i);
      elnumArg->object = elnum;
      pushArg->next->object = ctr_array_get (myself, elnumArg);
      pushArg->object = elnumArg->object;
      ctr_object* value = ctr_block_run (func, pushArg, func);
      elements[i + myself->value.avalue->tail] = value;
    }
  return myself;
}

/**
 * Array foldl: [Block] accumulator: [Object]
 *
 * reduces an array according to a block (which takes an accumulator and the value, and returns the next acc) from the left (index 0)
 *
 * ([1,2,3,4]) foldl: {:acc:v ^acc + v.} accumulator: 0. #=> Equivalent to ([1,2,3,4]) sum.
 */
ctr_object *
ctr_array_foldl (ctr_object * myself, ctr_argument * argumentList)
{
  ctr_object *func = argumentList->object;
  CTR_ENSURE_TYPE_BLOCK (func);

  ctr_object *accumulator = argumentList->next->object;
  ctr_argument earg = {0}, *elnumArg = &earg;
  ctr_argument aarg = {0}, *accArg = &aarg, anext = {0};
  accArg->next = &anext;
  accArg->object = accumulator;
  ctr_size i;
  for (i = 0; i < myself->value.avalue->head - myself->value.avalue->tail; i++)
    {
      ctr_object *elnum = ctr_build_number_from_float ((ctr_number) i);
      elnumArg->object = elnum;
      accArg->next->object = ctr_array_get (myself, elnumArg);
      accArg->object = ctr_block_run (func, accArg, func);
    }
  accumulator = accArg->object;
  return accumulator;
}

/**
 * Array foldl: [Block]
 *
 * reduces an array according to a block (which takes an accumulator and the value, and returns the next acc) from the left (index 0)
 * Errors if there's no element in the array
 *
 * ([1,2,3,4]) foldl: {:acc:v ^acc + v.}. #=> Equivalent to ([1,2,3,4]) sum.
 */
ctr_object *
ctr_array_foldl0 (ctr_object * myself, ctr_argument * argumentList)
{
  ctr_object *func = argumentList->object;
  CTR_ENSURE_TYPE_BLOCK (func);

  ctr_size i = 0;
  ctr_argument earg = {0}, *elnumArg = &earg;
  ctr_object *elnum = ctr_build_number_from_float ((ctr_number) i);
  elnumArg->object = elnum;
  ctr_object *accumulator = ctr_array_get(myself, elnumArg);
  if (CtrStdFlow)
    return CtrStdNil;
  ctr_argument aarg = {0}, *accArg = &aarg, anext = {0};
  accArg->next = &anext;
  accArg->object = accumulator;
  for (i = 1; i < myself->value.avalue->head - myself->value.avalue->tail; i++)
    {
      elnumArg->object->value.nvalue = i;
      accArg->next->object = ctr_array_get (myself, elnumArg);
      accArg->object = ctr_block_run (func, accArg, func);
    }
  accumulator = accArg->object;
  return accumulator;
}

/**
 * [Array] filter: [Block<i,v>]
 *
 * Include the element iff block returns True for the element
 */
ctr_object *
ctr_array_filter (ctr_object * myself, ctr_argument * argumentList)
{
  ctr_object *block = argumentList->object;
  CTR_ENSURE_TYPE_BLOCK (block);
  ctr_size i = 0;
  block->info.sticky = 1;
  ctr_argument args = {0}, *arguments = &args;
  ctr_argument arg2 = {0}, *argument2 = &arg2;
  arguments->next = argument2;
  ctr_object *newArr = ctr_array_new (CtrStdArray, NULL);
  ctr_object *current;
  for (i = myself->value.avalue->tail; i < myself->value.avalue->head; i++)
    {
      current = *(myself->value.avalue->elements + i);
      arguments->object = ctr_build_number_from_float (i);
      argument2->object = current;
      ctr_object *fv = ctr_block_run (block, arguments, block);
      if (ctr_internal_cast2bool (fv)->value.bvalue)
	{
	  argument2->object = current;
	  ctr_array_push (newArr, argument2);
	}
      if (CtrStdFlow == CtrStdContinue)
	CtrStdFlow = NULL;
      if (CtrStdFlow)
	break;
    }
  if (CtrStdFlow == CtrStdBreak)
    CtrStdFlow = NULL;		/* consume break */
  block->info.mark = 0;
  block->info.sticky = 0;
  return newArr;
}

/**
 * [Array] filter_v: [Block<v>]
 *
 * Include the element iff block returns True for the element
 */
ctr_object *
ctr_array_filter_v (ctr_object * myself, ctr_argument * argumentList)
{
  ctr_object *block = argumentList->object;
  CTR_ENSURE_TYPE_BLOCK (block);
  ctr_size i = 0;
  block->info.sticky = 1;
  ctr_argument args = {0}, *arguments = &args;
  ctr_object *newArr = ctr_array_new (CtrStdArray, NULL);
  ctr_object *current;
  for (i = myself->value.avalue->tail; i < myself->value.avalue->head; i++)
    {
      current = *(myself->value.avalue->elements + i);
      arguments->object = current;
      ctr_object *fv = ctr_block_run (block, arguments, block);
      if (ctr_internal_cast2bool (fv)->value.bvalue)
	{
	  arguments->object = current;
	  ctr_array_push (newArr, arguments);
	}
      if (CtrStdFlow == CtrStdContinue)
	CtrStdFlow = NULL;
      if (CtrStdFlow)
	break;
    }
  if (CtrStdFlow == CtrStdBreak)
    CtrStdFlow = NULL;		/* consume break */
  block->info.mark = 0;
  block->info.sticky = 0;
  return newArr;
}

/**
 * CtrArrayToArgumentList
 * @internal
 *
 * unpacks an array to an argumentlist
 */
ctr_argument *
ctr_array_to_argument_list (ctr_object * arr, ctr_argument * provided)
{
  if (!arr)
    return NULL;
  ctr_size i = arr->value.avalue->tail, arr_max_len = arr->value.avalue->head - arr->value.avalue->tail;
  if (arr_max_len == 0)
    return provided;
  if (!provided)
    provided = ctr_heap_allocate (sizeof (ctr_argument));
  ctr_argument *arg = provided;
  while (provided)
    {
      if (!provided->next)
	break;
      if (i >= arr_max_len)
	provided->object = NULL;
      else
	provided->object = arr->value.avalue->elements[i];
      provided = provided->next;
      i++;
    }
  while (i < arr_max_len)
    {
      provided->object = arr->value.avalue->elements[i++];
      if (likely (i == arr_max_len))
	{
	  provided->next = NULL;
	  break;
	}
      provided->next = ctr_heap_allocate (sizeof (ctr_argument));
      provided = provided->next;
    }
  return arg;
}

void
ctr_deallocate_argument_list (ctr_argument * argumentList)
{
  ctr_argument *arg;
  while (argumentList)
    {
      arg = argumentList->next;
      ctr_heap_free (argumentList);
      argumentList = arg;
    }
}

void
ctr_nullify_argument_list (ctr_argument * argumentList)
{
  ctr_argument *arg;
  while (argumentList)
    {
      arg = argumentList->next;
      argumentList->object = NULL;
      argumentList = arg;
    }
}

/**
 * [Array] fmap: [Block] from: [Block|Array|Map|supports `items`] [filter: [Block]]
 * @Experimental
 * list comprehension
 */
ctr_object *
ctr_array_select_from_if (ctr_object * myself, ctr_argument * argumentList)
{
  ctr_object *selector = argumentList->object,
    *from = argumentList->next->object,
    *ifexp = argumentList->next->next
    && argumentList->next->next->object ? argumentList->next->next->object : NULL, *result = ctr_array_new (CtrStdArray, NULL), *elem, *old_from;
  ctr_argument *argument = ctr_heap_allocate (sizeof (ctr_argument));
  if (from->info.type == CTR_OBJECT_TYPE_OTBLOCK)
    from = ctr_block_run (from, NULL, NULL);
  if (from->info.type == CTR_OBJECT_TYPE_OTARRAY)
    {
      printf ("%p\n", from);
      for (ctr_size i = from->value.avalue->tail; i < from->value.avalue->head; i++)
	{
	  elem = from->value.avalue->elements[i];
	  if (elem->info.type == CTR_OBJECT_TYPE_OTARRAY)
	    argument = ctr_array_to_argument_list (elem, argument);
	  else
	    {
	      ctr_nullify_argument_list (argument);
	      argument->object = elem;
	    }
	  if (ifexp)
	    {
	      ctr_object *clause = ctr_internal_cast2bool (ctr_block_run (ifexp, argument,
									  ifexp));
	      if (!clause->value.bvalue)
		continue;
	    }
	  ctr_object *maybe = ctr_block_run (selector, argument, selector);
	  argument->object = maybe;
	  ctr_array_push (result, argument);
	}
      ctr_deallocate_argument_list (argument);
      return result;
    }
  else if (ctr_internal_has_responder (from, ctr_build_string_from_cstring ("items")))
    {
      from = ctr_send_message (from, "items", 5, NULL);
      if (from->info.type != CTR_OBJECT_TYPE_OTARRAY)
	{
	  ctr_deallocate_argument_list (argument);
	  CtrStdFlow = ctr_build_string_from_cstring ("Generator 'items' does not return a valid value.");
	  return CtrStdNil;
	}
      argumentList->next->object = from;
      return ctr_array_select_from_if (myself, argumentList);
    }
}

/**
 * [Array] from: [Block|Array|Map|supports `each`] [if: [Block]]
 * @Experimental
 * list comprehension
 */
/**
 * [Array] unpack: [Array:{Ref:string}], [Object:ctx]
 * Element-wise assign
 * (Always prefer using algebraic deconstruction assignments: look at section 'Assignment')
 */

ctr_object *
ctr_array_assign (ctr_object * myself, ctr_argument * argumentList)
{
  ctr_object *to = argumentList->object;
  if (!ctr_reflect_check_bind_valid (myself, to, 0))
    {
      CtrStdFlow = ctr_build_string_from_cstring ("Invalid bind");
      return CtrStdNil;
    }

  ctr_argument earg = {0}, *elnumArg = &earg;
  ctr_argument aarg = {0}, *accArg = &aarg;
  accArg->next = argumentList->next;
  ctr_size i;
  if (to->info.type == CTR_OBJECT_TYPE_OTARRAY)
    {
      int other;
      int saw_catch_all = 0;
      int m_count = myself->value.avalue->head - myself->value.avalue->tail;
      for (other = (i = to->value.avalue->tail); i < to->value.avalue->head; other++, i++)
	{
	  ctr_object *elnum = ctr_build_number_from_float ((ctr_number) i);
	  elnumArg->object = elnum;
	  ctr_object *to_elem;
	  accArg->object = ctr_array_get (to, elnumArg);
	  if (accArg->object->interfaces->link == CtrStdSymbol && accArg->object->value.svalue->vlen == 1
	      && *accArg->object->value.svalue->value == '_')
	    continue;
	  if (accArg->object->interfaces->link == CtrStdSymbol && *accArg->object->value.svalue->value == '*')
	    {
	      //We got a catch-all *var
	      if (saw_catch_all)
		{
		  ctr_object *cc = ctr_build_string_from_cstring ("Pattern (");
		  ctr_send_message_variadic (cc, "append:", 7, 1, to);
		  ctr_send_message_variadic (cc, "append:", 7, 1, ctr_build_string_from_cstring (") cannot contain two starred names."));
		  CtrStdFlow = cc;
		  goto clear;
		}
	      saw_catch_all = 1;
	      accArg->object = ctr_get_or_create_symbol_table_entry (accArg->object->value.svalue->value + 1, accArg->object->value.svalue->vlen - 1);
	      int g_skip = m_count - other - 1;
	      to_elem = ctr_array_new (CtrStdArray, NULL);
	      if (other - to->value.avalue->tail <= myself->value.avalue->head - myself->value.avalue->tail)
		for (int _i = other - to->value.avalue->tail + myself->value.avalue->tail;
		     _i < g_skip + other - to->value.avalue->tail + myself->value.avalue->tail; _i++)
		  {
		    elnumArg->object = myself->value.avalue->elements[_i];
		    ctr_array_push (to_elem, elnumArg);
		  }
	      to_elem->value.avalue->immutable = myself->value.avalue->immutable;
	      // other += g_skip - to->value.avalue->tail;
	    }
	  else
	    {
	      elnumArg->object = ctr_build_number_from_float (other);
	      to_elem = ctr_array_get (myself, elnumArg);
	    }
	  ctr_send_message (to_elem, "unpack:", 7, accArg);
	}
    }
  else if (to->interfaces->link == CtrStdSymbol)
    {
      if (argumentList->object->value.svalue->vlen == 0
	  || (argumentList->object->value.svalue->vlen == 1 && *argumentList->object->value.svalue->value == '_'))
	goto clear;
      ctr_internal_object_add_property (argumentList->next->object ? : ctr_contexts[ctr_context_id], ctr_symbol_as_string (to), myself, 0);
    }
clear:
  return myself;
}

/**
 * @internal
 *
 * Internal sort function, for use with ArraySort.
 * Interfaces with qsort-compatible function.
 */
ctr_object *temp_sorter;
int
ctr_sort_cmp (const void *a, const void *b)
{
  ctr_argument earg = {0}, *arg1 = &earg;
  ctr_argument aarg = {0}, *arg2 = &aarg;
  ctr_object *result;
  ctr_object *numResult;
  arg1->next = arg2;
  arg1->object = *((ctr_object **) a);
  arg2->object = *((ctr_object **) b);
  result = ctr_block_run (temp_sorter, arg1, NULL);
  numResult = ctr_internal_cast2number (result);
  return (int) numResult->value.nvalue;
}

/**
 * [Array] sort: [Block]
 *
 * Sorts the contents of an array using a sort block.
 * Uses qsort.
 */
ctr_object *
ctr_array_sort (ctr_object * myself, ctr_argument * argumentList)
{
  ctr_object *sorter = argumentList->object;
  if (sorter->info.type != CTR_OBJECT_TYPE_OTBLOCK)
    {
      CtrStdFlow = ctr_build_string_from_cstring ("Expected block.");
      CtrStdFlow->info.sticky = 1;
      return myself;
    }
  temp_sorter = sorter;
  qsort ((myself->value.avalue->elements + myself->value.avalue->tail),
	 myself->value.avalue->head - myself->value.avalue->tail, sizeof (ctr_object *), ctr_sort_cmp);
  return myself;
}

/**
 * [Array] toString
 *
 * Returns a string representation of the array and its contents.
 * This representation will be encoded in the Citron language itself and is
 * therefore evallable.
 *
 * Usage:
 *
 * a := Array ← 'hello' ; 'world'.
 * b := a toString.
 * c := b eval.
 * x := c @ 1. #world
 *
 * toString messages are implicitly send by some objects, for instance when
 * attempting to write an Array using a Pen.
 *
 * You can also use the alias 'serialize'.
 */
ctr_object *
ctr_array_to_string (ctr_object * myself, ctr_argument * argumentList)
{
  ctr_size i;
  ctr_object *arrayElement;
  ctr_argument narg = {0}, *newArgumentList;
  ctr_object *string = ctr_build_empty_string ();
  newArgumentList = &narg;
  if (myself->value.avalue->tail == myself->value.avalue->head)
    {
      newArgumentList->object = ctr_build_string_from_cstring (myself->value.avalue->immutable ? "[" : CTR_DICT_CODEGEN_ARRAY_NEW);
      string = ctr_string_append (string, newArgumentList);
    }
  else
    {
      newArgumentList->object = ctr_build_string_from_cstring (myself->value.avalue->immutable ? "[" : CTR_DICT_CODEGEN_ARRAY_NEW_PUSH);
      string = ctr_string_append (string, newArgumentList);
    }
  for (i = myself->value.avalue->tail; i < myself->value.avalue->head; i++)
    {
      arrayElement = *(myself->value.avalue->elements + i);
      if (!arrayElement)
	{
	  newArgumentList->object = ctr_build_string_from_cstring ("':NULLPtr:'");
	  string = ctr_string_append (string, newArgumentList);
	}
      else if (arrayElement == myself)
	{
	  newArgumentList->object = ctr_build_string_from_cstring ("':selfReference:'");
	  string = ctr_string_append (string, newArgumentList);
	}
      else if (arrayElement->info.type == CTR_OBJECT_TYPE_OTBOOL
	       || arrayElement->info.type == CTR_OBJECT_TYPE_OTNUMBER || arrayElement->info.type == CTR_OBJECT_TYPE_OTNIL)
	{
	  newArgumentList->object = arrayElement;
	  string = ctr_string_append (string, newArgumentList);
	}
      else if (arrayElement->info.type == CTR_OBJECT_TYPE_OTSTRING)
	{
	  newArgumentList->object = ctr_build_string_from_cstring ("'");
	  string = ctr_string_append (string, newArgumentList);
	  newArgumentList->object = ctr_string_quotes_escape (arrayElement, newArgumentList);
	  string = ctr_string_append (string, newArgumentList);
	  newArgumentList->object = ctr_build_string_from_cstring ("'");
	  string = ctr_string_append (string, newArgumentList);
	}
      else if (arrayElement->info.type == CTR_OBJECT_TYPE_OTARRAY && arrayElement->value.avalue->immutable)
	{
	  newArgumentList->object = arrayElement;
	  string = ctr_string_append (string, newArgumentList);
	}
      else
	{
	  newArgumentList->object = ctr_build_string_from_cstring ("(");
	  ctr_string_append (string, newArgumentList);
	  newArgumentList->object = arrayElement;
	  string = ctr_string_append (string, newArgumentList);
	  newArgumentList->object = ctr_build_string_from_cstring (")");
	  ctr_string_append (string, newArgumentList);
	}
      if ((i + 1) < myself->value.avalue->head)
	{
	  newArgumentList->object = ctr_build_string_from_cstring (myself->value.avalue->immutable ? ", " : " ; ");
	  string = ctr_string_append (string, newArgumentList);
	}
    }
  if (myself->value.avalue->immutable)
    {
      newArgumentList->object = ctr_build_string_from_cstring ("]");
      string = ctr_string_append (string, newArgumentList);
    }
  return string;
}

ctr_object *
ctr_array_fill (ctr_object * myself, ctr_argument * argumentList)
{
  ctr_size n;
  ctr_size i;
  ctr_argument narg = {0}, *newArgumentList = &narg;
  n = ctr_internal_cast2number (argumentList->object)->value.nvalue;
  ctr_collection *coll = myself->value.avalue;
  ctr_size empty = (coll->head - coll->tail);
  if (empty < n)
    {
      coll->elements = ctr_heap_reallocate (coll->elements, (coll->length + n - empty) * sizeof (ctr_object *));
      coll->length = n;
    }
  ctr_object *memb = argumentList->next->object;
  ctr_object *res = NULL;
  if (memb->info.type == CTR_OBJECT_TYPE_OTBLOCK || memb->info.type == CTR_OBJECT_TYPE_OTNATFUNC)
    {
      ctr_object *_i = ctr_internal_create_standalone_object (CTR_OBJECT_TYPE_OTNUMBER);
      ctr_set_link_all (_i, CtrStdNumber);
      ctr_open_context ();
      for (i = 0; i < n; i++)
	{
	  _i->value.nvalue = i;
	  newArgumentList->object = _i;
	  res = ctr_block_run_here (memb, newArgumentList, memb);
	  coll->elements[coll->head++] = res;
	}
      ctr_close_context ();
    }
  else
    {
      for (i = 0; i < n; i++)
	{
	  coll->elements[coll->head++] = memb;
	}
    }
  return myself;
}

ctr_object *
ctr_array_column (ctr_object * myself, ctr_argument * argumentList)
{
  ctr_size i;
  ctr_size n;
  ctr_argument narg = {0}, *newArgumentList = &narg;
  ctr_object *newArray;
  ctr_object *element;
  newArray = ctr_array_new (CtrStdArray, NULL);
  n = ctr_internal_cast2number (argumentList->object)->value.nvalue;
  if (n <= 0)
    {
      return newArray;
    }
  for (i = myself->value.avalue->tail; i < myself->value.avalue->head; i++)
    {
      element = *(myself->value.avalue->elements + i);
      if (element->info.type != CTR_OBJECT_TYPE_OTARRAY)
	continue;
      if (n >= element->value.avalue->head)
	continue;
      newArgumentList->object = *(element->value.avalue->elements + element->value.avalue->tail + n);
      ctr_array_push (newArray, newArgumentList);
    }
  return newArray;
}

/**
 * [Array] serialize
 *
 * Alias for [Array] toString.
 *
 * See 'Map serialize' for the reason for this alias.
 */

/**@I_OBJ_DEF Map*/

/**
 * Map
 *
 * Creates a Map object.
 *
 * Usage:
 *
 * files := Map new.
 * files put: 'readme.txt' at: 'textfile'.
 */
ctr_object *
ctr_map_new (ctr_object * myclass, ctr_argument * argumentList)
{
  ctr_object *s = ctr_internal_create_object (CTR_OBJECT_TYPE_OTOBJECT);
  ctr_set_link_all (s, myclass);
  s->value.defvalue = CtrStdNil;
  return s;
}

ctr_object *
ctr_map_new_ (ctr_object * myclass, ctr_argument * argumentList)
{
  ctr_object *s = ctr_internal_create_object (CTR_OBJECT_TYPE_OTOBJECT);
  ctr_set_link_all (s, myclass);
  ctr_object *defaultv = ctr_build_nil ();
  if (argumentList->object)
    {
      switch (argumentList->object->info.type)
	{
	case CTR_OBJECT_TYPE_OTBLOCK:
	  defaultv = argumentList->object;
	  break;
	default:
	  CtrStdFlow = ctr_format_str ("EMap::'new:' expects a block, not %s", ctr_send_message (argumentList->object, "type", 4, NULL));
	}
    }
  s->value.defvalue = defaultv;
  return s;
}

/**
 * [Map] type
 *
 * Returns the string 'Map'.
 *
 **/
ctr_object *
ctr_map_type (ctr_object * myself, ctr_argument * argumentList)
{
  return ctr_build_string_from_cstring ("Map");
}

/**
 * [Map] put: [Element] at: [Key]
 *
 * Puts a key-value pair in a map.
 *
 * Usage:
 *
 * map put: 'hello' at: 'world'.
 *
 */
ctr_object *
ctr_map_put (ctr_object * myself, ctr_argument * argumentList)
{
  char *key;
  long keyLen;
  ctr_object *putValue = argumentList->object;
  ctr_argument *nextArgument = argumentList->next;
  ctr_object *hasher = ctr_get_responder (argumentList->next->object, "iHash", 5);
  if (!hasher)
    {
      ctr_object *putKey;
      putKey = ctr_send_message (nextArgument->object, "toString", 8, NULL);

      /* If developer returns something other than string (ouch, toString), then cast anyway */
      if (putKey->info.type != CTR_OBJECT_TYPE_OTSTRING)
	{
	  putKey = ctr_internal_cast2string (putKey);
	}

      key = ctr_heap_allocate (putKey->value.svalue->vlen * sizeof (char));
      keyLen = putKey->value.svalue->vlen;
      memcpy (key, putKey->value.svalue->value, keyLen);
      ctr_internal_object_delete_property (myself, ctr_build_string (key, keyLen), 0);
      ctr_internal_object_add_property (myself, ctr_build_string (key, keyLen), putValue, 0);
      return myself;
    }
  else
    {
      ctr_object *putKeyHash;
      putKeyHash = ctr_send_message (nextArgument->object, "iHash", 5, NULL);

      /* If developer returns something other than a number (ouch, iHash), then hash that */
      ctr_number hashk;
      if (putKeyHash->info.type != CTR_OBJECT_TYPE_OTNUMBER)
	hashk = ctr_internal_index_hash (ctr_internal_cast2string (nextArgument->object));
      else
	hashk = putKeyHash->value.nvalue;
      ctr_internal_object_delete_property_with_hash (myself, nextArgument->object, *(uint64_t *) (&hashk), 0);
      ctr_internal_object_add_property_with_hash (myself, nextArgument->object, *(uint64_t *) (&hashk), putValue, 0);
      return myself;
    }
}

/**
 * [Map] deleteAt: [key]
 *
 * deletes an entry (Noop if not in the map)
 *
 * Usage:
 *
 * map deleteAt: 'world'.
 *
 */
ctr_object *
ctr_map_rm (ctr_object * myself, ctr_argument * argumentList)
{
  char *key;
  ctr_object *putKey = argumentList->object;
  ctr_argument earg = {0}, *emptyArgumentList = &earg;
  emptyArgumentList->next = NULL;
  emptyArgumentList->object = NULL;
  ctr_object *hasher = ctr_get_responder (putKey, "iHash", 5);
  if (!hasher)
    {
      putKey = ctr_send_message (putKey, "toString", 8, emptyArgumentList);

      /* If developer returns something other than string (ouch, toString), then cast anyway */
      if (putKey->info.type != CTR_OBJECT_TYPE_OTSTRING)
	{
	  putKey = ctr_internal_cast2string (putKey);
	}

      key = ctr_heap_allocate_cstring (putKey);
      ctr_internal_object_delete_property (myself, ctr_build_string_from_cstring (key), 0);
      ctr_heap_free (key);
      return myself;
    }
  else
    {
      ctr_number hashk;
      ctr_object *putKeyHash = ctr_send_message (putKey, "iHash", 5, NULL);

      /* If developer returns something other than a number (ouch, iHash), then hash internally */
      if (putKeyHash->info.type != CTR_OBJECT_TYPE_OTNUMBER)
	hashk = ctr_internal_index_hash (putKey);
      else
	hashk = putKeyHash->value.nvalue;
      ctr_internal_object_delete_property_with_hash (myself, putKey, *(uint64_t *) & hashk, 0);
      return myself;
    }
}

/**
 * [Map] at: [Key]
 *
 * Retrieves the value specified by the key from the map.
 */
 /**
  * [Map] @ [Key]
  *
  * Alias for [Map] at: [Key].
  *
  */
ctr_object *
ctr_map_get (ctr_object * myself, ctr_argument * argumentList)
{
  ctr_object *searchKey;
  ctr_object *foundObject;

  searchKey = argumentList->object;
  ctr_object *hasher = ctr_get_responder (searchKey, "iHash", 5);
  /* Give developer a chance to define a key */
  if (!hasher)
    {
      searchKey = ctr_send_message (searchKey, "toString", 8, NULL);

      /* If developer returns something other than string (ouch, toString), then cast anyway */
      if (searchKey->info.type != CTR_OBJECT_TYPE_OTSTRING)
	{
	  searchKey = ctr_internal_cast2string (searchKey);
	}

      foundObject = ctr_internal_object_find_property (myself, searchKey, 0);
      goto retv;
    }
  else
    {
      ctr_number hashk;
      ctr_object *searchKeyHasho = ctr_send_message (searchKey, "iHash", 5, NULL);
      if (searchKeyHasho->info.type != CTR_OBJECT_TYPE_OTNUMBER)
	{
	  foundObject = ctr_internal_object_find_property (myself, searchKey, 0);
	}
      else
	{
	  hashk = searchKeyHasho->value.nvalue;
	  foundObject = ctr_internal_object_find_property_with_hash (myself, searchKey, *(uint64_t *) & hashk, 0);
	}
    retv:
      if (foundObject == NULL)
	{
	  ctr_object *kvres = myself->value.defvalue;
	  if (kvres == CtrStdNil)
	    foundObject = kvres;
	  else
	    foundObject = ctr_block_run (kvres, argumentList, myself);
	}
      return foundObject;
    }
}

/**
 * [Map] getOrInsert: [Key]
 *
 * Gets a value at key, or returns a placeholder object (modified by &&val is ...)
 *
 * var &val is Map new getOrInsert: 'a'.
 * &&val is 123.
 * #original map => Map put: 123 at: 'a'
 */
ctr_object*
ctr_map_get_or_insert(ctr_object* myself, ctr_argument* argumentList)
{
  ctr_object *searchKey;
  ctr_object *foundObject;

  searchKey = argumentList->object;
  ctr_object *hasher = ctr_get_responder (searchKey, "iHash", 5);
  /* Give developer a chance to define a key */
  if (!hasher)
    {
      searchKey = ctr_send_message (searchKey, "toString", 8, NULL);

      /* If developer returns something other than string (ouch, toString), then cast anyway */
      if (searchKey->info.type != CTR_OBJECT_TYPE_OTSTRING)
  {
    searchKey = ctr_internal_cast2string (searchKey);
  }

      foundObject = ctr_internal_object_find_property_or_create (myself, searchKey, 0);
      goto retv;
    }
  else
    {
      ctr_number hashk;
      ctr_object *searchKeyHasho = ctr_send_message (searchKey, "iHash", 5, NULL);
      if (searchKeyHasho->info.type != CTR_OBJECT_TYPE_OTNUMBER)
  {
    foundObject = ctr_internal_object_find_property (myself, searchKey, 0);
  }
      else
  {
    hashk = searchKeyHasho->value.nvalue;
    foundObject = ctr_internal_object_find_property_or_create_with_hash (myself, searchKey, *(uint64_t *) & hashk, 0);
  }
    retv:
      if (foundObject == NULL)
  {
    ctr_object *kvres = myself->value.defvalue;
    if (kvres == CtrStdNil)
      foundObject = kvres;
    else
      foundObject = ctr_block_run (kvres, argumentList, myself);
  }
      return foundObject;
    }
}

/**
 * [Map] count
 *
 * Returns the number of elements in the map.
 */
ctr_object *
ctr_map_count (ctr_object * myself, ctr_argument * argumentList)
{
  return ctr_build_number_from_float (myself->properties->size);
}

/**
 * [Map] each: [Block]
 *
 * Iterates over the map, passing key-value pairs to the specified block.
 * Note that within an each/map block, 'me' and 'my' refer to the collection.
 */
ctr_object *
ctr_map_each (ctr_object * myself, ctr_argument * argumentList)
{
  ctr_object *block = argumentList->object;
  ctr_mapitem *m;
  if (block->info.type != CTR_OBJECT_TYPE_OTBLOCK)
    {
      CtrStdFlow = ctr_build_string_from_cstring ("Expected Block.");
      CtrStdFlow->info.sticky = 1;
    }
  block->info.sticky = 1;
  m = myself->properties->head;
  ctr_argument args = {0}, *arguments = &args;
  ctr_argument arg2 = {0}, *argument2 = &arg2;
  ctr_argument arg3 = {0}, *argument3 = &arg3;
  while (m && !CtrStdFlow)
    {
      arguments->object = m->key;
      argument2->object = m->value;
      argument3->object = myself;
      arguments->next = argument2;
      argument2->next = argument3;
      ctr_block_run (block, arguments, NULL);
      if (CtrStdFlow == CtrStdContinue)
	CtrStdFlow = NULL;
      m = m->next;
    }
  if (CtrStdFlow == CtrStdBreak)
    CtrStdFlow = NULL;
  block->info.mark = 0;
  block->info.sticky = 0;
  return myself;
}

void ctr_internal_object_set_property_with_hash (ctr_object * owner, ctr_object * key, uint64_t hashKey, ctr_object * value, int is_method);

//TODO: register this {
ctr_object *
ctr_map_merge (ctr_object * myself, ctr_argument * argumentList)
{
  if (!argumentList)
    goto error_invalid_input;
  ctr_object *smap = argumentList->object;
  if (!smap)
    goto error_invalid_input;
  if (ctr_reflect_get_primitive_link (smap) != CtrStdMap)
    goto error_invalid_input;
  ctr_mapitem *s = smap->properties->head;
  ctr_size sl = smap->properties->size;
  for (; sl--;)
    {
      ctr_internal_object_set_property_with_hash (myself, s->key, s->hashKey, s->value, 0);
      s = s->next;
    }
  return myself;
error_invalid_input:
  CtrStdFlow = ctr_build_string_from_cstring ("Expected an argument of type Map");
  return CtrStdNil;
}

ctr_object *
ctr_map_keys (ctr_object * myself, ctr_argument * argumentList)
{
  ctr_object *keys = ctr_array_new (CtrStdArray, NULL);
  ctr_mapitem *s = myself->properties->head;
  ctr_size sl = myself->properties->size;
  ctr_argument arg = { NULL, NULL };
  for (; sl--;)
    {
      arg.object = s->key;
      ctr_array_push (keys, &arg);
      s = s->next;
    }
  return keys;
}

ctr_object *
ctr_map_values (ctr_object * myself, ctr_argument * argumentList)
{
  ctr_object *values = ctr_array_new (CtrStdArray, NULL);
  ctr_mapitem *s = myself->properties->head;
  ctr_size sl = myself->properties->size;
  ctr_argument arg = { NULL, NULL };
  for (; sl--;)
    {
      arg.object = s->value;
      ctr_array_push (values, &arg);
      s = s->next;
    }
  return values;
}

/**
 * [Map] fmap: [Block<key,value>]
 *
 * Iterates over the map, passing the key and the value to the function, and replacing the value with the result
 * (Or itself if a value is not returned)
 *
 */
ctr_object *
ctr_map_fmap (ctr_object * myself, ctr_argument * argumentList)
{
  ctr_object *block = argumentList->object;
  ctr_mapitem *m;
  if (block->info.type != CTR_OBJECT_TYPE_OTBLOCK)
    {
      CtrStdFlow = ctr_build_string_from_cstring ("Expected Block.");
      CtrStdFlow->info.sticky = 1;
      return myself;
    }
  block->info.sticky = 1;
  m = myself->properties->head;
  ctr_argument arg = {0}, *arguments = &arg, varg = {0};
  arguments->next = &varg;
  ctr_object *newmap = ctr_map_new (CtrStdMap, NULL);
  while (m)
    {
      arguments->object = m->key;
      arguments->next->object = m->value;
      arguments->object = ctr_block_run (block, arguments, myself);
      if (CtrStdFlow)
	{
	  if (CtrStdFlow == CtrStdContinue)
	    {
	      CtrStdFlow = NULL;
	      m = m->next;
	      continue;
	    }
	  if (CtrStdFlow == CtrStdBreak)
	    CtrStdFlow = NULL;
	  break;
	}
      if (arguments->object == block)
	{
	  arguments->object = m->key;
	  arguments->next->object = m->value;
	}
      else
	arguments->next->object = m->key;
      ctr_map_put (newmap, arguments);
      m = m->next;
    }
  block->info.mark = 0;
  block->info.sticky = 0;
  return newmap;
}

/**
 * [Map] fmap!: [Block<key,value>]
 *
 * Iterates over the map, passing the key and the value to the function, and replacing the value with the result
 * (Or itself if a value is not returned)
 * Modifies the collection inplace
 *
 */
ctr_object *
ctr_map_fmap_inp (ctr_object * myself, ctr_argument * argumentList)
{
  ctr_object *block = argumentList->object;
  ctr_mapitem *m;
  if (block->info.type != CTR_OBJECT_TYPE_OTBLOCK)
    {
      CtrStdFlow = ctr_build_string_from_cstring ("Expected a Block.");
      CtrStdFlow->info.sticky = 1;
      return myself;
    }
  block->info.sticky = 1;
  m = myself->properties->head;
  ctr_argument arg = {0}, *arguments = &arg, varg = {0};
  arguments->next = &varg;
  ctr_object* value;
  while (m)
    {
      arguments->object = m->key;
      arguments->next->object = m->value;
      value = ctr_block_run (block, arguments, myself);
      if (CtrStdFlow)
	{
	  if (CtrStdFlow == CtrStdContinue)
	    {
	      CtrStdFlow = NULL;
	      m = m->next;
	      continue;
	    }
	  if (CtrStdFlow == CtrStdBreak)
	    CtrStdFlow = NULL;
	  break;
	}
      if (value == block)
	{
	  value = m->value;
	}
      m->value = value;
      m = m->next;
    }
  block->info.mark = 0;
  block->info.sticky = 0;
  return myself;
}

/**
 * [Map] kvmap: [Block]
 *
 * Iterates over the map, passing a tuple of [key, value] to the specified block.
 * And constructs a new Map based off the returned kv-tuple
 * Note that within an each/map block, 'me' and 'my' refer to the collection.
 */
ctr_object *
ctr_map_kvmap (ctr_object * myself, ctr_argument * argumentList)
{
  ctr_object *block = argumentList->object;
  ctr_mapitem *m;
  if (block->info.type != CTR_OBJECT_TYPE_OTBLOCK)
    {
      CtrStdFlow = ctr_build_string_from_cstring ("Expected Block.");
      CtrStdFlow->info.sticky = 1;
      return myself;
    }
  block->info.sticky = 1;
  m = myself->properties->head;
  ctr_argument args = {0}, vargs = {0}, *arguments = &args;
  arguments->next = &vargs;
  ctr_argument largs = {0}, *larguments = &largs;
  ctr_object *kvtup = ctr_array_new (CtrStdArray, NULL);
  ctr_object *newmap = ctr_map_new (CtrStdMap, NULL);
  while (m)
    {
      arguments->object = m->key;
      arguments->next->object = ctr_build_number_from_float (0);
      ctr_array_put (kvtup, arguments);

      arguments->object = m->value;
      arguments->next->object = ctr_build_number_from_float (1);
      ctr_array_put (kvtup, arguments);

      arguments->object = kvtup;
      arguments->object = ctr_block_run (block, arguments, myself);
      if (arguments->object->info.type != CTR_OBJECT_TYPE_OTARRAY)
	{
	  CtrStdFlow = ctr_build_string_from_cstring ("kvmap block must only return a 2-tuple or nothing.");
	  return CtrStdNil;
	}
      if (arguments->object == block)
	{
	  arguments->object = m->key;
	  arguments->next->object = m->value;
	}
      else
	{
	  ctr_object *ret = arguments->object;
	  larguments->object = ctr_build_number_from_float (1);
	  arguments->object = ctr_array_get (ret, larguments);
	  larguments->object = ctr_build_number_from_float (0);
	  arguments->next->object = ctr_array_get (ret, larguments);
	}
      ctr_map_put (newmap, arguments);
      m = m->next;
    }
  kvtup->info.mark = 1;
  block->info.mark = 0;
  block->info.sticky = 0;
  return newmap;
}

/**
 * [Map] kvlist: [Block]
 *
 * Iterates over the map, passing a tuple of [key, value] to the specified block.
 * And constructs a new list based off the returned kv-tuple
 * Note that within an each/map block, 'me' and 'my' refer to the collection.
 */
ctr_object *
ctr_map_kvlist (ctr_object * myself, ctr_argument * argumentList)
{
  ctr_object *block = argumentList->object;
  ctr_mapitem *m;
  if (block->info.type != CTR_OBJECT_TYPE_OTBLOCK)
    {
      CtrStdFlow = ctr_build_string_from_cstring ("Expected Block.");
      CtrStdFlow->info.sticky = 1;
      return myself;
    }
  block->info.sticky = 1;
  m = myself->properties->head;
  ctr_argument args = {0}, vargs = {0}, *arguments = &args;
  arguments->next = &vargs;
  ctr_argument largs = {0}, *larguments = &largs;
  ctr_object *kvtup = ctr_array_new (CtrStdArray, NULL);
  ctr_object *list = ctr_array_new (CtrStdArray, NULL);
  while (m)
    {
      arguments->object = m->key;
      arguments->next->object = ctr_build_number_from_float (0);
      ctr_array_put (kvtup, arguments);

      arguments->object = m->value;
      arguments->next->object = ctr_build_number_from_float (1);
      ctr_array_put (kvtup, arguments);

      arguments->object = kvtup;
      arguments->object = ctr_block_run (block, arguments, myself);
      /* if (arguments->object->info.type != CTR_OBJECT_TYPE_OTARRAY && arguments->object != block)
         {
         CtrStdFlow = ctr_build_string_from_cstring ("kvmap block must only return a 2-tuple or nothing.");
         return CtrStdNil;
         } */
      if (arguments->object == block)
	{
	  ctr_object *lst = ctr_array_new (CtrStdArray, NULL);
	  arguments->object = m->key;
	  ctr_array_push (lst, arguments);
	  arguments->object = m->value;
	  ctr_array_push (lst, arguments);
	  arguments->object = lst;
	}
      ctr_array_push (list, arguments);
      m = m->next;
    }
  kvtup->info.mark = 1;
  block->info.mark = 0;
  block->info.sticky = 0;
  return list;
}

/**
 * [Map] flip
 *
 * flips the keys and the values of the map. (same-value keys will be overwritten)
 */
ctr_object *
ctr_map_flip (ctr_object * myself, ctr_argument * argumentList)
{
  ctr_mapitem *m;
  m = myself->properties->head;
  ctr_object *map_new = ctr_map_new (CtrStdMap, NULL);
  ctr_argument args = {0}, *arguments = &args;
  ctr_argument arg2 = {0}, *argument2 = &arg2;
  while (m)
    {
      arguments->object = m->key;
      argument2->object = m->value;
      arguments->next = argument2;
      ctr_map_put (map_new, arguments);
      m = m->next;
    }
  return map_new;
}

/**
 * [Map] toString
 *
 * Returns a string representation of a map encoded in Citron itself.
 * This will give you an
 * evallable representation of the map and all of its members.
 *
 * Usage
 *
 * m := (Map new) put: 'hello' at: 'world'.
 * x := m toString
 * m := x eval.
 *
 * The toString method is automatically invoked when attempting to
 * print a Map:
 *
 * Pen write: (Map new). #prints Map new.
 *
 * You can also use the alias 'serialize'.
 */
/**
 * [Map] serialize
 *
 * Alias for [Map] toString.
 * A toString message, sometimes implicitly send by other messages like
 * 'Pen write:' will give you a serialized version of the Map or Array.
 * This is far more useful than a 'dumb' textual description of the object like
 * 'array' or 'object'. However, when working with very large Maps or Arrays
 * accidentally dumping the entire contents can be annoying, in this case you can
 * override the toString behaviour but you don't have to remap the original, you
 * can just use this alias. Also, this alias can be used if you want to make
 * the serialization more explicit.
 */
ctr_object *
ctr_map_to_string (ctr_object * myself, ctr_argument * argumentList)
{
  ctr_object *string;
  ctr_mapitem *mapItem;
  ctr_argument nargs = {0}, *newArgumentList = &nargs;
  string = ctr_build_string_from_cstring (CTR_DICT_CODEGEN_MAP_NEW);
  mapItem = myself->properties->head;
  while (mapItem)
    {
      if (!mapItem->value || !mapItem->key)
	goto next;
      newArgumentList->object = ctr_build_string_from_cstring (CTR_DICT_CODEGEN_MAP_PUT);
      ctr_string_append (string, newArgumentList);
      if (mapItem->value == myself)
	{
	  newArgumentList->object = ctr_build_string_from_cstring ("':selfReference:'");
	  ctr_string_append (string, newArgumentList);
	}
      else if (mapItem->value->info.type == CTR_OBJECT_TYPE_OTBOOL
	       || mapItem->value->info.type == CTR_OBJECT_TYPE_OTNUMBER || mapItem->value->info.type == CTR_OBJECT_TYPE_OTNIL)
	{
	  newArgumentList->object = mapItem->value;
	  ctr_string_append (string, newArgumentList);
	}
      else if (mapItem->value->info.type == CTR_OBJECT_TYPE_OTSTRING)
	{
	  newArgumentList->object = ctr_build_string_from_cstring ("'");
	  ctr_string_append (string, newArgumentList);
	  newArgumentList->object = ctr_string_quotes_escape (mapItem->value, newArgumentList);
	  ctr_string_append (string, newArgumentList);
	  newArgumentList->object = ctr_build_string_from_cstring ("'");
	  ctr_string_append (string, newArgumentList);
	}
      else
	{
	  newArgumentList->object = ctr_build_string_from_cstring ("(");
	  ctr_string_append (string, newArgumentList);
	  newArgumentList->object = mapItem->value;
	  ctr_string_append (string, newArgumentList);
	  newArgumentList->object = ctr_build_string_from_cstring (")");
	  ctr_string_append (string, newArgumentList);
	}
      newArgumentList->object = ctr_build_string_from_cstring (CTR_DICT_CODEGEN_MAP_PUT_AT);
      ctr_string_append (string, newArgumentList);
      if (mapItem->key->info.type == CTR_OBJECT_TYPE_OTBOOL
	  || mapItem->key->info.type == CTR_OBJECT_TYPE_OTNUMBER || mapItem->value->info.type == CTR_OBJECT_TYPE_OTNIL)
	{
	  newArgumentList->object = mapItem->key;
	  ctr_string_append (string, newArgumentList);
	}
      else if (mapItem->key->info.type == CTR_OBJECT_TYPE_OTSTRING)
	{
	  newArgumentList->object = ctr_build_string_from_cstring ("'");
	  ctr_string_append (string, newArgumentList);
	  newArgumentList->object = ctr_string_quotes_escape (mapItem->key, newArgumentList);
	  ctr_string_append (string, newArgumentList);
	  newArgumentList->object = ctr_build_string_from_cstring ("'");
	  ctr_string_append (string, newArgumentList);
	}
      else
	{
	  newArgumentList->object = ctr_build_string_from_cstring ("(");
	  ctr_string_append (string, newArgumentList);
	  newArgumentList->object = mapItem->key;
	  ctr_string_append (string, newArgumentList);
	  newArgumentList->object = ctr_build_string_from_cstring (")");
	  ctr_string_append (string, newArgumentList);
	}
    next:
      mapItem = mapItem->next;
      if (mapItem)
	{
	  newArgumentList->object = ctr_build_string_from_cstring (", ");
	  ctr_string_append (string, newArgumentList);
	}
    }
  return string;
}

/**
* [Map] unpack: [Map:{Ref:AlternativeName}], [Object:ctx]
* Key-wise assign
* Give alternative names as the values of the constructor
*
* e.g. (Map new put: 'Hello' at: 'test0', put: 'World' at: 'test1') unpack: (Map cnew: {my test0 is 'a'. my test1 is 'b'.})
*
* (Always prefer using algebraic deconstruction assignments: look at section 'Assignment')
*/

ctr_object *
ctr_map_assign (ctr_object * myself, ctr_argument * argumentList)
{
  ctr_object *to = argumentList->object;
  if (!ctr_reflect_check_bind_valid (myself, to, 0))
    {
      CtrStdFlow = ctr_build_string_from_cstring ("Invalid bind");
      return CtrStdNil;
    }

  ctr_mapitem *mapItem;
  ctr_argument narg = {0}, *newArgumentList = &narg;
  mapItem = myself->properties->head;
  newArgumentList->next = argumentList->next;
  while (mapItem)
    {
      if (mapItem->value == myself)
	{
	  CtrStdFlow = ctr_build_string_from_cstring ("Assign cannot have selfReference.");
	  return myself;
	}
      ctr_object *value;
      if (!(value = ctr_internal_object_find_property (to, mapItem->key, 0)))
	{
	  mapItem = mapItem->next;
	  continue;
	}
      value->info.raw = 1;
      if (!ctr_reflect_check_bind_valid (mapItem->value, value, 0))
	{
	  // // CtrStdFlow = NULL; //get rid of the error, and bind the result back to the name
	  // newArgumentList->object = mapItem->key;
	  // ctr_send_message (mapItem->value, "unpack:", 7, newArgumentList);
	  // value->info.raw = 0;
	  CtrStdFlow = ctr_build_string_from_cstring ("Invalid bind");
	  break;
	}
      else
	{
	  //Is a valid binding, so bind those
	  newArgumentList->object = value;
	  ctr_send_message (mapItem->value, "unpack:", 7, newArgumentList);
	}
      mapItem = mapItem->next;
    }
  return myself;
}

/**
 * [Map] contains: [Object]
 *
 * check if map contains key
 */
ctr_object *
ctr_map_contains (ctr_object * myself, ctr_argument * argumentList)
{
  ctr_object *searchKey;
  ctr_object *foundObject;

  searchKey = argumentList->object;
  ctr_object *hasher = ctr_get_responder (searchKey, "iHash", 5);
  /* Give developer a chance to define a key */
  if (!hasher)
    {
      searchKey = ctr_send_message (searchKey, "toString", 8, NULL);

      /* If developer returns something other than string (ouch, toString), then cast anyway */
      if (searchKey->info.type != CTR_OBJECT_TYPE_OTSTRING)
	{
	  searchKey = ctr_internal_cast2string (searchKey);
	}

      foundObject = ctr_internal_object_find_property (myself, searchKey, 0);
      return ctr_build_bool (foundObject != NULL);
    }
  else
    {
      ctr_number hashk;
      ctr_object *searchKeyHasho = ctr_send_message (searchKey, "iHash", 5, NULL);
      if (searchKeyHasho->info.type != CTR_OBJECT_TYPE_OTNUMBER)
	{
	  foundObject = ctr_internal_object_find_property (myself, searchKey, 0);
	}
      else
	{
	  hashk = searchKeyHasho->value.nvalue;
	  foundObject = ctr_internal_object_find_property_with_hash (myself, searchKey, *(uint64_t *) & hashk, 0);
	}
      return ctr_build_bool (foundObject != NULL);
    }
}
