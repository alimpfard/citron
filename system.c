#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdarg.h>
#include <math.h>
#include <unistd.h>
#include <stdint.h>
#include <time.h>
#include <errno.h>
#include <sys/wait.h>
#include <sys/ioctl.h>
#include <syslog.h>
#include <signal.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include "compcompat_pthread.h"


#ifdef withBoehmGC
#include <gc/gc.h>
#define pthread_create GC_pthread_create
#endif

#ifdef withTermios
#include <termios.h>
static struct termios oldTermios, newTermios;

#endif
#ifdef forLinux
#include <bsd/stdlib.h>

#else
//Arc4random

#include <sys/types.h>
#include <sys/time.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>

struct arc4_stream
{
  u_int8_t i;
  u_int8_t j;
  u_int8_t s[256];
};

#define	RANDOMDEV	"/dev/urandom"
#define KEYSIZE		128

#ifdef __REENTRANT
static pthread_mutex_t arc4random_mtx = PTHREAD_MUTEX_INITIALIZER;
#define	THREAD_LOCK()	pthread_mutex_lock(&arc4random_mtx)
#define	THREAD_UNLOCK()	pthread_mutex_unlock(&arc4random_mtx)
#else
#define	THREAD_LOCK()
#define	THREAD_UNLOCK()
#endif

static struct arc4_stream rs;
static int rs_initialized;
static int rs_stired;
static int arc4_count;

static inline u_int8_t arc4_getbyte (void);
static void arc4_stir (void);

static inline void
arc4_init (void)
{
  int n;

  for (n = 0; n < 256; n++)
    rs.s[n] = n;
  rs.i = 0;
  rs.j = 0;
}

static inline void
arc4_addrandom (u_char * dat, int datlen)
{
  int n;
  u_int8_t si;

  rs.i--;
  for (n = 0; n < 256; n++)
    {
      rs.i = (rs.i + 1);
      si = rs.s[rs.i];
      rs.j = (rs.j + si + dat[n % datlen]);
      rs.s[rs.i] = rs.s[rs.j];
      rs.s[rs.j] = si;
    }
  rs.j = rs.i;
}

static void
arc4_stir (void)
{
  int done, fd, n;
  struct
  {
    struct timeval tv;
    pid_t pid;
    u_int8_t rnd[KEYSIZE];
  } rdat;

  fd = open (RANDOMDEV, O_RDONLY, 0);
  done = 0;
  if (fd >= 0)
    {
      if (read (fd, &rdat, KEYSIZE) == KEYSIZE)
	done = 1;
      (void) close (fd);
    }
  if (!done)
    {
      (void) gettimeofday (&rdat.tv, NULL);
      rdat.pid = getpid ();
      /* We'll just take whatever was on the stack too... */
    }

  arc4_addrandom ((u_char *) & rdat, KEYSIZE);

  /*
   * Throw away the first N bytes of output, as suggested in the
   * paper "Weaknesses in the Key Scheduling Algorithm of RC4"
   * by Fluher, Mantin, and Shamir.  N=1024 is based on
   * suggestions in the paper "(Not So) Random Shuffles of RC4"
   * by Ilya Mironov.
   */
  for (n = 0; n < 1024; n++)
    (void) arc4_getbyte ();
  arc4_count = 1600000;
}

static inline u_int8_t
arc4_getbyte (void)
{
  u_int8_t si, sj;

  rs.i = (rs.i + 1);
  si = rs.s[rs.i];
  rs.j = (rs.j + si);
  sj = rs.s[rs.j];
  rs.s[rs.i] = sj;
  rs.s[rs.j] = si;

  return (rs.s[(si + sj) & 0xff]);
}

static inline u_int32_t
arc4_getword (void)
{
  u_int32_t val;

  val = arc4_getbyte () << 24;
  val |= arc4_getbyte () << 16;
  val |= arc4_getbyte () << 8;
  val |= arc4_getbyte ();

  return (val);
}

static void
arc4_check_init (void)
{
  if (!rs_initialized)
    {
      arc4_init ();
      rs_initialized = 1;
    }
}

static inline void
arc4_check_stir (void)
{
  if (!rs_stired || arc4_count <= 0)
    {
      arc4_stir ();
      rs_stired = 1;
    }
}

void
arc4random_stir (void)
{
  THREAD_LOCK ();
  arc4_check_init ();
  arc4_stir ();
  rs_stired = 1;
  THREAD_UNLOCK ();
}

void
arc4random_addrandom (u_char * dat, int datlen)
{
  THREAD_LOCK ();
  arc4_check_init ();
  arc4_check_stir ();
  arc4_addrandom (dat, datlen);
  THREAD_UNLOCK ();
}

u_int32_t
arc4random (void)
{
  u_int32_t rnd;

  THREAD_LOCK ();
  arc4_check_init ();
  arc4_check_stir ();
  rnd = arc4_getword ();
  arc4_count -= 4;
  THREAD_UNLOCK ();

  return (rnd);
}

void
arc4random_buf (void *_buf, size_t n)
{
  u_char *buf = (u_char *) _buf;

  THREAD_LOCK ();
  arc4_check_init ();
  while (n--)
    {
      arc4_check_stir ();
      buf[n] = arc4_getbyte ();
      arc4_count--;
    }
  THREAD_UNLOCK ();
}

/*
 * Calculate a uniformly distributed random number less than upper_bound
 * avoiding "modulo bias".
 *
 * Uniformity is achieved by generating new random numbers until the one
 * returned is outside the range [0, 2**32 % upper_bound).  This
 * guarantees the selected random number will be inside
 * [2**32 % upper_bound, 2**32) which maps back to [0, upper_bound)
 * after reduction modulo upper_bound.
 */
u_int32_t
arc4random_uniform (u_int32_t upper_bound)
{
  u_int32_t r, min;

  if (upper_bound < 2)
    return (0);

#if (ULONG_MAX > 0xffffffffUL)
  min = 0x100000000UL % upper_bound;
#else
  /* Calculate (2**32 % upper_bound) avoiding 64-bit math */
  if (upper_bound > 0x80000000)
    min = 1 + ~upper_bound;	/* 2**32 - upper_bound */
  else
    {
      /* (2**32 - (x * 2)) % x == 2**32 % x when x <= 2**31 */
      min = ((0xffffffff - (upper_bound * 2)) + 1) % upper_bound;
    }
#endif

  /*
   * This could theoretically loop forever but each retry has
   * p > 0.5 (worst case, usually far better) of selecting a
   * number inside the range we need, so it should rarely need
   * to re-roll.
   */
  for (;;)
    {
      r = arc4random ();
      if (r >= min)
	break;
    }

  return (r % upper_bound);
}

#endif
#include "citron.h"
#include "siphash.h"


#ifndef withBoehmGC

#define CTR_GC_BACKLOG_MAX 32


ctr_object *gc_checked[CTR_GC_BACKLOG_MAX];	//32 backward spanning last_checked pointers.
/**@I_OBJ_DEF Broom*/

/**
 * @internal
 * GarbageCollector Marker
 *
 * @param last_vector_index : last vector index for this object. pass -1.
 */
void
ctr_gc_mark (ctr_object * object, int last_vector_index)
{
  ctr_object *el;
  ctr_mapitem *item;
  ctr_object *o;
  ctr_object *k;
  //for(int i=0; i<CTR_GC_BACKLOG_MAX; i++) printf("%p, ", gc_checked[i]);
  ctr_size i;
  if (last_vector_index > CTR_GC_BACKLOG_MAX - 1)
    last_vector_index = CTR_GC_BACKLOG_MAX - 1;
  if (last_vector_index > -1)
    for (int i = 0; i < CTR_GC_BACKLOG_MAX && i <= last_vector_index; i++)
      if (gc_checked[i] == object)
	return;
  gc_checked[(last_vector_index += 1)] = object;
  if (object->info.type == CTR_OBJECT_TYPE_OTARRAY && object->value.avalue)
    {

      for (i = object->value.avalue->tail; i < object->value.avalue->head; i++)
	{
	  el = *(object->value.avalue->elements + i);
	  if (el == object)
	    continue;
	  el->info.mark = 1;
	  if (el != object)
	    ctr_gc_mark (el, last_vector_index);
	}
    }
  if (object->properties)
    {
      item = object->properties->head;
      while (item)
	{
	  k = item->key;
	  o = item->value;
	  if (!o || !k)
	    continue;
	  o->info.mark = 1;
	  k->info.mark = 1;
	  if (o != object)
	    ctr_gc_mark (o, last_vector_index);
	  item = item->next;
	}
    }
  if (object->properties)
    {
      item = object->methods->head;
      while (item)
	{
	  o = item->value;
	  k = item->key;
	  if (!o || !k)
	    continue;
	  o->info.mark = 1;
	  k->info.mark = 1;
	  if (o != object)
	    ctr_gc_mark (o, last_vector_index);
	  item = item->next;
	}
    }
}

/**
 * @internal
 * GarbageCollector Sweeper
 */
void
ctr_gc_sweep (int all)
{
  ctr_object *previousObject = NULL;
  ctr_object *currentObject = ctr_first_object;
  ctr_object *nextObject = NULL;
  ctr_mapitem *mapItem = NULL;
  ctr_mapitem *tmp = NULL;
  int i = 0;
  while (currentObject)
    {
      ctr_gc_object_counter++;
      if (all)
	{
	  // if (currentObject->info.type == CTR_OBJECT_TYPE_OTEX) ctr_send_message(currentObject, "destruct", 8, NULL);
	  void (*free_heap_maybe_shared) (void *) = currentObject->info.shared == 0 ? &ctr_heap_free : &ctr_heap_free_shared;
	  ctr_gc_dust_counter++;
	  /* remove from linked list */
	  if (previousObject)
	    {
	      if (currentObject->gnext)
		{
		  previousObject->gnext = currentObject->gnext;
		  nextObject = currentObject->gnext;
		}
	      else
		{
		  previousObject->gnext = NULL;
		  nextObject = NULL;
		}
	    }
	  else
	    {
	      if (currentObject->gnext)
		{
		  ctr_first_object = currentObject->gnext;
		  nextObject = currentObject->gnext;
		}
	      else
		{
		  ctr_first_object = NULL;
		  nextObject = NULL;
		}
	    }
	  if (currentObject->methods->head)
	    {
	      mapItem = currentObject->methods->head;
	      while (mapItem)
		{
		  tmp = mapItem->next;
		  free_heap_maybe_shared (mapItem);
		  mapItem = tmp;
		}
	    }
	  if (currentObject->properties->head)
	    {
	      mapItem = currentObject->properties->head;
	      while (mapItem)
		{
		  tmp = mapItem->next;
		  free_heap_maybe_shared (mapItem);
		  mapItem = tmp;
		}
	    }
	  free_heap_maybe_shared (currentObject->methods);
	  free_heap_maybe_shared (currentObject->properties);
	  switch (currentObject->info.type)
	    {
	    case CTR_OBJECT_TYPE_OTSTRING:
	      if (currentObject->value.svalue != NULL)
		{
		  if (currentObject->value.svalue->vlen > 0)
		    {
		      free_heap_maybe_shared (currentObject->value.svalue->value);
		    }
		  free_heap_maybe_shared (currentObject->value.svalue);
		}
	      break;
	    case CTR_OBJECT_TYPE_OTARRAY:
	      free_heap_maybe_shared (currentObject->value.avalue->elements);
	      free_heap_maybe_shared (currentObject->value.avalue);
	      break;
	    case CTR_OBJECT_TYPE_OTEX:
	      if (currentObject->value.rvalue != NULL)
		{
		  if (currentObject->release_hook)
		    ((voidptrfn_t *) currentObject->release_hook) (currentObject->value.rvalue->ptr);
		  free_heap_maybe_shared (currentObject->value.rvalue);
		}
	      break;
	    }
	  free_heap_maybe_shared (currentObject);
	  currentObject = nextObject;
	}
      else
	{
	  ctr_gc_kept_counter++;
//        // if (currentObject->info.sticky == 1)
	  //   ctr_gc_sticky_counter++;
	  if (currentObject->info.mark == 1)
	    {
	      currentObject->info.mark = 0;
	    }
	  previousObject = currentObject;
	  currentObject = currentObject->gnext;
	}
    }
}

/**
 * @internal
 * Garbage Collector sweep.
 */
void
ctr_gc_internal_collect ()
{
  if (ctr_gc_mode == 0)
    return;
  ctr_object *context;
  int oldcid;
  ctr_gc_dust_counter = 0;
  ctr_gc_object_counter = 0;
  ctr_gc_kept_counter = 0;
  ctr_gc_sticky_counter = 0;
  context = ctr_contexts[ctr_context_id];
  oldcid = ctr_context_id;
  while (ctr_context_id > -1)
    {
      ctr_gc_mark (context, 0);
      ctr_context_id--;
      context = ctr_contexts[ctr_context_id];
    }
  ctr_gc_sweep (0);
  ctr_context_id = oldcid;
}

/**
 * Broom
 *
 * GarbageCollector, to invoke use:
 *
 * [Broom] sweep.
 */
ctr_object *
ctr_gc_collect (ctr_object * myself, ctr_argument * argumentList)
{
  ctr_gc_internal_collect ();	/* calls internal because automatic GC has to use this function as well and requires low overhead. */
  return myself;
}

