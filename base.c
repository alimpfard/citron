#include <ctype.h>
#include <math.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#ifdef POSIXRE
#include <regex.h>
#else
#include "pcrs/pcrs.c"
#include <pcre.h>
#endif
#include <errno.h>

#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>

#include "citron.h"
#include "siphash.h"
#include "symbol.h"

#ifndef POSIXRE
#include "pcre_split.h"
#endif

int more_exception_data = 1;

#include "generator.h"
char *ctr_itoa(int value, char *buffer, int base);
static inline void ctr_assign_block_parameters(ctr_tlistitem *, ctr_argument *,
                                               ctr_object *);

/**@I_OBJ_DEF Nil*/
/**
 *
 * Nil represents 'nothing' or NULL in other languages.
 * Any object property that has not been assigned a value
 * will contain Nil. Unlike some other programming languages
 * Citron has no concept of 'undefined' or isset, Nil is actually the
 * same as 'undefined' or not set.
 *
 * Literal:
 *
 * Nil
 */
ctr_object *ctr_build_nil() { return CtrStdNil; }

/**
 *[Nil] isNil
 *
 * @see ctr_object_is_nil
 *
 * Nil always answers this message with a boolean object 'True'.
 */
ctr_object *ctr_nil_is_nil(ctr_object *myself, ctr_argument *argumentList) {
  return ctr_build_bool(1);
}

/**
 *[Nil] toString
 *
 * @see ctr_object_to_string
 *
 * Returns the string representation of Nil: 'Nil'.
 */
ctr_object *ctr_nil_to_string(ctr_object *myself, ctr_argument *argumentList) {
  return ctr_build_string_from_cstring("Nil");
}

/**
 *[Nil] toNumber
 *
 * @see ctr_object_to_number
 *
 * Returns the numerical representation of Nil: 0.
 */
ctr_object *ctr_nil_to_number(ctr_object *myself,
                              ctr_argument *ctr_argumentList) {
  return ctr_build_number_from_float(0);
}

/**
 *[Nil] toBoolean
 *
 * @see ctr_object_to_boolean
 *
 * Returns the boolean representation of Nil: False.
 */
ctr_object *ctr_nil_to_boolean(ctr_object *myself,
                               ctr_argument *ctr_argumentList) {
  return ctr_build_bool(0);
}

/**
 *[Nil] unpack: [Ref:string], [Object:context]
 *
 * Assigns Nil into Ref
 * (Always prefer using algebraic deconstruction assignments: look at section
 *'Assignment')
 */

ctr_object *ctr_nil_assign(ctr_object *myself, ctr_argument *argumentList) {
  if (!ctr_reflect_check_bind_valid(myself, argumentList->object, 0)) {
    CtrStdFlow = ctr_build_string_from_cstring("Invalid bind");
    return CtrStdNil;
  }
  if (argumentList->object->interfaces->link != CtrStdSymbol) {
    if (!ctr_internal_object_is_equal(myself, argumentList->object)) {
      CtrStdFlow =
          ctr_build_string_from_cstring("Nil cannot be constructed by ");
      ctr_string_append(CtrStdFlow, argumentList);
    }
    return myself;
  }
  if (argumentList->object->value.svalue->vlen == 0 ||
      (argumentList->object->value.svalue->vlen == 1 &&
       *argumentList->object->value.svalue->value == '_'))
    return myself;
  ctr_internal_object_set_property(
      argumentList->next->object ?: ctr_contexts[ctr_context_id],
      ctr_symbol_as_string(argumentList->object), CtrStdNil, 0);
  return myself;
}

/**@I_OBJ_DEF Object*/
/**
 *
 * This is the base object, the parent of all other objects.
 * It contains essential object oriented programming features.
 */
ctr_object *ctr_object_make(ctr_object *myself, ctr_argument *argumentList) {
  ctr_object *objectInstance = NULL;
  objectInstance = ctr_internal_create_object(CTR_OBJECT_TYPE_OTOBJECT);
  ctr_set_link_all(objectInstance, myself);
  return objectInstance;
}

/**
 *Object cnew: [Block]
 *
 * Object constructor with some initial parameters.
 * Object is passed to the block as me/my
 */
ctr_object *ctr_object_ctor(ctr_object *myself, ctr_argument *argumentList) {
  ctr_object *objectInstance = NULL;
  objectInstance = ctr_internal_create_object(CTR_OBJECT_TYPE_OTOBJECT);
  ctr_set_link_all(objectInstance, myself);
  CTR_ENSURE_TYPE_BLOCK(argumentList->object);
  ctr_argument vargs = {0}, *args = &vargs;
  ctr_block_run(argumentList->object, args, objectInstance);
  return objectInstance;
}

/**
 *Object genAccessors: [String]
 *
 * generate reader/writer methods for property.
 * creates the property is it doesn't exist
 */
ctr_object *ctr_object_attr_accessor(ctr_object *myself,
                                     ctr_argument *argumentList) {
  if (argumentList->object->info.type != CTR_OBJECT_TYPE_OTARRAY) {
    ctr_object *name = ctr_internal_cast2string(argumentList->object);
    ctr_object *property = ctr_internal_object_find_property(myself, name, 0);
    if (property == NULL)
      ctr_internal_object_set_property(myself, name, CtrStdNil, 0);

    ctr_argument varg = {0}, *arglist = &varg, vargn;
    arglist->next = &vargn;
    char *mname = ctr_heap_allocate(sizeof(char) * 512);
    sprintf(mname, "{:x my %.*s is x.}", (int)name->value.svalue->vlen,
            name->value.svalue->value);
    arglist->object = ctr_build_string_from_cstring(":");
    arglist->object = ctr_string_concat(name, arglist);
    arglist->next->object =
        ctr_string_eval(ctr_build_string_from_cstring(mname), NULL);
    ctr_object_on_do(myself, arglist);
    sprintf(mname, "{^my %.*s.}", (int)name->value.svalue->vlen,
            name->value.svalue->value);
    arglist->object = name;
    arglist->next->object =
        ctr_string_eval(ctr_build_string_from_cstring(mname), NULL);
    ctr_object_on_do(myself, arglist);
    ctr_heap_free(mname);
    return myself;
  } else {
    ctr_argument vargs = {0}, *args = &vargs;
    ctr_argument earg = {0}, *elnumArg = &earg;
    ctr_size i;
    for (i = argumentList->object->value.avalue->tail;
         i < argumentList->object->value.avalue->head; i++) {
      elnumArg->object = ctr_build_number_from_float((ctr_number)i);
      args->object = ctr_array_get(argumentList->object, elnumArg);
      ctr_object_attr_accessor(myself, args);
    }
    return myself;
  }
}

/**
 *Object genReader: [String]
 *
 * generate reader method for property.
 * creates the property is it doesn't exist
 */
ctr_object *ctr_object_attr_reader(ctr_object *myself,
                                   ctr_argument *argumentList) {
  if (argumentList->object->info.type != CTR_OBJECT_TYPE_OTARRAY) {
    ctr_object *name = ctr_internal_cast2string(argumentList->object);
    ctr_object *property = ctr_internal_object_find_property(myself, name, 0);
    if (property == NULL)
      ctr_internal_object_set_property(myself, name, CtrStdNil, 0);
    ctr_argument varg = {0}, narg = {0}, *arglist = &varg;
    arglist->next = &narg;
    char *mname = ctr_heap_allocate(sizeof(char) * 512);
    sprintf(mname, "{^my %.*s.}", (int)name->value.svalue->vlen,
            name->value.svalue->value);
    arglist->object = ctr_build_string_from_cstring(":");
    arglist->object = ctr_string_concat(name, arglist);
    arglist->next->object =
        ctr_string_eval(ctr_build_string_from_cstring(mname), NULL);
    ctr_object_on_do(myself, arglist);
    ctr_heap_free(mname);
    return myself;
  } else {
    ctr_argument varg = {0}, *args = &varg;
    ctr_argument earg = {0}, *elnumArg = &earg;
    ctr_size i;
    for (i = argumentList->object->value.avalue->tail;
         i < argumentList->object->value.avalue->head; i++) {
      elnumArg->object = ctr_build_number_from_float((ctr_number)i);
      args->object = ctr_array_get(argumentList->object, elnumArg);
      ctr_object_attr_reader(myself, args);
    }
    return myself;
  }
}

/**
 *Object genWriter: [String]
 *
 * generate writer method for property.
 * does not create the property is it doesn't exist
 */
ctr_object *ctr_object_attr_writer(ctr_object *myself,
                                   ctr_argument *argumentList) {
  if (argumentList->object->info.type != CTR_OBJECT_TYPE_OTARRAY) {
    ctr_object *name = ctr_internal_cast2string(argumentList->object);
    ctr_argument varg = {0}, narg = {0}, *arglist = &varg;
    arglist->next = &narg;
    char *mname = ctr_heap_allocate(sizeof(char) * 512);
    sprintf(mname, "{:x my %.*s is x.}", (int)name->value.svalue->vlen,
            name->value.svalue->value);
    arglist->object = name;
    arglist->next->object =
        ctr_string_eval(ctr_build_string_from_cstring(mname), NULL);
    ctr_object_on_do(myself, arglist);
    ctr_heap_free(mname);
    return myself;
  } else {
    ctr_argument varg = {0}, *args = &varg;
    ctr_argument earg = {0}, *elnumArg = &earg;
    ctr_size i;
    for (i = argumentList->object->value.avalue->tail;
         i < argumentList->object->value.avalue->head; i++) {
      elnumArg->object = ctr_build_number_from_float((ctr_number)i);
      args->object = ctr_array_get(argumentList->object, elnumArg);
      ctr_object_attr_writer(myself, args);
    }
    return myself;
  }
}

/**
 *[Object] unpack: [Object:{Ref*}], [Object:Context]
 *
 * Element-wise object assign
 * (Always prefer using algebraic deconstruction assignments: look at section
 *'Assignment')
 */

ctr_object *ctr_object_assign(ctr_object *myself, ctr_argument *argumentList) {
  if (argumentList->object->info.type == CTR_OBJECT_TYPE_OTMISC &&
      argumentList->object->interfaces->link == CtrStdSymbol) {
    if (argumentList->object->value.svalue->vlen == 0 ||
        (argumentList->object->value.svalue->vlen == 1 &&
         *argumentList->object->value.svalue->value == '_'))
      return myself;
    ctr_internal_object_add_property(
        argumentList->next->object ?: ctr_contexts[ctr_context_id],
        ctr_symbol_as_string(argumentList->object), myself, 0);
    return myself;
  }
  if (!ctr_reflect_check_bind_valid(myself, argumentList->object, 0)) {
    CtrStdFlow = ctr_build_string_from_cstring("Invalid bind");
    return CtrStdNil;
  }
  ctr_object *oldlink = myself->interfaces->link;
  ctr_set_link_all(myself, CtrStdMap); // cast to map
  ctr_map_assign(myself, argumentList);
  ctr_set_link_all(myself, oldlink); // cast back to whatever we were
  return myself;
}

/**
 *[Object] iHash
 *
 * fallback hashing
 * returns the hash of the object's string form.
 */
ctr_object *ctr_object_hash(ctr_object *myself, ctr_argument *argumentList) {
  uint64_t hh = ctr_internal_index_hash(myself);
  ctr_object *hash = ctr_build_number_from_float(hh);
  return hash;
}

ctr_object *ctr_object_make_hiding(ctr_object *myself,
                                   ctr_argument *argumentList) {
  ctr_object *hide_arr = argumentList->object;
  if (hide_arr->info.type != CTR_OBJECT_TYPE_OTARRAY) {
    CtrStdFlow = ctr_build_string_from_cstring("Hiding expects array.");
    return CtrStdFlow;
  }
  ctr_object *instance = ctr_internal_create_object(CTR_OBJECT_TYPE_OTOBJECT);
  ctr_set_link_all(instance, myself);
  ctr_size length = (int)ctr_array_count(hide_arr, NULL)->value.nvalue;
  if (length > 0) {
    ctr_object *key = ctr_build_string_from_cstring("hides");
    ctr_internal_object_set_property(myself, key, hide_arr, 0);
  }
  return instance;
}

/**
 *[Object] swapRefs: [Object]
 *
 * swap two objects with each other.
 * Effectively 'xchg %ra, %rb'
 */
ctr_object *ctr_object_swap(ctr_object *myself, ctr_argument *argumentList) {
  if (argumentList == NULL || argumentList->object == NULL) {
    CtrStdFlow =
        ctr_build_string_from_cstring("Cannot swap with Effectively nothing.");
    return CtrStdNil;
  }
  // LONG AF
  if (myself == CtrStdObject || myself == CtrStdBlock ||
      myself == CtrStdString || myself == CtrStdNumber ||
      myself == CtrStdBool || myself == CtrStdConsole || myself == CtrStdNil ||
      myself == CtrStdGC || myself == CtrStdMap || myself == CtrStdArray ||
      myself == CtrStdFile || myself == CtrStdSystem || myself == CtrStdDice ||
      myself == CtrStdCommand || myself == CtrStdSlurp ||
      myself == CtrStdShell || myself == CtrStdClock || myself == CtrStdFlow ||
      myself == CtrStdBreak || myself == CtrStdContinue ||
      myself == CtrStdExit || myself == CtrStdReflect ||
      myself == CtrStdFiber || argumentList->object == CtrStdObject ||
      argumentList->object == CtrStdBlock ||
      argumentList->object == CtrStdString ||
      argumentList->object == CtrStdNumber ||
      argumentList->object == CtrStdBool ||
      argumentList->object == CtrStdConsole ||
      argumentList->object == CtrStdNil || argumentList->object == CtrStdGC ||
      argumentList->object == CtrStdMap ||
      argumentList->object == CtrStdArray ||
      argumentList->object == CtrStdFile ||
      argumentList->object == CtrStdSystem ||
      argumentList->object == CtrStdDice ||
      argumentList->object == CtrStdCommand ||
      argumentList->object == CtrStdSlurp ||
      argumentList->object == CtrStdShell ||
      argumentList->object == CtrStdClock ||
      argumentList->object == CtrStdFlow ||
      argumentList->object == CtrStdBreak ||
      argumentList->object == CtrStdContinue ||
      argumentList->object == CtrStdExit ||
      argumentList->object == CtrStdReflect ||
      argumentList->object == CtrStdFiber) {
    CtrStdFlow = ctr_build_string_from_cstring(
        "Cannot swap literals/language constants.");
    return CtrStdNil;
  }
  ctr_object tmp = *myself;
  *myself = *(argumentList->object);
  *(argumentList->object) = tmp;
  return myself;
}

/**
 *[Object] type
 *
 * Returns a string representation of the type of object.
 */
ctr_object *ctr_object_type(ctr_object *myself, ctr_argument *argumentList) {
  switch (myself->info.type) {
  case CTR_OBJECT_TYPE_OTNIL:
    return ctr_build_string_from_cstring("Nil");
  case CTR_OBJECT_TYPE_OTBOOL:
    return ctr_build_string_from_cstring("Boolean");
  case CTR_OBJECT_TYPE_OTNUMBER:
    return ctr_build_string_from_cstring("Number");
  case CTR_OBJECT_TYPE_OTSTRING:
    return ctr_build_string_from_cstring("String");
  case CTR_OBJECT_TYPE_OTBLOCK:
  case CTR_OBJECT_TYPE_OTNATFUNC:
    return ctr_build_string_from_cstring("Block");
  default:
    return ctr_build_string_from_cstring("Object");
  }
}

/**
 *[Object] toString
 *
 * Returns a string representation of a generic object.
 * This string representation will be:
 *
 * [Object]
 */
ctr_object *ctr_object_to_string(ctr_object *myself,
                                 ctr_argument *argumentList) {
  char *name = "";
  char *np;
  if (myself->lexical_name) {
    name = ctr_heap_allocate_cstring(myself->lexical_name);
    np = ctr_heap_allocate(sizeof(char) *
                           (myself->lexical_name->value.svalue->vlen + 10));
  } else
    np = ctr_heap_allocate(sizeof(char) * 10);
  sprintf(np, "[%s:Object]", name);
  if (name[0])
    ctr_heap_free(name);
  ctr_object *res = ctr_build_string_from_cstring(np);
  ctr_heap_free(np);
  return res;
}

/**
 *[Object] toNumber
 *
 * Errors out. Objects cannot be represented by numbers.
 * (default behavior)
 */
ctr_object *ctr_object_to_number(ctr_object *myself,
                                 ctr_argument *ctr_argumentList) {
  CtrStdFlow = ctr_build_string_from_cstring("Object#toNumber is disallowed");
  // return 1 anyway
  return ctr_build_number_from_float(1);
}

/**
 *[Object] toBoolean
 *
 * Returns a boolean representation of the object. This basic behavior, part
 * of any object will just return True. Other objects typically override this
 * behavior with more useful implementations.
 */
ctr_object *ctr_object_to_boolean(ctr_object *myself,
                                  ctr_argument *ctr_argumentList) {
  return myself->info.type == CTR_OBJECT_TYPE_OTBOOL ? myself
                                                     : ctr_build_bool(1);
}

/**
 *[Object] equals: [other]
 *
 * Tests whether the current instance is the same as
 * the argument.
 *
 * Alias: =
 *
 * Usage:
 * object equals: other
 */
ctr_object *ctr_object_equals(ctr_object *myself, ctr_argument *argumentList) {
  ctr_object *other = argumentList->object;
  return ctr_build_bool(ctr_internal_object_is_equal(myself, other));
}

/**
 *[Object] id
 *
 * returns a unique identifier for the object
 */
ctr_object *ctr_object_id(ctr_object *myself, ctr_argument *argumentList) {
  return ctr_build_number_from_float((intptr_t)myself);
}

/**
 *[Object] myself
 *
 * Returns the object itself.
 */
ctr_object *ctr_object_myself(ctr_object *myself, ctr_argument *argumentList) {
  return myself;
}

/**
 *[Object] learn: [String] means: [String].
 *
 * Teaches any object to repsond to the first specified message just like
 * it would upon receiving the second. This allows you to map existing
 * responses to new messages. You can use this to translate messages into your
 *native language. After mapping, sending the alias message will be just as fast
 * as sending the original message. You can use this to create programs
 * in your native language without sacrficing performance. Of course the mapping
 *itself has a cost, but the mapped calls will be 'toll-free'.
 *
 * Usage:
 *
 * #in this example we'll map a message to a Dutch word:
 *
 * Boolean learn: 'alsWaar:'
 *         means: 'ifTrue:'.
 *
 * (2 > 1) alsWaar: {
 *   Pen write: 'alsWaar means ifTrue in Dutch'.
 * }
 */
ctr_object *ctr_object_learn_meaning(ctr_object *myself,
                                     ctr_argument *ctr_argumentList) {
  char *current_method_name_str;
  ctr_size current_method_name_len;
  ctr_size i = 0;
  ctr_size len = 0;
  ctr_mapitem *current_method = myself->methods->head;
  ctr_object *target_method_name =
      ctr_internal_cast2string(ctr_argumentList->next->object);
  char *target_method_name_str = target_method_name->value.svalue->value;
  ctr_size target_method_name_len = target_method_name->value.svalue->vlen;
  ctr_object *alias = ctr_internal_cast2string(ctr_argumentList->object);
  while (i < myself->methods->size) {
    current_method_name_str = current_method->key->value.svalue->value;
    current_method_name_len = current_method->key->value.svalue->vlen;
    if (current_method_name_len > target_method_name_len) {
      len = current_method_name_len;
    } else {
      len = target_method_name_len;
    }
    if (strncmp(current_method_name_str, target_method_name_str, len) == 0) {
      ctr_internal_object_add_property(myself, alias, current_method->value, 1);
      break;
    }
    current_method = current_method->next;
    i++;
  }
  return myself;
}

/**
 *[Object] do
 *
 * Activates 'chain mode'. If chain mode is active, all messages will
 * return the recipient object regardless of their return signature.
 *
 * Usage:
 *
 * a := Array < 'hello' ; 'world' ; True ; Nil ; 666.
 * a do pop shift unshift: 'hi', push: 999, done.
 *
 * Because of 'chain mode' you can do 'a do pop shift' etc, instead of
 *
 * a pop.
 * a shift.
 * etc..
 *
 * The 'do' message tells the object to always return itself and disgard
 * the original return value until the message 'done' has been received.
 */
ctr_object *ctr_object_do(ctr_object *myself, ctr_argument *argumentList) {
  myself->info.chainMode = 1;
  return myself;
}

/**
 *[Object] done
 *
 * Deactivates 'chain mode'.
 */
ctr_object *ctr_object_done(ctr_object *myself, ctr_argument *argumentList) {
  myself->info.chainMode = 0;
  return myself;
}

/**
 *[Object] message: [String] arguments: [Array]
 *
 * Sends a custom or 'dynamic' message to an object. This takes a string
 *containing the message to be send to the object and an array listing the
 *arguments at the correct indexes. If the array fails to provide the correct
 *indexes this will generate an out-of-bounds error coming from the Array
 *object. If something other than an Array is provided an error will be thrown
 *as well.
 *
 * Usage:
 *
 * var str := 'write:'.
 * Pen message: 'write:' arguments: (Array < 'Hello World').
 *
 * This will print the string 'Hello world' on the screen using a dynamically
 * crafted message.
 */
ctr_object *ctr_object_message(ctr_object *myself, ctr_argument *argumentList) {
  ctr_object *message = ctr_internal_cast2string(argumentList->object);
  ctr_object *arr = argumentList->next->object;
  if (arr->info.type != CTR_OBJECT_TYPE_OTARRAY) {
    CtrStdFlow =
        ctr_build_string_from_cstring("Dynamic message expects array.");
    return CtrStdNil;
  }
  ctr_size length = (int)ctr_array_count(arr, NULL)->value.nvalue;
  int i = 0;
  ctr_argument *args = ctr_heap_allocate(sizeof(ctr_argument));
  ctr_argument *cur = args;
  ctr_argument *index = ctr_heap_allocate(sizeof(ctr_argument));
  for (i = 0; i < length; i++) {
    if (i > 0) {
      cur->next = ctr_heap_allocate(sizeof(ctr_argument));
      cur = cur->next;
    }
    index->object = ctr_build_number_from_float((double)i);
    cur->object = ctr_array_get(arr, index);
  }
  ctr_heap_free(index);
  char *flatMessage = ctr_heap_allocate_cstring(message);
  ctr_object *answer =
      ctr_send_message(myself, flatMessage, message->value.svalue->vlen, args);
  cur = args;
  if (length == 0) {
    ctr_heap_free(args);
  } else {
    for (i = 0; i < length; i++) {
      ctr_argument *a = cur;
      if (i < length - 1)
        cur = cur->next;
      ctr_heap_free(a);
    }
  }
  ctr_heap_free(flatMessage);
  return answer;
}

/**
 *[Object] on: [String] do: [Block]
 *
 * Makes the object respond to a new kind of message.
 * Use the semicolons to indicate the positions of the arguments to be
 * passed.
 *
 * Usage:
 *
 * object on: 'greet' do: { ... }.
 * object on: 'between:and:' do: { ... }.
 *
 */
ctr_object *ctr_object_on_do(ctr_object *myself, ctr_argument *argumentList) {
  ctr_argument *nextArgument;
  ctr_object *methodBlock;
  ctr_object *methodName = argumentList->object;
  if (methodName->info.type != CTR_OBJECT_TYPE_OTSTRING) {
    CtrStdFlow = ctr_build_string_from_cstring(
        "Expected on: argument to be of type string.");
    CtrStdFlow->info.sticky = 1;
    return myself;
  }
  nextArgument = argumentList->next;
  methodBlock = nextArgument->object;
  if (methodBlock->info.type != CTR_OBJECT_TYPE_OTBLOCK &&
      methodBlock->info.type != CTR_OBJECT_TYPE_OTNATFUNC) {
    CtrStdFlow = ctr_build_string_from_cstring(
        "Expected argument do: to be of type block.");
    CtrStdFlow->info.sticky = 1;
    return myself;
  }
#if CTR_TAGS_ONLY
  ctr_internal_tag_whitelist(methodBlock);
#endif
  ctr_internal_object_set_property(myself, methodName, methodBlock, 1);
  return myself;
}

ctr_object *ctr_sock_error(int fd, int want2close) {
  CtrStdFlow = ctr_format_str("Socket error: %s", strerror(errno));
  if (want2close) {
    shutdown(fd, SHUT_RDWR);
    close(fd);
  }
  return CtrStdNil;
}

union ctr_socket_addr_inet {
  struct sockaddr_in serv_addr;
  struct sockaddr_in6 serv_addr6;
};

#ifndef h_addr
#define h_addr h_addr_list[0]
#endif
ctr_object *ctr_object_send2remote(ctr_object *myself,
                                   ctr_argument *argumentList) {
  char *ip;
  int sockfd = 0, n = 0;
  char *responseBuff;
  size_t responseLength;
  int inet_family_is_v6 = 1;
  ctr_object *answer;
  ctr_object *messageObj;
  ctr_object *ipObj;
  ctr_object *portObj;
  ipObj = ctr_internal_object_find_property(myself,
                                            ctr_build_string_from_cstring("%"),
                                            CTR_CATEGORY_PRIVATE_PROPERTY);
  if (ipObj != NULL) {
    inet_family_is_v6 = (int)(ipObj->value.bvalue);
    ipObj = NULL; // restore to initial state.
  }
  union ctr_socket_addr_inet serv_addr;
  struct hostent *server;
  uint16_t port;
  ipObj = ctr_internal_object_find_property(myself,
                                            ctr_build_string_from_cstring("@"),
                                            CTR_CATEGORY_PRIVATE_PROPERTY);
  portObj = ctr_internal_object_find_property(
      myself, ctr_build_string_from_cstring(":"),
      CTR_CATEGORY_PRIVATE_PROPERTY);
  if (ipObj == NULL)
    return CtrStdNil;
  ipObj = ctr_internal_cast2string(ipObj);
  ip = ctr_heap_allocate_cstring(ipObj);
  if (portObj == NULL || portObj->value.nvalue == -1)
    port = ctr_default_port;
  else
    port = (uint16_t)portObj->value.nvalue;
  answer = ctr_build_empty_string();
  messageObj = ctr_internal_cast2string(argumentList->object);
  if (inet_family_is_v6) {
    if ((sockfd = socket(AF_INET6, SOCK_STREAM, 0)) < 0)
      return ctr_sock_error(sockfd, 0);
  } else {
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
      return ctr_sock_error(sockfd, 0);
  }
  printf("socketfd %d\n", sockfd);
  memset(&serv_addr, '0', sizeof(serv_addr));
  if (inet_family_is_v6)
#ifdef __DEFAULT_SOURCE
    server = gethostbyname2(ip, AF_INET6);
#else
    server = gethostbyname(ip);
#endif
  else
#ifdef __DEFAULT_SOURCE
    server = gethostbyname2(ip, AF_INET);
#else
    server = gethostbyname(ip);
#endif
  if (server == NULL) {
    CtrStdFlow = ctr_format_str("ERROR : No such host %s found.", ip);
    return CtrStdFlow;
  }
  memset((char *)&serv_addr, 0, sizeof(serv_addr));
  if (inet_family_is_v6) {
    serv_addr.serv_addr6.sin6_flowinfo = 0;
    serv_addr.serv_addr6.sin6_family = AF_INET6;
    memmove((char *)&serv_addr.serv_addr6.sin6_addr.s6_addr,
            (char *)server->h_addr, server->h_length);
    serv_addr.serv_addr6.sin6_port = htons(port);
    // printf ("%s", inet_ntoa(serv_addr.serv_addr6.sin6_addr));
  } else {
    serv_addr.serv_addr.sin_family = AF_INET;
    memmove((char *)&serv_addr.serv_addr.sin_addr.s_addr,
            (char *)server->h_addr, server->h_length);
    serv_addr.serv_addr.sin_port = htons(port);
    printf("%s", inet_ntoa(serv_addr.serv_addr.sin_addr));
  }
  int c = 0;
  if (inet_family_is_v6)
    c = connect(sockfd, (struct sockaddr *)&(serv_addr.serv_addr6),
                sizeof(serv_addr.serv_addr6));
  else
    c = connect(sockfd, (struct sockaddr *)&(serv_addr.serv_addr),
                sizeof(serv_addr.serv_addr));
  printf("conn %d\n", c);
  if (c != 0)
    return ctr_sock_error(sockfd, 1);
  // c = send(sockfd, (size_t*) &messageObj->value.svalue->vlen, sizeof(size_t),
  // 0); if ( c < 0 ) ctr_sock_error( sockfd, 1 ); printf("%s\n",
  // messageObj->value.svalue->value);
  c = send(sockfd, messageObj->value.svalue->value,
           messageObj->value.svalue->vlen, 0);
  printf("send %d\n", c);
  if (c < 0)
    ctr_sock_error(sockfd, 1);
  // n = read(sockfd, (size_t*) &responseLength, sizeof(responseLength));
  // if ( n == 0 ) ctr_sock_error( sockfd, 1 );
  responseLength = 3000 * sizeof(size_t);
  responseBuff = ctr_heap_allocate(responseLength + 1);
  n = read(sockfd, responseBuff, responseLength);
  printf("read %d\n", n);
  if (n == 0)
    ctr_sock_error(sockfd, 1);
  answer = ctr_build_string_from_cstring(responseBuff);
  shutdown(sockfd, SHUT_RDWR);
  close(sockfd);
  ctr_heap_free(ip);
  ctr_heap_free(responseBuff);
  return answer;
}

/**
 *[Object] respondTo: [String]
 *
 * Variations:
 *
 * [Object] respondTo: [String] with: [String]
 * [Object] respondTo: [String] with: [String] and: [String]
 *
 * Default respond-to implementation, does nothing.
 */
