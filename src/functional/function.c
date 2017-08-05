#include "function.h"
#include "gc.h"

function_t* bind_args_impl(function_t* fn, list_t* arguments) {
	function_t* closure = gc_malloc(sizeof *closure);

	return closure
		? (*closure = (function_t) { fn->thunk, arguments }, closure)
		: NULL;
}

void* apply(function_t* fn, list_t* arguments) {
	return fn->thunk(concat(fn->arguments, arguments));
}
