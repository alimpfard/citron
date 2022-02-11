#ifndef CTR_GC_H
#define CTR_GC_H

#ifdef withBoehmGC
#    define GC_THREAD
#    include <gc/gc.h>
#else
// FIXME: These need to be implemented!
#    define GC_collect_a_little()
#    define GC_disable()
#    define GC_dont_gc()
#    define GC_dump()
#    define GC_enable()
#    define GC_enable_incremental()
#    define GC_gcollect()
#    define GC_get_free_bytes() 0
#    define GC_get_heap_size() 0
#    define GC_get_unmapped_bytes() 0
#    define GC_pthread_create pthread_create
#    define GC_register_displacement(...)
#    define GC_set_max_heap_size(...)
#endif

#endif // CTR_GC_H