ctr_object *ctr_object_respond(ctr_object *myself, ctr_argument *argumentList) {
  if (myself->info.remote == 0) {
    ctr_object *meth = argumentList->object;
    ctr_object *err = ctr_build_string_from_cstring("Unknown method ");
    argumentList->object = ctr_send_message(myself, "type", 4, 0);
    ctr_string_append(err, argumentList);
    argumentList->object = ctr_build_string_from_cstring("::'");
    ctr_string_append(err, argumentList);
    argumentList->object = meth;
    ctr_string_append(err, argumentList);
    argumentList->object = ctr_build_string_from_cstring("' was called");
    ctr_string_append(err, argumentList);
    CtrStdFlow = err;
    return myself;
  }
  ctr_object *arr;
  ctr_object *answer;
  ctr_argument argv = {0}, *newArgumentList = &argv;
  arr = ctr_array_new(CtrStdArray, argumentList);
  newArgumentList->object = argumentList->object;
  ctr_array_push(arr, newArgumentList);
  newArgumentList->object = ctr_array_to_string(arr, NULL);
  answer = ctr_object_send2remote(myself, newArgumentList);
  return answer;
}

ctr_object *ctr_object_respond_and(ctr_object *myself,
                                   ctr_argument *argumentList) {
  if (myself->info.remote == 0) {
    ctr_object *meth = argumentList->object;
    ctr_object *err = ctr_build_string_from_cstring("Unknown method ");
    argumentList->object = ctr_send_message(myself, "type", 4, 0);
    ctr_string_append(err, argumentList);
    argumentList->object = ctr_build_string_from_cstring("::'");
    ctr_string_append(err, argumentList);
    argumentList->object = meth;
    ctr_string_append(err, argumentList);
    argumentList->object = ctr_build_string_from_cstring("' was called");
    ctr_string_append(err, argumentList);
    CtrStdFlow = err;
    return myself;
  }
  ctr_object *arr;
  ctr_object *answer;
  ctr_argument varg = {0}, *newArgumentList = &varg;
  arr = ctr_array_new(CtrStdArray, argumentList);
  newArgumentList->object = argumentList->object;
  ctr_array_push(arr, newArgumentList);
  newArgumentList->object = argumentList->next->object;
  ctr_array_push(arr, newArgumentList);
  newArgumentList->object = ctr_array_to_string(arr, NULL);
  answer = ctr_object_send2remote(myself, newArgumentList);
  return answer;
}

ctr_object *ctr_object_respond_and_and(ctr_object *myself,
                                       ctr_argument *argumentList) {
  if (myself->info.remote == 0) {
    ctr_object *meth = argumentList->object;
    ctr_object *err = ctr_build_string_from_cstring("Unknown method ");
    argumentList->object = ctr_send_message(myself, "type", 4, 0);
    ctr_string_append(err, argumentList);
    argumentList->object = ctr_build_string_from_cstring("::'");
    ctr_string_append(err, argumentList);
    argumentList->object = meth;
    ctr_string_append(err, argumentList);
    argumentList->object = ctr_build_string_from_cstring("' was called");
    ctr_string_append(err, argumentList);
    CtrStdFlow = err;
    return myself;
  }
  ctr_object *arr;
  ctr_object *answer;
  ctr_argument varg = {0}, *newArgumentList = &varg;
  arr = ctr_array_new(CtrStdArray, argumentList);
  newArgumentList->object = argumentList->object;
  ctr_array_push(arr, newArgumentList);
  newArgumentList->object = argumentList->next->object;
  ctr_array_push(arr, newArgumentList);
  newArgumentList->object = argumentList->next->next->object;
  ctr_array_push(arr, newArgumentList);
  newArgumentList->object = ctr_array_to_string(arr, NULL);
  answer = ctr_object_send2remote(myself, newArgumentList);
  return answer;
}

/**
 *[Object] isNil
 *
 * Default isNil implementation.
 *
 * Always returns boolean object False.
 */
ctr_object *ctr_object_is_nil(ctr_object *myself, ctr_argument *argumentList) {
  (void) myself;
  (void) argumentList;

  return ctr_build_bool(0);
}

/**@I_OBJ_DEF Boolean*/
/**
 * Boolean
 *
 * Literal:
 *
 * True
 * False
 */
ctr_object *ctr_build_bool(int truth) {
  ctr_object *boolObject = ctr_internal_create_object(CTR_OBJECT_TYPE_OTBOOL);
  if (truth)
    boolObject->value.bvalue = 1;
  else
    boolObject->value.bvalue = 0;
  boolObject->info.type = CTR_OBJECT_TYPE_OTBOOL;
  ctr_set_link_all(boolObject, CtrStdBool);
  return boolObject;
}

/**
 *[Boolean] unpack: [String:Ref], [Object:Context]
 * Assign ref to boolean
 * (Always prefer using algebraic deconstruction assignments: look at section
 *'Assignment')
 */

ctr_object *ctr_bool_assign(ctr_object *myself, ctr_argument *argumentList) {
  if (!ctr_reflect_check_bind_valid(myself, argumentList->object, 0)) {
    CtrStdFlow = ctr_build_string_from_cstring("Invalid bind");
    return CtrStdNil;
  }
  if (argumentList->object->interfaces->link != CtrStdSymbol) {
    if (!ctr_internal_object_is_equal(myself, argumentList->object)) {
      CtrStdFlow =
          ctr_build_string_from_cstring("Boolean cannot be constructed by ");
      ctr_string_append(CtrStdFlow, argumentList);
    }
    return myself;
  }
  if (argumentList->object->value.svalue->vlen == 0 ||
      (argumentList->object->value.svalue->vlen == 1 &&
       *argumentList->object->value.svalue->value == '_'))
    return myself;
  ctr_internal_object_add_property(
      argumentList->next->object ?: ctr_contexts[ctr_context_id],
      ctr_symbol_as_string(argumentList->object), myself, 0);
  return myself;
}

/**
 *[Boolean] = [other]
 *
 * Tests whether the other object (as a boolean) has the
 * same value (boolean state True or False) as the current one.
 *
 * Usage:
 *
 * (True = False) ifFalse: { Pen write: 'This is not True!'. }.
 */
ctr_object *ctr_bool_eq(ctr_object *myself, ctr_argument *argumentList) {
  return ctr_build_bool(
      ctr_internal_cast2bool(argumentList->object)->value.bvalue ==
      myself->value.bvalue);
}

/**
 *[Boolean] != [other]
 *
 * Tests whether the other object (as a boolean) has the
 * same value (boolean state True or False) as the current one.
 *
 * Usage:
 *
 * (True != False) ifTrue: { Pen write: 'This is not True!'. }.
 */
ctr_object *ctr_bool_neq(ctr_object *myself, ctr_argument *argumentList) {
  return ctr_build_bool(
      ctr_internal_cast2bool(argumentList->object)->value.bvalue !=
      myself->value.bvalue);
}

/**
 *[Boolean] toString
 *
 * Simple cast function.
 */
ctr_object *ctr_bool_to_string(ctr_object *myself, ctr_argument *argumentList) {
  (void) argumentList;

  if (myself->value.bvalue == 1) {
    return ctr_build_string_from_cstring("True");
  } else {
    return ctr_build_string_from_cstring("False");
  }
}

/**
 *[Boolean] break
 *
 * Breaks out of the current block and bubbles up to the parent block if
 * the value of the receiver equals boolean True.
 *
 * Usage:
 *
 * (iteration > 10) break. #breaks out of loop after 10 iterations
 */
ctr_object *ctr_bool_break(ctr_object *myself, ctr_argument *argumentList) {
  (void) argumentList;

  if (myself->value.bvalue) {
    CtrStdFlow = CtrStdBreak; /* If error = Break it's a break, there is no real
                                 error. */
  }
  return myself;
}

/**
 *[Boolean] continue
 *
 * Skips the remainder of the current block in a loop, continues to the next
 * iteration.
 *
 * Usage:
 *
 * (iteration > 10) continue.
 */
ctr_object *ctr_bool_continue(ctr_object *myself, ctr_argument *argumentList) {
  (void) argumentList;

  if (myself->value.bvalue) {
    CtrStdFlow = CtrStdContinue; /* If error = Continue, then it breaks only one
                                    iteration (return). */
  }
  return myself;
}

/**
 *[Boolean] ifTrue: [block]
 *
 * Executes a block of code if the value of the boolean
 * object is True.
 *
 * Usage:
 * (some expression) ifTrue: { ... }.
 *
 * You can also use ifFalse and ifTrue with other objects because the
 * Object instance also responds to these messages.
 */
ctr_object *ctr_bool_if_true(ctr_object *myself, ctr_argument *argumentList) {
  ctr_object *result;
  ctr_argument arg = {0}, *arguments = &arg;
  if (myself->value.bvalue) {
    ctr_object *codeBlock = argumentList->object;
    arguments->object = myself;
    result = ctr_block_run(codeBlock, arguments, NULL);
    if (result != codeBlock) {
      ctr_internal_next_return = 1;
      return result;
    }
    return myself;
  }
  if (CtrStdFlow == CtrStdBreak)
    CtrStdFlow = NULL; /* consume break */
  return myself;
}

/**
 *[Boolean] ifFalse: [block]
 *
 * Executes a block of code if the value of the boolean
 * object is True.
 *
 * Usage:
 * (some expression) ifFalse: { ... }.
 *
 * You can also use ifFalse and ifTrue with other objects because the
 * Object instance also responds to these messages.
 */
ctr_object *ctr_bool_if_false(ctr_object *myself, ctr_argument *argumentList) {
  ctr_object *result;
  ctr_argument arg = {0}, *arguments = &arg;
  if (!myself->value.bvalue) {
    ctr_object *codeBlock = argumentList->object;
    arguments->object = myself;
    result = ctr_block_run(codeBlock, arguments, NULL);
    if (result != codeBlock) {
      ctr_internal_next_return = 1;
      return result;
    }
    return myself;
  }
  if (CtrStdFlow == CtrStdBreak)
    CtrStdFlow = NULL; /* consume break */
  return myself;
}

/**
 *[Boolean] ifTrue: [block] ifFalse: [Block]
 */
ctr_object *ctr_bool_if_tf(ctr_object *myself, ctr_argument *argumentList) {
  ctr_object *result;
  ctr_argument arg = {0}, *arguments = &arg;
  if (myself->value.bvalue) {
    ctr_object *codeBlock = argumentList->object;
    arguments->object = myself;
    result = ctr_block_run_here(codeBlock, arguments, NULL);
    if (result != codeBlock) {
      ctr_internal_next_return = 1;
      return result;
    }
    return myself;
  } else {
    ctr_object *codeBlock = argumentList->next->object;
    arguments->object = myself;
    result = ctr_block_run_here(codeBlock, arguments, NULL);
    if (result != codeBlock) {
      ctr_internal_next_return = 1;
      return result;
    }
    return myself;
  }
  if (CtrStdFlow == CtrStdBreak)
    CtrStdFlow = NULL; /* consume break */
  return myself;
}

/**
 *[b:Object] or: [Block|Object]
 *
 * Evaluates and returns the block if b evaluates to false, else returns b
 */

ctr_object *ctr_object_elvis_op(ctr_object *myself,
                                ctr_argument *argumentList) {
  if (ctr_send_message(myself, "toBoolean", 9, NULL)->value.bvalue)
    return myself;
  return argumentList->object->info.type == CTR_OBJECT_TYPE_OTBLOCK
             ? ctr_block_runIt(argumentList->object, NULL)
             : argumentList->object;
}

/**
 * @internal
 */
ctr_object *ctr_object_if_false(ctr_object *myself,
                                ctr_argument *argumentList) {
  return ctr_bool_if_false(ctr_internal_cast2bool(myself), argumentList);
}

/**
 * @internal
 */
ctr_object *ctr_object_if_true(ctr_object *myself, ctr_argument *argumentList) {
  return ctr_bool_if_true(ctr_internal_cast2bool(myself), argumentList);
}

/**
 * @internal
 */
ctr_object *ctr_object_if_tf(ctr_object *myself, ctr_argument *argumentList) {
  return ctr_bool_if_tf(ctr_internal_cast2bool(myself), argumentList);
}

/**
 *[Boolean] not
 *
 * Returns the opposite of the current value.
 *
 * Usage:
 * True := False not.
 *
 */
ctr_object *ctr_bool_not(ctr_object *myself, ctr_argument *argumentList) {
  (void) argumentList;

  return ctr_build_bool(!myself->value.bvalue);
}

/**
 *[Boolean] flip
 *
 * 'Flips a coin'. Returns a random boolean value True or False.
 *
 * Usage:
 * coinLandsOn := (Boolean flip).
 */
ctr_object *ctr_bool_flip(ctr_object *myself, ctr_argument *argumentList) {
  (void) myself;
  (void) argumentList;

  return ctr_build_bool((rand() % 2));
}

/**
 *[Boolean] either: [this] or: [that]
 *
 * Returns argument #1 if boolean value is True and argument #2 otherwise.
 *
 * If either object is a block, it will be executed and its result returned if
 * it is supposed to be returned.
 *
 * Usage:
 * Pen write: 'the coin lands on: ' + (Boolean flip either: 'head' or: 'tail').
 * Pen write: 'the coin lands on: ' + (Boolean flip either: {^'head' * 10000.}
 *or: 'tail').
 */
ctr_object *ctr_bool_either_or(ctr_object *myself, ctr_argument *argumentList) {
  int b = ctr_internal_cast2bool(myself)->value.bvalue;
  if (b) {
    if (argumentList->object->info.type == CTR_OBJECT_TYPE_OTNATFUNC ||
        argumentList->object->info.type == CTR_OBJECT_TYPE_OTBLOCK)
      return ctr_block_runIt(argumentList->object, NULL);
    return argumentList->object;
  } else {
    if (argumentList->next->object->info.type == CTR_OBJECT_TYPE_OTNATFUNC ||
        argumentList->next->object->info.type == CTR_OBJECT_TYPE_OTBLOCK)
      return ctr_block_runIt(argumentList->next->object, NULL);
    return argumentList->next->object;
  }
}

/**
 *[Boolean] & [other]
 *
 * Returns True if both the object value is True and the
 * argument is True as well.
 *
 * Usage:
 *
 * a & b
 *
 */
ctr_object *ctr_bool_and(ctr_object *myself, ctr_argument *argumentList) {
  ctr_object *other = ctr_internal_cast2bool(argumentList->object);
  return ctr_build_bool((myself->value.bvalue && other->value.bvalue));
}

/**
 *[Boolean] ! [other]
 *
 * Returns True if the object value is False and the
 * argument is False as well.
 *
 * Usage:
 *
 * a ! b
 *
 */
ctr_object *ctr_bool_nor(ctr_object *myself, ctr_argument *argumentList) {
  ctr_object *other = ctr_internal_cast2bool(argumentList->object);
  return ctr_build_bool((!myself->value.bvalue && !other->value.bvalue));
}

/**
 *[Boolean] | [other]
 *
 * Returns True if either the object value is True or the
 * argument is True or both are True.
 *
 * Usage:
 *
 * a | b
 */
ctr_object *ctr_bool_or(ctr_object *myself, ctr_argument *argumentList) {
  ctr_object *other = ctr_internal_cast2bool(argumentList->object);
  return ctr_build_bool((myself->value.bvalue || other->value.bvalue));
}

/**
 *[Boolean] ? [other]
 *
 * Returns True if either the object value is True or the
 * argument is True but not both.
 *
 * Usage:
 *
 * a ? b
 */
ctr_object *ctr_bool_xor(ctr_object *myself, ctr_argument *argumentList) {
  ctr_object *other = ctr_internal_cast2bool(argumentList->object);
  return ctr_build_bool((myself->value.bvalue ^ other->value.bvalue));
}

/**
 *[Boolean] toNumber
 *
 * Returns 0 if boolean is False and 1 otherwise.
 */
ctr_object *ctr_bool_to_number(ctr_object *myself, ctr_argument *argumentList) {
  (void) argumentList;

  return ctr_build_number_from_float((ctr_number)myself->value.bvalue);
}

/**@I_OBJ_DEF Number*/
/**
 * Number
 *
 * Literal:
 *
 * 0
 * 1
 * -8
 * 2.5
 *
 * Represents a number object in Citron.
 */
ctr_object *ctr_build_number(char *n) {
  ctr_object *numberObject =
      ctr_internal_create_object(CTR_OBJECT_TYPE_OTNUMBER);
  if (strncasecmp(n, "0x", 2) == 0 || strncasecmp(n, "0b", 2) == 0)
    numberObject->value.nvalue = (double)strtol(n, NULL, 0);
  else
    numberObject->value.nvalue = atof(n);
  ctr_set_link_all(numberObject, CtrStdNumber);
  return numberObject;
}

/**
 *[Number] unpack: [String:Ref], [Object:context]
 * Assign ref to number
 * (Always prefer using algebraic deconstruction assignments: look at section
 *'Assignment')
 */

ctr_object *ctr_number_assign(ctr_object *myself, ctr_argument *argumentList) {
  if (!ctr_reflect_check_bind_valid(myself, argumentList->object, 0)) {
    CtrStdFlow = ctr_build_string_from_cstring("Invalid bind");
    return CtrStdNil;
  }
  if (argumentList->object->interfaces->link != CtrStdSymbol) {
    if (!ctr_internal_object_is_equal(myself, argumentList->object)) {
      CtrStdFlow =
          ctr_build_string_from_cstring("Number cannot be constructed by ");
      ctr_string_append(CtrStdFlow, argumentList);
    }
    return myself;
  }
  if (argumentList->object->value.svalue->vlen == 0 ||
      (argumentList->object->value.svalue->vlen == 1 &&
       *argumentList->object->value.svalue->value == '_'))
    return myself;
  ctr_internal_object_add_property(
      argumentList->next->object ?: ctr_contexts[ctr_context_id],
      ctr_symbol_as_string(argumentList->object), myself, 0);
  return myself;
}

/**
 * @internal
 * BuildNumberFromString
 */
ctr_object *ctr_build_number_from_string(char *str, ctr_size length) {
  char *numCStr;
  ctr_object *numberObject =
      ctr_internal_create_object(CTR_OBJECT_TYPE_OTNUMBER);
  /* turn string into a C-string before feeding it to atof */
  int stringNumberLength = (length <= 40) ? length : 40;
  /* max length is 40 (and that's probably even too long... ) */
  numCStr = (char *)ctr_heap_allocate(41 * sizeof(char));
  memcpy(numCStr, str, stringNumberLength);
  char *baseptr = NULL, bases[] = "xXcCoObB";
  if (numCStr[0] == '0' && length > 1 && numCStr[1] != '.') {
    if (numCStr[1] == '0') {
      CtrStdFlow = ctr_format_str("EInvalid number format at ->%s: extra `0'",
                                  numCStr + 1);
      return CtrStdNil;
    }
    int base = 10;
    baseptr = strchr(bases, numCStr[1]);
    if (baseptr == NULL) {
      base = 8;
      baseptr = numCStr + 1;
    } else
      switch (baseptr - bases) {
      case 0:
      case 1:
        baseptr = numCStr + 2;
        base = 16;
        break;
      case 6:
      case 7:
        baseptr = numCStr + 2;
        base = 2;
        break;
      default:
        baseptr = numCStr;
        break;
      }
    numberObject->value.nvalue = strtol(baseptr, 0, base);
  } else {
    char *endptr = NULL;
    numberObject->value.nvalue = strtod(numCStr, &endptr);
    if (endptr < numCStr + stringNumberLength) {
      ctr_heap_free(numCStr);
      CtrStdFlow = ctr_build_string_from_cstring(
          "cannot build number, extra characters in string");
      return CtrStdNil;
    }
  }
  ctr_set_link_all(numberObject, CtrStdNumber);
  ctr_heap_free(numCStr);
  return numberObject;
}

/**
 * @internal
 * BuildNumberFromFloat
 *
 * Creates a number object from a float.
 * Internal use only.
 */
ctr_object *ctr_build_number_from_float(ctr_number f) {
  ctr_object *numberObject =
      ctr_internal_create_object(CTR_OBJECT_TYPE_OTNUMBER);
  numberObject->value.nvalue = f;
  ctr_set_link_all(numberObject, CtrStdNumber);
  return numberObject;
}

/**
 *[Number] > [other]
 *
 * Returns True if the number is higher than other number.
 */
ctr_object *ctr_number_higherThan(ctr_object *myself,
                                  ctr_argument *argumentList) {
  CTR_ENSURE_TYPE_NUMBER(argumentList->object);
  ctr_object *otherNum = ctr_internal_cast2number(argumentList->object);
  return ctr_build_bool((myself->value.nvalue > otherNum->value.nvalue));
}

/**
 *[Number] >=: [other]
 *
 * Returns True if the number is higher than or equal to other number.
 */
ctr_object *ctr_number_higherEqThan(ctr_object *myself,
                                    ctr_argument *argumentList) {
  CTR_ENSURE_TYPE_NUMBER(argumentList->object);
  ctr_object *otherNum = ctr_internal_cast2number(argumentList->object);
  return ctr_build_bool((myself->value.nvalue >= otherNum->value.nvalue));
}

/**
 *[Number] < [other]
 *
 * Returns True if the number is less than other number.
 */
ctr_object *ctr_number_lowerThan(ctr_object *myself,
                                 ctr_argument *argumentList) {
  CTR_ENSURE_TYPE_NUMBER(argumentList->object);
  ctr_object *otherNum = ctr_internal_cast2number(argumentList->object);
  return ctr_build_bool((myself->value.nvalue < otherNum->value.nvalue));
}

/**
 *[Number] <=: [other]
 *
 * Returns True if the number is less than or equal to other number.
 */
ctr_object *ctr_number_lowerEqThan(ctr_object *myself,
                                   ctr_argument *argumentList) {
  CTR_ENSURE_TYPE_NUMBER(argumentList->object);
  ctr_object *otherNum = ctr_internal_cast2number(argumentList->object);
  return ctr_build_bool((myself->value.nvalue <= otherNum->value.nvalue));
}

/**
 *[Number] = [other]
 *
 * Returns True if the number equals the other number.
 */
ctr_object *ctr_number_eq(ctr_object *myself, ctr_argument *argumentList) {
  // CTR_ENSURE_TYPE_NUMBER (argumentList->object);
  if (argumentList->object->info.type != CTR_OBJECT_TYPE_OTNUMBER)
    return ctr_build_bool(0);
  ctr_object *otherNum = ctr_internal_cast2number(argumentList->object);
  return ctr_build_bool(myself->value.nvalue == otherNum->value.nvalue);
}

/**
 *[Number] !=: [other]
 *
 * Returns True if the number does not equal the other number.
 */
ctr_object *ctr_number_neq(ctr_object *myself, ctr_argument *argumentList) {
  if (argumentList->object->info.type != CTR_OBJECT_TYPE_OTNUMBER)
    return ctr_build_bool(1);
  ctr_object *otherNum = ctr_internal_cast2number(argumentList->object);
  return ctr_build_bool(myself->value.nvalue != otherNum->value.nvalue);
}

/**
 *[Number] between: [low] and: [high]
 *
 * Returns True if the number instance has a value between the two
 * specified values.
 *
 * Usage:
 *
 * q between: x and: y
 */
ctr_object *ctr_number_between(ctr_object *myself, ctr_argument *argumentList) {
  ctr_object *otherNum = ctr_internal_cast2number(argumentList->object);
  ctr_argument *nextArgumentItem = argumentList->next;
  ctr_object *nextArgument = ctr_internal_cast2number(nextArgumentItem->object);
  return ctr_build_bool((myself->value.nvalue >= otherNum->value.nvalue) &&
                        (myself->value.nvalue <= nextArgument->value.nvalue));
}

/**
 *[Number] odd
 *
 * Returns True if the number is odd and False otherwise.
 */
ctr_object *ctr_number_odd(ctr_object *myself, ctr_argument *argumentList) {
  (void) argumentList;

  return ctr_build_bool((int)myself->value.nvalue % 2);
}

/**
 *[Number] even
 *
 * Returns True if the number is even and False otherwise.
 */
ctr_object *ctr_number_even(ctr_object *myself, ctr_argument *argumentList) {
  (void) argumentList;

  return ctr_build_bool(!((int)myself->value.nvalue % 2));
}

/**
 *[Number] + [Number]
 *
 * Adds the other number to the current one. Returns a new
 * number object.
 */
ctr_object *ctr_number_add(ctr_object *myself, ctr_argument *argumentList) {
  ctr_argument varg = {0}, *newArg = &varg;
  ctr_object *otherNum = argumentList->object;
  ctr_object *result;
  ctr_number a;
  ctr_number b;
  ctr_object *strObject;
  if (otherNum->info.type == CTR_OBJECT_TYPE_OTSTRING) {
    strObject = ctr_internal_cast2string(myself);
    newArg->object = otherNum;
    result = ctr_string_concat(strObject, newArg);
    return result;
  } else {
    otherNum = ctr_internal_cast2number(otherNum);
  }
  a = myself->value.nvalue;
  b = otherNum->value.nvalue;
  return ctr_build_number_from_float((a + b));
}

/**
 *[Number] +=: [Number]
 *
 * Increases the number ITSELF by the specified amount, this message will change
 *the value of the number object itself instead of returning a new number.
 */
ctr_object *ctr_number_inc(ctr_object *myself, ctr_argument *argumentList) {
  ctr_object *otherNum = ctr_internal_cast2number(argumentList->object);
  myself->value.nvalue += otherNum->value.nvalue;
  return myself;
}

/**
 *[Number] - [Number]
 *
 * Subtracts the other number from the current one. Returns a new
 * number object.
 */
ctr_object *ctr_number_minus(ctr_object *myself, ctr_argument *argumentList) {
  ctr_object *otherNum = ctr_internal_cast2number(argumentList->object);
  ctr_number a = myself->value.nvalue;
  ctr_number b = otherNum->value.nvalue;
  return ctr_build_number_from_float((a - b));
}

/**
 *[Number] -=: [number]
 *
 * Decreases the number ITSELF by the specified amount, this message will change
 *the value of the number object itself instead of returning a new number.
 */
ctr_object *ctr_number_dec(ctr_object *myself, ctr_argument *argumentList) {
  ctr_object *otherNum = ctr_internal_cast2number(argumentList->object);
  myself->value.nvalue -= otherNum->value.nvalue;
  return myself;
}

/**
 *[Number] negate
 *
 * Negates the value of the number object
 */
ctr_object *ctr_number_negate(ctr_object *myself, ctr_argument *argumentList) {
  (void) argumentList;

  myself->value.nvalue = 0 - myself->value.nvalue;
  return myself;
}

/**
 *[Number] * [Number or Block]
 *
 * Multiplies the number by the specified multiplier. Returns a new
 * number object.
 */
ctr_object *ctr_number_multiply(ctr_object *myself,
                                ctr_argument *argumentList) {
  ctr_object *otherNum;
  ctr_number a;
  ctr_number b;
  otherNum = ctr_internal_cast2number(argumentList->object);
  a = myself->value.nvalue;
  b = otherNum->value.nvalue;
  return ctr_build_number_from_float(a * b);
}

/**
 *[Number] times: [Block]
 *
 * Runs the block of code a 'Number' of times.
 * This is the most basic form of a loop.
 *
 * Usage:
 *
 * 7 times: { :i Pen write: i. }.
 *
 * The example above runs the block 7 times. The current iteration
 * number is passed to the block as a parameter (i in this example).
 */
ctr_object *ctr_number_times(ctr_object *myself, ctr_argument *argumentList) {
  ctr_object *indexNumber;
  ctr_object *block = argumentList->object;
  ctr_argument varg = {0}, *arguments = &varg;
  int t;
  int i;
  if (block->info.type != CTR_OBJECT_TYPE_OTBLOCK &&
      block->info.type != CTR_OBJECT_TYPE_OTNATFUNC) {
    CtrStdFlow = ctr_build_string_from_cstring("Expected a code block.");
    return myself;
  }
  block->info.sticky = 1;
  t = myself->value.nvalue;

  if (block->info.type == CTR_OBJECT_TYPE_OTNATFUNC) {
    ctr_object *idx = arguments->object =
        ctr_internal_create_standalone_object(CTR_OBJECT_TYPE_OTNUMBER);
    for (i = 0; i < t; i++) {
      arguments->object->value.nvalue = i;
      block->value.fvalue(block, arguments);
    }
    ctr_internal_delete_standalone_object(idx);
    ctr_close_context();
    return myself;
  }
  if (block->value.block->lexical)
    ctr_contexts[++ctr_context_id] = block;
  else
    ctr_open_context();
  ctr_tnode *node = block->value.block;
  ctr_tlistitem *codeBlockParts = node->nodes;
  ctr_tnode *codeBlockPart1 = codeBlockParts->node;
  ctr_tnode *codeBlockPart2 = codeBlockParts->next->node;
  ctr_tlistitem *parameterList = codeBlockPart1->nodes;
  ctr_tnode *parameter;
  // ctr_open_context();
  if (parameterList && parameterList->node) {
    parameter = parameterList->node;
    if (parameter->vlen == 4 && strncmp(parameter->value, "self", 4) == 0) {
      // assign self selectively, skip that parameter
      ctr_assign_value_to_local_by_ref(
          ctr_build_string(parameter->value, parameter->vlen), block);
      parameterList = parameterList->next;
    }
  }
  ctr_object *argument_name = parameterList && parameterList->node
                                  ? ctr_build_string(parameterList->node->value,
                                                     parameterList->node->vlen)
                                  : NULL;
  ctr_assign_value_to_local_by_ref(
      &CTR_CLEX_KW_ME,
      block); /* me should always point to object, otherwise you have to store
                 me in self and can't use in if */
  ctr_assign_value_to_local(&CTR_CLEX_KW_THIS,
                            block); /* otherwise running block may get gc'ed. */
  indexNumber = ctr_internal_create_standalone_object(CTR_OBJECT_TYPE_OTNUMBER);
  for (i = 0; i < t; i++) {
    indexNumber->value.nvalue = (ctr_number)i;
    if (parameterList && parameterList->node)
      ctr_assign_value_to_local(argument_name, indexNumber);
    ctr_cwlk_run(codeBlockPart2);
    // ctr_close_context();
    if (CtrStdFlow != NULL && CtrStdFlow != CtrStdBreak &&
        CtrStdFlow != CtrStdContinue) {
      ctr_object *catchBlock = ctr_internal_object_find_property(
          block, ctr_build_string_from_cstring("catch"), 0);
      if (catchBlock != NULL) {
        ctr_object *catch_type = ctr_internal_object_find_property(
            catchBlock, ctr_build_string_from_cstring("%catch"), 0);
        ctr_argument av = {0}, avv = {0}, *a = &av;
        ctr_object *exdata = ctr_build_string_from_cstring(":exdata"),
                   *ex = CtrStdFlow,
                   *getexinfo = ctr_build_string_from_cstring("exceptionInfo");
        ctr_internal_object_set_property(ex, exdata, ctr_internal_ex_data(), 0);
        ctr_internal_create_func(ex, getexinfo, &ctr_exception_getinfo);
        int setstr = 0;
        if (ex->info.type == CTR_OBJECT_TYPE_OTSTRING) {
          ex->info.type = CTR_OBJECT_TYPE_OTOBJECT;
          setstr = 1;
          ctr_internal_create_func(ex,
                                   ctr_build_string_from_cstring("toString"),
                                   &ctr_string_to_string);
        }
        a->object = ex;
        a->next = &avv;
        a->next->object = catch_type;
        if (!catch_type ||
            ctr_reflect_is_linked_to(CtrStdReflect, a)->value.bvalue) {
          CtrStdFlow = NULL;
          a->object = ex;
          ctr_block_run_here(catchBlock, a, block);
        }
        ctr_internal_object_delete_property(ex, exdata, 0);
        ctr_internal_object_delete_property(ex, getexinfo, 1);
        if (setstr)
          ex->info.type = CTR_OBJECT_TYPE_OTSTRING;
      }
    }
    if (CtrStdFlow == CtrStdContinue)
      CtrStdFlow = NULL; /* consume continue */
    if (CtrStdFlow)
      break;
  }
  // ctr_object* ctx = ctr_contexts[ctr_context_id];
  ctr_internal_delete_standalone_object(indexNumber);
  ctr_close_context();
  // arguments->object = ctx;
  // ctr_gc_sweep_this(CtrStdGC, arguments);
  if (CtrStdFlow == CtrStdBreak)
    CtrStdFlow = NULL; /* consume break */
  block->info.mark = 0;
  block->info.sticky = 0;
  return myself;
}