ctr_object *
ctr_gc_sweep_this (ctr_object * myself, ctr_argument * argumentList)
{
  int all = 0;
  ctr_object *previousObject = NULL;
  ctr_object *currentObject = argumentList->object;
  ctr_object *nextObject = NULL;
  ctr_mapitem *mapItem = NULL;
  ctr_mapitem *tmp = NULL;
  while (currentObject)
    {
      ctr_gc_object_counter++;
      void (*free_heap_maybe_shared) (void *) = currentObject->info.shared == 0 ? &ctr_heap_free : &ctr_heap_free_shared;
      ctr_gc_dust_counter++;
      if (currentObject->methods->head)
	{
	  mapItem = currentObject->methods->head;
	  while (mapItem)
	    {
	      tmp = mapItem->next;
	      free_heap_maybe_shared (mapItem);
	      mapItem = tmp;
	    }
	}
      if (currentObject->properties->head)
	{
	  mapItem = currentObject->properties->head;
	  while (mapItem)
	    {
	      tmp = mapItem->next;
	      free_heap_maybe_shared (mapItem);
	      mapItem = tmp;
	    }
	}
      free_heap_maybe_shared (currentObject->methods);
      free_heap_maybe_shared (currentObject->properties);
      switch (currentObject->info.type)
	{
	case CTR_OBJECT_TYPE_OTSTRING:
	  if (currentObject->value.svalue != NULL)
	    {
	      if (currentObject->value.svalue->vlen > 0)
		{
		  free_heap_maybe_shared (currentObject->value.svalue->value);
		}
	      free_heap_maybe_shared (currentObject->value.svalue);
	    }
	  break;
	case CTR_OBJECT_TYPE_OTARRAY:
	  free_heap_maybe_shared (currentObject->value.avalue->elements);
	  free_heap_maybe_shared (currentObject->value.avalue);
	  break;
	case CTR_OBJECT_TYPE_OTEX:
	  if (currentObject->value.rvalue != NULL)
	    {
	      if (currentObject->release_hook)
		((voidptrfn_t *) currentObject->release_hook) (currentObject->value.rvalue->ptr);
	      free_heap_maybe_shared (currentObject->value.rvalue);
	    }
	  break;
	}
      free_heap_maybe_shared (currentObject);
      currentObject = nextObject;
    }
  return myself;
}
#else //withBoehmGC

void
ctr_gc_sweep (int all)
{
  GC_gcollect ();
}

void
ctr_gc_internal_collect ()
{
  GC_gcollect ();
}

ctr_object *
ctr_gc_collect (ctr_object * myself, ctr_argument * argumentList)
{
  GC_gcollect ();
  return myself;
}

ctr_object *
ctr_gc_sweep_this (ctr_object * myself, ctr_argument * argumentList)
{
  return ctr_build_nil ();
}				//no-op

#endif //withBoehmGC
/**
 * [Broom] dust
 *
 * Returns the number of objects collected.
 */
ctr_object *
ctr_gc_dust (ctr_object * myself, ctr_argument * argumentList)
{
  return ctr_build_number_from_float ((ctr_number) ctr_gc_dust_counter);
}

/**
 * [Broom] unpack: [String:Ref]
 * assigns the instance to the reference
 * (Always prefer using algebraic deconstruction assignments: look at section 'Assignment')
 */
ctr_object *
ctr_gc_assign (ctr_object * myself, ctr_argument * argumentList)
{
  return ctr_object_assign (myself, argumentList);
}

/**
 * [Broom] objectCount
 *
 * Returns the total number of objects considered in the latest collect
 * cycle.
 */
ctr_object *
ctr_gc_object_count (ctr_object * myself, ctr_argument * argumentList)
{
  return ctr_build_number_from_float ((ctr_number) ctr_gc_object_counter);
}

/**
 * [Broom] keptCount
 *
 * Returns the total number of objects that have been marked during the
 * latest cycle and have therefore been allowed to stay in memory.
 */
ctr_object *
ctr_gc_kept_count (ctr_object * myself, ctr_argument * argumentList)
{
  return ctr_build_number_from_float ((ctr_number) ctr_gc_kept_counter);
}

/**
 * [Broom] keptAlloc
 *
 * Returns the amount of allocated memory.
 */
ctr_object *
ctr_gc_kept_alloc (ctr_object * myself, ctr_argument * argumentList)
{
  return ctr_build_number_from_float ((ctr_number)
#ifdef withBoehmGC
	            GC_get_heap_size() - GC_get_free_bytes() - GC_get_unmapped_bytes()
#else
				      ctr_gc_alloc
#endif
    );
}

/**
 * [Broom] stickyCount
 *
 * Returns the total number of objects that have a sticky flag.
 * These objects will never be removed.
 */
ctr_object *
ctr_gc_sticky_count (ctr_object * myself, ctr_argument * argumentList)
{
  return ctr_build_number_from_float ((ctr_number) ctr_gc_sticky_counter);
}

/**
 * [Broom] memoryLimit
 *
 * Sets the memory limit, if this limit gets exceeded the program will produce
 * an out-of-memory error.
 */
ctr_object *
ctr_gc_setmemlimit (ctr_object * myself, ctr_argument * argumentList)
{
  ctr_gc_memlimit = (uint64_t) ctr_internal_cast2number (argumentList->object)->value.nvalue;
#ifdef withBoehmGC
  GC_set_max_heap_size (CTR_LIMIT_MEM ? ctr_gc_memlimit : 0);
#endif
  return myself;
}

/**
 * [Broom] mode: [Number]
 *
 * Selects mode of operation for GC.
 *
 * Available Modes:
 * 0 - No Garbage Collection
 * 1 - Activate Garbage Collector
 * 4 - Activate Garbage Collector for every single step (testing only)
 */
ctr_object *
ctr_gc_setmode (ctr_object * myself, ctr_argument * argumentList)
{
  ctr_gc_mode = (int) ctr_internal_cast2number (argumentList->object)->value.nvalue;
  return myself;
}

/**
 * [Broom] autoAlloc: [Boolean]
 *
 * Set whether the max allowed memory cap is enforced or not
 */
ctr_object *
ctr_gc_setautoalloc (ctr_object * myself, ctr_argument * argumentList)
{
  CTR_LIMIT_MEM = !(ctr_internal_cast2bool (argumentList->object)->value.bvalue);
  return myself;
}

/**@I_OBJ_DEF Shell*/

/**
 * [Shell] call: [String]
 *
 * Performs a Shell operation. The Shell object uses a fluid API, so you can
 * mix shell code with programming logic. For instance to list the contents
 * of a directory use:
 *
 * Shell ls
 *
 * This will output the contents of the current working directly, you
 * can also pass keyword messages like so:
 *
 * Shell echo: 'Hello from the Shell!'.
 *
 * The example above will output the specified message to the console.
 * Every message you send will be turned into a string and dispatched to
 * the 'call:' message.
 */
ctr_object *
ctr_shell_call (ctr_object * myself, ctr_argument * argumentList)
{
  ctr_check_permission (CTR_SECPRO_NO_SHELL);
  ctr_did_side_effect = 1;
  FILE *stream;
  char *outputBuffer;
  ctr_argument *newArgumentList;
  ctr_object *appendString;
  ctr_object *outputString;
  outputBuffer = ctr_heap_allocate (512);
  ctr_object *arg = ctr_internal_cast2string (argumentList->object);
  long vlen = arg->value.svalue->vlen;
  char *comString = ctr_heap_allocate (sizeof (char) * (vlen + 1));
  memcpy (comString, arg->value.svalue->value, vlen);
  memcpy (comString + vlen, "\0", 1);
  newArgumentList = (ctr_argument *) ctr_heap_allocate (sizeof (ctr_argument));
  if (!(stream = popen (comString, "r")))
    {
      CtrStdFlow = ctr_build_string_from_cstring ("Unable to execute command.");
    }
  outputString = ctr_build_empty_string ();
  while (fgets (outputBuffer, 512, stream))
    {
      appendString = ctr_build_string_from_cstring (outputBuffer);
      newArgumentList->object = appendString;
      ctr_string_append (outputString, newArgumentList);
    }
  ctr_heap_free (outputBuffer);
  ctr_heap_free (newArgumentList);
  ctr_heap_free (comString);
  return outputString;
}

/**
 * [Shell] unpack: [String:Ref]
 * assigns the instance to the reference
 * (Always prefer using algebraic deconstruction assignments: look at section 'Assignment')
 */
ctr_object *
ctr_shell_assign (ctr_object * myself, ctr_argument * argumentList)
{
  return ctr_object_assign (myself, argumentList);
}

/**
 * @internal
 *
 * Shell Objects use a fluid API.
 */
ctr_object *
ctr_shell_respond_to_and (ctr_object * myself, ctr_argument * argumentList)
{
  ctr_object *commandObj;
  ctr_object *prefix;
  ctr_object *suffix;
  ctr_argument *newArgumentList;
  ctr_object *result;
  char *command;
  int len;
  prefix = ctr_internal_cast2string (argumentList->object);
  suffix = ctr_internal_cast2string (argumentList->next->object);
  len = prefix->value.svalue->vlen + suffix->value.svalue->vlen;
  if (len == 0)
    return myself;
  command = (char *) ctr_heap_allocate ((sizeof (char) * len));	/* actually we need +1 for the space between commands, but we dont because we remove the colon : ! */
  strncpy (command, prefix->value.svalue->value, prefix->value.svalue->vlen - 1);	/* remove colon, gives room for space */
  strncpy (command + (prefix->value.svalue->vlen - 1), " ", 1);	/* space to separate commands */
  strncpy (command + (prefix->value.svalue->vlen), suffix->value.svalue->value, suffix->value.svalue->vlen);
  commandObj = ctr_build_string (command, len);
  newArgumentList = (ctr_argument *) ctr_heap_allocate (sizeof (ctr_argument));
  newArgumentList->object = commandObj;
  result = ctr_shell_call (myself, newArgumentList);
  ctr_heap_free (newArgumentList);
  ctr_heap_free (command);
  return result;
}

/**
 * @internal
 *
 * Shell Objects use a fluid API.
 */
ctr_object *
ctr_shell_respond_to (ctr_object * myself, ctr_argument * argumentList)
{
  return ctr_shell_call (myself, argumentList);
}

/**
 * @internal
 *
 * Slurp Objects use a fluid API.
 */
ctr_object *
ctr_slurp_respond_to (ctr_object * myself, ctr_argument * argumentList)
{
  ctr_argument *newArgumentList;
  ctr_object *newCommandObj;
  ctr_object *commandObj;
  ctr_object *key;
  newCommandObj = argumentList->object;
  key = ctr_build_string_from_cstring ("command");
  commandObj = ctr_internal_object_find_property (myself, key, CTR_CATEGORY_PRIVATE_PROPERTY);
  newArgumentList = (ctr_argument *) ctr_heap_allocate (sizeof (ctr_argument));
  if (commandObj == NULL)
    {
      commandObj = ctr_build_empty_string ();
      ctr_internal_object_set_property (myself, key, commandObj, CTR_CATEGORY_PRIVATE_PROPERTY);
    }
  else
    {
      newArgumentList->object = ctr_build_string_from_cstring (" ");
      ctr_string_append (commandObj, newArgumentList);
    }
  newArgumentList->object = newCommandObj;
  ctr_string_append (commandObj, newArgumentList);
  ctr_heap_free (newArgumentList);
  return myself;
}

/**
 * @internal
 *
 * Slurp uses a fluid API
 */
ctr_object *
ctr_slurp_respond_to_and (ctr_object * myself, ctr_argument * argumentList)
{
  ctr_object *str;
  ctr_argument *newArgumentList;
  newArgumentList = (ctr_argument *) ctr_heap_allocate (sizeof (ctr_argument));
  str = ctr_internal_cast2string (argumentList->object);
  if ((str->value.svalue->vlen > 0) && *(str->value.svalue->value + (str->value.svalue->vlen - 1)) == ':')
    {
      char *ncstr = ctr_heap_allocate (str->value.svalue->vlen - 1);
      memcpy (ncstr, str->value.svalue->value, str->value.svalue->vlen - 1);
      newArgumentList->object = ctr_build_string_from_cstring (ncstr);
      ctr_slurp_respond_to (myself, newArgumentList);
      ctr_heap_free (ncstr);
    }
  else
    {
      newArgumentList->object = argumentList->object;
      ctr_slurp_respond_to (myself, newArgumentList);
    }
  newArgumentList->object = argumentList->next->object;
  ctr_slurp_respond_to (myself, newArgumentList);
  ctr_heap_free (newArgumentList);
  return myself;
}

/**@I_OBJ_DEF Slurp*/

