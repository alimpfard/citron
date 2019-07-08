#include <ctype.h>
#include <errno.h>
#include <math.h>
#include <sys/types.h>
#include <signal.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#define CTR_GLOBALS_DEFINE
#include "citron.h"
#undef CTR_GLOBALS_DEFINE

#include "siphash.h"

static int ctr_world_initialized = 0;
extern ctr_object *generator_end_marker;

// #define withGIL 1 //we all know this is a bad idea...

#ifdef withGIL
#include <pthread.h>
static pthread_mutex_t ctr_message_mutex = {{PTHREAD_MUTEX_RECURSIVE}};

#define CTR_THREAD_UNLOCK() pthread_mutex_unlock(&ctr_message_mutex)
#else
#define CTR_THREAD_LOCK()
#define CTR_THREAD_UNLOCK()
#endif

#ifdef withBoehmGC
#define GC_THREAD
#include <gc/gc.h>
#endif
#ifdef withBoehmGC_P
#define ctr_heap_allocate_typed_(s, t) ctr_heap_allocate_typed(s, t)
#else
#define ctr_heap_allocate_typed_(s, t) ctr_heap_allocate(s)
#endif


#include "promise.h"

void ctr_load_required_native_modules();

static const int all_signals[] = {
#ifdef SIGHUP
    SIGHUP,
#endif
#ifdef SIGINT
    SIGINT,
#endif
#ifdef SIGQUIT
    SIGQUIT,
#endif
#ifdef SIGILL
    SIGILL,
#endif
#ifdef SIGABRT
    SIGABRT,
#endif
#ifdef SIGFPE
    SIGFPE,
#endif
#ifdef SIGSEGV
    SIGSEGV,
#endif
#ifdef SIGPIPE
    SIGPIPE,
#endif
#ifdef SIGALRM
    SIGALRM,
#endif
#ifdef SIGTERM
    SIGTERM,
#endif
#ifdef SIGUSR1
    SIGUSR1,
#endif
#ifdef SIGUSR2
    SIGUSR2,
#endif
#ifdef SIGCHLD
    SIGCHLD,
#endif
#ifdef SIGCONT
    SIGCONT,
#endif
#ifdef SIGSTP
    SIGSTP,
#endif
#ifdef SIGTTIN
    SIGTTIN,
#endif
#ifdef SIGTTOU
    SIGTTOU,
#endif
#ifdef SIGBUS
    SIGBUS,
#endif
#ifdef SIGPOLL
    SIGPOLL,
#endif
#ifdef SIGPROF
    SIGPROF,
#endif
#ifdef SIGSYS
    SIGSYS,
#endif
#ifdef SIGTRAP
    SIGTRAP,
#endif
#ifdef SIGURG
    SIGURG,
#endif
#ifdef SIGVTALRM
    SIGVTALRM,
#endif
#ifdef SIGXCPU
    SIGXCPU,
#endif
#ifdef SIGXFSZ
    SIGXFSZ,
#endif
#ifdef SIGEMT
    SIGEMT,
#endif
#ifdef SIGSTKFLT
    SIGSTKFLT,
#endif
#ifdef SIGIO
    SIGIO,
#endif
#ifdef SIGPWR
    SIGPWR,
#endif
#ifdef SIGLOST
    SIGLOST,
#endif
#ifdef SIGWINCH
    SIGWINCH,
#endif
    SIGKILL, // last one. must exist
};

static ctr_object* ctr_gc_dump(ctr_object* myself, ctr_argument* argumentList) {
  GC_dump();
  return myself;
}

static void register_signal_handlers() {
  struct sigaction act;
  int i = 0;
  memset(&act, 0, sizeof act);
  act.sa_handler = ctr_int_handler;
  act.sa_flags = SA_NOCLDSTOP;
  do {
    if (sigaction(all_signals[i], &act, NULL))
      fprintf(stderr, "Could not install signal %d handler: %s (Ignoring)\n",
              all_signals[i], strerror(errno));
  } while (all_signals[++i] != SIGKILL);
}

/**
 * @internal
 *
 * ReadFile
 *
 * Reads in an entire file.
 */
char *ctr_internal_readf(char *file_name, uint64_t *total_size) {
  char *prg;
  char ch;
  int prev;
  uint64_t size;
  uint64_t real_size;
  FILE *fp;
  fp = fopen(file_name, "r");
  if (fp == NULL) {
    printf("Error while opening the file (%s).\n", file_name);
    exit(1);
  }
  prev = ftell(fp);
  fseek(fp, 0L, SEEK_END);
  size = ftell(fp);
  fseek(fp, prev, SEEK_SET);
  real_size = (size + 4) * sizeof(char);
  prg = ctr_heap_allocate(
      real_size); /* add 4 bytes, 3 for optional closing sequence verbatim mode
                     and one lucky byte! */
  ctr_program_length = 0;
  while ((ch = fgetc(fp)) != EOF)
    prg[ctr_program_length++] = ch;
  fclose(fp);
  *total_size = (uint64_t)real_size;
  return prg;
}

/**
 * @internal
 *
 * ReadFile
 *
 * Reads in an entire file.
 */
char *ctr_internal_readfp(FILE *fp, uint64_t *total_size) {
  char *prg;
  char ch;
  ctr_size real_size = 0, size = 1024;
  ctr_program_length = 0;
  prg = ctr_heap_allocate(size * sizeof(char));
  while ((ch = fgetc(fp)) != EOF) {
    if (size - ctr_program_length < 128) {
      size *= 2;
      prg = ctr_heap_reallocate(prg, size);
    }
    real_size++;
    prg[ctr_program_length++] = ch;
  }
  *total_size = (uint64_t)real_size;
  return prg;
}

/**
 * @internal
 *
 * InternalObjectIsEqual
 *
 * Detemines whether two objects are identical.
 */
int ctr_internal_object_is_equal(ctr_object *object1, ctr_object *object2) {
  if (object1 == object2)
    return 1;
  char *string1;
  char *string2;
  ctr_size len1;
  ctr_size len2;
  ctr_size d;
  if (object1->info.type == CTR_OBJECT_TYPE_OTSTRING &&
      object2->info.type == CTR_OBJECT_TYPE_OTSTRING) {
    string1 = object1->value.svalue->value;
    string2 = object2->value.svalue->value;
    len1 = object1->value.svalue->vlen;
    len2 = object2->value.svalue->vlen;
    if (len1 != len2)
      return 0;
    d = memcmp(string1, string2, len1);
    if (d == 0)
      return 1;
    return 0;
  }
  if (object1->info.type == CTR_OBJECT_TYPE_OTNUMBER &&
      object2->info.type == CTR_OBJECT_TYPE_OTNUMBER) {
    ctr_number num1 = object1->value.nvalue;
    ctr_number num2 = object2->value.nvalue;
    if (num1 == num2)
      return 1;
    return 0;
  }
  if (object1->info.type == CTR_OBJECT_TYPE_OTBOOL &&
      object2->info.type == CTR_OBJECT_TYPE_OTBOOL) {
    int b1 = object1->value.bvalue;
    int b2 = object2->value.bvalue;
    if (b1 == b2)
      return 1;
    return 0;
  }
  if (object1->info.type == CTR_OBJECT_TYPE_OTARRAY &&
      object2->info.type == CTR_OBJECT_TYPE_OTARRAY) {
    int count = ctr_array_count(object1, NULL)->value.nvalue;
    if (ctr_array_count(object2, NULL)->value.nvalue != count)
      return 0;
    int i = 1;
    ctr_argument *args = ctr_heap_allocate(sizeof(ctr_argument));
    for (; count > 0 && i; count--) {
      args->object = ctr_build_number_from_float(count - 1);
      i = i && ctr_internal_object_is_equal(ctr_array_get(object1, args),
                                            ctr_array_get(object2, args));
    }
    ctr_heap_free(args);
    return i;
  }
  if (object1->info.type == CTR_OBJECT_TYPE_OTOBJECT &&
      object2->info.type == CTR_OBJECT_TYPE_OTOBJECT &&
      ctr_reflect_get_primitive_link(object1) == CtrStdMap &&
      ctr_reflect_get_primitive_link(object2) == CtrStdMap) {
    ctr_argument *args = ctr_heap_allocate(sizeof(ctr_argument));
    args->object = object1;
    ctr_object *o1t = ctr_reflect_describe_value(CtrStdReflect, args);
    args->object = object2;
    ctr_object *o2t = ctr_reflect_describe_value(CtrStdReflect, args);
    ctr_heap_free(args);
    return ctr_internal_object_is_equal(o1t, o2t);
  }
  if (ctr_internal_has_responder(object1, ctr_build_string_from_cstring("=")))
    if (ctr_internal_has_responder(object2,
                                   ctr_build_string_from_cstring("="))) {
      int a = ctr_internal_cast2bool(
                  ctr_send_message_variadic(object1, "=", 1, 1, object2))
                  ->value.bvalue &&
              ctr_internal_cast2bool(
                  ctr_send_message_variadic(object2, "=", 1, 1, object1))
                  ->value.bvalue;
      return a;
    }
  return 0;
}

/**
 * @internal
 *
 * InternalObjectIsConstructible
 *
 * Detemines whether an object can be constructed (partly or fully) from another
 */
int ctr_internal_object_is_constructible(ctr_object *object1,
                                         ctr_object *object2) {
  return ctr_internal_object_is_constructible_(object1, object2, 0);
}

ctr_object *ctr_object_is_constructible(ctr_object *myself,
                                        ctr_argument *argumentList) {
  ctr_object *sl0 = myself, *sl1 = argumentList->object;
  if (sl1->properties->size > sl0->properties->size)
    return ctr_build_bool(0);

  ctr_mapitem *sp1 = sl1->properties->head;
  for (int i = 0; i < sl1->properties->size; i++) {
    if (ctr_internal_object_find_property(
            sl0, sp1->key, CTR_CATEGORY_PRIVATE_PROPERTY) == NULL)
      return ctr_build_bool(0);
    sp1 = sp1->next;
  }
  return ctr_build_bool(1);
}

int ctr_internal_object_is_constructible_(ctr_object *object1,
                                          ctr_object *object2, int raw) {
  // char* s = ctr_heap_allocate_cstring(ctr_format_str("+unification of %S ->
  // %S", ctr_internal_cast2string(object1),
  // ctr_internal_cast2string(object2))); printf("%s\n", s);
  if (object1 == object2)
    return 1;
  char *string1;
  char *string2;
  ctr_size len1;
  ctr_size len2;
  // ignore
  if (object2->interfaces->link == CtrStdSymbol) // it's a binding
  {
    // printf("Accepting binding\n");
    return 1;
  }
  // both are strings
  if (object1->info.type == CTR_OBJECT_TYPE_OTSTRING &&
      object2->info.type == CTR_OBJECT_TYPE_OTSTRING) {
    string1 = object1->value.svalue->value;
    string2 = object2->value.svalue->value;
    len1 = object1->value.svalue->vlen;
    len2 = object2->value.svalue->vlen;
    if (len1 != len2)
      return 0;
    return memcmp(string1, string2, len1) == 0;
  }
  // both are numbers
  if (object1->info.type == CTR_OBJECT_TYPE_OTNUMBER &&
      object2->info.type == CTR_OBJECT_TYPE_OTNUMBER) {
    ctr_number num1 = object1->value.nvalue;
    ctr_number num2 = object2->value.nvalue;
    return num1 == num2;
  }
  // both are bools
  if (object1->info.type == CTR_OBJECT_TYPE_OTBOOL &&
      object2->info.type == CTR_OBJECT_TYPE_OTBOOL) {
    int b1 = object1->value.bvalue;
    int b2 = object2->value.bvalue;
    return (b1 == b2);
  }
  // both are arrays
  if (object1->info.type == CTR_OBJECT_TYPE_OTARRAY &&
      object2->info.type == CTR_OBJECT_TYPE_OTARRAY) {
    int count = ctr_array_count(object2, NULL)->value.nvalue;
    int count1 = ctr_array_count(object1, NULL)->value.nvalue;
    int catch_all = 0;
    char *ignores = ctr_heap_allocate(sizeof(char) * count);
    int ignore = 0;
    ctr_object *f;
    for (int _i = 0; _i < count; _i++) {
      f = object2->value.avalue->elements[_i];
      if (f->interfaces->link == CtrStdSymbol) {
        // we have a binding on our hands
        if (f->value.svalue->vlen > 1 && *f->value.svalue->value == '*')
          catch_all++;
        if (f->value.svalue->vlen == 1 && *f->value.svalue->value == '_') {
          ignores[_i] = 1;
          ignore++;
        } else
          ignores[_i] = 0;
      }
    }
    if (ctr_array_count(object1, NULL)->value.nvalue != count && catch_all == 0)
      return 0; // It requires more/less parameters than object1 can provide,
                // and we don't have a catch-all binding
    int i = 1;
    int _x = 0;
    int _y = 0;
    int _i = count;
    ctr_argument *args = ctr_heap_allocate(sizeof(ctr_argument));
    ctr_argument *elnu1 = ctr_heap_allocate(sizeof(ctr_argument));
    ctr_argument *elnu2 = ctr_heap_allocate(sizeof(ctr_argument));
    // char* s = ctr_heap_allocate_cstring(ctr_format_str("-UNIFY %S <-> %S",
    // ctr_internal_cast2string(object1), ctr_internal_cast2string(object2)));
    // printf("%s\n", s);
    for (; count > 0 && _x < _i && i; count--, _x++, _y++) {
      // printf("%d -- %d (%d -- %d)\n", _x, _i, _y, count1);
      if (_y > count1) // the rest goes into the catch_all
      {
        _y = 0;
        catch_all = -2;
      }
      elnu1->object = ctr_build_number_from_float(_y);
      while (ignores[_x])
        _x--;
      elnu2->object = ctr_build_number_from_float(_x);
      args->object = ctr_array_get(object2, elnu2);
      i = i &&
          (catch_all == -2 || args->object->interfaces->link == CtrStdSymbol ||
           ctr_internal_object_is_constructible_(ctr_array_get(object1, elnu1),
                                                 args->object, raw));
    }
    ctr_heap_free(args);
    // printf("array unification -> %d\n", i);
    return i;
  }
  if (object1->info.type == CTR_OBJECT_TYPE_OTOBJECT &&
      object2->info.type == CTR_OBJECT_TYPE_OTOBJECT &&
      ctr_reflect_get_primitive_link(object1) == CtrStdMap &&
      ctr_reflect_get_primitive_link(object2) == CtrStdMap) {
    if (object2->properties->size == 0)
      return 1;
    if (object2->properties->size > object1->properties->size)
      return 0;
    ctr_mapitem *mI = object2->properties->head;
    ctr_argument arg;
    while (mI) {
      arg.object = mI->key;
      ctr_object *tI = ctr_map_get(object1, &arg);
      if (!tI)
        return 0;
      if (!ctr_internal_object_is_constructible_(tI, mI->value, raw))
        return 0;
      mI = mI->next;
    }
    return 1;
  }
  ctr_argument *args = ctr_heap_allocate(sizeof(ctr_argument));
  args->object = object2;
  ctr_object *ret =
      (ctr_send_message_blocking(object1, "isConstructible:", 16, args));
  ctr_heap_free(args);
  return (ret->info.type == CTR_OBJECT_TYPE_OTBOOL) ? ret->value.bvalue : 0;
}

/**
 * @internal
 *
 * InternalObjectIndexHash
 *
 * Given an object, this function will calculate a hash to speed-up
 * lookup.
 */
uint64_t ctr_internal_index_hash(ctr_object *key) {
  ctr_object *stringKey = ctr_internal_cast2string(key);
  return siphash24(stringKey->value.svalue->value,
                   stringKey->value.svalue->vlen, CtrHashKey);
}

// /**
//  * @internal
//  *
//  * InternalObjectAltHash
//  *
//  * Given an object, this function will calculate a hash to speed-up
//  * lookup.
//  */
//// uint64_t ctr_internal_alt_hash(ctr_object * key)
// {
//      ctr_object *stringKey = ctr_internal_cast2string(key);
//      uint32_t out;
//      MurmurHash3_x86_32(stringKey->value.svalue->value,
//                       stringKey->value.svalue->vlen, *(uint32_t*)CtrHashKey,
//                       &out);
//      return out;
// }

/**
 * @internal
 *
 * InternalObjectFindProperty
 *
 * Finds property in object.
 */
ctr_object *ctr_internal_object_find_property(ctr_object *owner,
                                              ctr_object *key, int is_method) {
  return ctr_internal_object_find_property_with_hash(
      owner, key, ctr_internal_index_hash(key), is_method);
}

/**
 * @internal
 *
 * InternalObjectFindPropertyOrCreate
 *
 * Finds property in object, or adds a placeholder one.
 */
ctr_object *ctr_internal_object_find_property_or_create(ctr_object *owner,
                                                        ctr_object *key,
                                                        int is_method) {
  return ctr_internal_object_find_property_or_create_with_hash(
      owner, key, ctr_internal_index_hash(key), is_method);
}

ctr_object *ctr_internal_object_find_property_ignore(ctr_object *owner,
                                                     ctr_object *key,
                                                     int is_method,
                                                     int ignore) {
  return ctr_internal_object_find_property(owner, key, is_method);
}

ctr_object *ctr_internal_object_find_property_with_hash(ctr_object *owner,
                                                        ctr_object *key,
                                                        uint64_t hashKey,
                                                        int is_method) {
  // TODO: Use a binary tree-map
  ctr_mapitem *head, *first_head;
  ctr_map *lookup;
  if (is_method) {
    if (!owner->methods)
      return NULL;
    lookup = owner->methods;
  } else
    lookup = owner->properties;
  if (unlikely(!lookup))
      return NULL;
  if (unlikely(lookup->size == 1 &&
               (head = lookup->head)->hashKey == hashKey)) {
    if (likely(ctr_internal_object_is_equal(head->key, key))) {
      head->hits++;
      return head->value;
    }
    return NULL;
  }

  head = lookup->head;
  while (head) {
    if ((hashKey == head->hashKey) &&
        ctr_internal_object_is_equal(head->key, key)) {
      ctr_object *val = head->value;
      first_head = head->prev;
      if (!first_head || first_head == head)
        return val;
      if (++head->hits > first_head->hits) {
        int fh = first_head->hits;
        void *fk = first_head->key, *fv = first_head->value;
        uint64_t fhk = first_head->hashKey;
        first_head->hits = head->hits;
        first_head->key = head->key;
        first_head->value = val;
        first_head->hashKey = hashKey;
        head->hits = fh;
        head->key = fk;
        head->value = fv;
        head->hashKey = fhk;
      }
      return val;
    }
    head->hits = 0;
    head = head->next;
  }
  return NULL;
}

ctr_object *ctr_internal_object_find_property_or_create_with_hash(
    ctr_object *owner, ctr_object *key, uint64_t hashKey, int is_method) {
  ctr_mapitem *head, *first_head, *last_head;
  ctr_map *lookup;
  if (is_method) {
    if (!owner->methods)
      return NULL;
    lookup = owner->methods;
  } else
    lookup = owner->properties;
  if (unlikely(!lookup))
      return NULL;
  if (unlikely(lookup->size == 0)) {
    ctr_object *repl = ctr_internal_create_object(CTR_OBJECT_TYPE_OTNIL);
    ctr_set_link_all(repl, CtrStdNil);
    ctr_internal_object_add_property_with_hash(owner, key, hashKey, repl,
                                               is_method);
    return repl;
  }
  if (unlikely(lookup->size == 1 &&
               (head = lookup->head)->hashKey == hashKey)) {
    if (likely(ctr_internal_object_is_equal(head->key, key))) {
      head->hits++;
      return head->value;
    }
    lookup->size++;
    ctr_mapitem *new_item = ctr_heap_allocate(sizeof(ctr_mapitem));
    new_item->key = key;
    new_item->hashKey = hashKey;
    ctr_object *repl = ctr_internal_create_object(CTR_OBJECT_TYPE_OTNIL);
    ctr_set_link_all(repl, CtrStdNil);
    new_item->value = repl;
    new_item->next = NULL;
    lookup->head->next = new_item;
    new_item->prev = lookup->head;
    return new_item->value;
  }

  head = lookup->head;
  while (head) {
    if ((hashKey == head->hashKey) &&
        ctr_internal_object_is_equal(head->key, key)) {
      ctr_object *val = head->value;
      first_head = head->prev;
      if (!first_head || first_head == head)
        return val;
      if (++head->hits > first_head->hits) {
        int fh = first_head->hits;
        void *fk = first_head->key, *fv = first_head->value;
        uint64_t fhk = first_head->hashKey;
        first_head->hits = head->hits;
        first_head->key = head->key;
        first_head->value = val;
        first_head->hashKey = hashKey;
        head->hits = fh;
        head->key = fk;
        head->value = fv;
        head->hashKey = fhk;
      }
      return val;
    }
    head->hits = 0;
    last_head = head;
    head = head->next;
  }
  lookup->size++;
  ctr_mapitem *new_item = ctr_heap_allocate(sizeof(ctr_mapitem));
  new_item->key = key;
  new_item->hashKey = hashKey;
  ctr_object *repl = ctr_internal_create_object(CTR_OBJECT_TYPE_OTNIL);
  ctr_set_link_all(repl, CtrStdNil);
  new_item->value = repl;
  new_item->next = NULL;
  last_head->next = new_item;
  new_item->prev = last_head;
  return new_item->value;
}

/**
 * @internal
 *
 * InternalObjectDeleteProperty
 *
 * Deletes the specified property from the object.
 * NOTE: Does not update the bloom filter TODO: Try to fix
 */
void ctr_internal_object_delete_property(ctr_object *owner, ctr_object *key,
                                         int is_method) {
  ctr_internal_object_delete_property_with_hash(
      owner, key, ctr_internal_index_hash(key), is_method);
}

/**
 * @internal
 *
 * InternalObjectDeletePropertyWithHash
 *
 * Deletes the specified property from the object given a hash value.
 */
void ctr_internal_object_delete_property_with_hash(ctr_object *owner,
                                                   ctr_object *key,
                                                   uint64_t hashKey,
                                                   int is_method) {
  ctr_did_side_effect = 1;
  ctr_mapitem *head;
  if (is_method) {
    if (!owner->methods || owner->methods->size == 0) {
      return;
    }
    // if((owner->methods->s_hash&hashKey) != hashKey)
    // return;
    head = owner->methods->head;
  } else {
    if (!owner->properties)
        return;
    if (owner->properties->size == 0) {
      return;
    }
    // if((owner->properties->s_hash&hashKey) != hashKey)
    // return;
    head = owner->properties->head;
  }
  while (head) {
    if ((hashKey == head->hashKey) &&
        ctr_internal_object_is_equal(head->key, key)) {
      if (head->next && head->prev) {
        head->next->prev = head->prev;
        head->prev->next = head->next;
      } else {
        if (head->next) {
          head->next->prev = NULL;
        }
        if (head->prev) {
          head->prev->next = NULL;
        }
      }
      if (is_method) {
        if (owner->methods->head == head) {
          if (head->next) {
            owner->methods->head = head->next;
          } else {
            owner->methods->head = NULL;
          }
        }
        owner->methods->size--;
      } else {
        if (owner->properties->head == head) {
          if (head->next) {
            owner->properties->head = head->next;
          } else {
            owner->properties->head = NULL;
          }
        }
        owner->properties->size--;
      }
      ctr_heap_free(head);
      return;
    }
    head = head->next;
  }
  return;
}

/**
 * @internal
 *
 * InternalObjectAddProperty
 *
 * Adds a property to an object.
 */
void ctr_internal_object_add_property(ctr_object *owner, ctr_object *key,
                                      ctr_object *value, int m) {
  return ctr_internal_object_add_property_with_hash(
      owner, key, ctr_internal_index_hash(key), value, m);
}

/**
 * @internal
 *
 * InternalObjectAddPropertyWithHash
 *
 * Adds a property to an object.
 */
void ctr_internal_object_add_property_with_hash(ctr_object *owner,
                                                ctr_object *key,
                                                uint64_t hashKey,
                                                ctr_object *value, int m) {
  ctr_did_side_effect = 1;
  ctr_mapitem *new_item = ctr_heap_allocate(sizeof(ctr_mapitem));
  ctr_mapitem *current_head = NULL;
  new_item->key = key;
  new_item->hashKey = hashKey;
  //              uint32_t ahash = ctr_internal_alt_hash(key);
  new_item->value = value;
  new_item->next = NULL;
  new_item->prev = NULL;
  if (m) {
    if (!owner->methods || owner->methods->size == 0) {
      owner->methods->head = new_item;
    } else {
      current_head = owner->methods->head;
      current_head->prev = new_item;
      new_item->next = current_head;
      owner->methods->head = new_item;
    }
    // owner->methods->s_hash = owner->methods->s_hash | new_item->hashKey;
    // owner->methods->m_hash |= ahash;
    owner->methods->size++;
  } else {
    if (owner->properties->size == 0) {
      owner->properties->head = new_item;
    } else {
      current_head = owner->properties->head;
      current_head->prev = new_item;
      new_item->next = current_head;
      owner->properties->head = new_item;
    }
    // owner->properties->s_hash |= new_item->hashKey;
    // owner->properties->m_hash |= ahash;
    owner->properties->size++;
  }
}

/**
 * @internal
 *
 * InternalObjectSetProperty
 *
 * Sets a property on an object
 */
void ctr_internal_object_set_property(ctr_object *owner, ctr_object *key,
                                      ctr_object *value, int is_method) {
  ctr_internal_object_delete_property(owner, key, is_method);
  ctr_internal_object_add_property(owner, key, value, is_method);
}

/**
 * @internal
 *
 * InternalObjectSetPropertyWithHash
 *
 * Sets a property on an object.
 */
