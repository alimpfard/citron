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

/**
 * [Array] new
 *
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
ctr_object* ctr_array_new(ctr_object* myclass, ctr_argument* argumentList) {
    ctr_object* s = ctr_internal_create_object(CTR_OBJECT_TYPE_OTARRAY);
    s->link = myclass;
    s->value.avalue = (ctr_collection*) ctr_heap_allocate(sizeof(ctr_collection));
    s->value.avalue->immutable = 0;
    s->value.avalue->length = 1;
    s->value.avalue->elements = (ctr_object**) ctr_heap_allocate(sizeof(ctr_object*)*1);
    s->value.avalue->head = 0;
    s->value.avalue->tail = 0;
    return s;
}

/**
 * [Array] copy
 *
 * shallow copy of the array
 **/
ctr_object* ctr_array_copy(ctr_object* myself, ctr_argument* argumentList) {
  ctr_object* arr = ctr_array_new(CtrStdArray, NULL);
  ctr_argument* pushArg = ctr_heap_allocate(sizeof(ctr_argument));
  for(int i=0; i<myself->value.avalue->length; i++) {
    pushArg->object = *(myself->value.avalue->elements+i);
    ctr_array_push(arr, pushArg);
  }
  ctr_heap_free(pushArg);
  return arr;
}

/**
 * [Array] type
 *
 * Returns the string 'Array'.
 *
 **/
