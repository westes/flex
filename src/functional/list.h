#ifndef LIST_H
#define LIST_H

#include <stddef.h>
#include <stdbool.h>

typedef struct list_t list_t;

struct list_t {
	void* head;
	void* tail;
};

list_t* cons(void* head, void* tail);
void* head(list_t* list);
void* tail(list_t* list);
bool is_empty(list_t* list);
list_t* concat(list_t* left, list_t* right);
void* nth(size_t index, list_t* list);
size_t length(list_t* list);
list_t* list_of_impl(size_t element_size, void* first, void* last);
list_t* list_from(void* elements[], size_t size);

#define list_of(type, first, last)							\
	list_of_impl(sizeof(type), first, last)

#define list(...) list_from(								\
	(void* []) { __VA_ARGS__ },								\
	sizeof (void* []) { __VA_ARGS__ } / sizeof (void*) )

#endif