/**
 * [Slurp] obtain.
 *
 * Obtains the string generated using the Slurp object.
 * A Slurp object collects all messages send to it and flushes its buffer while
 * returning the resulting string after an 'obtain' message has been received.
 *
 * Usage:
 *
 * Slurp hello world.
 * Pen write: (Slurp obtain).
 *
 * This will output: 'hello world'.
 * Use the Slurp object to integrate verbose shell commands, other programming languages
 * (like SQL) etc into your main program without overusing strings.
 *
 * Example:
 *
 * query select: '*', from users where: 'id = 2'.
 *
 * Note that we can't use the = and * unfortunately right now
 * because = is also a method in the main object. While * can be used
 * theoretically, it expects an identifier, and 'from' is not a real
 * identifier, it's just another unary message, so instead of using a binary
 * * we simply use a keyword message select: with argument '*' and then
 * proceed our SQL query with a comma (,) to chain the rest.
 * This is an artifact of the fact that the DSL has to be embedded within
 * the language of Citron. However even with these restrictions (some of which might be
 * alleviated in future versions) it's quite comfortable and readable to interweave
 * an external language in your Citron script code.
 */
ctr_object *
ctr_slurp_obtain (ctr_object * myself, ctr_argument * argumentList)
{
  ctr_object *commandObj;
  ctr_object *key;
  key = ctr_build_string_from_cstring ("command");
  commandObj = ctr_internal_object_find_property (myself, ctr_build_string_from_cstring ("command"), CTR_CATEGORY_PRIVATE_PROPERTY);
  if (commandObj == NULL)
    {
      commandObj = ctr_build_empty_string ();
    }
  ctr_internal_object_delete_property (myself, key, CTR_CATEGORY_PRIVATE_PROPERTY);
  return commandObj;
}

/**@I_OBJ_DEF Program*/

/**
 * [Program] argument: [Number]
 *
 * Obtains an argument from the CLI invocation.
 */
ctr_object *
ctr_command_argument (ctr_object * myself, ctr_argument * argumentList)
{
  ctr_object *numberObject = ctr_internal_cast2number (argumentList->object);
  int n = (int) numberObject->value.nvalue;
  if (n >= ctr_argc || n < 0)
    return CtrStdNil;
  return ctr_build_string (ctr_argv[n], strlen (ctr_argv[n]));
}

/**
 * [Program] argCount
 *
 * Returns the number of CLI arguments passed to the script.
 */
ctr_object *
ctr_command_num_of_args (ctr_object * myself, ctr_argument * argumentList)
{
  return ctr_build_number_from_float ((ctr_number) ctr_argc);
}

/**
 * [Program] unpack: [String:Ref]
 * assigns the instance to the reference
 * (Always prefer using algebraic deconstruction assignments: look at section 'Assignment')
 */
ctr_object *
ctr_program_assign (ctr_object * myself, ctr_argument * argumentList)
{
  return ctr_object_assign (myself, argumentList);
}

/**
 * [Program] exit
 *
 * Exits program immediately.
 */
ctr_object *
ctr_command_exit (ctr_object * myself, ctr_argument * argumentList)
{
  ctr_did_side_effect = 1;
  CtrStdFlow = CtrStdExit;
  return CtrStdNil;
}

/**
 * [Program] env: [String]
 *
 * Returns the value of an environment variable.
 *
 * Usage:
 *
 * x := Command env: 'MY_PATH_VAR'.
 */
ctr_object *
ctr_command_get_env (ctr_object * myself, ctr_argument * argumentList)
{
  ctr_object *envVarNameObj;
  char *envVarNameStr;
  char *envVal;
  ctr_check_permission (CTR_SECPRO_NO_FILE_READ);
  envVarNameObj = ctr_internal_cast2string (argumentList->object);
  envVarNameStr = ctr_heap_allocate ((envVarNameObj->value.svalue->vlen + 1) * sizeof (char));
  strncpy (envVarNameStr, envVarNameObj->value.svalue->value, envVarNameObj->value.svalue->vlen);
  *(envVarNameStr + (envVarNameObj->value.svalue->vlen)) = '\0';
  envVal = getenv (envVarNameStr);
  ctr_heap_free (envVarNameStr);
  if (envVal == NULL)
    {
      return CtrStdNil;
    }
  return ctr_build_string_from_cstring (envVal);
}

/**
 * [Program] env: [Key] val: [Value]
 *
 * Sets the value of an environment variable.
 */
ctr_object *
ctr_command_set_env (ctr_object * myself, ctr_argument * argumentList)
{
  ctr_object *envVarNameObj;
  ctr_object *envValObj;
  char *envVarNameStr;
  char *envValStr;
  ctr_check_permission (CTR_SECPRO_NO_FILE_WRITE);
  ctr_did_side_effect = 1;
  envVarNameObj = ctr_internal_cast2string (argumentList->object);
  envValObj = ctr_internal_cast2string (argumentList->next->object);
  envVarNameStr = ctr_heap_allocate_cstring (envVarNameObj);
  envValStr = ctr_heap_allocate_cstring (envValObj);
  setenv (envVarNameStr, envValStr, 1);
  ctr_heap_free (envValStr);
  ctr_heap_free (envVarNameStr);
  return myself;
}

/**
 * [Program] changeDirectory: [path:String]
 *
 * chdir to path, returns the past directory on success
 */
ctr_object *
ctr_command_chdir (ctr_object * myself, ctr_argument * argumentList)
{
  ctr_did_side_effect = 1;
  CTR_ENSURE_TYPE_STRING (argumentList->object);
  char *path = ctr_heap_allocate_cstring (argumentList->object);
  char *curpath = realpath (".", NULL);
  if (!curpath)
    {
      CtrStdFlow = ctr_build_string_from_cstring ("Could not extends real path of current directory");
      return CtrStdNil;
    }
  ctr_object *lpath = ctr_build_string_from_cstring (curpath);
  free (curpath);
  if (chdir (path) == 0)
    {
      ctr_heap_free (path);
      return lpath;
    }
  ctr_object *err = ctr_build_string_from_cstring ("Error occurred while changing directory: ");
  ctr_invoke_variadic (err, &ctr_string_append, 1, ctr_build_string_from_cstring (strerror (errno)));
  CtrStdFlow = err;
  return CtrStdNil;
}

/**
 * [Program] waitForInput
 *
 * Ask a question on the command-line, resumes program
 * only after pressing the enter key.
 * Only reads up to 100 characters.
 *
 * Usage:
 *
 * Pen write: 'What is your name ?'.
 * x := Command waitForInput.
 * Pen write: 'Hello ' + x + ' !', brk.
 *
 * The example above asks the user for his/her name and
 * then displays the input received.
 */
ctr_object *
ctr_command_waitforinput (ctr_object * myself, ctr_argument * argumentList)
{
  ctr_check_permission (CTR_SECPRO_COUNTDOWN);
  ctr_did_side_effect = 1;
  /*int c;
     ctr_size bytes = 0;
     char* buff;
     ctr_size page = 10;
     buff = ctr_heap_allocate(page * sizeof(char));
     while ((c = getchar()) != '\n') {
     buff[bytes] = c;
     bytes++;
     if (bytes > page) {
     page *= 2;
     buff = (char*) ctr_heap_reallocate(buff, page * sizeof(char));
     if (buff == NULL) {
     CtrStdFlow = ctr_build_string_from_cstring("Out of memory");
     return CtrStdNil;
     }
     }
     }
     ctr_heap_free(buff); *///TODO: Finish memory management
  char *line = NULL;
  ctr_size len = 0;
  getline (&line, &len, stdin);
  ctr_object *ret = ctr_build_string (line, strlen (line));
  free (line);
  return ret;
}

/**
 * [Program] getCharacter
 *
 * reads a character from STDIN.
 * Will only return after reading one ASCII character and a return.
 */
ctr_object *
ctr_command_getc (ctr_object * myself, ctr_argument * argumentList)
{
  ctr_check_permission (CTR_SECPRO_COUNTDOWN);
  ctr_did_side_effect = 1;
  // char c;
  char c;
#ifdef withTermios
  tcgetattr (STDIN_FILENO, &oldTermios);
  newTermios = oldTermios;
  cfmakeraw (&newTermios);
  tcsetattr (STDIN_FILENO, TCSANOW, &newTermios);
#endif
  while ((c = fgetc (stdin)) == EOF);
#ifdef withTermios
  tcsetattr (STDIN_FILENO, TCSANOW, &oldTermios);
#endif
  return ctr_build_string (&c, 1);
}

/**
 * [Program] input.
 *
 * Reads all raw input from STDIN.
 *
 * Usage (for instance to read raw CGI post):
 *
 * post := Program input.
 */
ctr_object *
ctr_command_input (ctr_object * myself, ctr_argument * argumentList)
{
  ctr_did_side_effect = 1;
  ctr_size page = 64;
  char buffer[page];
  ctr_size content_size = 1;
  char *content = ctr_heap_allocate (sizeof (char) * page);
  while (fgets (buffer, page, stdin))
    {
      content_size += strlen (buffer);
      content = ctr_heap_reallocate (content, content_size);
      strcat (content, buffer);
    }
  ctr_object *str = ctr_build_string_from_cstring (content);
  ctr_heap_free (content);
  return str;

}

ctr_object *
ctr_command_set_INT_handler (ctr_object * myself, ctr_argument * argumentList)
{
  ctr_did_side_effect = 1;
  if (argumentList->object->info.type != CTR_OBJECT_TYPE_OTBLOCK)
    {
      printf ("Expected a block to handle interrupts with.\n");
      exit (1);
    }
  ctr_global_interrupt_handler = argumentList->object;
  return myself;
}

void
ctr_int_handler (int isig)
{
  //printf("%i\n", isig);
  if (ctr_global_interrupt_handler != NULL)
    {
      ctr_invoke_variadic (ctr_global_interrupt_handler, &ctr_block_runIt, 1, ctr_build_number_from_float (isig));
      if (CtrStdFlow)
	CtrStdFlow = CtrStdExit;	//exit on exception in interrupt handler
    }
  else
    {
      switch (isig)
	{
	case SIGINT:
	  // if (CtrStdFlow) {
	  CtrStdFlow = ctr_format_str ("Interrupt");
	  // }
	  break;
	case SIGTERM:
	case SIGSEGV:
	case SIGBUS:
	case SIGQUIT:
	case SIGHUP:
	  printf ("Got signal '%s', quitting\n", strsignal (isig));
	  exit (isig);
	default:
	  return;
	}
    }
}

/**
 * @internal
 *
 * Checks whether the user is allowed to perform this kind of operation.
 */
void
ctr_check_permission (uint8_t operationID)
{
  char *reason;
  if ((ctr_command_security_profile & operationID))
    {
      reason = "This program is not allowed to perform this operation.";
      if (operationID == CTR_SECPRO_NO_SHELL)
	{
	  reason = "This program is not allowed to execute shell commands.";
	}
      if (operationID == CTR_SECPRO_NO_FILE_WRITE)
	{
	  reason = "This program is not allowed to modify or delete any files or folders.";
	}
      if (operationID == CTR_SECPRO_NO_FILE_READ)
	{
	  reason = "This program is not allowed to perform any file operations.";
	}
      if (operationID == CTR_SECPRO_NO_INCLUDE)
	{
	  reason = "This program is not allowed to include any other files for code execution.";
	}
      if (operationID == CTR_SECPRO_FORK)
	{
	  reason = "This program is not allowed to spawn other processes or serve remote objects.";
	}
      printf ("%s\n", reason);
      exit (1);
    }
}

/**
 * [Program] forbidShell
 *
 * This method is part of the security profiles feature of Citron.
 * This will forbid the program to execute any shell operations. All
 * external libraries and plugins are assumed to respect this setting as well.
 *
 * Usage:
 *
 * Program forbidShell.
 */
ctr_object *
ctr_command_forbid_shell (ctr_object * myself, ctr_argument * argumentList)
{
  ctr_command_security_profile |= CTR_SECPRO_NO_SHELL;
  return myself;
}

/**
 * [Program] forbidFileWrite
 *
 * This method is part of the security profiles feature of Citron.
 * This will forbid the program to modify, create or delete any files. All
 * external libraries and plugins are assumed to respect this setting as well.
 *
 * Usage:
 *
 * Program forbidFileWrite.
 */
ctr_object *
ctr_command_forbid_file_write (ctr_object * myself, ctr_argument * argumentList)
{
  ctr_command_security_profile |= CTR_SECPRO_NO_FILE_WRITE;
  return myself;
}

/**
 * [Program] forbidFileRead
 *
 * This method is part of the security profiles feature of Citron.
 * This will forbid the program to read any files. In fact this will prevent you from
 * creating the file object at all.
 * This will also prevent you from reading environment variables.
 * All external libraries and plugins are assumed to respect this setting as well.
 * Forbidding a program to read files also has the effect to forbid including other
 * source files.
 *
 * Usage:
 *
 * Program forbidFileRead.
 */
ctr_object *
ctr_command_forbid_file_read (ctr_object * myself, ctr_argument * argumentList)
{
  ctr_command_security_profile |= CTR_SECPRO_NO_FILE_READ;
  return myself;
}

/**
 * [Program] forbidInclude
 *
 * This method is part of the security profiles feature of Citron.
 * This will forbid the program to include any other files. All
 * external libraries and plugins are assumed to respect this setting as well.
 *
 * Usage:
 *
 * Program forbidInclude.
 */
ctr_object *
ctr_command_forbid_include (ctr_object * myself, ctr_argument * argumentList)
{
  ctr_command_security_profile |= CTR_SECPRO_NO_INCLUDE;
  return myself;
}

/**
 * [Program] forbidFork.
 */
