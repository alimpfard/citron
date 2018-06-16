#include "promise.h"
#include <pthread.h>

static ctr_object* ctr_promise_native_respond = NULL;

void *ctr_promise_thread_sf(void* data) {
  struct ctr_promise_entry *entry = data, *c_ent=entry;
  ctr_object* result = CtrStdNil, *target, *receiver;
  ctr_argument* args = ctr_heap_allocate(sizeof(*args));
  struct ctr_context_t octx;
again:
  pthread_mutex_lock(entry->lock);
  ctr_dump_context(&octx);
  ctr_load_context(c_ent->context);
  switch(c_ent->next_type) {
    case CTR_PROMISE_CALL:
      target = c_ent->target.respond;
      receiver = c_ent->receiver;
      receiver = receiver?:result;
      result = ctr_block_run(target, c_ent->arguments, receiver);
      break;
    case CTR_PROMISE_MESSAGE:
      result = ctr_send_message_blocking(result, c_ent->target.message.value, c_ent->target.message.vlen, c_ent->arguments);
      break;
  }
  ctr_load_context(octx);
  // printf("%p -> %s\n", result, ctr_heap_allocate_cstring(ctr_internal_cast2string(result)));
  pthread_mutex_unlock(entry->lock);
  while(pthread_mutex_trylock(entry->return_lock)) {
    if(c_ent->next) {
      c_ent = c_ent->next;
      goto again;
    }
  }
  pthread_mutex_unlock(entry->return_lock);
  if(c_ent->next) {
    c_ent = c_ent->next;
    goto again;
  }
  return result;
}

ctr_object* ctr_promise_make(ctr_object* myself, ctr_object* target, ctr_argument* argumentList, ctr_object* receiver) {
  ctr_argument* args = ctr_heap_allocate(sizeof(*args)), *cargs=args;
  while(argumentList) {
    args->object = argumentList->object;
    if (argumentList->next) {
      args->next = ctr_heap_allocate(sizeof(*args));
      args = args->next;
    }
    argumentList = argumentList->next;
  }
  argumentList = cargs;
  ctr_object* instance = ctr_internal_create_standalone_object(CTR_OBJECT_TYPE_OTMISC);
  ctr_set_link_all(instance, myself);
  ctr_resource* res = ctr_heap_allocate(sizeof(*res));
  static struct ctr_context_t context;
  ctr_dump_context(&context);
  static struct ctr_promise_entry sentry = {
    .next = NULL,
    .next_type = CTR_PROMISE_CALL,
  };
  sentry.context = context;
  sentry.target.respond = target;
  sentry.lock = ctr_heap_allocate(sizeof(*sentry.lock));
  sentry.return_lock = ctr_heap_allocate(sizeof(*sentry.return_lock));
  sentry.arguments = argumentList;
  sentry.receiver = receiver;
  int err = pthread_mutex_init(sentry.lock, NULL);
  if (err) {
    CtrStdFlow = ctr_format_str("ECreation of mutex failed for promise with error: %d", err);
    return CtrStdNil;
  }
  err = pthread_mutex_init(sentry.return_lock, NULL);
  if (err) {
    CtrStdFlow = ctr_format_str("ECreation of return mutex failed for promise with error: %d", err);
    return CtrStdNil;
  }
  pthread_mutex_lock(sentry.return_lock); //should _not_ fail
  struct ctr_promise_entry* entry = ctr_heap_allocate(sizeof(*entry));
  *entry = sentry;
  res->ptr = entry;
  res->type = 0;
  instance->value.rvalue = res;
  if((err = pthread_create(&entry->thread, NULL, &ctr_promise_thread_sf, entry))) {
    CtrStdFlow = ctr_format_str("ECreation of thread failed for promise with error: %d", err);
    return CtrStdNil;
  }
  return instance;
}

