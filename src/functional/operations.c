#include "operations.h"
#include "boxes.h"

list_t* map(function_t* fn, list_t* list) {
	return is_empty(list)
		? list()
		: cons(call(fn, head(list)), map(fn, tail(list)));
}

DEFINE_PUBLIC_FUNCTION(
	map_fn,
	RETURN_POINTER(
		map,
		list_t,
		BIND_POINTER_ARG(function_t, 0),
		BIND_POINTER_ARG(list_t, 1)
	)
)

list_t* filter(function_t* predicate, list_t* list) {
	return is_empty(list)
		? list()
		: concat(
			*(bool*) call(predicate, head(list)) ? list(head(list)) : list(),
			filter(predicate, tail(list))
		);
}

DEFINE_PUBLIC_FUNCTION(
	filter_fn,
	RETURN_POINTER(
		filter,
		list_t,
		BIND_POINTER_ARG(function_t, 0),
		BIND_POINTER_ARG(list_t, 1)
	)
)

void* fold(function_t* reducer, void* seed, list_t* list) {
	return is_empty(list)
		? seed
		: fold(reducer, call(reducer, seed, head(list)), tail(list));
}

DEFINE_PUBLIC_FUNCTION(
	fold_fn,
	RETURN_POINTER(
		fold,
		void,
		BIND_POINTER_ARG(function_t, 0),
		BIND_POINTER_ARG(void, 1),
		BIND_POINTER_ARG(list_t, 2)
	)
)

void* foldr(function_t* reducer, void* seed, list_t* list) {
	return is_empty(list)
		? seed
		: call(reducer, foldr(reducer, seed, tail(list)), head(list));
}

DEFINE_PUBLIC_FUNCTION(
	foldr_fn,
	RETURN_POINTER(
		foldr,
		void,
		BIND_POINTER_ARG(function_t, 0),
		BIND_POINTER_ARG(void, 1),
		BIND_POINTER_ARG(list_t, 2)
	)
)

list_t* generic_range(int begin, int end, int step) {
	return begin >= end
		? list()
		: cons(int_number(begin), generic_range(begin + step, end, step));
}

list_t* canonical_range(int begin, int end) {
	return generic_range(begin, end, 1);
}

static void* unary_call(void* value, function_t* fn) {
	return call(fn, value);
}

DEFINE_PRIVATE_FUNCTION(
	call_fn,
	RETURN_POINTER(
		unary_call,
		void,
		BIND_POINTER_ARG(function_t, 0),
		BIND_POINTER_ARG(void, 1)
	)
)

static void* compose_functions_impl(list_t* functions, void* value) {
	return foldr(call_fn, value, functions);
}

DEFINE_PRIVATE_FUNCTION(
	compose_functions,
	RETURN_POINTER(
		compose_functions_impl,
		void,
		BIND_POINTER_ARG(list_t, 0),
		BIND_POINTER_ARG(void, 1)
	)
)

function_t* composition_of(list_t* functions) {
	return bind_args(compose_functions, functions);
}
