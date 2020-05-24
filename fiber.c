#include "citron.h"

/*
 * Fiber Impl suited for citron blocks
 */

/* HEADER> */
#ifndef LIBFIBER_H
#define LIBFIBER_H 1

#define LF_NOERROR 0
#define LF_MAXFIBERS 1
#define LF_MALLOCERROR 2
#define LF_CLONEERROR 3
#define LF_INFIBER 4
#define LF_SIGNALERROR 5

// Define a debugging output macro
// #define LF_DEBUG 1
#ifdef LF_DEBUG

#include <stdio.h>
#define LF_DEBUG_OUT(string, arg)                                              \
  fprintf(stderr, "libfiber debug: " string "\n", arg)

#else

#define LF_DEBUG_OUT(string, arg)

#endif

/* The maximum number of fibers that can be active at once. */
#define MAX_FIBERS 1024
/* The size of the stack for each fiber. */
#define FIBER_STACK (1024 * 1024)

// Should be called before executing any of the other functions
extern void initFibers();

// Creates a new fiber, running the function that is passed as an argument.
extern int spawnFiber(ctr_object *block);

// Yield control to another execution context
extern void fiberYield();

// Execute the fibers until they all quit
extern int waitForAllFibers();

#endif
/* <HEADER */

#include <malloc.h>
#include <ucontext.h>

ctr_object *FIBER_YIELDED = NULL; // to pass values around

/* The Fiber Structure
 *  Contains the information about individual fibers.
 */
typedef struct {
  ucontext_t context; // Stores the current context
  struct ctr_context_t ctr_context;
  int active : 1; // A boolean flag, 0 if it is not active, 1 if it is
} fiber;

// The fiber "queue"
fiber fiberList[MAX_FIBERS];

// The index of the currently executing fiber
static int currentFiber = -1;
// A boolean flag indicating if we are in the main process or if we are in a
// fiber
static int inFiber = 0;
// The number of active fibers
static int numFibers = 0;

// The "main" execution context
static ucontext_t mainContext;

// Sets all the fibers to be initially inactive
void initFibers() {
  int i;
  for (i = 0; i < MAX_FIBERS; ++i) {
    fiberList[i].active = 0;
  }

  return;
}

// Switches from a fiber to main or from main to a fiber
void fiberYield() {
  // If we are in a fiber, switch to the main process
  if (inFiber) {
    // Switch to the main context
    LF_DEBUG_OUT("libfiber debug: Fiber %d yielding the processor...",
                 currentFiber);

    swapcontext(&fiberList[currentFiber].context, &mainContext);
  }
  // Else, we are in the main process and we need to dispatch a new fiber
  else {
    if (numFibers == 0)
      return;

    // Saved the state to call the next fiber
    currentFiber = (currentFiber + 1) % numFibers;
    struct ctr_context_t ctx;
    ctr_dump_context(&ctx);
    LF_DEBUG_OUT("Switching to fiber %d.", currentFiber);
    inFiber = 1;
    ctr_load_context(fiberList[currentFiber].ctr_context);
    swapcontext(&mainContext, &fiberList[currentFiber].context);
    inFiber = 0;
    ctr_load_context(ctx);
    LF_DEBUG_OUT("Fiber %d switched to main context.", currentFiber);

    if (fiberList[currentFiber].active == 0) {
      LF_DEBUG_OUT("Fiber %d is finished. Cleaning up.\n", currentFiber);
      // Free the "current" fiber's stack
      free(fiberList[currentFiber].context.uc_stack.ss_sp);

      // Swap the last fiber with the current, now empty the entry
      --numFibers;
      if (currentFiber != numFibers) {
        fiberList[currentFiber] = fiberList[numFibers];
      }
      fiberList[numFibers].active = 0;
    }
  }
  return;
}

// Records when the fiber has started and when it is done
// so that we know when to free its resources. It is called in the fiber's
// context of execution.
static void fiberStart(ctr_object *block) {
  fiberList[currentFiber].active = 1;
  ctr_block_run_here(block, NULL, block);
  fiberList[currentFiber].active = 0;

  // Yield control, but because active == 0, this will free the fiber
  fiberYield();
}