void ctr_internal_object_set_property_with_hash(ctr_object *owner,
                                                ctr_object *key,
                                                uint64_t hashKey,
                                                ctr_object *value,
                                                int is_method) {
  ctr_internal_object_delete_property_with_hash(owner, key, hashKey, is_method);
  ctr_internal_object_add_property_with_hash(owner, key, hashKey, value,
                                             is_method);
}

/**
 * @internal
 *
 * InternalMemMem
 *
 * memmem implementation because this not available on every system.
 */
char *ctr_internal_memmem(char *haystack, long hlen, char *needle, long nlen,
                          int reverse) {
  char *cur;
  char *last;
  char *begin;
  int step = (1 - reverse * 2); /* 1 if reverse = 0, -1 if reverse = 1 */
  if (nlen == 0)
    return NULL;
  if (hlen == 0)
    return NULL;
  if (hlen < nlen)
    return NULL;
  if (!reverse) {
    begin = haystack;
    last = haystack + hlen - nlen + 1;
  } else {
    begin = haystack + hlen - nlen;
    last = haystack - 1;
  }
  for (cur = begin; cur != last; cur += step) {
    if (memcmp(cur, needle, nlen) == 0)
      return cur;
  }
  return NULL;
}

/**
 * @internal
 *
 * InternalObjectCreate
 *
 * Creates an object.
 */
// __attribute__ ((always_inline))
ctr_object *ctr_internal_create_object(int type) {
  return ctr_internal_create_mapped_object(type, 0);
}

ctr_object *ctr_internal_create_mapped_object_shared(int type);
ctr_object *ctr_internal_create_mapped_object_unshared(int type);

// __attribute__ ((always_inline))
ctr_object *ctr_internal_create_mapped_object(int type, int shared) {
  if (shared)
    return ctr_internal_create_mapped_object_shared(type);
  else
    return ctr_internal_create_mapped_object_unshared(type);
}

#ifndef withBoehmGC
void ctr_finalize_clear(GC_PTR obj, GC_PTR user_data) {
  ctr_object *o = obj;
  if (o->release_hook)
    o->release_hook(o->value.rvalue);
}
#endif
// __attribute__ ((always_inline))
ctr_object *ctr_internal_create_mapped_object_shared(int type) {
  ctr_object *o;
  o = ctr_heap_allocate_shared(sizeof(ctr_object));
  o->properties = ctr_heap_allocate_shared(sizeof(ctr_map));
  o->methods = ctr_heap_allocate_shared(sizeof(ctr_map));
  o->properties->size = 0;
  o->methods->size = 0;
  o->properties->head = NULL;
  o->methods->head = NULL;
  o->release_hook = NULL;
  o->info.type = type;
  o->info.asyncMode = 0;
  o->info.mark = 0;
  o->info.remote = 0;
  o->info.shared = 1;
  o->info.raw = 0;
  o->interfaces = ctr_heap_allocate_shared(sizeof(ctr_interfaces));
  if (type == CTR_OBJECT_TYPE_OTBOOL)
    o->value.bvalue = 0;
  if (type == CTR_OBJECT_TYPE_OTNUMBER)
    o->value.nvalue = 0;
  if (type == CTR_OBJECT_TYPE_OTSTRING) {
    o->value.svalue = ctr_heap_allocate_shared(sizeof(ctr_string));
    o->value.svalue->value = "";
    o->value.svalue->vlen = 0;
  }
#ifndef withBoehmGC
  o->gnext = NULL;
  if (ctr_first_object == NULL) {
    ctr_first_object = o;
  } else {
    o->gnext = ctr_first_object;
    ctr_first_object = o;
  }
#endif
  return o;
}

// __attribute__ ((always_inline))
ctr_object *ctr_internal_create_mapped_object_unshared(int type) {
  ctr_object *o;
  o = ctr_heap_allocate_typed_(sizeof(*o), type);
  o->properties = ctr_heap_allocate(sizeof(ctr_map));
  o->methods = ctr_heap_allocate(sizeof(ctr_map));
  o->properties->size = 0;
  // o->properties->s_hash = 0;
  // o->properties->m_hash = 0;
  o->methods->size = 0;
  // o->methods->s_hash = 0;
  // o->methods->m_hash = 0;
  o->properties->head = NULL;
  o->methods->head = NULL;
  o->release_hook = NULL;
  o->info.type = type;
  o->info.asyncMode = 0;
  o->info.mark = 0;
  o->info.remote = 0;
  o->info.shared = 0;
  o->info.raw = 0;
  o->interfaces = ctr_heap_allocate(sizeof(ctr_interfaces));
  if (type == CTR_OBJECT_TYPE_OTBOOL)
    o->value.bvalue = 0;
  if (type == CTR_OBJECT_TYPE_OTNUMBER)
    o->value.nvalue = 0;
  if (type == CTR_OBJECT_TYPE_OTSTRING) {
    o->value.svalue = ctr_heap_allocate(sizeof(ctr_string));
    o->value.svalue->value = "";
    o->value.svalue->vlen = 0;
  }
#ifndef withBoehmGC
  o->gnext = NULL;
  if (ctr_first_object == NULL) {
    ctr_first_object = o;
  } else {
    o->gnext = ctr_first_object;
    ctr_first_object = o;
  }
#endif
  return o;
}

/**
 * @internal
 *
 * InternalStandaloneObjectCreate
 *
 * Creates an object that is not connected to the rest of the objects.
 * the caller must clean this object up.
 */
ctr_object *ctr_internal_create_standalone_object(int type) {
  return ctr_internal_create_mapped_standalone_object(type, 0);
}

__attribute__((always_inline)) void
ctr_transfer_object_ownership(ctr_object *to, ctr_object *what) {
  char nbuf[1024];
  int len = sprintf(nbuf, "property%lu", ctr_internal_index_hash(what));
  char *name = ctr_heap_allocate(sizeof(char) * len + 1);
  memcpy(name, nbuf, len + 1);
  ctr_internal_object_add_property(to, ctr_build_string_from_cstring(name),
                                   what, 0);
}

// __attribute__ ((always_inline))
ctr_object *ctr_internal_create_mapped_standalone_object(int type, int shared) {
  ctr_object *o;
  o = shared == 1 ? ctr_heap_allocate_shared(sizeof(ctr_object))
                  : ctr_heap_allocate_typed_(sizeof(ctr_object), type);
  o->properties = shared == 1 ? ctr_heap_allocate_shared(sizeof(ctr_map))
                              : ctr_heap_allocate(sizeof(ctr_map));
  o->methods = shared == 1 ? ctr_heap_allocate_shared(sizeof(ctr_map))
                           : ctr_heap_allocate(sizeof(ctr_map));
  o->properties->size = 0;
  o->methods->size = 0;
  o->properties->head = NULL;
  o->methods->head = NULL;
  o->release_hook = NULL;
  o->info.type = type;
  o->info.asyncMode = 0;
  o->info.mark = 0;
  o->info.remote = 0;
  o->info.shared = shared;
  o->info.raw = 0;
  o->interfaces = shared == 1 ? ctr_heap_allocate_shared(sizeof(ctr_interfaces))
                              : ctr_heap_allocate(sizeof(ctr_interfaces));
  if (type == CTR_OBJECT_TYPE_OTBOOL)
    o->value.bvalue = 0;
  if (type == CTR_OBJECT_TYPE_OTNUMBER)
    o->value.nvalue = 0;
  if (type == CTR_OBJECT_TYPE_OTSTRING) {
    o->value.svalue = shared == 1 ? ctr_heap_allocate_shared(sizeof(ctr_string))
                                  : ctr_heap_allocate(sizeof(ctr_string));
    o->value.svalue->value = "";
    o->value.svalue->vlen = 0;
  }
#ifndef withBoehmGC
  o->gnext = NULL;
#endif
  return o;
}

/**
 * @internal
 *
 * InternalStandaloneObjectDelete
 *
 * Deletes an object that was previously made with
 * InternalStandaloneObjectCreate
 */
void ctr_internal_delete_standalone_object(ctr_object *o) {
  void (*free_heap_maybe_shared)(void *) =
      o->info.shared ? &ctr_heap_free_shared : &ctr_heap_free;
  free_heap_maybe_shared(o->properties);
  free_heap_maybe_shared(o->methods);
  free_heap_maybe_shared(o);
}

/**
 * @internal
 *
 * InternalFunctionCreate
 *
 * Create a function and add this to the object as a method.
 */
void ctr_internal_create_func(ctr_object *o, ctr_object *key,
                              ctr_object *(*func)(ctr_object *,
                                                  ctr_argument *)) {
  ctr_object *methodObject =
      ctr_internal_create_object(CTR_OBJECT_TYPE_OTNATFUNC);
  ctr_set_link_all(methodObject, CtrStdBlock);
  methodObject->value.fvalue = func;
  ctr_internal_object_add_property(o, key, methodObject, 1);
}

/**
 * @internal
 *
 * InternalNumberCast
 *
 * Casts an object to a number object.
 */
ctr_object *ctr_internal_cast2number(ctr_object *o) {
  if (o->info.type == CTR_OBJECT_TYPE_OTNUMBER)
    return o;
  ctr_argument *a = ctr_heap_allocate(sizeof(ctr_argument));
  a->object = CtrStdNil;
  ctr_object *numObject = ctr_send_message_blocking(o, "toNumber", 8, a);
  ctr_heap_free(a);
  if (CtrStdFlow) {
    ctr_object *err = CtrStdFlow;
    CtrStdFlow = NULL;
    CtrStdFlow =
        ctr_format_str("ECast of type %$ to type Number failed: %$",
                       ctr_send_message_blocking(o, "type", 4, NULL), err);
    return ctr_build_number_from_float((ctr_number)0);
  } else if (numObject->info.type != CTR_OBJECT_TYPE_OTNUMBER) {
    CtrStdFlow = ctr_format_str(
        "ENo implicit cast arises from the use of type %$ to type Number",
        ctr_send_message_blocking(numObject, "type", 4, NULL));
    return ctr_build_number_from_float((ctr_number)0);
  }
  return numObject;
}

/**
 * @internal
 *
 * InternalStringCast
 *
 * Casts an object to a string object.
 */
ctr_object *ctr_internal_cast2string(ctr_object *o) {
  if (!o)
    return ctr_build_nil();
  if (o->info.type == CTR_OBJECT_TYPE_OTSTRING)
    return o;
  ctr_argument *a = ctr_heap_allocate(sizeof(ctr_argument));
  a->object = CtrStdNil;
  ctr_object *stringObject;
  // printf("%d, %s\n", o->info.type,
  // o->lexical_name?o->lexical_name->value.svalue->value:"No-lexical-name");
  switch (o->info.type) {
  case CTR_OBJECT_TYPE_OTARRAY:
    stringObject = ctr_array_to_string(o, NULL);
    break;
  case CTR_OBJECT_TYPE_OTBOOL:
    stringObject = ctr_bool_to_string(o, NULL);
    break;
  case CTR_OBJECT_TYPE_OTNUMBER:
    stringObject = ctr_number_to_string(o, NULL);
    break;
  case CTR_OBJECT_TYPE_OTOBJECT:
    stringObject = ctr_reflect_get_primitive_link(o) == CtrStdMap
                       ? ctr_map_to_string(o, NULL)
                       : ctr_send_message_blocking(o, "toString", 8, a);
    break;
  default:
    stringObject = ctr_send_message_blocking(o, "toString", 8, a);
    break;
  }
  ctr_heap_free(a);
  if (stringObject->info.type == CTR_OBJECT_TYPE_OTNIL)
    return ctr_build_string_from_cstring("(null)");
  if (stringObject->info.type != CTR_OBJECT_TYPE_OTSTRING) {
    printf("wanted 3, got %d from %d\n", stringObject->info.type, o->info.type);
    CtrStdFlow =
        ctr_build_string_from_cstring("toString must return a string.");
    ctr_print_stack_trace();
#ifdef DEBUG_BUILD
    sttrace_print(NULL);
#endif
    return ctr_build_string_from_cstring("?");
  }
  return stringObject;
}

/**
 * @internal
 *
 * InternalBooleanCast
 *
 * Casts an object to a boolean.
 */
ctr_object *ctr_internal_cast2bool(ctr_object *o) {
  if (o->info.type == CTR_OBJECT_TYPE_OTBOOL)
    return o;
  ctr_argument *a = ctr_heap_allocate(sizeof(ctr_argument));
  a->object = CtrStdNil;
  ctr_object *boolObject = ctr_send_message_blocking(o, "toBoolean", 9, a);
  ctr_heap_free(a);
  if (boolObject->info.type != CTR_OBJECT_TYPE_OTBOOL) {
    // CtrStdFlow =
    //     ctr_build_string_from_cstring
    //     ("toBoolean must return a boolean.");
    return ctr_build_bool(0);
  }
  return boolObject;
}

/**
 * @internal
 *
 * ContextSwitch
 *
 * Switches to an existing context.
 */
void ctr_switch_context(ctr_object *context) {
  if (ctr_context_id >= CTR_CONTEXT_VECTOR_DEPTH) {
    CtrStdFlow = ctr_build_string_from_cstring("Too many nested calls.");
    CtrStdFlow->info.sticky = 1;
  }
  context->info.sticky = 1;
  ctr_contexts[++ctr_context_id] = context;
}

/**
 * @internal
 *
 * ContextDump
 *
 * Dumps a context to a given struct
 *
 * @param ctx
 *     Pointer to the structure that will be filled
 */
void ctr_dump_context(struct ctr_context_t *ctx) {
  for (int index = 0; index < CTR_CONTEXT_VECTOR_DEPTH; index++)
    ctx->contexts[index] = ctr_contexts[index];
  ctx->id = ctr_context_id;
}

/**
 *
 * ContextLoad
 *
 * Load a whole context system
 * @param ctx
 *     The structure that is loaded
 */
void ctr_load_context(struct ctr_context_t ctx) {
  for (int index = 0; index < CTR_CONTEXT_VECTOR_DEPTH; index++)
    ctr_contexts[index] = ctx.contexts[index];
  ctr_context_id = ctx.id;
}

/**
 * @internal
 *
 * ContextOpen
 *
 * Opens a new context to keep track of variables.
 */
void ctr_open_context() {
  ctr_object *context;
  if (ctr_context_id >= CTR_CONTEXT_VECTOR_DEPTH) {
    CtrStdFlow = ctr_build_string_from_cstring("Too many nested calls.");
    CtrStdFlow->info.sticky = 1;
  }
  context = ctr_internal_create_object(CTR_OBJECT_TYPE_OTOBJECT);
  context->info.sticky = 1;
  ctr_contexts[++ctr_context_id] = context;
}

/**
 * @internal
 *
 * ContextClose
 *
 * Closes a context.
 */
void ctr_close_context() {
  // ctr_contexts[ctr_context_id]->properties->head = NULL;
  // ctr_contexts[ctr_context_id]->properties->size = 0;
  ctr_contexts[ctr_context_id]->info.sticky = 0;
  if (ctr_context_id == 0)
    return;
  ctr_context_id--;
}

/**
 * @internal
 *
 * CTRFind
 *
 * Tries to locate a variable in the current context or one
 * of the contexts beneath.
 */
ctr_object *ctr_find_(ctr_object *key, int noerror) {
  int i = ctr_context_id;
  ctr_object *foundObject = NULL;
  if (CtrStdFlow)
    return CtrStdNil;
  while ((i > -1 && foundObject == NULL)) {
    ctr_object *context = ctr_contexts[i];
    foundObject = ctr_internal_object_find_property(context, key, 0);
    i--;
  }
  if (!foundObject && (key == &CTR_CLEX_KW_ME))
    return NULL;
  if (foundObject)
    return foundObject;
  ctr_internal_plugin_find(key);
  foundObject = ctr_internal_object_find_property(CtrStdWorld, key, 0);
  if (!foundObject) {
    ctr_object *ctx = ctr_find_(&CTR_CLEX_KW_ME, 1);
    if (ctx) {
      ctr_object *msg = ctr_internal_object_find_property(ctx, key, 1);
      if (msg) {
        switch (msg->info.type) {
        case CTR_OBJECT_TYPE_OTNATFUNC:
          foundObject = (msg->value.fvalue)(ctx, NULL);
          break;
        case CTR_OBJECT_TYPE_OTBLOCK:
          foundObject = ctr_block_run(msg, NULL, ctx);
          break;
        }
      } else {
        ctr_object *msg =
            ctr_internal_object_find_property(ctx, &CTR_CLEX_KW_RESPONDTO, 1);
        if (msg) {
          static ctr_argument argument = {NULL};
          argument.object = key;
          switch (msg->info.type) {
          case CTR_OBJECT_TYPE_OTNATFUNC:
            foundObject = (msg->value.fvalue)(ctx, &argument);
            break;
          case CTR_OBJECT_TYPE_OTBLOCK:
            foundObject = ctr_block_run(msg, &argument, ctx);
            break;
          }
        }
      }
    }
  }
  if (foundObject == NULL) {
    if (noerror)
      return NULL;
    char *key_name;
    char *message;
    char *full_message;
    int message_size;
    message = "Key not found: ";
    message_size = ((strlen(message)) + key->value.svalue->vlen);
    full_message = ctr_heap_allocate(message_size * sizeof(char));
    key_name = ctr_heap_allocate_cstring(key);
    memcpy(full_message, message, strlen(message));
    memcpy(full_message + strlen(message), key_name, key->value.svalue->vlen);
    CtrStdFlow = ctr_build_string(full_message, message_size);
    CtrStdFlow->info.sticky = 1;
    ctr_heap_free(full_message);
    ctr_heap_free(key_name);
    return CtrStdNil;
  }
  return foundObject;
}

ctr_object *ctr_find(ctr_object *key) { return ctr_find_(key, 0); }

/**
 * @internal
 *
 * CTRFindInMy
 *
 * Tries to locate a property of an object.
 */
ctr_object *ctr_find_in_my(ctr_object *key) {
  ctr_object *context = ctr_find(&CTR_CLEX_KW_ME);
  ctr_object *foundObject = ctr_internal_object_find_property(context, key, 0);
  if (CtrStdFlow)
    return CtrStdNil;
  if (foundObject == NULL) {
    char *key_name;
    char *message;
    char *full_message;
    int message_size;
    message = "Object property not found: ";
    message_size = ((strlen(message)) + key->value.svalue->vlen);
    full_message = ctr_heap_allocate(message_size * sizeof(char));
    key_name = ctr_heap_allocate_cstring(key);
    memcpy(full_message, message, strlen(message));
    memcpy(full_message + strlen(message), key_name, key->value.svalue->vlen);
    CtrStdFlow = ctr_build_string(full_message, message_size);
    CtrStdFlow->info.sticky = 1;
    ctr_heap_free(full_message);
    ctr_heap_free(key_name);
    return CtrStdNil;
  }
  return foundObject;
}

/**
 * @internal
 *
 * CTRSetBasic
 *
 * Sets a proeprty in an object (context).
 */
void ctr_set(ctr_object *key, ctr_object *object) {
  int i = ctr_context_id;
  ctr_object *context;
  ctr_object *foundObject = NULL;
  if (key->value.svalue->value[0] == '&') { // we're setting a double-reference
    key->value.svalue->value++;
    key->value.svalue->vlen--;
    foundObject = ctr_find(key);
    if (foundObject)
      *foundObject = *object;
    key->value.svalue->value--;
    key->value.svalue->vlen++;
    return;
  }
  if (ctr_contexts[ctr_context_id] == ctr_world_ptr) {
    ctr_internal_object_set_property(ctr_contexts[ctr_context_id], key, object,
                                     0);
    return;
  }
  while ((i > -1 && foundObject == NULL)) {
    context = ctr_contexts[i];
    foundObject = ctr_internal_object_find_property(context, key, 0);
    if (foundObject)
      break;
    i--;
  }
  if (!foundObject) {
    if (ctr_context_id <= 1)
      goto assign_anyway;
    char *key_name;
    char *message;
    char *full_message;
    int message_size;
    message = "Cannot assign to undefined variable: ";
    message_size = ((strlen(message)) + key->value.svalue->vlen);
    full_message = ctr_heap_allocate(message_size * sizeof(char));
    key_name = ctr_heap_allocate_cstring(key);
    memcpy(full_message, message, strlen(message));
    memcpy(full_message + strlen(message), key_name, key->value.svalue->vlen);
    CtrStdFlow = ctr_build_string(full_message, message_size);
    CtrStdFlow->info.sticky = 1;
    ctr_heap_free(full_message);
    ctr_heap_free(key_name);
    return;
  }
assign_anyway:
  if (object != generator_end_marker &&
      strncmp(key->value.svalue->value, "me", key->value.svalue->vlen) != 0)
    object->lexical_name = key;
  ctr_internal_object_set_property(context, key, object, 0);
}

ctr_object *ctr_give_version(ctr_object *myself, ctr_argument *argumentList) {
  return ctr_build_string_from_cstring(CTR_VERSION);
}

ctr_object *ctr_object_destruct(ctr_object *object, ctr_argument *nothing) {
  return CtrStdNil;
}

ctr_object *ctr_get_frame(ctr_object *myself, ctr_argument *argumentList) {
  ctr_object *c = ctr_contexts[ctr_context_id];
  ctr_object *map = ctr_internal_create_object(CTR_OBJECT_TYPE_OTOBJECT);
  ctr_set_link_all(map, CtrStdMap);
  map->value = c->value;
  map->properties = c->properties;
  return map;
}

ctr_object *ctr_get_frame_id(ctr_object *myself, ctr_argument *argumentList) {
  return ctr_build_number_from_float(ctr_context_id);
}

ctr_object *ctr_get_frame_with_id(ctr_object *myself,
                                  ctr_argument *argumentList) {
  int id = argumentList->object->value.nvalue;
  if (id < 0 || id > ctr_context_id) {
    CtrStdFlow =
        ctr_format_str("ENo frame with id %d: IDs must be in range(0, %d)", id,
                       ctr_context_id);
    return CtrStdNil;
  }
  ctr_object *c = ctr_contexts[id];
  ctr_object *map = ctr_internal_create_object(CTR_OBJECT_TYPE_OTOBJECT);
  ctr_set_link_all(map, CtrStdMap);
  map->value = c->value;
  map->properties = c->properties;
  return map;
}

ctr_object *ctr_frame_present(ctr_object *myself, ctr_argument *argumentList) {
  int p = 0;
  ctr_object *frame = argumentList->object;
  for (int i = ctr_context_id; i >= 0 && !p;
       i--, p = p || ctr_contexts[i] == frame)
    ;
  return ctr_build_bool(p);
}

void ctr_initialize_world_minimal() {
  if (ctr_world_initialized)
    return;

  volatile ctr_thread_workaround_double_list_t* tw = ctr_heap_allocate_tracked(sizeof(*tw));
  tw->next = NULL;
  tw->prev = ctr_thread_workaround_double_list;
  tw->context = ctr_contexts;
  ctr_thread_workaround_double_list = (ctr_thread_workaround_double_list_t*) tw;

  trace_ignore_count = 0;
  ctr_world_initialized = 1;
  // register_signal_handlers ();
  ctr_instrument = 0;
  ctr_global_instrum = NULL;
  int i;
  srand((unsigned)time(NULL));
  for (i = 0; i < 16; i++) {
    CtrHashKey[i] = (rand() % 255);
  }
  ctr_first_object = NULL;
  //----//
  CTR_CLEX_KW_ME_SV.value = "me";
  CTR_CLEX_KW_ME_SV.vlen = 2;
  //----//
  CTR_CLEX_KW_ME.info.type = CTR_OBJECT_TYPE_OTSTRING;
  CTR_CLEX_KW_ME.info.mark = 0;
  CTR_CLEX_KW_ME.info.sticky = 1;
  CTR_CLEX_KW_ME.info.chainMode = 0;
  CTR_CLEX_KW_ME.info.remote = 0;
  CTR_CLEX_KW_ME.info.shared = 0;
  CTR_CLEX_KW_ME.info.raw = 0;
  CTR_CLEX_KW_ME.value.svalue = &CTR_CLEX_KW_ME_SV;
  CTR_CLEX_KW_ME.interfaces = ctr_heap_allocate(sizeof(ctr_interfaces));
  //----//
  CTR_CLEX_KW_THIS_SV.value = "thisBlock";
  CTR_CLEX_KW_THIS_SV.vlen = 9;
  //----//
  CTR_CLEX_KW_THIS.info.type = CTR_OBJECT_TYPE_OTSTRING;
  CTR_CLEX_KW_THIS.info.mark = 0;
  CTR_CLEX_KW_THIS.info.sticky = 1;
  CTR_CLEX_KW_THIS.info.chainMode = 0;
  CTR_CLEX_KW_THIS.info.remote = 0;
  CTR_CLEX_KW_THIS.info.shared = 0;
  CTR_CLEX_KW_THIS.info.raw = 0;
  CTR_CLEX_KW_THIS.value.svalue = &CTR_CLEX_KW_THIS_SV;
  CTR_CLEX_KW_THIS.interfaces = ctr_heap_allocate(sizeof(ctr_interfaces));
  //----//
  CTR_CLEX_US_SV.value = "_";
  CTR_CLEX_US_SV.vlen = 1;
  //----//
  CTR_CLEX_US.info.type = CTR_OBJECT_TYPE_OTSTRING;
  CTR_CLEX_US.info.mark = 0;
  CTR_CLEX_US.info.sticky = 1;
  CTR_CLEX_US.info.chainMode = 0;
  CTR_CLEX_US.info.remote = 0;
  CTR_CLEX_US.info.shared = 0;
  CTR_CLEX_US.info.raw = 0;
  CTR_CLEX_US.value.svalue = &CTR_CLEX_US_SV;
  CTR_CLEX_US.interfaces = ctr_heap_allocate(sizeof(ctr_interfaces));
  //----//
  CTR_CLEX_KW_RESPONDTO_SV.value = "respondTo:";
  CTR_CLEX_KW_RESPONDTO_SV.vlen = 10;
  //----//
  CTR_CLEX_KW_RESPONDTO.info.type = CTR_OBJECT_TYPE_OTSTRING;
  CTR_CLEX_KW_RESPONDTO.info.mark = 0;
  CTR_CLEX_KW_RESPONDTO.info.sticky = 1;
  CTR_CLEX_KW_RESPONDTO.info.chainMode = 0;
  CTR_CLEX_KW_RESPONDTO.info.remote = 0;
  CTR_CLEX_KW_RESPONDTO.info.shared = 0;
  CTR_CLEX_KW_RESPONDTO.info.raw = 0;
  CTR_CLEX_KW_RESPONDTO.value.svalue = &CTR_CLEX_KW_RESPONDTO_SV;
  CTR_CLEX_KW_RESPONDTO.interfaces = ctr_heap_allocate(sizeof(ctr_interfaces));
  //----//
  CtrStdWorld = ctr_internal_create_object(CTR_OBJECT_TYPE_OTOBJECT);
  CtrStdWorld->info.sticky = 1;
  ctr_contexts[0] = CtrStdWorld;
  ctr_world_ptr = CtrStdWorld;

  ctr_instrumentor_funcs = ctr_internal_create_object(
      CTR_OBJECT_TYPE_OTOBJECT); // register instrumentors to nil
  // ctr_past_instrumentor_func = NULL;   //register instrumentors to nil
  /* Object */
  CtrStdObject = ctr_internal_create_object(CTR_OBJECT_TYPE_OTOBJECT);
  CtrStdString = ctr_internal_create_object(CTR_OBJECT_TYPE_OTSTRING);
  CtrStdBlock = ctr_internal_create_object(CTR_OBJECT_TYPE_OTBLOCK);
  ctr_set_link_all(CtrStdString, CtrStdObject);
  ctr_set_link_all(CtrStdBlock, CtrStdObject);
  CtrStdObject->interfaces->link = NULL;
  CtrStdObject->interfaces->count = 0;
  CtrStdObject->interfaces->ifs = ctr_heap_allocate(sizeof(ctr_object *));
  CtrStdObject->interfaces->ifs[0] = NULL;
  CtrStdObject->info.sticky = 1;
  ctr_set_link_all(CtrStdString, CtrStdObject);
}