/**
 *[Number] *=: [Number]
 *
 * Multiplies the number ITSELF by multiplier, this message will change the
 * value of the number object itself instead of returning a new number.
 *
 * Usage:
 *
 * x := 5.
 * x *=: 2. #x is now 10.
 *
 * Use this message to apply the operation to the object itself instead
 * of creating and returning a new object.
 */
ctr_object *ctr_number_mul(ctr_object *myself, ctr_argument *argumentList) {
  ctr_object *otherNum = ctr_internal_cast2number(argumentList->object);
  myself->value.nvalue *= otherNum->value.nvalue;
  return myself;
}

/**
 *[Number] / [Number]
 *
 * Divides the number by the specified divider. Returns a new
 * number object.
 */
ctr_object *ctr_number_divide(ctr_object *myself, ctr_argument *argumentList) {
  ctr_object *otherNum = ctr_internal_cast2number(argumentList->object);
  ctr_number a = myself->value.nvalue;
  ctr_number b = otherNum->value.nvalue;
  if (b == 0) {
    CtrStdFlow = ctr_build_string_from_cstring("Division by zero.");
    CtrStdFlow->info.sticky = 1;
    return myself;
  }
  return ctr_build_number_from_float((a / b));
}

/**
 *[Number] /=: [Number]
 *
 * Divides the number ITSELF by divider, this message will change the
 * value of the number object itself instead of returning a new number.
 *
 * Usage:
 *
 * x := 10.
 * x /=: 2. #x will now be 5.
 *
 * Use this message to apply the operation to the object itself instead
 * of generating a new object.
 */
ctr_object *ctr_number_div(ctr_object *myself, ctr_argument *argumentList) {
  ctr_object *otherNum = ctr_internal_cast2number(argumentList->object);
  if (otherNum->value.nvalue == 0) {
    CtrStdFlow = ctr_build_string_from_cstring("Division by zero.");
    return myself;
  }
  myself->value.nvalue /= otherNum->value.nvalue;
  return myself;
}

/**
 *[Number] mod: [modulo]
 *
 * Returns the modulo of the number. This message will return a new
 * object representing the modulo of the recipient.
 *
 * Usage:
 *
 * x := 11 mod: 3. #x will now be 2
 *
 * Use this message to apply the operation of division to the
 * object itself instead of generating a new one.
 */
ctr_object *ctr_number_modulo(ctr_object *myself, ctr_argument *argumentList) {
  ctr_object *otherNum = ctr_internal_cast2number(argumentList->object);
  ctr_number a = myself->value.nvalue;
  ctr_number b = otherNum->value.nvalue;
  if (b == 0) {
    CtrStdFlow = ctr_build_string_from_cstring("Division by zero.");
    return myself;
  }
  return ctr_build_number_from_float(fmod(a, b));
}

/**
 *[Number] pow: [power]
 *
 * Returns a new object representing the
 * number to the specified power.
 *
 * Usage:
 *
 * x := 2 pow: 8. #x will be 256
 *
 * The example above will raise 2 to the power of 8 resulting in
 * a new Number object: 256.
 */
ctr_object *ctr_number_pow(ctr_object *myself, ctr_argument *argumentList) {
  ctr_object *otherNum = ctr_internal_cast2number(argumentList->object);
  ctr_number a = myself->value.nvalue;
  ctr_number b = otherNum->value.nvalue;
  return ctr_build_number_from_float(pow(a, b));
}

/**
 *[Number] pos
 *
 * Returns a boolean indicating wether the number is positive.
 * This message will return a boolean object 'True' if the recipient is
 * positive and 'False' otherwise.
 *
 * Usage:
 *
 * hope := 0.1.
 * ( hope pos ) ifTrue: { Pen write: 'Still a little hope for humanity'. }.
 *
 * The example above will print the message because hope is higher than 0.
 */
ctr_object *ctr_number_positive(ctr_object *myself,
                                ctr_argument *argumentList) {
  (void) argumentList;

  return ctr_build_bool((myself->value.nvalue > 0));
}

/**
 *[Number] neg
 *
 * Returns a boolean indicating wether the number is negative.
 * This message will return a boolean object 'True' if the recipient is
 * negative and 'False' otherwise. It's the eaxct opposite of the 'positive'
 * message.
 *
 * Usage:
 *
 * hope := -1.
 * (hope neg) ifTrue: { Pen write: 'No hope left'. }.
 *
 * The example above will print the message because the value of the variable
 * hope is less than 0.
 */
ctr_object *ctr_number_negative(ctr_object *myself,
                                ctr_argument *argumentList) {
  (void) argumentList;

  return ctr_build_bool((myself->value.nvalue < 0));
}

/**
 *[Number] max: [other]
 *
 * Returns the biggest number of the two.
 *
 * Usage:
 *
 * x := 6 max: 4. #x is 6
 * x := 6 max: 7. #x is 7
 */
ctr_object *ctr_number_max(ctr_object *myself, ctr_argument *argumentList) {
  ctr_object *otherNum = ctr_internal_cast2number(argumentList->object);
  ctr_number a = myself->value.nvalue;
  ctr_number b = otherNum->value.nvalue;
  return ctr_build_number_from_float((a >= b) ? a : b);
}

/**
 *[Number] min: [other]
 *
 * Returns a the smallest number.
 *
 * Usage:
 *
 * x := 6 min: 4. #x is 4
 * x := 6 min: 7. #x is 7
 */
ctr_object *ctr_number_min(ctr_object *myself, ctr_argument *argumentList) {
  ctr_object *otherNum = ctr_internal_cast2number(argumentList->object);
  ctr_number a = myself->value.nvalue;
  ctr_number b = otherNum->value.nvalue;
  return ctr_build_number_from_float((a <= b) ? a : b);
}

/**
 *[Number] factorial
 *
 * Calculates the factorial of a number.
 */
ctr_object *ctr_number_factorial(ctr_object *myself,
                                 ctr_argument *argumentList) {
  (void) argumentList;

  ctr_number t = myself->value.nvalue;
  int i;
  ctr_number a = 1;
  for (i = (int)t; i > 0; i--) {
    a = a * i;
  }
  return ctr_build_number_from_float(a);
}

/**
 *[Number] to: [number] step: [step] do: [block]
 *
 * Runs the specified block for each step it takes to go from
 * the start value to the target value using the specified step size.
 * This is basically how you write for-loops in Citron.
 *
 * Usage:
 *
 * 1 to: 5 step: 1 do: { :step Pen write: 'this is step #'+step. }.
 */
ctr_object *ctr_number_to_step_do(ctr_object *myself,
                                  ctr_argument *argumentList) {
  ctr_number startValue = myself->value.nvalue;
  ctr_number endValue =
      ctr_internal_cast2number(argumentList->object)->value.nvalue;
  ctr_number incValue =
      ctr_internal_cast2number(argumentList->next->object)->value.nvalue;
  ctr_number curValue = startValue;
  ctr_object *codeBlock = argumentList->next->next->object;
  ctr_argument argv = {0}, *arguments = &argv;
  int forward = 0;
  if (startValue == endValue)
    return myself;
  forward = (startValue < endValue);
  if (codeBlock->info.type != CTR_OBJECT_TYPE_OTBLOCK &&
      codeBlock->info.type != CTR_OBJECT_TYPE_OTNATFUNC) {
    CtrStdFlow = ctr_build_string_from_cstring("Expected block.");
    return myself;
  }
  if (!codeBlock->value.block->lexical)
    ctr_open_context();
  ctr_object *arg =
      ctr_internal_create_standalone_object(CTR_OBJECT_TYPE_OTNUMBER);
  arg->value.nvalue = curValue;
  while ((forward ? curValue < endValue : curValue > endValue) && !CtrStdFlow) {
    arg->value.nvalue = curValue;
    arguments->object = arg;
    ctr_block_run_here(codeBlock, arguments, codeBlock);
    if (CtrStdFlow == CtrStdContinue)
      CtrStdFlow = NULL; /* consume continue and go on */
    curValue += incValue;
  }
  if (!codeBlock->value.block->lexical)
    ctr_close_context();
  if (CtrStdFlow == CtrStdBreak)
    CtrStdFlow = NULL; /* consume break */
  return myself;
}

/**
 *[Number] to: [number] step: [step]
 *
 */
ctr_object *ctr_number_to_step(ctr_object *myself, ctr_argument *argumentList) {
  ctr_argument args = {.object = myself, .next = argumentList};
  return ctr_generator_make(ctr_std_generator, &args);
}

/**
 *[Number] floor
 *
 * Gives the largest integer less than the recipient.
 *
 * Usage:
 *
 * x := 4.5
 * y := x floor. #y will be 4
 *
 * The example above applies the floor function to the recipient (4.5)
 * returning a new number object (4).
 */
ctr_object *ctr_number_floor(ctr_object *myself, ctr_argument *argumentList) {
  (void) argumentList;

  return ctr_build_number_from_float(floor(myself->value.nvalue));
}

ctr_object *ctr_number_qualify(ctr_object *myself, ctr_argument *argumentList) {
  ctr_internal_object_set_property(
      myself, ctr_build_string_from_cstring("qualification"),
      ctr_internal_cast2string(argumentList->object),
      CTR_CATEGORY_PRIVATE_PROPERTY);
  return myself;
}

ctr_object *ctr_number_respond_to(ctr_object *myself,
                                  ctr_argument *argumentList) {
  return ctr_number_qualify(myself, argumentList);
}

/**
 *[Number] ceil
 *
 * Rounds up the recipient number and returns the next higher integer number
 * as a result.
 *
 * Usage:
 *
 * x := 4.5.
 * y = x ceil. #y will be 5
 *
 * The example above applies the ceiling function to the recipient (4.5)
 * returning a new number object (5).
 */
ctr_object *ctr_number_ceil(ctr_object *myself, ctr_argument *argumentList) {
  (void) argumentList;

  return ctr_build_number_from_float(ceil(myself->value.nvalue));
}

/**
 *[Number] round
 *
 * Returns the rounded number.
 */
ctr_object *ctr_number_round(ctr_object *myself, ctr_argument *argumentList) {
  (void) argumentList;

  return ctr_build_number_from_float(round(myself->value.nvalue));
}

/**
 *[Number] abs
 *
 * Returns the absolute (unsigned, positive) value of the number.
 *
 * Usage:
 *
 * x := -7.
 * y := x abs. #y will be 7
 *
 * The example above strips the sign off the value -7 resulting
 * in 7.
 */
ctr_object *ctr_number_abs(ctr_object *myself, ctr_argument *argumentList) {
  (void) argumentList;

  return ctr_build_number_from_float(fabs(myself->value.nvalue));
}

/**
 *[Number] sqrt
 *
 * Returns the square root of the recipient.
 *
 * Usage:
 *
 * x := 49.
 * y := x sqrt. #y will be 7
 *
 * The example above takes the square root of 49, resulting in the
 * number 7.
 */
ctr_object *ctr_number_sqrt(ctr_object *myself, ctr_argument *argumentList) {
  (void) argumentList;

  return ctr_build_number_from_float(sqrt(myself->value.nvalue));
}

/**
 *[Number] exp
 *
 * Returns the exponent of the number.
 */
ctr_object *ctr_number_exp(ctr_object *myself, ctr_argument *argumentList) {
  return ctr_build_number_from_float(exp(myself->value.nvalue));
}

/**
 *[Number] sin
 *
 * Returns the sine of the number.
 */

ctr_object *ctr_number_sin(ctr_object *myself, ctr_argument *argumentList) {
  (void) argumentList;

  return ctr_build_number_from_float(sin(myself->value.nvalue));
}

/**
 *[Number] cos
 *
 * Returns the cosine of the number.
 */
ctr_object *ctr_number_cos(ctr_object *myself, ctr_argument *argumentList) {
  (void) argumentList;

  return ctr_build_number_from_float(cos(myself->value.nvalue));
}

/**
 *[Number] tan
 *
 * Caculates the tangent of a number.
 */
ctr_object *ctr_number_tan(ctr_object *myself, ctr_argument *argumentList) {
  (void) argumentList;

  return ctr_build_number_from_float(tan(myself->value.nvalue));
}

/**
 *[Number] atan
 *
 * Caculates the arctangent of a number.
 */
ctr_object *ctr_number_atan(ctr_object *myself, ctr_argument *argumentList) {
  (void) argumentList;

  return ctr_build_number_from_float(atan(myself->value.nvalue));
}

/**
 *[Number] log
 *
 * Calculates the logarithm of a number.
 */
ctr_object *ctr_number_log(ctr_object *myself, ctr_argument *argumentList) {
  (void) argumentList;

  return ctr_build_number_from_float(log(myself->value.nvalue));
}

/**
 *[Number] shiftRight: [Number]
 */
ctr_object *ctr_number_shr(ctr_object *myself, ctr_argument *argumentList) {
  return ctr_build_number_from_float(
      (((unsigned long long)(myself->value.nvalue)) >>
       (unsigned long long)((ctr_internal_cast2number(argumentList->object))
                                ->value.nvalue)));
}

/**
 *[Number] shiftLeft: [Number]
 *
 */
ctr_object *ctr_number_shl(ctr_object *myself, ctr_argument *argumentList) {
  return ctr_build_number_from_float(
      (((unsigned long long)(myself->value.nvalue))
       << (unsigned long long)((ctr_internal_cast2number(argumentList->object))
                                   ->value.nvalue)));
}

/**
 *[Number] bitOr: [Number]
 *
 */
ctr_object *ctr_number_or(ctr_object *myself, ctr_argument *argumentList) {
  return ctr_build_number_from_float(
      (((unsigned long long)(myself->value.nvalue)) |
       (unsigned long long)((ctr_internal_cast2number(argumentList->object))
                                ->value.nvalue)));
}

/**
 *[Number] bitAnd: [Number]
 *
 */
ctr_object *ctr_number_and(ctr_object *myself, ctr_argument *argumentList) {
  return ctr_build_number_from_float(
      (((unsigned long long)(myself->value.nvalue)) &
       (unsigned long long)((ctr_internal_cast2number(argumentList->object))
                                ->value.nvalue)));
}

/**
 *[Number] bitXor: [Number]
 *
 */
ctr_object *ctr_number_xor(ctr_object *myself, ctr_argument *argumentList) {
  return ctr_build_number_from_float(
      (((unsigned long long)(myself->value.nvalue)) ^
       (unsigned long long)((ctr_internal_cast2number(argumentList->object))
                                ->value.nvalue)));
}

/**
 *[Number] toByte
 *
 * Converts a number to a single byte.
 */
ctr_object *ctr_number_to_byte(ctr_object *myself, ctr_argument *argumentList) {
  (void) argumentList;

  ctr_object *str = ctr_build_empty_string();
  str->value.svalue->value = ctr_heap_allocate(1);
  str->value.svalue->vlen = 1;
  *(str->value.svalue->value) = (uint8_t)myself->value.nvalue;
  return str;
}

char *bitrep(unsigned int v, int s) {
  char *buf = ctr_heap_allocate(sizeof(char) * (s + 1));
  for (int i = s; i >= 0; i--)
    *(buf + i) = ('0' + ((v >> i) & 1));
  return buf;
}

ctr_object *ctr_number_uint_binrep(ctr_object *myself,
                                   ctr_argument *argumentList) {
  int bsize = argumentList
                  ? ctr_internal_cast2number(argumentList->object)->value.nvalue
                  : sizeof(double) * 8;
  unsigned int value = ctr_internal_cast2number(myself)->value.nvalue;
  char *buf = bitrep(value, bsize);
  ctr_object *str = ctr_build_string(buf, bsize);
  ctr_heap_free(buf);
  return str;
}

/**
 * [Number] toStringInBase: [Number]
 *
 * cast to number in the given base
 */
ctr_object *ctr_number_to_string_base(ctr_object *myself,
                                      ctr_argument *argumentList) {
  ctr_object *o = myself;
  int slen;
  char *s;
  char *buf;
  int bufSize;
  ctr_object *stringObject;
  int base = ctr_internal_cast2number(argumentList->object)->value.nvalue;
  s = ctr_heap_allocate(200 * sizeof(char));
  bufSize = 2000 / 8 * sizeof(char);
  buf = ctr_heap_allocate(bufSize);
  ctr_itoa(o->value.nvalue, buf, base);
  strncpy(s, buf, strlen(buf));
  ctr_heap_free(buf);
  slen = strlen(s);
  stringObject = ctr_build_string(s, slen);
  ctr_heap_free(s);
  return stringObject;
}

/**
 *[Number] toString
 *
 * Wrapper for cast function.
 */
ctr_object *ctr_number_to_string(ctr_object *myself,
                                 ctr_argument *argumentList) {
  (void) argumentList;

  ctr_object *o = myself;
  int slen;
  char *s;
  char *p;
  char *buf;
  int bufSize;
  ctr_object *stringObject;
  s = ctr_heap_allocate(200 * sizeof(char));
  bufSize = 2000 / 8 * sizeof(char);
  buf = ctr_heap_allocate(bufSize);
  snprintf(buf, 199, "%.10f", o->value.nvalue);
  p = buf + strlen(buf) - 1;
  while (*p == '0' && *p-- != '.')
    ;
  *(p + 1) = '\0';
  if (*p == '.')
    *p = '\0';
  strncpy(s, buf, strlen(buf));
  ctr_heap_free(buf);
  slen = strlen(s);
  stringObject = ctr_build_string(s, slen);
  ctr_heap_free(s);
  return stringObject;
}

/**
 *[Number] toBoolean
 *
 * Casts a number to a boolean object.
 */
ctr_object *ctr_number_to_boolean(ctr_object *myself,
                                  ctr_argument *argumentList) {
  (void) argumentList;

  return ctr_build_bool(myself->value.nvalue);
}

/**@I_OBJ_DEF String*/
/**
 * String
 *
 * Literal:
 *
 * 'Hello World, this is a String.'
 *
 * A sequence of characters. In Citron, strings are UTF-8 aware.
 * You may only use single quotes. To escape a character use the
 * backslash '\' character.
 *
 */
ctr_object *ctr_build_string(char *stringValue, long size) {
  ctr_object *stringObject =
      ctr_internal_create_object(CTR_OBJECT_TYPE_OTSTRING);
  if (size != 0) {
    stringObject->value.svalue->value = ctr_heap_allocate(size * sizeof(char));
    memcpy(stringObject->value.svalue->value, stringValue,
           (sizeof(char) * size));
  } else
    stringObject->value.svalue->value = NULL;
  stringObject->value.svalue->vlen = size;
  ctr_set_link_all(stringObject, CtrStdString);
  return stringObject;
}

ctr_object *
ctr_build_string_from_cformat(char *format, int count,
                              ...) { // TODO: Infer count from format
  va_list ap;
  va_start(ap, count);
  int len = vsnprintf(NULL, 0, format, ap);
  char *buf = malloc(len * sizeof(char));
  vsprintf(buf, format, ap);
  va_end(ap);
  ctr_object *fmt = ctr_build_string(buf, len);
  free(buf);
  return fmt;
}

/**
 *[String] isConstructible: [Object]
 *
 * returns whether object is constructible from string
 */
ctr_object *ctr_string_is_ctor(ctr_object *myself, ctr_argument *argumentList) {
  if (argumentList->object->info.type == CTR_OBJECT_TYPE_OTARRAY) {
    ctr_object *myarr = ctr_string_characters(myself, NULL);
    return ctr_build_bool(ctr_internal_object_is_constructible_(
        myarr, argumentList->object, argumentList->object->info.raw));
  }

  if (argumentList->object->info.type == CTR_OBJECT_TYPE_OTSTRING)
    return ctr_build_bool(ctr_internal_object_is_constructible_(
        myself, argumentList->object, argumentList->object->info.raw));

  return ctr_build_bool(0);
}

/**
 *[String] unpack: [String:Ref],[Object:ctx]
 * Assign ref to string
 * (Always prefer using algebraic deconstruction assignments: look at section
 *'Assignment')
 */

ctr_object *ctr_string_assign(ctr_object *myself, ctr_argument *argumentList) {
  if (argumentList->object->info.type == CTR_OBJECT_TYPE_OTARRAY) {
    ctr_collection *coll = argumentList->object->value.avalue;
    int idx = 0;
    int len = myself->value.svalue->vlen;
    for (ctr_size i = coll->tail; i < coll->head; i++) {
      ctr_object *cs = coll->elements[i];
      if (cs->info.type == CTR_OBJECT_TYPE_OTSTRING) {
        int ln = cs->value.svalue->vlen;
        if (ln <= len - idx && strncmp(myself->value.svalue->value + idx,
                                       cs->value.svalue->value, ln) == 0) {
          idx += ln;
          continue;
        }
      }
      if (cs->interfaces->link == CtrStdSymbol) {
        int x = 1;
      again:;
        if (i + x == coll->head) {
          if (cs->value.svalue->vlen != 0 &&
              !(cs->value.svalue->vlen == 1 && *cs->value.svalue->value == '_'))
            ctr_internal_object_add_property(
                argumentList->next->object ?: ctr_contexts[ctr_context_id],
                ctr_symbol_as_string(cs),
                ctr_build_string(myself->value.svalue->value + idx, len - idx),
                0);
          return myself;
        }
        ctr_object *csnext = coll->elements[i + x];
        if (csnext->info.type == CTR_OBJECT_TYPE_OTSTRING) {
          if (csnext->value.svalue->vlen == 0) {
            x++;
            goto again;
          }
          char *t = csnext->value.svalue->value;
          char *s = myself->value.svalue->value + idx;
          char *r = strstr(s, t);
          if (r != NULL) {
            if (cs->value.svalue->vlen != 0 &&
                !(cs->value.svalue->vlen == 1 &&
                  *cs->value.svalue->value == '_'))
              ctr_internal_object_add_property(
                  argumentList->next->object ?: ctr_contexts[ctr_context_id],
                  ctr_symbol_as_string(cs),
                  ctr_build_string(myself->value.svalue->value + idx, r - s),
                  0);
            idx += r - s;
            continue;
          }
        }
      }
      CtrStdFlow = ctr_build_string_from_cstring(
          "Invalid pattern in string-to-string match");
      return CtrStdNil;
    }
    return myself;
  }
  if (!ctr_reflect_check_bind_valid(myself, argumentList->object, 0)) {
    CtrStdFlow = ctr_build_string_from_cstring("Invalid bind");
    return CtrStdNil;
  }
  if (argumentList->object->interfaces->link != CtrStdSymbol) {
    if (!ctr_internal_object_is_equal(myself, argumentList->object)) {
      CtrStdFlow =
          ctr_build_string_from_cstring("String cannot be constructed by ");
      ctr_string_append(CtrStdFlow, argumentList);
    }
    return myself;
  }
  if (argumentList->object->value.svalue->vlen == 0 ||
      (argumentList->object->value.svalue->vlen == 1 &&
       *argumentList->object->value.svalue->value == '_'))
    return myself;
  ctr_internal_object_add_property(
      argumentList->next->object ?: ctr_contexts[ctr_context_id],
      ctr_symbol_as_string(argumentList->object), myself, 0);
  return myself;
}

/**
 * @internal
 * BuildStringFromCString
 *
 * Creates a Citron String from a 0 terminated C String.
 */
ctr_object *ctr_build_string_from_cstring(char *cstring) {
  return ctr_build_string(cstring, strlen(cstring));
}

/**
 * @internal
 * BuildEmptyString
 *
 * Creates an empty string object, use this to avoid using
 * the 'magic' number 0 when building a string, it is more
 * readable this way and your intention is clearer.
 */
ctr_object *ctr_build_empty_string() { return ctr_build_string("", 0); }

/**
 * [String] escape: '\n'.
 *
 * Escapes the specified ASCII character(s) in a string.
 */
ctr_object *ctr_string_escape_ascii(ctr_object *myself,
                                    ctr_argument *argumentList) {
  ctr_object *escape =
      argumentList && argumentList->object && argumentList->object != CtrStdNil
          ? ctr_internal_cast2string(argumentList->object)
          : ctr_build_string_from_cstring((char[]){
            0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b,
            0x0c, 0x0d, 0x0e, 0x0f, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
            0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x0a
          }); // sensible default
  ctr_object *newString = NULL;
  char *str = myself->value.svalue->value;
  long len = myself->value.svalue->vlen;
  char *tstr;
  long i = 0;
  long k = 0;
  ctr_size q = 0;
  ctr_size nchars = 0;
  long tlen = 0;
  char *characters;
  char character;
  char descr;
  char is_cchar = 0;
  char escaped;
  long tag_len = 0;
  characters = escape->value.svalue->value;
  nchars = escape->value.svalue->vlen;
  if (nchars < 1) {
    return myself;
  }
  for (q = 0; q < nchars; ++q) {
    character = characters[q];
    is_cchar = 0;
    descr = character;
    for (i = 0; i < len; i++) {
      char c = str[i];
      if (c == character) {
        tag_len += 2;
      }
    }
  }
  tlen = len + tag_len;
  tstr = ctr_heap_allocate(tlen * sizeof(char));
  for (i = 0; i < len; i++) {
    char c = str[i];
    escaped = 0;
    for (q = 0; q < nchars; q++) {
      character = characters[q];
      is_cchar = 0;
      switch (character) {
      case '\t':
        descr = 't';
        is_cchar = 1;
        break;
      case '\r':
        descr = 'r';
        is_cchar = 1;
        break;
      case '\n':
        descr = 'n';
        is_cchar = 1;
        break;
      case '\b':
        descr = 'b';
        is_cchar = 1;
        break;
      }
      if (c == character) {
        tstr[k++] = '\\';
        if (is_cchar) {
          tstr[k++] = descr;
        } else {
          tstr[k++] = str[i];
        }
        escaped = 1;
        break;
      }
    }
    if (!escaped) {
      tstr[k++] = str[i];
    }
  }
  newString = ctr_build_string(tstr, tlen);
  ctr_heap_free(tstr);
  return newString;
}

/**
 *[String] bytes
 *
 * Returns the number of bytes in a string, as opposed to
 * length which returns the number of UTF-8 code points (symbols or characters).
 */
ctr_object *ctr_string_bytes(ctr_object *myself, ctr_argument *argumentList) {
  (void) argumentList;

  return ctr_build_number_from_float((float)myself->value.svalue->vlen);
}

ctr_object *ctr_string_put_at(ctr_object *myself, ctr_argument *argumentList) {
  if (!myself->value.svalue->vlen) {
    CtrStdFlow = ctr_build_string_from_cstring("Index out of bounds");
    return ctr_build_nil();
  }
  ctr_object *fromPos = ctr_internal_cast2number(argumentList->next->object);
  long a = (fromPos->value.nvalue);
  ctr_size ua = getBytesUtf8(myself->value.svalue->value, 0, a);
  if (ua >= myself->value.svalue->vlen) {
    CtrStdFlow = ctr_build_string_from_cstring("Index out of bounds");
    return ctr_build_nil();
  }
  ctr_size ub = getBytesUtf8(myself->value.svalue->value, ua, 1);
  ctr_string *replacement =
      ctr_internal_cast2string(argumentList->object)->value.svalue;
  int diff = 0;
  ctr_string *p = myself->value.svalue;
  if (replacement->vlen < ub) {
    // move from ua+ub -> ua+replacement
    memmove(p->value + ua + replacement->vlen, p->value + ua + ub,
            p->vlen - ua - ub);
  }
  // truncate away
  if (replacement->vlen != ub) {
    p->value = ctr_heap_reallocate(
        p->value, (p->vlen += (diff = replacement->vlen - ub)));
  }
  if (replacement->vlen > ub)
    memmove(p->value + ua + replacement->vlen, p->value + ua + ub,
            p->vlen - ua - ub);
  memcpy(p->value + ua, replacement->value, replacement->vlen);
  return myself;
}

/**
 *[String] = [other]
 *
 * Returns True if the other string is the same (in bytes).
 */