int spawnFiber(ctr_object *block) {
  if (numFibers == MAX_FIBERS)
    return LF_MAXFIBERS;

  // Add the new function to the end of the fiber list
  getcontext(&fiberList[numFibers].context);

  // Set the context to a newly allocated stack
  fiberList[numFibers].context.uc_link = 0;
  fiberList[numFibers].context.uc_stack.ss_sp = malloc(FIBER_STACK);
  fiberList[numFibers].context.uc_stack.ss_size = FIBER_STACK;
  fiberList[numFibers].context.uc_stack.ss_flags = 0;

  ctr_dump_context(&fiberList[numFibers].ctr_context);

  if (fiberList[numFibers].context.uc_stack.ss_sp == 0) {
    LF_DEBUG_OUT("Error: Could not allocate stack.", 0);
    return LF_MALLOCERROR;
  }
  // Create the context. The context calls fiberStart( func ).
  makecontext(&fiberList[numFibers].context, (void (*)(void)) & fiberStart, 1,
              block);
  ++numFibers;

  return LF_NOERROR;
}

int waitForAllFibers() {
  int fibersRemaining = 0;

  // If we are in a fiber, wait for all the *other* fibers to quit
  if (inFiber)
    fibersRemaining = 1;

  LF_DEBUG_OUT("Waiting until there are only %d threads remaining...",
               fibersRemaining);

  // Execute the fibers until they quit
  while (numFibers > fibersRemaining)
    fiberYield();

  return LF_NOERROR;
}

int waitForFiber(int id) {
  int fibersRemaining = id;

  // If we are in a fiber, wait for all the *other* fibers to quit
  if (inFiber)
    fibersRemaining = id + 1;

  LF_DEBUG_OUT("Waiting until there are only %d threads remaining...",
               fibersRemaining);

  // Execute the fibers until they quit
  while (numFibers > fibersRemaining)
    fiberYield();

  return LF_NOERROR;
}

int yieldNTimes(int n) {
  while (--n != 0) {
    fiberYield();
  }
  return LF_NOERROR;
}

/*
 * //Done
 */

ctr_object *ctr_fiber_spawn(ctr_object *myself, ctr_argument *argumentList);
ctr_object *ctr_fiber_yield(ctr_object *myself, ctr_argument *argumentList);
ctr_object *ctr_fiber_join_all(ctr_object *myself, ctr_argument *argumentList);
ctr_object *ctr_fiber_tostring(ctr_object *myself, ctr_argument *argumentList);
ctr_object *ctr_fiber_yielded(ctr_object *myself, ctr_argument *argumentList);
void ctr_fiber_begin_init(void);

extern ctr_object *CtrStdFiber; // Namespace for all fibers, conatining all the
                         // relevant functions
/**@I_OBJ_DEF Fiber*/

/**
 * Fiber new: [Block]
 *
 * Creates a new Fiber object that does (Block) upon control being yielded to it
 *
 * Fiber is a co-processing method in citron.
 * Think software level threads
 */
ctr_object *ctr_fiber_create(ctr_object *myself, ctr_argument *argumentList) {
  ctr_object *ctrl = ctr_internal_create_object(CTR_OBJECT_TYPE_OTEX);
  ctr_set_link_all(ctrl, CtrStdFiber);
  return ctrl;
}

ctr_object *ctr_fiber_spawn(ctr_object *myself, ctr_argument *argumentList) {
  ctr_object *fiberObj = ctr_fiber_create(myself, NULL);
  ctr_object *blk = argumentList->object;
  if (blk == NULL || blk->info.type != CTR_OBJECT_TYPE_OTBLOCK) {
    CtrStdFlow =
        ctr_build_string_from_cstring("Fiber's spawning requires a block.");
    return myself;
  }
  int fiber = spawnFiber(argumentList->object);
  ctr_internal_object_add_property(
      fiberObj, ctr_build_string_from_cstring("fiberId"),
      ctr_build_number_from_float(numFibers), CTR_CATEGORY_PRIVATE_PROPERTY);
  return fiberObj;
}

/**
 * [Fiber] yield
 *
 * yield control from the current object
 */
/**
 * [Fiber] yield: [Object]
 *
 * yield control and pass an object along
 */
ctr_object *ctr_fiber_yield(ctr_object *myself, ctr_argument *argumentList) {
  // int fiber = ctr_internal_object_find_property(myself,
  // ctr_build_string_from_cstring("fiberId"),
  // CTR_CATEGORY_PRIVATE_PROPERTY)->value.nvalue;
  // FIBER_YIELDED = argumentList->object;      // record the output, or set to
  // NULL
  ctr_object *ys = ctr_build_string_from_cstring("yielded");
  ctr_internal_object_set_property(
      myself, ys, argumentList->object ? argumentList->object : CtrStdNil,
      CTR_CATEGORY_PRIVATE_PROPERTY);
  fiberYield();
  return ctr_internal_object_find_property(myself, ys,
                                           CTR_CATEGORY_PRIVATE_PROPERTY);
}