ctr_object *
ctr_command_forbid_fork (ctr_object * myself, ctr_argument * argumentList)
{
  ctr_command_security_profile |= CTR_SECPRO_FORK;
  return myself;
}

/**
 * [Program] remainingMessages: [Number]
 *
 * This method is part of the security profiles feature of Citron.
 * This will initiate a countdown for the program, you can specify the maximum quota of
 * messages the program may process, once this quota has been exhausted the program will
 * be killed entirely (no exception).
 *
 * Usage:
 *
 * Program remainingMessages: 100.
 */
ctr_object *
ctr_command_countdown (ctr_object * myself, ctr_argument * argumentList)
{
  if (ctr_command_security_profile & CTR_SECPRO_COUNTDOWN)
    {
      printf ("Message quota cannot change.\n");
      exit (1);
    }
    ctr_did_side_effect = 1;
  ctr_command_security_profile |= CTR_SECPRO_COUNTDOWN;
  ctr_command_maxtick = (uint64_t) ctr_internal_cast2number (argumentList->object)->value.nvalue;
  return myself;
}

/**
 * [Program] flush.
 *
 * Flushes the STDOUT output buffer.
 */
ctr_object *
ctr_command_flush (ctr_object * myself, ctr_argument * ctr_argumentList)
{
  fflush (stdout);
  ctr_did_side_effect = 1;
  return myself;
}

/**
 * [Program] new: [Block].
 *
 * Forks the program into two programs.
 * Creates another program that will run at the same time as the
 * current program. Both the parent and the child will obtain a reference
 * to the newly created program. The child will obtain a reference to
 * itself passed as a parameter to the code block while the parent will
 * obtain its version of the program instance as the return value of the
 * new: message.
 *
 * Note that spawning a new program will leak memory.
 * The file descriptors used to setup communication between parent and
 * child will be removed when the main program ends but any newly created
 * program will add a descriptor pair to the set. This is a limitation
 * in the current implementation.
 *
 * Usage:
 *
 * child := Program new: { :program
 * 	Pen write: 'Child', brk.
 * }.
 * Pen write: 'Parent'.
 */
ctr_object *
ctr_command_fork (ctr_object * myself, ctr_argument * argumentList)
{
  int p;
  int *ps;
  FILE *pipes;
  ctr_object *child;
  ctr_argument *newArgumentList;
  ctr_resource *rs;
  ctr_check_permission (CTR_SECPRO_COUNTDOWN);
  ctr_check_permission (CTR_SECPRO_FORK);
  ctr_did_side_effect = 1;
  newArgumentList = ctr_heap_allocate (sizeof (ctr_argument));
  child = ctr_internal_create_object (CTR_OBJECT_TYPE_OTOBJECT);
  ctr_set_link_all (child, myself);
  ps = ctr_heap_allocate (sizeof (int) * 4);
  pipes = ctr_heap_allocate_tracked (
#ifndef forLinux
				      216 * 2
#else
				      sizeof (FILE) * 2
#endif
    );
  rs = ctr_heap_allocate_tracked (sizeof (ctr_resource));
  rs->type = 2;
  rs->ptr = (void *) pipes;
  child->value.rvalue = rs;
  newArgumentList->object = child;
  pipe (ps);
  pipe (ps + 2);
  p = fork ();
  if (p < 0)
    {
      CtrStdFlow = ctr_build_string_from_cstring ("Unable to fork");
      ctr_heap_free (newArgumentList);
      ctr_heap_free (rs);
      return CtrStdNil;
    }
  if (p == 0)
    {
      close (*(ps + 0));
      close (*(ps + 3));
      *((FILE **) rs->ptr + 1) = fdopen (*(ps + 1), "wb");
      *((FILE **) rs->ptr + 2) = fdopen (*(ps + 2), "rb");
      setvbuf (*((FILE **) rs->ptr + 1), NULL, _IONBF, 0);
      setvbuf (*((FILE **) rs->ptr + 2), NULL, _IONBF, 0);
      rs->type = 3;
      ctr_block_runIt (argumentList->object, newArgumentList);
      fclose (*((FILE **) rs->ptr + 1));
      fclose (*((FILE **) rs->ptr + 2));
      ctr_heap_free (newArgumentList);
      ctr_heap_free (ps);
      if (CtrStdFlow == NULL || CtrStdFlow == CtrStdBreak || CtrStdFlow == CtrStdContinue)
	CtrStdFlow = CtrStdExit;	//promote it to exit
      return CtrStdNil;
    }
  else
    {
      ctr_internal_object_set_property (child,
					ctr_build_string_from_cstring
					("pid"), ctr_build_number_from_float ((ctr_number) p), CTR_CATEGORY_PRIVATE_PROPERTY);
      close (*(ps + 1));
      close (*(ps + 2));
      *((FILE **) rs->ptr + 3) = fdopen (*(ps + 3), "wb");
      *((FILE **) rs->ptr + 0) = fdopen (*(ps + 0), "rb");
      setvbuf (*((FILE **) rs->ptr + 3), NULL, _IONBF, 0);
      setvbuf (*((FILE **) rs->ptr + 0), NULL, _IONBF, 0);
      ctr_heap_free (newArgumentList);
      ctr_heap_free (ps);
    }
  return child;
}

/**
 * [Program] message: [String].
 *
 * Sends a message to another program, i.e. a child or a parent that is
 * running at the same time.
 */
ctr_object *
ctr_command_message (ctr_object * myself, ctr_argument * argumentList)
{
  char *str;
  ctr_size n;
  ctr_object *string;
  ctr_resource *rs;
  int q;
  FILE *fd;
  ctr_did_side_effect = 1;
  string = ctr_internal_cast2string (argumentList->object);
  str = ctr_heap_allocate_cstring (string);
  n = string->value.svalue->vlen;
  q = 1;
  rs = myself->value.rvalue;
  if (rs->type == 2)
    q = 3;
  fd = *((FILE **) rs->ptr + q);
  fwrite (&n, sizeof (ctr_size), 1, fd);
  fwrite (str, 1, n, fd);
  ctr_heap_free (str);
  return myself;
}

/**
 * [Program] listen: [Block].
 * [Program] listen: [Block] timeout: [Number {qualification:timespec}]
 *
 * Stops the current flow of the program and starts listening for
 * messages from other programs that are running at the same time.
 * Upon receiving a message, the specified block will be invocated
 * and passed the message that has been received.

 * If a timeout is set, and it expires without getting a message,
 * this message will throw.
 */
ctr_object *
ctr_command_listen (ctr_object * myself, ctr_argument * argumentList)
{
  ctr_object *answer;
  ctr_resource *r;
  ctr_argument *newArgumentList;
  int q;
  ctr_size sz;
  FILE *fd;
  char *blob;
  q = 0;
  r = myself->value.rvalue;
  ctr_did_side_effect = 1;
  if (r == NULL)
    {
      CtrStdFlow = ctr_build_string_from_cstring ("The main program is not allowed to wait for messages.");
      return CtrStdFlow;
    }
  if (r->type == 3)
    q = 2;
  fd = *((FILE **) r->ptr + q);
  sz = 0;
  ctr_size *szptr = &sz;
  ssize_t szcp = sizeof (ctr_size);
  ssize_t readp;
  int timeout = -1;
  _Bool dotime = 0;
  if (argumentList->next && argumentList->next->object) {
    dotime = 1;
    ctr_object* tobj = ctr_internal_cast2number(argumentList->next->object);
    timeout = tobj->value.nvalue;
    ctr_object *qual = ctr_internal_object_find_property (tobj,
  							ctr_build_string_from_cstring (CTR_DICT_QUALIFICATION),
  							CTR_CATEGORY_PRIVATE_PROPERTY);
    if (qual)
      {
        char *qualf = ctr_heap_allocate_cstring (qual);
        if (strncasecmp (qualf, "ns", 2) == 0)
  	  timeout /= 1000000000;
        else if (strncasecmp (qualf, "us", 2) == 0)
  	  timeout /= 1000000;
        else if (strncasecmp (qualf, "ms", 2) == 0)
  	  timeout /= 1000;
        else if (strncasecmp (qualf, "mi", 2) == 0)
  	  timeout *= 60;
        else if (strncasecmp (qualf, "ho", 2) == 0)
  	  timeout *= 60*60;
      }
  }
  time_t t0 = time(NULL);
  while ((readp = read (fileno (fd), szptr, szcp)) < szcp && (!dotime || (dotime&&time(NULL)-t0<timeout)))
    //timeout listening if nothing was given
    {
      if (readp == -1)
	{
	  perror ("Error occurred while reading pipe");
	  continue;
	}
      szcp -= readp;
      szptr += readp;
    }
  if (dotime && time(NULL) - t0 >= timeout) {
    CtrStdFlow = ctr_build_string_from_cstring("Timeout expired");
    return myself;
  }
  blob = ctr_heap_allocate (sz);
  char *blobptr = blob;
  szcp = sz;
  while ((readp = read (fileno (fd), blobptr, szcp)) < szcp)
    {
      if (readp == -1)
	{
	  perror ("Error occurred while reading pipe");
	  continue;
	}
      szcp -= readp;
      blobptr += readp;
    }
  newArgumentList = ctr_heap_allocate (sizeof (ctr_argument));
  newArgumentList->object = ctr_build_string (blob, sz);
  answer = ctr_block_runIt (argumentList->object, newArgumentList);
  ctr_heap_free (blob);
  ctr_heap_free (newArgumentList);
  return answer;
}

/**
 * [Program] join
 *
 * Rejoins the program with the main program.
 * This message will cause the current program to stop and wait
 * for the child program to end.
 */
ctr_object *
ctr_command_join (ctr_object * myself, ctr_argument * argumentList)
{
  int pid;
  ctr_resource *rs = myself->value.rvalue;
  if (rs == NULL)
    return CtrStdNil;
    ctr_did_side_effect = 1;
  if (rs->type == 3)
    {
      CtrStdFlow = ctr_build_string_from_cstring ("a child process can not join.");
      return CtrStdNil;
    }
  pid = (int) ctr_internal_object_find_property (myself, ctr_build_string_from_cstring ("pid"), CTR_CATEGORY_PRIVATE_PROPERTY)->value.nvalue;
  fclose (*((FILE **) rs->ptr + 3));
  waitpid (pid, 0, 0);
  ctr_object *retval = CtrStdNil;
  size_t sz = 0;
  char *blob = ctr_heap_allocate (sizeof (size_t));
  char *blobptr = blob;
  FILE *rfp = *((FILE **) rs->ptr + 0);
  ssize_t iret = read (fileno (rfp), &sz, sizeof (size_t));
  if (iret == -1 || iret == 0)
    goto end_close;
  //we've read something, try to return.
  iret = read (fileno (rfp), blobptr, sz);	//this program won't write anything else
  if (iret < sz)
    goto end_close;
  retval = ctr_build_string (blob, sz);
end_close:
  ctr_heap_free (blob);
  fclose (rfp);
  return retval;
}

ctr_object *
ctr_command_pid (ctr_object * myself, ctr_argument * argumentList)
{
  ctr_object *pidObject;
  pidObject = ctr_internal_object_find_property (myself, ctr_build_string_from_cstring ("pid"), CTR_CATEGORY_PRIVATE_PROPERTY);
  if (pidObject == NULL)
    return ctr_build_number_from_float (getpid ());
  return ctr_internal_cast2number (pidObject);
}

ctr_object *
ctr_command_sig (ctr_object * myself, ctr_argument * argumentList)
{
  int sig = ctr_internal_cast2number (argumentList->object)->value.nvalue;
  ctr_object *pid_o = ctr_internal_object_find_property (myself,
							 ctr_build_string_from_cstring ("pid"),
							 0);
  if (!pid_o)
    {
      CtrStdFlow = ctr_build_string_from_cstring ("Cannot send a signal to a non-program object");
      return CtrStdNil;
    }
  int pid = pid_o->value.nvalue;
  ctr_did_side_effect = 1;
  if (kill (pid, sig) != 0)
    {
      CtrStdFlow = ctr_build_string_from_cstring (strerror (errno));
      return myself;
    }
  return myself;
}

ctr_object *
ctr_command_sigmap (ctr_object * myself, ctr_argument * argumentList)
{
  return myself;		//TODO: Implement
}

ctr_object *
ctr_command_log_generic (ctr_object * myself, ctr_argument * argumentList, int level)
{
  char *message;
  ctr_check_permission (CTR_SECPRO_COUNTDOWN);
  ctr_did_side_effect = 1;
  message = ctr_heap_allocate_cstring (ctr_internal_cast2string (argumentList->object));
  syslog (level, "%s", message);
  ctr_heap_free (message);
  return myself;
}

ctr_object *
ctr_command_log (ctr_object * myself, ctr_argument * argumentList)
{
  return ctr_command_log_generic (myself, argumentList, LOG_NOTICE);
}

ctr_object *
ctr_command_warn (ctr_object * myself, ctr_argument * argumentList)
{
  return ctr_command_log_generic (myself, argumentList, LOG_WARNING);
}

ctr_object *
ctr_command_err (ctr_object * myself, ctr_argument * argumentList)
{
  return ctr_command_log_generic (myself, argumentList, LOG_ERR);
}

ctr_object *
ctr_command_crit (ctr_object * myself, ctr_argument * argumentList)
{
  return ctr_command_log_generic (myself, argumentList, LOG_EMERG);
}