/**
 * @internal
 *
 * WorldInitialize
 *
 * Populate the World of Citron.
 */
void ctr_initialize_world() {
  if (ctr_world_initialized)
    return;
  volatile ctr_thread_workaround_double_list_t* tw = ctr_heap_allocate_tracked(sizeof(*tw));
  tw->next = NULL;
  tw->prev = ctr_thread_workaround_double_list;
  tw->context = ctr_contexts;
  ctr_thread_workaround_double_list = (ctr_thread_workaround_double_list_t*) tw;

  trace_ignore_count = 0;
  ctr_world_initialized = 1;
  // register_signal_handlers ();
  ctr_instrument = 0;
  ctr_global_instrum = NULL;
  int i;
  srand((unsigned)time(NULL));
  for (i = 0; i < 16; i++) {
    CtrHashKey[i] = (rand() % 255);
  }
  ctr_first_object = NULL;
  //----//
  CTR_CLEX_KW_ME_SV.value = "me";
  CTR_CLEX_KW_ME_SV.vlen = 2;
  //----//
  CTR_CLEX_KW_ME.info.type = CTR_OBJECT_TYPE_OTSTRING;
  CTR_CLEX_KW_ME.info.mark = 0;
  CTR_CLEX_KW_ME.info.sticky = 1;
  CTR_CLEX_KW_ME.info.chainMode = 0;
  CTR_CLEX_KW_ME.info.remote = 0;
  CTR_CLEX_KW_ME.info.shared = 0;
  CTR_CLEX_KW_ME.info.raw = 0;
  CTR_CLEX_KW_ME.value.svalue = &CTR_CLEX_KW_ME_SV;
  CTR_CLEX_KW_ME.interfaces = ctr_heap_allocate(sizeof(ctr_interfaces));
  //----//
  CTR_CLEX_KW_THIS_SV.value = "thisBlock";
  CTR_CLEX_KW_THIS_SV.vlen = 9;
  //----//
  CTR_CLEX_KW_THIS.info.type = CTR_OBJECT_TYPE_OTSTRING;
  CTR_CLEX_KW_THIS.info.mark = 0;
  CTR_CLEX_KW_THIS.info.sticky = 1;
  CTR_CLEX_KW_THIS.info.chainMode = 0;
  CTR_CLEX_KW_THIS.info.remote = 0;
  CTR_CLEX_KW_THIS.info.shared = 0;
  CTR_CLEX_KW_THIS.info.raw = 0;
  CTR_CLEX_KW_THIS.value.svalue = &CTR_CLEX_KW_THIS_SV;
  CTR_CLEX_KW_THIS.interfaces = ctr_heap_allocate(sizeof(ctr_interfaces));
  //----//
  CTR_CLEX_US_SV.value = "_";
  CTR_CLEX_US_SV.vlen = 1;
  //----//
  CTR_CLEX_US.info.type = CTR_OBJECT_TYPE_OTSTRING;
  CTR_CLEX_US.info.mark = 0;
  CTR_CLEX_US.info.sticky = 1;
  CTR_CLEX_US.info.chainMode = 0;
  CTR_CLEX_US.info.remote = 0;
  CTR_CLEX_US.info.shared = 0;
  CTR_CLEX_US.info.raw = 0;
  CTR_CLEX_US.value.svalue = &CTR_CLEX_US_SV;
  CTR_CLEX_US.interfaces = ctr_heap_allocate(sizeof(ctr_interfaces));
  //----//
  CTR_CLEX_KW_RESPONDTO_SV.value = "respondTo:";
  CTR_CLEX_KW_RESPONDTO_SV.vlen = 10;
  //----//
  CTR_CLEX_KW_RESPONDTO.info.type = CTR_OBJECT_TYPE_OTSTRING;
  CTR_CLEX_KW_RESPONDTO.info.mark = 0;
  CTR_CLEX_KW_RESPONDTO.info.sticky = 1;
  CTR_CLEX_KW_RESPONDTO.info.chainMode = 0;
  CTR_CLEX_KW_RESPONDTO.info.remote = 0;
  CTR_CLEX_KW_RESPONDTO.info.shared = 0;
  CTR_CLEX_KW_RESPONDTO.info.raw = 0;
  CTR_CLEX_KW_RESPONDTO.value.svalue = &CTR_CLEX_KW_RESPONDTO_SV;
  CTR_CLEX_KW_RESPONDTO.interfaces = ctr_heap_allocate(sizeof(ctr_interfaces));
  //----//
  CtrStdWorld = ctr_internal_create_object(CTR_OBJECT_TYPE_OTOBJECT);
  CtrStdWorld->info.sticky = 1;
  ctr_contexts[0] = CtrStdWorld;
  ctr_world_ptr = CtrStdWorld;

  ctr_instrumentor_funcs = ctr_internal_create_object(
      CTR_OBJECT_TYPE_OTOBJECT); // register instrumentors to nil
  // ctr_past_instrumentor_func = NULL;   //register instrumentors to nil
  /* Object */
  CtrStdObject = ctr_internal_create_object(CTR_OBJECT_TYPE_OTOBJECT);
  CtrStdString = ctr_internal_create_object(CTR_OBJECT_TYPE_OTSTRING);
  CtrStdSymbol = ctr_internal_create_object(CTR_OBJECT_TYPE_OTMISC);
  CtrStdBlock = ctr_internal_create_object(CTR_OBJECT_TYPE_OTBLOCK);
  ctr_object *CtrStdOpaque =
      ctr_internal_create_object(CTR_OBJECT_TYPE_OTOBJECT);
  ctr_internal_create_func(CtrStdOpaque,
                           ctr_build_string_from_cstring(CTR_DICT_NEW),
                           &ctr_object_make);
  ctr_internal_create_func(CtrStdOpaque,
                           ctr_build_string_from_cstring(CTR_DICT_CTOR_NEW),
                           &ctr_object_ctor);
  ctr_internal_create_func(CtrStdOpaque,
                           ctr_build_string_from_cstring(CTR_DICT_ONDO),
                           &ctr_object_on_do);
  ctr_internal_create_func(CtrStdOpaque,
                           ctr_build_string_from_cstring(CTR_DICT_RESPOND_TO),
                           &ctr_object_respond);
  ctr_internal_create_func(
      CtrStdOpaque, ctr_build_string_from_cstring(CTR_DICT_RESPOND_TO_AND),
      &ctr_object_respond_and);
  ctr_internal_create_func(
      CtrStdOpaque, ctr_build_string_from_cstring(CTR_DICT_RESPOND_TO_AND_AND),
      &ctr_object_respond_and_and);
  ctr_internal_object_add_property(
      CtrStdWorld, ctr_build_string_from_cstring("Opaque"), CtrStdOpaque, 0);
  ctr_set_link_all(CtrStdString, CtrStdObject);
  ctr_set_link_all(CtrStdSymbol, CtrStdString);
  ctr_set_link_all(CtrStdBlock, CtrStdObject);

  ctr_internal_create_func(CtrStdObject,
                           ctr_build_string_from_cstring(CTR_DICT_NEW),
                           &ctr_object_make);
  ctr_internal_create_func(CtrStdObject,
                           ctr_build_string_from_cstring(CTR_DICT_CTOR_NEW),
                           &ctr_object_ctor);
  ctr_internal_create_func(CtrStdObject,
                           ctr_build_string_from_cstring(CTR_DICT_GENACC),
                           &ctr_object_attr_accessor);
  ctr_internal_create_func(CtrStdObject,
                           ctr_build_string_from_cstring(CTR_DICT_GENREA),
                           &ctr_object_attr_reader);
  ctr_internal_create_func(CtrStdObject,
                           ctr_build_string_from_cstring(CTR_DICT_GENWRI),
                           &ctr_object_attr_writer);
#ifdef CTR_DEBUG_HIDING
  ctr_internal_create_func(CtrStdObject,
                           ctr_build_string_from_cstring(CTR_DICT_NEW_HIDING),
                           &ctr_object_make_hiding);
#endif // CTR_DEBUG_HIDING
  ctr_internal_create_func(CtrStdObject,
                           ctr_build_string_from_cstring(CTR_DICT_SWAP),
                           &ctr_object_swap);
  ctr_internal_create_func(CtrStdObject,
                           ctr_build_string_from_cstring(CTR_DICT_EQUALS),
                           &ctr_object_equals);
  ctr_internal_create_func(
      CtrStdObject, ctr_build_string_from_cstring(CTR_DICT_SYMBOL_EQUALS),
      &ctr_object_equals);
  ctr_internal_create_func(CtrStdObject, ctr_build_string_from_cstring("id"),
                           &ctr_object_id);
  ctr_internal_create_func(CtrStdObject,
                           ctr_build_string_from_cstring(CTR_DICT_ONDO),
                           &ctr_object_on_do);
  ctr_internal_create_func(CtrStdObject,
                           ctr_build_string_from_cstring(CTR_DICT_RESPOND_TO),
                           &ctr_object_respond);
  ctr_internal_create_func(
      CtrStdObject, ctr_build_string_from_cstring(CTR_DICT_RESPOND_TO_AND),
      &ctr_object_respond_and);
  ctr_internal_create_func(
      CtrStdObject, ctr_build_string_from_cstring(CTR_DICT_RESPOND_TO_AND_AND),
      &ctr_object_respond_and_and);
  ctr_internal_create_func(CtrStdObject,
                           ctr_build_string_from_cstring(CTR_DICT_TYPE),
                           &ctr_object_type);
  ctr_internal_create_func(CtrStdObject,
                           ctr_build_string_from_cstring(CTR_DICT_ISNIL),
                           &ctr_object_is_nil);
  ctr_internal_create_func(CtrStdObject,
                           ctr_build_string_from_cstring(CTR_DICT_MYSELF),
                           &ctr_object_myself);
  ctr_internal_create_func(
      CtrStdObject, ctr_build_string_from_cstring(CTR_DICT_DO), &ctr_object_do);
  ctr_internal_create_func(CtrStdObject,
                           ctr_build_string_from_cstring(CTR_DICT_DONE),
                           &ctr_object_done);
  ctr_internal_create_func(CtrStdObject, ctr_build_string_from_cstring("or:"),
                           &ctr_object_elvis_op);
  ctr_internal_create_func(CtrStdObject,
                           ctr_build_string_from_cstring(CTR_DICT_IFFALSE),
                           &ctr_object_if_false);
  ctr_internal_create_func(CtrStdObject,
                           ctr_build_string_from_cstring(CTR_DICT_IFTRUE),
                           &ctr_object_if_true);
  ctr_internal_create_func(CtrStdObject,
                           ctr_build_string_from_cstring("ifTrue:ifFalse:"),
                           &ctr_object_if_tf);
  ctr_internal_create_func(CtrStdObject,
                           ctr_build_string_from_cstring(CTR_DICT_MESSAGEARGS),
                           &ctr_object_message);
  ctr_internal_create_func(CtrStdObject,
                           ctr_build_string_from_cstring(CTR_DICT_LEARN),
                           &ctr_object_learn_meaning);
  ctr_internal_create_func(CtrStdObject,
                           ctr_build_string_from_cstring(CTR_DICT_TOSTRING),
                           &ctr_object_to_string);
  ctr_internal_create_func(CtrStdObject,
                           ctr_build_string_from_cstring(CTR_DICT_TONUMBER),
                           &ctr_object_to_number);
  ctr_internal_create_func(CtrStdObject,
                           ctr_build_string_from_cstring(CTR_DICT_TOBOOL),
                           &ctr_object_to_boolean);
  ctr_internal_create_func(CtrStdObject,
                           ctr_build_string_from_cstring(CTR_DICT_SOCK_I6),
                           &ctr_command_remote);
  ctr_internal_create_func(CtrStdObject,
                           ctr_build_string_from_cstring(CTR_DICT_SOCK_P),
                           &ctr_command_remote);
  ctr_internal_create_func(CtrStdObject,
                           ctr_build_string_from_cstring(CTR_DICT_SOCK),
                           &ctr_command_remote);
  ctr_internal_create_func(CtrStdObject,
                           ctr_build_string_from_cstring("destruct"),
                           &ctr_object_destruct);
  ctr_internal_create_func(CtrStdObject,
                           ctr_build_string_from_cstring(CTR_DICT_UNPACK),
                           &ctr_object_assign);
  ctr_internal_create_func(CtrStdObject, ctr_build_string_from_cstring("iHash"),
                           &ctr_object_hash);
  ctr_internal_create_func(CtrStdObject,
                           ctr_build_string_from_cstring("isConstructible:"),
                           &ctr_object_is_constructible);

  ctr_internal_object_add_property(
      CtrStdWorld, ctr_build_string_from_cstring(CTR_DICT_OBJECT), CtrStdObject,
      0);
  CtrStdObject->interfaces->link = NULL;
  CtrStdObject->interfaces->count = 0;
  CtrStdObject->interfaces->ifs = ctr_heap_allocate(sizeof(ctr_object *));
  CtrStdObject->interfaces->ifs[0] = NULL;
  CtrStdObject->info.sticky = 1;

  /* Nil */
  CtrStdNil = ctr_internal_create_object(CTR_OBJECT_TYPE_OTNIL);
  ctr_internal_object_add_property(
      CtrStdWorld, ctr_build_string_from_cstring(CTR_DICT_NIL), CtrStdNil, 0);
  ctr_internal_create_func(CtrStdNil,
                           ctr_build_string_from_cstring(CTR_DICT_ISNIL),
                           &ctr_nil_is_nil);
  ctr_internal_create_func(CtrStdNil,
                           ctr_build_string_from_cstring(CTR_DICT_TOSTRING),
                           &ctr_nil_to_string);
  ctr_internal_create_func(CtrStdNil,
                           ctr_build_string_from_cstring(CTR_DICT_TONUMBER),
                           &ctr_nil_to_number);
  ctr_internal_create_func(CtrStdNil,
                           ctr_build_string_from_cstring(CTR_DICT_TOBOOL),
                           &ctr_nil_to_boolean);
  ctr_internal_create_func(CtrStdNil,
                           ctr_build_string_from_cstring(CTR_DICT_UNPACK),
                           &ctr_nil_assign);
  ctr_set_link_all(CtrStdNil, CtrStdObject);
  CtrStdNil->info.sticky = 1;

  /* Boolean */
  CtrStdBool = ctr_internal_create_object(CTR_OBJECT_TYPE_OTBOOL);
  ctr_internal_create_func(CtrStdBool,
                           ctr_build_string_from_cstring(CTR_DICT_IFTRUE),
                           &ctr_bool_if_true);
  ctr_internal_create_func(CtrStdBool,
                           ctr_build_string_from_cstring(CTR_DICT_IFFALSE),
                           &ctr_bool_if_false);
  ctr_internal_create_func(CtrStdBool,
                           ctr_build_string_from_cstring("ifTrue:ifFalse:"),
                           &ctr_bool_if_tf);
  ctr_internal_create_func(CtrStdBool,
                           ctr_build_string_from_cstring(CTR_DICT_BREAK),
                           &ctr_bool_break);
  ctr_internal_create_func(CtrStdBool,
                           ctr_build_string_from_cstring(CTR_DICT_CONTINUE),
                           &ctr_bool_continue);
  ctr_internal_create_func(CtrStdBool,
                           ctr_build_string_from_cstring(CTR_DICT_ELSE),
                           &ctr_bool_if_false);
  ctr_internal_create_func(
      CtrStdBool, ctr_build_string_from_cstring(CTR_DICT_NOT), &ctr_bool_not);
  ctr_internal_create_func(
      CtrStdBool, ctr_build_string_from_cstring(CTR_DICT_AND), &ctr_bool_and);
  ctr_internal_create_func(
      CtrStdBool, ctr_build_string_from_cstring(CTR_DICT_NOR), &ctr_bool_nor);
  ctr_internal_create_func(
      CtrStdBool, ctr_build_string_from_cstring(CTR_DICT_OR), &ctr_bool_or);
  ctr_internal_create_func(
      CtrStdBool, ctr_build_string_from_cstring(CTR_DICT_XOR), &ctr_bool_xor);
  ctr_internal_create_func(
      CtrStdBool, ctr_build_string_from_cstring(CTR_DICT_SYMBOL_EQUALS),
      &ctr_bool_eq);
  ctr_internal_create_func(CtrStdBool,
                           ctr_build_string_from_cstring(CTR_DICT_NOTEQUAL),
                           &ctr_bool_neq);
  ctr_internal_create_func(CtrStdBool,
                           ctr_build_string_from_cstring(CTR_DICT_TONUMBER),
                           &ctr_bool_to_number);
  ctr_internal_create_func(CtrStdBool,
                           ctr_build_string_from_cstring(CTR_DICT_TOSTRING),
                           &ctr_bool_to_string);
  ctr_internal_create_func(
      CtrStdBool, ctr_build_string_from_cstring(CTR_DICT_FLIP), &ctr_bool_flip);
  ctr_internal_create_func(CtrStdBool,
                           ctr_build_string_from_cstring(CTR_DICT_UNPACK),
                           &ctr_bool_assign);
  ctr_internal_create_func(CtrStdObject,
                           ctr_build_string_from_cstring(CTR_DICT_EITHEROR),
                           &ctr_bool_either_or);
  ctr_internal_object_add_property(
      CtrStdWorld, ctr_build_string_from_cstring(CTR_DICT_BOOLEAN), CtrStdBool,
      0);
  ctr_set_link_all(CtrStdBool, CtrStdObject);
  CtrStdBool->info.sticky = 1;

  /* Number */
  CtrStdNumber = ctr_internal_create_object(CTR_OBJECT_TYPE_OTNUMBER);
  ctr_internal_create_func(CtrStdNumber,
                           ctr_build_string_from_cstring(CTR_DICT_TOSTEPDO),
                           &ctr_number_to_step_do);
  ctr_internal_create_func(CtrStdNumber,
                           ctr_build_string_from_cstring(CTR_DICT_PLUS),
                           &ctr_number_add);
  ctr_internal_create_func(CtrStdNumber,
                           ctr_build_string_from_cstring(CTR_DICT_ADD),
                           &ctr_number_inc);
  ctr_internal_create_func(CtrStdNumber,
                           ctr_build_string_from_cstring(CTR_DICT_MINUS),
                           &ctr_number_minus);
  ctr_internal_create_func(CtrStdNumber,
                           ctr_build_string_from_cstring(CTR_DICT_SUBTRACT),
                           &ctr_number_dec);
  ctr_internal_create_func(CtrStdNumber,
                           ctr_build_string_from_cstring(CTR_DICT_MULTIPLIER),
                           &ctr_number_multiply);
  ctr_internal_create_func(CtrStdNumber,
                           ctr_build_string_from_cstring(CTR_DICT_TIMES),
                           &ctr_number_times);
  ctr_internal_create_func(CtrStdNumber,
                           ctr_build_string_from_cstring(CTR_DICT_MULTIPLY),
                           &ctr_number_mul);
  ctr_internal_create_func(CtrStdNumber,
                           ctr_build_string_from_cstring(CTR_DICT_DIVISION),
                           &ctr_number_divide);
  ctr_internal_create_func(CtrStdNumber,
                           ctr_build_string_from_cstring(CTR_DICT_DIVIDE),
                           &ctr_number_div);
  ctr_internal_create_func(CtrStdNumber,
                           ctr_build_string_from_cstring(CTR_DICT_GREATER),
                           &ctr_number_higherThan);
  ctr_internal_create_func(
      CtrStdNumber, ctr_build_string_from_cstring(CTR_DICT_GREATER_OR_EQUAL),
      &ctr_number_higherEqThan);
  ctr_internal_create_func(CtrStdNumber,
                           ctr_build_string_from_cstring(CTR_DICT_LESS),
                           &ctr_number_lowerThan);
  ctr_internal_create_func(
      CtrStdNumber, ctr_build_string_from_cstring(CTR_DICT_LESS_OR_EQUAL),
      &ctr_number_lowerEqThan);
  ctr_internal_create_func(
      CtrStdNumber, ctr_build_string_from_cstring(CTR_DICT_SYMBOL_EQUALS),
      &ctr_number_eq);
  ctr_internal_create_func(CtrStdNumber,
                           ctr_build_string_from_cstring(CTR_DICT_NOTEQUAL),
                           &ctr_number_neq);
  ctr_internal_create_func(CtrStdNumber,
                           ctr_build_string_from_cstring(CTR_DICT_MODULO),
                           &ctr_number_modulo);
  ctr_internal_create_func(CtrStdNumber,
                           ctr_build_string_from_cstring(CTR_DICT_FACTORIAL),
                           &ctr_number_factorial);
  ctr_internal_create_func(CtrStdNumber,
                           ctr_build_string_from_cstring(CTR_DICT_FLOOR),
                           &ctr_number_floor);
  ctr_internal_create_func(CtrStdNumber,
                           ctr_build_string_from_cstring(CTR_DICT_CEIL),
                           &ctr_number_ceil);
  ctr_internal_create_func(CtrStdNumber,
                           ctr_build_string_from_cstring(CTR_DICT_ROUND),
                           &ctr_number_round);
  ctr_internal_create_func(CtrStdNumber,
                           ctr_build_string_from_cstring(CTR_DICT_ABS),
                           &ctr_number_abs);
  ctr_internal_create_func(CtrStdNumber,
                           ctr_build_string_from_cstring(CTR_DICT_SIN),
                           &ctr_number_sin);
  ctr_internal_create_func(CtrStdNumber,
                           ctr_build_string_from_cstring(CTR_DICT_COS),
                           &ctr_number_cos);
  ctr_internal_create_func(CtrStdNumber,
                           ctr_build_string_from_cstring(CTR_DICT_EXP),
                           &ctr_number_exp);
  ctr_internal_create_func(CtrStdNumber,
                           ctr_build_string_from_cstring(CTR_DICT_SQRT),
                           &ctr_number_sqrt);
  ctr_internal_create_func(CtrStdNumber,
                           ctr_build_string_from_cstring(CTR_DICT_TAN),
                           &ctr_number_tan);
  ctr_internal_create_func(CtrStdNumber,
                           ctr_build_string_from_cstring(CTR_DICT_ATAN),
                           &ctr_number_atan);
  ctr_internal_create_func(CtrStdNumber,
                           ctr_build_string_from_cstring(CTR_DICT_LOG),
                           &ctr_number_log);
  ctr_internal_create_func(CtrStdNumber,
                           ctr_build_string_from_cstring(CTR_DICT_POWER),
                           &ctr_number_pow);
  ctr_internal_create_func(CtrStdNumber,
                           ctr_build_string_from_cstring(CTR_DICT_MIN_ARG),
                           &ctr_number_min);
  ctr_internal_create_func(CtrStdNumber,
                           ctr_build_string_from_cstring(CTR_DICT_MAX_ARG),
                           &ctr_number_max);
  ctr_internal_create_func(CtrStdNumber,
                           ctr_build_string_from_cstring(CTR_DICT_ODD),
                           &ctr_number_odd);
  ctr_internal_create_func(CtrStdNumber,
                           ctr_build_string_from_cstring(CTR_DICT_EVEN),
                           &ctr_number_even);
  ctr_internal_create_func(CtrStdNumber,
                           ctr_build_string_from_cstring(CTR_DICT_POS),
                           &ctr_number_positive);
  ctr_internal_create_func(CtrStdNumber,
                           ctr_build_string_from_cstring(CTR_DICT_NEG),
                           &ctr_number_negative);
  ctr_internal_create_func(CtrStdNumber,
                           ctr_build_string_from_cstring(CTR_DICT_TOSTRING),
                           &ctr_number_to_string);
  ctr_internal_create_func(CtrStdNumber,
                           ctr_build_string_from_cstring("toStringInBase:"),
                           &ctr_number_to_string_base);
  ctr_internal_create_func(CtrStdNumber,
                           ctr_build_string_from_cstring(CTR_DICT_TOBOOL),
                           &ctr_number_to_boolean);
  ctr_internal_create_func(CtrStdNumber,
                           ctr_build_string_from_cstring(CTR_DICT_TONUMBER),
                           &ctr_object_myself);
  ctr_internal_create_func(CtrStdNumber,
                           ctr_build_string_from_cstring(CTR_DICT_BETWEEN),
                           &ctr_number_between);
  ctr_internal_create_func(CtrStdNumber,
                           ctr_build_string_from_cstring(CTR_DICT_SLL),
                           &ctr_number_shl);
  ctr_internal_create_func(CtrStdNumber,
                           ctr_build_string_from_cstring(CTR_DICT_SRL),
                           &ctr_number_shr);
  ctr_internal_create_func(CtrStdNumber,
                           ctr_build_string_from_cstring(CTR_DICT_INTOR),
                           &ctr_number_or);
  ctr_internal_create_func(CtrStdNumber,
                           ctr_build_string_from_cstring(CTR_DICT_INTAND),
                           &ctr_number_and);
  ctr_internal_create_func(CtrStdNumber,
                           ctr_build_string_from_cstring(CTR_DICT_INTXOR),
                           &ctr_number_xor);
  ctr_internal_create_func(CtrStdNumber,
                           ctr_build_string_from_cstring(CTR_DICT_TOBITS),
                           &ctr_number_uint_binrep);
  ctr_internal_create_func(CtrStdNumber,
                           ctr_build_string_from_cstring(CTR_DICT_UNPACK),
                           &ctr_number_assign);
  ctr_internal_create_func(CtrStdNumber,
                           ctr_build_string_from_cstring(CTR_DICT_TO_BYTE),
                           &ctr_number_to_byte);
  ctr_internal_create_func(CtrStdNumber,
                           ctr_build_string_from_cstring("negate"),
                           &ctr_number_negate);
  ctr_internal_create_func(CtrStdNumber,
                           ctr_build_string_from_cstring(CTR_DICT_QUALIFY),
                           &ctr_number_qualify);
  ctr_internal_create_func(CtrStdNumber,
                           ctr_build_string_from_cstring(CTR_DICT_RESPOND_TO),
                           &ctr_number_qualify);
  ctr_internal_object_add_property(
      CtrStdWorld, ctr_build_string_from_cstring(CTR_DICT_NUMBER), CtrStdNumber,
      0);
  ctr_set_link_all(CtrStdNumber, CtrStdObject);
  CtrStdNumber->info.sticky = 1;

  /* String */
  // CtrStdString = ctr_internal_create_object(CTR_OBJECT_TYPE_OTSTRING);
  ctr_linkstr();
  ctr_internal_create_func(CtrStdString,
                           ctr_build_string_from_cstring("escape:"),
                           &ctr_string_escape_ascii);
  ctr_internal_create_func(CtrStdString,
                           ctr_build_string_from_cstring("escapeAsciiControls"),
                           &ctr_string_escape_ascii);
  ctr_internal_create_func(CtrStdString,
                           ctr_build_string_from_cstring(CTR_DICT_BYTES),
                           &ctr_string_bytes);
  ctr_internal_create_func(CtrStdString,
                           ctr_build_string_from_cstring(CTR_DICT_LENGTH),
                           &ctr_string_length);
  ctr_internal_create_func(CtrStdString, ctr_build_string_from_cstring("empty"),
                           &ctr_string_empty);
  ctr_internal_create_func(CtrStdString,
                           ctr_build_string_from_cstring(CTR_DICT_FROM_TO),
                           &ctr_string_fromto);
  ctr_internal_create_func(CtrStdString,
                           ctr_build_string_from_cstring(CTR_DICT_FROM_LENGTH),
                           &ctr_string_from_length);
  ctr_internal_create_func(CtrStdString,
                           ctr_build_string_from_cstring(CTR_DICT_PLUS),
                           &ctr_string_concat);
  ctr_internal_create_func(
      CtrStdString, ctr_build_string_from_cstring(CTR_DICT_SYMBOL_EQUALS),
      &ctr_string_eq);
  ctr_internal_create_func(CtrStdString,
                           ctr_build_string_from_cstring(CTR_DICT_UNEQUALS),
                           &ctr_string_neq);
  ctr_internal_create_func(CtrStdString,
                           ctr_build_string_from_cstring(CTR_DICT_TRIM),
                           &ctr_string_trim);
  ctr_internal_create_func(CtrStdString,
                           ctr_build_string_from_cstring(CTR_DICT_LEFT_TRIM),
                           &ctr_string_ltrim);
  ctr_internal_create_func(CtrStdString,
                           ctr_build_string_from_cstring(CTR_DICT_RIGHT_TRIM),
                           &ctr_string_rtrim);
  ctr_internal_create_func(CtrStdString,
                           ctr_build_string_from_cstring(CTR_DICT_HTML_ESCAPE),
                           &ctr_string_html_escape);
  ctr_internal_create_func(CtrStdString,
                           ctr_build_string_from_cstring(CTR_DICT_COUNTOF),
                           &ctr_string_count_of);
  ctr_internal_create_func(CtrStdString,
                           ctr_build_string_from_cstring(CTR_DICT_SLICEFROMTO),
                           &ctr_string_slice);
  ctr_internal_create_func(
      CtrStdString, ctr_build_string_from_cstring(CTR_DICT_AT), &ctr_string_at);
  ctr_internal_create_func(CtrStdString,
                           ctr_build_string_from_cstring(CTR_DICT_AT_SYMBOL),
                           &ctr_string_at);
  ctr_internal_create_func(CtrStdString,
                           ctr_build_string_from_cstring(CTR_DICT_PUT_AT),
                           &ctr_string_put_at);
  ctr_internal_create_func(CtrStdString,
                           ctr_build_string_from_cstring(CTR_DICT_BYTE_AT),
                           &ctr_string_byte_at);
  ctr_internal_create_func(CtrStdString,
                           ctr_build_string_from_cstring(CTR_DICT_INDEX_OF),
                           &ctr_string_index_of);
  ctr_internal_create_func(CtrStdString, ctr_build_string_from_cstring("fmap:"),
                           &ctr_string_fmap);
  ctr_internal_create_func(CtrStdString, ctr_build_string_from_cstring("imap:"),
                           &ctr_string_imap);
  ctr_internal_create_func(CtrStdString,
                           ctr_build_string_from_cstring("filter:"),
                           &ctr_string_filter);
  ctr_internal_create_func(CtrStdString,
                           ctr_build_string_from_cstring("reIndexOf:"),
                           &ctr_string_re_index_of);
  ctr_internal_create_func(CtrStdString,
                           ctr_build_string_from_cstring("startsWith:"),
                           &ctr_string_starts_with);
  ctr_internal_create_func(CtrStdString,
                           ctr_build_string_from_cstring("endsWith:"),
                           &ctr_string_ends_with);
  ctr_internal_create_func(
      CtrStdString, ctr_build_string_from_cstring(CTR_DICT_LAST_INDEX_OF),
      &ctr_string_last_index_of);
  ctr_internal_create_func(CtrStdString,
                           ctr_build_string_from_cstring("reLastIndexOf:"),
                           &ctr_string_re_last_index_of);
  ctr_internal_create_func(CtrStdString,
                           ctr_build_string_from_cstring(CTR_DICT_REPLACE_WITH),
                           &ctr_string_replace_with);
  ctr_internal_create_func(CtrStdString,
                           ctr_build_string_from_cstring(CTR_DICT_SPLIT),
                           &ctr_string_split);
  ctr_internal_create_func(CtrStdString,
                           ctr_build_string_from_cstring(CTR_DICT_SPLIT "max:"),
                           &ctr_string_split);
  ctr_internal_create_func(CtrStdString,
                           ctr_build_string_from_cstring("reSplit:"),
                           &ctr_string_split_re);
  ctr_internal_create_func(CtrStdString,
                           ctr_build_string_from_cstring("reSplitGen:"),
                           &ctr_string_split_re_gen);
  ctr_internal_create_func(
      CtrStdString, ctr_build_string_from_cstring(CTR_DICT_ASCII_UPPER_CASE),
      &ctr_string_to_upper);
  ctr_internal_create_func(
      CtrStdString, ctr_build_string_from_cstring(CTR_DICT_ASCII_LOWER_CASE),
      &ctr_string_to_lower);
  ctr_internal_create_func(
      CtrStdString, ctr_build_string_from_cstring(CTR_DICT_ASCII_UPPER_CASE_1),
      &ctr_string_to_upper1st);
  ctr_internal_create_func(
      CtrStdString, ctr_build_string_from_cstring(CTR_DICT_ASCII_LOWER_CASE_1),
      &ctr_string_to_lower1st);
  ctr_internal_create_func(CtrStdString,
                           ctr_build_string_from_cstring(CTR_DICT_SKIP),
                           &ctr_string_skip);
  ctr_internal_create_func(CtrStdString,
                           ctr_build_string_from_cstring(CTR_DICT_APPEND),
                           &ctr_string_append);
  ctr_internal_create_func(CtrStdString,
                           ctr_build_string_from_cstring("multiply:"),
                           &ctr_string_multiply);
  ctr_internal_create_func(CtrStdString,
                           ctr_build_string_from_cstring(CTR_DICT_MULTIPLIER),
                           &ctr_string_multiply);
  ctr_internal_create_func(CtrStdString,
                           ctr_build_string_from_cstring(CTR_DICT_ADD),
                           &ctr_string_append);
  ctr_internal_create_func(CtrStdString,
                           ctr_build_string_from_cstring(CTR_DICT_TO_NUMBER),
                           &ctr_string_to_number);
  ctr_internal_create_func(CtrStdString,
                           ctr_build_string_from_cstring(CTR_DICT_TOBOOL),
                           &ctr_string_to_boolean);
  ctr_internal_create_func(
      CtrStdString, ctr_build_string_from_cstring(CTR_DICT_FIND_PATTERN_DO),
      &ctr_string_find_pattern_do);
  ctr_internal_create_func(
      CtrStdString,
      ctr_build_string_from_cstring(CTR_DICT_FIND_PATTERN_DO_OPTIONS),
      &ctr_string_find_pattern_options_do);
  ctr_internal_create_func(
      CtrStdString,
      ctr_build_string_from_cstring("compileRegex:substitute:options:"),
      &ctr_string_reg_compile);
  ctr_internal_create_func(CtrStdString,
                           ctr_build_string_from_cstring("compileRegex:"),
                           &ctr_string_reg_compile);
  ctr_internal_create_func(
      CtrStdString, ctr_build_string_from_cstring("regex:substitute:options:"),
      &ctr_string_reg_replace);
  ctr_internal_create_func(CtrStdString,
                           ctr_build_string_from_cstring("regex:substitute:"),
                           &ctr_string_reg_replace);
  ctr_internal_create_func(CtrStdString, ctr_build_string_from_cstring("~"),
                           &ctr_string_reg_replace);
  ctr_internal_create_func(CtrStdString,
                           ctr_build_string_from_cstring("p/:s/:o/:"),
                           &ctr_string_reg_replace);
  ctr_internal_create_func(CtrStdString,
                           ctr_build_string_from_cstring("p/:s/:"),
                           &ctr_string_reg_replace);
  ctr_internal_create_func(CtrStdString,
                           ctr_build_string_from_cstring(CTR_DICT_REGEX_ISPCRE),
                           &ctr_string_is_regex_pcre);
  ctr_internal_create_func(
      CtrStdString, ctr_build_string_from_cstring(CTR_DICT_CONTAINS_PATTERN),
      &ctr_string_contains_pattern);
  ctr_internal_create_func(
      CtrStdString, ctr_build_string_from_cstring(CTR_DICT_HASH_WITH_KEY),
      &ctr_string_hash_with_key);
  ctr_internal_create_func(CtrStdString,
                           ctr_build_string_from_cstring(CTR_DICT_EVAL),
                           &ctr_string_eval);
  ctr_internal_create_func(CtrStdString, ctr_build_string_from_cstring("exec"),
                           &ctr_string_exec);
  ctr_internal_create_func(CtrStdString,
                           ctr_build_string_from_cstring(CTR_DICT_TOSTRING),
                           &ctr_string_to_string);
  ctr_internal_create_func(CtrStdString,
                           ctr_build_string_from_cstring(CTR_DICT_STRFMT),
                           &ctr_string_format);
  ctr_internal_create_func(CtrStdString, ctr_build_string_from_cstring("%"),
                           &ctr_string_format);
  ctr_internal_create_func(CtrStdString,
                           ctr_build_string_from_cstring(CTR_DICT_STRFMTMAP),
                           &ctr_string_format_map);
  ctr_internal_create_func(CtrStdString, ctr_build_string_from_cstring("%~:"),
                           &ctr_string_format_map);
  ctr_internal_create_func(CtrStdString,
                           ctr_build_string_from_cstring("escapeDoubleQuotes"),
                           &ctr_string_dquotes_escape);
  ctr_internal_create_func(CtrStdString,
                           ctr_build_string_from_cstring("escapeQuotes"),
                           &ctr_string_quotes_escape);
  ctr_internal_create_func(CtrStdString,
                           ctr_build_string_from_cstring(CTR_DICT_CHARACTERS),
                           &ctr_string_characters);
  ctr_internal_create_func(
      CtrStdString, ctr_build_string_from_cstring(CTR_DICT_TO_BYTE_ARRAY),
      &ctr_string_to_byte_array);
  ctr_internal_create_func(CtrStdString,
                           ctr_build_string_from_cstring(CTR_DICT_APPEND_BYTE),
                           &ctr_string_append_byte);
  ctr_internal_create_func(CtrStdString,
                           ctr_build_string_from_cstring("charSub:"),
                           &ctr_string_csub);
  ctr_internal_create_func(CtrStdString,
                           ctr_build_string_from_cstring("charAdd:"),
                           &ctr_string_cadd);
  ctr_internal_create_func(CtrStdString,
                           ctr_build_string_from_cstring(CTR_DICT_CONTAINS),
                           &ctr_string_contains);
  ctr_internal_create_func(CtrStdString,
                           ctr_build_string_from_cstring("paddingLeft:"),
                           &ctr_string_padding_left);
  ctr_internal_create_func(CtrStdString,
                           ctr_build_string_from_cstring("paddingRight:"),
                           &ctr_string_padding_right);
  ctr_internal_create_func(
      CtrStdString, ctr_build_string_from_cstring("randomizeBytesWithLength:"),
      &ctr_string_randomize_bytes);
  ctr_internal_create_func(CtrStdString,
                           ctr_build_string_from_cstring("reverse"),
                           &ctr_string_reverse);
  // ctr_internal_create_func (CtrStdString, ctr_build_string_from_cstring
  // ("isConstructible:"), &ctr_string_is_ctor);
  ctr_internal_create_func(CtrStdString,
                           ctr_build_string_from_cstring(CTR_DICT_UNPACK),
                           &ctr_string_assign);
  ctr_internal_object_add_property(
      CtrStdWorld, ctr_build_string_from_cstring(CTR_DICT_STRING), CtrStdString,
      0);
  ctr_set_link_all(CtrStdString, CtrStdObject);
  CtrStdString->info.sticky = 1;

  /* Block */
  ctr_internal_create_func(CtrStdBlock,
                           ctr_build_string_from_cstring(CTR_DICT_RUN),
                           &ctr_block_runIt);
  ctr_internal_create_func(CtrStdBlock,
                           ctr_build_string_from_cstring("specialize:with:"),
                           &ctr_block_specialise);
  ctr_internal_create_func(CtrStdBlock,
                           ctr_build_string_from_cstring(CTR_DICT_APPLY_TO),
                           &ctr_block_runIt);
  ctr_internal_create_func(CtrStdBlock,
                           ctr_build_string_from_cstring(CTR_DICT_APPLY_TO_AND),
                           &ctr_block_runIt);
  ctr_internal_create_func(
      CtrStdBlock, ctr_build_string_from_cstring(CTR_DICT_APPLY_TO_AND_AND),
      &ctr_block_runIt);
  ctr_internal_create_func(
      CtrStdBlock, ctr_build_string_from_cstring(CTR_DICT_APPLY_TO_AND_AND),
      &ctr_block_runIt);
  ctr_internal_create_func(
      CtrStdBlock,
      ctr_build_string_from_cstring(CTR_DICT_APPLY_TO_AND_AND "and:"),
      &ctr_block_runIt);
  ctr_internal_create_func(
      CtrStdBlock,
      ctr_build_string_from_cstring(CTR_DICT_APPLY_TO_AND_AND "and:and:"),
      &ctr_block_runIt);

  ctr_internal_create_func(CtrStdBlock,
                           ctr_build_string_from_cstring("applyAll:"),
                           &ctr_block_runall);

  ctr_internal_create_func(CtrStdBlock,
                           ctr_build_string_from_cstring(CTR_DICT_SET_VALUE),
                           &ctr_block_set);
  ctr_internal_create_func(CtrStdBlock,
                           ctr_build_string_from_cstring(CTR_DICT_ERROR),
                           &ctr_block_error);
  ctr_internal_create_func(CtrStdBlock,
                           ctr_build_string_from_cstring(CTR_DICT_CATCH),
                           &ctr_block_catch);
  ctr_internal_create_func(CtrStdBlock,
                           ctr_build_string_from_cstring("catch:type:"),
                           &ctr_block_catch_type);
  ctr_internal_create_func(CtrStdBlock,
                           ctr_build_string_from_cstring(CTR_DICT_WHILE_TRUE),
                           &ctr_block_while_true);
  ctr_internal_create_func(CtrStdBlock,
                           ctr_build_string_from_cstring(CTR_DICT_WHILE_FALSE),
                           &ctr_block_while_false);
  ctr_internal_create_func(CtrStdBlock,
                           ctr_build_string_from_cstring("forever"),
                           &ctr_block_forever);
  ctr_internal_create_func(CtrStdBlock,
                           ctr_build_string_from_cstring(CTR_DICT_TOSTRING),
                           &ctr_block_to_string);
  ctr_internal_create_func(CtrStdBlock,
                           ctr_build_string_from_cstring(CTR_DICT_UNPACK),
                           &ctr_block_assign);
  ctr_internal_object_add_property(
      CtrStdWorld, ctr_build_string_from_cstring(CTR_DICT_CODE_BLOCK),
      CtrStdBlock, 0);
  ctr_set_link_all(CtrStdBlock, CtrStdObject);
  CtrStdBlock->info.sticky = 1;

  /* Array */
  CtrStdArray = ctr_array_new(CtrStdObject, NULL);
  ctr_internal_create_func(
      CtrStdArray, ctr_build_string_from_cstring(CTR_DICT_NEW), &ctr_array_new);
  ctr_internal_create_func(CtrStdArray,
                           ctr_build_string_from_cstring(CTR_DICT_NEW_ARG),
                           &ctr_array_new);
  ctr_internal_create_func(CtrStdArray,
                           ctr_build_string_from_cstring("newFill:with:"),
                           &ctr_array_alloc);
  ctr_internal_create_func(
      CtrStdArray, ctr_build_string_from_cstring(CTR_DICT_NEW_ARRAY_AND_PUSH),
      &ctr_array_new_and_push);
  ctr_internal_create_func(
      CtrStdArray,
      ctr_build_string_from_cstring(CTR_DICT_NEW_ARRAY_AND_PUSH_SYMBOL),
      &ctr_array_new_and_push);
  ctr_internal_create_func(CtrStdArray,
                           ctr_build_string_from_cstring(CTR_DICT_TYPE),
                           &ctr_array_type);
  ctr_internal_create_func(CtrStdArray,
                           ctr_build_string_from_cstring(CTR_DICT_PUSH),
                           &ctr_array_push);
  ctr_internal_create_func(CtrStdArray,
                           ctr_build_string_from_cstring(CTR_DICT_PUSH_SYMBOL),
                           &ctr_array_push);
  ctr_internal_create_func(CtrStdArray, ctr_build_string_from_cstring("fmap:"),
                           &ctr_array_fmap);
  ctr_internal_create_func(CtrStdArray, ctr_build_string_from_cstring("imap:"),
                           &ctr_array_imap);
  ctr_internal_create_func(CtrStdArray, ctr_build_string_from_cstring("fmap!:"),
                           &ctr_array_fmap_inp);
  ctr_internal_create_func(CtrStdArray, ctr_build_string_from_cstring("imap!:"),
                           &ctr_array_imap_inp);
  ctr_internal_create_func(CtrStdArray,
                           ctr_build_string_from_cstring("foldl:accumulator:"),
                           &ctr_array_foldl);
  ctr_internal_create_func(CtrStdArray, ctr_build_string_from_cstring("foldl:"),
                           &ctr_array_foldl0);
  ctr_internal_create_func(
      CtrStdArray, ctr_build_string_from_cstring("filter:"), &ctr_array_filter);
  ctr_internal_create_func(CtrStdArray,
                           ctr_build_string_from_cstring("filter_v:"),
                           &ctr_array_filter_v);
  ctr_internal_create_func(CtrStdArray,
                           ctr_build_string_from_cstring("fmap:from:"),
                           &ctr_array_select_from_if);
  ctr_internal_create_func(CtrStdArray,
                           ctr_build_string_from_cstring("fmap:from:filter:"),
                           &ctr_array_select_from_if);
  ctr_internal_create_func(CtrStdArray, ctr_build_string_from_cstring("copy"),
                           &ctr_array_copy);
  ctr_internal_create_func(CtrStdArray, ctr_build_string_from_cstring("items"),
                           &ctr_array_copy);
  ctr_internal_create_func(CtrStdArray,
                           ctr_build_string_from_cstring("reverse"),
                           &ctr_array_reverse);
  ctr_internal_create_func(CtrStdArray,
                           ctr_build_string_from_cstring(CTR_DICT_UNSHIFT),
                           &ctr_array_unshift);
  ctr_internal_create_func(CtrStdArray,
                           ctr_build_string_from_cstring(CTR_DICT_SHIFT),
                           &ctr_array_shift);
  ctr_internal_create_func(CtrStdArray,
                           ctr_build_string_from_cstring(CTR_DICT_COUNT),
                           &ctr_array_count);
  ctr_internal_create_func(CtrStdArray, ctr_build_string_from_cstring("empty"),
                           &ctr_array_empty);
  ctr_internal_create_func(CtrStdArray,
                           ctr_build_string_from_cstring(CTR_DICT_JOIN),
                           &ctr_array_join);
  ctr_internal_create_func(
      CtrStdArray, ctr_build_string_from_cstring(CTR_DICT_POP), &ctr_array_pop);
  ctr_internal_create_func(CtrStdArray,
                           ctr_build_string_from_cstring(CTR_DICT_POP ":"),
                           &ctr_array_pop);
  ctr_internal_create_func(
      CtrStdArray, ctr_build_string_from_cstring(CTR_DICT_AT), &ctr_array_get);
  ctr_internal_create_func(CtrStdArray,
                           ctr_build_string_from_cstring(CTR_DICT_AT_SYMBOL),
                           &ctr_array_get);
  ctr_internal_create_func(
      CtrStdArray, ctr_build_string_from_cstring("indexOf:"), &ctr_array_index);
  ctr_internal_create_func(CtrStdArray,
                           ctr_build_string_from_cstring("contains:"),
                           &ctr_array_contains);
  ctr_internal_create_func(CtrStdArray,
                           ctr_build_string_from_cstring(CTR_DICT_SORT),
                           &ctr_array_sort);
  ctr_internal_create_func(CtrStdArray,
                           ctr_build_string_from_cstring(CTR_DICT_PUT_AT),
                           &ctr_array_put);
  ctr_internal_create_func(CtrStdArray,
                           ctr_build_string_from_cstring(CTR_DICT_FROM_LENGTH),
                           &ctr_array_from_length);
  ctr_internal_create_func(CtrStdArray, ctr_build_string_from_cstring("skip:"),
                           &ctr_array_skip);
  ctr_internal_create_func(CtrStdArray, ctr_build_string_from_cstring("zip"),
                           &ctr_array_zip);
  ctr_internal_create_func(CtrStdArray,
                           ctr_build_string_from_cstring("internal-product"),
                           &ctr_array_internal_product);
  ctr_internal_create_func(CtrStdArray,
                           ctr_build_string_from_cstring("zipWith:"),
                           &ctr_array_zip_with);
  ctr_internal_create_func(CtrStdArray,
                           ctr_build_string_from_cstring(CTR_DICT_PLUS),
                           &ctr_array_add);
  ctr_internal_create_func(CtrStdArray, ctr_build_string_from_cstring("head"),
                           &ctr_array_head);
  ctr_internal_create_func(CtrStdArray, ctr_build_string_from_cstring("tail"),
                           &ctr_array_tail);
  ctr_internal_create_func(CtrStdArray, ctr_build_string_from_cstring("init"),
                           &ctr_array_init);
  ctr_internal_create_func(CtrStdArray, ctr_build_string_from_cstring("last"),
                           &ctr_array_last);
  ctr_internal_create_func(
      CtrStdArray, ctr_build_string_from_cstring(CTR_DICT_MAP), &ctr_array_map);
  ctr_internal_create_func(CtrStdArray, ctr_build_string_from_cstring("map_v:"),
                           &ctr_array_map_v);
  ctr_internal_create_func(CtrStdArray,
                           ctr_build_string_from_cstring(CTR_DICT_EACH),
                           &ctr_array_map);
  ctr_internal_create_func(
      CtrStdArray, ctr_build_string_from_cstring("each_v:"), &ctr_array_map_v);
  ctr_internal_create_func(
      CtrStdArray, ctr_build_string_from_cstring(CTR_DICT_MIN), &ctr_array_min);
  ctr_internal_create_func(
      CtrStdArray, ctr_build_string_from_cstring(CTR_DICT_MAX), &ctr_array_max);
  ctr_internal_create_func(
      CtrStdArray, ctr_build_string_from_cstring(CTR_DICT_SUM), &ctr_array_sum);
  ctr_internal_create_func(CtrStdArray,
                           ctr_build_string_from_cstring(CTR_DICT_PRODUCT),
                           &ctr_array_product);
  ctr_internal_create_func(CtrStdArray, ctr_build_string_from_cstring("*"),
                           &ctr_array_multiply);
  ctr_internal_create_func(CtrStdArray,
                           ctr_build_string_from_cstring(CTR_DICT_TOSTRING),
                           &ctr_array_to_string);
  ctr_internal_create_func(CtrStdArray,
                           ctr_build_string_from_cstring("fill:with:"),
                           &ctr_array_fill);
  ctr_internal_create_func(
      CtrStdArray, ctr_build_string_from_cstring("column:"), &ctr_array_column);
  ctr_internal_create_func(CtrStdArray,
                           ctr_build_string_from_cstring(CTR_DICT_UNPACK),
                           &ctr_array_assign);
  ctr_internal_create_func(CtrStdArray,
                           ctr_build_string_from_cstring(CTR_DICT_SERIALIZE),
                           &ctr_array_to_string);
  ctr_internal_create_func(CtrStdArray,
                           ctr_build_string_from_cstring("intersperse:"),
                           &ctr_array_intersperse);
  ctr_internal_create_func(
      CtrStdArray, ctr_build_string_from_cstring("chunks:"), &ctr_array_chunks);
  ctr_internal_create_func(CtrStdArray,
                           ctr_build_string_from_cstring("chunks:fill:"),
                           &ctr_array_chunks);
  ctr_internal_create_func(CtrStdArray,
                           ctr_build_string_from_cstring("every:do:"),
                           &ctr_array_every_do_fill);
  ctr_internal_create_func(CtrStdArray,
                           ctr_build_string_from_cstring("every:do:fill:"),
                           &ctr_array_every_do_fill);
  ctr_internal_create_func(CtrStdArray,
                           ctr_build_string_from_cstring("from:lengthMax:"),
                           &ctr_array_slice);
  ctr_internal_create_func(CtrStdArray, ctr_build_string_from_cstring("any"),
                           &ctr_array_any);
  ctr_internal_create_func(CtrStdArray, ctr_build_string_from_cstring("all"),
                           &ctr_array_all);
  ctr_internal_create_func(CtrStdArray, ctr_build_string_from_cstring("any:"),
                           &ctr_array_any);
  ctr_internal_create_func(CtrStdArray, ctr_build_string_from_cstring("all:"),
                           &ctr_array_all);
  ctr_internal_object_add_property(
      CtrStdWorld, ctr_build_string_from_cstring(CTR_DICT_ARRAY), CtrStdArray,
      0);
  ctr_set_link_all(CtrStdArray, CtrStdObject);
  CtrStdArray->info.sticky = 1;

  /* Map */
  CtrStdMap = ctr_internal_create_object(CTR_OBJECT_TYPE_OTOBJECT);
  ctr_internal_create_func(
      CtrStdMap, ctr_build_string_from_cstring(CTR_DICT_NEW), &ctr_map_new);
  ctr_internal_create_func(CtrStdMap,
                           ctr_build_string_from_cstring(CTR_DICT_NEW_ARG),
                           &ctr_map_new_);
  ctr_internal_create_func(
      CtrStdMap, ctr_build_string_from_cstring(CTR_DICT_TYPE), &ctr_map_type);
  ctr_internal_create_func(
      CtrStdMap, ctr_build_string_from_cstring(CTR_DICT_PUT_AT), &ctr_map_put);
  ctr_internal_create_func(
      CtrStdMap, ctr_build_string_from_cstring("deleteAt:"), &ctr_map_rm);
  ctr_internal_create_func(
      CtrStdMap, ctr_build_string_from_cstring(CTR_DICT_AT), &ctr_map_get);
  ctr_internal_create_func(CtrStdMap,
                           ctr_build_string_from_cstring(CTR_DICT_AT_SYMBOL),
                           &ctr_map_get);
  ctr_internal_create_func(CtrStdMap,
                           ctr_build_string_from_cstring("getOrInsert:"),
                           &ctr_map_get_or_insert);
  ctr_internal_create_func(CtrStdMap, ctr_build_string_from_cstring("@|"),
                           &ctr_map_get_or_insert);
  ctr_internal_create_func(
      CtrStdMap, ctr_build_string_from_cstring(CTR_DICT_COUNT), &ctr_map_count);
  ctr_internal_create_func(CtrStdMap, ctr_build_string_from_cstring("empty"),
                           &ctr_map_empty);
  ctr_internal_create_func(
      CtrStdMap, ctr_build_string_from_cstring(CTR_DICT_EACH), &ctr_map_each);
  ctr_internal_create_func(
      CtrStdMap, ctr_build_string_from_cstring(CTR_DICT_MAP), &ctr_map_each);
  ctr_internal_create_func(CtrStdMap, ctr_build_string_from_cstring("fmap:"),
                           &ctr_map_fmap);
  ctr_internal_create_func(CtrStdMap, ctr_build_string_from_cstring("fmap!:"),
                           &ctr_map_fmap_inp);
  ctr_internal_create_func(CtrStdMap, ctr_build_string_from_cstring("kvmap:"),
                           &ctr_map_kvmap);
  ctr_internal_create_func(CtrStdMap, ctr_build_string_from_cstring("kvlist:"),
                           &ctr_map_kvlist);
  ctr_internal_create_func(
      CtrStdMap, ctr_build_string_from_cstring("contains:"), &ctr_map_contains);
  ctr_internal_create_func(
      CtrStdMap, ctr_build_string_from_cstring(CTR_DICT_FLIP), &ctr_map_flip);
  ctr_internal_create_func(CtrStdMap,
                           ctr_build_string_from_cstring(CTR_DICT_UNPACK),
                           &ctr_map_assign);
  ctr_internal_create_func(CtrStdMap,
                           ctr_build_string_from_cstring(CTR_DICT_TOSTRING),
                           &ctr_map_to_string);
  ctr_internal_create_func(CtrStdMap,
                           ctr_build_string_from_cstring(CTR_DICT_SERIALIZE),
                           &ctr_map_to_string);
  ctr_internal_object_add_property(
      CtrStdWorld, ctr_build_string_from_cstring(CTR_DICT_MAP_OBJECT),
      CtrStdMap, 0);
  ctr_set_link_all(CtrStdMap, CtrStdObject);
  CtrStdMap->info.sticky = 1;

  /* Map */
  CtrStdHashMap = ctr_internal_create_object(CTR_OBJECT_TYPE_OTOBJECT);
  ctr_internal_create_func(CtrStdHashMap,
                           ctr_build_string_from_cstring(CTR_DICT_NEW),
                           &ctr_hmap_new);
  ctr_internal_create_func(CtrStdHashMap,
                           ctr_build_string_from_cstring(CTR_DICT_TYPE),
                           &ctr_hmap_type);
  ctr_internal_create_func(CtrStdHashMap,
                           ctr_build_string_from_cstring(CTR_DICT_PUT_AT),
                           &ctr_hmap_put);
  ctr_internal_create_func(
      CtrStdHashMap, ctr_build_string_from_cstring("deleteAt:"), &ctr_hmap_rm);
  ctr_internal_create_func(
      CtrStdHashMap, ctr_build_string_from_cstring(CTR_DICT_AT), &ctr_hmap_get);
  ctr_internal_create_func(CtrStdHashMap,
                           ctr_build_string_from_cstring(CTR_DICT_AT_SYMBOL),
                           &ctr_hmap_get);
  ctr_internal_create_func(CtrStdHashMap,
                           ctr_build_string_from_cstring("getOrInsert:"),
                           &ctr_hmap_get_or_insert);
  ctr_internal_create_func(CtrStdHashMap, ctr_build_string_from_cstring("@|"),
                           &ctr_hmap_get_or_insert);
  ctr_internal_create_func(CtrStdHashMap,
                           ctr_build_string_from_cstring(CTR_DICT_COUNT),
                           &ctr_hmap_count);
  ctr_internal_create_func(
      CtrStdHashMap, ctr_build_string_from_cstring("empty"), &ctr_hmap_empty);
  ctr_internal_create_func(CtrStdHashMap,
                           ctr_build_string_from_cstring(CTR_DICT_EACH),
                           &ctr_hmap_each);
  ctr_internal_create_func(CtrStdHashMap,
                           ctr_build_string_from_cstring(CTR_DICT_MAP),
                           &ctr_hmap_each);
  ctr_internal_create_func(
      CtrStdHashMap, ctr_build_string_from_cstring("fmap:"), &ctr_hmap_fmap);
  ctr_internal_create_func(CtrStdHashMap,
                           ctr_build_string_from_cstring("fmap!:"),
                           &ctr_hmap_fmap_inp);
  ctr_internal_create_func(CtrStdHashMap,
                           ctr_build_string_from_cstring("contains:"),
                           &ctr_hmap_contains);
  ctr_internal_create_func(CtrStdHashMap,
                           ctr_build_string_from_cstring(CTR_DICT_FLIP),
                           &ctr_hmap_flip);
  ctr_internal_create_func(CtrStdHashMap,
                           ctr_build_string_from_cstring(CTR_DICT_UNPACK),
                           &ctr_hmap_assign);
  ctr_internal_create_func(CtrStdHashMap,
                           ctr_build_string_from_cstring(CTR_DICT_TOSTRING),
                           &ctr_hmap_to_string);
  ctr_internal_create_func(CtrStdHashMap,
                           ctr_build_string_from_cstring(CTR_DICT_SERIALIZE),
                           &ctr_hmap_to_string);
  ctr_internal_object_add_property(
      CtrStdWorld, ctr_build_string_from_cstring("HashMap"), CtrStdHashMap, 0);
  ctr_set_link_all(CtrStdHashMap, CtrStdObject);
  CtrStdHashMap->info.sticky = 1;

  /* Console */
  CtrStdConsole = ctr_internal_create_object(CTR_OBJECT_TYPE_OTOBJECT);
  ctr_internal_create_func(CtrStdConsole,
                           ctr_build_string_from_cstring(CTR_DICT_WRITE),
                           &ctr_console_write);
  ctr_internal_create_func(CtrStdConsole,
                           ctr_build_string_from_cstring("writeln:"),
                           &ctr_console_writeln);
  ctr_internal_create_func(
      CtrStdConsole,
      ctr_build_string_from_cstring(CTR_DICT_WRITE_TEMPLATE_SYMBOL),
      &ctr_console_write);
  ctr_internal_create_func(CtrStdConsole,
                           ctr_build_string_from_cstring(CTR_DICT_BRK),
                           &ctr_console_brk);
  ctr_internal_create_func(CtrStdConsole,
                           ctr_build_string_from_cstring(CTR_DICT_RED),
                           &ctr_console_red);
  ctr_internal_create_func(CtrStdConsole,
                           ctr_build_string_from_cstring(CTR_DICT_GREEN),
                           &ctr_console_green);
  ctr_internal_create_func(CtrStdConsole,
                           ctr_build_string_from_cstring(CTR_DICT_YELLOW),
                           &ctr_console_yellow);
  ctr_internal_create_func(CtrStdConsole,
                           ctr_build_string_from_cstring(CTR_DICT_BLUE),
                           &ctr_console_blue);
  ctr_internal_create_func(CtrStdConsole,
                           ctr_build_string_from_cstring(CTR_DICT_MAGENTA),
                           &ctr_console_magenta);
  ctr_internal_create_func(CtrStdConsole,
                           ctr_build_string_from_cstring(CTR_DICT_PURPLE),
                           &ctr_console_magenta);
  ctr_internal_create_func(CtrStdConsole,
                           ctr_build_string_from_cstring(CTR_DICT_CYAN),
                           &ctr_console_cyan);
  ctr_internal_create_func(CtrStdConsole,
                           ctr_build_string_from_cstring(CTR_DICT_RESET_COLOR),
                           &ctr_console_reset);
  ctr_internal_create_func(CtrStdConsole,
                           ctr_build_string_from_cstring(CTR_DICT_TAB),
                           &ctr_console_tab);
  ctr_internal_create_func(CtrStdConsole,
                           ctr_build_string_from_cstring(CTR_DICT_LINE),
                           &ctr_console_line);
  ctr_internal_create_func(CtrStdConsole,
                           ctr_build_string_from_cstring("clear"),
                           &ctr_console_clear);
  ctr_internal_create_func(CtrStdConsole,
                           ctr_build_string_from_cstring("clearLine"),
                           &ctr_console_clear_line);
  ctr_internal_object_add_property(CtrStdWorld,
                                   ctr_build_string_from_cstring(CTR_DICT_PEN),
                                   CtrStdConsole, 0);
  ctr_internal_object_add_property(
      CtrStdWorld, ctr_build_string_from_cstring(CTR_DICT_PEN_TEMPLATE_SYMBOL),
      CtrStdConsole, 0);
  ctr_set_link_all(CtrStdConsole, CtrStdObject);
  CtrStdConsole->info.sticky = 1;

  /* File */
  CtrStdFile = ctr_internal_create_object(CTR_OBJECT_TYPE_OTOBJECT);
  CtrStdFile->value.rvalue = NULL;
  ctr_internal_create_func(CtrStdFile,
                           ctr_build_string_from_cstring(CTR_DICT_NEW_ARG),
                           &ctr_file_new);
  ctr_internal_create_func(
      CtrStdFile, ctr_build_string_from_cstring(CTR_DICT_PATH), &ctr_file_path);
  ctr_internal_create_func(CtrStdFile,
                           ctr_build_string_from_cstring("fileDescriptor"),
                           &ctr_file_get_descriptor);
  ctr_internal_create_func(CtrStdFile,
                           ctr_build_string_from_cstring("memopen:mode:"),
                           &ctr_file_memopen);
  ctr_internal_create_func(
      CtrStdFile, ctr_build_string_from_cstring("duplicateDescriptor:"),
      &ctr_file_ddup);
  ctr_internal_create_func(
      CtrStdFile,
      ctr_build_string_from_cstring("duplicateDescriptor:toDescriptor:"),
      &ctr_file_ddup);
#ifdef forLinux
  ctr_internal_create_func(CtrStdFile,
                           ctr_build_string_from_cstring(
                               "duplicateDescriptor:toDescriptor:withFlags:"),
                           &ctr_file_ddup);
#endif
  ctr_internal_create_func(
      CtrStdFile, ctr_build_string_from_cstring("realPath"), &ctr_file_rpath);
  ctr_internal_create_func(CtrStdFile, ctr_build_string_from_cstring("expand:"),
                           &ctr_file_expand);
  ctr_internal_create_func(CtrStdFile,
                           ctr_build_string_from_cstring(CTR_DICT_EXT_PATH),
                           &ctr_file_stdext_path);
  ctr_internal_create_func(
      CtrStdFile, ctr_build_string_from_cstring(CTR_DICT_READ), &ctr_file_read);

  ctr_internal_create_func(
      CtrStdFile, ctr_build_string_from_cstring("generateLines"), &ctr_file_generate_lines);

  ctr_internal_create_func(
      CtrStdFile, ctr_build_string_from_cstring("generateLinesBlocking:"), &ctr_file_generate_lines);

  // ctr_internal_create_func(
  //     CtrStdFile, ctr_build_string_from_cstring("generateLinesNonblocking"), &ctr_file_generate_lines);
  ctr_internal_create_func(CtrStdFile,
                           ctr_build_string_from_cstring(CTR_DICT_WRITE),
                           &ctr_file_write);
  ctr_internal_create_func(CtrStdFile,
                           ctr_build_string_from_cstring(CTR_DICT_APPEND),
                           &ctr_file_append);
  ctr_internal_create_func(CtrStdFile,
                           ctr_build_string_from_cstring(CTR_DICT_EXISTS),
                           &ctr_file_exists);
  ctr_internal_create_func(
      CtrStdFile, ctr_build_string_from_cstring(CTR_DICT_SIZE), &ctr_file_size);
  ctr_internal_create_func(CtrStdFile,
                           ctr_build_string_from_cstring(CTR_DICT_DELETE),
                           &ctr_file_delete);
  ctr_internal_create_func(CtrStdFile,
                           ctr_build_string_from_cstring(CTR_DICT_INCLUDE),
                           &ctr_file_include);
  ctr_internal_create_func(CtrStdFile,
                           ctr_build_string_from_cstring(CTR_DICT_INCLUDEHERE),
                           &ctr_file_include_here);
  ctr_internal_create_func(
      CtrStdFile, ctr_build_string_from_cstring(CTR_DICT_OPEN), &ctr_file_open);
  ctr_internal_create_func(CtrStdFile,
                           ctr_build_string_from_cstring(CTR_DICT_CLOSE),
                           &ctr_file_close);
  ctr_internal_create_func(CtrStdFile, ctr_build_string_from_cstring("flush"),
                           &ctr_file_flush);
  ctr_internal_create_func(CtrStdFile,
                           ctr_build_string_from_cstring(CTR_DICT_READ_BYTES),
                           &ctr_file_read_bytes);
  ctr_internal_create_func(CtrStdFile,
                           ctr_build_string_from_cstring(CTR_DICT_WRITE_BYTES),
                           &ctr_file_write_bytes);
  ctr_internal_create_func(
      CtrStdFile, ctr_build_string_from_cstring(CTR_DICT_SEEK), &ctr_file_seek);
  ctr_internal_create_func(CtrStdFile,
                           ctr_build_string_from_cstring(CTR_DICT_REWIND),
                           &ctr_file_seek_rewind);
  ctr_internal_create_func(CtrStdFile,
                           ctr_build_string_from_cstring(CTR_DICT_END),
                           &ctr_file_seek_end);
  ctr_internal_create_func(
      CtrStdFile, ctr_build_string_from_cstring(CTR_DICT_LOCK), &ctr_file_lock);
  ctr_internal_create_func(CtrStdFile,
                           ctr_build_string_from_cstring(CTR_DICT_UNLOCK),
                           &ctr_file_unlock);
  ctr_internal_create_func(
      CtrStdFile, ctr_build_string_from_cstring(CTR_DICT_LIST), &ctr_file_list);
  ctr_internal_create_func(CtrStdFile, ctr_build_string_from_cstring("typeOf:"),
                           &ctr_file_type);
  ctr_internal_create_func(CtrStdFile,
                           ctr_build_string_from_cstring(CTR_DICT_TMPFILE),
                           &ctr_file_tmp);
  ctr_internal_create_func(CtrStdFile, ctr_build_string_from_cstring("unpack:"),
                           &ctr_file_assign);
  ctr_internal_create_func(
      CtrStdFile, ctr_build_string_from_cstring("special:"), &ctr_file_special);
  ctr_internal_create_func(CtrStdFile,
                           ctr_build_string_from_cstring("fdopen:mode:"),
                           &ctr_file_special);
  ctr_internal_create_func(CtrStdFile,
                           ctr_build_string_from_cstring(CTR_DICT_TOSTRING),
                           &ctr_file_to_string);
  ctr_internal_create_func(CtrStdFile, ctr_build_string_from_cstring("mkdir"),
                           &ctr_file_mkdir);
  ctr_internal_create_func(CtrStdFile, ctr_build_string_from_cstring("mkdir:"),
                           &ctr_file_mkdir);
  CTR_FILE_STDIN = ctr_internal_create_object(CTR_OBJECT_TYPE_OTEX);
  CTR_FILE_STDOUT = ctr_internal_create_object(CTR_OBJECT_TYPE_OTEX);
  CTR_FILE_STDERR = ctr_internal_create_object(CTR_OBJECT_TYPE_OTEX);
  ctr_set_link_all(CTR_FILE_STDIN, CtrStdObject);
  ctr_set_link_all(CTR_FILE_STDOUT, CtrStdObject);
  ctr_set_link_all(CTR_FILE_STDERR, CtrStdObject);
  CTR_FILE_STDIN->info.sticky = 1;
  CTR_FILE_STDOUT->info.sticky = 1;
  CTR_FILE_STDERR->info.sticky = 1;
  CTR_FILE_STDIN_STR = ctr_build_string_from_cstring("stdin");
  CTR_FILE_STDOUT_STR = ctr_build_string_from_cstring("stdout");
  CTR_FILE_STDERR_STR = ctr_build_string_from_cstring("stderr");
  ctr_internal_object_add_property(CtrStdFile, CTR_FILE_STDIN_STR,
                                   CTR_FILE_STDIN, 0);
  ctr_internal_object_add_property(CtrStdFile, CTR_FILE_STDOUT_STR,
                                   CTR_FILE_STDOUT, 0);
  ctr_internal_object_add_property(CtrStdFile, CTR_FILE_STDERR_STR,
                                   CTR_FILE_STDERR, 0);
  ctr_internal_object_add_property(
      CtrStdWorld, ctr_build_string_from_cstring(CTR_DICT_FILE), CtrStdFile, 0);

  ctr_set_link_all(CtrStdFile, CtrStdObject);
  CtrStdFile->info.sticky = 1;

  /* Command */
  CtrStdCommand = ctr_internal_create_object(CTR_OBJECT_TYPE_OTOBJECT);
  ctr_internal_create_func(CtrStdCommand,
                           ctr_build_string_from_cstring(CTR_DICT_ARGUMENT),
                           &ctr_command_argument);
  ctr_internal_create_func(
      CtrStdCommand, ctr_build_string_from_cstring(CTR_DICT_ARGUMENT_COUNT),
      &ctr_command_num_of_args);
  ctr_internal_create_func(
      CtrStdCommand,
      ctr_build_string_from_cstring(CTR_DICT_ENVIRONMENT_VARIABLE),
      &ctr_command_get_env);
  ctr_internal_create_func(
      CtrStdCommand,
      ctr_build_string_from_cstring(CTR_DICT_SET_ENVIRONMENT_VARIABLE),
      &ctr_command_set_env);
  ctr_internal_create_func(
      CtrStdCommand, ctr_build_string_from_cstring(CTR_DICT_WAIT_FOR_INPUT),
      &ctr_command_waitforinput);
  ctr_internal_create_func(CtrStdCommand,
                           ctr_build_string_from_cstring(CTR_DICT_GETC),
                           &ctr_command_getc);
  ctr_internal_create_func(CtrStdCommand,
                           ctr_build_string_from_cstring(CTR_DICT_INPUT),
                           &ctr_command_input);
  ctr_internal_create_func(
      CtrStdCommand, ctr_build_string_from_cstring("setInterruptHandler:"),
      &ctr_command_set_INT_handler);
  ctr_internal_create_func(CtrStdCommand,
                           ctr_build_string_from_cstring(CTR_DICT_EXIT),
                           &ctr_command_exit);
  ctr_internal_create_func(CtrStdCommand,
                           ctr_build_string_from_cstring(CTR_DICT_EXIT ":"),
                           &ctr_command_exit);
  ctr_internal_create_func(CtrStdCommand,
                           ctr_build_string_from_cstring(CTR_DICT_FLUSH),
                           &ctr_command_flush);
  ctr_internal_create_func(CtrStdCommand,
                           ctr_build_string_from_cstring(CTR_DICT_FORBID_SHELL),
                           &ctr_command_forbid_shell);
  ctr_internal_create_func(
      CtrStdCommand, ctr_build_string_from_cstring(CTR_DICT_FORBID_FILE_WRITE),
      &ctr_command_forbid_file_write);
  ctr_internal_create_func(
      CtrStdCommand, ctr_build_string_from_cstring(CTR_DICT_FORBID_FILE_READ),
      &ctr_command_forbid_file_read);
  ctr_internal_create_func(
      CtrStdCommand, ctr_build_string_from_cstring(CTR_DICT_FORBID_INCLUDE),
      &ctr_command_forbid_include);
  ctr_internal_create_func(CtrStdCommand,
                           ctr_build_string_from_cstring(CTR_DICT_FORBID_FORK),
                           &ctr_command_forbid_fork);
  ctr_internal_create_func(
      CtrStdCommand,
      ctr_build_string_from_cstring(CTR_DICT_SET_REMAINING_MESSAGES),
      &ctr_command_countdown);
  ctr_internal_create_func(CtrStdCommand,
                           ctr_build_string_from_cstring(CTR_DICT_NEW_SET),
                           &ctr_command_fork);
  ctr_internal_create_func(CtrStdCommand,
                           ctr_build_string_from_cstring(CTR_DICT_MESSAGE),
                           &ctr_command_message);
  ctr_internal_create_func(CtrStdCommand,
                           ctr_build_string_from_cstring(CTR_DICT_LISTEN),
                           &ctr_command_listen);
  ctr_internal_create_func(
      CtrStdCommand, ctr_build_string_from_cstring(CTR_DICT_LISTEN "timeout:"),
      &ctr_command_listen);
  ctr_internal_create_func(CtrStdCommand,
                           ctr_build_string_from_cstring(CTR_DICT_JOIN_PROCESS),
                           &ctr_command_join);
  ctr_internal_create_func(CtrStdCommand,
                           ctr_build_string_from_cstring(CTR_DICT_LOG_SET),
                           &ctr_command_log);
  ctr_internal_create_func(CtrStdCommand,
                           ctr_build_string_from_cstring(CTR_DICT_WARNING),
                           &ctr_command_warn);
  ctr_internal_create_func(CtrStdCommand,
                           ctr_build_string_from_cstring(CTR_DICT_ALERT),
                           &ctr_command_crit);
  ctr_internal_create_func(CtrStdCommand,
                           ctr_build_string_from_cstring(CTR_DICT_ERROR),
                           &ctr_command_err);
  ctr_internal_create_func(CtrStdCommand,
                           ctr_build_string_from_cstring(CTR_DICT_PID),
                           &ctr_command_pid);
  ctr_internal_create_func(CtrStdCommand,
                           ctr_build_string_from_cstring("signal:"),
                           &ctr_command_sig);
  ctr_internal_create_func(CtrStdCommand,
                           ctr_build_string_from_cstring(CTR_DICT_SERVE),
                           &ctr_command_accept);
  ctr_internal_create_func(CtrStdCommand,
                           ctr_build_string_from_cstring("serve_ipv4:"),
                           &ctr_command_accepti4);
  ctr_internal_create_func(CtrStdCommand,
                           ctr_build_string_from_cstring(CTR_DICT_CONN_LIMIT),
                           &ctr_command_accept_number);
  ctr_internal_create_func(CtrStdCommand,
                           ctr_build_string_from_cstring(CTR_DICT_PORT),
                           &ctr_command_default_port);
  ctr_internal_create_func(CtrStdCommand,
                           ctr_build_string_from_cstring("changeDirectory:"),
                           &ctr_command_chdir);
  ctr_internal_object_add_property(
      CtrStdWorld, ctr_build_string_from_cstring(CTR_DICT_PROGRAM),
      CtrStdCommand, 0);
  ctr_set_link_all(CtrStdCommand, CtrStdObject);
  CtrStdCommand->info.sticky = 1;

  /* Clock */
  CtrStdClock = ctr_internal_create_object(CTR_OBJECT_TYPE_OTOBJECT);
  ctr_internal_create_func(CtrStdClock,
                           ctr_build_string_from_cstring(CTR_DICT_WAIT),
                           &ctr_clock_wait);
  ctr_internal_create_func(CtrStdClock,
                           ctr_build_string_from_cstring(CTR_DICT_TIME),
                           &ctr_clock_time);
  ctr_internal_create_func(CtrStdClock,
                           ctr_build_string_from_cstring(CTR_DICT_NEW_SET),
                           &ctr_clock_new_set);
  ctr_internal_create_func(
      CtrStdClock, ctr_build_string_from_cstring(CTR_DICT_NEW), &ctr_clock_new);
  ctr_internal_create_func(
      CtrStdClock, ctr_build_string_from_cstring(CTR_DICT_DAY), &ctr_clock_day);
  ctr_internal_create_func(CtrStdClock,
                           ctr_build_string_from_cstring(CTR_DICT_SET_DAY),
                           &ctr_clock_set_day);
  ctr_internal_create_func(CtrStdClock,
                           ctr_build_string_from_cstring(CTR_DICT_MONTH),
                           &ctr_clock_month);
  ctr_internal_create_func(CtrStdClock,
                           ctr_build_string_from_cstring(CTR_DICT_SET_MONTH),
                           &ctr_clock_set_month);
  ctr_internal_create_func(CtrStdClock,
                           ctr_build_string_from_cstring(CTR_DICT_YEAR),
                           &ctr_clock_year);
  ctr_internal_create_func(CtrStdClock,
                           ctr_build_string_from_cstring(CTR_DICT_SET_YEAR),
                           &ctr_clock_set_year);
  ctr_internal_create_func(CtrStdClock,
                           ctr_build_string_from_cstring(CTR_DICT_HOUR),
                           &ctr_clock_hour);
  ctr_internal_create_func(CtrStdClock,
                           ctr_build_string_from_cstring(CTR_DICT_SET_HOUR),
                           &ctr_clock_set_hour);
  ctr_internal_create_func(CtrStdClock,
                           ctr_build_string_from_cstring(CTR_DICT_MINUTE),
                           &ctr_clock_minute);
  ctr_internal_create_func(CtrStdClock,
                           ctr_build_string_from_cstring(CTR_DICT_SET_MINUTE),
                           &ctr_clock_set_minute);
  ctr_internal_create_func(CtrStdClock,
                           ctr_build_string_from_cstring(CTR_DICT_SECOND),
                           &ctr_clock_second);
  ctr_internal_create_func(CtrStdClock,
                           ctr_build_string_from_cstring(CTR_DICT_SET_SECOND),
                           &ctr_clock_set_second);
  ctr_internal_create_func(CtrStdClock,
                           ctr_build_string_from_cstring(CTR_DICT_WEEK_DAY),
                           &ctr_clock_weekday);
  ctr_internal_create_func(CtrStdClock,
                           ctr_build_string_from_cstring(CTR_DICT_YEAR_DAY),
                           &ctr_clock_yearday);
  ctr_internal_create_func(CtrStdClock,
                           ctr_build_string_from_cstring(CTR_DICT_WEEK),
                           &ctr_clock_week);
  ctr_internal_create_func(CtrStdClock,
                           ctr_build_string_from_cstring(CTR_DICT_ZONE),
                           &ctr_clock_get_zone);
  ctr_internal_create_func(CtrStdClock,
                           ctr_build_string_from_cstring(CTR_DICT_ZONE_SET),
                           &ctr_clock_set_zone);
  ctr_internal_create_func(CtrStdClock,
                           ctr_build_string_from_cstring(CTR_DICT_FORMAT),
                           &ctr_clock_format);
  ctr_internal_create_func(CtrStdClock,
                           ctr_build_string_from_cstring(CTR_DICT_LIKE),
                           &ctr_clock_like);
  ctr_internal_create_func(CtrStdClock,
                           ctr_build_string_from_cstring(CTR_DICT_TOSTRING),
                           &ctr_clock_to_string);
  ctr_internal_create_func(CtrStdClock,
                           ctr_build_string_from_cstring(CTR_DICT_ADD_SET),
                           &ctr_clock_add);
  ctr_internal_create_func(CtrStdClock,
                           ctr_build_string_from_cstring(CTR_DICT_SUBTRACT_SET),
                           &ctr_clock_subtract);
  ctr_internal_create_func(CtrStdClock,
                           ctr_build_string_from_cstring("isDaylightSaving"),
                           &ctr_clock_isdst);
  ctr_internal_create_func(CtrStdClock,
                           ctr_build_string_from_cstring("processorClock"),
                           &ctr_clock_processor_time);
  ctr_internal_create_func(CtrStdClock,
                           ctr_build_string_from_cstring("ticksPerSecond"),
                           &ctr_clock_processor_ticks_ps);
  ctr_internal_create_func(CtrStdClock,
                           ctr_build_string_from_cstring("timeExecutionOf:"),
                           &ctr_clock_time_exec);
  ctr_internal_create_func(CtrStdClock,
                           ctr_build_string_from_cstring("executionSeconds:"),
                           &ctr_clock_time_exec_s);
  ctr_internal_object_add_property(
      CtrStdWorld, ctr_build_string_from_cstring(CTR_DICT_CLOCK), CtrStdClock,
      0);
  ctr_clock_init(CtrStdClock);
  ctr_set_link_all(CtrStdClock, CtrStdObject);
  CtrStdClock->info.sticky = 1;

  /* Dice */
  CtrStdDice = ctr_internal_create_object(CTR_OBJECT_TYPE_OTOBJECT);
  ctr_internal_create_func(CtrStdDice,
                           ctr_build_string_from_cstring(CTR_DICT_ROLL),
                           &ctr_dice_throw);
  ctr_internal_create_func(
      CtrStdDice, ctr_build_string_from_cstring(CTR_DICT_ROLL_WITH_SIDES),
      &ctr_dice_sides);
  ctr_internal_create_func(
      CtrStdDice, ctr_build_string_from_cstring(CTR_DICT_RAW_RANDOM_NUMBER),
      &ctr_dice_rand);
  ctr_internal_object_add_property(
      CtrStdWorld, ctr_build_string_from_cstring(CTR_DICT_DICE), CtrStdDice, 0);
  ctr_set_link_all(CtrStdDice, CtrStdObject);
  CtrStdDice->info.sticky = 1;

  /* Slurp */
  CtrStdSlurp = ctr_internal_create_object(CTR_OBJECT_TYPE_OTOBJECT);
  ctr_internal_create_func(CtrStdSlurp,
                           ctr_build_string_from_cstring(CTR_DICT_OBTAIN),
                           &ctr_slurp_obtain);
  ctr_internal_create_func(CtrStdSlurp,
                           ctr_build_string_from_cstring(CTR_DICT_RESPOND_TO),
                           &ctr_slurp_respond_to);
  ctr_internal_create_func(
      CtrStdSlurp, ctr_build_string_from_cstring(CTR_DICT_RESPOND_TO_AND),
      &ctr_slurp_respond_to_and);
  ctr_internal_object_add_property(
      CtrStdWorld, ctr_build_string_from_cstring(CTR_DICT_SLURP), CtrStdSlurp,
      0);
  ctr_set_link_all(CtrStdSlurp, CtrStdObject);
  CtrStdSlurp->info.sticky = 1;

  /* Shell */
  CtrStdShell = ctr_internal_create_object(CTR_OBJECT_TYPE_OTOBJECT);
  ctr_internal_create_func(CtrStdShell,
                           ctr_build_string_from_cstring(CTR_DICT_RESPOND_TO),
                           &ctr_shell_respond_to);
  ctr_internal_create_func(
      CtrStdShell, ctr_build_string_from_cstring(CTR_DICT_RESPOND_TO_AND),
      &ctr_shell_respond_to_and);
  ctr_internal_create_func(CtrStdShell,
                           ctr_build_string_from_cstring(CTR_DICT_CALL),
                           &ctr_shell_call);
  ctr_internal_create_func(CtrStdShell,
                           ctr_build_string_from_cstring("open:mode:"),
                           &ctr_shell_open);
  ctr_internal_object_add_property(
      CtrStdWorld, ctr_build_string_from_cstring(CTR_DICT_SHELL), CtrStdShell,
      0);
  ctr_set_link_all(CtrStdShell, CtrStdObject);
  CtrStdShell->info.sticky = 1;

  /* Broom */
  CtrStdGC = ctr_internal_create_object(CTR_OBJECT_TYPE_OTOBJECT);
  ctr_internal_create_func(CtrStdGC, ctr_build_string_from_cstring("noGC:"),
                           &ctr_gc_with_gc_disabled);
  ctr_internal_create_func(CtrStdGC, ctr_build_string_from_cstring("dump"),
                           &ctr_gc_dump);
  ctr_internal_create_func(
      CtrStdGC, ctr_build_string_from_cstring(CTR_DICT_SWEEP), &ctr_gc_collect);
  ctr_internal_create_func(CtrStdGC,
                           ctr_build_string_from_cstring(CTR_DICT_SWEEP ":"),
                           &ctr_gc_sweep_this);
  ctr_internal_create_func(
      CtrStdGC, ctr_build_string_from_cstring(CTR_DICT_DUST), &ctr_gc_dust);
  ctr_internal_create_func(CtrStdGC,
                           ctr_build_string_from_cstring(CTR_DICT_OBJECT_COUNT),
                           &ctr_gc_object_count);
  ctr_internal_create_func(CtrStdGC,
                           ctr_build_string_from_cstring(CTR_DICT_KEPT_COUNT),
                           &ctr_gc_kept_count);
  ctr_internal_create_func(CtrStdGC,
                           ctr_build_string_from_cstring(CTR_DICT_KEPT_ALLOC),
                           &ctr_gc_kept_alloc);
  ctr_internal_create_func(CtrStdGC,
                           ctr_build_string_from_cstring(CTR_DICT_STICKY_COUNT),
                           &ctr_gc_sticky_count);
  ctr_internal_create_func(CtrStdGC,
                           ctr_build_string_from_cstring(CTR_DICT_MEMORY_LIMIT),
                           &ctr_gc_setmemlimit);
  ctr_internal_create_func(
      CtrStdGC, ctr_build_string_from_cstring(CTR_DICT_MODE), &ctr_gc_setmode);
  ctr_internal_create_func(CtrStdGC,
                           ctr_build_string_from_cstring("autoAlloc:"),
                           &ctr_gc_setautoalloc);
  ctr_internal_object_add_property(
      CtrStdWorld, ctr_build_string_from_cstring(CTR_DICT_BROOM), CtrStdGC, 0);
  ctr_set_link_all(CtrStdGC, CtrStdObject);
  CtrStdGC->info.sticky = 1;

  CtrStdReflect = ctr_reflect_new(CtrStdObject, NULL);

  ctr_internal_create_func(CtrStdReflect,
                           ctr_build_string_from_cstring("addGlobalVariable:"),
                           &ctr_reflect_add_glob);
  ctr_internal_create_func(CtrStdReflect,
                           ctr_build_string_from_cstring("addLocalVariable:"),
                           &ctr_reflect_add_local);
  ctr_internal_create_func(CtrStdReflect,
                           ctr_build_string_from_cstring("addPrivateVariable:"),
                           &ctr_reflect_add_my);
  ctr_internal_create_func(CtrStdReflect,
                           ctr_build_string_from_cstring("set:to:"),
                           &ctr_reflect_set_to);
  ctr_internal_create_func(CtrStdReflect,
                           ctr_build_string_from_cstring("getStrNativeTypeOf:"),
                           &ctr_reflect_nat_type);
  ctr_internal_create_func(CtrStdReflect,
                           ctr_build_string_from_cstring("getContext"),
                           &ctr_reflect_dump_context);
  ctr_internal_create_func(CtrStdReflect,
                           ctr_build_string_from_cstring("thisContext"),
                           &ctr_reflect_this_context);
  ctr_internal_create_func(CtrStdReflect,
                           ctr_build_string_from_cstring("getMethodsOf:"),
                           &ctr_reflect_dump_context_spec);
  ctr_internal_create_func(CtrStdReflect,
                           ctr_build_string_from_cstring("mapPropertiesOf:"),
                           &ctr_reflect_dump_context_prop);
  ctr_internal_create_func(CtrStdReflect,
                           ctr_build_string_from_cstring("getPropertiesOf:"),
                           &ctr_reflect_dump_context_spec_prop);
  ctr_internal_create_func(CtrStdReflect,
                           ctr_build_string_from_cstring("getObject:"),
                           &ctr_reflect_find_obj);
  ctr_internal_create_func(CtrStdReflect,
                           ctr_build_string_from_cstring("objectExists:"),
                           &ctr_reflect_find_obj_ex);
  ctr_internal_create_func(CtrStdReflect,
                           ctr_build_string_from_cstring("argumentListOf:"),
                           &ctr_reflect_cb_ac);
  ctr_internal_create_func(
      CtrStdReflect, ctr_build_string_from_cstring("addArgumentTo:named:"),
      &ctr_reflect_cb_add_param);
  ctr_internal_create_func(CtrStdReflect,
                           ctr_build_string_from_cstring("copyBlock:"),
                           &ctr_reflect_fn_copy);
  ctr_internal_create_func(CtrStdReflect,
                           ctr_build_string_from_cstring("newSharedObject"),
                           &ctr_reflect_share_memory);
  ctr_internal_create_func(CtrStdReflect,
                           ctr_build_string_from_cstring("newSharedObject:"),
                           &ctr_reflect_share_memory);
  ctr_internal_create_func(CtrStdReflect,
                           ctr_build_string_from_cstring("link:to:"),
                           &ctr_reflect_link_to);
  ctr_internal_create_func(CtrStdReflect,
                           ctr_build_string_from_cstring("parentOf:"),
                           &ctr_reflect_get_first_link);
  ctr_internal_create_func(CtrStdReflect,
                           ctr_build_string_from_cstring("isObject:linkedTo:"),
                           &ctr_reflect_is_linked_to);
  ctr_internal_create_func(CtrStdReflect,
                           ctr_build_string_from_cstring("isObject:childOf:"),
                           &ctr_reflect_child_of);
  ctr_internal_create_func(CtrStdReflect,
                           ctr_build_string_from_cstring("generateLinkTree:"),
                           &ctr_reflect_generate_inheritance_tree);
  ctr_internal_create_func(CtrStdReflect,
                           ctr_build_string_from_cstring("typeOf:"),
                           &ctr_reflect_describe_type);
  ctr_internal_create_func(CtrStdReflect,
                           ctr_build_string_from_cstring("strTypeOf:"),
                           &ctr_reflect_type_descriptor_print);
  ctr_internal_create_func(
      CtrStdReflect, ctr_build_string_from_cstring("isObject:constructibleBy:"),
      &ctr_reflect_check_bind_valid_v);
  ctr_internal_create_func(CtrStdReflect,
                           ctr_build_string_from_cstring("unpack:to:"),
                           &ctr_reflect_bind);
  ctr_internal_create_func(
      CtrStdReflect, ctr_build_string_from_cstring("getResponder:ofObject:"),
      &ctr_reflect_get_responder);
  ctr_internal_create_func(CtrStdReflect,
                           ctr_build_string_from_cstring("closure:of:"),
                           &ctr_reflect_closure_of);
  ctr_internal_create_func(
      CtrStdReflect, ctr_build_string_from_cstring("run:forObject:arguments:"),
      &ctr_reflect_run_for_object);
  ctr_internal_create_func(
      CtrStdReflect,
      ctr_build_string_from_cstring("runHere:forObject:arguments:"),
      &ctr_reflect_run_for_object_ctx);
  ctr_internal_create_func(CtrStdReflect,
                           ctr_build_string_from_cstring("primitiveLinkOf:"),
                           &ctr_reflect_get_primitive);
  ctr_internal_create_func(
      CtrStdReflect, ctr_build_string_from_cstring("getProperty:ofObject:"),
      &ctr_reflect_get_property);
  ctr_internal_create_func(
      CtrStdReflect,
      ctr_build_string_from_cstring("setProperty:ofObject:toValue:"),
      &ctr_reflect_set_property);
  ctr_internal_create_func(
      CtrStdReflect, ctr_build_string_from_cstring("object:hasOwnResponder:"),
      &ctr_reflect_has_own_responder);
  ctr_internal_create_func(
      CtrStdReflect, ctr_build_string_from_cstring("disableInstrumentation"),
      &ctr_reflect_rawmsg);
  ctr_internal_create_func(
      CtrStdReflect, ctr_build_string_from_cstring("enableInstrumentation"),
      &ctr_reflect_instrmsg);
  ctr_internal_create_func(
      CtrStdReflect,
      ctr_build_string_from_cstring("registerInstrumentor:forObject:"),
      &ctr_reflect_register_instrumentor);
  ctr_internal_create_func(
      CtrStdReflect,
      ctr_build_string_from_cstring("unregisterInstrumetationForObject:"),
      &ctr_reflect_unregister_instrumentor);
  ctr_internal_create_func(
      CtrStdReflect, ctr_build_string_from_cstring("currentInstrumentorFor:"),
      &ctr_reflect_get_instrumentor);
  ctr_internal_create_func(CtrStdReflect,
                           ctr_build_string_from_cstring("globalInstrument:"),
                           &ctr_reflect_ginstr);
  ctr_internal_create_func(
      CtrStdReflect, ctr_build_string_from_cstring("disableGlobalInstrument"),
      &ctr_reflect_noginstr);
  ctr_internal_create_func(
      CtrStdReflect, ctr_build_string_from_cstring("runAtGlobal:arguments:"),
      &ctr_reflect_run_glob);
  ctr_internal_create_func(CtrStdReflect,
                           ctr_build_string_from_cstring("version"),
                           &ctr_give_version);
  ctr_internal_create_func(CtrStdReflect,
                           ctr_build_string_from_cstring("compilerInfo"),
                           &ctr_reflect_compilerinfo);
  ctr_internal_create_func(CtrStdObject,
                           ctr_build_string_from_cstring("setPrivate:value:"),
                           &ctr_reflect_delegate_set_private_property);
  ctr_internal_create_func(CtrStdObject,
                           ctr_build_string_from_cstring("ignoreInTrace:"),
                           &ctr_reflect_set_ignore_file);
  ctr_internal_create_func(CtrStdObject,
                           ctr_build_string_from_cstring("&method:"),
                           &ctr_reflect_object_delegate_get_responder);
  ctr_internal_create_func(CtrStdObject,
                           ctr_build_string_from_cstring("&responder:"),
                           &ctr_reflect_object_get_responder);
  ctr_internal_create_func(CtrStdReflect,
                           ctr_build_string_from_cstring("lastTrace"),
                           &ctr_get_last_trace);
  ctr_internal_create_func(CtrStdReflect,
                           ctr_build_string_from_cstring("stackTrace"),
                           &ctr_get_last_trace_stringified);
  ctr_internal_create_func(
      CtrStdReflect, ctr_build_string_from_cstring("frame"), &ctr_get_frame);
  ctr_internal_create_func(CtrStdReflect,
                           ctr_build_string_from_cstring("frame:"),
                           &ctr_get_frame_with_id);
  ctr_internal_create_func(CtrStdReflect,
                           ctr_build_string_from_cstring("frameId"),
                           &ctr_get_frame_id);
  ctr_internal_create_func(CtrStdReflect,
                           ctr_build_string_from_cstring("isInFrame:"),
                           &ctr_frame_present);
  ctr_internal_create_func(
      CtrStdReflect, ctr_build_string_from_cstring("run:inContext:arguments:"),
      &ctr_reflect_run_for_object_in_ctx);
  ctr_internal_create_func(
      CtrStdReflect,
      ctr_build_string_from_cstring("run:inContext:forObject:arguments:"),
      &ctr_reflect_run_for_object_inside_ctx);
  ctr_internal_create_func(
      CtrStdReflect,
      ctr_build_string_from_cstring("run:inContextAsWorld:arguments:"),
      &ctr_reflect_run_for_object_in_ctx_as_world);
  ctr_internal_create_func(
      CtrStdReflect,
      ctr_build_string_from_cstring("run:inContextAsMain:arguments:"),
      &ctr_reflect_run_for_object_in_ctx_as_main);
  ctr_internal_create_func(CtrStdReflect,
                           ctr_build_string_from_cstring("worldSnapshot"),
                           &ctr_reflect_world_snap);
  ctr_internal_create_func(CtrStdReflect,
                           ctr_build_string_from_cstring("runInNewContext:"),
                           &ctr_reflect_run_in_new_ctx);
  ctr_internal_create_func(CtrStdReflect,
                           ctr_build_string_from_cstring("marshal:"),
                           &ctr_reflect_marshal);
  ctr_internal_create_func(CtrStdReflect,
                           ctr_build_string_from_cstring("unmarshal:"),
                           &ctr_reflect_unmarshal);

  ctr_internal_object_add_property(
      CtrStdWorld, ctr_build_string_from_cstring("Reflect"), CtrStdReflect, 0);
  CtrStdReflect->info.sticky = 1;

  CtrStdThread = ctr_internal_create_object(CTR_OBJECT_TYPE_OTEX);
  ctr_set_link_all(CtrStdThread, CtrStdObject);
  CtrStdThread->info.sticky = 1;

  ctr_internal_create_func(CtrStdThread, ctr_build_string_from_cstring("new"),
                           &ctr_thread_make);
  ctr_internal_create_func(CtrStdThread, ctr_build_string_from_cstring("new:"),
                           &ctr_thread_make_set_target);
  ctr_internal_create_func(CtrStdThread,
                           ctr_build_string_from_cstring("new:args:"),
                           &ctr_thread_make_set_target);
  ctr_internal_create_func(CtrStdThread,
                           ctr_build_string_from_cstring("target:"),
                           &ctr_thread_set_target);
  ctr_internal_create_func(CtrStdThread, ctr_build_string_from_cstring("run"),
                           &ctr_thread_run);
  ctr_internal_create_func(CtrStdThread, ctr_build_string_from_cstring("finished"),
                           &ctr_thread_finished);
  ctr_internal_create_func(CtrStdThread, ctr_build_string_from_cstring("join"),
                           &ctr_thread_join);
  ctr_internal_create_func(CtrStdThread, ctr_build_string_from_cstring("detach"),
                           &ctr_thread_detach);
  ctr_internal_create_func(CtrStdThread, ctr_build_string_from_cstring("id"),
                           &ctr_thread_id);
  ctr_internal_create_func(CtrStdThread, ctr_build_string_from_cstring("name:"),
                           &ctr_thread_names);
  ctr_internal_create_func(CtrStdThread, ctr_build_string_from_cstring("name"),
                           &ctr_thread_name);
  ctr_internal_create_func(CtrStdThread,
                           ctr_build_string_from_cstring(CTR_DICT_TOSTRING),
                           &ctr_thread_to_string);

  ctr_internal_object_add_property(
      CtrStdWorld, ctr_build_string_from_cstring("Thread"), CtrStdThread, 0);

  CtrStdReflect_cons = ctr_internal_create_object(CTR_OBJECT_TYPE_OTOBJECT);
  ctr_set_link_all(CtrStdReflect_cons, CtrStdObject);

  ctr_internal_create_func(CtrStdReflect_cons,
                           ctr_build_string_from_cstring("of:"),
                           &ctr_reflect_cons_of);
  ctr_internal_create_func(CtrStdReflect_cons,
                           ctr_build_string_from_cstring("value"),
                           &ctr_reflect_cons_value);
  ctr_internal_create_func(CtrStdReflect_cons,
                           ctr_build_string_from_cstring("toString"),
                           &ctr_reflect_cons_str);

  ctr_internal_object_add_property(CtrStdWorld,
                                   ctr_build_string_from_cstring("%ctor"),
                                   CtrStdReflect_cons, 0);
  CtrStdReflect_cons->info.sticky = 1;

#if withInjectNative
  CtrStdInject = CtrStdObject;
  CtrStdInject = ctr_inject_make(NULL, NULL);
  ctr_internal_object_add_property(
      CtrStdWorld, ctr_build_string_from_cstring("Inject"), CtrStdInject, 0);
  ctr_internal_create_func(CtrStdInject,
                           ctr_build_string_from_cstring("newWithOutputMode:"),
                           &ctr_inject_make);
  ctr_internal_create_func(CtrStdInject, ctr_build_string_from_cstring("new"),
                           &ctr_inject_make);
  ctr_internal_create_func(CtrStdInject,
                           ctr_build_string_from_cstring("definedFunctions"),
                           &ctr_inject_defined_functions);
  ctr_internal_create_func(CtrStdInject,
                           ctr_build_string_from_cstring("definedFunctions:"),
                           &ctr_inject_defined_functions);
  ctr_internal_create_func(CtrStdInject,
                           ctr_build_string_from_cstring("compile:"),
                           &ctr_inject_compile);
  ctr_internal_create_func(CtrStdInject,
                           ctr_build_string_from_cstring("run:arguments:"),
                           &ctr_inject_run);
  ctr_internal_create_func(
      CtrStdInject, ctr_build_string_from_cstring("run:arguments:function:"),
      &ctr_inject_run_named);
  ctr_internal_create_func(CtrStdInject,
                           ctr_build_string_from_cstring("symbol:"),
                           &ctr_inject_get_symbol);
  ctr_internal_create_func(CtrStdInject,
                           ctr_build_string_from_cstring("addLibraryPath:"),
                           &ctr_inject_add_libp);
  ctr_internal_create_func(CtrStdInject,
                           ctr_build_string_from_cstring("libraryPaths"),
                           &ctr_inject_get_libp);
  ctr_internal_create_func(CtrStdInject,
                           ctr_build_string_from_cstring("addIncludePath:"),
                           &ctr_inject_add_inclp);
  ctr_internal_create_func(CtrStdInject,
                           ctr_build_string_from_cstring("includePaths"),
                           &ctr_inject_get_inclp);
  ctr_internal_create_func(CtrStdInject,
                           ctr_build_string_from_cstring("linkInLibrary:"),
                           &ctr_inject_add_lib);
  ctr_internal_create_func(CtrStdInject,
                           ctr_build_string_from_cstring("errorHandler:"),
                           &ctr_inject_set_error_handler);
  ctr_internal_create_func(CtrStdInject,
                           ctr_build_string_from_cstring("outputTo:"),
                           &ctr_inject_generate_output);
  ctr_internal_create_func(CtrStdInject,
                           ctr_build_string_from_cstring("destruct"),
                           &ctr_inject_finish);
#endif // withInjectNative

  static ctr_object ctr_dummy_import;
  static ctr_interfaces ifs;
  if (!with_stdlib) {
    ctr_dummy_import.interfaces = &ifs;
    ctr_set_link_all(&ctr_dummy_import, CtrStdObject);
  }
  // importlib
  CtrStdImportLib =
      ctr_importlib_begin(CtrStdObject, NULL) ?: &ctr_dummy_import;

  // Fiber
  ctr_fiber_begin_init();
  initiailize_base_extensions();
  promise_begin();

#if withCTypesNative
  // FFI
  ctr_ffi_begin();
#endif

  /* Other objects */
  CtrStdBreak = ctr_internal_create_object(CTR_OBJECT_TYPE_OTOBJECT);
  CtrStdContinue = ctr_internal_create_object(CTR_OBJECT_TYPE_OTOBJECT);
  CtrStdExit = ctr_internal_create_object(CTR_OBJECT_TYPE_OTOBJECT);
  CtrStdBreak->info.sticky = 1;
  CtrStdContinue->info.sticky = 1;
  CtrStdExit->info.sticky = 1;

  /* Construct the whitelist for eval */
  ctr_secpro_eval_whitelist[0] = ctr_array_push;
  ctr_secpro_eval_whitelist[1] = ctr_map_new;
  ctr_secpro_eval_whitelist[2] = ctr_map_put;
  ctr_secpro_eval_whitelist[3] = ctr_array_new;
  ctr_secpro_eval_whitelist[4] = ctr_nil_to_string;
  ctr_secpro_eval_whitelist[5] = ctr_bool_to_string;
  ctr_secpro_eval_whitelist[6] = ctr_number_to_string;
  ctr_secpro_eval_whitelist[7] = ctr_string_to_string;
  ctr_secpro_eval_whitelist[8] = ctr_array_new_and_push;

  /* relax eval a bit */
  ctr_secpro_eval_whitelist[9] = ctr_number_add;
  ctr_secpro_eval_whitelist[10] = ctr_number_minus;
  ctr_secpro_eval_whitelist[11] = ctr_number_divide;
  ctr_secpro_eval_whitelist[12] = ctr_number_multiply;
  ctr_secpro_eval_whitelist[13] = ctr_number_sqrt;
  ctr_secpro_eval_whitelist[14] = ctr_number_pow;

  /* maximum number of connections to accept (in total) */
  ctr_accept_n_connections = 0;

  /* load native modules */
  ctr_load_required_native_modules();
}