ctr_object* ctr_array_type(ctr_object* myself, ctr_argument* argumentList) {
    return ctr_build_string_from_cstring("Array");
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
ctr_object* ctr_array_push(ctr_object* myself, ctr_argument* argumentList) {
  if (myself->value.avalue->immutable) {
    CtrStdFlow = ctr_build_string_from_cstring("Cannot change immutable array's structure");
    return myself;
  }
    ctr_object* pushValue;
    if (myself->value.avalue->length <= (myself->value.avalue->head + 1)) {
        myself->value.avalue->length = myself->value.avalue->length * 3;
        myself->value.avalue->elements = (ctr_object**) ctr_heap_reallocate(myself->value.avalue->elements,
                (sizeof(ctr_object*) * (myself->value.avalue->length))
                );
    }
    pushValue = argumentList->object;
    *(myself->value.avalue->elements + myself->value.avalue->head) = pushValue;
    myself->value.avalue->head++;
    return myself;
}
ctr_object* ctr_array_push_imm(ctr_object* myself, ctr_argument* argumentList) {
    ctr_object* pushValue;
    if (myself->value.avalue->length <= (myself->value.avalue->head + 1)) {
        myself->value.avalue->length = myself->value.avalue->length * 3;
        myself->value.avalue->elements = (ctr_object**) ctr_heap_reallocate(myself->value.avalue->elements,
                (sizeof(ctr_object*) * (myself->value.avalue->length))
                );
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
ctr_object* ctr_array_min(ctr_object* myself, ctr_argument* argumentList) {
    double min = 0;
    double v = 0;
    ctr_object* el;
    size_t i = 0;
    for(i = 0; i < myself->value.avalue->head; i++) {
        el = *(myself->value.avalue->elements + i);
        v = ctr_internal_cast2number(el)->value.nvalue;
        if (i == 0 || v < min) {
            min = v;
        }
    }
    return ctr_build_number_from_float(min);
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
ctr_object* ctr_array_max(ctr_object* myself, ctr_argument* argumentList) {
    double max = 0;
    double v = 0;
    ctr_object* el;
    size_t i = 0;
    for(i = 0; i < myself->value.avalue->head; i++) {
        el = *(myself->value.avalue->elements + i);
        v = ctr_internal_cast2number(el)->value.nvalue;
        if (i == 0 || max < v) {
            max = v;
        }
    }
    return ctr_build_number_from_float(max);
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
ctr_object* ctr_array_sum(ctr_object* myself, ctr_argument* argumentList) {
    ctr_object* sum;
    ctr_object* el;
    size_t i = 0;
    for(i = 0; i < myself->value.avalue->head; i++) {
        if (!i) {
          sum = *myself->value.avalue->elements;
          continue;
        }
        el = *(myself->value.avalue->elements + i);
        sum = ctr_send_message_variadic(sum, "+", 1, 1, el);
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
ctr_object* ctr_array_product(ctr_object* myself, ctr_argument* argumentList) {
  ctr_object* prod;
  ctr_object* el;
  size_t i = 0;
  for(i = 0; i < myself->value.avalue->head; i++) {
      if (!i) {
        prod = *myself->value.avalue->elements;
        continue;
      }
      el = *(myself->value.avalue->elements + i);
      prod = ctr_send_message_variadic(prod, "*", 1, 1, el);
  }
  return (prod);
}

/**
 * [Array] * [o:Number | Array]
 *
 * Repeats the array o times if o is a number, and generates an array multiplication
 * for myself and o if o is an array
 */
ctr_object* ctr_array_multiply(ctr_object* myself, ctr_argument* argumentList) {
  ctr_object* mand = argumentList->object;
  if (mand->info.type == CTR_OBJECT_TYPE_OTNUMBER) {
    int rep = (int)mand->value.nvalue;
    if (rep == 0) return ctr_array_new(CtrStdArray, NULL);
    else if (rep == 1) return myself;
    else {
      rep--;
      ctr_object* newArr = ctr_array_new(CtrStdArray, NULL);
      ctr_argument* arg = ctr_heap_allocate(sizeof(ctr_argument));
      for (;rep>=0;rep--) {
        arg->object = myself;
        newArr = ctr_array_add(newArr, arg);
      }
      ctr_heap_free(arg);
      return newArr;
    }
  }
  if (mand->info.type == CTR_OBJECT_TYPE_OTARRAY) {
    ctr_object* newArr = ctr_array_new(CtrStdArray, NULL);
    ctr_argument* args = ctr_heap_allocate(sizeof(ctr_argument));
    for(int i=myself->value.avalue->tail; i<myself->value.avalue->head; i++) {
      ctr_object* elem0 = *(myself->value.avalue->elements+i);
      for (int j=mand->value.avalue->tail; j<mand->value.avalue->head; j++) {
        ctr_object* lst = ctr_array_new(CtrStdArray, NULL);
        args->object = elem0;
        ctr_array_push(lst, args);
        args->object = *(mand->value.avalue->elements+j);
        ctr_array_push(lst, args);
        lst->value.avalue->immutable = 1;
        args->object = lst;
        ctr_array_push(newArr, args);
      }
    }
    ctr_heap_free(args);
    return newArr;
  }
}

/**
 * [Array] intersperse: [o: Object]
 *
 * places an 'o' between all array elements
 * returns an array
 **/
 ctr_object* ctr_array_intersperse(ctr_object* myself, ctr_argument* argumentList) {
   ctr_object* newArr = ctr_array_new(CtrStdArray, NULL);
   size_t i = 0;
   ctr_argument* pushArg = ctr_heap_allocate(sizeof(ctr_argument));
   for(i = 0; i < myself->value.avalue->head; i++) {
       pushArg->object = *(myself->value.avalue->elements + i);
       newArr = ctr_array_push(newArr, pushArg);
       if (i<myself->value.avalue->head-1) {
         pushArg->object = argumentList->object;
         newArr = ctr_array_push(newArr, pushArg);
     }
   }
   ctr_heap_free(pushArg);
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
ctr_object* ctr_array_map(ctr_object* myself, ctr_argument* argumentList) {
    ctr_object* block = argumentList->object;
    int i = 0;
    if (block->info.type != CTR_OBJECT_TYPE_OTBLOCK) {
        CtrStdFlow = ctr_build_string_from_cstring("Expected Block.");
        CtrStdFlow->info.sticky = 1;
    }
    block->info.sticky = 1;
    ctr_argument* arguments = (ctr_argument*) ctr_heap_allocate( sizeof( ctr_argument ) );
    ctr_argument* argument2 = (ctr_argument*) ctr_heap_allocate( sizeof( ctr_argument ) );
    ctr_argument* argument3 = (ctr_argument*) ctr_heap_allocate( sizeof( ctr_argument ) );

    if (!myself->value.avalue->immutable)
      argument3 = (ctr_argument*) ctr_heap_allocate( sizeof( ctr_argument ) );

    for(i = myself->value.avalue->tail; i < myself->value.avalue->head; i++) {
        arguments->object = ctr_build_number_from_float((double) i);
        argument2->object = *(myself->value.avalue->elements + i);
        if (!myself->value.avalue->immutable)
          argument3->object = myself;
        arguments->next = argument2;
        if (!myself->value.avalue->immutable)
          argument2->next = argument3;
        ctr_block_run(block, arguments, NULL);
        if (CtrStdFlow == CtrStdContinue) CtrStdFlow = NULL;
        if (CtrStdFlow) break;
    }
    ctr_heap_free( arguments );
    ctr_heap_free( argument2 );
    if (!myself->value.avalue->immutable)
      ctr_heap_free( argument3 );
    if (CtrStdFlow == CtrStdBreak) CtrStdFlow = NULL; /* consume break */
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
 * or if you like ASCII-only:
 *
 * a := Array < 1 ; 2 ; 3.
 *
 * Note that the ; symbol here is an alias for 'push:'.
 */
ctr_object* ctr_array_new_and_push(ctr_object* myclass, ctr_argument* argumentList) {
    ctr_object* s = ctr_array_new(myclass, NULL);
    return ctr_array_push(s, argumentList);
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
ctr_object* ctr_array_unshift(ctr_object* myself, ctr_argument* argumentList) {
  if (myself->value.avalue->immutable) {
    CtrStdFlow = ctr_build_string_from_cstring("Cannot change immutable array's structure");
    return myself;
  }
    ctr_object* pushValue = argumentList->object;
    if (myself->value.avalue->tail > 0) {
        myself->value.avalue->tail--;
    } else {
        if (myself->value.avalue->length <= (myself->value.avalue->head + 1)) {
            myself->value.avalue->length = myself->value.avalue->length * 3;
            myself->value.avalue->elements = (ctr_object**) ctr_heap_reallocate(myself->value.avalue->elements, (sizeof(ctr_object*) * (myself->value.avalue->length)));
        }
        myself->value.avalue->head++;
        memmove(myself->value.avalue->elements+1, myself->value.avalue->elements,myself->value.avalue->head*sizeof(ctr_object*));
    }
    *(myself->value.avalue->elements + myself->value.avalue->tail) = pushValue;
    return myself;
}

/**
 * [Array] reverse
 */
ctr_object* ctr_array_reverse(ctr_object* myself, ctr_argument* argumentList) {
  ctr_object* newArr = ctr_array_new(CtrStdArray, NULL);
  int i = ctr_array_count(myself, NULL)->value.nvalue;
  ctr_argument* args = ctr_heap_allocate(sizeof(ctr_argument));
  for (;i>0;i--) {
    args->object = ctr_build_number_from_float(i-1);
    args->object = ctr_array_get(myself, args);
    ctr_array_push(newArr, args);
  }
  ctr_heap_free(args);
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
ctr_object* ctr_array_join(ctr_object* myself, ctr_argument* argumentList) {
    int i;
    char* result;
    ctr_size len = 0;
    ctr_size pos;
    ctr_object* o;
    ctr_object* str;
    ctr_object* resultStr;
    ctr_object* glue = ctr_internal_cast2string(argumentList->object);
    ctr_size glen = glue->value.svalue->vlen;
    for(i=myself->value.avalue->tail; i<myself->value.avalue->head; i++) {
        o = *( myself->value.avalue->elements + i );
        str = ctr_internal_cast2string(o);
        pos = len;
        if (len == 0) {
            len = str->value.svalue->vlen;
            result = ctr_heap_allocate(sizeof(char)*len);
        } else {
            len += str->value.svalue->vlen + glen;
            result = ctr_heap_reallocate(result, sizeof(char)*len );
            memcpy(result+pos, glue->value.svalue->value, glen);
            pos += glen;
        }
        memcpy(result+pos, str->value.svalue->value, str->value.svalue->vlen);
    }
    resultStr = ctr_build_string(result, len);
    if (len > 0) ctr_heap_free( result );
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
ctr_object* ctr_array_get(ctr_object* myself, ctr_argument* argumentList) {
    ctr_object* getIndex = argumentList->object;
    int i;
    if (getIndex->info.type != CTR_OBJECT_TYPE_OTNUMBER) {
        //printf("Index must be number.\n"); exit(1);
        CtrStdFlow = ctr_build_string_from_cstring("Array index must be a number.");
        return CtrStdNil;
    }
    i = (int) getIndex->value.nvalue;
    if (i<0) i += myself->value.avalue->head - myself->value.avalue->tail;
    if (myself->value.avalue->head <= (i + myself->value.avalue->tail) || i < 0) {
        CtrStdFlow = ctr_build_string_from_cstring("Index out of bounds.");
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
ctr_object* ctr_array_index(ctr_object* myself, ctr_argument* argumentList) {
  int i;
  ctr_object* o;
  ctr_object* needle = argumentList->object;
  for(i=myself->value.avalue->tail; i<myself->value.avalue->head; i++) {
      o = *( myself->value.avalue->elements + i );
      if (ctr_internal_object_is_equal(o, needle))
        return ctr_build_number_from_float(i);
  }
  return ctr_build_number_from_float(-1);
}

/**
 * [Array] contains: [Object]
 *
 * Returns whether the array contains the object or not
 */
ctr_object* ctr_array_contains(ctr_object* myself, ctr_argument* argumentList) {
  int i;
  ctr_object* o;
  ctr_object* needle = argumentList->object;
  for(i=myself->value.avalue->tail; i<myself->value.avalue->head; i++) {
      o = *( myself->value.avalue->elements + i );
      if (ctr_internal_object_is_equal(o, needle))
        return ctr_build_bool(1);
  }
  return ctr_build_bool(0);
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
ctr_object* ctr_array_put(ctr_object* myself, ctr_argument* argumentList) {
  if (myself->value.avalue->immutable) {
    CtrStdFlow = ctr_build_string_from_cstring("Cannot change immutable array's structure");
    return myself;
  }
    ctr_object* putValue = argumentList->object;
    ctr_object* putIndex = ctr_internal_cast2number(argumentList->next->object);
    ctr_size putIndexNumber;
    ctr_size head;
    ctr_size tail;

    if (putIndex->value.nvalue < 0) {
        CtrStdFlow = ctr_build_string_from_cstring("Index out of bounds.");
        CtrStdFlow->info.sticky = 1;
        return myself;
    }

    head = (ctr_size) myself->value.avalue->head;
    tail = (ctr_size) myself->value.avalue->tail;
    putIndexNumber = (ctr_size) putIndex->value.nvalue;
    if (head <= putIndexNumber) {
      if (myself->value.avalue->immutable) {
        CtrStdFlow = ctr_build_string_from_cstring("Cannot change immutable array's structure");
        return myself;
      }
        ctr_size j;
        for(j = head; j <= putIndexNumber; j++) {
            ctr_argument* argument;
            argument = (ctr_argument*) ctr_heap_allocate( sizeof( ctr_argument ) );
            argument->object = CtrStdNil;
            ctr_array_push(myself, argument);
            ctr_heap_free( argument );
        }
        myself->value.avalue->head = putIndexNumber + 1;
    }
    if (putIndexNumber < tail) {
        ctr_size j;
        for(j = tail; j > putIndexNumber; j--) {
            *(myself->value.avalue->elements + j) = CtrStdNil;
        }
        myself->value.avalue->tail = putIndexNumber;
    }
    *(myself->value.avalue->elements + putIndexNumber) = putValue;
    return myself;
}

/**
 * [Array] pop
 *
 * Pops off the last element of the array.
 */
ctr_object* ctr_array_pop(ctr_object* myself, ctr_argument* argumentList) {
  if (myself->value.avalue->immutable) {
    CtrStdFlow = ctr_build_string_from_cstring("Cannot change immutable array's structure");
    return myself;
  }
    if (myself->value.avalue->tail >= myself->value.avalue->head) {
        return CtrStdNil;
    }
    myself->value.avalue->head--;
    return *(myself->value.avalue->elements + myself->value.avalue->head);
}

/**
 * [Array] shift
 *
 * Shifts off the first element of the array.
 */
ctr_object* ctr_array_shift(ctr_object* myself, ctr_argument* argumentList) {
  if (myself->value.avalue->immutable) {
    CtrStdFlow = ctr_build_string_from_cstring("Cannot change immutable array's structure");
    return myself;
  }
    ctr_object* shiftedOff;
    if (myself->value.avalue->tail >= myself->value.avalue->head) {
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
ctr_object* ctr_array_count(ctr_object* myself, ctr_argument* argumentList) {
    ctr_number d = 0;
    d = (ctr_number) myself->value.avalue->head - myself->value.avalue->tail;
    return ctr_build_number_from_float( (ctr_number) d );
}

/**
 * [Array] from: [Begin] length: [End]
 *
 * Copies part of an array indicated by from and to and
 * returns a new array consisting of a copy of this region.
 */
ctr_object* ctr_array_from_length(ctr_object* myself, ctr_argument* argumentList) {
    ctr_argument* pushArg;
    ctr_argument* elnumArg;
    ctr_object* elnum;
    ctr_object* startElement = ctr_internal_cast2number(argumentList->object);
    ctr_object* count = ctr_internal_cast2number(argumentList->next->object);
    int start = (int) startElement->value.nvalue;
    int len = (int) count->value.nvalue;
    int i = 0;
    ctr_object* newArray = ctr_array_new(CtrStdArray, NULL);
    for(i = start; i < start + len; i++) {
        pushArg = (ctr_argument*) ctr_heap_allocate( sizeof( ctr_argument ) );
        elnumArg = (ctr_argument*) ctr_heap_allocate( sizeof( ctr_argument ) );
        elnum = ctr_build_number_from_float((ctr_number) i);
        elnumArg->object = elnum;
        pushArg->object = ctr_array_get(myself, elnumArg);
        ctr_array_push(newArray, pushArg);
        ctr_heap_free( elnumArg );
        ctr_heap_free( pushArg );
    }
    return newArray;
}

/**
 * [Array] head
 * see also (tail, init, last)
 *
 * returns the first element of the array
 * Does generate exceptions when array is empty
 */
ctr_object* ctr_array_head(ctr_object* myself, ctr_argument* argumentList) {
    return ctr_send_message_variadic(myself, "at:", 3, 1, ctr_build_number_from_float(0));
}

/**
 * [Array] tail
 * see also (head, init, last)
 *
 * returns all of the array sans the first element
 * Does not generate exceptions when array is empty
 */
ctr_object* ctr_array_tail(ctr_object* myself, ctr_argument* argumentList) {
    ctr_object* arr = ctr_array_new(CtrStdArray, NULL);
    if (myself->value.avalue->head == myself->value.avalue->tail)
      return arr;
    else {
      ctr_argument* arg = ctr_heap_allocate(sizeof(ctr_argument));
      for(int i=1; i<myself->value.avalue->head - myself->value.avalue->tail; i++) {
        arg->object = ctr_build_number_from_float(i);
        arg->object = ctr_send_message(myself, "at:", 3, arg);
        ctr_send_message(arr, "push:", 5, arg);
      }
      ctr_heap_free(arg);
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
ctr_object* ctr_array_init(ctr_object* myself, ctr_argument* argumentList) {
    ctr_object* arr = ctr_array_new(CtrStdArray, NULL);
    if (myself->value.avalue->head == myself->value.avalue->tail)
      return arr;
    else {
      ctr_argument* arg = ctr_heap_allocate(sizeof(ctr_argument));
      for(int i=0; i<myself->value.avalue->head - myself->value.avalue->tail-1; i++) {
        arg->object = ctr_build_number_from_float(i);
        arg->object = ctr_send_message(myself, "at:", 3, arg);
        ctr_send_message(arr, "push:", 5, arg);
      }
      ctr_heap_free(arg);
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
ctr_object* ctr_array_last(ctr_object* myself, ctr_argument* argumentList) {
    return ctr_send_message_variadic(myself, "at:", 3, 1, ctr_build_number_from_float(myself->value.avalue->head-myself->value.avalue->tail-1));
}

/**
 * [Array] + [Array]
 *
 * Returns a new array, containing elements of itself and the other
 * array.
 */
ctr_object* ctr_array_add(ctr_object* myself, ctr_argument* argumentList) {
    ctr_object* otherArray = argumentList->object;
    ctr_object* newArray = ctr_array_new(CtrStdArray, NULL);
    ctr_argument* pushArg = (ctr_argument*) ctr_heap_allocate( sizeof( ctr_argument ) );
    ctr_argument* elnumArg = (ctr_argument*) ctr_heap_allocate( sizeof( ctr_argument ) );
    int i;
    for(i = myself->value.avalue->tail; i<myself->value.avalue->head; i++) {
        ctr_object* elnum = ctr_build_number_from_float((ctr_number) i);
        elnumArg->object = elnum;
        pushArg->object = ctr_array_get(myself, elnumArg);
        ctr_array_push(newArray, pushArg);
    }
    if (otherArray->info.type == CTR_OBJECT_TYPE_OTARRAY) {
        for(i = otherArray->value.avalue->tail; i<otherArray->value.avalue->head; i++) {
            ctr_object* elnum = ctr_build_number_from_float((ctr_number) i);
            elnumArg->object = elnum;
            pushArg->object = ctr_array_get(otherArray, elnumArg);
            ctr_array_push(newArray, pushArg);
        }
        ctr_heap_free( elnumArg );
        ctr_heap_free( pushArg );
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
ctr_object* ctr_array_fmap(ctr_object* myself, ctr_argument* argumentList) {
  ctr_object* func = argumentList->object;
  CTR_ENSURE_TYPE_BLOCK(func);

  ctr_object* newArray = ctr_array_new(CtrStdArray, NULL);
  ctr_argument* pushArg = (ctr_argument*) ctr_heap_allocate( sizeof( ctr_argument ) );
  ctr_argument* elnumArg = (ctr_argument*) ctr_heap_allocate( sizeof( ctr_argument ) );
  int i;
  for(i = myself->value.avalue->tail; i<myself->value.avalue->head; i++) {
      ctr_object* elnum = ctr_build_number_from_float((ctr_number) i);
      elnumArg->object = elnum;
      pushArg->object = ctr_array_get(myself, elnumArg);
      pushArg->object = ctr_block_run(func, pushArg, func);
      ctr_array_push(newArray, pushArg);
  }
  ctr_heap_free(elnumArg);
  ctr_heap_free(pushArg);
  return newArray;
}

/**
 * Array imap: [Block]
 *
 * Maps a function over the block. this function should accept an index and a value.
 *
 * [1,2,3,4] imap: {:i:v ^v + i.}. #=> Array new < 1 ; 3 ; 5 ; 7
 */
ctr_object* ctr_array_imap(ctr_object* myself, ctr_argument* argumentList) {
  ctr_object* func = argumentList->object;
  CTR_ENSURE_TYPE_BLOCK(func);

  ctr_object* newArray = ctr_array_new(CtrStdArray, NULL);
  ctr_argument* pushArg = (ctr_argument*) ctr_heap_allocate( sizeof( ctr_argument ) );
  pushArg->next = (ctr_argument*) ctr_heap_allocate( sizeof( ctr_argument ) );
  ctr_argument* elnumArg = (ctr_argument*) ctr_heap_allocate( sizeof( ctr_argument ) );
  int i;
  for(i = myself->value.avalue->tail; i<myself->value.avalue->head; i++) {
      ctr_object* elnum = ctr_build_number_from_float((ctr_number) i);
      elnumArg->object = elnum;
      pushArg->next->object = ctr_array_get(myself, elnumArg);
      pushArg->object = elnumArg->object;
      pushArg->object = ctr_block_run(func, pushArg, func);
      ctr_array_push(newArray, pushArg);
  }
  ctr_heap_free(elnumArg);
  ctr_heap_free(pushArg->next);
  ctr_heap_free(pushArg);
  return newArray;
}

/**
 * Array foldl: [Block]
 *
 * reduces an array according to a block (which takes an accumulator and the value, and returns the next acc) from the left (index 0)
 *
 * ([1,2,3,4]) foldl: {:acc:v ^acc + v.}. #=> Equivalent to ([1,2,3,4]) sum.
 */
 ctr_object* ctr_array_foldl(ctr_object* myself, ctr_argument* argumentList) {
   ctr_object* func = argumentList->object;
   CTR_ENSURE_TYPE_BLOCK(func);

   ctr_object* accumulator = argumentList->next->object;
   ctr_argument* elnumArg = (ctr_argument*) ctr_heap_allocate( sizeof( ctr_argument ) );
   ctr_argument* accArg = (ctr_argument*) ctr_heap_allocate( sizeof( ctr_argument ) );
   accArg->next = (ctr_argument*) ctr_heap_allocate( sizeof( ctr_argument ) );
   accArg->object = accumulator;
   int i;
   for(i = myself->value.avalue->tail; i<myself->value.avalue->head; i++) {
       ctr_object* elnum = ctr_build_number_from_float((ctr_number) i);
       elnumArg->object = elnum;
       accArg->next->object = ctr_array_get(myself, elnumArg);
       accArg->object = ctr_block_run(func, accArg, func);
   }
   accumulator = accArg->object;
   ctr_heap_free(elnumArg);
   ctr_heap_free(accArg->next);
   ctr_heap_free(accArg);
   return accumulator;
 }

/**
 * [Array] filter: [Block]
 *
 * Include the element iff block returns True for the element
 */
  ctr_object* ctr_array_filter(ctr_object* myself, ctr_argument* argumentList) {
    ctr_object* func = argumentList->object;
    CTR_ENSURE_TYPE_BLOCK(func);

    ctr_object* newArray = ctr_array_new(CtrStdArray, NULL);
    ctr_argument* elnumArg = (ctr_argument*) ctr_heap_allocate( sizeof( ctr_argument ) );
    ctr_argument* accArg = (ctr_argument*) ctr_heap_allocate( sizeof( ctr_argument ) );
    accArg->next = (ctr_argument*) ctr_heap_allocate( sizeof( ctr_argument ) );
    int i;
    for(i = myself->value.avalue->tail; i<myself->value.avalue->head; i++) {
        ctr_object* elnum = ctr_build_number_from_float((ctr_number) i);
        elnumArg->object = elnum;
        accArg->next->object = ctr_array_get(myself, elnumArg);
        accArg->object = elnum;
        if (ctr_internal_cast2bool(ctr_block_run(func, accArg, func))->value.bvalue) {
          accArg->object = accArg->next->object;
          accArg->next->object = NULL;
          ctr_array_push(newArray, accArg);
        }
    }
    ctr_heap_free(elnumArg);
    ctr_heap_free(accArg->next);
    ctr_heap_free(accArg);
    return newArray;
  }

  /**
   * [Array] unpack: [Array:{Ref:string}]
   * Element-wise assign
   */

  ctr_object* ctr_array_assign(ctr_object* myself, ctr_argument* argumentList) {
    ctr_object* to = argumentList->object;
    if (!ctr_reflect_check_bind_valid(myself, to)) return CtrStdNil;

    ctr_argument* elnumArg = (ctr_argument*) ctr_heap_allocate( sizeof( ctr_argument ) );
    ctr_argument* accArg = (ctr_argument*) ctr_heap_allocate( sizeof( ctr_argument ) );
    int i;
    if (to->info.type == CTR_OBJECT_TYPE_OTARRAY) {
      int other;
      int saw_catch_all = 0;
      for(other=(i = to->value.avalue->tail); i<to->value.avalue->head; other++,i++) {
          ctr_object* elnum = ctr_build_number_from_float((ctr_number) i);
          elnumArg->object = elnum;
          ctr_object* to_elem;
          accArg->object = ctr_array_get(to, elnumArg);
          if (
            accArg->object->info.type == CTR_OBJECT_TYPE_OTSTRING &&
            ctr_internal_object_is_equal(
              ctr_build_string_from_cstring("*"),
              ctr_send_message_variadic(accArg->object, "at:", 3, 1, ctr_build_number_from_float(0))
            )) {
            //We got a catch-all *var
            if (saw_catch_all) {
              ctr_object* cc = ctr_build_string_from_cstring("Pattern (");
              ctr_send_message_variadic(cc, "append:", 7, 1, to);
              ctr_send_message_variadic(cc, "append:", 7, 1, ctr_build_string_from_cstring(") cannot contain two starred names."));
              CtrStdFlow = cc;
              goto clear;
            }
            saw_catch_all=1;
            accArg->object = ctr_send_message_variadic(accArg->object, "skip:", 5, 1, ctr_build_number_from_float(1)); //skip the '*'
            accArg->object->info.raw = 1;
            int skip = ctr_array_count(myself, NULL)->value.nvalue - to->value.avalue->head-i;
            to_elem = ctr_send_message_variadic(myself, "from:length:", 12, 2, ctr_build_number_from_float(other-to->value.avalue->tail), ctr_build_number_from_float(skip+other-to->value.avalue->tail+1));
            to_elem->value.avalue->immutable = myself->value.avalue->immutable;
            other+=skip+other-to->value.avalue->tail;
          } else {
            elnumArg->object = ctr_build_number_from_float(other);
            to_elem = ctr_array_get(myself, elnumArg);
          }
          ctr_send_message(to_elem, "unpack:", 7, accArg);
      }
    } else if (to->info.type == CTR_OBJECT_TYPE_OTSTRING) {
      if (ctr_internal_object_is_equal(argumentList->object, ctr_build_string_from_cstring("_")) || ctr_internal_object_is_equal(argumentList->object, ctr_build_empty_string()))
        goto clear;
      ctr_internal_object_add_property(ctr_contexts[ctr_context_id], to, myself, 0);
    }
    clear:
    ctr_heap_free(elnumArg);
    ctr_heap_free(accArg);
    return myself;
  }

/**
 * @internal
 *
 * Internal sort function, for use with ArraySort.
 * Interfaces with qsort-compatible function.
 */
ctr_object* temp_sorter;
int ctr_sort_cmp(const void * a, const void * b) {
    ctr_argument* arg1 = (ctr_argument*) ctr_heap_allocate( sizeof( ctr_argument ) );
    ctr_argument* arg2 = (ctr_argument*) ctr_heap_allocate( sizeof( ctr_argument ) );
    ctr_object* result;
    ctr_object* numResult;
    arg1->next = arg2;
    arg1->object = *((ctr_object**) a);
    arg2->object = *((ctr_object**) b);
    result = ctr_block_run(temp_sorter, arg1, NULL);
    numResult = ctr_internal_cast2number(result);
    ctr_heap_free( arg1 );
    ctr_heap_free( arg2 );
    return (int) numResult->value.nvalue;
}

/**
 * [Array] sort: [Block]
 *
 * Sorts the contents of an array using a sort block.
 * Uses qsort.
 */
ctr_object* ctr_array_sort(ctr_object* myself, ctr_argument* argumentList) {
    ctr_object* sorter = argumentList->object;
    if (sorter->info.type != CTR_OBJECT_TYPE_OTBLOCK) {
        CtrStdFlow = ctr_build_string_from_cstring("Expected block.");
        CtrStdFlow->info.sticky = 1;
        return myself;
    }
    temp_sorter = sorter;
    qsort((myself->value.avalue->elements+myself->value.avalue->tail), myself->value.avalue->head-myself->value.avalue->tail, sizeof(ctr_object*), ctr_sort_cmp);
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
ctr_object* ctr_array_to_string( ctr_object* myself, ctr_argument* argumentList ) {
    int i;
    ctr_object* arrayElement;
    ctr_argument* newArgumentList;
    ctr_object* string = ctr_build_empty_string();
    newArgumentList = ctr_heap_allocate( sizeof( ctr_argument ) );
    if ( myself->value.avalue->tail == myself->value.avalue->head ) {
        newArgumentList->object = ctr_build_string_from_cstring( myself->value.avalue->immutable?"[":CTR_DICT_CODEGEN_ARRAY_NEW );
        string = ctr_string_append( string, newArgumentList );
    } else {
        newArgumentList->object = ctr_build_string_from_cstring( myself->value.avalue->immutable?"[":CTR_DICT_CODEGEN_ARRAY_NEW_PUSH );
        string = ctr_string_append( string, newArgumentList );
    }
    for(i=myself->value.avalue->tail; i<myself->value.avalue->head; i++) {
        arrayElement = *( myself->value.avalue->elements + i );
        if (arrayElement == myself) {
          newArgumentList->object = ctr_build_string_from_cstring("':selfReference:'");
          string = ctr_string_append( string, newArgumentList );
        }
        else if ( arrayElement->info.type == CTR_OBJECT_TYPE_OTBOOL || arrayElement->info.type == CTR_OBJECT_TYPE_OTNUMBER
                || arrayElement->info.type == CTR_OBJECT_TYPE_OTNIL ) {
            newArgumentList->object = arrayElement;
            string = ctr_string_append( string, newArgumentList );
        } else if ( arrayElement->info.type == CTR_OBJECT_TYPE_OTSTRING ) {
            newArgumentList->object = ctr_build_string_from_cstring("'");
            string = ctr_string_append( string, newArgumentList );
            newArgumentList->object = ctr_string_quotes_escape( arrayElement, newArgumentList );
            string = ctr_string_append( string, newArgumentList );
            newArgumentList->object = ctr_build_string_from_cstring("'");
            string = ctr_string_append( string, newArgumentList );
        } else if (arrayElement->info.type == CTR_OBJECT_TYPE_OTARRAY && arrayElement->value.avalue->immutable) {
          newArgumentList->object = arrayElement;
          string = ctr_string_append( string, newArgumentList );
        } else {
            newArgumentList->object = ctr_build_string_from_cstring("(");
            ctr_string_append( string, newArgumentList );
            newArgumentList->object = arrayElement;
            string = ctr_string_append( string, newArgumentList );
            newArgumentList->object = ctr_build_string_from_cstring(")");
            ctr_string_append( string, newArgumentList );
        }
        if (  (i + 1 )<myself->value.avalue->head ) {
            newArgumentList->object = ctr_build_string_from_cstring(myself->value.avalue->immutable?", ":" ; ");
            string = ctr_string_append( string, newArgumentList );
        }
    }
    if (myself->value.avalue->immutable) {
      newArgumentList->object = ctr_build_string_from_cstring("]");
      string = ctr_string_append( string, newArgumentList );
    }
    ctr_heap_free( newArgumentList );
    return string;
}

ctr_object* ctr_array_fill( ctr_object* myself, ctr_argument* argumentList ) {
    size_t n;
    int i;
    ctr_argument* newArgumentList;
    n = ctr_internal_cast2number( argumentList->object )->value.nvalue;
    newArgumentList = ctr_heap_allocate( sizeof(ctr_argument) );
    newArgumentList->object = argumentList->next->object;
    for(i = 0; i < n; i ++ ) {
        ctr_array_push( myself, newArgumentList );
    }
    ctr_heap_free(newArgumentList);
    return myself;
}

ctr_object* ctr_array_column( ctr_object* myself, ctr_argument* argumentList ) {
    int i;
    size_t n;
    ctr_argument* newArgumentList;
    ctr_object* newArray;
    ctr_object* element;
    newArray = ctr_array_new( CtrStdArray, NULL );
    n = ctr_internal_cast2number( argumentList->object )->value.nvalue;
    if ( n <= 0 ) {
        return newArray;
    }
    newArgumentList = ctr_heap_allocate(sizeof(ctr_argument));
    for(i = myself->value.avalue->tail; i < myself->value.avalue->head; i++) {
        element = *(myself->value.avalue->elements + i);
        if ( element->info.type != CTR_OBJECT_TYPE_OTARRAY ) continue;
        if ( n >= element->value.avalue->head ) continue;
        newArgumentList->object = *(element->value.avalue->elements + element->value.avalue->tail + n);
        ctr_array_push( newArray, newArgumentList );
    }
    ctr_heap_free( newArgumentList );
    return newArray;
}

/**
 * [Array] serialize
 *
 * Alias for [Array] toString.
 *
 * See 'Map serialize' for the reason for this alias.
 */

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
ctr_object* ctr_map_new(ctr_object* myclass, ctr_argument* argumentList) {
    ctr_object* s = ctr_internal_create_object(CTR_OBJECT_TYPE_OTOBJECT);
    s->link = myclass;
    return s;
}

/**
 * [Map] type
 *
 * Returns the string 'Map'.
 *
 **/
ctr_object* ctr_map_type(ctr_object* myself, ctr_argument* argumentList) {
    return ctr_build_string_from_cstring("Map");
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
ctr_object* ctr_map_put(ctr_object* myself, ctr_argument* argumentList) {
    char* key;
    long keyLen;
    ctr_object* putKey;
    ctr_object* putValue = argumentList->object;
    ctr_argument* nextArgument = argumentList->next;
    ctr_argument* emptyArgumentList = ctr_heap_allocate(sizeof(ctr_argument));
    emptyArgumentList->next = NULL;
    emptyArgumentList->object = NULL;

    putKey = ctr_send_message(nextArgument->object, "toString", 8, emptyArgumentList);

    /* If developer returns something other than string (ouch, toString), then cast anyway */
    if (putKey->info.type != CTR_OBJECT_TYPE_OTSTRING) {
        putKey = ctr_internal_cast2string(putKey);
    }

    key = ctr_heap_allocate( putKey->value.svalue->vlen * sizeof( char ) );
    keyLen = putKey->value.svalue->vlen;
    memcpy(key, putKey->value.svalue->value, keyLen);
    ctr_internal_object_delete_property(myself, ctr_build_string(key, keyLen), 0);
    ctr_internal_object_add_property(myself, ctr_build_string(key, keyLen), putValue, 0);
    ctr_heap_free( emptyArgumentList );
    ctr_heap_free( key );
    return myself;
}

/**
 * [Map] at: [Key]
 *
 * Retrieves the value specified by the key from the map.
 */
ctr_object* ctr_map_get(ctr_object* myself, ctr_argument* argumentList) {

    ctr_argument* emptyArgumentList;
    ctr_object*   searchKey;
    ctr_object*   foundObject;

    emptyArgumentList = ctr_heap_allocate(sizeof(ctr_argument));
    emptyArgumentList->next = NULL;
    emptyArgumentList->object = NULL;

    searchKey = argumentList->object;

    /* Give developer a chance to define a key for array */
    searchKey = ctr_send_message(searchKey, "toString", 8, emptyArgumentList);
    ctr_heap_free( emptyArgumentList );

    /* If developer returns something other than string (ouch, toString), then cast anyway */
    if (searchKey->info.type != CTR_OBJECT_TYPE_OTSTRING) {
        searchKey = ctr_internal_cast2string(searchKey);
    }

    foundObject = ctr_internal_object_find_property(myself, searchKey, 0);
    if (foundObject == NULL) foundObject = ctr_build_nil();
    return foundObject;
}

/**
 * [Map] @ [Key]
 *
 * Alias for [Map] at: [Key].
 *
 */

/**
 * [Map] count
 *
 * Returns the number of elements in the map.
 */
ctr_object* ctr_map_count(ctr_object* myself, ctr_argument* argumentList) {
    return ctr_build_number_from_float( myself->properties->size );
}

/**
 * [Map] each: [Block]
 *
 * Iterates over the map, passing key-value pairs to the specified block.
 * Note that within an each/map block, 'me' and 'my' refer to the collection.
 */
ctr_object* ctr_map_each(ctr_object* myself, ctr_argument* argumentList) {
    ctr_object* block = argumentList->object;
    ctr_mapitem* m;
    if (block->info.type != CTR_OBJECT_TYPE_OTBLOCK) {
        CtrStdFlow = ctr_build_string_from_cstring("Expected Block.");
        CtrStdFlow->info.sticky = 1;
    }
    block->info.sticky = 1;
    m = myself->properties->head;
    ctr_argument* arguments = (ctr_argument*) ctr_heap_allocate( sizeof( ctr_argument ) );
    ctr_argument* argument2 = (ctr_argument*) ctr_heap_allocate( sizeof( ctr_argument ) );
    ctr_argument* argument3 = (ctr_argument*) ctr_heap_allocate( sizeof( ctr_argument ) );
    while(m && !CtrStdFlow) {
        arguments->object = m->key;
        argument2->object = m->value;
        argument3->object = myself;
        arguments->next = argument2;
        argument2->next = argument3;
        ctr_block_run(block, arguments, NULL);
        if (CtrStdFlow == CtrStdContinue) CtrStdFlow = NULL;
        m = m->next;
    }
    ctr_heap_free( arguments );
    ctr_heap_free( argument2 );
    ctr_heap_free( argument3 );
    if (CtrStdFlow == CtrStdBreak) CtrStdFlow = NULL;
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
ctr_object* ctr_map_kvmap(ctr_object* myself, ctr_argument* argumentList) {
    ctr_object* block = argumentList->object;
    ctr_mapitem* m;
    if (block->info.type != CTR_OBJECT_TYPE_OTBLOCK) {
        CtrStdFlow = ctr_build_string_from_cstring("Expected Block.");
        CtrStdFlow->info.sticky = 1;
        return myself;
    }
    block->info.sticky = 1;
    m = myself->properties->head;
    ctr_argument* arguments = (ctr_argument*) ctr_heap_allocate( sizeof( ctr_argument ) );
    arguments->next = (ctr_argument*) ctr_heap_allocate( sizeof( ctr_argument ) );
    ctr_argument* larguments = (ctr_argument*) ctr_heap_allocate( sizeof( ctr_argument ) );
    ctr_object* kvtup = ctr_array_new(CtrStdArray, NULL);
    ctr_object* newmap = ctr_map_new(CtrStdMap, NULL);
    while(m) {
        arguments->object = m->key;
        arguments->next->object = ctr_build_number_from_float(0);
        ctr_array_put(kvtup, arguments);

        arguments->object = m->value;
        arguments->next->object = ctr_build_number_from_float(1);
        ctr_array_put(kvtup, arguments);

        arguments->object = kvtup;
        arguments->object = ctr_block_run(block, arguments, NULL);
        if (arguments->object->info.type != CTR_OBJECT_TYPE_OTARRAY) {
          CtrStdFlow = ctr_build_string_from_cstring("kvmap block must only return a 2-tuple or nothing.");
          return CtrStdNil;
        }
        if (arguments->object == myself) {
          arguments->object = m->key;
          arguments->next->object = m->value;
        } else {
          ctr_object* ret = arguments->object;
          larguments->object = ctr_build_number_from_float(1);
          arguments->object = ctr_array_get(ret, larguments);
          larguments->object = ctr_build_number_from_float(0);
          arguments->next->object = ctr_array_get(ret, larguments);
        }
        ctr_map_put(newmap, arguments);
        m = m->next;
    }
    kvtup->info.mark = 1;
    ctr_heap_free( arguments->next );
    ctr_heap_free( arguments );
    ctr_heap_free( larguments );
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
ctr_object* ctr_map_kvlist(ctr_object* myself, ctr_argument* argumentList) {
    ctr_object* block = argumentList->object;
    ctr_mapitem* m;
    if (block->info.type != CTR_OBJECT_TYPE_OTBLOCK) {
        CtrStdFlow = ctr_build_string_from_cstring("Expected Block.");
        CtrStdFlow->info.sticky = 1;
        return myself;
    }
    block->info.sticky = 1;
    m = myself->properties->head;
    ctr_argument* arguments = (ctr_argument*) ctr_heap_allocate( sizeof( ctr_argument ) );
    arguments->next = (ctr_argument*) ctr_heap_allocate( sizeof( ctr_argument ) );
    ctr_argument* larguments = (ctr_argument*) ctr_heap_allocate( sizeof( ctr_argument ) );
    ctr_object* kvtup = ctr_array_new(CtrStdArray, NULL);
    ctr_object* list = ctr_array_new(CtrStdArray, NULL);
    while(m) {
        arguments->object = m->key;
        arguments->next->object = ctr_build_number_from_float(0);
        ctr_array_put(kvtup, arguments);

        arguments->object = m->value;
        arguments->next->object = ctr_build_number_from_float(1);
        ctr_array_put(kvtup, arguments);

        arguments->object = kvtup;
        arguments->object = ctr_block_run(block, arguments, NULL);
        if (arguments->object->info.type != CTR_OBJECT_TYPE_OTARRAY) {
          CtrStdFlow = ctr_build_string_from_cstring("kvmap block must only return a 2-tuple or nothing.");
          return CtrStdNil;
        }
        if (arguments->object == myself) {
          ctr_object* lst = ctr_array_new(CtrStdArray, NULL);
          arguments->object = m->key;
          ctr_array_push(lst, arguments);
          arguments->object = m->value;
          ctr_array_push(lst, arguments);
          arguments->object = lst;
        }
        ctr_array_push(list, arguments);
        m = m->next;
    }
    kvtup->info.mark = 1;
    ctr_heap_free( arguments->next );
    ctr_heap_free( arguments );
    ctr_heap_free( larguments );
    block->info.mark = 0;
    block->info.sticky = 0;
    return list;
}

/**
 * [Map] flip
 *
 * flips the keys and the values of the map. (same-value keys will be overwritten)
 */
ctr_object* ctr_map_flip(ctr_object* myself, ctr_argument* argumentList) {
    ctr_mapitem* m;
    m = myself->properties->head;
    ctr_object* map_new = ctr_map_new(CtrStdMap, NULL);
    ctr_argument* arguments = (ctr_argument*) ctr_heap_allocate( sizeof( ctr_argument ) );
    ctr_argument* argument2 = (ctr_argument*) ctr_heap_allocate( sizeof( ctr_argument ) );
    while(m) {
        arguments->object = m->key;
        argument2->object = m->value;
        arguments->next = argument2;
        ctr_map_put(map_new, arguments);
        m = m->next;
    }
    ctr_heap_free( arguments );
    ctr_heap_free( argument2 );
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
ctr_object* ctr_map_to_string( ctr_object* myself, ctr_argument* argumentList) {
    ctr_object*  string;
    ctr_mapitem* mapItem;
    ctr_argument* newArgumentList;
    string  = ctr_build_string_from_cstring( CTR_DICT_CODEGEN_MAP_NEW );
    mapItem = myself->properties->head;
    newArgumentList = ctr_heap_allocate( sizeof( ctr_argument ) );
    while( mapItem ) {
        newArgumentList->object = ctr_build_string_from_cstring( CTR_DICT_CODEGEN_MAP_PUT );
        ctr_string_append( string, newArgumentList );
        if (mapItem->value == myself) {
          newArgumentList->object = ctr_build_string_from_cstring("':selfReference:'");
          ctr_string_append( string, newArgumentList );
        }
        else if ( mapItem->value->info.type == CTR_OBJECT_TYPE_OTBOOL || mapItem->value->info.type == CTR_OBJECT_TYPE_OTNUMBER
                || mapItem->value->info.type == CTR_OBJECT_TYPE_OTNIL
           ) {
            newArgumentList->object = mapItem->value;
            ctr_string_append( string, newArgumentList );
        } else if ( mapItem->value->info.type == CTR_OBJECT_TYPE_OTSTRING ) {
            newArgumentList->object = ctr_build_string_from_cstring( "'" );
            ctr_string_append( string, newArgumentList );
            newArgumentList->object = ctr_string_quotes_escape( mapItem->value, newArgumentList );
            ctr_string_append( string, newArgumentList );
            newArgumentList->object = ctr_build_string_from_cstring( "'" );
            ctr_string_append( string, newArgumentList );
        } else {
            newArgumentList->object = ctr_build_string_from_cstring( "(" );
            ctr_string_append( string, newArgumentList );
            newArgumentList->object = mapItem->value;
            ctr_string_append( string, newArgumentList );
            newArgumentList->object = ctr_build_string_from_cstring( ")" );
            ctr_string_append( string, newArgumentList );
        }
        newArgumentList->object = ctr_build_string_from_cstring( CTR_DICT_CODEGEN_MAP_PUT_AT );
        ctr_string_append( string, newArgumentList );
        if ( mapItem->key->info.type == CTR_OBJECT_TYPE_OTBOOL || mapItem->key->info.type == CTR_OBJECT_TYPE_OTNUMBER
                || mapItem->value->info.type == CTR_OBJECT_TYPE_OTNIL ) {
            newArgumentList->object = mapItem->key;
            ctr_string_append( string, newArgumentList );
        } else if ( mapItem->key->info.type == CTR_OBJECT_TYPE_OTSTRING ) {
            newArgumentList->object = ctr_build_string_from_cstring( "'" );
            ctr_string_append( string, newArgumentList );
            newArgumentList->object = ctr_string_quotes_escape( mapItem->key, newArgumentList );
            ctr_string_append( string, newArgumentList );
            newArgumentList->object = ctr_build_string_from_cstring( "'" );
            ctr_string_append( string, newArgumentList );
        } else {
            newArgumentList->object = ctr_build_string_from_cstring( "(" );
            ctr_string_append( string, newArgumentList );
            newArgumentList->object = mapItem->key;
            ctr_string_append( string, newArgumentList );
            newArgumentList->object = ctr_build_string_from_cstring( ")" );
            ctr_string_append( string, newArgumentList );
        }
        mapItem = mapItem->next;
        if ( mapItem ) {
            newArgumentList->object = ctr_build_string_from_cstring( ", " );
            ctr_string_append( string, newArgumentList );
        }
    }
    ctr_heap_free( newArgumentList );
    return string;
}

/**
 * [Map] unpack: [Map:{Ref:string}]
 * Key-wise assign
 */

ctr_object* ctr_map_assign(ctr_object* myself, ctr_argument* argumentList) {
  ctr_object* to = argumentList->object;
  if(!ctr_reflect_check_bind_valid(myself, to)) return CtrStdNil;

  ctr_mapitem* mapItem;
  ctr_argument* newArgumentList;
  mapItem = myself->properties->head;
  newArgumentList = ctr_heap_allocate( sizeof( ctr_argument ) );
  while( mapItem ) {
      if (mapItem->value == myself) {
        CtrStdFlow = ctr_build_string_from_cstring("Assign cannot have selfReference.");
        return myself;
      }
      ctr_object* value;
      if(!(value = ctr_internal_object_find_property(to, mapItem->key, 0))) {mapItem = mapItem->next; continue;}
      value->info.raw = 1;
      if (!ctr_reflect_check_bind_valid(mapItem->value, value)) {
        // CtrStdFlow = NULL; //get rid of the error, and bind the result back to the name
        newArgumentList->object = mapItem->key;
        ctr_send_message(mapItem->value, "unpack:", 7, newArgumentList);
        value->info.raw = 0;
      } else {
        //Is a valid binding, so bind those
        newArgumentList->object = value;
        ctr_send_message(mapItem->value, "unpack:", 7, newArgumentList);
      }
      mapItem = mapItem->next;
  }
  ctr_heap_free( newArgumentList );
  return myself;
}

/**
 * Iterator
 *
 * Iterator (range), an object that supports next and current,
 * and steps through some values before (optionally) reaching an end
 */

struct ctr_iters_type {
  ctr_object** ctr_iterator_range_func;
  ctr_object** ctr_iterator_uncapped_range_func;
};
struct ctr_iters_type ctr_iterators = {&ctr_iter_range, &ctr_iter_urange};

ctr_object* ctr_iterator_make(ctr_object* myself, ctr_argument* argumentList) {
  ctr_object* instance = ctr_internal_create_object(CTR_OBJECT_TYPE_OTOBJECT);
  instance->link = myself;
  return instance;
}

/**
 * @internal
 *
 * Iterator set seed
 */
ctr_object* ctr_iterator_set_seed(ctr_object* myself, ctr_argument* argumentList) {
  ctr_internal_object_set_property(
    myself,
    ctr_build_string_from_cstring("seed"),
    argumentList->object,
    CTR_CATEGORY_PRIVATE_PROPERTY
  );
  return myself;
}
/**
 * @internal
 *
 * Iterator set function
 */
ctr_object* ctr_iterator_set_func_(ctr_object* myself, ctr_object* function, ctr_object* end, ctr_object* step) {
  ctr_object* func = ctr_invoke_variadic(CtrStdReflect, &ctr_reflect_fn_copy, 1, function);

  ctr_internal_object_set_property(
    myself,
    ctr_build_string_from_cstring("func"),
    func,
    CTR_CATEGORY_PRIVATE_PROPERTY
  );
  if (end)
  ctr_internal_object_set_property(
    func,
    ctr_build_string_from_cstring("end_value"),
    end,
    CTR_CATEGORY_PRIVATE_PROPERTY
  );
  if (step)
  ctr_internal_object_set_property(
    func,
    ctr_build_string_from_cstring("step"),
    step,
    CTR_CATEGORY_PRIVATE_PROPERTY
  );
  ctr_internal_object_set_property(
    func,
    ctr_build_string_from_cstring("iterator"),
    myself,
    CTR_CATEGORY_PRIVATE_PROPERTY
  );
  return myself;
}

/**
 * [Iterator] setFunc: [f:Block]
 *
 * sets the iterator step function.
 */
ctr_object* ctr_iterator_set_func(ctr_object* myself, ctr_argument* argumentList) {
  ctr_object* func = ctr_reflect_fn_copy(CtrStdReflect, argumentList);

  ctr_internal_object_set_property(
    myself,
    ctr_build_string_from_cstring("func"),
    func,
    CTR_CATEGORY_PRIVATE_PROPERTY
  );
  ctr_internal_object_set_property(
    func,
    ctr_build_string_from_cstring("iterator"),
    myself,
    CTR_CATEGORY_PRIVATE_PROPERTY
  );
  return myself;
}

/**
 * [Iterator] rangeFrom: [f:Number] to: [t:Number] step: [s:Number]
 *
 * makes a range iterator in range [f,t] with steps of s
 *
 */
ctr_object* ctr_iterator_make_range(ctr_object* myself, ctr_argument* argumentList) {
  ctr_argument* argo = argumentList;

  ctr_object* instance = ctr_iterator_make(myself, NULL);
  argo->object = ctr_invoke_variadic(argo->object, &ctr_number_minus, 1, argo->next->next->object);
  ctr_iterator_set_seed (instance, argo);
  argo->object = *ctr_iterators.ctr_iterator_range_func;
  ctr_iterator_set_func_ (instance, argo->object, argo->next->object, argo->next->next->object);

  return instance;
}

/**
 * [Iterator] rangeFrom: [f:(supports +)] step: [s:(supports +)]
 *
 * makes a range iterator in range [f,inf) with steps of s
 * works on all objects that support the message '+'
 */
ctr_object* ctr_iterator_make_uncapped_range(ctr_object* myself, ctr_argument* argumentList) {
  ctr_argument* argo = argumentList;

  ctr_object* instance = ctr_iterator_make(myself, NULL);
  ctr_iterator_set_seed (instance, argo);
  argo->object = *ctr_iterators.ctr_iterator_uncapped_range_func;
  ctr_iterator_set_func_ (instance, argo->object, NULL, argo->next->object);

  return instance;
}

/**
 * [Iterator] next
 *
 * gives the next value or throws an exception (to signal the end of the iterator)
 */
ctr_object* ctr_iterator_next(ctr_object* myself, ctr_argument* argumentList) {
  ctr_object* iter = ctr_internal_object_find_property(myself, ctr_build_string_from_cstring("func"), 0);
  ctr_object* seed = ctr_internal_object_find_property(myself, ctr_build_string_from_cstring("seed"), 0);
  if (!(iter&&seed)) {
    CtrStdFlow = ctr_build_string_from_cstring("Invalid iterator.");
    return myself;
  }
  seed = ctr_block_run_variadic(iter, 1, seed);
  ctr_internal_object_set_property(myself, ctr_build_string_from_cstring("seed"), seed, 0);
  return seed;
}

/**
 * [Iterator] each: [Block<value>]
 *
 * Runs the block for each value in the iterator while stepping through it
 */
ctr_object* ctr_iterator_each(ctr_object* myself, ctr_argument* argumentList) {
  ctr_object* value;
  ctr_object* dothis = argumentList->object;
  ctr_argument* arg = ctr_heap_allocate(sizeof(ctr_argument));
  while (1) {
    value = ctr_iterator_next(myself, NULL);
    if (CtrStdFlow) break;
    arg->object = value;
    ctr_block_runIt(dothis, arg);
  }
  CtrStdFlow = NULL;
  return myself;
}

/**
 * [Iterator] fmap: [b:Block<value>]
 * Equivalent to [Array] fmap: [Block<value>]
 *
 * transforms the iterator with the block to an array
 */
ctr_object* ctr_iterator_fmap(ctr_object* myself, ctr_argument* argumentList) {
  ctr_object* value;
  ctr_object* dothis = argumentList->object;
  ctr_object* ret = ctr_array_new(CtrStdArray, NULL);
  ctr_argument* arg = ctr_heap_allocate(sizeof(ctr_argument));
  while (1) {
    value = ctr_iterator_next(myself, NULL);
    if (CtrStdFlow) break;
    arg->object = value;
    arg->object = ctr_block_runIt(dothis, arg);
    ctr_array_push(ret, arg);
  }
  CtrStdFlow = NULL;
  ctr_heap_free(arg);
  return ret;
}

/**
 * [Iterator] count
 *
 * Steps through the iterator until it ends and returns the count.
 * resets the state of the iterator
 */
ctr_object* ctr_iterator_count(ctr_object* myself, ctr_argument* argumentList) {
  ctr_object* ret = ctr_build_number_from_float(0);
  ctr_object* seed = ctr_internal_object_find_property(myself, ctr_build_string_from_cstring("seed"), 0);
  ctr_argument* arg = ctr_heap_allocate(sizeof(ctr_argument));
  arg->object = ctr_build_number_from_float(1);
  while (1) {
   ctr_iterator_next(myself, NULL);
   if (CtrStdFlow) break;
   ret = ctr_number_add(ret, arg);
  }
  CtrStdFlow = NULL;
  arg->object = seed;
  ctr_iterator_set_seed(myself, arg);
  ctr_heap_free(arg);
  return ret;
}

/**
 * [Iterator] take: [t:Number]
 *
 * takes at most <t> number of elements from the Iterator
 */
ctr_object* ctr_iterator_take(ctr_object* myself, ctr_argument* argumentList) {
  ctr_object* value;
  int count = ctr_internal_cast2number(argumentList->object)->value.nvalue;
  ctr_object* ret = ctr_array_new(CtrStdArray, NULL);
  ctr_argument* arg = ctr_heap_allocate(sizeof(ctr_argument));
  while (count--) {
    value = ctr_iterator_next(myself, NULL);
    if (CtrStdFlow) break;
    arg->object = value;
    ctr_array_push(ret, arg);
  }
  CtrStdFlow = NULL;
  ctr_heap_free(arg);
  return ret;
}

/**
 * [Iterator] takeWhile: [predicate:Block]
 *
 * takes values as long as predicate returns true.
 */
ctr_object* ctr_iterator_takewhile(ctr_object* myself, ctr_argument* argumentList) {
  ctr_object* value;
  ctr_object* pred = argumentList->object;
  ctr_object* last_seed = ctr_internal_object_find_property(myself, ctr_build_string_from_cstring("seed"), 0);
  ctr_object* ret = ctr_array_new(CtrStdArray, NULL);
  ctr_argument* arg = ctr_heap_allocate(sizeof(ctr_argument));
  while(1) {
    if (value) last_seed = value;
    value = ctr_iterator_next(myself, NULL);
    if (CtrStdFlow) break;
    arg->object = value;
    if(!ctr_block_runIt(pred, arg)->value.bvalue) {
      ctr_internal_object_set_property(myself, ctr_build_string_from_cstring("seed"), last_seed, 0);
      break;
    }
    ctr_array_push(ret, arg);
  }
  CtrStdFlow = NULL;
  ctr_heap_free(arg);
  return ret;
}

/**
 * [Iterator] endBlock
 *
 * returns a block that throws an IteratorEndException.
 */
ctr_object* ctr_iterator_end(ctr_object* myself, ctr_argument* argumentList) {
  ctr_object* iterend = ctr_string_eval(ctr_build_string_from_cstring("{thisBlock error: 'IteratorEndException'.}"), NULL);
  return iterend;
}

/**
 * [Iterator] endIf: [clause:Block]
 *
 * ends the Iterator if clause returns True
 */
ctr_object* ctr_iterator_end_check(ctr_object* myself, ctr_argument* argumentList) {
  ctr_object* clause = (ctr_block_run(argumentList->object, NULL, myself));
  if (CtrStdFlow) {
    return myself;
  }
  if (ctr_internal_cast2bool(clause)->value.bvalue)
    CtrStdFlow = ctr_build_string_from_cstring("IteratorEndException");
  return myself;
}

/* Array interface to iterator */
/**
 * [Iterator] toArray
 *
 * Collects all of the iterator values into an array
 */
 ctr_object* ctr_iterator_to_array(ctr_object* myself, ctr_argument* argumentList) {
   ctr_object* value;
   ctr_object* ret = ctr_array_new(CtrStdArray, NULL);
   ctr_argument* arg = ctr_heap_allocate(sizeof(ctr_argument));
   while (1) {
     value = ctr_iterator_next(myself, NULL);
     if (CtrStdFlow) break;
     arg->object = value;
     ctr_array_push(ret, arg);
   }
   CtrStdFlow = NULL;
   ctr_heap_free(arg);
   return ret;
 }

/**
 * [Iterator] skip: [n:Number]
 *
 * skips n iterations and returns the nth value
 * does not reset the evaluation. (iterator will advance to index n)
 */
 ctr_object* ctr_iterator_skip(ctr_object* myself, ctr_argument* argumentList) {
   int i = ctr_internal_cast2number(argumentList->object)->value.nvalue;
   ctr_object* value = NULL;
   for(;i>0;i--) {
     if(CtrStdFlow) break;
     value = ctr_iterator_next(myself, NULL);
   }
   return value?value:ctr_internal_object_find_property(myself, ctr_build_string_from_cstring("seed"), 0);
 }
