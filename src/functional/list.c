#include "list.h"
#include "gc.h"

list_t* cons(void* head, void* tail) {
	list_t* list = gc_malloc(sizeof *list);

	return list ? (*list = (list_t) { head, tail }, list) : NULL;
}

void* head(list_t* list) {
	return list->head; 
}

void* tail(list_t* list) {
	return list->tail; 
}

bool is_empty(list_t* list) {
	return list == NULL;
}

list_t* concat(list_t* left, list_t* right) {
	return is_empty(left)
		? right
		: cons(head(left), concat(tail(left), right));
}

void* nth(size_t index, list_t* list) {
	return index == 0 ? head(list) : nth(index - 1, tail(list));
}

size_t length(list_t* list) {
	return is_empty(list) ? 0 : 1 + length(tail(list));
}

list_t* list_of_impl(size_t element_size, void* first, void* last) {
	list_t* head = NULL;
	char* start = (char*) first - element_size;
	char* stop = (char*) last - element_size;
	char* cursor = stop;

	for (; cursor != start; cursor -= element_size) {
		head = cons(cursor, head);
	}

	return head;
}

list_t* list_from(void* elements[], size_t size) {
	return size == 0
		? NULL
		: cons(elements[0], list_from(&elements[1], size - 1));
}