ctr_object *
ctr_internal_get_responder_from_interfaces(ctr_object *receiverObject,
                                           char *message, int vlen) {
  ctr_object *methodObject = NULL;
#pragma omp parallel default(none),                                            \
    shared(receiverObject, vlen, message, methodObject)
  {
    int abort = 0;
#pragma omp for
    for (int i = 0; i < receiverObject->interfaces->count; i++) {
      if (!abort) {
        // printf("interface %d of %p for message %.*s\n", i, receiverObject,
        // vlen, message);
        ctr_object *meth = ctr_get_responder(receiverObject->interfaces->ifs[i],
                                             message, vlen);
        if (meth) {
#pragma omp critical(methodObject)
          methodObject = meth;
          abort = 1;
#pragma omp flush(abort)
        }
      }
    }
  }
  return methodObject;
}

ctr_object *ctr_get_responder(ctr_object *receiverObject, char *message,
                              long vlen) {
  if (receiverObject->info.type == 0 && receiverObject->interfaces == NULL) {
    return NULL;
  }
  ctr_object *methodObject;
  ctr_object *searchObject;
  methodObject = NULL;
  searchObject = receiverObject;
  int toParent = 0;
  ctr_object *me;
  ctr_object *msg = NULL;
  if (vlen > 1 && message[0] == '`') {
    me = ctr_internal_object_find_property(
        ctr_contexts[ctr_context_id],
        ctr_build_string_from_cstring(ctr_clex_keyword_me), 0);
    if (searchObject == me) {
      toParent = 1;
      message = message + 1;
      vlen--;
    }
  }
  msg = ctr_build_string(message, vlen);
  msg->info.sticky =
      1; /* prevent message from being swept, no need to free(), GC will do */
  while (!methodObject) {
    if (!searchObject)
      break;
    methodObject = ctr_internal_object_find_property(searchObject, msg, 1);
    if (methodObject && toParent) {
      toParent = 0;
      methodObject = NULL;
    }
    if (methodObject)
      break;
    if (!searchObject->interfaces->link)
      break;
    searchObject = searchObject->interfaces->link;
  }
  ctr_heap_free(msg);
  if (!methodObject) {
    searchObject = receiverObject;
    while (!methodObject) {
      if (!searchObject)
        break;
      methodObject = ctr_internal_get_responder_from_interfaces(searchObject,
                                                                message, vlen);
      if (methodObject)
        break;
      if (!searchObject->interfaces->link)
        break;
      searchObject = searchObject->interfaces->link;
    }
  }
  return methodObject;
}