ctr_object *ctr_string_eq(ctr_object *myself, ctr_argument *argumentList) {
  ctr_object *other = ctr_internal_cast2string(argumentList->object);
  if (other->value.svalue->vlen != myself->value.svalue->vlen) {
    return ctr_build_bool(0);
  }
  return ctr_build_bool(
      (strncmp(other->value.svalue->value, myself->value.svalue->value,
               myself->value.svalue->vlen) == 0));
}

/**
 *[String] != [other]
 *
 * Returns True if the other string is not the same (in bytes).
 */
ctr_object *ctr_string_neq(ctr_object *myself, ctr_argument *argumentList) {
  ctr_object *other = ctr_internal_cast2string(argumentList->object);
  if (other->value.svalue->vlen != myself->value.svalue->vlen) {
    return ctr_build_bool(1);
  }
  return ctr_build_bool(
      !(strncmp(other->value.svalue->value, myself->value.svalue->value,
                myself->value.svalue->vlen) == 0));
}

/**
 *[String] length
 *
 * Returns the length of the string in symbols.
 * This message is UTF-8 unicode aware. A 4 byte character will be counted as
 *ONE.
 */
ctr_object *ctr_string_length(ctr_object *myself, ctr_argument *argumentList) {
  (void) argumentList;

  ctr_size n = ctr_getutf8len(myself->value.svalue->value,
                              (ctr_size)myself->value.svalue->vlen);
  return ctr_build_number_from_float((ctr_number)n);
}

/**
 *[String] empty
 *
 * returns whether this string is empty
 */
ctr_object *ctr_string_empty(ctr_object *myself, ctr_argument *argumentList) {
  (void) argumentList;

  ctr_size n = ctr_getutf8len(myself->value.svalue->value,
                              (ctr_size)myself->value.svalue->vlen);
  return ctr_build_bool(n == 0);
}

/**
 *[String] + [other]
 *
 * Appends other string to self and returns the resulting
 * string as a new object.
 */
ctr_object *ctr_string_concat(ctr_object *myself, ctr_argument *argumentList) {
  ctr_object *strObject = ctr_internal_create_object(CTR_OBJECT_TYPE_OTSTRING);
  ctr_size n1;
  ctr_size n2;
  char *dest;
  ctr_object *newString;
  strObject = ctr_internal_cast2string(argumentList->object);
  n1 = myself->value.svalue->vlen;
  n2 = strObject->value.svalue->vlen;
  if (n1 == 0)
    return ctr_build_string(strObject->value.svalue->value, n2);
  if (n2 == 0)
    return ctr_build_string(myself->value.svalue->value, n1);
  dest = ctr_heap_allocate(sizeof(char) * (n1 + n2));
  if (n1 > 1)
    memcpy(dest, myself->value.svalue->value, n1);
  else
    *dest = *(myself->value.svalue->value);
  if (n2 > 1)
    memcpy(dest + n1, strObject->value.svalue->value, n2);
  else
    *(dest + n1) = *(strObject->value.svalue->value);
  newString = ctr_build_string(dest, (n1 + n2));
  ctr_heap_free(dest);
  return newString;
}

/**
 *[String] append: [String].
 *
 * Appends the specified string to itself. This is different from the '+'
 * message, the '+' message adds the specified string while creating a new
 *string. Appends on the other hand modifies the original string.
 *
 * Usage:
 *
 * x := 'Hello '.
 * x append: 'World'.
 * Pen write: x. #Hello World
 *
 * Instead of using the append message you may also use its short form,
 * like this:
 *
 * x +=: 'World'.
 */
ctr_object *ctr_string_append(ctr_object *myself, ctr_argument *argumentList) {
  ctr_object *strObject;
  ctr_size n1;
  ctr_size n2;
  char *dest;
  strObject = ctr_internal_cast2string(argumentList->object);
  n1 = myself->value.svalue->vlen;
  n2 = strObject->value.svalue->vlen;
  dest = ctr_heap_allocate(sizeof(char) * (n1 + n2));
  memcpy(dest, myself->value.svalue->value, n1);
  memcpy(dest + n1, strObject->value.svalue->value, n2);
  if (myself->value.svalue->vlen > 0) {
    ctr_heap_free(myself->value.svalue->value);
  }
  myself->value.svalue->value = dest;
  myself->value.svalue->vlen = (n1 + n2);
  return myself;
}

/**
 *[String] multiply: [Number].
 *
 * Appends the specified string to itself as many times as [Number]
 * alias: *
 */
ctr_object *ctr_string_multiply(ctr_object *myself,
                                ctr_argument *argumentList) {
  int count = ctr_internal_cast2number(argumentList->object)->value.nvalue;
  size_t old_length = myself->value.svalue->vlen;
  size_t new_length = old_length * count;
  if (count < 1)
    return ctr_build_empty_string();
  if (count == 1)
    return ctr_build_string(myself->value.svalue->value, old_length);
  char *o_str = (myself->value.svalue->value);
  char *newbuf = ctr_heap_allocate(count * old_length * sizeof(char));
  char *pbuf = newbuf;
  char *endbuf = newbuf + new_length;
  while (1)
    if ((pbuf = memcpy(pbuf, o_str, old_length) + old_length) == endbuf)
      break;
  return ctr_build_string(newbuf, new_length);
}

/**
 *[String] formatObjects: [Array].
 *
 * Creates a string with the template format and the specified objects
 *
 * Alias: %
 * Usage:
 *
 * fmt := 'Hello, %s! %d is a number!'
 * str := fmt formatObjects: (Array new < 'World', 23).
 *
 *
 * formatObjects format specifiers:
 * %[opts]<format type>
 * opts:
 *  `:` get the element and use it as a delimiter for the next element (list)
 *  `.<number>` string padding (works for anything but L)
 * types:
 *  `s` string
 *  `d` number (citron number : real)
 *  `L` list
 *  `%` literal percent sign
 *  `c|i|x` C number (int) cast to char|int|hex
 *  `f` C float
 *   otherwise used as parameter for printf-style format
 */
int ctr_str_count_substr(char *str, char *substr, int overlap) {
  if (strlen(substr) == 0)
    return -1; // forbid empty substr

  int count = 0;
  int increment = overlap ? 1 : strlen(substr);
  for (char *s = (char *)str; (s = strstr(s, substr)); s += increment)
    ++count;
  return count;
}

ctr_object *ctr_string_format(ctr_object *myself, ctr_argument *argumentList) {
  ctr_object *objects = argumentList->object;
  int specified_count = ctr_array_count(objects, NULL)->value.nvalue;
  int specifier_count = ctr_str_count_substr(
      myself->value.svalue->value, "%", 1); // allowing overlap skips a strlen
  int wefoold = ctr_str_count_substr(myself->value.svalue->value, "%%", 0);
  specifier_count = specifier_count - wefoold * 2;
  if (specifier_count == 0)
    return myself; // if no specifier, just spit the format string back out
  if (specifier_count > specified_count) {
    CtrStdFlow = ctr_format_str("EFormat string requires more objects than was "
                                "passed (%d specified, %d required).",
                                specified_count, specifier_count);
    return myself;
  }
  int len = myself->value.svalue->vlen;
  char *buf = ctr_heap_allocate(
      sizeof(char) * len); // Eh. Why is the user passing a format string
                           // without any specifiers, but we haven't returned?
  char *fmt = myself->value.svalue->value;
  ctr_object *buffer = ctr_build_empty_string();
  char c = *fmt;
  int specnum = -1;
  int fmtct = 0;
  ctr_argument varg = {0}, *args = &varg;
  ctr_object *cp_delim_opt = ctr_build_string_from_cstring(", ");
  /*
     0 - no opt
     1 - opt delimiter (cp_delim_opt) for %:L (list)
     2 - opt len (cp_len_opt) for %.*s -- TODO: Implement.
   */
  unsigned int in_opt_mode = 0;
  for (int i = 0; i < len; i++, c = *(fmt + i)) {
    in_opt_mode = 0;
    if (c == '%') {
      if (i > len - 2) {
      error_out:;
        CtrStdFlow =
            ctr_format_str("Malformed format string at index %d(%c)", i, c);
        return myself;
      error_out_wrong_args:;
        CtrStdFlow =
            ctr_format_str("Incorrect format args for spec %d(%c)", specnum, c);
        return myself;
      }
      if (fmtct) {
        args->object = ctr_build_string(buf, fmtct);
        ctr_string_append(buffer, args);
        fmtct = 0;
      }
      specnum++;
      c = *(fmt + 1 + (i++));
      if (c == ':') {
        if (in_opt_mode != 0)
          goto error_out;
        in_opt_mode = 1;
        c = *(fmt + 1 + i++);
      }
      if (c == '.') {
        if ((c = *(fmt + i)) == '*') {
          if (in_opt_mode != 0)
            goto error_out;
          in_opt_mode = 2;
          c = *(fmt + 1 + i++);
        }
      }
      if (c == 'L') {
        if (in_opt_mode & 1) {
          args->object = ctr_build_number_from_float(specnum++);
          cp_delim_opt = ctr_array_get(objects, args);
        }
        args->object = ctr_build_number_from_float(specnum);
        ctr_object *arr = ctr_array_get(objects, args);
        if (arr->info.type != CTR_OBJECT_TYPE_OTARRAY)
          goto error_out_wrong_args;
        for (ctr_size i = arr->value.avalue->tail; i < arr->value.avalue->head;
             i++) {
          args->object = arr->value.avalue->elements[i];
          ctr_string_append(buffer, args);
          if (i != arr->value.avalue->head - 1) {
            args->object = cp_delim_opt;
            ctr_string_append(buffer, args);
          }
        }
        continue;
      }
      if (c == 's') {
        args->object = ctr_build_number_from_float(specnum);
        args->object =
            ctr_send_message(ctr_array_get(objects, args), "toString", 8, NULL);
        ctr_string_append(buffer, args);
        continue;
      }
      if (c == 'd') {
        args->object = ctr_build_number_from_float(specnum);
        args->object =
            ctr_send_message(ctr_array_get(objects, args), "toNumber", 8, NULL);
        ctr_string_append(buffer, args);
        continue;
      }
      if (c == 'b') {
        args->object = ctr_build_number_from_float(specnum);
        args->object = ctr_number_uint_binrep(
            ctr_send_message(ctr_array_get(objects, args), "toNumber", 8, NULL),
            NULL);
        ctr_string_append(buffer, args);
        continue;
      }
      if (c == 'c') {
        args->object = ctr_build_number_from_float(specnum);
        args->object =
            ctr_send_message(ctr_array_get(objects, args), "toNumber", 8, NULL);
        char buf[2];
        sprintf(buf, "%c", (int)args->object->value.nvalue);
        ctr_invoke_variadic(buffer, &ctr_string_append, 1,
                            ctr_build_string(buf, 1));
        continue;
      }
      if (c == '%') {
        args->object = ctr_build_string_from_cstring("%");
        ctr_string_append(buffer, args);
        specnum--;
      } else {
        char _fmt_[1024];
        char buf[10240];
        _fmt_[0] = '%';
        int fi = 1;
        while (!isalpha(_fmt_[fi++] = fmt[i++]))
          ;
        --i;
        _fmt_[fi] = '\0';
        args->object = ctr_build_number_from_float(specnum);
        switch (_fmt_[fi - 1]) {
        case 'c':
        case 'd':
        case 'i':
        case 'x':
          args->object = ctr_send_message(ctr_array_get(objects, args),
                                          "toNumber", 8, NULL);
          sprintf(buf, _fmt_, (int)args->object->value.nvalue);
          break;
        case 'f':
          args->object = ctr_send_message(ctr_array_get(objects, args),
                                          "toNumber", 8, NULL);
          sprintf(buf, _fmt_, (float)args->object->value.nvalue);
          break;
        default:
          args->object = ctr_send_message(ctr_array_get(objects, args),
                                          "toString", 8, NULL);
          char *strr = ctr_heap_allocate_cstring(args->object);
          sprintf(buf, _fmt_, strr);
          ctr_heap_free(strr);
          break;
        }
        ctr_invoke_variadic(buffer, &ctr_string_append, 1,
                            ctr_build_string_from_cstring(buf));
      }
    } else
      buf[fmtct++] = c;
  }
  if (fmtct) {
    args->object = ctr_build_string(buf, fmtct);
    ctr_string_append(buffer, args);
    fmtct = 0;
  }
  ctr_heap_free(buf);
  return buffer;
}

/**
 *[String] formatMap: [Map]
 *
 * format a format string based off a map
 *
 * e.g.
 *  format := 'this \'%{string}\' contains some wierd %{shit}.'.
 *  fmtd := format formatMap: (Map new put: 'shit' at: 'shit', put: format at:
 *'string'). # 'this \'this \'%{string}\' contains some wierd %{shit}.\'
 *contains some weird shit.'
 *
 */
ctr_object *ctr_string_format_map(ctr_object *myself,
                                  ctr_argument *argumentList) {
  ctr_object *objects = argumentList->object;
  int specifier_count = ctr_str_count_substr(
      myself->value.svalue->value, "%{", 0); // doesn't make sense to overlap
  if (specifier_count == 0)
    return myself; // if no specifier, just spit the format string back out
  int len = myself->value.svalue->vlen;
  char *buf = ctr_heap_allocate(
      sizeof(char) * len); // Eh. Why is the user passing a format string
                           // without any specifiers, but we haven't returned?
  char *fmt = myself->value.svalue->value;
  ctr_object *buffer = ctr_build_empty_string();
  char c = *fmt;
  int in_spec = 0;
  int fmtct = 0;
  int interpolate = 0;
  ctr_argument vargs = {0}, *args = &vargs;
  for (int i = 0; i < len; i++, c = *(fmt + i)) {
    // buf has already been cleared, and fmtct set to 0, so we'll use buf for
    // spec
    if (in_spec) {
      if (c == '}') {
        in_spec = 0; // set to zero and allow interpolation
        interpolate = 1;
      } else
        buf[fmtct++] = c;
      continue;
    }
    if (interpolate) {
      ctr_object *prop = args->object = ctr_build_string(buf, fmtct);
      args->object = ctr_map_get(objects, args);
      if (args->object == CtrStdNil)
        args->object = ctr_internal_object_find_property(objects, prop, 0);
      args->object = ctr_send_message(args->object, "toString", 8, NULL);
      ctr_string_append(buffer, args);
      interpolate = 0;
      fmtct = 0;
      i--;
      continue;
    }
    if (c == '%' && (c = *(fmt + 1 + (i++))) == '{') {
      if (fmtct) {
        args->object = ctr_build_string(buf, fmtct);
        ctr_string_append(buffer, args);
        fmtct = 0;
      }
      in_spec = 1;
    } else
      buf[fmtct++] = c;
  }
  if (fmtct) {
    ctr_object *prop = args->object = ctr_build_string(buf, fmtct);
    if (interpolate) {
      args->object = ctr_map_get(objects, args);
      if (args->object == CtrStdNil)
        args->object = ctr_internal_object_find_property(objects, prop, 0);
      args->object = ctr_send_message(args->object, "toString", 8, NULL);
      interpolate = 0;
    }
    ctr_string_append(buffer, args);
    fmtct = 0;
  }
  ctr_heap_free(buf);
  return buffer;
}

/**
 *[String] from: [position] to: [destination]
 *
 * Returns a portion of a string defined by from-to values.
 * This message is UTF-8 unicode aware.
 *
 * Usage:
 *
 * 'hello' from: 2 to: 3. #ll
 */
ctr_object *ctr_string_fromto(ctr_object *myself, ctr_argument *argumentList) {
  ctr_object *fromPos = ctr_internal_cast2number(argumentList->object);
  ctr_object *toPos = ctr_internal_cast2number(argumentList->next->object);
  long len = myself->value.svalue->vlen;
  long a = (fromPos->value.nvalue);
  long b = (toPos->value.nvalue);
  long t;
  long ua, ub;
  char *dest;
  ctr_object *newString;
  if (b == a)
    return ctr_build_empty_string();
  if (a > b) {
    t = a;
    a = b;
    b = t;
  }
  if (a > len)
    return ctr_build_empty_string();
  if (b > len)
    b = len;
  if (a < 0)
    a = 0;
  if (b < 0)
    return ctr_build_empty_string();
  ua = getBytesUtf8(myself->value.svalue->value, 0, a);
  ub = getBytesUtf8(myself->value.svalue->value, ua, ((b - a)));
  dest = ctr_heap_allocate(ub * sizeof(char));
  memcpy(dest, (myself->value.svalue->value) + ua, ub);
  newString = ctr_build_string(dest, ub);
  ctr_heap_free(dest);
  return newString;
}

/**
 *[String] from: [start] length: [length]
 *
 * Returns a portion of a string defined by from
 * and length values.
 * This message is UTF-8 unicode aware.
 *
 * Usage:
 *
 * 'hello' from: 2 length: 3. #llo
 */
ctr_object *ctr_string_from_length(ctr_object *myself,
                                   ctr_argument *argumentList) {
  ctr_object *fromPos = ctr_internal_cast2number(argumentList->object);
  ctr_object *length = ctr_internal_cast2number(argumentList->next->object);
  long len = myself->value.svalue->vlen;
  long a = (fromPos->value.nvalue);
  long b = (length->value.nvalue);
  long ua, ub;
  char *dest;
  ctr_object *newString;
  if (b == 0)
    return ctr_build_empty_string();
  if (b < 0) {
    a = a + b;
    b = labs(b);
  }
  if (a < 0)
    a = 0;
  if (a > len)
    a = len;
  if ((a + b) > len)
    b = len - a;
  if ((a + b) < 0)
    b = b - a;
  ua = getBytesUtf8(myself->value.svalue->value, 0, a);
  ub = getBytesUtf8(myself->value.svalue->value, ua, b);
  dest = ctr_heap_allocate(ub * sizeof(char));
  memcpy(dest, (myself->value.svalue->value) + ua, ub);
  newString = ctr_build_string(dest, ub);
  ctr_heap_free(dest);
  return newString;
}

/**
 *[String] skip: [number]
 *
 * Returns a string without the first X characters.
 */
ctr_object *ctr_string_skip(ctr_object *myself, ctr_argument *argumentList) {
  ctr_argument arg1 = {0}, *argument1 = &arg1;
  ctr_argument arg2 = {0}, *argument2 = &arg2;
  ctr_object *result;
  if (myself->value.svalue->vlen < argumentList->object->value.nvalue)
    return ctr_build_empty_string();
  argument1->object = argumentList->object;
  argument1->next = argument2;
  argument2->object = ctr_build_number_from_float(
      myself->value.svalue->vlen - argumentList->object->value.nvalue);
  result = ctr_string_from_length(myself, argument1);
  return result;
}

/**
 *[String] sliceFrom: [number] length: [number]
 *
 * slice a string from,length and return the sliced. modifies string
 */
int str_cut(char *str, int l, int begin, int len) {
  if (len < 0)
    len = l - begin;
  if (begin + len > l)
    len = l - begin;
  memmove(str + begin, str + begin + len, l - len + 1);
  return len;
}

ctr_object *ctr_string_slice(ctr_object *myself, ctr_argument *argumentList) {
  ctr_argument arg = {0}, arg2 = {0}, *argument = &arg;
  ctr_object *result;
  if (myself->value.svalue->vlen < argumentList->object->value.nvalue)
    return ctr_build_empty_string();
  argument->object = argumentList->object;
  argument->next = &arg2;
  argument->next->object = argumentList->next->object;
  result = ctr_string_from_length(myself, argument);
  long a = getBytesUtf8(myself->value.svalue->value, 0,
                        argumentList->object->value.nvalue);
  long b = getBytesUtf8(myself->value.svalue->value, a,
                        argumentList->next->object->value.nvalue);
  str_cut(myself->value.svalue->value, myself->value.svalue->vlen, a, b);
  myself->value.svalue->vlen -= b;
  return result;
}

/**
 *[String] at: [position]
 *
 * Returns the character at the specified position (UTF8 aware).
 * You may also use the alias '@'.
 *
 * Usage:
 *
 * ('hello' at: 2). #l
 * ('hello' @ 2). #l
 */
ctr_object *ctr_string_at(ctr_object *myself, ctr_argument *argumentList) {
  if (!myself->value.svalue->vlen) {
    CtrStdFlow = ctr_build_string_from_cstring("Index out of bounds");
    return ctr_build_nil();
  }
  ctr_object *fromPos = ctr_internal_cast2number(argumentList->object);
  long a = (fromPos->value.nvalue);
  long ua = getBytesUtf8(myself->value.svalue->value, 0, a);
  long ub = getBytesUtf8(myself->value.svalue->value, ua, 1);
  ctr_object *newString;
  char *dest = ctr_heap_allocate(ub * sizeof(char));
  memcpy(dest, (myself->value.svalue->value) + ua, ub);
  newString = ctr_build_string(dest, ub);
  ctr_heap_free(dest);
  return newString;
}

/**
 *[String] byteAt: [position]
 *
 * Returns the byte at the specified position (in bytes).
 * Note that you cannot use the '@' message here because that will
 * return the unicode point at the specified position, not the byte.
 *
 * Usage:
 * ('abc' byteAt: 1). #98
 */
ctr_object *ctr_string_byte_at(ctr_object *myself, ctr_argument *argumentList) {
  unsigned char x;
  ctr_object *fromPos = ctr_internal_cast2number(argumentList->object);
  long a = (fromPos->value.nvalue);
  long len = myself->value.svalue->vlen;
  if (a > len)
    return CtrStdNil;
  if (a < 0)
    return CtrStdNil;
  x = (unsigned char)*(myself->value.svalue->value + a);
  return ctr_build_number_from_float((double)x);
}

/**
 *[String] indexOf: [subject]
 *
 * Returns the index (character number, not the byte!) of the
 * needle in the haystack.
 *
 * Usage:
 *
 * 'find the needle' indexOf: 'needle'. #9
 *
 */
ctr_object *ctr_string_index_of(ctr_object *myself,
                                ctr_argument *argumentList) {
  ctr_object *sub = ctr_internal_cast2string(argumentList->object);
  long hlen = myself->value.svalue->vlen;
  long nlen = sub->value.svalue->vlen;
  uintptr_t byte_index;
  ctr_size uchar_index;
  char *p = ctr_internal_memmem(myself->value.svalue->value, hlen,
                                sub->value.svalue->value, nlen, 0);
  if (p == NULL)
    return ctr_build_number_from_float((ctr_number)-1);
  byte_index = (uintptr_t)p - (uintptr_t)(myself->value.svalue->value);
  uchar_index = ctr_getutf8len(myself->value.svalue->value, byte_index);
  return ctr_build_number_from_float((ctr_number)uchar_index);
}

/**
 *[String] reLastIndexOf: [subject]
 *
 * Returns the index (character number, not the byte!) of the
 * last occurance of needle in the haystack, by regex
 *
 * Usage:
 *
 * 'find the needle' reLastIndexOf: 'n.{2}dle'. #9
 *
 */
ctr_object *ctr_string_re_last_index_of(ctr_object *myself,
                                        ctr_argument *argumentList) {
#ifndef POSIXRE
  char *re =
      ctr_heap_allocate_cstring(ctr_internal_cast2string(argumentList->object));
  char *str = ctr_heap_allocate_cstring(myself);
  ssize_t offset = pcre_last_indexof(re, str);
  offset = offset > 0 ? ctr_getutf8len(myself->value.svalue->value, offset) - 1
                      : offset;
  ctr_heap_free(re);
  ctr_heap_free(str);
  return ctr_build_number_from_float((ctr_number)offset);
#else
  CtrStdFlow = ctr_build_string_from_cstring(
      "Strin#reLastIndexOf: not implemented for posix re");
  return ctr_build_nil();
#endif
}

/**
 *[String] reIndexOf: [subject]
 *
 * Returns the index (character number, not the byte!) of the
 * needle in the haystack (last matched character), by regex
 *
 * Usage:
 *
 * 'find the needle' reIndexOf: 't.'. #6
 *
 */
ctr_object *ctr_string_re_index_of(ctr_object *myself,
                                   ctr_argument *argumentList) {
#ifndef POSIXRE
  char *re =
      ctr_heap_allocate_cstring(ctr_internal_cast2string(argumentList->object));
  char *str = ctr_heap_allocate_cstring(myself);
  ssize_t offset = pcre_indexof(re, str);
  offset = offset > 0 ? ctr_getutf8len(myself->value.svalue->value, offset) - 1
                      : offset;
  ctr_heap_free(re);
  ctr_heap_free(str);
  return ctr_build_number_from_float((ctr_number)offset);
#else
  CtrStdFlow = ctr_build_string_from_cstring(
      "Strin#reLastIndexOf: not implemented for posix re");
  return ctr_build_nil();
#endif
}

/**
 *[String] startsWith: [String]
 *
 * Returns whether the string starts with the arg
 *
 */
ctr_object *ctr_string_starts_with(ctr_object *myself,
                                   ctr_argument *argumentList) {
  ctr_object *pre = ctr_internal_cast2string(argumentList->object);
  size_t lenpre = ctr_string_length(pre, NULL)->value.nvalue,
         lenstr = ctr_string_length(myself, NULL)->value.nvalue;
  return lenstr < lenpre ? ctr_build_bool(0)
                         : ctr_build_bool(strncmp(pre->value.svalue->value,
                                                  myself->value.svalue->value,
                                                  lenpre) == 0);
}

/**
 *[String] endsWith: [String]
 *
 * Returns whether the string ends with the arg
 *
 */
ctr_object *ctr_string_ends_with(ctr_object *myself,
                                 ctr_argument *argumentList) {
  ctr_object *post = ctr_internal_cast2string(argumentList->object);
  size_t lenpost = ctr_string_length(post, NULL)->value.nvalue,
         lenstr = ctr_string_length(myself, NULL)->value.nvalue;
  return lenstr < lenpost ? ctr_build_bool(0)
                          : ctr_build_bool(memcmp(post->value.svalue->value,
                                                  myself->value.svalue->value +
                                                      (lenstr - lenpost),
                                                  lenpost) == 0);
}

/**
 *[String] upper
 *
 * Returns a new uppercased version of the string.
 * Note that this is just basic ASCII case functionality, this should only
 * be used for internal keys and as a basic utility function. This function
 * DOES NOT WORK WITH UTF8 characters !
 */
ctr_object *ctr_string_to_upper(ctr_object *myself,
                                ctr_argument *argumentList) {
  (void) argumentList;

  ctr_object *newString = NULL;
  char *str = myself->value.svalue->value;
  size_t len = myself->value.svalue->vlen;
  char *tstr = ctr_heap_allocate(len * sizeof(char));
  size_t i;
  for (i = 0; i < len; i++) {
    tstr[i] = toupper(str[i]);
  }
  newString = ctr_build_string(tstr, len);
  ctr_heap_free(tstr);
  return newString;
}

/**
 *[String] lower
 *
 * Returns a new lowercased version of the string.
 * Note that this is just basic ASCII case functionality, this should only
 * be used for internal keys and as a basic utility function. This function
 * DOES NOT WORK WITH UTF8 characters !
 */
ctr_object *ctr_string_to_lower(ctr_object *myself,
                                ctr_argument *argumentList) {
  (void) argumentList;

  ctr_object *newString = NULL;
  char *str = myself->value.svalue->value;
  size_t len = myself->value.svalue->vlen;
  char *tstr = ctr_heap_allocate(len * sizeof(char));
  size_t i;
  for (i = 0; i < len; i++) {
    tstr[i] = tolower(str[i]);
  }
  newString = ctr_build_string(tstr, len);
  ctr_heap_free(tstr);
  return newString;
}

/**
 *[String] lower1st
 *
 * Converts the first character of the recipient to lowercase and
 * returns the resulting string object.
 */
ctr_object *ctr_string_to_lower1st(ctr_object *myself,
                                   ctr_argument *argumentList) {
  (void) argumentList;

  ctr_object *newString = NULL;
  size_t len = myself->value.svalue->vlen;
  if (len == 0)
    return ctr_build_empty_string();
  char *tstr = ctr_heap_allocate(len * sizeof(char));
  strncpy(tstr, myself->value.svalue->value, len);
  tstr[0] = tolower(tstr[0]);
  newString = ctr_build_string(tstr, len);
  ctr_heap_free(tstr);
  return newString;
}

/**
 *[String] upper1st
 *
 * Converts the first character of the recipient to uppercase and
 * returns the resulting string object.
 */
ctr_object *ctr_string_to_upper1st(ctr_object *myself,
                                   ctr_argument *argumentList) {
  (void) argumentList;

  ctr_object *newString;
  size_t len = myself->value.svalue->vlen;
  if (len == 0)
    return ctr_build_empty_string();
  char *tstr = ctr_heap_allocate(len * sizeof(char));
  strncpy(tstr, myself->value.svalue->value, len);
  tstr[0] = toupper(tstr[0]);
  newString = ctr_build_string(tstr, len);
  ctr_heap_free(tstr);
  return newString;
}

ctr_object *ctr_string_to_string(ctr_object *myself,
                                 ctr_argument *argumentList) {
  (void) argumentList;

  return ctr_build_string(myself->value.svalue->value,
                          myself->value.svalue->vlen);
}

/**
 *[String] lastIndexOf: [subject]
 *
 * Returns the index (character number, not the byte!) of the
 * needle in the haystack.
 *
 * Usage:
 *
 * 'find the needle' lastIndexOf: 'needle'. #9
 */
ctr_object *ctr_string_last_index_of(ctr_object *myself,
                                     ctr_argument *argumentList) {
  ctr_object *sub = ctr_internal_cast2string(argumentList->object);
  ctr_size hlen = myself->value.svalue->vlen;
  ctr_size nlen = sub->value.svalue->vlen;
  ctr_size uchar_index;
  ctr_size byte_index;
  char *p = ctr_internal_memmem(myself->value.svalue->value, hlen,
                                sub->value.svalue->value, nlen, 1);
  if (p == NULL)
    return ctr_build_number_from_float((float)-1);
  byte_index = (ctr_size)(p - (myself->value.svalue->value));
  uchar_index = ctr_getutf8len(myself->value.svalue->value, byte_index);
  return ctr_build_number_from_float((float)uchar_index);
}

/**
 *[String] replace: [string] with: [other]
 *
 * Replaces needle with replacement in original string and returns
 * the result as a new string object.
 *
 * Usage:
 *
 * 'LiLo BootLoader' replace: 'L' with: 'l'. #lilo Bootloader
 */