/**
 * [Object] fromComputer: [String] [port: [Number] [inet6: [Boolean]]]
 *
 * Creates a remote object from the server specified by the
 * ip address.
 */
ctr_object *
ctr_command_remote (ctr_object * myself, ctr_argument * argumentList)
{
  ctr_check_permission (CTR_SECPRO_COUNTDOWN);
  ctr_did_side_effect = 1;
  ctr_object *remoteObj = ctr_internal_create_object (CTR_OBJECT_TYPE_OTOBJECT);
  ctr_set_link_all (remoteObj, CtrStdObject);
  remoteObj->info.remote = 1;
  ctr_object *port, *inet6;
  if (argumentList->next)
    port = argumentList->next->object;
  else
    port = ctr_build_number_from_float (-1);
  if (argumentList->next && argumentList->next->next)
    inet6 = argumentList->next->next->object;
  else
    inet6 = ctr_build_bool (1);
  ctr_internal_object_set_property (remoteObj,
				    ctr_build_string_from_cstring ("@"),
				    ctr_internal_cast2string (argumentList->object), CTR_CATEGORY_PRIVATE_PROPERTY);
  ctr_internal_object_set_property (remoteObj, ctr_build_string_from_cstring (":"), ctr_internal_cast2number (port), CTR_CATEGORY_PRIVATE_PROPERTY);
  ctr_internal_object_set_property (remoteObj, ctr_build_string_from_cstring ("%"), ctr_internal_cast2bool (inet6), CTR_CATEGORY_PRIVATE_PROPERTY);
  return remoteObj;
}

/**
 * [Program] port: [Number].
 *
 * Sets the port to use for remote connections.
 */
ctr_object *
ctr_command_default_port (ctr_object * myself, ctr_argument * argumentList)
{
  ctr_default_port = (uint16_t) ctr_internal_cast2number (argumentList->object)->value.nvalue;
  ctr_did_side_effect = 1;
  return myself;
}

/**
 * [Program] connectionLimit: [Number].
 *
 * Sets the maximum number of connections and requests that will be
 * accepted by the current program.
 */
ctr_object *
ctr_command_accept_number (ctr_object * myself, ctr_argument * argumentList)
{
  ctr_accept_n_connections = (uint8_t) ctr_internal_cast2number (argumentList->object)->value.nvalue;
  ctr_did_side_effect = 1;
  return myself;
}

/**
 * [Program] serve: [Object].
 *
 * Serves an object. Client programs can now communicate with this object
 * and send messages to it.
 */
ctr_object *
ctr_command_accept (ctr_object * myself, ctr_argument * argumentList)
{
  int listenfd = 0, connfd = 0;
  ctr_object *responder;
  ctr_object *answerObj;
  ctr_object *stringObj;
  ctr_object *messageDescriptorArray;
  ctr_object *messageSelector;
  ctr_argument *callArgument;
  char *dataBuff;
  ctr_size lengthBuff;
  struct sockaddr_in6 serv_addr;
  uint8_t x;
  ctr_check_permission (CTR_SECPRO_COUNTDOWN);
  ctr_check_permission (CTR_SECPRO_FORK);
  ctr_did_side_effect = 1;
  responder = argumentList->object;
  listenfd = socket (AF_INET6, SOCK_STREAM, 0);
  bzero ((char *) &serv_addr, sizeof (serv_addr));
  serv_addr.sin6_flowinfo = 0;
  serv_addr.sin6_family = AF_INET6;
  serv_addr.sin6_addr = in6addr_any;
  serv_addr.sin6_port = htons (ctr_default_port);	//atoi?
  bind (listenfd, (struct sockaddr *) &serv_addr, sizeof (serv_addr));
  if (listen (listenfd, 10) == -1)
    {
      CtrStdFlow = ctr_build_string_from_cstring ("Unable to listen to socket.");
      return CtrStdNil;
    }
  x = 0;
  while (1 && (ctr_accept_n_connections == 0 || (x < ctr_accept_n_connections)))
    {
      x++;
      while ((connfd = accept (listenfd, (struct sockaddr *) NULL, NULL)) == -1);	// accept awaiting request
      //read( connfd, &lengthBuff, sizeof(ctr_size));
      //printf("%d", lengthBuff);
      printf ("Accepted connection\n");
      lengthBuff = 3000 * sizeof (ctr_size);
      dataBuff = ctr_heap_allocate (lengthBuff + 1);
      if ((lengthBuff = read (connfd, dataBuff, lengthBuff)) == -1)
	{
	  printf ("%s\n", strerror (errno));
	  exit (1);
	}
      stringObj = ctr_string_ltrim (ctr_build_string_from_cstring (dataBuff), NULL);	//TODO:  JSON?
      if (stringObj->value.svalue->vlen >= 6 && strncmp (stringObj->value.svalue->value, "Array ", 6) == 0)
	{			//it was a correct message
	  messageDescriptorArray = ctr_string_eval (stringObj, NULL);
	  if (messageDescriptorArray->info.type == CTR_OBJECT_TYPE_OTARRAY)
	    {
	      messageSelector = ctr_array_shift (messageDescriptorArray, NULL);
	    }
	  else
	    {
	      messageSelector = ctr_internal_cast2string (messageDescriptorArray);
	    }
	  callArgument = ctr_heap_allocate (sizeof (ctr_argument));
	  callArgument->object = messageSelector;
	  callArgument->next = ctr_heap_allocate (sizeof (ctr_argument));
	  callArgument->next->object = messageDescriptorArray;
	  answerObj = ctr_internal_cast2string (ctr_object_message (responder, callArgument));
	  ctr_heap_free (callArgument->next);
	  ctr_heap_free (callArgument);
	}
      else
	{
	  answerObj = ctr_internal_cast2string (ctr_send_message_variadic (responder, "handleRequest:", 14, 1, stringObj));
	}
      //write( connfd, (ctr_size*) &answerObj->value.svalue->vlen, sizeof(ctr_size) );
      write (connfd, answerObj->value.svalue->value, answerObj->value.svalue->vlen);
      ctr_heap_free (dataBuff);
      close (connfd);
    }
  shutdown (listenfd, SHUT_RDWR);
  close (listenfd);
  return 0;
}

/**
 * [Program] serve_ipv4: [Object].
 *
 * Serves an object. Client programs can now communicate with this object
 * and send messages to it. starts the server with an ipv4 binding
 */
ctr_object *
ctr_command_accepti4 (ctr_object * myself, ctr_argument * argumentList)
{
  int listenfd = 0, connfd = 0;
  ctr_object *responder;
  ctr_object *answerObj;
  ctr_object *stringObj;
  ctr_object *messageDescriptorArray;
  ctr_object *messageSelector;
  ctr_argument *callArgument;
  char *dataBuff;
  ctr_size lengthBuff;
  struct sockaddr_in serv_addr;
  uint8_t x;
  ctr_check_permission (CTR_SECPRO_COUNTDOWN);
  ctr_check_permission (CTR_SECPRO_FORK);
  ctr_did_side_effect = 1;
  responder = argumentList->object;
  listenfd = socket (AF_INET, SOCK_STREAM, 0);
  bzero ((char *) &serv_addr, sizeof (serv_addr));
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_addr.s_addr = INADDR_ANY;
  serv_addr.sin_port = htons (ctr_default_port);	//atoi?
  bind (listenfd, (struct sockaddr *) &serv_addr, sizeof (serv_addr));
  if (listen (listenfd, 10) == -1)
    {
      CtrStdFlow = ctr_build_string_from_cstring ("Unable to listen to socket.");
      return CtrStdNil;
    }
  x = 0;
  while (1 && (ctr_accept_n_connections == 0 || (x < ctr_accept_n_connections)))
    {
      x++;
      while ((connfd = accept (listenfd, (struct sockaddr *) NULL, NULL)) == -1);	// accept awaiting request
      //read( connfd, &lengthBuff, sizeof(ctr_size));
      //printf("%d", lengthBuff);
      printf ("Accepted connection\n");
      lengthBuff = 3000 * sizeof (ctr_size);
      dataBuff = ctr_heap_allocate (lengthBuff + 1);
      if ((lengthBuff = read (connfd, dataBuff, lengthBuff)) == -1)
	{
	  printf ("%s\n", strerror (errno));
	  exit (1);
	}
      stringObj = ctr_string_ltrim (ctr_build_string_from_cstring (dataBuff), NULL);	//TODO:  JSON?
      if (stringObj->value.svalue->vlen >= 6 && strncmp (stringObj->value.svalue->value, "Array ", 6) == 0)
	{			//it was a correct message
	  messageDescriptorArray = ctr_string_eval (stringObj, NULL);
	  if (messageDescriptorArray->info.type == CTR_OBJECT_TYPE_OTARRAY)
	    {
	      messageSelector = ctr_array_shift (messageDescriptorArray, NULL);
	    }
	  else
	    {
	      messageSelector = ctr_internal_cast2string (messageDescriptorArray);
	    }
	  callArgument = ctr_heap_allocate (sizeof (ctr_argument));
	  callArgument->object = messageSelector;
	  callArgument->next = ctr_heap_allocate (sizeof (ctr_argument));
	  callArgument->next->object = messageDescriptorArray;
	  answerObj = ctr_internal_cast2string (ctr_object_message (responder, callArgument));
	  ctr_heap_free (callArgument->next);
	  ctr_heap_free (callArgument);
	}
      else
	{
	  answerObj = ctr_internal_cast2string (ctr_send_message_variadic (responder, "handleRequest:", 14, 1, stringObj));
	}
      //write( connfd, (ctr_size*) &answerObj->value.svalue->vlen, sizeof(ctr_size) );
      write (connfd, answerObj->value.svalue->value, answerObj->value.svalue->vlen);
      ctr_heap_free (dataBuff);
      close (connfd);
    }
  shutdown (listenfd, SHUT_RDWR);
  close (listenfd);
  return 0;
}

/**@I_OBJ_DEF Dice*/

/**
 * [Dice] rollWithSides: [Number]
 *
 * Rolls the dice, generates a pseudo random number.
 */
ctr_object *
ctr_dice_sides (ctr_object * myself, ctr_argument * argumentList)
{
  ctr_did_side_effect = 1;
  ctr_object *arg = ctr_internal_cast2number (argumentList->object);
  return ctr_build_number_from_float ((ctr_number) arc4random_uniform ((uint32_t) (ceil (arg->value.nvalue))));
}

/**
 * [Dice] roll
 *
 * Rolls a standard dice with 6 sides.
 */
ctr_object *
ctr_dice_throw (ctr_object * myself, ctr_argument * argumentList)
{
  ctr_did_side_effect = 1;
  return ctr_build_number_from_float ((ctr_number) 1 + arc4random_uniform ((uint32_t) 6));
}

/**
 * [Dice] rawRandomNumber
 *
 * Generates a random number, the traditional way (like rand()).
 */
ctr_object *
ctr_dice_rand (ctr_object * myself, ctr_argument * argumentList)
{
  ctr_did_side_effect = 1;
  return ctr_build_number_from_float ((ctr_number) (arc4random ()));
}

/**@I_OBJ_DEF Clock*/

/**
 * [Clock] wait: [Number]
 *
 * Waits X useconds* depends on qualification
 * Quals:
 * ns[ecs] nanosecs
 * us[ecs] microsecs
 * ms[ecs] milisecs
 * se[cs] seconds
 * mi[nutes] minutes
 * ho[urs] hours
 */
ctr_object *
ctr_clock_wait (ctr_object * myself, ctr_argument * argumentList)
{
  ctr_check_permission (CTR_SECPRO_COUNTDOWN);
  ctr_did_side_effect = 1;
  ctr_object *arg = ctr_internal_cast2number (argumentList->object);
  int n = (int) arg->value.nvalue;
  ctr_object *qual = ctr_internal_object_find_property (argumentList->object,
							ctr_build_string_from_cstring (CTR_DICT_QUALIFICATION),
							CTR_CATEGORY_PRIVATE_PROPERTY);
  if (qual)
    {
      char *qualf = ctr_heap_allocate_cstring (qual);
      if (strncasecmp (qualf, "ns", 2) == 0)
	{
	  const struct timespec rq = {
	    .tv_sec = 0,
	    .tv_nsec = n
	  };
	  nanosleep (&rq, NULL);
	}
      else if (strncasecmp (qualf, "us", 2) == 0)
	{
	  usleep (n);
	}
      else if (strncasecmp (qualf, "ms", 2) == 0)
	{
	  usleep (n * 1000);
	}
      else if (strncasecmp (qualf, "se", 2) == 0)
	{
	  sleep (n);
	}
      else if (strncasecmp (qualf, "mi", 2) == 0)
	{
	  const struct timespec rq = {
	    .tv_sec = n * 60,
	    .tv_nsec = 0
	  };
	  nanosleep (&rq, NULL);
	}
      else if (strncasecmp (qualf, "ho", 2) == 0)
	{
	  const struct timespec rq = {
	    .tv_sec = n * 60 * 60,
	    .tv_nsec = 0
	  };
	  nanosleep (&rq, NULL);
	}
      else
	{
	  return CtrStdNil;
	}
    }
  else
    usleep (n);
  return myself;
}