/**
 * @internal
 *
 * CTRTransformArgumentList2CTRList
 *
 * Converts an argumentList to a citron list
 */
ctr_object *ctr_internal_argumentptr2tuple(ctr_argument *argumentList) {
  ctr_object *ret = ctr_array_new(CtrStdArray, NULL);
  if (!argumentList)
    return ret;
  while (argumentList->object) {
    ctr_array_push(ret, argumentList);
    argumentList = argumentList->next;
    if (!argumentList)
      break;
  }
  return ret;
}

ctr_object *ctr_get_appropriate_catch_all(char *message, long vlen,
                                          int argCount) {
  if (vlen == 1)
    return ctr_build_string("respondTo:and:", 14);
  if (argCount == -1) {
    char *msg = message, *msg_end = msg + vlen;
    int count = argCount;
    while (msg <= msg_end) {
      size_t plen = 0;
      while (*(msg++) == ':')
        plen++;
      if (plen == 1)
        continue;
      count += plen > 6 ? plen / 3 : 1;
      while (msg < msg_end && *(msg++) != ':')
        ;
      if (msg < msg_end && *(msg - 1) == ':')
        count++;
    }
    argCount = count;
  }
  switch (argCount) {
  case 0:
    return ctr_build_string_from_cstring("respondTo:");
  default:
  case 1:
    return ctr_build_string_from_cstring("respondTo:and:");
  case 2:
    return ctr_build_string_from_cstring("respondTo:and:and:");
  case 3:
    return ctr_build_string_from_cstring("respondTo:and:and:and:");
  case 4:
    return ctr_build_string_from_cstring("respondTo:and:and:and:and:");
  case 5:
    return ctr_build_string_from_cstring("respondTo:and:and:and:and:and:");
  }
}