ctr_object *ctr_string_replace_with(ctr_object *myself,
                                    ctr_argument *argumentList) {
  ctr_object *needle = ctr_internal_cast2string(argumentList->object);
  ctr_object *replacement =
      ctr_internal_cast2string(argumentList->next->object);
  ctr_object *str;
  char *dest;
  char *odest;
  char *src = myself->value.svalue->value;
  char *ndl = needle->value.svalue->value;
  char *rpl = replacement->value.svalue->value;
  long hlen = myself->value.svalue->vlen;
  long nlen = needle->value.svalue->vlen;
  long rlen = replacement->value.svalue->vlen;
  long dlen = hlen;
  char *p;
  long i = 0;
  long offset = 0;
  long d;
  if (nlen == 0 || hlen == 0) {
    return ctr_build_string(src, hlen);
  }
  dest = (char *)ctr_heap_allocate(dlen * sizeof(char));
  odest = dest;
  while (1) {
    p = ctr_internal_memmem(src, hlen, ndl, nlen, 0);
    if (p == NULL)
      break;
    d = (dest - odest);
    if ((dlen - nlen + rlen) > dlen) {
      dlen = (dlen - nlen + rlen);
      odest = (char *)ctr_heap_reallocate(odest, dlen * sizeof(char));
      dest = (odest + d);
    } else {
      dlen = (dlen - nlen + rlen);
    }
    offset = (p - src);
    memcpy(dest, src, offset);
    dest = dest + offset;
    memcpy(dest, rpl, rlen);
    dest = dest + rlen;
    hlen = hlen - (offset + nlen);
    src = src + (offset + nlen);
    i++;
  }
  memcpy(dest, src, hlen);
  str = ctr_build_string(odest, dlen);
  ctr_heap_free(odest);
  return str;
}

/**
 *[String] findPattern: [String] do: [Block] options: [String].
 *
 * Matches the POSIX or PCRE (depending on the #defines) regular expression in
 *the first argument against the string and executes the specified block on
 *every match passing an array containing the matches.
 *
 * The options parameter can be used to pass specific flags to the
 * regular expression engine. As of the moment of writing this functionality
 * has not been implemented yet. The only flag you can set at this moment is
 * the 'ignore' flag, just a test flag. This flag does not execute the block.
 *
 * Usage:
 *
 * 'hello world' findPattern: '([hl])' do: { :arr
 *  Pen write: (arr join: '|'), brk.
 * } options: ''.
 *
 * On every match the block gets executed and the matches are
 * passed to the block as arguments. You can also use this feature to replace
 * parts of the string, simply return the replacement string in your block.
 */
#ifdef POSIXRE
ctr_object *ctr_string_find_pattern_options_do(ctr_object *myself,
                                               ctr_argument *argumentList) {
  regex_t pattern;
  int reti;
  int regex_error = 0;
  size_t n = 511;
  size_t i = 0;
  regmatch_t matches[511];
  char *needle = ctr_heap_allocate_cstring(argumentList->object);
  char *options = ctr_heap_allocate_cstring(argumentList->next->next->object);
  uint8_t olen = strlen(options);
  uint8_t p = 0;
  uint8_t flagIgnore = 0;
  uint8_t flagNewLine = 0;
  uint8_t flagCI = 0;
  for (p = 0; p < olen; p++) {
    if (options[p] == '!') {
      flagIgnore = 1;
    }
    if (options[p] == 'n') {
      flagNewLine = 1;
    }
    if (options[p] == 'i') {
      flagCI = 1;
    }
  }
  ctr_object *block = argumentList->next->object;
  int eflags = REG_EXTENDED;
  if (flagNewLine)
    eflags |= REG_NEWLINE;
  if (flagCI)
    eflags |= REG_ICASE;
  reti = regcomp(&pattern, needle, eflags);
  if (reti) {
    CtrStdFlow =
        ctr_build_string_from_cstring("Could not compile regular expression.");
    return CtrStdNil;
  }
  char *haystack = ctr_heap_allocate_cstring(myself);
  size_t offset = 0;
  ctr_object *newString = ctr_build_empty_string();
  ctr_argument varg = {0}, *arg = &varg;
  ctr_argument argvb = {0}, *blockArguments = &argvb;
  ctr_argument argva = {0}, *arrayConstructorArgument = &argva;
  ctr_argument argvg = {0}, *group = &argvg;
  while (1) {
    regex_error = regexec(&pattern, haystack + offset, n, matches, 0);
    if (regex_error != 0)
      break;
    blockArguments->object =
        ctr_array_new(CtrStdArray, arrayConstructorArgument);
    for (i = 0; i < n; i++) {
      if (matches[i].rm_so == -1)
        break;
      size_t len = (matches[i].rm_eo - matches[i].rm_so);
      char *tmp = ctr_heap_allocate(len + 1);
      memcpy(tmp, haystack + offset + matches[i].rm_so, len);
      group->object = ctr_build_string_from_cstring(tmp);
      ctr_array_push(blockArguments->object, group);
      ctr_heap_free(tmp);
    }
    if (matches[0].rm_eo != -1) {
      arg->object = ctr_build_string(haystack + offset, matches[0].rm_so);
      ctr_object *replacement = ctr_block_run(block, blockArguments, block);
      arg->object = replacement == block ? arg->object : replacement;
      ctr_string_append(newString, arg);
      offset += matches[0].rm_eo;
    }
  }
  arg->object = ctr_build_string(haystack + offset, strlen(haystack + offset));
  ctr_string_append(newString, arg);
  ctr_heap_free(needle);
  ctr_heap_free(haystack);
  ctr_heap_free(options);
  regfree(&pattern);
  return newString;
}
#else
ctr_object *ctr_string_find_pattern_options_do(
    ctr_object *myself,
    ctr_argument
        *argumentList) { // pattern, blk, options: {!=Ignore, n=newline, i=CI}
  pcre *pattern;
  int regex_error_offset = 0;
  int regex_count = 0;
  size_t n = 511;
  int i = 0;
  int first = 1;
  int matches[511];
  char *needle = ctr_heap_allocate_cstring(argumentList->object);
  char *options = ctr_heap_allocate_cstring(argumentList->next->next->object);
  const char *err;

  uint8_t olen = strlen(options);
  uint8_t p = 0;
  uint8_t flagIgnore = 0;
  uint8_t flagNewLine = 0;
  uint8_t flagCI = 0;
  for (p = 0; p < olen; p++) {
    if (options[p] == '!') {
      flagIgnore = 1;
    }
    if (options[p] == 'n') {
      flagNewLine = 1;
    }
    if (options[p] == 'i') {
      flagCI = 1;
    }
  }
  ctr_object *block = argumentList->next->object;
  int eflags = PCRE_EXTENDED;
  if (flagNewLine)
    eflags |= PCRE_MULTILINE;
  if (flagCI)
    eflags |= PCRE_MULTILINE;
  pattern = pcre_compile(needle, eflags, &err, &regex_error_offset, NULL);
  if (pattern == NULL) {
    CtrStdFlow =
        ctr_format_str("E(pcre) %s at index %d", err, regex_error_offset);
    return CtrStdNil;
  }
  char *haystack = ctr_heap_allocate_cstring(myself);
  size_t offset = 0;
  size_t last_end = 0;
  ctr_object *newString = ctr_build_empty_string();
  ctr_argument argb = {0}, *blockArguments = &argb;
  ctr_argument argg = {0}, *group = &argg;
  ctr_argument arga = {0}, *arg = &arga;
  while ((regex_count > 0 || first) && !flagIgnore) {
    if (first)
      first = 0;
    regex_count =
        pcre_exec(pattern, NULL, haystack + offset,
                  myself->value.svalue->vlen - offset, 0, 0, matches, n);
    if (regex_count <= 0)
      break;
    blockArguments->object = ctr_array_new(CtrStdArray, NULL);
    for (i = 0; i < regex_count; i++) {
      int len = (matches[2 * i + 1] - matches[2 * i]);
      char *tmp = ctr_heap_allocate(len + 1);
      memcpy(tmp, haystack + offset + matches[2 * i], len);
      group->object = ctr_build_string_from_cstring(tmp);
      ctr_array_push(blockArguments->object, group);
      ctr_heap_free(tmp);
    }
    if (matches[0] != -1) {
      if (offset + matches[1] - last_end > 0) {
        arg->object = ctr_build_string(haystack + last_end,
                                       offset + matches[0] - last_end);
        ctr_string_append(newString, arg);
      }
      last_end = offset + matches[1];
      arg->object =
          ctr_build_string(haystack + offset, matches[1] - matches[0]);
      ctr_object *replacement = ctr_block_run(block, blockArguments, block);
      arg->object = replacement == block ? arg->object : replacement;
      ctr_string_append(newString, arg);
      offset += matches[1];
    } else
      break;
  }
  arg->object = ctr_build_string(haystack + offset, strlen(haystack + offset));
  ctr_string_append(newString, arg);
  // printf("-> %s\n", newString->value.svalue->value);
  // ctr_heap_free( (void*)err );
  ctr_heap_free(needle);
  ctr_heap_free(haystack);
  ctr_heap_free(options);
  pcre_free(pattern);
  return newString;
}
#endif

/**
 *[String] findPattern: [String] do: [Block].
 *
 * Same as findPattern:do:options: but without the options, no flags will
 * be send to the regex engine.
 */
ctr_object *ctr_string_find_pattern_do(ctr_object *myself,
                                       ctr_argument *argumentList) {
  if (!argumentList->next->next)
    argumentList->next->next = ctr_heap_allocate(sizeof (ctr_argument));
  argumentList->next->next->object = ctr_build_empty_string();
  ctr_object *answer;
  answer = ctr_string_find_pattern_options_do(myself, argumentList);
  return answer;
}

#ifdef POSIXRE
ctr_object *ctr_string_reg_replace(ctr_object *myself,
                                   ctr_argument *argumentList) {
  CtrStdFlow =
      ctr_build_string_from_cstring("Regex replace not implemented for POSIX; "
                                    "use String#findPattern:do:options:");
  return myself;
}
ctr_object *ctr_string_reg_compile(ctr_object *myself,
                                   ctr_argument *argumentList) {
  CtrStdFlow =
      ctr_build_string_from_cstring("Regex compile not implemented for POSIX; "
                                    "use String#findPattern:do:options:");
  return myself;
}
#else
ctr_object *ctr_string_reg_compile(ctr_object *myself,
                                   ctr_argument *argumentList) {
  (void) myself;

  if (argumentList->next->object) { // pattern, substitute, [options]
    ctr_object *pat = argumentList->object;
    ctr_object *sub = argumentList->next->object;
    ctr_object *opts =
        (argumentList->next->next && argumentList->next->next->object)
            ? argumentList->next->next->object
            : ctr_build_empty_string();
    char *p = ctr_heap_allocate_cstring(pat),
         *s = ctr_heap_allocate_cstring(sub),
         *o = ctr_heap_allocate_cstring(opts);
    int err;
    pcrs_job *job = pcrs_compile(p, s, o, &err);
    if (job == NULL) {
      ctr_heap_free(p);
      ctr_heap_free(o);
      ctr_heap_free(s);
      CtrStdFlow = ctr_build_string_from_cstring((char *)pcrs_strerror(err));
      return CtrStdNil;
    }
    ctr_object *res = ctr_object_make(CtrStdObject, NULL);
    ctr_resource *resource = ctr_heap_allocate(sizeof(*resource));
    resource->type = 73;
    resource->ptr = job;
    res->info.type = CTR_OBJECT_TYPE_OTEX;
    res->value.rvalue = resource;

    ctr_heap_free(p);
    ctr_heap_free(o);
    ctr_heap_free(s);
    return res;
  } else { // s/pattern/replacement/flags
    ctr_object *cmd = argumentList->object;
    char *c = ctr_heap_allocate_cstring(cmd);
    int err;
    pcrs_job *job = pcrs_compile_command(c, &err);
    if (job == NULL) {
      ctr_heap_free(cmd);
      CtrStdFlow = ctr_build_string_from_cstring((char *)pcrs_strerror(err));
      return CtrStdNil;
    }
    ctr_object *res = ctr_object_make(CtrStdObject, NULL);
    ctr_resource *resource = ctr_heap_allocate(sizeof(*resource));
    resource->type = 73;
    resource->ptr = job;
    res->info.type = CTR_OBJECT_TYPE_OTEX;
    res->value.rvalue = resource;
    ctr_heap_free(c);
    return res;
  }
}
ctr_object *ctr_string_reg_replace(ctr_object *myself,
                                   ctr_argument *argumentList) {
  (void) argumentList;

  if (argumentList->next->object) { // pattern, substitute, [options]
    ctr_object *pat = argumentList->object;
    ctr_object *sub = argumentList->next->object;
    ctr_object *opts =
        (argumentList->next->next && argumentList->next->next->object)
            ? argumentList->next->next->object
            : ctr_build_empty_string();
    char *p = ctr_heap_allocate_cstring(pat),
         *s = ctr_heap_allocate_cstring(sub),
         *o = ctr_heap_allocate_cstring(opts);
    int err;
    pcrs_job *job = pcrs_compile(p, s, o, &err);
    if (job == NULL) {
      ctr_heap_free(p);
      ctr_heap_free(o);
      ctr_heap_free(s);
      CtrStdFlow = ctr_build_string_from_cstring((char *)pcrs_strerror(err));
      return CtrStdNil;
    }
    char *input = ctr_heap_allocate_cstring(myself), *result;
    ctr_size length = myself->value.svalue->vlen, rlen;
    if (0 > (err = pcrs_execute(job, input, length, &result, &rlen))) {
      pcrs_free_job(job);
      ctr_heap_free(p);
      ctr_heap_free(o);
      ctr_heap_free(s);
      ctr_heap_free(input);
      CtrStdFlow = ctr_build_string_from_cstring((char *)pcrs_strerror(err));
      return CtrStdNil;
    }
    ctr_heap_free(p);
    ctr_heap_free(o);
    ctr_heap_free(s);
    ctr_heap_free(input);
    return ctr_build_string(result, rlen);
  } else { // s/pattern/replacement/flags
    ctr_object *cmd = argumentList->object;
    pcrs_job *job;
    int err;
    int alloc = 0;
    char *c;
    if (cmd->info.type == CTR_OBJECT_TYPE_OTEX) {
      job = cmd->value.rvalue->ptr;
    } else {
      alloc = 1;
      c = ctr_heap_allocate_cstring(cmd);
      job = pcrs_compile_command(c, &err);
      if (job == NULL) {
        ctr_heap_free(cmd);
        CtrStdFlow = ctr_build_string_from_cstring((char *)pcrs_strerror(err));
        return CtrStdNil;
      }
    }

    char *input = ctr_heap_allocate_cstring(myself), *result;
    ctr_size length = myself->value.svalue->vlen, rlen;
    if (0 > (err = pcrs_execute(job, input, length, &result, &rlen))) {
      if (alloc)
        pcrs_free_job(job);
      if (alloc)
        ctr_heap_free(c);
      ctr_heap_free(input);
      CtrStdFlow = ctr_build_string_from_cstring((char *)pcrs_strerror(err));
      return CtrStdNil;
    }
    return ctr_build_string(result, rlen);
  }
}
#endif

/**
 *[String] contains: [String]
 *
 * Returns True if the other string is a substring.
 */
ctr_object *ctr_string_contains(ctr_object *myself,
                                ctr_argument *argumentList) {
  return ctr_build_bool(
      ctr_internal_cast2number(ctr_string_index_of(myself, argumentList))
          ->value.nvalue > -1);
}

/**
 *[String] containsPattern: [String].
 *
 * Tests the pattern against the string and returns True if there is a match
 * and False otherwise.
 *
 * Usage:
 *
 * var match := 'Hello World' containsPattern: '[:space:]'.
 * #match will be True because there is a space in 'Hello World'
 */
#ifdef POSIXRE
ctr_object *ctr_string_contains_pattern(ctr_object *myself,
                                        ctr_argument *argumentList) {
  regex_t pattern;
  int regex_error = 0;
  int result = 0;
  char *error_message = ctr_heap_allocate(255);
  char *needle = ctr_heap_allocate_cstring(argumentList->object);
  char *haystack = ctr_heap_allocate_cstring(myself);
  ctr_object *answer;
  regex_error = regcomp(&pattern, needle, REG_EXTENDED);
  if (regex_error) {
    CtrStdFlow =
        ctr_build_string_from_cstring("Could not compile regular expression.");
    answer = CtrStdNil;
  } else {
    result = regexec(&pattern, haystack, 0, NULL, 0);
    if (!result) {
      answer = ctr_build_bool(1);
    } else if (result == REG_NOMATCH) {
      answer = ctr_build_bool(0);
    } else {
      CtrStdFlow = ctr_build_string_from_cstring(error_message);
      answer = CtrStdNil;
    }
  }
  regfree(&pattern);
  ctr_heap_free(error_message);
  ctr_heap_free(needle);
  ctr_heap_free(haystack);
  return answer;
}
#else
ctr_object *ctr_string_contains_pattern(ctr_object *myself,
                                        ctr_argument *argumentList) {
  pcre *pattern;
  int regex_error = 0;
  int result = 0;
  char *error_message = ctr_heap_allocate(255);
  char *needle = ctr_heap_allocate_cstring(argumentList->object);
  char *haystack = ctr_heap_allocate_cstring(myself);
  ctr_object *answer;
  pattern =
      pcre_compile(needle, PCRE_EXTENDED, (char const**) &error_message, &regex_error, NULL);
  if (!pattern) {
    CtrStdFlow = ctr_build_string_from_cstring(error_message);
    answer = CtrStdNil;
  } else {
    result = pcre_exec(pattern, NULL, haystack, myself->value.svalue->vlen, 0,
                       0, NULL, 0);
    if (!result) {
      answer = ctr_build_bool(1);
    } else if (result == PCRE_ERROR_NOMATCH) {
      answer = ctr_build_bool(0);
    } else {
      CtrStdFlow = ctr_build_string_from_cstring(error_message);
      answer = CtrStdNil;
    }
  }
  pcre_free(pattern);
  ctr_heap_free(needle);
  ctr_heap_free(haystack);
  return answer;
}
#endif

ctr_object *ctr_string_is_regex_pcre(ctr_object *myself,
                                     ctr_argument *argumentList) {
  (void) myself;
  (void) argumentList;

  int ispcre =
#ifndef POSIXRE
      1
#else
      0
#endif
      ;
  return ctr_build_bool(ispcre);
}

ctr_object *ctr_string_count_of(ctr_object *myself,
                                ctr_argument *argumentList) {
  ctr_object *sub_o = ctr_internal_cast2string(argumentList->object);
  char *sub = ctr_heap_allocate_cstring(sub_o);
  char *p = myself->value.svalue->value;
  int t = myself->value.svalue->vlen;
  int total = 0, i = 0, sublen = ctr_string_length(sub_o, NULL)->value.nvalue;
  while (t > i && (p = strstr(p, sub)) != NULL) {
    total++;
    p += sublen;
    i += sublen;
  }
  ctr_heap_free(sub);
  return ctr_build_number_from_float(total);
}

/**
 *[String] trim
 *
 * Trims a string. Removes surrounding white space characters
 * from string and returns the result as a new string object.
 *
 * Usage:
 *
 * ' hello ' trim. #hello
 *
 * The example above will strip all white space characters from the
 * recipient on both sides of the text. Also see: leftTrim and rightTrim
 * for variations of this message.
 */
ctr_object *ctr_string_trim(ctr_object *myself, ctr_argument *argumentList) {
  (void) argumentList;

  ctr_object *newString = NULL;
  char *str = myself->value.svalue->value;
  long len = myself->value.svalue->vlen;
  long i, begin, end, tlen;
  if (len == 0)
    return ctr_build_empty_string();
  i = 0;
  while (i < len && isspace(*(str + i)))
    i++;
  begin = i;
  i = len - 1;
  while (i > begin && isspace(*(str + i)))
    i--;
  end = i + 1;
  tlen = (end - begin);
  newString = ctr_build_string((str + begin), tlen);
  return newString;
}

/**
 *[String] leftTrim
 *
 * Removes all the whitespace at the left side of the string.
 *
 * Usage:
 *
 * message := ' hello world  '.
 * message leftTrim.
 *
 * The example above will remove all the whitespace at the left of the
 * string but leave the spaces at the right side intact.
 */
ctr_object *ctr_string_ltrim(ctr_object *myself, ctr_argument *argumentList) {
  (void) argumentList;

  ctr_object *newString = NULL;
  char *str = myself->value.svalue->value;
  size_t len = myself->value.svalue->vlen;
  size_t i = 0, begin;
  size_t tlen;
  if (len == 0)
    return ctr_build_empty_string();
  while (i < len && isspace(*(str + i)))
    i++;
  begin = i;
  i = len - 1;
  tlen = (len - begin);
  newString = ctr_build_string(str + begin, tlen);
  return newString;
}

ctr_object *ctr_string_padding(ctr_object *myself, ctr_argument *argumentList,
                               int left) {
  uint16_t padding;
  char *buffer;
  char *format;
  char *stringParam;
  ctr_size bufferSize;
  ctr_argument arga = {0}, *a = &arga;
  ctr_object *answer;
  ctr_object *formatObj;
  if (left == 1) {
    formatObj = ctr_build_string_from_cstring("%");
  } else {
    formatObj = ctr_build_string_from_cstring("%-");
  }
  padding =
      (uint16_t)ctr_internal_cast2number(argumentList->object)->value.nvalue;
  a->object = ctr_internal_cast2string(
      ctr_build_number_from_float((ctr_number)padding));
  formatObj = ctr_string_concat(formatObj, a);
  a->object = ctr_build_string_from_cstring("s");
  formatObj = ctr_string_concat(formatObj, a);
  format = ctr_heap_allocate_cstring(formatObj);
  bufferSize = (myself->value.svalue->vlen + padding + 1);
  buffer = ctr_heap_allocate(bufferSize);
  stringParam = ctr_heap_allocate_cstring(myself);
  sprintf(buffer, format, stringParam);
  answer = ctr_build_string_from_cstring(buffer);
  ctr_heap_free(buffer);
  ctr_heap_free(stringParam);
  ctr_heap_free(format);
  return answer;
}

ctr_object *ctr_string_padding_left(ctr_object *myself,
                                    ctr_argument *argumentList) {
  return ctr_string_padding(myself, argumentList, 1);
}

ctr_object *ctr_string_padding_right(ctr_object *myself,
                                     ctr_argument *argumentList) {
  return ctr_string_padding(myself, argumentList, 0);
}

/**
 *[String] rightTrim
 *
 * Removes all the whitespace at the right side of the string.
 *
 * Usage:
 *
 * message := ' hello world  '.
 * message rightTrim.
 *
 * The example above will remove all the whitespace at the right of the
 * string but leave the spaces at the left side intact.
 */
ctr_object *ctr_string_rtrim(ctr_object *myself, ctr_argument *argumentList) {
  (void) argumentList;

  ctr_object *newString = NULL;
  char *str = myself->value.svalue->value;
  long len = myself->value.svalue->vlen;
  long i = 0, end, tlen;
  if (len == 0)
    return ctr_build_empty_string();
  i = len - 1;
  while (i > 0 && isspace(*(str + i)))
    i--;
  end = i + 1;
  tlen = end;
  newString = ctr_build_string(str, tlen);
  return newString;
}

/**
 *[String] toNumber
 *
 * Converts string to a number.
 */
ctr_object *ctr_string_to_number(ctr_object *myself,
                                 ctr_argument *argumentList) {
  (void) argumentList;

  return ctr_build_number_from_string(myself->value.svalue->value,
                                      myself->value.svalue->vlen);
}

/**
 *[String] toBoolean
 *
 * Converts string to boolean
 */
ctr_object *ctr_string_to_boolean(ctr_object *myself,
                                  ctr_argument *argumentList) {
  (void) argumentList;

  return ctr_build_bool(!!myself->value.svalue->vlen);
}

/**
 *[String] split: [String]
 *
 * Converts a string to an array by splitting the string using
 * the specified delimiter (also a string).
 */
ctr_object *ctr_string_split(ctr_object *myself, ctr_argument *argumentList) {
  char *str = myself->value.svalue->value;
  long len = myself->value.svalue->vlen;
  ctr_object *delimObject = ctr_internal_cast2string(argumentList->object);
  char *dstr = delimObject->value.svalue->value;
  long dlen = delimObject->value.svalue->vlen;
  ctr_number times = 0;
  _Bool p = 0;
  if (argumentList->next && argumentList->next->object) {
    times = ctr_internal_cast2number(argumentList->next->object)->value.nvalue;
    p = 1;
  }
  ctr_argument earg = {0}, *arg = &earg;
  char *elem;
  ctr_object *arr = ctr_array_new(CtrStdArray, NULL);
  long i;
  long j = 0;
  char *buffer = ctr_heap_allocate(sizeof(char) * len);
  for (i = 0; i < len; i++) {
    buffer[j] = str[i];
    j++;
    if (ctr_internal_memmem(buffer, j, dstr, dlen, 0) != NULL &&
        (!p || times > 0)) {
      elem = ctr_heap_allocate(sizeof(char) * (j - dlen));
      memcpy(elem, buffer, j - dlen);
      arg->object = ctr_build_string(elem, j - dlen);
      ctr_array_push(arr, arg);
      ctr_heap_free(elem);
      j = 0;
      times--;
    }
  }
  if (j > 0) {
    elem = ctr_heap_allocate(sizeof(char) * j);
    memcpy(elem, buffer, j);
    arg->object = ctr_build_string(elem, j);
    ctr_array_push(arr, arg);
    ctr_heap_free(elem);
  }
  ctr_heap_free(buffer);
  return arr;
}

/**
 *[String] reSplit: [String]
 *
 * Converts a string to an array by splitting the string using
 * the specified regex (also a string).
 * ONLY WITH PCRE
 */

#ifdef POSIXRE
ctr_object *ctr_string_split_re(ctr_object *myself,
                                ctr_argument *argumentList) {
  CtrStdFlow = ctr_build_string_from_cstring(
      "Regex split not implemented for POSIX regex");
  return myself;
}
ctr_object *ctr_string_split_re_gen(ctr_object *myself,
                                    ctr_argument *argumentList) {
  CtrStdFlow = ctr_build_string_from_cstring(
      "Regex split not implemented for POSIX regex");
  return myself;
}
#else
ctr_object *ctr_string_split_re(ctr_object *myself,
                                ctr_argument *argumentList) {
  if (argumentList->object->info.type != CTR_OBJECT_TYPE_OTSTRING ||
      argumentList->object->value.svalue->vlen == 0) {
    CtrStdFlow = ctr_build_string_from_cstring("Invalid regex or not a string");
    return ctr_build_nil();
  }
  char *re = ctr_heap_allocate_cstring(argumentList->object),
       *str = ctr_heap_allocate_cstring(myself);
  ctr_object *ret = pcre_split(re, str);
  ctr_heap_free(re);
  ctr_heap_free(myself);
  return ret;
}
ctr_object *ctr_string_split_re_gen(ctr_object *myself,
                                    ctr_argument *argumentList) {
  if (argumentList->object->info.type != CTR_OBJECT_TYPE_OTSTRING ||
      argumentList->object->value.svalue->vlen == 0) {
    CtrStdFlow = ctr_build_string_from_cstring("Invalid regex or not a string");
    return ctr_build_nil();
  }
  char *re = ctr_heap_allocate_cstring(argumentList->object),
       *str = ctr_heap_allocate_cstring(myself);
  ctr_object *ret = pcre_split_gen(re, str);
  ctr_heap_free(re);
  ctr_heap_free(myself);
  return ret;
}
#endif

/**
 *[String] characters.
 *
 * Splits the string in UTF-8 characters and returns
 * those as an array.
 *
 * Usage:
 *
 * a := 'abc' characters.
 * a count. #3
 */
ctr_object *ctr_string_characters(ctr_object *myself,
                                  ctr_argument *argumentList) {
  (void) argumentList;

  ctr_size i;
  int charSize;
  ctr_object *arr;
  ctr_argument narg = {0}, *newArgumentList = &narg;
  arr = ctr_array_new(CtrStdArray, NULL);
  i = 0;
  while (i < myself->value.svalue->vlen) {
    charSize = ctr_utf8size(*(myself->value.svalue->value + i));
    newArgumentList->object =
        ctr_build_string(myself->value.svalue->value + i, charSize);
    if (!newArgumentList->object)
      continue;
    ctr_array_push(arr, newArgumentList);
    i += charSize;
  }
  return arr;
}

/**
 *[String] toByteArray
 *
 * Returns an array of bytes representing the string.
 */
ctr_object *ctr_string_to_byte_array(ctr_object *myself,
                                     ctr_argument *argumentList) {
  (void) argumentList;

  ctr_size i;
  ctr_object *arr;
  ctr_argument narg = {0}, *newArgumentList = &narg;
  arr = ctr_array_new(CtrStdArray, NULL);
  i = 0;
  while (i < myself->value.svalue->vlen) {
    newArgumentList->object = ctr_build_number_from_float(
        (double)(uint8_t) * (myself->value.svalue->value + i));
    ctr_array_push(arr, newArgumentList);
    i++;
  }
  return arr;
}

/**
 *[String] fmap: [Block]
 *
 * maps a function over the string and returns a string
 */
ctr_object *ctr_string_fmap(ctr_object *myself, ctr_argument *argumentList) {
  ctr_size i;
  ctr_object *arr;
  ctr_argument narg = {0}, *newArgumentList = &narg;
  ctr_object *fmapb = argumentList->object;
  CTR_ENSURE_TYPE_BLOCK(fmapb);
  arr = ctr_array_new(CtrStdArray, NULL);
  i = 0;
  while (i < ctr_string_length(myself, NULL)->value.nvalue) {
    newArgumentList->object = ctr_build_number_from_float(i);
    newArgumentList->object = ctr_string_at(myself, newArgumentList);
    newArgumentList->object = ctr_block_runIt(fmapb, newArgumentList);
    ctr_array_push(arr, newArgumentList);
    i++;
  }
  newArgumentList->object = ctr_build_empty_string();
  arr = ctr_array_join(arr, newArgumentList);
  return arr;
}