/**
 * [Clock] new: [Number].
 *
 * Creates a new clock instance from a UNIX time stamp.
 */
ctr_object *
ctr_clock_new_set (ctr_object * myself, ctr_argument * argumentList)
{
  ctr_object *clock;
  clock = ctr_clock_new (myself, argumentList);
  ctr_internal_object_add_property (clock,
				    ctr_build_string_from_cstring
				    (CTR_DICT_TIME), ctr_internal_cast2number (argumentList->object), CTR_CATEGORY_PRIVATE_PROPERTY);
  return clock;
}

/**
 * @internal
 */
ctr_object *
ctr_clock_get_time (ctr_object * myself, ctr_argument * argumentList, char part)
{
  struct tm *date;
  time_t timeStamp;
  ctr_object *answer;
  char *zone;
  timeStamp = (time_t)
    ctr_internal_cast2number (ctr_internal_object_find_property
			      (myself, ctr_build_string_from_cstring (CTR_DICT_TIME), CTR_CATEGORY_PRIVATE_PROPERTY))->value.nvalue;
  zone =
    ctr_heap_allocate_cstring (ctr_internal_cast2string
			       (ctr_internal_object_find_property
				(myself, ctr_build_string_from_cstring (CTR_DICT_ZONE), CTR_CATEGORY_PRIVATE_PROPERTY)));
  setenv ("TZ", zone, 1);
  date = localtime (&timeStamp);
  setenv ("TZ", "UTC", 1);
  ctr_did_side_effect = 1;
  switch (part)
    {
    case 'Y':
      answer = ctr_build_number_from_float ((ctr_number) date->tm_year + 1900);
      break;
    case 'm':
      answer = ctr_build_number_from_float ((ctr_number) date->tm_mon + 1);
      break;
    case 'd':
      answer = ctr_build_number_from_float ((ctr_number) date->tm_mday);
      break;
    case 'H':
      answer = ctr_build_number_from_float ((ctr_number) date->tm_hour);
      break;
    case 'i':
      answer = ctr_build_number_from_float ((ctr_number) date->tm_min);
      break;
    case 's':
      answer = ctr_build_number_from_float ((ctr_number) date->tm_sec);
      break;
    }
  ctr_heap_free (zone);
  return answer;
}

/**
 * @internal
 */
ctr_object *
ctr_clock_set_time (ctr_object * myself, ctr_argument * argumentList, char part)
{
  struct tm *date;
  time_t timeStamp;
  ctr_object *key;
  char *zone;
  key = ctr_build_string_from_cstring (CTR_DICT_TIME);
  timeStamp = (time_t) ctr_internal_cast2number (ctr_internal_object_find_property (myself, key, 0))->value.nvalue;
  zone =
    ctr_heap_allocate_cstring (ctr_internal_cast2string
			       (ctr_internal_object_find_property
				(myself, ctr_build_string_from_cstring (CTR_DICT_ZONE), CTR_CATEGORY_PRIVATE_PROPERTY)));
  setenv ("TZ", zone, 1);
  ctr_did_side_effect = 1;
  date = localtime (&timeStamp);
  switch (part)
    {
    case 'Y':
      date->tm_year = ctr_internal_cast2number (argumentList->object)->value.nvalue - 1900;
      break;
    case 'm':
      date->tm_mon = ctr_internal_cast2number (argumentList->object)->value.nvalue - 1;
      break;
    case 'd':
      date->tm_mday = ctr_internal_cast2number (argumentList->object)->value.nvalue;
      break;
    case 'H':
      date->tm_hour = ctr_internal_cast2number (argumentList->object)->value.nvalue;
      break;
    case 'i':
      date->tm_min = ctr_internal_cast2number (argumentList->object)->value.nvalue;
      break;
    case 's':
      date->tm_sec = ctr_internal_cast2number (argumentList->object)->value.nvalue;
      break;
    }
  ctr_heap_free (zone);
  ctr_internal_object_set_property (myself, key, ctr_build_number_from_float ((double_t) mktime (date)), 0);
  setenv ("TZ", "UTC", 1);
  return myself;
}

/**
 * [Clock] like: [Clock]
 *
 * Syncs a clock. Copies the time AND zone from the other clock.
 *
 * Usage:
 * clock := Clock new: timeStamp.
 * copyClock := Clock new like: clock.
 */
ctr_object *
ctr_clock_like (ctr_object * myself, ctr_argument * argumentList)
{
  ctr_object *otherClock;
  ctr_object *time;
  ctr_object *zone;
  otherClock = argumentList->object;
  time = ctr_internal_object_find_property (otherClock, ctr_build_string_from_cstring (CTR_DICT_TIME), CTR_CATEGORY_PRIVATE_PROPERTY);
  if (time == NULL)
    {
      time = ctr_build_number_from_float (0);
    }
  else
    {
      time = ctr_internal_cast2number (time);
    }
  zone = ctr_internal_object_find_property (otherClock, ctr_build_string_from_cstring (CTR_DICT_ZONE), CTR_CATEGORY_PRIVATE_PROPERTY);
  if (zone == NULL)
    {
      zone = ctr_build_string_from_cstring ("UTC");
    }
  else
    {
      zone = ctr_internal_cast2string (zone);
    }
  ctr_internal_object_set_property (myself, ctr_build_string_from_cstring (CTR_DICT_ZONE), zone, CTR_CATEGORY_PRIVATE_PROPERTY);
  ctr_internal_object_set_property (myself, ctr_build_string_from_cstring (CTR_DICT_TIME), time, CTR_CATEGORY_PRIVATE_PROPERTY);
  return myself;
}

/**
 * [Clock] zone: [String]
 *
 * Sets the time zone of the clock.
 */
ctr_object *
ctr_clock_set_zone (ctr_object * myself, ctr_argument * argumentList)
{
  ctr_internal_object_set_property (myself,
				    ctr_build_string_from_cstring
				    (CTR_DICT_ZONE), ctr_internal_cast2string (argumentList->object), CTR_CATEGORY_PRIVATE_PROPERTY);
  return myself;
}

/**
 * [Clock] zone
 *
 * Returns time zone of the clock.
 */
ctr_object *
ctr_clock_get_zone (ctr_object * myself, ctr_argument * argumentList)
{
  return ctr_internal_object_find_property (myself, ctr_build_string_from_cstring (CTR_DICT_ZONE), CTR_CATEGORY_PRIVATE_PROPERTY);
}

/**
 * [Clock] zone: [Number]
 *
 * Sets the year of the clock.
 */
ctr_object *
ctr_clock_set_year (ctr_object * myself, ctr_argument * argumentList)
{
  return ctr_clock_set_time (myself, argumentList, 'Y');
}

/**
 * [Clock] zone: [Number]
 *
 * Sets the month of the clock.
 */
ctr_object *
ctr_clock_set_month (ctr_object * myself, ctr_argument * argumentList)
{
  return ctr_clock_set_time (myself, argumentList, 'm');
}

/**
 * [Clock] zone: [Number]
 *
 * Sets the day of the clock.
 */
ctr_object *
ctr_clock_set_day (ctr_object * myself, ctr_argument * argumentList)
{
  return ctr_clock_set_time (myself, argumentList, 'd');
}

/**
 * [Clock] zone: [Number]
 *
 * Sets the hour of the clock.
 */
ctr_object *
ctr_clock_set_hour (ctr_object * myself, ctr_argument * argumentList)
{
  return ctr_clock_set_time (myself, argumentList, 'H');
}

/**
 * [Clock] zone: [Number]
 *
 * Sets the minute of the clock.
 */
ctr_object *
ctr_clock_set_minute (ctr_object * myself, ctr_argument * argumentList)
{
  return ctr_clock_set_time (myself, argumentList, 'i');
}

/**
 * [Clock] zone: [Number]
 *
 * Sets the second of the clock.
 */
ctr_object *
ctr_clock_set_second (ctr_object * myself, ctr_argument * argumentList)
{
  return ctr_clock_set_time (myself, argumentList, 's');
}

/**
 * [Clock] year
 *
 * Returns year of the clock.
 */
ctr_object *
ctr_clock_year (ctr_object * myself, ctr_argument * argumentList)
{
  return ctr_clock_get_time (myself, argumentList, 'Y');
}

/**
 * [Clock] month
 *
 * Returns month of the clock.
 */
ctr_object *
ctr_clock_month (ctr_object * myself, ctr_argument * argumentList)
{
  return ctr_clock_get_time (myself, argumentList, 'm');
}

/**
 * [Clock] day
 *
 * Returns day of the clock.
 */
ctr_object *
ctr_clock_day (ctr_object * myself, ctr_argument * argumentList)
{
  return ctr_clock_get_time (myself, argumentList, 'd');
}

/**
 * [Clock] hour
 *
 * Returns hour of the clock.
 */
ctr_object *
ctr_clock_hour (ctr_object * myself, ctr_argument * argumentList)
{
  return ctr_clock_get_time (myself, argumentList, 'H');
}

/**
 * [Clock] minute
 *
 * Returns minute of the clock.
 */
ctr_object *
ctr_clock_minute (ctr_object * myself, ctr_argument * argumentList)
{
  return ctr_clock_get_time (myself, argumentList, 'i');
}

/**
 * [Clock] second
 *
 * Returns second of the clock.
 */
ctr_object *
ctr_clock_second (ctr_object * myself, ctr_argument * argumentList)
{
  return ctr_clock_get_time (myself, argumentList, 's');
}

/**
 * [Clock] yearday
 *
 * Returns day number of the year.
 */
ctr_object *
ctr_clock_yearday (ctr_object * myself, ctr_argument * argumentList)
{
  struct tm *date;
  time_t timeStamp;
  timeStamp = (time_t)
    ctr_internal_cast2number (ctr_internal_object_find_property (myself, ctr_build_string_from_cstring (CTR_DICT_TIME), 0))->value.nvalue;
  date = localtime (&timeStamp);
  return ctr_build_number_from_float ((double_t) date->tm_yday);
}

/**
 * [Clock] weekday
 *
 * Returns the week day number of the clock.
 */
ctr_object *
ctr_clock_weekday (ctr_object * myself, ctr_argument * argumentList)
{
  struct tm *date;
  time_t timeStamp;
  timeStamp = (time_t)
    ctr_internal_cast2number (ctr_internal_object_find_property (myself, ctr_build_string_from_cstring (CTR_DICT_TIME), 0))->value.nvalue;
  date = localtime (&timeStamp);
  return ctr_build_number_from_float ((double_t) date->tm_wday);
}

/**
 * [Clock] time.
 *
 * Returns the UNIX time stamp representation of the time.
 * Note: this is the time OF CREATION OF THE OBJECT. To get the actual time use:
 *
 * [Clock] new time.
 */
ctr_object *
ctr_clock_time (ctr_object * myself, ctr_argument * argumentList)
{
  time_t timeStamp;
  timeStamp = (time_t)
    ctr_internal_cast2number (ctr_internal_object_find_property (myself, ctr_build_string_from_cstring (CTR_DICT_TIME), 0))->value.nvalue;
  return ctr_build_number_from_float ((double_t) timeStamp);
}

/**
 * [Clock] week
 *
 * Returns the week number of the clock.
 */
ctr_object *
ctr_clock_week (ctr_object * myself, ctr_argument * argumentList)
{
  ctr_object *weekNumber;
  char *str;
  time_t timeStamp;
  timeStamp = (time_t)
    ctr_internal_cast2number (ctr_internal_object_find_property (myself, ctr_build_string_from_cstring (CTR_DICT_TIME), 0))->value.nvalue;
  str = ctr_heap_allocate (4);
  strftime (str, 3, "%W", localtime (&timeStamp));
  weekNumber = ctr_internal_cast2number (ctr_build_string_from_cstring (str));
  ctr_heap_free (str);
  return weekNumber;
}

/**
 * [Clock] format: [String]
 *
 * Returns a string describing the date and time represented by the clock object
 * according to the specified format. See strftime for format syntax details.
 */
ctr_object *
ctr_clock_format (ctr_object * myself, ctr_argument * argumentList)
{
  char *zone;
  char *description;
  ctr_object *answer;
  time_t timeStamp;
  char *format;
  format = ctr_heap_allocate_cstring (ctr_internal_cast2string (argumentList->object));
  zone =
    ctr_heap_allocate_cstring (ctr_internal_cast2string
			       (ctr_internal_object_find_property
				(myself, ctr_build_string_from_cstring (CTR_DICT_ZONE), CTR_CATEGORY_PRIVATE_PROPERTY)));
  timeStamp = (time_t)
    ctr_internal_cast2number (ctr_internal_object_find_property (myself, ctr_build_string_from_cstring (CTR_DICT_TIME), 0))->value.nvalue;
  description = ctr_heap_allocate (41);
  setenv ("TZ", zone, 1);
  strftime (description, 40, format, localtime (&timeStamp));
  setenv ("TZ", SystemTZ, 1);
  answer = ctr_build_string_from_cstring (description);
  ctr_heap_free (description);
  ctr_heap_free (format);
  ctr_heap_free (zone);
  return answer;
}

/**
 * [Clock] toString
 *
 * Returns a string describing the date and time
 * represented by the clock object.
 */
