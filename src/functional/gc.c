#include "gc.h"

#include <stdlib.h>

typedef struct allocation_t allocation_t;

struct allocation_t {
	void* block;
	finalizer_t finalize;
	allocation_t* prev;
};

static allocation_t* allocation(
	void* block,
	finalizer_t finalize,
	allocation_t* prev)
{
	allocation_t* record = malloc(sizeof *record);

	return record
		? (*record  = (allocation_t) { block, finalize, prev }, record)
		: NULL;
}

static void gc_collect_from(allocation_t* allocation) {
	if (allocation == NULL) {
		return;
	}

	allocation->finalize(allocation->block);
	allocation->block = NULL;

	free(allocation);
	gc_collect_from(allocation->prev);
	allocation = NULL;
}

static allocation_t* allocations = NULL;

void* gc_finalizable_malloc(size_t size, finalizer_t finalize) {
	void* block = malloc(size);
	allocations = allocation(block, finalize, allocations);
	/* handle memory allocation fail */

	return block;
}

void* gc_default_malloc(size_t size) {
	return gc_malloc(size, free);
}

void gc_collect() {
	gc_collect_from(allocations);
}