/**
 *[String] imap: [Block]
 *
 * maps a function over the string with indices and returns a string
 */
ctr_object *ctr_string_imap(ctr_object *myself, ctr_argument *argumentList) {
  ctr_size i;
  ctr_object *arr;
  ctr_argument narg = {0}, nnext = {0}, *newArgumentList = &narg;
  ctr_object *fmapb = argumentList->object;
  CTR_ENSURE_TYPE_BLOCK(fmapb);
  arr = ctr_array_new(CtrStdArray, NULL);
  newArgumentList->next = &nnext;
  i = 0;
  while (i < ctr_string_length(myself, NULL)->value.nvalue) {
    newArgumentList->object = ctr_build_number_from_float(i);
    newArgumentList->next->object = ctr_string_at(myself, newArgumentList);
    newArgumentList->object = ctr_block_runIt(fmapb, newArgumentList);
    ctr_array_push(arr, newArgumentList);
    i++;
  }
  newArgumentList->object = ctr_build_empty_string();
  arr = ctr_array_join(arr, newArgumentList);
  return arr;
}

/**
 *[String] filter: [Block]
 *
 * filters a string based on a predicate. returns a string
 */
ctr_object *ctr_string_filter(ctr_object *myself, ctr_argument *argumentList) {
  ctr_size i;
  ctr_object *arr;
  ctr_argument narg = {0}, nnext = {0}, *newArgumentList = &narg;
  ctr_object *fmapb = argumentList->object;
  CTR_ENSURE_TYPE_BLOCK(fmapb);
  arr = ctr_array_new(CtrStdArray, NULL);
  newArgumentList->next = &nnext;
  i = 0;
  while (i < ctr_string_length(myself, NULL)->value.nvalue) {
    newArgumentList->object = ctr_build_number_from_float(i);
    newArgumentList->next->object = ctr_string_at(myself, newArgumentList);
    if (ctr_block_runIt(fmapb, newArgumentList)->value.bvalue) {
      newArgumentList->object = newArgumentList->next->object;
      ctr_array_push(arr, newArgumentList);
    }
    i++;
  }
  newArgumentList->object = ctr_build_empty_string();
  arr = ctr_array_join(arr, newArgumentList);
  return arr;
}

/**
 *[String] appendByte: [Number].
 *
 * Appends a raw byte to a string.
 */
ctr_object *ctr_string_append_byte(ctr_object *myself,
                                   ctr_argument *argumentList) {
  char *dest;
  char byte;
  byte = (uint8_t)ctr_internal_cast2number(argumentList->object)->value.nvalue;
  dest = ctr_heap_allocate(myself->value.svalue->vlen + 1);
  memcpy(dest, myself->value.svalue->value, myself->value.svalue->vlen);
  *(dest + myself->value.svalue->vlen) = byte;
  if (myself->value.svalue->vlen > 0) {
    ctr_heap_free(myself->value.svalue->value);
  }
  myself->value.svalue->value = dest;
  myself->value.svalue->vlen++;
  return myself;
}

/**
 *[String(length=1)] charSub: [Number | String(length=1)]
 *
 * works on the underlaying character.
 */
ctr_object *ctr_string_csub(ctr_object *myself, ctr_argument *argumentList) {
  int v = argumentList->object->info.type == CTR_OBJECT_TYPE_OTSTRING;
  if (!v && argumentList->object->info.type != CTR_OBJECT_TYPE_OTNUMBER) {
    CtrStdFlow = ctr_build_string_from_cstring(
        "string minus expects character or number.");
    return myself;
  }
  if (ctr_string_length(myself, NULL)->value.nvalue > 1 ||
      (v && ctr_string_length(argumentList->object, NULL)->value.nvalue > 1)) {
    CtrStdFlow = ctr_build_string_from_cstring(
        "underlaying string for '-' must be one character only.");
    return myself;
  }

  return ctr_invoke_variadic(
      ctr_build_string_from_cstring(""), &ctr_string_append_byte, 1,
      ctr_build_number_from_float(
          ctr_invoke_variadic(myself, &ctr_string_byte_at, 1,
                              ctr_build_number_from_float(0))
              ->value.nvalue -
          (v ? ctr_invoke_variadic(argumentList->object, &ctr_string_byte_at, 1,
                                   ctr_build_number_from_float(0))
                   ->value.nvalue
             : argumentList->object->value.nvalue)));
}

/**
 *[String(length=1)] charAdd: [Number | String(length=1)]
 *
 * works on the underlaying character.
 */
ctr_object *ctr_string_cadd(ctr_object *myself, ctr_argument *argumentList) {
  int v = argumentList->object->info.type == CTR_OBJECT_TYPE_OTSTRING;
  if (!v && argumentList->object->info.type != CTR_OBJECT_TYPE_OTNUMBER) {
    CtrStdFlow = ctr_build_string_from_cstring(
        "string minus expects character or number.");
    return myself;
  }
  if (ctr_string_length(myself, NULL)->value.nvalue > 1 ||
      (v && ctr_string_length(argumentList->object, NULL)->value.nvalue > 1)) {
    CtrStdFlow = ctr_build_string_from_cstring(
        "underlaying string for '-' must be one character only.");
    return myself;
  }

  return ctr_invoke_variadic(
      ctr_build_string_from_cstring(""), &ctr_string_append_byte, 1,
      ctr_build_number_from_float(
          ctr_invoke_variadic(myself, &ctr_string_byte_at, 1,
                              ctr_build_number_from_float(0))
              ->value.nvalue +
          (v ? ctr_invoke_variadic(argumentList->object, &ctr_string_byte_at, 1,
                                   ctr_build_number_from_float(0))
                   ->value.nvalue
             : argumentList->object->value.nvalue)));
}

/**
 *[String] htmlEscape
 *
 * Escapes HTML chars.
 */

ctr_object *ctr_string_html_escape(ctr_object *myself,
                                   ctr_argument *argumentList) {
  (void) argumentList;

  ctr_object *newString = NULL;
  char *str = myself->value.svalue->value;
  long len = myself->value.svalue->vlen;
  char *tstr;
  long i = 0;
  long j = 0;
  long k = 0;
  long rlen;
  long tlen = 0;
  long tag_len = 0;
  long tag_rlen = 0;
  char *replacement;
  for (i = 0; i < len; i++) {
    char c = str[i];
    switch (c) {
    case '<':
      tag_len += 4;
      tag_rlen += 1;
      break;
    case '>':
      tag_len += 4;
      tag_rlen += 1;
      break;
    case '&':
      tag_len += 5;
      tag_rlen += 1;
      break;
    case '"':
      tag_len += 6;
      tag_rlen += 1;
      break;
    case '\'':
      tag_len += 6;
      tag_rlen += 1;
      break;
    default:
      if (c < 32) {
        tag_len += 7; // &#nnnn;
        tag_rlen += 1;
      }
      break;
    }
  }
  tlen = len + tag_len - tag_rlen;
  tstr = ctr_heap_allocate(tlen * sizeof(char));
  for (i = 0; i < len; i++) {
    char c = str[i];
    switch (c) {
    case '<':
      replacement = "&lt;";
      rlen = 4;
      for (j = 0; j < rlen; j++)
        tstr[k++] = replacement[j];
      break;
    case '>':
      replacement = "&gt;";
      rlen = 4;
      for (j = 0; j < rlen; j++)
        tstr[k++] = replacement[j];
      break;
    case '&':
      replacement = "&amp;";
      rlen = 5;
      for (j = 0; j < rlen; j++)
        tstr[k++] = replacement[j];
      break;
    case '"':
      replacement = "&quot;";
      rlen = 6;
      for (j = 0; j < rlen; j++)
        tstr[k++] = replacement[j];
      break;
    case '\'':
      replacement = "&apos;";
      rlen = 6;
      for (j = 0; j < rlen; j++)
        tstr[k++] = replacement[j];
      break;
    default:
      if (c < 32) {
        char rep[8];
        sprintf(rep, "&#%04d;", c);
        replacement = rep;
        rlen = 7;
        for (j = 0; j < rlen; j++)
          tstr[k++] = replacement[j];
      }
      else
        tstr[k++] = str[i];
      break;
    }
  }
  newString = ctr_build_string(tstr, tlen);
  ctr_heap_free(tstr);
  return newString;
}

// ctr_object *
// ctr_string_literal_escape (ctr_object * myself, ctr_argument * argumentList)
// {
//
// }

/**
 *[String] hashWithKey: [String]
 *
 * Returns the hash of the recipient String using the specified key.
 * The default hash in Citron is the SipHash which is also used internally.
 * SipHash can protect against hash flooding attacks.
 */
ctr_object *ctr_string_hash_with_key(ctr_object *myself,
                                     ctr_argument *argumentList) {
  char *keyString = ctr_heap_allocate_cstring(argumentList->object);
  if (strlen(keyString) < 16) {
    ctr_heap_free(keyString);
    CtrStdFlow =
        ctr_build_string_from_cstring("Key must be exactly 16 bytes long.");
    return CtrStdNil;
  }
  uint64_t t = siphash24(myself->value.svalue->value,
                         myself->value.svalue->vlen, keyString);
  char *dest = ctr_heap_allocate(40);
  snprintf(dest, 40, "%" PRIu64, t);
  ctr_object *hash = ctr_build_string_from_cstring(dest);
  ctr_heap_free(dest);
  ctr_heap_free(keyString);
  return hash;
}

/**
 *[String] eval
 *
 * Evaluates the contents of the string as code.
 *
 * Usage:
 *
 * a := 'Array < 1 ; 2 ; 3' eval.
 * x := a @ 2. #3
 */
ctr_object *ctr_string_eval(ctr_object *myself, ctr_argument *argumentList) {
  (void) argumentList;

  ctr_tnode *parsedCode;
  char *pathString;
  ctr_object *result;
  ctr_object *code;

  if (myself->value.svalue->vlen == 0)
    return myself;

#ifdef EVALSECURITY
  /* activate white-list based security profile */
  ctr_command_security_profile ^= CTR_SECPRO_EVAL;
#endif

  pathString = ctr_heap_allocate_tracked(sizeof(char) * 5);
  memcpy(pathString, "<eval>\0", 7);
  /* add a return statement so we can catch result */
  ctr_argument narg = {0}, *newArgumentList = &narg;
  newArgumentList->object = myself;
  code =
      ctr_string_append(ctr_build_string_from_cstring("^\n"), newArgumentList);
  newArgumentList->object = ctr_build_string_from_cstring("\n.");
  code = ctr_string_append(code, newArgumentList);
  ctr_program_length = code->value.svalue->vlen;
  parsedCode = ctr_cparse_parse(code->value.svalue->value, pathString);
  ctr_cwlk_subprogram++;
  result = ctr_cwlk_run(parsedCode);
  ctr_cwlk_subprogram--;
  if (result == NULL)
    result = CtrStdNil;

#ifdef EVALSECURITY
  ctr_command_security_profile ^= CTR_SECPRO_EVAL;
#endif

  return result;
}

/**
 *[String] exec
 *
 * Executes the contents of the string as code.
 * Does <i>not</i> return a value unless the code itself does.
 */
ctr_object *ctr_string_exec(ctr_object *myself, ctr_argument *argumentList) {
  (void) argumentList;

  ctr_tnode *parsedCode;
  char *pathString;
  ctr_object *result;

  if (myself->value.svalue->vlen == 0)
    return CtrStdNil;

  pathString = ctr_heap_allocate_tracked(sizeof(char) * 5);
  memcpy(pathString, "<eval>\0", 7);
  ctr_program_length = myself->value.svalue->vlen;
  parsedCode = ctr_cparse_parse(myself->value.svalue->value, pathString);
  ctr_cwlk_subprogram++;
  result = ctr_cwlk_run(parsedCode);
  ctr_cwlk_subprogram--;

  return CtrStdNil;
}

/**
 *[String] escapeQuotes.
 *
 * Escapes all single quotes in a string. Sending this message to a
 * string will cause all single quotes (') to be replaced with (\').
 */
ctr_object *ctr_string_quotes_escape(ctr_object *myself,
                                     ctr_argument *argumentList) {
  (void) argumentList;

  ctr_object *answer;
  char *str;
  ctr_size len;
  ctr_size i;
  ctr_size j;
  len = myself->value.svalue->vlen;
  for (i = 0; i < myself->value.svalue->vlen; i++) {
    if (*(myself->value.svalue->value + i) == '\'') {
      len++;
    }
  }
  str = ctr_heap_allocate(len + 1);
  j = 0;
  for (i = 0; i < myself->value.svalue->vlen; i++) {
    if (*(myself->value.svalue->value + i) == '\'') {
      str[j + i] = '\\';
      j++;
    }
    str[j + i] = *(myself->value.svalue->value + i);
  }
  answer = ctr_build_string_from_cstring(str);
  ctr_heap_free(str);
  return answer;
}

ctr_object *ctr_string_dquotes_escape(ctr_object *myself,
                                      ctr_argument *argumentList) {
  (void) argumentList;

  ctr_object *answer;
  char *str;
  ctr_size len;
  ctr_size i;
  ctr_size j;
  len = myself->value.svalue->vlen;
  for (i = 0; i < myself->value.svalue->vlen; i++) {
    if (*(myself->value.svalue->value + i) == '"') {
      len++;
    }
  }
  str = ctr_heap_allocate(len + 1);
  j = 0;
  for (i = 0; i < myself->value.svalue->vlen; i++) {
    if (*(myself->value.svalue->value + i) == '"') {
      str[j + i] = '\\';
      j++;
    }
    str[j + i] = *(myself->value.svalue->value + i);
  }
  answer = ctr_build_string_from_cstring(str);
  ctr_heap_free(str);
  return answer;
}

/**
 *[String] randomizeBytesWithLength: [Number].
 *
 * Returns a randomized string with the specified length using the pool of
 * bytes contained in the String object.
 */
ctr_object *ctr_string_randomize_bytes(ctr_object *myself,
                                       ctr_argument *argumentList) {
  size_t i;
  size_t j;
  size_t plen;
  size_t len;
  char *buffer;
  char *newBuffer;
  ctr_object *answer;
  plen = myself->value.svalue->vlen;
  len = (size_t)ctr_internal_cast2number(argumentList->object)->value.nvalue;
  if (len == 0)
    return ctr_build_empty_string();
  buffer = ctr_heap_allocate_cstring(myself);
  newBuffer = (char *)ctr_heap_allocate(len);
  for (i = 0; i < len; i++) {
    j = (ctr_size)((random() % (plen)));
    newBuffer[i] = buffer[j];
  }
  answer = ctr_build_string_from_cstring(newBuffer);
  ctr_heap_free(newBuffer);
  ctr_heap_free(buffer);
  return answer;
}

/**
 *[String] reverse
 *
 * reverse the string
 */
ctr_object *ctr_string_reverse(ctr_object *myself, ctr_argument *argumentList) {
  (void) argumentList;

  ctr_object *newString = ctr_build_empty_string();
  size_t i = ctr_string_length(myself, NULL)->value.nvalue;
  ctr_argument varg = {0}, *args = &varg;
  for (; i > 0; i--) {
    args->object = ctr_build_number_from_float(i - 1);
    args->object = ctr_string_at(myself, args);
    ctr_string_append(newString, args);
  }
  return newString;
}

/**@I_OBJ_DEF Tuple*/
/**
 * Tuple
 *
 * Literal:
 *
 * [ item , item , ... ] or []
 *
 * Examples:
 *
 * [ 1 , 2 , 3 ]
 * []
 *
 */
ctr_object *ctr_build_immutable(ctr_tnode *node) {
  ctr_object *tupleobj = ctr_internal_create_object(CTR_OBJECT_TYPE_OTARRAY);
  ctr_set_link_all(tupleobj, CtrStdArray);
  tupleobj->value.avalue =
      (ctr_collection *)ctr_heap_allocate(sizeof(ctr_collection));
  tupleobj->value.avalue->immutable = 1;
  tupleobj->value.avalue->length = 1;
  tupleobj->value.avalue->elements =
      (ctr_object **)ctr_heap_allocate(sizeof(ctr_object *) * 1);
  tupleobj->value.avalue->head = 0;
  tupleobj->value.avalue->tail = 0;
  ctr_object *result;
  ctr_argument varg = {0}, *args = &varg;
  ctr_tlistitem *items = node->nodes->node->nodes;

  char *wasReturn = "";
  while (items && items->node) {
    result = ctr_cwlk_expr(items->node, wasReturn);
    result->info.raw = 1;
    args->object = result;
    ctr_array_push_imm(tupleobj, args);
    if (!items->next)
      break;
    items = items->next;
  }
  tupleobj->info.raw = 1;
  return tupleobj;
}

/**@I_OBJ_DEF CodeBlock*/
/**
 * Block
 *
 * Literal:
 *
 * { parameters (if any) here... code here... }
 *
 * each parameter has to be prefixed with
 * a colon (:).
 *
 * Examples:
 *
 * { Pen write: 'a simple code block'. } run.
 * { :param Pen write: param. } applyTo: 'write this!'.
 * { :a :b ^ a + b. } applyTo: 1 and: 2.
 * { :a :b :c ^ a + b + c. } applyTo: 1 and: 2 and: 3.
 *
 * to enable context auto-capture,
 * qualify captures with the 'const' modifier
 */
ctr_object *ctr_build_block(ctr_tnode *node) {
  ctr_object *codeBlockObject =
      ctr_internal_create_object(CTR_OBJECT_TYPE_OTBLOCK);
  codeBlockObject->value.block = node;
  ctr_set_link_all(codeBlockObject, CtrStdBlock);
  //printf("----------- %p ------------\n", node);
  //ctr_internal_debug_tree(node, 1);
  ctr_capture_refs(node, codeBlockObject);
  //printf("----------- %p ------------\n", node);
  //ctr_internal_debug_tree(node, 1);
  //printf("----------- %p ------------\n", node);
  return codeBlockObject;
}

ctr_tnode *ctr_expr_to_block(ctr_tnode *node) {
  ctr_tnode *blknode = ctr_heap_allocate(sizeof(*blknode));
  blknode->type = CTR_AST_NODE_CODEBLOCK;
  blknode->nodes =
      ctr_heap_allocate(sizeof(ctr_tlistitem)); // codeBlockPart1 : arguments
  blknode->nodes->node = ctr_heap_allocate(sizeof(ctr_tnode)); // arguments
  blknode->nodes->node->type = CTR_AST_NODE_PARAMLIST;
  blknode->nodes->next =
      ctr_heap_allocate(sizeof(ctr_tlistitem)); // codeBlockPart2 : instrlist
  blknode->nodes->next->node = ctr_heap_allocate(sizeof(*blknode)); // instrlist
  blknode->nodes->next->node->type = CTR_AST_NODE_INSTRLIST;
  blknode->nodes->next->node->nodes = ctr_heap_allocate(sizeof(ctr_tlistitem));
  blknode->nodes->next->node->nodes->node =
      ctr_heap_allocate(sizeof(ctr_tnode));
  blknode->nodes->next->node->nodes->node->type = CTR_AST_NODE_RETURNFROMBLOCK;
  blknode->nodes->next->node->nodes->node->nodes =
      ctr_heap_allocate(sizeof(ctr_tlistitem));
  blknode->nodes->next->node->nodes->node->nodes->node = node;
  return blknode;
}

static ctr_object free_ref_marker;
ctr_object *ctr_scan_free_refs(ctr_tnode *node) {
  ctr_object *ret = ctr_map_new(CtrStdMap, NULL);
  ctr_argument argmv = {0}, *argm = &argmv;
  ctr_tnode *t = node;
  switch (t->type) {
  case CTR_AST_NODE_REFERENCE: {
    ctr_object *key = ctr_build_string(t->value, t->vlen);
    ctr_object *value = ctr_find_(key, 1);
    if (!value)
      ctr_internal_object_set_property(ret, key, &free_ref_marker, 0);
    break;
  }
  case CTR_AST_NODE_CODEBLOCK:
    if (!t->lexical) {
      argm->object = ctr_scan_free_refs(t->nodes->next->node); // instrlist
      ret = ctr_map_merge(ret, argm);
    }
    break;
  case CTR_AST_NODE_EXPRASSIGNMENT:
    argm->object = ctr_scan_free_refs(t->nodes->next->node); // expr
    ret = ctr_map_merge(ret, argm);
    break;
  case CTR_AST_NODE_EXPRMESSAGE:
    argm->object = ctr_scan_free_refs(t->nodes->node); // Receiver
    ret = ctr_map_merge(ret, argm);
    ctr_tlistitem *tli = t->nodes->next;
    while (tli) {
      argm->object = ctr_scan_free_refs(tli->node); // argument
      ret = ctr_map_merge(ret, argm);
      tli = tli->next;
    }
    break;
  case CTR_AST_NODE_BINMESSAGE:
    argm->object = ctr_scan_free_refs(t->nodes->node); // argument
    ret = ctr_map_merge(ret, argm);
    break;
  case CTR_AST_NODE_IMMUTABLE: {
    ctr_tlistitem *tli = t->nodes->node->nodes;
    while (tli) {
      argm->object = ctr_scan_free_refs(tli->node); // argument
      ret = ctr_map_merge(ret, argm);
      tli = tli->next;
    }
    break;
  }
  case CTR_AST_NODE_KWMESSAGE: {
    ctr_tlistitem *tli = t->nodes;
    while (tli) {
      argm->object = ctr_scan_free_refs(tli->node); // argument
      ret = ctr_map_merge(ret, argm);
      tli = tli->next;
    }
    break;
  }
  case CTR_AST_NODE_NESTED:
    argm->object = ctr_scan_free_refs(t->nodes->node); // inner
    ret = ctr_map_merge(ret, argm);
    break;
  case CTR_AST_NODE_LTRNUM:
  case CTR_AST_NODE_PARAMLIST:
  case CTR_AST_NODE_ENDOFPROGRAM:
  case CTR_AST_NODE_LTRSTRING:
  case CTR_AST_NODE_LTRBOOLFALSE:
  case CTR_AST_NODE_LTRBOOLTRUE:
  case CTR_AST_NODE_LTRNIL:
  default:
    break;
  }
  //      if (!li->next)
  //              break;
  //      li = li->next;
  //      t = li->node;
  // }
  return ret;
}

ctr_object *ctr_array_internal_product(ctr_object *myself,
                                       ctr_argument *argumentList) {
  ctr_argument argl;
  if (!argumentList)
    argumentList = &argl;
  ctr_object *prod;
  ctr_object *el;
  ctr_size i = 0;
  for (i = myself->value.avalue->tail; i < myself->value.avalue->head; i++) {
    if (!i) {
      prod = *myself->value.avalue->elements;
      if (myself->value.avalue->head - myself->value.avalue->tail == 1) {
        // single element, explode it
        el = prod;
        switch (el->info.type) {
        case CTR_OBJECT_TYPE_OTARRAY: {
          ctr_object *new_arr = ctr_array_new(CtrStdArray, NULL);
          for (ctr_size j = el->value.avalue->tail; j < el->value.avalue->head;
               j++) {
            ctr_object *elarr = ctr_array_new(CtrStdArray, NULL);
            argumentList->object = *(el->value.avalue->elements + j);
            argumentList->object = ctr_array_push(elarr, argumentList);
            ctr_array_push(new_arr, argumentList);
          }
          return new_arr;
        }
        default: {
          ctr_object *fmap = ctr_get_responder(el, "fmap:", 5);
          if (!fmap)
            return myself;
          argumentList->object = ctr_string_eval(
              ctr_build_string_from_cstring("{:value ^[value].}"), NULL);
          prod = ctr_send_message(el, "fmap:", 5, argumentList);
          if (prod->interfaces->link == ctr_std_generator)
            prod = ctr_generator_toarray(prod, argumentList);
          return prod;
        }
        }
      }
      continue;
    }
    el = *(myself->value.avalue->elements + i);
    argumentList->object = el;
    prod = ctr_send_message(prod, "*", 1, argumentList);
  }
  return (prod);
}

ctr_object *ctr_build_listcomp(ctr_tnode *node) {
  ctr_tnode *main_expr = node->nodes->node,
            *generators = node->nodes->next->node,
            *preds = node->nodes->next->next->node;

  ctr_object *free_refs = ctr_map_keys(ctr_scan_free_refs(main_expr), NULL),
             *mainexprb = ctr_build_block(ctr_expr_to_block(main_expr));
  ctr_object *resolved_refs = ctr_array_new(CtrStdArray, NULL);

  ctr_object *bindings = ctr_array_new(CtrStdArray, NULL);
  ctr_object *predicates = ctr_array_new(CtrStdArray, NULL);
  ctr_argument *argm = ctr_heap_allocate(sizeof(*argm));
  char dummy;

  //(gen*) -> [gen*]
  if (generators) {
    ctr_tlistitem *generator = generators->nodes;
    while (generator) {
      ctr_object *exprn;
      ctr_tlistitem *genv = generator;

      if (generator->node->type == CTR_AST_NODE_NESTED &&
          generator->node->nodes->node->type == CTR_AST_NODE_EXPRMESSAGE &&
          generator->node->nodes->node->nodes->node->vlen == 2 &&
          strncmp(generator->node->nodes->node->nodes->node->value, "me",
                  generator->node->nodes->node->nodes->node->vlen) == 0) {
        generator = generator->node->nodes->node->nodes->next;
        if (generator->node->type == CTR_AST_NODE_KWMESSAGE) {
          int i = 0;
          char *sv = NULL;
          while ((sv = strchr(generator->node->value, ':')) != NULL && i++ == 0)
            ;
          if (i != 2) {
            CtrStdFlow =
                ctr_format_str("EExpected exactly one keyword to name the "
                               "expression in listcomp, not %d (%s)",
                               i, generator->node->value);
            return CtrStdNil;
          }
          exprn = ctr_build_string(generator->node->value,
                                   generator->node->vlen - 1);
          generator = generator->node->nodes;
        } else {
          CtrStdFlow = ctr_build_string_from_cstring(
              "Invalid use of non-keyword message to dynamic resolution `me' "
              "in list comprehension");
          return CtrStdNil;
        }
      } else {
        if (free_refs->value.avalue->head - free_refs->value.avalue->tail ==
            0) {
          CtrStdFlow = ctr_build_string_from_cstring(
              "Extraneous expression without a name in a list comprehension");
          return CtrStdNil;
        }
        exprn = ctr_array_shift(free_refs, NULL);
      }
      argm->object = exprn;
      // name
      ctr_array_push(resolved_refs, argm);
      // binding expression
      argm->object = ctr_ast_from_node(generator->node);
      ctr_array_push(bindings, argm);
      generator = genv->next;
    }
  }
  size_t ps, fs;
  if ((ps = ctr_array_count(bindings, NULL)->value.nvalue) <
      (fs = ctr_array_count(resolved_refs, NULL)->value.nvalue)) {
    CtrStdFlow = ctr_format_str(
        "-Number of bindings do not match the number of symbols (%d vs %d)", ps,
        fs);
    return CtrStdNil;
  }
  ctr_object* gen_handler_s = ctr_build_string_from_cstring(
    "{:obj "
      "obj isA: Generator, ifTrue: {"
        "^obj toArray."
      "} ifFalse: {"
        "^obj."
      "}."
    "}"
  );
  ctr_object *gen_handler = ctr_string_eval(gen_handler_s, NULL);
  //(pred*) -> [{^pred}*]
  if (preds) {
    ctr_tlistitem *predicate = preds->nodes;
    while (predicate) {
      if (predicate->node) {
        argm->object = ctr_build_block(ctr_expr_to_block(predicate->node));
        ctr_array_push(predicates, argm);
      }
      predicate = predicate->next;
    }
  }
  if (!generators && !preds) { //[e ,,,]
    ctr_object *res = ctr_array_new(CtrStdArray, NULL);
    argm->object = ctr_cwlk_expr(main_expr, &dummy);
    return ctr_array_push(res, argm);
  }
  if (!generators && preds) { //[e ,,, p+] -> [e] if all(p) else []
    ctr_object *res = ctr_array_new(CtrStdArray, NULL);
    ctr_object *filter_s = ctr_build_string_from_cstring("{:pblk ^pblk run.}");
    argm->object = ctr_string_eval(filter_s, NULL);
    if (ctr_array_all(predicates, argm)->value.bvalue) {
      argm->object = ctr_cwlk_expr(main_expr, &dummy);
      return ctr_array_push(res, argm);
    }
    return res;
  }
  if (generators && !preds) { // no filter: [e ,, gs@g+] -> [evaluate(e) ]
    ctr_object *filter_s =
        ctr_build_string_from_cstring("{:gen "
                                      "var syms is my syms. "
                                      "^{:blk "
                                      "^const syms letEqual: const gen in: blk."
                                      "}."
                                      "}");
    argm->object = ctr_string_eval(filter_s, NULL);
    ctr_internal_object_add_property(
        argm->object, ctr_build_string_from_cstring("syms"), resolved_refs, 0);
    ctr_object *filter_sobj = argm->object;
    ctr_object *filter_sv = ctr_build_string_from_cstring(
        "{"
        "var gen-handler is my gen-handler."
        "^(my names fmap: \\:__vname gen-handler applyTo: (Reflect getObject: __vname)) "
        "internal-product fmap: my filter_s."
        "}");
    ctr_object *filter_svobj;
    filter_svobj = ctr_string_eval(filter_sv, NULL);
    ctr_internal_object_add_property(
        filter_svobj, ctr_build_string_from_cstring("gen-handler"), gen_handler,
        CTR_CATEGORY_PRIVATE_PROPERTY);
    ctr_internal_object_add_property(
        filter_svobj, ctr_build_string_from_cstring("names"), resolved_refs,
        CTR_CATEGORY_PRIVATE_PROPERTY);
    ctr_internal_object_add_property(
        filter_svobj, ctr_build_string_from_cstring("filter_s"), filter_sobj,
        CTR_CATEGORY_PRIVATE_PROPERTY);
    // ctr_argument arg = {bindings, NULL};
    // ctr_console_writeln(CtrStdConsole, &arg);
    // names letEqualAst: bindings in: { internal-product[names-as-names] fmap:
    // filter_s }, fmap: (main_expr $)
    ctr_object *bindingfns = ctr_send_message_variadic(
        resolved_refs, "letEqualAst:in:", 15, 2, bindings, filter_svobj);
    ctr_object *call_s =
        ctr_build_string_from_cstring("{:blk ^blk applyTo: my main_expr.}");
    argm->object = ctr_string_eval(call_s, NULL);
    ctr_internal_object_add_property(
        argm->object, ctr_build_string_from_cstring("main_expr"), mainexprb, 0);
    ctr_object *res = ctr_send_message(bindingfns, "fmap:", 5, argm);
    // ctr_object *res = ctr_array_fmap (bindingfns, argm);
    return res;
  }
  // expression with generators and predicates
  ctr_object *filter_s = ctr_build_string_from_cstring(
      "{:gen "
      "var syms is my syms."
      "my filters fmap: {:filter "
      "^syms letEqualAst: gen in: filter."
      "}, all: {:x ^x.}, not continue. "
      "^{:blk"
        "^const syms letEqualAst: const gen in: blk."
      "}."
      "}");
  argm->object = ctr_string_eval(filter_s, NULL);
  ctr_internal_object_add_property(
      argm->object, ctr_build_string_from_cstring("syms"), resolved_refs, 0);
  ctr_internal_object_add_property(
      argm->object, ctr_build_string_from_cstring("filters"), predicates, 0);
  ctr_object *filter_sobj = argm->object;
  ctr_object *filter_sv = ctr_build_string_from_cstring(
      "{"
      "var gen-handler is my gen-handler."
      "var nvs is (my names fmap: \\:__vname gen-handler applyTo: (Reflect getObject: __vname))."
      "^nvs internal-product fmap: my filter_s."
      "}");
  ctr_object *filter_svobj;
  filter_svobj = ctr_string_eval(filter_sv, NULL);
  ctr_internal_object_add_property(
      filter_svobj, ctr_build_string_from_cstring("gen-handler"), gen_handler,
      CTR_CATEGORY_PRIVATE_PROPERTY);
  ctr_internal_object_add_property(
      filter_svobj, ctr_build_string_from_cstring("names"), resolved_refs,
      CTR_CATEGORY_PRIVATE_PROPERTY);
  ctr_internal_object_add_property(filter_svobj,
                                   ctr_build_string_from_cstring("filter_s"),
                                   filter_sobj, CTR_CATEGORY_PRIVATE_PROPERTY);
  ctr_object *bindingfns = ctr_send_message_variadic(
      resolved_refs, "letEqualAst:in:", 15, 2, bindings, filter_svobj);
  ctr_object *call_s =
      ctr_build_string_from_cstring("{:blk ^blk applyTo: my main_expr.}");
  argm->object = ctr_string_eval(call_s, NULL);
  ctr_internal_object_add_property(
      argm->object, ctr_build_string_from_cstring("main_expr"), mainexprb, 0);
  ctr_object *res = ctr_send_message(bindingfns, "fmap:", 5, argm);
  // ctr_object *res = ctr_array_fmap (bindingfns, argm);
  return res;
}