__attribute__((always_inline))
ctr_object* ctr_promise_then(ctr_object* myself, ctr_object* target) {
  ctr_resource* res = ctr_heap_allocate(sizeof(*res));
  static struct ctr_promise_entry sentry = {
    .next = NULL,
    .arguments = NULL,
    .next_type = CTR_PROMISE_CALL,
    .receiver = NULL,
  };
  struct ctr_promise_entry* before = myself->value.rvalue->ptr;
  while(before->next)
    before = before->next;
  sentry.context = before->context;
  sentry.target.respond = target;
  sentry.lock = before->lock;
  sentry.return_lock = before->return_lock;
  struct ctr_promise_entry* entry = ctr_heap_allocate(sizeof(*entry));
  *entry = sentry;
  before->next = entry;
  return myself;
}

ctr_object* ctr_promise_then_(ctr_object* myself, ctr_argument* argumentList) {
  return ctr_promise_then(myself, argumentList->object);
}

ctr_object* ctr_promise_pass_message(ctr_object* myself, ctr_argument* argumentList) {
  ctr_argument* args = ctr_heap_allocate(sizeof(*args)), *cargs=args;
  while(argumentList) {
    args->object = argumentList->object;
    if (argumentList->next) {
      args->next = ctr_heap_allocate(sizeof(*args));
      args = args->next;
    }
    argumentList = argumentList->next;
  }
  argumentList = cargs;
  ctr_resource* res = ctr_heap_allocate(sizeof(*res));
  static struct ctr_promise_entry sentry = {
    .next = NULL,
    .arguments = NULL,
    .next_type = CTR_PROMISE_MESSAGE,
  };
  struct ctr_promise_entry* before = myself->value.rvalue->ptr;
  while(before->next)
    before = before->next;
  sentry.context = before->context;
  sentry.target.message = *argumentList->object->value.svalue;
  sentry.lock = before->lock;
  sentry.return_lock = before->return_lock;
  sentry.arguments = argumentList->next;
  struct ctr_promise_entry* entry = ctr_heap_allocate(sizeof(*entry));
  *entry = sentry;
  before->next = entry;
  return myself;
}

ctr_object* ctr_promise_dummy(ctr_object* myself, ctr_argument* argumentList) {
  ctr_argument* args = ctr_heap_allocate(sizeof(*args));
  args->object = ctr_build_number_from_float(2);
  return ctr_send_message_async(ctr_build_string_from_cstring("Dummy"), "skip:", 5, args);
}

ctr_object* ctr_promise_wait(ctr_object* myself,ctr_argument* argumentList) {
  struct ctr_promise_entry* entry = myself->value.rvalue->ptr;
  pthread_mutex_unlock(entry->return_lock);
  void* retval;
  pthread_join(entry->thread, &retval);
  return retval;
}

ctr_object* ctr_promise_make_async(ctr_object* myself, ctr_argument* argumentList) {
  myself->info.asyncMode = 1;
  return myself;
}

void promise_begin() {
  ctr_promise_native_respond = ctr_internal_create_object(CTR_OBJECT_TYPE_OTNATFUNC);
  ctr_promise_native_respond->value.fvalue = &ctr_promise_pass_message;
  CtrStdPromise = ctr_internal_create_object(CTR_OBJECT_TYPE_OTMISC);
  ctr_internal_create_func(CtrStdPromise, ctr_build_string_from_cstring("then:"), ctr_promise_then_);
  ctr_internal_create_func(CtrStdPromise, ctr_build_string_from_cstring("await"), ctr_promise_wait);
  ctr_internal_create_func(CtrStdPromise, ctr_build_string_from_cstring("toString"), ctr_object_to_string);
  ctr_internal_create_func(CtrStdPromise, ctr_build_string_from_cstring("respondTo:"), ctr_promise_pass_message);
  ctr_internal_create_func(CtrStdPromise, ctr_build_string_from_cstring("respondTo:and:"), ctr_promise_pass_message);
  ctr_internal_create_func(CtrStdPromise, ctr_build_string_from_cstring("respondTo:and:and:"), ctr_promise_pass_message);
  ctr_internal_create_func(CtrStdPromise, ctr_build_string_from_cstring("respondTo:and:and:and:"), ctr_promise_pass_message);
  ctr_internal_create_func(CtrStdPromise, ctr_build_string_from_cstring("respondTo:and:and:and:and:"), ctr_promise_pass_message);
  ctr_internal_create_func(CtrStdObject, ctr_build_string_from_cstring("async"), ctr_promise_make_async);
}