ctr_object *
ctr_clock_to_string (ctr_object * myself, ctr_argument * argumentList)
{
  ctr_argument *newArgumentList;
  ctr_object *answer;
  newArgumentList = ctr_heap_allocate (sizeof (ctr_argument));
  newArgumentList->object = ctr_build_string_from_cstring ("%Y-%m-%d %H:%M:%S");
  answer = ctr_clock_format (myself, newArgumentList);
  ctr_heap_free (newArgumentList);
  return answer;
}

ctr_object *
ctr_clock_isdst (ctr_object * myself, ctr_argument * argumentList)
{
  time_t timestamp;
  time(&timestamp);
  struct tm *timeinfo = localtime(&timestamp);
  return ctr_build_bool(timeinfo->tm_isdst);
}
/**
 * @internal
 */
void
ctr_clock_init (ctr_object * clock)
{
  ctr_internal_object_add_property (clock, ctr_build_string_from_cstring (CTR_DICT_TIME), ctr_build_number_from_float ((double_t) time (NULL)), 0);
  ctr_internal_object_add_property (clock, ctr_build_string_from_cstring (CTR_DICT_ZONE), ctr_build_string_from_cstring (SystemTZ), 0);
}

/**
 * @internal
 */
ctr_object *
ctr_clock_change (ctr_object * myself, ctr_argument * argumentList, uint8_t forward)
{
  ctr_number number;
  ctr_object *numberObject;
  ctr_object *qual;
  char *zone;
  ctr_object *timeObject;
  ctr_size l;
  time_t time;
  char *unit;
  struct tm *date;
  numberObject = ctr_internal_cast2number (argumentList->object);
  number = numberObject->value.nvalue * (forward ? 1 : -1);
  qual =
    ctr_internal_object_find_property (argumentList->object, ctr_build_string_from_cstring (CTR_DICT_QUALIFICATION), CTR_CATEGORY_PRIVATE_PROPERTY);
  if (qual == NULL)
    {
      return myself;
    }
  qual = ctr_internal_cast2string (qual);
  unit = qual->value.svalue->value;
  l = qual->value.svalue->vlen;
  timeObject = ctr_internal_object_find_property (myself, ctr_build_string_from_cstring (CTR_DICT_TIME), CTR_CATEGORY_PRIVATE_PROPERTY);
  if (timeObject == NULL)
    {
      return myself;
    }
  time = (time_t) ctr_internal_cast2number (timeObject)->value.nvalue;
  zone =
    ctr_heap_allocate_cstring (ctr_internal_cast2string
			       (ctr_internal_object_find_property
				(myself, ctr_build_string_from_cstring (CTR_DICT_ZONE), CTR_CATEGORY_PRIVATE_PROPERTY)));
  setenv ("TZ", zone, 1);
  date = localtime (&time);
  if (strncmp (unit, CTR_DICT_HOURS, l) == 0 || strncmp (unit, CTR_DICT_HOUR, l) == 0 || strncmp (unit, CTR_DICT_HOURS_ABBR, l) == 0)
    {
      date->tm_hour += number;
    }
  else if (strncmp (unit, CTR_DICT_MINUTES, l) == 0 || strncmp (unit, CTR_DICT_MINUTE, l) == 0 || strncmp (unit, CTR_DICT_MINUTES_ABBR, l) == 0)
    {
      date->tm_min += number;
    }
  else if (strncmp (unit, CTR_DICT_SECONDS, l) == 0 || strncmp (unit, CTR_DICT_SECOND, l) == 0 || strncmp (unit, CTR_DICT_SECONDS_ABBR, l) == 0)
    {
      date->tm_sec += number;
    }
  else if (strncmp (unit, CTR_DICT_DAYS, l) == 0 || strncmp (unit, CTR_DICT_DAY, l) == 0)
    {
      date->tm_mday += number;
    }
  else if (strncmp (unit, CTR_DICT_MONTHS, l) == 0 || strncmp (unit, CTR_DICT_MONTH, l) == 0)
    {
      date->tm_mon += number;
    }
  else if (strncmp (unit, CTR_DICT_YEARS, l) == 0 || strncmp (unit, CTR_DICT_YEAR, l) == 0)
    {
      date->tm_year += number;
    }
  else if (strncmp (unit, CTR_DICT_WEEKS, l) == 0 || strncmp (unit, CTR_DICT_WEEK, l) == 0)
    {
      date->tm_mday += number * 7;
    }
  ctr_internal_object_set_property (myself,
				    ctr_build_string_from_cstring
				    (CTR_DICT_TIME), ctr_build_number_from_float ((ctr_number) mktime (date)), CTR_CATEGORY_PRIVATE_PROPERTY);
  setenv ("TZ", "UTC", 1);
  ctr_heap_free (zone);
  return myself;
}

/**
 * [Clock] add: [Number].
 *
 * Adds the number to the clock, updating its time accordingly.
 * Note that this is typically used with a qualifier.
 * If the qualifier is 'hours' the number is treated as hours and
 * the specified number of hours will be added to the time.
 *
 * The Clock object understands the following qualifiers
 * if the selected language is English:
 *
 * sec, second, seconds,
 * min, minute, minutes,
 * hrs, hour, hours,
 * day, days,
 * week, weeks,
 * month, months,
 * year, years
 *
 * Note that it does not matter which form you use, 2 hour means
 * the same as 2 hours (plural).
 *
 * Usage:
 *
 * clock add: 3 minutes. #adds 3 minutes
 * clock add: 1 hour.    #adds 1 hour
 * clock add: 2 second.  #adds 2 seconds
 */
ctr_object *
ctr_clock_add (ctr_object * myself, ctr_argument * argumentList)
{
  return ctr_clock_change (myself, argumentList, 1);
}

/**
 * [Clock] subtract: [Number].
 *
 * Same as '[Clock] add:' but subtracts the number instead of adding it to
 * the clock's time.
 */
ctr_object *
ctr_clock_subtract (ctr_object * myself, ctr_argument * argumentList)
{
  return ctr_clock_change (myself, argumentList, 0);
}

/**
 * [Clock] new
 *
 * Creates a new clock, by default a clock will be set to
 * the UTC timezone having the current time.
 */
ctr_object *
ctr_clock_new (ctr_object * myself, ctr_argument * argumentList)
{
  ctr_object *clock;
  clock = ctr_internal_create_object (CTR_OBJECT_TYPE_OTOBJECT);
  ctr_set_link_all (clock, myself);
  ctr_clock_init (clock);
  return clock;
}

/**
 * [Clock] processorClock
 *
 * returns the number of processor ticks since the beginning of this program.
 * Note that this will be reset depending on the processor
 */
ctr_object *
ctr_clock_processor_time (ctr_object * myself, ctr_argument * argumentList)
{
  ctr_object *ptime = ctr_build_number_from_float ((double) clock ());
  return ptime;
}

/**
 * [Clock] ticksPerSecond
 *
 * returns the count of processor ticks in a second
 */
ctr_object *
ctr_clock_processor_ticks_ps (ctr_object * myself, ctr_argument * argumentList)
{
  ctr_object *tps = ctr_build_number_from_float ((double) CLOCKS_PER_SEC);
  return tps;
}

/**
 * [Clock] timeExecutionOf: [Block]
 *
 * How long does the execution of the block take? (returns in cpu ticks)
 */
ctr_object *
ctr_clock_time_exec (ctr_object * myself, ctr_argument * argumentList)
{
  ctr_argument *args = ctr_heap_allocate (sizeof (ctr_argument));
  long int init = clock ();
  ctr_block_runIt (argumentList->object, args);
  long int end = clock ();
  ctr_heap_free (args);
  ctr_did_side_effect = 1;
  return ctr_build_number_from_float ((end - init));
}

/**
 * [Clock] executionSeconds: [Block]
 *
 * How long does the execution of the block take? (returns in seconds)
 */
ctr_object *
ctr_clock_time_exec_s (ctr_object * myself, ctr_argument * argumentList)
{
  ctr_argument *args = ctr_heap_allocate (sizeof (ctr_argument));
  long int init = clock ();
  ctr_block_runIt (argumentList->object, args);
  long int end = clock ();
  ctr_heap_free (args);
  ctr_did_side_effect = 1;
  return ctr_build_number_from_float ((((double) (end - init)) / (double) CLOCKS_PER_SEC));
}

/**@I_OBJ_DEF Pen*/

/**
 * [Pen] write: [String]
 *
 * Writes string to console.
 */
ctr_object *
ctr_console_write (ctr_object * myself, ctr_argument * argumentList)
{
  ctr_object *argument1 = argumentList->object;
  ctr_object *strObject = ctr_internal_cast2string (argument1);
  fwrite (strObject->value.svalue->value, strObject->value.svalue->vlen, 1, stdout);
  ctr_did_side_effect = 1;
  return myself;
}

/**
 * [Pen] writeln: [String]
 *
 * Writes string to console, with a newline at the end.
 */
ctr_object *
ctr_console_writeln (ctr_object * myself, ctr_argument * argumentList)
{
  ctr_object *argument1 = argumentList->object;
  ctr_object *strObject = ctr_internal_cast2string (argument1);
  fwrite (strObject->value.svalue->value, strObject->value.svalue->vlen, 1, stdout);
  fwrite ("\n", 1, 1, stdout);
  ctr_did_side_effect = 1;
  return myself;
}

/**
 * [Pen] brk
 *
 * Outputs a newline character.
 */
ctr_object *
ctr_console_brk (ctr_object * myself, ctr_argument * argumentList)
{
  fwrite ("\n", sizeof (char), 1, stdout);
  ctr_did_side_effect = 1;
  return myself;
}

ctr_object *
ctr_console_red (ctr_object * myself, ctr_argument * argumentList)
{
  fwrite (CTR_ANSI_COLOR_RED, sizeof (char), strlen (CTR_ANSI_COLOR_RED), stdout);
  ctr_did_side_effect = 1;
  return myself;
}

ctr_object *
ctr_console_green (ctr_object * myself, ctr_argument * argumentList)
{
  fwrite (CTR_ANSI_COLOR_GREEN, sizeof (char), strlen (CTR_ANSI_COLOR_GREEN), stdout);
  ctr_did_side_effect = 1;
  return myself;
}

ctr_object *
ctr_console_yellow (ctr_object * myself, ctr_argument * argumentList)
{
  fwrite (CTR_ANSI_COLOR_YELLOW, sizeof (char), strlen (CTR_ANSI_COLOR_YELLOW), stdout);
  ctr_did_side_effect = 1;
  return myself;
}

ctr_object *
ctr_console_blue (ctr_object * myself, ctr_argument * argumentList)
{
  fwrite (CTR_ANSI_COLOR_BLUE, sizeof (char), strlen (CTR_ANSI_COLOR_BLUE), stdout);
  ctr_did_side_effect = 1;
  return myself;
}

ctr_object *
ctr_console_magenta (ctr_object * myself, ctr_argument * argumentList)
{
  fwrite (CTR_ANSI_COLOR_MAGENTA, sizeof (char), strlen (CTR_ANSI_COLOR_MAGENTA), stdout);
  ctr_did_side_effect = 1;
  return myself;
}

ctr_object *
ctr_console_cyan (ctr_object * myself, ctr_argument * argumentList)
{
  fwrite (CTR_ANSI_COLOR_CYAN, sizeof (char), strlen (CTR_ANSI_COLOR_CYAN), stdout);
  ctr_did_side_effect = 1;
  return myself;
}

ctr_object *
ctr_console_reset (ctr_object * myself, ctr_argument * argumentList)
{
  fwrite (CTR_ANSI_COLOR_RESET, sizeof (char), strlen (CTR_ANSI_COLOR_RESET), stdout);
  ctr_did_side_effect = 1;
  return myself;
}

ctr_object *
ctr_console_tab (ctr_object * myself, ctr_argument * argumentList)
{
  fwrite ("\t", sizeof (char), strlen ("\t"), stdout);
  ctr_did_side_effect = 1;
  return myself;
}

ctr_object *
ctr_console_line (ctr_object * myself, ctr_argument * argumentList)
{
  char *line = "---------------------------------------\n";
  fwrite (line, sizeof (char), strlen (line), stdout);
  ctr_did_side_effect = 1;
  return myself;
}

#if 1
ctr_object *
ctr_console_clear (ctr_object * myself, ctr_argument * argumentList)
{
  char *line = "\033[H\033[J";
  fwrite (line, sizeof (char), strlen (line), stdout);
  ctr_did_side_effect = 1;
  return myself;
}

ctr_object *
ctr_console_clear_line (ctr_object * myself, ctr_argument * argumentList)
{
  char *line = "\033[2K\r";
  fwrite (line, sizeof (char), 0, stdout);
  ctr_did_side_effect = 1;
  return myself;
}
#else
ctr_object *
ctr_console_clear (ctr_object * myself, ctr_argument * argumentList)
{
  char *line = "";
  fwrite (line, sizeof (char), strlen (line), stdout);
  ctr_did_side_effect = 1;
  return myself;
}

ctr_object *
ctr_console_clear_line (ctr_object * myself, ctr_argument * argumentList)
{
  char *line = "";
  fwrite (line, sizeof (char), 0, stdout);
  ctr_did_side_effect = 1;
  return myself;
}
#endif

/**@I_OBJ_DEF Thread*/