/**
 * @internal
 *
 * Captures all const qualified values and adds them to private space
 * If a nested block requires a capture, it will be added right now, and ignored
 * if the binding cannot be found at the time
 */
void ctr_capture_refs_(ctr_tnode *ti, ctr_object *block, ctr_object *parent,
                       int noerror);
void ctr_capture_refs(ctr_tnode *ti, ctr_object *block) {
  ctr_capture_refs_(ti, block, ctr_find_(&CTR_CLEX_KW_ME, 0), 0);
}

void ctr_capture_refs_(ctr_tnode *ti, ctr_object *block, ctr_object *parent,
                       int noerror) {
  ctr_tlistitem *li;
  ctr_tnode *t;
  // if (indent>20) exit(1);
  li = ti->nodes;
  if (!li)
    return;
  t = li->node;
  while (1) {
    if (!t)
      return;
    switch (t->type) {
    case CTR_AST_NODE_REFERENCE:
      if (t->modifier == 3) {
        ctr_object *key = ctr_build_string(t->value, t->vlen);
        if (!!ctr_internal_object_find_property(block, key, 0))
          return;
        ctr_object *value = ctr_find_(key, 1);
        if (!value && parent)
          value = ctr_internal_object_find_property(
              parent, key, CTR_CATEGORY_PRIVATE_PROPERTY);
        if (value) {
          //if (value != CtrStdNil) {
          //  t->type = CTR_AST_NODE_EMBED;
          //  t->modifier = 1;
          //  t->nodes = ctr_heap_allocate(sizeof(ctr_tlistitem));
          //  t->nodes->node = (ctr_tnode*) value;
          //} else
            ctr_internal_object_add_property(block, key, value, 0);
        }
        else if (!noerror)
          ctr_find_(key, noerror);
      }
      break;

    case CTR_AST_NODE_CODEBLOCK:
      ctr_capture_refs_(t, block, parent, 1); // capture all that we can
      break;

    case CTR_AST_NODE_EMBED: {
      if (t->modifier)
        break;
      ctr_object *p = ctr_cwlk_expr(t->nodes->node, "\0");
      if (CtrStdFlow && CtrStdFlow != CtrStdExit &&
          CtrStdFlow != CtrStdContinue && CtrStdFlow != CtrStdBreak) {
        ctr_heap_free(CtrStdFlow);
        CtrStdFlow = NULL;
      } else {
        t->nodes->node = (ctr_tnode *)p;
        t->modifier = 1;
      }
      break;
    }
    case CTR_AST_NODE_LTRNUM:
    case CTR_AST_NODE_PARAMLIST:
    case CTR_AST_NODE_ENDOFPROGRAM:
    case CTR_AST_NODE_LTRSTRING:
    case CTR_AST_NODE_LTRBOOLFALSE:
    case CTR_AST_NODE_LTRBOOLTRUE:
    case CTR_AST_NODE_LTRNIL:
    default:
      ctr_capture_refs_(t, block, parent, noerror);
    }
    if (!li->next)
      break;
    li = li->next;
    t = li->node;
  }
}

/**
 *[Block] unpack: [String:Ref], [Object:ctx]
 * Assign ref to block
 * (Always prefer using algebraic deconstruction assignments: look at section
 *'Assignment')
 */

ctr_object *ctr_block_assign(ctr_object *myself, ctr_argument *argumentList) {
  if (!ctr_reflect_check_bind_valid(myself, argumentList->object, 0)) {
    CtrStdFlow = ctr_build_string_from_cstring("Invalid bind");
    return CtrStdNil;
  }
  if (argumentList->object->interfaces->link != CtrStdSymbol) {
    if (!ctr_internal_object_is_equal(myself, argumentList->object)) {
      CtrStdFlow =
          ctr_build_string_from_cstring("Block cannot be constructed by ");
      ctr_string_append(CtrStdFlow, argumentList);
    }
    return myself;
  }
  if (argumentList->object->value.svalue->vlen == 0 ||
      (argumentList->object->value.svalue->vlen == 1 &&
       *argumentList->object->value.svalue->value == '_'))
    return myself;
  ctr_internal_object_add_property(
      argumentList->next->object ?: ctr_contexts[ctr_context_id],
      ctr_symbol_as_string(argumentList->object), myself, 0);
  return myself;
}

/**
 * [Block] specialize: [types...] with: [Block]
 *
 * Specialise a block for the given types with the given block
 */
ctr_overload_set *ctr_internal_next_bucket(ctr_overload_set *set,
                                           ctr_argument *arg);
ctr_object *ctr_block_specialise(ctr_object *myself,
                                 ctr_argument *argumentList) {
  ctr_object *types = argumentList->object;
  if (!types || types->info.type != CTR_OBJECT_TYPE_OTARRAY) {
    CtrStdFlow = ctr_build_string_from_cstring(
        "Invalid argument for specialize:(*)with:");
    return myself;
  }
  ctr_object *blk = argumentList->next->object;
  if (blk->info.overloaded) {
    CtrStdFlow =
        ctr_build_string_from_cstring("Refusing to merge specialisations");
    return myself;
  }
  if (!blk || (blk->info.type != CTR_OBJECT_TYPE_OTBLOCK &&
               blk->info.type != CTR_OBJECT_TYPE_OTNATFUNC)) {
    CtrStdFlow = ctr_build_string_from_cstring(
        "Invalid argument for specialize:with:(*)");
    return myself;
  }
  ctr_collection *tycoll = types->value.avalue;
  blk->info.overloaded = 1; // link the specialisations

  if (!myself->info.overloaded) {
    myself->info.overloaded = 1;
    myself->overloads = ctr_heap_allocate(sizeof(ctr_overload_set));
    myself->overloads->bucket_count = 0;
    myself->overloads->sub_buckets =
        ctr_heap_allocate(sizeof(ctr_overload_set *));
  }
  ctr_overload_set *overload = myself->overloads;
  ctr_argument arg;
  for (ctr_size i = tycoll->tail; i < tycoll->head; i++) {
    arg.object = tycoll->elements[i];
    ctr_overload_set *next = ctr_internal_next_bucket(overload, &arg);
    if (!next) {
      overload->sub_buckets =
          overload->bucket_count++
              ? ctr_heap_reallocate(overload->sub_buckets,
                                    sizeof(ctr_overload_set *) *
                                        overload->bucket_count)
              : ctr_heap_allocate(sizeof(ctr_overload_set *) *
                                  overload->bucket_count);
      next = (overload->sub_buckets[overload->bucket_count - 1] =
                  ctr_heap_allocate(sizeof(ctr_overload_set)));
      next->this_terminating_bucket = arg.object;
    }
    // if (i<tycoll->head-1)
    overload = next;
  }
  overload->this_terminating_value = blk;
  blk->overloads = myself->overloads;
  return myself;
}

/**
 *[Block] applyTo: [object]
 *
 * Runs a block of code using the specified object as a parameter.
 * If you run a block using the messages 'run' or 'applyTo:', me/my will
 * refer to the block itself instead of the containing object.
 *
 * the passed 'my' will be respected first,
 * and if lookup fails, it will be swapped for the block itself
 */

int ctr_args_eq(ctr_argument *arg0, ctr_argument *arg1) {
  if (!((arg0 == NULL) && (arg1 == NULL)))
    return 0;
  while (arg0 && arg1) {
    if ((arg0->object == NULL) ^ (arg1->object == NULL))
      return 0;
    if (!ctr_internal_object_is_equal(arg0->object, arg1->object))
      return 0;
    if ((arg0->next == NULL) ^ (arg1->next == NULL))
      return 0;
    arg0 = arg0->next;
    arg1 = arg1->next;
  }
  return 1;
}

ctr_object *ctr_block_run_array(ctr_object *myself, ctr_object *argArray,
                                ctr_object *my) {
  ctr_object *result;
  ctr_argument *argList = ctr_array_to_argument_list(argArray, NULL);
  if (myself->info.type == CTR_OBJECT_TYPE_OTNATFUNC) {
    ctr_object *result = myself->value.fvalue(my, argList);
    // ctr_deallocate_argument_list(arglistc);
    return result;
  }
  // overload begin
  if (myself->info.overloaded) {
    // find proper overload to run
    if (myself->overloads)
      myself = ctr_internal_find_overload(myself, argList);
  }
  // overload end

  int is_tail_call = 0, id;
  for (id = ctr_context_id;
       id > 0 && !is_tail_call && ctr_current_node_is_return;
       id--, is_tail_call = ctr_contexts[id] == myself)
    ;
  if (is_tail_call) {
#ifdef DEBUG_BUILD
    // printf ("tailcall at %p (%d from %d)\n", myself, id, ctr_context_id);
#endif
    // ctr_context_id = id;
  }
  ctr_tnode *node = myself->value.block;
  ctr_tlistitem *codeBlockParts = node->nodes;
  ctr_tnode *codeBlockPart1 = codeBlockParts->node;
  ctr_tnode *codeBlockPart2 = codeBlockParts->next->node;
  ctr_tlistitem *parameterList = codeBlockPart1->nodes;
  if (!is_tail_call) {
    if (myself->value.block->lexical && (!my || my == myself))
      ctr_contexts[++ctr_context_id] = myself;
    else
      ctr_open_context();
  }
  ctr_assign_block_parameters(parameterList, argList, my);
  if (my)
    ctr_assign_value_to_local_by_ref(
        &CTR_CLEX_KW_ME, my); /* me should always point to object, otherwise you
                                 have to store me in self and can't use in if */
  ctr_object *this_block = ctr_build_string("thisBlock", 9);
  ctr_assign_value_to_local(
      this_block, myself); /* otherwise running block may get gc'ed. */
  // ctr_heap_free(this_block);
  int p = myself->properties->size - 1;
  struct ctr_mapitem *head;
  head = myself->properties->head;
  while (p > -1) {
    if (!head)
      break;
    ctr_assign_value_to_my(head->key, head->value);
    head = head->next;
    p--;
  }
  // ctr_block_run_cache_set_ready_for_comp();
  result = ctr_cwlk_run(codeBlockPart2);
  if (result == NULL) {
    if (my)
      result = my;
    else
      result = myself;
  }
  if (CtrStdFlow != NULL && CtrStdFlow != CtrStdBreak &&
      CtrStdFlow != CtrStdContinue && CtrStdFlow != CtrStdExit) {
    ctr_object *catchBlock = ctr_internal_object_find_property(
        myself, ctr_build_string_from_cstring("catch"), 0);
    if (catchBlock != NULL) {
      ctr_object *catch_type = ctr_internal_object_find_property(
          catchBlock, ctr_build_string_from_cstring("%catch"), 0);
      ctr_argument aa = {0}, ab = {0}, *a = &aa;
      ctr_object *exdata = ctr_build_string_from_cstring(":exdata"),
                 *ex = CtrStdFlow,
                 *getexinfo = ctr_build_string_from_cstring("exceptionInfo");
      ctr_internal_object_set_property(ex, exdata, ctr_internal_ex_data(), 0);
      ctr_internal_create_func(ex, getexinfo, &ctr_exception_getinfo);
      int setstr = 0;
      if (ex->info.type == CTR_OBJECT_TYPE_OTSTRING) {
        ex->info.type = CTR_OBJECT_TYPE_OTOBJECT;
        setstr = 1;
        ctr_internal_create_func(ex, ctr_build_string_from_cstring("toString"),
                                 &ctr_string_to_string);
      }
      a->object = ex;
      a->next = &ab;
      a->next->object = catch_type;
      if (!catch_type ||
          ctr_reflect_is_linked_to(CtrStdReflect, a)->value.bvalue) {
        CtrStdFlow = NULL;
        a->object = ex;
        ctr_object *alternative = ctr_block_run_here(catchBlock, a, my);
        result = alternative;
      }
      ctr_internal_object_delete_property(ex, exdata, 0);
      ctr_internal_object_delete_property(ex, getexinfo, 1);
      if (setstr)
        ex->info.type = CTR_OBJECT_TYPE_OTSTRING;
    }
  }
  if (!is_tail_call)
    ctr_close_context();
  ctr_deallocate_argument_list(argList);
  // ctr_block_run_cache_result_if_expensive(myself, argList, result);
  return result;
}

extern int ctr_cwlk_replace_refs;
extern int ctr_cwlk_last_msg_level;
extern int ctr_cwlk_msg_level;
__attribute__((always_inline)) static inline void
ctr_assign_block_parameters(ctr_tlistitem *parameterList, ctr_argument *argList,
                            ctr_object *my) {
  ctr_tnode *parameter;
  ctr_object *a;
  int was_vararg, noskip = 0;
  if (parameterList && parameterList->node) {
    parameter = parameterList->node;
    if (likely(parameter->type != CTR_AST_NODE_NESTED))
      if (parameter->vlen == 4 && strncmp(parameter->value, "self", 4) == 0) {
        // assign self selectively, skip that parameter
        ctr_object *name = ctr_build_string(parameter->value, parameter->vlen);
        ctr_assign_value_to_local_by_ref(name, my);
        parameterList = parameterList->next;
      }
  }
  if (likely(parameterList && parameterList->node)) {
    parameter = parameterList->node;
    while (argList) {
      __asm__ __volatile__("");
      if (!parameter)
        goto noparam_;
      if (unlikely(parameter->type == CTR_AST_NODE_NESTED)) {
        int old_replace = ctr_cwlk_replace_refs;
        ctr_cwlk_replace_refs = 1;
        ctr_cwlk_last_msg_level = ctr_cwlk_msg_level;
        ctr_object *pattern = ctr_cwlk_expr(parameter, "");
        ctr_cwlk_replace_refs = old_replace; // set back in case we didn't reset
        ctr_send_message(
            argList->object, "unpack:", 7,
            &(ctr_argument){
                pattern, &(ctr_argument){ctr_contexts[ctr_context_id],
                                         NULL}}); // hand the block the context
      } else {
        was_vararg = (strncmp(parameter->value, "*", 1) == 0);
        if (!argList->object) {
          ctr_object *arr =
              was_vararg ? ctr_array_new(CtrStdArray, NULL) : CtrStdNil;
          ctr_object *name = ctr_build_string(parameter->value + was_vararg,
                                              parameter->vlen - was_vararg);
          ctr_assign_value_to_local(name, arr);
          // ctr_heap_free(name);
          if (!argList || !argList->next) {
            noskip = 1;
            break;
          }
          argList = argList->next;
          if (!parameterList->next)
            break;
          parameterList = parameterList->next;
          parameter = parameterList->node;
          continue;
        }
        if (parameterList->next) {
          a = argList->object;
          ctr_object *name =
              ctr_build_string(parameter->value, parameter->vlen);
          ctr_assign_value_to_local(name, a);
          // ctr_heap_free(name);
        } else if (!parameterList->next && was_vararg) {
          ctr_object *arr = ctr_array_new(CtrStdArray, NULL);
          ctr_argument arglist__;
          while (argList && argList->object) {
            arglist__.object = argList->object;
            ctr_array_push(arr, &arglist__);
            argList = argList->next;
          }
          ctr_object *name =
              ctr_build_string(parameter->value + 1, parameter->vlen - 1);
          ctr_assign_value_to_local(name, arr);
          // ctr_heap_free(name);
          // ctr_heap_free(arr);
        } else if (unlikely(!was_vararg)) {
          a = argList->object;
          ctr_object *name =
              ctr_build_string(parameter->value, parameter->vlen);
          ctr_assign_value_to_local(name, a);
          // ctr_heap_free(name);
        }
      }
    noparam_:;
      if (!argList || !argList->next)
        break;
      argList = argList->next;
      if (!parameterList->next)
        break;
      parameterList = parameterList->next;
      parameter = parameterList->node;
    }
    parameterList = parameterList->next;
    while (parameterList) {
      was_vararg = (strncmp(parameterList->node->value, "*", 1) == 0);
      ctr_object *name =
          ctr_build_string(parameterList->node->value + was_vararg,
                           parameterList->node->vlen - was_vararg);
      ctr_assign_value_to_local(
          name, was_vararg ? ctr_array_new(CtrStdArray, NULL) : CtrStdNil);
      // ctr_heap_free(name);
      if (!parameterList->next)
        break;
      parameterList = parameterList->next;
    }
  }
}

ctr_object *ctr_block_run(ctr_object *myself, ctr_argument *argList,
                          ctr_object *my) {
  if (!myself)
    return CtrStdNil;
  if (myself->info.type == CTR_OBJECT_TYPE_OTNATFUNC) {
    ctr_object *result = myself->value.fvalue(my, argList);
    return result;
  }
  // overload begin
  if (myself->info.overloaded) {
    // find proper overload to run
    if (myself->overloads)
      myself = ctr_internal_find_overload(myself, argList);
  }
  // overload end
  int is_tail_call = 0, id;
  if (myself->value.block && myself->value.block->lexical)
    for (id = ctr_context_id;
         id > 0 && !is_tail_call && ctr_current_node_is_return;
         id--, is_tail_call = ctr_contexts[id] == myself)
      ;
  if (is_tail_call) {
#ifdef DEBUG_BUILD
    // printf ("tailcall at %p (%d from %d)\n", myself, id, ctr_context_id);
#endif
    // ctr_context_id = id;
  }
  ctr_object *result;
  ctr_tnode *node = myself->value.block;
  ctr_tlistitem *codeBlockParts = node->nodes;
  ctr_tnode *codeBlockPart1 = codeBlockParts->node;
  ctr_tnode *codeBlockPart2 = codeBlockParts->next->node;
  ctr_tlistitem *parameterList = codeBlockPart1->nodes;
  if (!is_tail_call) {
    if (myself->value.block->lexical && (!my || my == myself))
      ctr_contexts[++ctr_context_id] = myself;
    else
      ctr_open_context();
  }
  ctr_assign_block_parameters(parameterList, argList, my);
  if (my)
    ctr_assign_value_to_local_by_ref(
        &CTR_CLEX_KW_ME, my); /* me should always point to object, otherwise you
                                 have to store me in self and can't use in if */
  ctr_object *this_block = ctr_build_string("thisBlock", 9);
  ctr_assign_value_to_local(
      this_block, myself); /* otherwise running block may get gc'ed. */
  // ctr_heap_free(this_block);
  int p = myself->properties->size - 1;
  struct ctr_mapitem *head;
  head = myself->properties->head;
  while (p > -1) {
    if (!head)
      break;
    ctr_assign_value_to_my(head->key, head->value);
    head = head->next;
    p--;
  }
  // ctr_block_run_cache_set_ready_for_comp();
  result = ctr_cwlk_run(codeBlockPart2);
  if (result == NULL) {
    if (my)
      result = my;
    else
      result = myself;
  }
  if (CtrStdFlow != NULL && CtrStdFlow != CtrStdBreak &&
      CtrStdFlow != CtrStdContinue && CtrStdFlow != CtrStdExit) {
    ctr_object *catchBlock = ctr_internal_object_find_property(
        myself, ctr_build_string_from_cstring("catch"), 0);
    if (catchBlock != NULL) {
      ctr_object *catch_type = ctr_internal_object_find_property(
          catchBlock, ctr_build_string_from_cstring("%catch"), 0);
      ctr_argument aa = {0}, ab = {0}, *a = &aa;
      ctr_object *exdata = ctr_build_string_from_cstring(":exdata"),
                 *ex = CtrStdFlow,
                 *getexinfo = ctr_build_string_from_cstring("exceptionInfo"),
                 *exd = ctr_internal_ex_data();
      ctr_internal_object_add_property(ex, exdata, exd, 0);
      ctr_internal_create_func(ex, getexinfo, &ctr_exception_getinfo);
      int setstr = 0;
      if (ex->info.type == CTR_OBJECT_TYPE_OTSTRING) {
        ex->info.type = CTR_OBJECT_TYPE_OTOBJECT;
        setstr = 1;
        ctr_internal_create_func(ex, ctr_build_string_from_cstring("toString"),
                                 &ctr_string_to_string);
      }
      a->object = ex;
      a->next = &ab;
      a->next->object = catch_type;
      if (!catch_type ||
          ctr_reflect_is_linked_to(CtrStdReflect, a)->value.bvalue) {
        CtrStdFlow = NULL;
        a->object = ex;
        ctr_object *alternative = ctr_block_run_here(catchBlock, a, my);
        result = alternative;
      }
      ctr_internal_object_delete_property(ex, exdata, 0);
      ctr_internal_object_delete_property(ex, getexinfo, 1);
      if (setstr)
        ex->info.type = CTR_OBJECT_TYPE_OTSTRING;
    }
  }
  if (!is_tail_call)
    ctr_close_context();

  // ctr_block_run_cache_result_if_expensive(myself, argList, result);
  return result;
}

/**
 * @internal
 *
 * Run a block in the current context
 */
ctr_object *ctr_block_run_here(ctr_object *myself, ctr_argument *argList,
                               ctr_object *my) {
  if (myself->info.type == CTR_OBJECT_TYPE_OTNATFUNC) {
    ctr_object *result = myself->value.fvalue(my, argList);
    return result;
  }
  // overload begin
  if (myself->info.overloaded) {
    // find proper overload to run
    if (myself->overloads)
      myself = ctr_internal_find_overload(myself, argList);
  }
  // overload end

  ctr_object *result;
  ctr_tnode *node = myself->value.block;
  ctr_tlistitem *codeBlockParts = node->nodes;
  ctr_tnode *codeBlockPart1 = codeBlockParts->node;
  ctr_tnode *codeBlockPart2 = codeBlockParts->next->node;
  ctr_tlistitem *parameterList = codeBlockPart1->nodes;
  // ctr_open_context();
  ctr_assign_block_parameters(parameterList, argList, my);
  if (my)
    ctr_assign_value_to_local_by_ref(
        &CTR_CLEX_KW_ME, my); /* me should always point to object, otherwise you
                                 have to store me in self and can't use in if */
  ctr_assign_value_to_local(
      &CTR_CLEX_KW_THIS, myself); /* otherwise running block may get gc'ed. */
  result = ctr_cwlk_run(codeBlockPart2);
  if (result == NULL) {
    if (my)
      result = my;
    else
      result = myself;
  }
  // ctr_close_context();
  if (CtrStdFlow != NULL && CtrStdFlow != CtrStdBreak &&
      CtrStdFlow != CtrStdContinue) {
    ctr_object *catchBlock = ctr_internal_object_find_property(
        myself, ctr_build_string_from_cstring("catch"), 0);
    if (catchBlock != NULL) {
      ctr_object *catch_type = ctr_internal_object_find_property(
          catchBlock, ctr_build_string_from_cstring("%catch"), 0);
      ctr_argument aa = {0}, ab = {0}, *a = &aa;
      ctr_object *exdata = ctr_build_string_from_cstring(":exdata"),
                 *ex = CtrStdFlow,
                 *getexinfo = ctr_build_string_from_cstring("exceptionInfo");
      ctr_internal_object_set_property(ex, exdata, ctr_internal_ex_data(), 0);
      ctr_internal_create_func(ex, getexinfo, &ctr_exception_getinfo);
      int setstr = 0;
      if (ex->info.type == CTR_OBJECT_TYPE_OTSTRING) {
        ex->info.type = CTR_OBJECT_TYPE_OTOBJECT;
        setstr = 1;
        ctr_internal_create_func(ex, ctr_build_string_from_cstring("toString"),
                                 &ctr_string_to_string);
      }
      a->object = ex;
      a->next = &ab;
      a->next->object = catch_type;
      if (!catch_type ||
          ctr_reflect_is_linked_to(CtrStdReflect, a)->value.bvalue) {
        CtrStdFlow = NULL;
        a->object = ex;
        ctr_object *alternative = ctr_block_run_here(catchBlock, a, my);
        result = alternative;
      }
      ctr_internal_object_delete_property(ex, exdata, 0);
      ctr_internal_object_delete_property(ex, getexinfo, 1);
      if (setstr)
        ex->info.type = CTR_OBJECT_TYPE_OTSTRING;
    }
  }
  return result;
}

/**
 *[Block] whileTrue: [block]
 *
 * Runs a block of code, depending on the outcome runs the other block
 * as long as the result of the first one equals boolean True.
 *
 * Usage:
 *
 * x := 0.
 * { ^(x < 6). } whileFalse:
 * { x add: 1. }. #increment x until it reaches 6.
 *
 * Here we increment variable x by one until it reaches 6.
 * While the number x is lower than 6 we keep incrementing it.
 * Don't forget to use the return ^ symbol in the first block.
 */
ctr_object *ctr_block_while_true(ctr_object *myself,
                                 ctr_argument *argumentList) {
  int sticky1, sticky2;
  sticky1 = myself->info.sticky;
  sticky2 = argumentList->object->info.sticky;
  myself->info.sticky = 1;
  argumentList->object->info.sticky = 1;
  ctr_object *my = myself, *block = my, *runblock = argumentList->object,
             *result, *my0;
  if (myself->value.block->lexical)
    ctr_contexts[++ctr_context_id] = myself;
  else
    ctr_open_context();
  ctr_tnode *node = myself->value.block;
  ctr_tlistitem *codeBlockParts = node->nodes,
                *cbparts1 = runblock->value.block->nodes;
  ctr_tnode *codeBlockPart1 = codeBlockParts->node, *cb1parts1 = cbparts1->node;
  ctr_tnode *codeBlockPart2 = codeBlockParts->next->node,
            *cb1parts2 = cbparts1->next->node;
  ctr_tlistitem *parameterList = codeBlockPart1->nodes, *paramList0;
  ctr_tnode *parameter;
  ctr_tnode *block0;
  int snd_stage = 0; // are we running the predicate, or the block
  while (1 && !CtrStdFlow) {
  resolve_value : {
    if (parameterList && parameterList->node) {
      parameter = parameterList->node;
      if (parameter->vlen == 4 && strncmp(parameter->value, "self", 4) == 0) {
        // assign self selectively, skip that parameter
        ctr_assign_value_to_local_by_ref(
            ctr_build_string(parameter->value, parameter->vlen), my);
        parameterList = parameterList->next;
      }
    }
    ctr_assign_value_to_local_by_ref(
        &CTR_CLEX_KW_ME, my); /* me should always point to object, otherwise you
                                 have to store me in self and can't use in if */
    ctr_assign_value_to_local(
        &CTR_CLEX_KW_THIS, block); /* otherwise running block may get gc'ed. */
    result = ctr_cwlk_run(codeBlockPart2);
    if (result == NULL)
      result = my;
    if (CtrStdFlow != NULL && CtrStdFlow != CtrStdBreak &&
        CtrStdFlow != CtrStdContinue) {
      ctr_object *catchBlock = ctr_internal_object_find_property(
          block, ctr_build_string_from_cstring("catch"), 0);
      if (catchBlock != NULL) {
        ctr_object *catch_type = ctr_internal_object_find_property(
            catchBlock, ctr_build_string_from_cstring("%catch"), 0);
        ctr_argument aa = {0}, ab = {0}, *a = &aa;
        ctr_object *exdata = ctr_build_string_from_cstring(":exdata"),
                   *ex = CtrStdFlow,
                   *getexinfo = ctr_build_string_from_cstring("exceptionInfo");
        ctr_internal_object_set_property(ex, exdata, ctr_internal_ex_data(), 0);
        ctr_internal_create_func(ex, getexinfo, &ctr_exception_getinfo);
        int setstr = 0;
        if (ex->info.type == CTR_OBJECT_TYPE_OTSTRING) {
          ex->info.type = CTR_OBJECT_TYPE_OTOBJECT;
          setstr = 1;
          ctr_internal_create_func(ex,
                                   ctr_build_string_from_cstring("toString"),
                                   &ctr_string_to_string);
        }
        a->object = ex;
        a->next = &ab;
        a->next->object = catch_type;
        if (!catch_type ||
            ctr_reflect_is_linked_to(CtrStdReflect, a)->value.bvalue) {
          CtrStdFlow = NULL;
          a->object = ex;
          ctr_object *alternative = ctr_block_run_here(catchBlock, a, my);
          result = alternative;
        }
        ctr_internal_object_delete_property(ex, exdata, 0);
        ctr_internal_object_delete_property(ex, getexinfo, 1);
        if (setstr)
          ex->info.type = CTR_OBJECT_TYPE_OTSTRING;
      }
    }
  }
    if (snd_stage)
      goto restore_stuff;
    result = ctr_internal_cast2bool(result);
    if (result->value.bvalue == 0 || CtrStdFlow)
      break;
    // save the stuff for the predicate block
    paramList0 = parameterList;
    block0 = codeBlockPart2;
    my0 = my;
    // subst for the stuff for the running block
    parameterList = cb1parts1->nodes;
    codeBlockPart2 = cb1parts2;
    my = runblock;
    snd_stage = 1;
    goto resolve_value;
  restore_stuff:;
    snd_stage = 0;
    parameterList = paramList0;
    codeBlockPart2 = block0;
    my = my0;
    if (CtrStdFlow == CtrStdContinue)
      CtrStdFlow = NULL; /* consume continue */
  }
  ctr_close_context();
  if (CtrStdFlow == CtrStdBreak)
    CtrStdFlow = NULL; /* consume break */
  myself->info.sticky = sticky1;
  argumentList->object->info.sticky = sticky2;
  return myself;
}