#if CTR_TAGS_ONLY
struct ctr_internal_tag_ll {
  void *value;
  struct ctr_internal_tag_ll *next;
};

#include "rforeach.h"

ctr_object *ctr_array_internal_zip(ctr_object *, ctr_argument *);
ctr_object *ctr_ast_each(ctr_object *, ctr_argument *);
ctr_object *ctr_ast_eachv(ctr_object *, ctr_argument *);
ctr_object *ctr_ast_evaluate(ctr_object *, ctr_argument *);
ctr_object *ctr_ast_flex(ctr_object *, ctr_argument *);
ctr_object *ctr_ast_get_block(ctr_object *, ctr_argument *);
ctr_object *ctr_ast_get_mod(ctr_object *, ctr_argument *);
ctr_object *ctr_ast_get_type(ctr_object *, ctr_argument *);
ctr_object *ctr_ast_get_value(ctr_object *, ctr_argument *);
ctr_object *ctr_ast_insert_nth(ctr_object *, ctr_argument *);
ctr_object *ctr_ast_instrcount(ctr_object *, ctr_argument *);
ctr_object *ctr_ast_lexbuf(ctr_object *, ctr_argument *);
ctr_object *ctr_ast_lexline(ctr_object *, ctr_argument *);
ctr_object *ctr_ast_lexpos(ctr_object *, ctr_argument *);
ctr_object *ctr_ast_lexputback(ctr_object *, ctr_argument *);
ctr_object *ctr_ast_lexskip(ctr_object *, ctr_argument *);
ctr_object *ctr_ast_lexstring(ctr_object *, ctr_argument *);
ctr_object *ctr_ast_lextoken(ctr_object *, ctr_argument *);
ctr_object *ctr_ast_marshal(ctr_object *, ctr_argument *);
ctr_object *ctr_ast_nth(ctr_object *, ctr_argument *);
ctr_object *ctr_ast_parse(ctr_object *, ctr_argument *);
ctr_object *ctr_ast_set_mod(ctr_object *, ctr_argument *);
ctr_object *ctr_ast_set_nth(ctr_object *, ctr_argument *);
ctr_object *ctr_ast_set_type(ctr_object *, ctr_argument *);
ctr_object *ctr_ast_set_value(ctr_object *, ctr_argument *);
ctr_object *ctr_ast_stringify(ctr_object *, ctr_argument *);
ctr_object *ctr_ast_tostring(ctr_object *, ctr_argument *);
ctr_object *ctr_ast_with_value(ctr_object *, ctr_argument *);
ctr_object *ctr_block_assign(ctr_object *, ctr_argument *);
ctr_object *ctr_block_catch(ctr_object *, ctr_argument *);
ctr_object *ctr_block_catch_type(ctr_object *, ctr_argument *);
ctr_object *ctr_block_error(ctr_object *, ctr_argument *);
ctr_object *ctr_block_forever(ctr_object *, ctr_argument *);
ctr_object *ctr_block_let(ctr_object *, ctr_argument *);
ctr_object *ctr_block_letast(ctr_object *, ctr_argument *);
ctr_object *ctr_block_runIt(ctr_object *, ctr_argument *);
ctr_object *ctr_block_runall(ctr_object *, ctr_argument *);
ctr_object *ctr_block_set(ctr_object *, ctr_argument *);
ctr_object *ctr_block_specialise(ctr_object *, ctr_argument *);
ctr_object *ctr_block_to_string(ctr_object *, ctr_argument *);
ctr_object *ctr_block_while_false(ctr_object *, ctr_argument *);
ctr_object *ctr_block_while_true(ctr_object *, ctr_argument *);
ctr_object *ctr_bool_and(ctr_object *, ctr_argument *);
ctr_object *ctr_bool_assign(ctr_object *, ctr_argument *);
ctr_object *ctr_bool_break(ctr_object *, ctr_argument *);
ctr_object *ctr_bool_continue(ctr_object *, ctr_argument *);
ctr_object *ctr_bool_either_or(ctr_object *, ctr_argument *);
ctr_object *ctr_bool_eq(ctr_object *, ctr_argument *);
ctr_object *ctr_bool_flip(ctr_object *, ctr_argument *);
ctr_object *ctr_bool_if_false(ctr_object *, ctr_argument *);
ctr_object *ctr_bool_if_tf(ctr_object *, ctr_argument *);
ctr_object *ctr_bool_if_true(ctr_object *, ctr_argument *);
ctr_object *ctr_bool_neq(ctr_object *, ctr_argument *);
ctr_object *ctr_bool_nor(ctr_object *, ctr_argument *);
ctr_object *ctr_bool_not(ctr_object *, ctr_argument *);
ctr_object *ctr_bool_or(ctr_object *, ctr_argument *);
ctr_object *ctr_bool_to_number(ctr_object *, ctr_argument *);
ctr_object *ctr_bool_to_string(ctr_object *, ctr_argument *);
ctr_object *ctr_bool_xor(ctr_object *, ctr_argument *);
ctr_object *ctr_coro_error(ctr_object *, ctr_argument *);
ctr_object *ctr_coro_isrunning(ctr_object *, ctr_argument *);
ctr_object *ctr_coro_make(ctr_object *, ctr_argument *);
ctr_object *ctr_coro_new(ctr_object *, ctr_argument *);
ctr_object *ctr_coro_resume(ctr_object *, ctr_argument *);
ctr_object *ctr_coro_state(ctr_object *, ctr_argument *);
ctr_object *ctr_coro_yield(ctr_object *, ctr_argument *);
ctr_object *ctr_exception_getinfo(ctr_object *, ctr_argument *);
ctr_object *ctr_get_last_trace(ctr_object *, ctr_argument *);
ctr_object *ctr_get_last_trace_stringified(ctr_object *, ctr_argument *);
ctr_object *ctr_nil_assign(ctr_object *, ctr_argument *);
ctr_object *ctr_nil_is_nil(ctr_object *, ctr_argument *);
ctr_object *ctr_nil_to_boolean(ctr_object *, ctr_argument *);
ctr_object *ctr_nil_to_number(ctr_object *, ctr_argument *);
ctr_object *ctr_nil_to_string(ctr_object *, ctr_argument *);
ctr_object *ctr_number_abs(ctr_object *, ctr_argument *);
ctr_object *ctr_number_add(ctr_object *, ctr_argument *);
ctr_object *ctr_number_and(ctr_object *, ctr_argument *);
ctr_object *ctr_number_assign(ctr_object *, ctr_argument *);
ctr_object *ctr_number_atan(ctr_object *, ctr_argument *);
ctr_object *ctr_number_between(ctr_object *, ctr_argument *);
ctr_object *ctr_number_ceil(ctr_object *, ctr_argument *);
ctr_object *ctr_number_cos(ctr_object *, ctr_argument *);
ctr_object *ctr_number_dec(ctr_object *, ctr_argument *);
ctr_object *ctr_number_div(ctr_object *, ctr_argument *);
ctr_object *ctr_number_divide(ctr_object *, ctr_argument *);
ctr_object *ctr_number_eq(ctr_object *, ctr_argument *);
ctr_object *ctr_number_even(ctr_object *, ctr_argument *);
ctr_object *ctr_number_exp(ctr_object *, ctr_argument *);
ctr_object *ctr_number_factorial(ctr_object *, ctr_argument *);
ctr_object *ctr_number_floor(ctr_object *, ctr_argument *);
ctr_object *ctr_number_higherEqThan(ctr_object *, ctr_argument *);
ctr_object *ctr_number_higherThan(ctr_object *, ctr_argument *);
ctr_object *ctr_number_inc(ctr_object *, ctr_argument *);
ctr_object *ctr_number_log(ctr_object *, ctr_argument *);
ctr_object *ctr_number_lowerEqThan(ctr_object *, ctr_argument *);
ctr_object *ctr_number_lowerThan(ctr_object *, ctr_argument *);
ctr_object *ctr_number_max(ctr_object *, ctr_argument *);
ctr_object *ctr_number_min(ctr_object *, ctr_argument *);
ctr_object *ctr_number_minus(ctr_object *, ctr_argument *);
ctr_object *ctr_number_modulo(ctr_object *, ctr_argument *);
ctr_object *ctr_number_mul(ctr_object *, ctr_argument *);
ctr_object *ctr_number_multiply(ctr_object *, ctr_argument *);
ctr_object *ctr_number_negate(ctr_object *, ctr_argument *);
ctr_object *ctr_number_negative(ctr_object *, ctr_argument *);
ctr_object *ctr_number_neq(ctr_object *, ctr_argument *);
ctr_object *ctr_number_odd(ctr_object *, ctr_argument *);
ctr_object *ctr_number_or(ctr_object *, ctr_argument *);
ctr_object *ctr_number_positive(ctr_object *, ctr_argument *);
ctr_object *ctr_number_pow(ctr_object *, ctr_argument *);
ctr_object *ctr_number_qualify(ctr_object *, ctr_argument *);
ctr_object *ctr_number_respond_to(ctr_object *, ctr_argument *);
ctr_object *ctr_number_round(ctr_object *, ctr_argument *);
ctr_object *ctr_number_shl(ctr_object *, ctr_argument *);
ctr_object *ctr_number_shr(ctr_object *, ctr_argument *);
ctr_object *ctr_number_sin(ctr_object *, ctr_argument *);
ctr_object *ctr_number_sqrt(ctr_object *, ctr_argument *);
ctr_object *ctr_number_tan(ctr_object *, ctr_argument *);
ctr_object *ctr_number_times(ctr_object *, ctr_argument *);
ctr_object *ctr_number_to_boolean(ctr_object *, ctr_argument *);
ctr_object *ctr_number_to_byte(ctr_object *, ctr_argument *);
ctr_object *ctr_number_to_step(ctr_object *, ctr_argument *);
ctr_object *ctr_number_to_step_do(ctr_object *, ctr_argument *);
ctr_object *ctr_number_to_string(ctr_object *, ctr_argument *);
ctr_object *ctr_number_uint_binrep(ctr_object *, ctr_argument *);
ctr_object *ctr_number_xor(ctr_object *, ctr_argument *);
ctr_object *ctr_object_assign(ctr_object *, ctr_argument *);
ctr_object *ctr_object_attr_accessor(ctr_object *, ctr_argument *);
ctr_object *ctr_object_attr_reader(ctr_object *, ctr_argument *);
ctr_object *ctr_object_attr_writer(ctr_object *, ctr_argument *);
ctr_object *ctr_object_ctor(ctr_object *, ctr_argument *);
ctr_object *ctr_object_do(ctr_object *, ctr_argument *);
ctr_object *ctr_object_done(ctr_object *, ctr_argument *);
ctr_object *ctr_object_elvis_op(ctr_object *, ctr_argument *);
ctr_object *ctr_object_equals(ctr_object *, ctr_argument *);
ctr_object *ctr_object_hash(ctr_object *, ctr_argument *);
ctr_object *ctr_object_id(ctr_object *, ctr_argument *);
ctr_object *ctr_object_if_false(ctr_object *, ctr_argument *);
ctr_object *ctr_object_if_tf(ctr_object *, ctr_argument *);
ctr_object *ctr_object_if_true(ctr_object *, ctr_argument *);
ctr_object *ctr_object_inh_check(ctr_object *, ctr_argument *);
ctr_object *ctr_object_inherit(ctr_object *, ctr_argument *);
ctr_object *ctr_object_is_nil(ctr_object *, ctr_argument *);
ctr_object *ctr_object_learn_meaning(ctr_object *, ctr_argument *);
ctr_object *ctr_object_make(ctr_object *, ctr_argument *);
ctr_object *ctr_object_make_hiding(ctr_object *, ctr_argument *);
ctr_object *ctr_object_message(ctr_object *, ctr_argument *);
ctr_object *ctr_object_myself(ctr_object *, ctr_argument *);
ctr_object *ctr_object_on_do(ctr_object *, ctr_argument *);
ctr_object *ctr_object_swap(ctr_object *, ctr_argument *);
ctr_object *ctr_object_to_boolean(ctr_object *, ctr_argument *);
ctr_object *ctr_object_to_number(ctr_object *, ctr_argument *);
ctr_object *ctr_object_to_string(ctr_object *, ctr_argument *);
ctr_object *ctr_object_type(ctr_object *, ctr_argument *);
ctr_object *ctr_string_append(ctr_object *, ctr_argument *);
ctr_object *ctr_string_append_byte(ctr_object *, ctr_argument *);
ctr_object *ctr_string_assign(ctr_object *, ctr_argument *);
ctr_object *ctr_string_at(ctr_object *, ctr_argument *);
ctr_object *ctr_string_byte_at(ctr_object *, ctr_argument *);
ctr_object *ctr_string_bytes(ctr_object *, ctr_argument *);
ctr_object *ctr_string_cadd(ctr_object *, ctr_argument *);
ctr_object *ctr_string_characters(ctr_object *, ctr_argument *);
ctr_object *ctr_string_concat(ctr_object *, ctr_argument *);
ctr_object *ctr_string_contains(ctr_object *, ctr_argument *);
ctr_object *ctr_string_contains_pattern(ctr_object *, ctr_argument *);
ctr_object *ctr_string_count_of(ctr_object *, ctr_argument *);
ctr_object *ctr_string_csub(ctr_object *, ctr_argument *);
ctr_object *ctr_string_dquotes_escape(ctr_object *, ctr_argument *);
ctr_object *ctr_string_ends_with(ctr_object *, ctr_argument *);
ctr_object *ctr_string_eq(ctr_object *, ctr_argument *);
ctr_object *ctr_string_filter(ctr_object *, ctr_argument *);
ctr_object *ctr_string_find_pattern_do(ctr_object *, ctr_argument *);
ctr_object *ctr_string_find_pattern_options_do(ctr_object *, ctr_argument *);
ctr_object *ctr_string_fmap(ctr_object *, ctr_argument *);
ctr_object *ctr_string_format(ctr_object *, ctr_argument *);
ctr_object *ctr_string_format_map(ctr_object *, ctr_argument *);
ctr_object *ctr_string_from_length(ctr_object *, ctr_argument *);
ctr_object *ctr_string_fromto(ctr_object *, ctr_argument *);
ctr_object *ctr_string_hash_with_key(ctr_object *, ctr_argument *);
ctr_object *ctr_string_html_escape(ctr_object *, ctr_argument *);
ctr_object *ctr_string_imap(ctr_object *, ctr_argument *);
ctr_object *ctr_string_index_of(ctr_object *, ctr_argument *);
ctr_object *ctr_string_is_ctor(ctr_object *, ctr_argument *);
ctr_object *ctr_string_is_regex_pcre(ctr_object *, ctr_argument *);
ctr_object *ctr_string_last_index_of(ctr_object *, ctr_argument *);
ctr_object *ctr_string_length(ctr_object *, ctr_argument *);
ctr_object *ctr_string_ltrim(ctr_object *, ctr_argument *);
ctr_object *ctr_string_multiply(ctr_object *, ctr_argument *);
ctr_object *ctr_string_neq(ctr_object *, ctr_argument *);
ctr_object *ctr_string_padding(ctr_object *, ctr_argument *);
ctr_object *ctr_string_padding_left(ctr_object *, ctr_argument *);
ctr_object *ctr_string_padding_right(ctr_object *, ctr_argument *);
ctr_object *ctr_string_put_at(ctr_object *, ctr_argument *);
ctr_object *ctr_string_quotes_escape(ctr_object *, ctr_argument *);
ctr_object *ctr_string_randomize_bytes(ctr_object *, ctr_argument *);
ctr_object *ctr_string_re_index_of(ctr_object *, ctr_argument *);
ctr_object *ctr_string_re_last_index_of(ctr_object *, ctr_argument *);
ctr_object *ctr_string_reg_replace(ctr_object *, ctr_argument *);
ctr_object *ctr_string_replace_with(ctr_object *, ctr_argument *);
ctr_object *ctr_string_reverse(ctr_object *, ctr_argument *);
ctr_object *ctr_string_rtrim(ctr_object *, ctr_argument *);
ctr_object *ctr_string_skip(ctr_object *, ctr_argument *);
ctr_object *ctr_string_slice(ctr_object *, ctr_argument *);
ctr_object *ctr_string_split(ctr_object *, ctr_argument *);
ctr_object *ctr_string_split_re(ctr_object *, ctr_argument *);
ctr_object *ctr_string_starts_with(ctr_object *, ctr_argument *);
ctr_object *ctr_string_to_boolean(ctr_object *, ctr_argument *);
ctr_object *ctr_string_to_byte_array(ctr_object *, ctr_argument *);
ctr_object *ctr_string_to_lower(ctr_object *, ctr_argument *);
ctr_object *ctr_string_to_lower1st(ctr_object *, ctr_argument *);
ctr_object *ctr_string_to_number(ctr_object *, ctr_argument *);
ctr_object *ctr_string_to_string(ctr_object *, ctr_argument *);
ctr_object *ctr_string_to_symbol(ctr_object *, ctr_argument *);
ctr_object *ctr_string_to_upper(ctr_object *, ctr_argument *);
ctr_object *ctr_string_to_upper1st(ctr_object *, ctr_argument *);
ctr_object *ctr_string_trim(ctr_object *, ctr_argument *);
#pragma endregion dummies

