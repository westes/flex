#ifndef CHARACTER_CLASS_H
#define CHARACTER_CLASS_H

#include "list.h"
#include "function.h"

typedef struct character_class_t character_class_t;

struct character_class_t {
    list_t* symbols;

	/*
	 * `length` field is redundant as it's value could be deduced
	 * from `symbols` field.
	 *
	 * it is present here only for backward compatibility
	 * since it points to element of `ccllen` array.
	 */
    int* length;
};

character_class_t  character_class_at_index_impl(size_t index);
extern function_t* character_class_at_index;

character_class_t to_equivalence_class_impl(character_class_t klass);
extern function_t* to_equivalence_class;

void deep_copy_character_class(
	character_class_t* destination,
	character_class_t* source
);

#endif