/**
 *[Block] whileFalse: [block]
 *
 * Runs a block of code, depending on the outcome runs the other block
 * as long as the result of the first one equals to False.
 *
 * Usage:
 *
 * x := 0.
 * { ^(x > 5). }
 * whileFalse: { x add: 1. }. #increment x until it reaches 6.
 *
 * Here we increment variable x by one until it reaches 6.
 * While the number x is not higher than 5 we keep incrementing it.
 * Don't forget to use the return ^ symbol in the first block.
 */
ctr_object *ctr_block_while_false(ctr_object *myself,
                                  ctr_argument *argumentList) {
  int sticky1, sticky2;
  sticky1 = myself->info.sticky;
  sticky2 = argumentList->object->info.sticky;
  myself->info.sticky = 1;
  argumentList->object->info.sticky = 1;
  ctr_object *my = myself, *block = my, *runblock = argumentList->object,
             *result, *my0;
  if (myself->value.block->lexical)
    ctr_contexts[++ctr_context_id] = myself;
  else
    ctr_open_context();
  ctr_tnode *node = myself->value.block;
  ctr_tlistitem *codeBlockParts = node->nodes,
                *cbparts1 = runblock->value.block->nodes;
  ctr_tnode *codeBlockPart1 = codeBlockParts->node, *cb1parts1 = cbparts1->node;
  ctr_tnode *codeBlockPart2 = codeBlockParts->next->node,
            *cb1parts2 = cbparts1->next->node;
  ctr_tlistitem *parameterList = codeBlockPart1->nodes, *paramList0;
  ctr_tnode *parameter;
  ctr_tnode *block0;
  int snd_stage = 0; // are we running the predicate, or the block
  while (1 && !CtrStdFlow) {
  resolve_value : {
    if (parameterList && parameterList->node) {
      parameter = parameterList->node;
      if (parameter->vlen == 4 && strncmp(parameter->value, "self", 4) == 0) {
        // assign self selectively, skip that parameter
        ctr_assign_value_to_local_by_ref(
            ctr_build_string(parameter->value, parameter->vlen), my);
        parameterList = parameterList->next;
      }
    }
    ctr_assign_value_to_local_by_ref(
        &CTR_CLEX_KW_ME, my); /* me should always point to object, otherwise you
                                 have to store me in self and can't use in if */
    ctr_assign_value_to_local(
        &CTR_CLEX_KW_THIS, block); /* otherwise running block may get gc'ed. */
    result = ctr_cwlk_run(codeBlockPart2);
    if (result == NULL)
      result = my;
    if (CtrStdFlow != NULL && CtrStdFlow != CtrStdBreak &&
        CtrStdFlow != CtrStdContinue) {
      ctr_object *catchBlock = ctr_internal_object_find_property(
          block, ctr_build_string_from_cstring("catch"), 0);
      if (catchBlock != NULL) {
        ctr_object *catch_type = ctr_internal_object_find_property(
            catchBlock, ctr_build_string_from_cstring("%catch"), 0);
        ctr_argument aa = {0}, ab = {0}, *a = &aa;
        ctr_object *exdata = ctr_build_string_from_cstring(":exdata"),
                   *ex = CtrStdFlow,
                   *getexinfo = ctr_build_string_from_cstring("exceptionInfo");
        ctr_internal_object_set_property(ex, exdata, ctr_internal_ex_data(), 0);
        ctr_internal_create_func(ex, getexinfo, &ctr_exception_getinfo);
        int setstr = 0;
        if (ex->info.type == CTR_OBJECT_TYPE_OTSTRING) {
          ex->info.type = CTR_OBJECT_TYPE_OTOBJECT;
          setstr = 1;
          ctr_internal_create_func(ex,
                                   ctr_build_string_from_cstring("toString"),
                                   &ctr_string_to_string);
        }
        a->object = ex;
        a->next = &ab;
        a->next->object = catch_type;
        if (!catch_type ||
            ctr_reflect_is_linked_to(CtrStdReflect, a)->value.bvalue) {
          CtrStdFlow = NULL;
          a->object = ex;
          ctr_object *alternative = ctr_block_run_here(catchBlock, a, my);
          result = alternative;
        }
        ctr_internal_object_delete_property(ex, exdata, 0);
        ctr_internal_object_delete_property(ex, getexinfo, 1);
        if (setstr)
          ex->info.type = CTR_OBJECT_TYPE_OTSTRING;
      }
    }
  }
    if (snd_stage)
      goto restore_stuff;
    result = ctr_internal_cast2bool(result);
    if (result->value.bvalue == 1 || CtrStdFlow)
      break;
    // save the stuff for the predicate block
    paramList0 = parameterList;
    block0 = codeBlockPart2;
    my0 = my;
    // subst for the stuff for the running block
    parameterList = cb1parts1->nodes;
    codeBlockPart2 = cb1parts2;
    my = runblock;
    snd_stage = 1;
    goto resolve_value;
  restore_stuff:;
    snd_stage = 0;
    parameterList = paramList0;
    codeBlockPart2 = block0;
    my = my0;
    if (CtrStdFlow == CtrStdContinue)
      CtrStdFlow = NULL; /* consume continue */
  }
  ctr_close_context();
  if (CtrStdFlow == CtrStdBreak)
    CtrStdFlow = NULL; /* consume break */
  myself->info.sticky = sticky1;
  argumentList->object->info.sticky = sticky2;
  return myself;
}

ctr_object *ctr_block_forever(ctr_object *myself, ctr_argument *argumentList) {
  (void) argumentList;

  ctr_tnode *cblock = myself->value.block;
  ctr_tlistitem *params = cblock->nodes->node->nodes;
  ctr_tnode *code = cblock->nodes->next->node;
  if (params && params->node && params->node->vlen == 4 &&
      strncmp(params->node->value, "self", 4) == 0) {
    // assign self selectively, skip that parameter
    ctr_object *name =
        ctr_build_string(params->node->value, params->node->vlen);
    ctr_assign_value_to_local_by_ref(name, myself);
    // ctr_heap_free(name);
  }
  ctr_object *catch = ctr_internal_object_find_property(
      myself, ctr_build_string_from_cstring("catch"), 0);
  ctr_argument agms;
  if (myself->value.block->lexical)
    ctr_contexts[++ctr_context_id] = myself;
  else
    ctr_open_context();
  while (1 && !CtrStdFlow) {
    ctr_assign_value_to_local_by_ref(
        &CTR_CLEX_KW_ME,
        myself); /* me should always point to object, otherwise you have to
                    store me in self and can't use in if */
    ctr_assign_value_to_local(
        &CTR_CLEX_KW_THIS, myself); /* otherwise running block may get gc'ed. */
    ctr_cwlk_run(code);
    if (CtrStdFlow == CtrStdBreak) {
      CtrStdFlow = NULL;
      break;
    }
    if (CtrStdFlow == CtrStdContinue) {
      CtrStdFlow = NULL;
      continue;
    }
    if (CtrStdFlow && catch) {
      agms.object = CtrStdFlow;
      CtrStdFlow = NULL;
      ctr_block_run_here(catch, &agms, catch);
    }
  }
  ctr_close_context();
  return myself;
}

/**
 *[Block] run
 *
 * Sending the unary message 'run' to a block will cause it to execute.
 * The run message takes no arguments, if you want to use the block as a
 *function and send arguments, consider using the applyTo-family of messages
 *instead. This message just simply runs the block of code without any
 *arguments.
 *
 * Usage:
 *
 * { Pen write: 'Hello World'. } run. #prints 'Hello World'
 *
 * The example above will run the code inside the block and display
 * the greeting.
 */
ctr_object *ctr_block_runIt(ctr_object *myself, ctr_argument *argumentList) {
  ctr_object *result;
  result = ctr_block_run(myself, argumentList,
                         myself); /* here me/my refers to block itself not
                                     object - this allows closures. */
  return result;
}

ctr_object *ctr_block_runall(ctr_object *myself, ctr_argument *argumentList) {
  ctr_object *result;
  result = ctr_block_run_array(myself, argumentList->object,
                               myself); /* here me/my refers to block itself not
                                           object - this allows closures. */
  if (CtrStdFlow == CtrStdBreak || CtrStdFlow == CtrStdContinue)
    CtrStdFlow = NULL; /* consume break */
  return result;
}

ctr_object *ctr_block_run_variadic_my(ctr_object *myself, ctr_object *my,
                                      int count, ...) {
  if (count < 1)
    return ctr_block_run(myself, NULL, my);
  ctr_argument argalloc[count]; // C99+
  memset(argalloc, 0, count * sizeof(ctr_argument));
  ctr_argument argv = {0}, *argumentList = &argv;
  ctr_argument *pass = argumentList;
  va_list ap;
  va_start(ap, count);
  for (size_t i = 0; i < (ctr_size) count; i++) {
    pass->object = va_arg(ap, ctr_object *);
    pass->next = argalloc + i;
    pass = pass->next;
  }
  va_end(ap);
  ctr_object *result = ctr_block_run(myself, argumentList, my);
  ctr_heap_free(argumentList);
  return result;
}

ctr_object *ctr_block_run_variadic(ctr_object *myself, int count, ...) {
  if (count < 1)
    return ctr_block_runIt(myself, NULL);
  ctr_argument argalloc[count]; // C99+
  memset(argalloc, 0, count * sizeof(ctr_argument));
  ctr_argument argv = {0}, *argumentList = &argv;
  ctr_argument *pass = argumentList;
  va_list ap;
  va_start(ap, count);
  for (size_t i = 0; i < (ctr_size) count; i++) {
    pass->object = va_arg(ap, ctr_object *);
    pass->next = argalloc + i;
    pass = pass->next;
  }
  va_end(ap);
  ctr_object *result = ctr_block_runIt(myself, argumentList);
  // ctr_heap_free(argumentList);
  return result;
}

ctr_object *ctr_send_message_variadic(ctr_object *myself, char *message,
                                      int msglen, int count, ...) {
  if (count < 1)
    return ctr_send_message(myself, message, msglen, NULL);
  ctr_argument argalloc[count]; // C99+
  memset(argalloc, 0, count * sizeof(ctr_argument));
  ctr_argument argv = {0}, *argumentList = &argv;
  ctr_argument *pass = argumentList;
  va_list ap;
  va_start(ap, count);
  for (size_t i = 0; i < (ctr_size) count; i++) {
    pass->object = va_arg(ap, ctr_object *);
    if (i + 1 != (ctr_size) count) {
      pass->next = argalloc + i;
      pass = pass->next;
    }
  }
  va_end(ap);
  ctr_object *result = ctr_send_message(myself, message, msglen, argumentList);
  // ctr_heap_free(argumentList);
  return result;
}

ctr_object *ctr_invoke_variadic(ctr_object *myself,
                                ctr_object *(*fun)(ctr_object *,
                                                   ctr_argument *),
                                int count, ...) {
  if (count < 1)
    return fun(myself, NULL);
  ctr_argument argalloc[count]; // C99+
  memset(argalloc, 0, count * sizeof(ctr_argument));
  ctr_argument argv = {0}, *argumentList = &argv;
  ctr_argument *pass = argumentList;
  va_list ap;
  va_start(ap, count);
  for (size_t i = 0; i < (ctr_size) count; i++) {
    pass->object = va_arg(ap, ctr_object *);
    if (i + 1 != (ctr_size) count) {
      pass->next = argalloc + i;
      pass = pass->next;
    }
  }
  va_end(ap);
  ctr_object *result = fun(myself, argumentList);
  // ctr_heap_free(argumentList);
  return result;
}

ctr_argument *ctr_allocate_argumentList(int count, ...) {
  if (count < 1)
    return NULL;
  ctr_argument *argumentList = ctr_heap_allocate(sizeof(ctr_argument));
  ctr_argument *pass = argumentList;
  va_list ap;
  va_start(ap, count);
  for (size_t i = 0; i < (ctr_size) count; i++) {
    pass->object = va_arg(ap, ctr_object *);
    if (i + 1 != (ctr_size) count) {
      pass->next = ctr_heap_allocate(sizeof(ctr_argument));
      pass = pass->next;
    }
  }
  va_end(ap);
  return argumentList;
}

void ctr_free_argumentList(ctr_argument *argumentList) {
  ctr_argument *pass;
  pass = argumentList;
  while (pass->next != NULL) {
    ctr_argument *prev = pass;
    pass = pass->next;
    if (prev != NULL)
      ctr_heap_free(prev);
  }
}

/**
 *[Block] set: [name] value: [object]
 *
 * Sets a variable in a block of code. This how you can get closure-like
 * functionality.
 *
 * Usage:
 *
 * shout := { Pen write: (my message + '!!!'). }.
 * shout set: 'message' value: 'hello'.
 * shout run.
 *
 * Here we assign a block to a variable named 'shout'.
 * We assign the string 'hello' to the variable 'message' inside the block.
 * When we invoke the block 'shout' by sending the run message without any
 * arguments it will display the string: 'hello!!!'.
 *
 * Similarly, you could use this technique to create a block that returns a
 * block that applies a formula (for instance simple multiplication) and then
 *set the multiplier to use in the formula. This way, you could create a block
 * building 'formula blocks'. This is how you implement & use closures
 * in Citron.
 *
 * There is no need to capture values with this message if you don't require the
 * name of the variable to be anything specific.
 * In such circumstances, you may use the automatic capture qualifier `const`
 * to capture the value directly where it is needed.
 */
ctr_object *ctr_block_set(ctr_object *myself, ctr_argument *argumentList) {
  ctr_object *key = ctr_internal_cast2string(argumentList->object);
  ctr_object *value = argumentList->next->object;
  ctr_internal_object_set_property(myself, key, value, 0);
  return myself;
}

/**
 *[Block] error: [object].
 *
 * Sets error flag on a block of code.
 * This will throw an error / exception.
 * You can attach an object to the error, for instance
 * an error message.
 *
 * Example:
 *
 * {
 *   thisBlock error: 'oops!'.
 * } catch: { :errorMessage
 *   Pen write: errorMessage.
 * }, run.
 */
ctr_object *ctr_block_error(ctr_object *myself, ctr_argument *argumentList) {
  CtrStdFlow = argumentList->object;
  CtrStdFlow->info.sticky = 1;
  return myself;
}

/**
 *[Block] catch: [otherBlock]
 *
 * Associates an error clause to a block.
 * If an error (exception) occurs within the block this block will be
 * executed, and its return substituted for the result of the expression
 *
 * Example:
 *
 * #Raise error on division by zero.
 * {
 *    var z := 4 / 0.
 * } catch: { :errorMessage
 *    Pen write: e, brk.
 * }, run.
 */
ctr_object *ctr_block_catch(ctr_object *myself, ctr_argument *argumentList) {
  ctr_object *catchBlock = argumentList->object;
  ctr_object *cname = ctr_build_string_from_cstring("catch");
  ctr_internal_object_delete_property(myself, cname, 0);
  ctr_internal_object_add_property(myself, cname, catchBlock, 0);
  // ctr_heap_free(cname);
  return myself;
}

/**
 *[Block] catch: [otherBlock] type: [Object]
 *
 * Associates an error clause to a block.
 * If the specified exception occurs within the block this block will be
 * executed, and its return substituted for the result of the expression
 *
 * Example:
 *
 * #Raise error on division by zero.
 * {
 *    var z := 4 / 0.
 * } catch: { :errorMessage
 *    Pen write: e, brk.
 * } type: String, run.
 */
ctr_object *ctr_block_catch_type(ctr_object *myself,
                                 ctr_argument *argumentList) {
  ctr_object *catchBlock = argumentList->object;
  ctr_object *cname = ctr_build_string_from_cstring("catch");
  ctr_object *pcname = ctr_build_string_from_cstring("%catch");
  ctr_internal_object_add_property(catchBlock, pcname,
                                   argumentList->next->object, 0);
  ctr_internal_object_set_property(myself, cname, catchBlock, 0);
  // ctr_heap_free(cname);
  // ctr_heap_free(pcname);
  return myself;
}

/**
 *[Block] toString
 *
 * Returns a string representation of the Block. This basic behavior, part
 * of any object will just return [Block]. Other objects typically override this
 * behavior with more useful implementations.
 */
ctr_object *ctr_block_to_string(ctr_object *myself,
                                ctr_argument *argumentList) {
  (void) argumentList;

  char *name = "";
  char *np;
  if (myself->lexical_name) {
    name = ctr_heap_allocate_cstring(myself->lexical_name);
    np = ctr_heap_allocate(sizeof(char) *
                           (myself->lexical_name->value.svalue->vlen + 9));
  } else
    np = ctr_heap_allocate(sizeof(char) * 9);
  sprintf(np, "[%s:Block]", name);
  if (name[0])
    ctr_heap_free(name);
  ctr_object *res = ctr_build_string_from_cstring(np);
  ctr_heap_free(np);
  return res;
}

int ctr_is_primitive(ctr_object *object) {
  return object == CtrStdWorld || object == CtrStdObject ||
         object == CtrStdBlock || object == CtrStdString ||
         object == CtrStdNumber || object == CtrStdBool ||
         object == CtrStdConsole || object == CtrStdNil || object == CtrStdGC ||
         object == CtrStdMap || object == CtrStdArray || object == CtrStdIter ||
         object == CtrStdFile || object == CtrStdSystem ||
         object == CtrStdDice || object == CtrStdCommand ||
         object == CtrStdSlurp || object == CtrStdShell ||
         object == CtrStdClock || object == CtrStdReflect ||
         object == CtrStdReflect_cons || object == CtrStdFiber ||
         object == CtrStdThread || object == CtrStdSymbol;
}

int strchru(char const *restrict str, int length, char c) {
  int i = 0;
  for (; i < length && str[i++] != c;)
    ;
  return i < length ? i - 1 : i;
}

// Stack-trace
ctr_object *ctr_get_stack_trace() {
  char trace[1024 * 1024]; // LOTS of bytes
  int line;
  char *currentProgram;
  ctr_source_map *mapItem;
  ctr_tnode *stackNode;

  for (int i = ctr_callstack_index; i > 0; i--) {
    sprintf(trace, "#%d ", i);
    stackNode = ctr_callstack[i - 1];
    snprintf(trace, stackNode->vlen, "%s", stackNode->value);
    mapItem = ctr_source_map_head;
    line = -1;
    while (mapItem) {
      if (line == -1 && mapItem->node == stackNode) {
        line = mapItem->line;
      }
      if (line > -1 && mapItem->node->type == CTR_AST_NODE_PROGRAM) {
        currentProgram = mapItem->node->value;
        sprintf(trace, " (%s: %d)", currentProgram, line + 1);
        break;
      }
      mapItem = mapItem->next;
    }
    sprintf(trace, "\n");
  }
  return ctr_build_string_from_cstring(trace);
}

extern int more_exception_data; /* flag: should we display more data for exceptions? */
void ctr_print_stack_trace() {
  int line;
  char *currentProgram;
  ctr_source_map *mapItem;
  ctr_tnode *stackNode;
  ctr_source_map *first = NULL, *lfirst = NULL;
  char *first_p = NULL, *lfirst_p = NULL;
  int first_ignore = 1;
  int is_eval = -1;
  for (int i = ctr_callstack_index; i > 0; i--) {
    printf("#%d ", i);
    stackNode = ctr_callstack[i - 1];
    fwrite(stackNode->value, sizeof(char), stackNode->vlen, stdout);
    if (!first_p) {
      first_p = ctr_heap_allocate(sizeof(char) * stackNode->vlen);
      memcpy(first_p, stackNode->value, stackNode->vlen);
    }
    mapItem = ctr_source_map_head;
    line = -1;
    while (mapItem) {
      if (line > -1 && mapItem->node->type == CTR_AST_NODE_PROGRAM) {
        currentProgram = mapItem->node->value;
        int ignored = 0;
        for (int j = 0; j < trace_ignore_count; j++) {
          char *s = ignore_in_trace[j];
          ctr_size l = strlen(s);
          if (l == mapItem->node->vlen &&
              strncmp(mapItem->node->value, s, l) == 0) {
            ignored = 1;
            break;
          }
        }
        if (ignored && first_ignore) {
          lfirst = first;
          lfirst_p = first_p;
          first = NULL;
          first_p = NULL;
        } else if (first_ignore) {
          first_ignore = 0;
          is_eval = strncmp(mapItem->node->value, "<eval>", 6) == 0
                        ? i
                        : -1; /*first=lfirst; first_p=lfirst_p; */
        }
        if (!more_exception_data) {
          char* fCurrentProgram = strrchr(currentProgram, '/');
          if (fCurrentProgram)
            currentProgram = fCurrentProgram + 1;
        }
        printf(" (%s: %d)%s", currentProgram, line + 1,
               ignored && more_exception_data ? CTR_ANSI_COLOR_CYAN " [Ignored]" CTR_ANSI_COLOR_RESET
                       : "");
        break;
      }
      if (line == -1 && mapItem->node == stackNode) {
        line = mapItem->line;
        if (!first)
          first = mapItem;
      }
      mapItem = mapItem->next;
    }
    printf("\n");
  }
  if (!first)
    return;
  char *ptr = first->p_ptr, *bptr = ptr, *here = ptr;
  if (!ptr)
    return;
  if (!more_exception_data)
    return;
  int p_tty = isatty(fileno(stdout));
  char *red = "", *reset = "", *magenta = "";
  if (p_tty) {
    red = CTR_ANSI_COLOR_RED;
    reset = CTR_ANSI_COLOR_RESET;
    magenta = CTR_ANSI_COLOR_MAGENTA;
  }
  printf("------------------------------------%s\n", red);
  printf("The probable cause of the exception: ");
  printf("%s", reset);
  int current_line = 0;
  int p = 2;
  if (is_eval > -1) {
    printf("%sEvaluated string (at index %d)%s\n", magenta, is_eval, reset);
    return;
  }
  putchar('\n');
  while (ptr >= first->s_ptr) {
    if (*ptr == '\n')
      p--;
    if (*ptr == 0 || p == 0)
      break;
    else
      ptr--;
  }
  current_line = first->line - 2 + p;
  p = 2;
  while (bptr < first->e_ptr && p >= 0) {
    if (*bptr == '\n')
      p--;
    if (*bptr == 0 || p == 0)
      break;
    else
      bptr++;
  }
  int pos_in_line = 0;
  int print_caret = 0;
  char c;
  static char numd[79];
  int slen = 0;
  int csl = strlen(first_p);
  while (ptr < bptr) {
    pos_in_line = 0;
    print_caret = -1;
    slen = sprintf(numd, "%d | ", ++current_line);
    printf("%s", numd);
    while ((c = *(ptr++)) != '\n') {
      if (ptr == here - csl + 1)
        printf("%s", magenta);
      putchar(c);
      if (ptr == here) {
        printf("%s", reset);
        print_caret = pos_in_line;
      }
      pos_in_line++;
    }
    putchar('\n');
    if (print_caret > -1) {
      printf("%s", magenta);
      for (int i = 0; i < print_caret + slen - 1; i++)
        putchar('~');
      putchar('^');
      puts(reset);
      print_caret = -1;
    }
  }
  putchar('\n');
}

ctr_object *ctr_get_last_trace_stringified(ctr_object *m_, ctr_argument *a_) {
  (void) m_;
  (void) a_;

  static char errorbufs[10240];
  char *errorbuf = errorbufs;

  int line;
  char *currentProgram;
  ctr_source_map *mapItem;
  ctr_tnode *stackNode;
  ctr_source_map *first = NULL, *lfirst = NULL;
  char *first_p = NULL, *lfirst_p = NULL;
  int first_ignore = 1;
  int is_eval = -1;
  for (int i = ctr_callstack_index; i > 0; i--) {
    errorbuf += sprintf(errorbuf, "#%d ", i);
    stackNode = ctr_callstack[i - 1];
    errorbuf += sprintf(errorbuf, "%.*s", (int) (sizeof(char) * stackNode->vlen),
                        stackNode->value);
    if (!first_p) {
      first_p = ctr_heap_allocate(sizeof(char) * stackNode->vlen);
      memcpy(first_p, stackNode->value, stackNode->vlen);
    }
    mapItem = ctr_source_map_head;
    line = -1;
    while (mapItem) {
      if (line > -1 && mapItem->node->type == CTR_AST_NODE_PROGRAM) {
        currentProgram = mapItem->node->value;
        int ignored = 0;
        for (int j = 0; j < trace_ignore_count; j++) {
          char *s = ignore_in_trace[j];
          ctr_size l = strlen(s);
          if (l == mapItem->node->vlen &&
              strncmp(mapItem->node->value, s, l) == 0) {
            ignored = 1;
            break;
          }
        }
        if (ignored && first_ignore) {
          lfirst = first;
          lfirst_p = first_p;
          first = NULL;
          first_p = NULL;
        } else if (first_ignore) {
          first_ignore = 0;
          is_eval = strncmp(mapItem->node->value, "<eval>", 6) == 0
                        ? i
                        : -1; /*first=lfirst; first_p=lfirst_p; */
        }
        errorbuf += sprintf(errorbuf, " (%s: %d)%s", currentProgram, line + 1,
                            ignored ? " [Ignored]" : "");
        break;
      }
      if (line == -1 && mapItem->node == stackNode) {
        line = mapItem->line;
        if (!first)
          first = mapItem;
      }
      mapItem = mapItem->next;
    }
    errorbuf += sprintf(errorbuf, "\n");
  }
  return ctr_build_string_from_cstring(errorbufs);
}

/** [exception in catch block] exceptionInfo
 *
 * Returns line info about an exception
 */
ctr_object *ctr_exception_getinfo(ctr_object *myself,
                                  ctr_argument *argumentList) {
  (void) argumentList;

  return ctr_internal_object_find_property(
      myself, ctr_build_string_from_cstring(":exdata"), 0);
}

ctr_object *ctr_internal_ex_data() {
  int lineno = -1, pos = -1;
  ctr_source_map *mapItem;
  ctr_tnode *stackNode = ctr_callstack[ctr_callstack_index - 1];
  mapItem = ctr_source_map_head;
  while (mapItem) {
    if (lineno == -1 && mapItem->node == stackNode) {
      lineno = mapItem->line;
      int firstsec = strchru(stackNode->value, stackNode->vlen, ':');
      pos = mapItem->p_ptr - mapItem->s_ptr - firstsec;
      break;
    }
    mapItem = mapItem->next;
  }

  ctr_object *linenumber = ctr_build_number_from_float(lineno),
             *apos = ctr_build_number_from_float(pos);

  ctr_argument arg;
  arg.object = linenumber; // line
  arg.next = NULL;
  ctr_object *data = ctr_array_new(CtrStdArray, NULL);
  ctr_array_push(data, &arg);
  arg.object = apos;
  ctr_array_push(data, &arg);
  return data;
}

ctr_object *ctr_get_last_trace(ctr_object *myself, ctr_argument *argumentList) {
  (void) myself;
  (void) argumentList;

  int line;
  char *currentProgram;
  ctr_source_map *mapItem;
  ctr_tnode *stackNode;
  stackNode = ctr_callstack[ctr_callstack_index - 2];
  mapItem = ctr_source_map_head;
  line = -1;
  ctr_object *ret = ctr_array_new(CtrStdArray, NULL);
  ctr_argument argv = {0}, *arg = &argv;
  for (int i = ctr_callstack_index; i > 0; i--) {
    stackNode = ctr_callstack[i - 1];
    mapItem = ctr_source_map_head;
    line = -1;
    while (mapItem) {
      if (line == -1 && mapItem->node == stackNode) {
        line = mapItem->line;
      }
      if (line > -1 && mapItem->node->type == CTR_AST_NODE_PROGRAM) {
        currentProgram = mapItem->node->value;
        arg->object = ctr_build_number_from_float(line + 1);
        ctr_array_push(ret, arg);
        arg->object = ctr_build_string(currentProgram, mapItem->node->vlen);
        ctr_array_push(ret, arg);
        break;
      }
      mapItem = mapItem->next;
    }
  }
  return ret;
}