typedef struct
{
  ctr_object *retval;
  ctr_object *stdFlow;
} ctr_thread_return_t;

typedef struct
{
  ctr_object *target;
  ctr_object *args;
  ctr_thread_return_t *last_result;	//please leave null
  pthread_mutex_t *mutex;
  pthread_t *thread;
} ctr_thread_t;
static pthread_mutex_t GLOBAL_MUTEX = PTHREAD_MUTEX_INITIALIZER;
void *
ctr_run_thread_func (ctr_thread_t * threadt)
{
  ctr_did_side_effect = 1;
  sigset_t set;
  sigfillset (&set);
  sigset_t oset;
  ctr_thread_return_t *rv = ctr_heap_allocate (sizeof (ctr_thread_return_t));
  // ctr_object* ctx = ctr_internal_create_object(CTR_OBJECT_TYPE_OTOBJECT);
  pthread_mutex_lock (&GLOBAL_MUTEX);
  pthread_mutex_lock (threadt->mutex);
  // ctr_object* oct = ctr_contexts[ctr_context_id];
  int sets = pthread_sigmask (SIG_SETMASK, &set, &oset);
  // ctr_switch_context(ctx);

  ctr_argument *args = ctr_heap_allocate (sizeof (ctr_argument));
  (void) ctr_array_to_argument_list (threadt->args, args);
  ctr_object *result = ctr_block_run_here (threadt->target, args, threadt->target);
  ctr_deallocate_argument_list (args);

  // ctr_switch_context(oct);
  rv->retval = result;
  rv->stdFlow = CtrStdFlow;
  threadt->last_result = rv;
  sets = pthread_sigmask (SIG_SETMASK, &oset, NULL);
  pthread_mutex_unlock (threadt->mutex);
  pthread_mutex_unlock (&GLOBAL_MUTEX);
  pthread_exit (rv);
}

void *
ctr_thread_free_res (void *res)
{
  ctr_did_side_effect = 1;
  if (likely (res))
    {
      ctr_thread_t *tres = res;
      pthread_mutex_destroy (tres->mutex);
      if (tres->last_result)
	ctr_heap_free (tres->last_result);
      ctr_heap_free (res);
    }
  return NULL;
}

ctr_size thread_current_number = 1;
/**
 * [Thread] new
 *
 * creates a new thread (no target)
 */
ctr_object *
ctr_thread_make (ctr_object * myself, ctr_argument * argumentList)
{
  ctr_object *inst = ctr_internal_create_object (CTR_OBJECT_TYPE_OTEX);
  ctr_set_link_all (inst, myself);
  inst->value.rvalue = ctr_heap_allocate (sizeof (ctr_resource));
  inst->release_hook = &ctr_thread_free_res;
  return inst;
}

/**
 * [Thread] target: [Block]
 *
 * set the target of the thread instance
 */
ctr_object *
ctr_thread_set_target (ctr_object * myself, ctr_argument * argumentList)
{
  ctr_did_side_effect = 1;
  pthread_t *thread;
  int err;
  if (!myself->value.rvalue->ptr)
    {
      ctr_thread_t *thdesc = ctr_heap_allocate (sizeof (ctr_thread_t));
      pthread_mutex_t *mutex = ctr_heap_allocate (sizeof (pthread_mutex_t));
      if ((err = pthread_mutex_init (mutex, NULL)) != 0)
	{
	  //Error
	  printf ("Bitching about mutex %p (cannot init (%s))\n", mutex, strerror (err));
	}
      thread = ctr_heap_allocate (sizeof (pthread_t));
      thdesc->mutex = mutex;
      thdesc->thread = thread;
      myself->value.rvalue->ptr = thdesc;
    }
  else
    thread = ((ctr_thread_t *) myself->value.rvalue->ptr)->thread;
  ((ctr_thread_t *) myself->value.rvalue->ptr)->target = argumentList->object;
  pthread_mutex_lock (((ctr_thread_t *) myself->value.rvalue->ptr)->mutex);
  pthread_create (thread, NULL, (voidptrfn_t *) & ctr_run_thread_func, myself->value.rvalue->ptr);
  char name[16];
  char pname[16];
  pthread_getname_np (pthread_self (), pname, 16);
  sprintf (name, "%.*s-%.*d", 15 - sizeof (ctr_size), pname, sizeof (ctr_size), thread_current_number % (sizeof (ctr_size)));
  thread_current_number++;
  if ((err = pthread_setname_np (*thread, name)) != 0)
    {
      //
    }
  return myself;
}

/**
 * [Thread] args: [Block]
 *
 * set the args of the thread instance
 */
ctr_object *
ctr_thread_set_args (ctr_object * myself, ctr_argument * argumentList)
{
  ctr_did_side_effect = 1;
  pthread_t *thread;
  int err;
  if (!myself->value.rvalue->ptr)
    {
      ctr_thread_t *thdesc = ctr_heap_allocate (sizeof (ctr_thread_t));
      pthread_mutex_t *mutex = ctr_heap_allocate (sizeof (pthread_mutex_t));
      if ((err = pthread_mutex_init (mutex, NULL)) != 0)
	{
	  //Error
	  printf ("Bitching about mutex %p (cannot init (%s))\n", mutex, strerror (err));
	}
      thread = ctr_heap_allocate (sizeof (pthread_t));
      thdesc->mutex = mutex;
      thdesc->thread = thread;
      myself->value.rvalue->ptr = thdesc;
    }
  else
    thread = ((ctr_thread_t *) myself->value.rvalue->ptr)->thread;
  ((ctr_thread_t *) myself->value.rvalue->ptr)->args = argumentList->object;
  pthread_mutex_lock (((ctr_thread_t *) myself->value.rvalue->ptr)->mutex);
  pthread_create (thread, NULL, (voidptrfn_t *) & ctr_run_thread_func, myself->value.rvalue->ptr);
  char name[16];
  char pname[16];
  pthread_getname_np (pthread_self (), pname, 16);
  sprintf (name, "%.*s-%.*d", 15 - sizeof (ctr_size), pname, sizeof (ctr_size), thread_current_number % (sizeof (ctr_size)));
  thread_current_number++;
  if ((err = pthread_setname_np (*thread, name)) != 0)
    {
      //
    }
  return myself;
}

/**
 * [Thread] new: [Block] [args: [Array]]
 *
 * create a new thread with a target and optionally some arguments
 */
ctr_object *
ctr_thread_make_set_target (ctr_object * myself, ctr_argument * argumentList)
{
  ctr_did_side_effect = 1;
  ctr_object *inst = ctr_internal_create_object (CTR_OBJECT_TYPE_OTEX);
  ctr_set_link_all (inst, CtrStdThread);
  inst->value.rvalue = ctr_heap_allocate (sizeof (ctr_resource));
  inst->release_hook = &ctr_thread_free_res;
  pthread_t *thread;
  ctr_thread_t *thdesc = ctr_heap_allocate (sizeof (ctr_thread_t));
  pthread_mutex_t *mutex = ctr_heap_allocate (sizeof (pthread_mutex_t));
  int err;
  if ((err = pthread_mutex_init (mutex, NULL)) != 0)
    {
      //Error
      printf ("Bitching about mutex %p (cannot init (%s))\n", mutex, strerror (err));
    }
  thread = ctr_heap_allocate (sizeof (pthread_t));
  thdesc->mutex = mutex;
  thdesc->thread = thread;
  if (argumentList->next && argumentList->next->object)
    thdesc->args = argumentList->next->object;
  else
    thdesc->args = NULL;
  inst->value.rvalue->ptr = thdesc;
  ((ctr_thread_t *) inst->value.rvalue->ptr)->target = argumentList->object;
  pthread_mutex_lock (((ctr_thread_t *) inst->value.rvalue->ptr)->mutex);
  pthread_create (thread, NULL, (voidptrfn_t *) & ctr_run_thread_func, inst->value.rvalue->ptr);
  char name[16];
  char pname[16];
  pthread_getname_np (pthread_self (), pname, 16);
  sprintf (name, "%.*s-%.*d", 15 - sizeof (ctr_size), pname, sizeof (ctr_size), thread_current_number % (sizeof (ctr_size)));
  thread_current_number++;
  if ((err = pthread_setname_np (*thread, name)) != 0)
    {
      //
    }
  return inst;
}

/**
 * [Thread] run
 *
 * runs the thread
 */
ctr_object *
ctr_thread_run (ctr_object * myself, ctr_argument * argumentList)
{
  ctr_did_side_effect = 1;
  if (!myself->value.rvalue->ptr)
    {
      CtrStdFlow = ctr_build_string_from_cstring ("Attempt to run a thread without a target");
      return CtrStdFlow;
    }
  if (!pthread_mutex_trylock (&GLOBAL_MUTEX))
    {
      pthread_mutex_unlock (&GLOBAL_MUTEX);
      pthread_mutex_unlock (((ctr_thread_t *) myself->value.rvalue->ptr)->mutex);
    }
  return myself;
}

/**
 * [Thread] join
 *
 * joins the thread
 */
ctr_object *
ctr_thread_join (ctr_object * myself, ctr_argument * argumentList)
{
  ctr_did_side_effect = 1;
  ctr_thread_return_t *retval;
  // pthread_mutex_lock(((ctr_thread_t*)myself->value.rvalue->ptr)->mutex);//get the mutex
  if (pthread_join (*(((ctr_thread_t *) myself->value.rvalue->ptr)->thread), (void **) &retval) != 0)
    {
      CtrStdFlow = ctr_build_string_from_cstring ("Thread could not join");
      return CtrStdNil;
    }
  if (/*retval == PTHREAD_CANCELED ||*/ !retval)
    {
      ctr_heap_free (retval);
      return CtrStdNil;
    }
  else
    {
      if (retval->stdFlow)
	{
	  CtrStdFlow = retval->stdFlow;
	  ctr_heap_free (retval);
	  return CtrStdNil;
	}
      ctr_object* rvt = retval->retval;
      ctr_heap_free (retval);
      if (rvt == NULL) return CtrStdNil;
      return rvt;
    }
}

/**
 * [Thread] id
 *
 * returns the thread ID
 */
ctr_object *
ctr_thread_id (ctr_object * myself, ctr_argument * argumentList)
{
  pthread_t id = pthread_self ();
  return ctr_build_number_from_float (*(uint64_t *) & id);
}

/**
 * [Thread] name [: [String]]
 *
 * returns the name of a thread (or alternatively sets it)
 */
ctr_object *
ctr_thread_name (ctr_object * myself, ctr_argument * argumentList)
{
  ctr_did_side_effect = 1;
  pthread_t *threadt;
  if (myself->value.rvalue)
    threadt = ((ctr_thread_t *) myself->value.rvalue->ptr)->thread;
  else
    {
      threadt = ctr_heap_allocate (sizeof (ctr_thread_t));
      *threadt = pthread_self ();
    }
  char name[16];
  int err;
  if ((err = pthread_getname_np (*threadt, name, 16)) != 0)
    {
      char errbuf[1024];
      sprintf (errbuf, "Error when reading thread name: %s", strerror (err));
      // CtrStdFlow = ctr_build_string_from_cstring(errbuf);
      return CtrStdNil;
    }
  return ctr_build_string_from_cstring (name);
}

ctr_object *
ctr_thread_names (ctr_object * myself, ctr_argument * argumentList)
{
  ctr_did_side_effect = 1;
  pthread_t *threadt;
  if (myself->value.rvalue)
    threadt = ((ctr_thread_t *) myself->value.rvalue->ptr)->thread;
  else
    {
      threadt = ctr_heap_allocate (sizeof (ctr_thread_t));
      *threadt = pthread_self ();
    }
  ctr_object *str;
  if ((str = ctr_internal_cast2string (argumentList->object))->value.svalue->vlen > 15)
    {
      CtrStdFlow = ctr_build_string_from_cstring ("Thread names may at most be 15 bytes in length");
      return CtrStdNil;
    }
  char *name = ctr_heap_allocate_cstring (str);
  int err;
  if ((err = pthread_setname_np (*threadt, name)) != 0)
    {
      char errbuf[1024];
      sprintf (errbuf, "Error when setting thread name: %s", strerror (err));
      CtrStdFlow = ctr_build_string_from_cstring (errbuf);
      return CtrStdNil;
    }
  ctr_heap_free (name);
  return myself;
}

ctr_object *
ctr_thread_to_string (ctr_object * myself, ctr_argument * argumentList)
{
  pthread_t *threadt;
  if (myself->value.rvalue)
    {
      if (!myself->value.rvalue->ptr)
	return ctr_build_string_from_cstring ("<Uninitialized Thread>");
      threadt = ((ctr_thread_t *) myself->value.rvalue->ptr)->thread;
    }
  else
    {
      threadt = ctr_heap_allocate (sizeof (ctr_thread_t));
      *threadt = pthread_self ();
    }
  char name[16];
  int err;
  if ((err = pthread_getname_np (*threadt, name, 16)) != 0)
    {
      char errbuf[1024];
      sprintf (errbuf, "Error when reading thread name: %s", strerror (err));
      // CtrStdFlow = ctr_build_string_from_cstring(errbuf);
      return CtrStdNil;
    }
  char buf[64];
  sprintf (buf, "<Thread %s>", name);
  return ctr_build_string_from_cstring (buf);
}
