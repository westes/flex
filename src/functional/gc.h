#ifndef GC_H
#define GC_H

#include <stddef.h>

typedef void (*finalizer_t)(void*);

void* gc_default_malloc(size_t size);
void* gc_finalizable_malloc(size_t size, finalizer_t finalize);
void gc_collect();

#define SELECT_GC_MALLOC(ARG1, ARG2, IMPL, ...) IMPL
#define gc_malloc(...) SELECT_GC_MALLOC(__VA_ARGS__,		\
	gc_finalizable_malloc, gc_default_malloc)(__VA_ARGS__)

#endif