#define INITIALISE_TAG_DESCR_HELPER(tag_ptr, inner)                            \
  &(struct ctr_internal_tag_ll) { tag_ptr, inner }
// 256 max
#define INITIALISE_TAG_DESCR(...)                                              \
  R_FOR_EACH(INITIALISE_TAG_DESCR_HELPER, __VA_ARGS__, {})

static struct ctr_internal_tag_ll *ctr_internal_tag_whitelist_list =
    INITIALISE_TAG_DESCR(
        &ctr_array_internal_zip, &ctr_ast_each, &ctr_ast_eachv,
        &ctr_ast_evaluate, &ctr_ast_flex, &ctr_ast_from_node,
        &ctr_ast_get_block, &ctr_ast_get_mod, &ctr_ast_get_type,
        &ctr_ast_get_value, &ctr_ast_insert_nth, &ctr_ast_instrcount,
        &ctr_ast_lexbuf, &ctr_ast_lexline, &ctr_ast_lexpos, &ctr_ast_lexputback,
        &ctr_ast_lexskip, &ctr_ast_lexstring, &ctr_ast_lextoken,
        &ctr_ast_marshal, &ctr_ast_nth, &ctr_ast_parse, &ctr_ast_set_mod,
        &ctr_ast_set_nth, &ctr_ast_set_type, &ctr_ast_set_value,
        &ctr_ast_stringify, &ctr_ast_tostring, &ctr_ast_with_value,
        &ctr_block_assign, &ctr_block_catch, &ctr_block_catch_type,
        &ctr_block_error, &ctr_block_forever, &ctr_block_let, &ctr_block_letast,
        &ctr_block_run, &ctr_block_runIt, &ctr_block_run_here,
        &ctr_block_runall, &ctr_block_set, &ctr_block_specialise,
        &ctr_block_to_string, &ctr_block_while_false, &ctr_block_while_true,
        &ctr_bool_and, &ctr_bool_assign, &ctr_bool_break, &ctr_bool_continue,
        &ctr_bool_either_or, &ctr_bool_eq, &ctr_bool_flip, &ctr_bool_if_false,
        &ctr_bool_if_tf, &ctr_bool_if_true, &ctr_bool_neq, &ctr_bool_nor,
        &ctr_bool_not, &ctr_bool_or, &ctr_bool_to_number, &ctr_bool_to_string,
        &ctr_bool_xor, &ctr_coro_error, &ctr_coro_isrunning, &ctr_coro_make,
        &ctr_coro_new, &ctr_coro_resume, &ctr_coro_state, &ctr_coro_yield,
        &ctr_exception_getinfo, &ctr_get_last_trace, &ctr_get_stack_trace,
        &ctr_invoke_variadic, &ctr_nil_assign, &ctr_nil_is_nil,
        &ctr_nil_to_boolean, &ctr_nil_to_number, &ctr_nil_to_string,
        &ctr_number_abs, &ctr_number_add, &ctr_number_and, &ctr_number_assign,
        &ctr_number_atan, &ctr_number_between, &ctr_number_ceil,
        &ctr_number_cos, &ctr_number_dec, &ctr_number_div, &ctr_number_divide,
        &ctr_number_eq, &ctr_number_even, &ctr_number_exp,
        &ctr_number_factorial, &ctr_number_floor, &ctr_number_higherEqThan,
        &ctr_number_higherThan, &ctr_number_inc, &ctr_number_log,
        &ctr_number_lowerEqThan, &ctr_number_lowerThan, &ctr_number_max,
        &ctr_number_min, &ctr_number_minus, &ctr_number_modulo, &ctr_number_mul,
        &ctr_number_multiply, &ctr_number_negate, &ctr_number_negative,
        &ctr_number_neq, &ctr_number_odd, &ctr_number_or, &ctr_number_positive,
        &ctr_number_pow, &ctr_number_qualify, &ctr_number_respond_to,
        &ctr_number_round, &ctr_number_shl, &ctr_number_shr, &ctr_number_sin,
        &ctr_number_sqrt, &ctr_number_tan, &ctr_number_times,
        &ctr_number_to_boolean, &ctr_number_to_byte, &ctr_number_to_step,
        &ctr_number_to_step_do, &ctr_number_to_string, &ctr_number_uint_binrep,
        &ctr_number_xor, &ctr_object_assign, &ctr_object_attr_accessor,
        &ctr_object_attr_reader, &ctr_object_attr_writer, &ctr_object_ctor,
        &ctr_object_do, &ctr_object_done, &ctr_object_elvis_op,
        &ctr_object_equals, &ctr_object_hash, &ctr_object_id,
        &ctr_object_if_false, &ctr_object_if_tf, &ctr_object_if_true,
        &ctr_object_inh_check, &ctr_object_inherit, &ctr_object_is_nil,
        &ctr_object_learn_meaning, &ctr_object_make, &ctr_object_make_hiding,
        &ctr_object_message, &ctr_object_myself, &ctr_object_on_do,
        &ctr_object_swap, &ctr_object_to_boolean, &ctr_object_to_number,
        &ctr_object_to_string, &ctr_object_type, &ctr_str_count_substr,
        &ctr_string_append, &ctr_string_append_byte, &ctr_string_assign,
        &ctr_string_at, &ctr_string_byte_at, &ctr_string_bytes,
        &ctr_string_cadd, &ctr_string_characters, &ctr_string_concat,
        &ctr_string_contains, &ctr_string_contains_pattern,
        &ctr_string_count_of, &ctr_string_csub, &ctr_string_dquotes_escape,
        &ctr_string_ends_with, &ctr_string_eq, &ctr_string_filter,
        &ctr_string_find_pattern_do, &ctr_string_find_pattern_options_do,
        &ctr_string_fmap, &ctr_string_format, &ctr_string_format_map,
        &ctr_string_from_length, &ctr_string_fromto, &ctr_string_hash_with_key,
        &ctr_string_html_escape, &ctr_string_imap, &ctr_string_index_of,
        &ctr_string_is_ctor, &ctr_string_is_regex_pcre,
        &ctr_string_last_index_of, &ctr_string_length, &ctr_string_ltrim,
        &ctr_string_multiply, &ctr_string_neq, &ctr_string_padding,
        &ctr_string_padding_left, &ctr_string_padding_right, &ctr_string_put_at,
        &ctr_string_quotes_escape, &ctr_string_randomize_bytes,
        &ctr_string_re_index_of, &ctr_string_re_last_index_of,
        &ctr_string_reg_replace, &ctr_string_replace_with, &ctr_string_reverse,
        &ctr_string_rtrim, &ctr_string_skip, &ctr_string_slice,
        &ctr_string_split, &ctr_string_split_re, &ctr_string_starts_with,
        &ctr_string_to_boolean, &ctr_string_to_byte_array, &ctr_string_to_lower,
        &ctr_string_to_lower1st, &ctr_string_to_number, &ctr_string_to_string,
        &ctr_string_to_symbol, &ctr_string_to_upper, &ctr_string_to_upper1st,
        &ctr_string_trim, &ctr_object_learn_meaning);

