#include "character-class.h"
#include "boxes.h"
#include "../flexdef.h"
#include "operations.h"

character_class_t  character_class_at_index_impl(size_t index) {
	list_t* symbols = list_of(
		unsigned char,
		&ccltbl[cclmap[index]],
		&ccltbl[cclmap[index] + ccllen[index]]
	);
	
	return (character_class_t) { symbols, &ccllen[index] };
}
DEFINE_PUBLIC_FUNCTION(
	character_class_at_index,
	RETURN_VALUE(
		character_class_at_index_impl,
		character_class_t,
		BIND_VALUE_ARG(size_t, 0)
	)
)


static list_t* character_into_equivalence_symbols_impl(
	list_t* accum,
	unsigned char* character_class_symbol
) {
	int symbol = ecgroup[*character_class_symbol];

	return symbol  > 0
		? cons(unsigned_symbol(symbol), accum)
		: accum;
}
DEFINE_PRIVATE_FUNCTION(
	character_into_equivalence_symbols,
	RETURN_POINTER(
		character_into_equivalence_symbols_impl,
		list_t,
		BIND_POINTER_ARG(list_t, 0),
		BIND_POINTER_ARG(unsigned char, 1)
	)
)


character_class_t to_equivalence_class_impl(character_class_t klass) {
	list_t* symbols = foldr(
		character_into_equivalence_symbols,
		list(),
		klass.symbols
	);

	return (character_class_t) { symbols, int_number(length(symbols)) };
}
DEFINE_PUBLIC_FUNCTION(
	to_equivalence_class,
	RETURN_VALUE(
		to_equivalence_class_impl,
		character_class_t,
		BIND_VALUE_ARG(character_class_t, 0)
	)
)


static void deep_copy_symbols(list_t* destination, list_t* source) {
    if (is_empty(source)) {
        return;
    }

    unsigned char* from = head(source);
    unsigned char* to = head(destination);

    *to = *from;
    deep_copy_symbols(tail(destination), tail(source));
}

void deep_copy_character_class(
	character_class_t* destination,
	character_class_t* source
) {
	deep_copy_symbols(destination->symbols, source->symbols);
	*destination->length = *source->length;
}
