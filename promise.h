#ifndef CTR_PROMISE_H
#define CTR_PROMISE_H
#include "citron.h"
#include <gc/gc.h>

#define CTR_PROMISE_CALL 0
#define CTR_PROMISE_MESSAGE 1

struct ctr_promise_entry {
    union {
        ctr_object* respond;
        ctr_string message;
    } target;
    ctr_object* receiver;
    ctr_argument* arguments;
    struct ctr_promise_entry* next;
    pthread_mutex_t* lock;
    pthread_mutex_t* return_lock;
    pthread_t thread;
    int next_type;
    struct ctr_context_t context;
};

ctr_object* ctr_promise_make(ctr_object*, ctr_object*, ctr_argument*, ctr_object*);
ctr_object* ctr_promise_poll(ctr_object*,ctr_argument*);
ctr_object* ctr_promise_wait(ctr_object*,ctr_argument*);
ctr_object* ctr_promise_then(ctr_object*, ctr_object *);
ctr_object* ctr_promise_then_(ctr_object*,ctr_argument*);
ctr_object* ctr_promise_pass_message(ctr_object* myself, ctr_argument* argumentList);
ctr_object* CtrStdPromise;
void promise_begin();

#endif /* end of include guard: CTR_PROMISE_H */
