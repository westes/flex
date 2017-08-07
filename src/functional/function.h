#ifndef FUNCTION_H
#define FUNCTION_H

#include "gc.h"
#include "list.h"

typedef void* (*thunk_t)(list_t*);

typedef struct function_t function_t;

struct function_t {
	thunk_t thunk;
	list_t* arguments;
};

function_t* bind_args_impl(function_t* fn, list_t* arguments);
#define bind_args(fn, ...) bind_args_impl(fn, list(__VA_ARGS__))

#define CONCAT(a, b) a##b
#define DEFINE_PUBLIC_FUNCTION(name, body)						\
	static void* CONCAT(name, _thunk)(list_t* arguments) {		\
		body													\
	}															\
	function_t* name = &(function_t) {							\
		CONCAT(name, _thunk),									\
		NULL													\
	};
#define DEFINE_PRIVATE_FUNCTION(name, body)						\
	static void* CONCAT(name, _thunk)(list_t* arguments) {		\
		body													\
	}															\
	static function_t* name = &(function_t) {					\
		CONCAT(name, _thunk),									\
		NULL													\
	};

#define BIND_VALUE_ARG(type, index) *(type*) nth(index, arguments)
#define BIND_POINTER_ARG(type, index) (type*) nth(index, arguments)

#define RETURN_NOTHING(fn, ...)							\
	return fn(__VA_ARGS__), NULL;
#define RETURN_POINTER(fn, type, ...)					\
	return fn(__VA_ARGS__);
#define RETURN_VALUE(fn, type, ...)						\
	type value = fn(__VA_ARGS__);						\
	type* result = gc_malloc(sizeof *result);			\
	return result ? (*result = value, result) : NULL;

void* apply(function_t* fn, list_t* arguments);
#define call(fn, ...) apply(fn, list(__VA_ARGS__))

#endif