int ctr_internal_check_tag_whitelisted(void *ptr) {
  struct ctr_internal_tag_ll *node = ctr_internal_tag_whitelist_list;
  while (node) {
    if (node->value == ptr)
      return 1;
    node = node->next;
  }
  return 0;
}

void ctr_internal_tag_whitelist(void *ptr) {
  if (!ptr)
    return;
  if (ctr_internal_check_tag_whitelisted(ptr))
    return;
  struct ctr_internal_tag_ll *node = ctr_heap_allocate(sizeof(*node));
  node->value = ptr;
  node->next = ctr_internal_tag_whitelist_list;
  ctr_internal_tag_whitelist_list = node;
}
#endif

char *msgName__ = NULL;
long msgLen__ = 0;
/**
 * @internal
 *
 * CTRMessageSend
 *
 * Sends a message to a receiver object.
 */
ctr_object *ctr_send_message_blocking(ctr_object *receiverObject, char *message,
                                      long vlen, ctr_argument *argumentList) {
  if (unlikely(receiverObject != CtrStdReflect && ctr_instrument)) {
    ctr_instrument = 0;
    ctr_object *ctr_instrumentor_func;
    if (ctr_global_instrum) {
      ctr_instrumentor_func = ctr_global_instrum;
      goto skip_intern;
    }
    ctr_instrumentor_func = ctr_internal_object_find_property_with_hash(
        ctr_instrumentor_funcs, receiverObject,
        ctr_send_message_blocking(receiverObject, "iHash", 5, NULL)
            ->value.nvalue,
        0);
  skip_intern:;
    if (unlikely(ctr_instrumentor_func)) {
      ctr_argument *blkargs = ctr_heap_allocate(sizeof(ctr_argument));
      blkargs->object = receiverObject;
      blkargs->next = ctr_heap_allocate(sizeof(ctr_argument));
      blkargs->next->object = ctr_build_string(message, vlen);
      blkargs->next->next = ctr_heap_allocate(sizeof(ctr_argument));
      blkargs->next->next->object =
          ctr_internal_argumentptr2tuple(argumentList);
      ctr_object *result =
          ctr_block_run(ctr_instrumentor_func, blkargs, ctr_instrumentor_func);
      ctr_instrument = 1;
      if (result == ctr_instrumentor_func)
        goto no_instrum;
      CTR_THREAD_UNLOCK();
      return result;
    }
    ctr_instrument = 1;
  }
no_instrum:;
  CTR_THREAD_LOCK();
  char toParent = 0;
  int i = 0;
  char messageApproved = 0;
  ctr_object *me;
  ctr_object *methodObject;
  ctr_object *searchObject;
  ctr_object *returnValue;
  ctr_argument *argCounter;
  ctr_argument *mesgArgument;
  ctr_object *result;
  ctr_object *(*funct)(ctr_object * receiverObject,
                       ctr_argument * argumentList);
  int argCount;
  if (CtrStdFlow != NULL) {
    msgName__ = message;
    msgLen__ = vlen;
    CTR_THREAD_UNLOCK();
    return CtrStdNil; /* Error mode, ignore subsequent messages until resolved.
                       */
  }
  if (ctr_command_security_profile & CTR_SECPRO_COUNTDOWN) {
    if (ctr_command_tick > ctr_command_maxtick) {
      printf("This program has exceeded the maximum number of messages.\n");
      exit(1);
    }
    ctr_command_tick += 1;
  }
  methodObject = ctr_get_responder(receiverObject, message, vlen);
  if (!methodObject) {
    argCounter = argumentList;
    argCount = 0;
    while (argCounter && argCounter->next && argCount < 4) {
      argCounter = argCounter->next;
      argCount++;
    }
    ctr_object *catch_all_s =
        ctr_get_appropriate_catch_all(message, vlen, argCount);
    char *catch_all = catch_all_s->value.svalue->value;
    size_t catch_all_v = catch_all_s->value.svalue->vlen;
    if (vlen == catch_all_v && message[9] == ':' &&
        strcmp(message, catch_all) == 0) {
      ctr_object *s = CtrStdString;
      if (receiverObject->lexical_name)
        s = receiverObject->lexical_name;
      CtrStdFlow = ctr_format_str(
          "E" CTR_DICT_RESPOND_TO_AND
          " calls itself with initiator ‘%.%s’ for object named ‘%S’",
          msgLen__, msgName__, s);
      CTR_THREAD_UNLOCK();
      msgName__ = message;
      msgLen__ = vlen;
      return receiverObject;
    }
    mesgArgument = (ctr_argument *)ctr_heap_allocate(sizeof(ctr_argument));
    mesgArgument->object = ctr_build_string(message, vlen);
    mesgArgument->next = argumentList;
    CTR_THREAD_UNLOCK();
    returnValue = ctr_send_message_blocking(receiverObject, catch_all,
                                            catch_all_v, mesgArgument);
    ctr_heap_free(mesgArgument);
    msgName__ = message;
    msgLen__ = vlen;
    if (receiverObject->info.chainMode == 1)
      return receiverObject;
    return returnValue;
  }
  if (methodObject->info.type == CTR_OBJECT_TYPE_OTNATFUNC) {
    funct = methodObject->value.fvalue;
#ifdef EVALSECURITY
    if (ctr_command_security_profile & CTR_SECPRO_EVAL) {
      messageApproved = 0;
      for (i = 0; i < 15; i++) {
        if (funct == ctr_secpro_eval_whitelist[i]) {
          messageApproved = 1;
          break;
        }
      }
      if (!messageApproved) {
        ctr_object *msg = ctr_build_string(message, vlen);
        printf("Native message not allowed in eval %s.\n",
               msg->value.svalue->value);
        ctr_print_stack_trace();
        exit(1);
      }
    }
#endif // EVALSECURITY

    CTR_THREAD_UNLOCK();

#if CTR_TAGS_ONLY
    if (!ctr_internal_check_tag_whitelisted(funct))
      return receiverObject;
#endif
    result = funct(receiverObject, argumentList);
  } else if (methodObject->info.type == CTR_OBJECT_TYPE_OTBLOCK) {
#ifdef EVALSECURITY
    if (ctr_command_security_profile & CTR_SECPRO_EVAL) {
      printf("Custom message not allowed in eval.\n");
      ctr_print_stack_trace();
      exit(1);
    }
#endif // EVALSECURITY
    CTR_THREAD_UNLOCK();
#if CTR_TAGS_ONLY
    if (!ctr_internal_check_tag_whitelisted(methodObject))
      return receiverObject;
#endif
    result = ctr_block_run(methodObject, argumentList, receiverObject);
  }
  msgName__ = message;
  msgLen__ = vlen;
  if (receiverObject->info.chainMode == 1)
    return receiverObject;
  return result; // Normally cascade down to native functions, so get the return
                 // type
}

__attribute__((always_inline)) ctr_object *
ctr_send_message(ctr_object *receiverObject, char *message, long vlen,
                 ctr_argument *argumentList) {
  if (receiverObject->info.asyncMode) {
    receiverObject->info.asyncMode = 0;
    return ctr_send_message_async(receiverObject, message, vlen, argumentList);
  }
  return ctr_send_message_blocking(receiverObject, message, vlen, argumentList);
}

ctr_object *ctr_send_message_async(ctr_object *receiverObject, char *message,
                                   long vlen, ctr_argument *argumentList) {
  ctr_object *methodObject = ctr_get_responder(receiverObject, message, vlen);
  if (!methodObject && receiverObject->interfaces->link == CtrStdPromise) {
    ctr_argument *args = ctr_heap_allocate(sizeof(*args));
    args->next = argumentList;
    args->object = ctr_build_string(message, vlen);
    return ctr_promise_pass_message(receiverObject, args);
  }
  if (!methodObject) {
    CtrStdFlow = ctr_format_str("ENo such async responder '%s'", message);
    return CtrStdNil;
  }
  return ctr_promise_make(CtrStdPromise, methodObject, argumentList,
                          receiverObject);
}

/**
 * @internal
 *
 * CTRValueAssignment
 *
 * Assigns a value to a variable in the current context.
 */
ctr_object *ctr_assign_value(ctr_object *key, ctr_object *o) {
  ctr_object *object = o;
  if (CtrStdFlow)
    return CtrStdNil;
  key->info.sticky = 0;
  int byref = key->value.svalue->vlen > 1 && key->value.svalue->value[0] == '&';
  if (byref) {
    key->value.svalue->value++;
    key->value.svalue->vlen--;
  } else
    switch (o->info.type) {
    case CTR_OBJECT_TYPE_OTBOOL:
      object = ctr_build_bool(o->value.bvalue);
      break;
    case CTR_OBJECT_TYPE_OTNUMBER:
      object = ctr_build_number_from_float(o->value.nvalue);
      break;
    case CTR_OBJECT_TYPE_OTSTRING:
      object = ctr_build_string(o->value.svalue->value, o->value.svalue->vlen);
      break;
    case CTR_OBJECT_TYPE_OTNIL:
    case CTR_OBJECT_TYPE_OTNATFUNC:
    case CTR_OBJECT_TYPE_OTOBJECT:
    case CTR_OBJECT_TYPE_OTEX:
    case CTR_OBJECT_TYPE_OTMISC:
    case CTR_OBJECT_TYPE_OTARRAY:
    case CTR_OBJECT_TYPE_OTBLOCK:
      object = o;
      break;
    }
  ctr_set(key, object);
  return object;
}

ctr_object *ctr_const_assign_value(ctr_object *key, ctr_object *o,
                                   ctr_object *context) {
  if (CtrStdFlow)
    return CtrStdNil;
  ctr_open_context();
  ctr_object *object = o;
  key->info.sticky = 0;
  ctr_set(key, object);
  ctr_internal_object_add_property(ctr_contexts[ctr_context_id],
                                   ctr_build_string_from_cstring("context"),
                                   context, 0);
  context = ctr_contexts[ctr_context_id];
  ctr_close_context();
  return context;
}

/**
 * @internal
 *
 * CTRAssignValueObject
 *
 * Assigns a value to a property of an object.
 */
ctr_object *ctr_assign_value_to_my(ctr_object *key, ctr_object *o) {
  ctr_object *object = o;
  ctr_object *my = ctr_find(&CTR_CLEX_KW_ME);
  if (CtrStdFlow)
    return CtrStdNil;
  key->info.sticky = 0;
  int byref = key->value.svalue->vlen > 1 && key->value.svalue->value[0] == '&';
  if (byref) {
    key->value.svalue->value++;
    key->value.svalue->vlen--;
  } else
    switch (o->info.type) {
    case CTR_OBJECT_TYPE_OTBOOL:
      object = ctr_build_bool(o->value.bvalue);
      break;
    case CTR_OBJECT_TYPE_OTNUMBER:
      object = ctr_build_number_from_float(o->value.nvalue);
      break;
    case CTR_OBJECT_TYPE_OTSTRING:
      object = ctr_build_string(o->value.svalue->value, o->value.svalue->vlen);
      break;
    case CTR_OBJECT_TYPE_OTNIL:
    case CTR_OBJECT_TYPE_OTNATFUNC:
    case CTR_OBJECT_TYPE_OTOBJECT:
    case CTR_OBJECT_TYPE_OTEX:
    case CTR_OBJECT_TYPE_OTMISC:
    case CTR_OBJECT_TYPE_OTARRAY:
    case CTR_OBJECT_TYPE_OTBLOCK:
      object = o;
      break;
    }
  ctr_internal_object_set_property(my, key, object, 0);
  return object;
}

/**
 * @internal
 *
 * CTRAssignValueObjectLocal
 *
 * Assigns a value to a local of an object.
 */
ctr_object *ctr_assign_value_to_local(ctr_object *key, ctr_object *o) {
  ctr_object *object = o;
  ctr_object *context;
  if (CtrStdFlow)
    return CtrStdNil;
  context = ctr_contexts[ctr_context_id];
  key->info.sticky = 0;
  int byref = key->value.svalue->vlen > 1 && key->value.svalue->value[0] == '&';
  if (byref) {
    key->value.svalue->value++;
    key->value.svalue->vlen--;
  } else
    switch (o->info.type) {
    case CTR_OBJECT_TYPE_OTBOOL:
      object = ctr_build_bool(o->value.bvalue);
      break;
    case CTR_OBJECT_TYPE_OTNUMBER:
      object = ctr_build_number_from_float(o->value.nvalue);
      break;
    case CTR_OBJECT_TYPE_OTSTRING:
    case CTR_OBJECT_TYPE_OTNIL:
    case CTR_OBJECT_TYPE_OTNATFUNC:
    case CTR_OBJECT_TYPE_OTOBJECT:
    case CTR_OBJECT_TYPE_OTEX:
    case CTR_OBJECT_TYPE_OTMISC:
    case CTR_OBJECT_TYPE_OTARRAY:
    case CTR_OBJECT_TYPE_OTBLOCK:
      object = o;
      break;
    }
  ctr_internal_object_set_property(context, key, object, 0);
  return object;
}

/**
 * @internal
 *
 * CTRAssignValueObjectLocalByRef
 *
 * Assigns a value to a local of an object.
 * Always assigns by reference.
 */
ctr_object *ctr_assign_value_to_local_by_ref(ctr_object *key, ctr_object *o) {
  ctr_object *object = NULL;
  ctr_object *context;
  if (CtrStdFlow)
    return CtrStdNil;
  context = ctr_contexts[ctr_context_id];
  key->info.sticky = 0;
  object = o;
  ctr_internal_object_set_property(context, key, object, 0);
  return object;
}

ctr_object *ctr_hand_value_to_global(ctr_object *key, ctr_object *o) {
  ctr_object *object = NULL;
  ctr_object *context;
  if (CtrStdFlow)
    return CtrStdNil;
  context = ctr_contexts[0];
  key->info.sticky = 0;
  object = o;
  ctr_internal_object_set_property(context, key, object, 0);
  return object;
}

static void ctr_linkstr() {
  CTR_CLEX_KW_ME.interfaces->link = CtrStdString;
  CTR_CLEX_KW_THIS.interfaces->link = CtrStdString;
  CTR_CLEX_US.interfaces->link = CtrStdString;
}

void ctr_set_link_all(ctr_object *what, ctr_object *to) {
  what->interfaces->link = to;
  if (!what->interfaces->ifs)
    return;
  int count = to->interfaces->count;
  what->interfaces->count = count;
  what->interfaces->ifs =
      what->info.shared
          ? ctr_heap_allocate_shared(sizeof(ctr_object *) * (count + 1))
          : ctr_heap_allocate(sizeof(ctr_object *) * (count + 1));
  // for (int i = 0; i < count; i++)
  memcpy(what->interfaces->ifs, to->interfaces->ifs,
         sizeof(ctr_object *) * count);
}

int ctr_reflect_is_linked_to_(ctr_argument *);

ctr_overload_set *ctr_internal_next_bucket(ctr_overload_set *set,
                                           ctr_argument *arg) {
  if (!set)
    return NULL;
  if (!arg)
    return set;
  if (!arg->object)
    return set;
  ctr_argument after = {NULL, NULL}, refl = {arg->object, &after};
  for (int i = 0; i < set->bucket_count; i++) {
    after.object = set->sub_buckets[i]->this_terminating_bucket;
    refl.object = arg->object;
    if (ctr_reflect_is_linked_to_(&refl)) {
      return set->sub_buckets[i];
    }
  }
  return NULL;
}

ctr_object *ctr_internal_find_overload(ctr_object *original,
                                       ctr_argument *argList) {
  ctr_overload_set *set = original->overloads;
  if (!set->bucket_count)
    return original;
  ctr_argument *arg;
  ctr_overload_set *overload, *ans = NULL;
  int complete = 0;
  for (arg = argList, overload = set;;) {
    if (!arg) {
      complete = 1;
      break;
    }
    if (!overload)
      break;
    if (!overload->bucket_count)
      break;
    ans = overload;
    overload = ctr_internal_next_bucket(overload, arg);
    arg = arg->next;
  }
  if (overload)
    ans = overload;
  if (complete && ans->this_terminating_value) {
    ctr_object *res = ans->this_terminating_value;
    return res;
  }
  return original;
}

ctr_object **get_CtrStdWorld() { return &CtrStdWorld; }

ctr_object **get_CtrStdObject() { return &CtrStdObject; }

ctr_object **get_CtrStdBlock() { return &CtrStdBlock; }

ctr_object **get_CtrStdString() { return &CtrStdString; }

ctr_object **get_CtrStdNumber() { return &CtrStdNumber; }

ctr_object **get_CtrStdBool() { return &CtrStdBool; }

ctr_object **get_CtrStdConsole() { return &CtrStdConsole; }

ctr_object **get_CtrStdNil() { return &CtrStdNil; }

ctr_object **get_CtrStdGC() { return &CtrStdGC; }

ctr_object **get_CtrStdMap() { return &CtrStdMap; }

ctr_object **get_CtrStdArray() { return &CtrStdArray; }

ctr_object **get_CtrStdIter() { return &CtrStdIter; }

ctr_object **get_CtrStdFile() { return &CtrStdFile; }

ctr_object **get_CtrStdSystem() { return &CtrStdSystem; }

ctr_object **get_CtrStdDice() { return &CtrStdDice; }

ctr_object **get_CtrStdCommand() { return &CtrStdCommand; }

ctr_object **get_CtrStdSlurp() { return &CtrStdSlurp; }

ctr_object **get_CtrStdShell() { return &CtrStdShell; }

ctr_object **get_CtrStdClock() { return &CtrStdClock; }

ctr_object **get_CtrStdFlow() { return &CtrStdFlow; }

ctr_object **get_CtrExceptionType() { return &CtrExceptionType; }

ctr_object **get_CtrStdBreak() { return &CtrStdBreak; }

ctr_object **get_CtrStdContinue() { return &CtrStdContinue; }

ctr_object **get_CtrStdExit() { return &CtrStdExit; }

ctr_object **get_CtrStdReflect() { return &CtrStdReflect; }

ctr_object **get_CtrStdReflect_cons() { return &CtrStdReflect_cons; }

ctr_object **get_CtrStdFiber() { return &CtrStdFiber; }

ctr_object **get_CtrStdThread() { return &CtrStdThread; }

ctr_object **get_CtrStdSymbol() { return &CtrStdSymbol; }

ctr_object **get_ctr_first_object() { return &ctr_first_object; }

ctr_object **get_CTR_FILE_STDIN() { return &CTR_FILE_STDIN; }

ctr_object **get_CTR_FILE_STDOUT() { return &CTR_FILE_STDOUT; }

ctr_object **get_CTR_FILE_STDERR() { return &CTR_FILE_STDERR; }

ctr_object **get_CTR_FILE_STDIN_STR() { return &CTR_FILE_STDIN_STR; }

ctr_object **get_CTR_FILE_STDOUT_STR() { return &CTR_FILE_STDOUT_STR; }

ctr_object **get_CTR_FILE_STDERR_STR() { return &CTR_FILE_STDERR_STR; }