/**
 * [Fiber] waitForAll
 *
 * Wait until all fibers return
 */
ctr_object *ctr_fiber_join_all(ctr_object *myself, ctr_argument *argumentList) {
  return ctr_build_bool(waitForAllFibers());
}

/**
 * [Fiber] waitFor:
 *
 * Wait until fiber returns
 */
ctr_object *ctr_fiber_join(ctr_object *myself, ctr_argument *argumentList) {
  waitForFiber((int)ctr_internal_object_find_property(
                   myself, ctr_build_string_from_cstring("fiberId"),
                   CTR_CATEGORY_PRIVATE_PROPERTY)
                   ->value.nvalue);
  ctr_object *y = ctr_internal_object_find_property(
      myself, ctr_build_string_from_cstring("yielded"),
      CTR_CATEGORY_PRIVATE_PROPERTY);
  return y ? y : CtrStdNil;
}

ctr_object *ctr_fiber_join_times(ctr_object *myself,
                                 ctr_argument *argumentList) {
  return ctr_build_bool(yieldNTimes(
      (int)ctr_internal_cast2number(argumentList->object)->value.nvalue));
}

ctr_object *ctr_fiber_tostring(ctr_object *myself, ctr_argument *argumentList) {
  return ctr_build_string_from_cstring("[Fiber]");
}

/**
 * [Fiber] yielded
 *
 * get the object that the last fiber yielded, or Nil if no object was yielded
 */
ctr_object *ctr_fiber_yielded(ctr_object *myself, ctr_argument *argumentList) {
  ctr_object *y = ctr_internal_object_find_property(
      myself, ctr_build_string_from_cstring("yielded"),
      CTR_CATEGORY_PRIVATE_PROPERTY);
  return y ? y : CtrStdNil;
}

/**
 * [Fiber] unpack: [String:Ref], [Object:ctx]
 * Assigns the fiber instance to the reference
 * (Always prefer using algebraic deconstruction assignments: look at section
 * 'Assignment')
 */
ctr_object *ctr_fiber_assign(ctr_object *myself, ctr_argument *argumentList) {
  return ctr_object_assign(myself, argumentList);
}

void ctr_fiber_begin_init() {
  initFibers();

  CtrStdFiber = ctr_internal_create_object(CTR_OBJECT_TYPE_OTEX);
  ctr_set_link_all(CtrStdFiber, CtrStdObject);
  ctr_internal_create_func(CtrStdFiber,
                           ctr_build_string_from_cstring("toString"),
                           &ctr_fiber_tostring);
  ctr_internal_create_func(CtrStdFiber, ctr_build_string_from_cstring("new:"),
                           &ctr_fiber_spawn);
  ctr_internal_create_func(CtrStdFiber, ctr_build_string_from_cstring("yield:"),
                           &ctr_fiber_yield); // with value
  ctr_internal_create_func(CtrStdFiber, ctr_build_string_from_cstring("yield"),
                           &ctr_fiber_yield); // without value
  ctr_internal_create_func(CtrStdFiber,
                           ctr_build_string_from_cstring("yielded"),
                           &ctr_fiber_yielded);
  ctr_internal_create_func(CtrStdFiber,
                           ctr_build_string_from_cstring("yieldTimes:"),
                           &ctr_fiber_join_times);
  ctr_internal_create_func(CtrStdFiber,
                           ctr_build_string_from_cstring("waitForAll"),
                           &ctr_fiber_join_all);
  ctr_internal_create_func(CtrStdFiber, ctr_build_string_from_cstring("wait"),
                           &ctr_fiber_join);
  ctr_internal_create_func(
      CtrStdFiber, ctr_build_string_from_cstring("unpack:"), &ctr_fiber_assign);
  ctr_internal_object_add_property(
      CtrStdFiber, ctr_build_string_from_cstring("fiberId"),
      ctr_build_number_from_float(numFibers), CTR_CATEGORY_PRIVATE_PROPERTY);
  CtrStdFiber->info.sticky = 1;

  ctr_internal_object_add_property(
      CtrStdWorld, ctr_build_string_from_cstring("Fiber"), CtrStdFiber, 0);
}
