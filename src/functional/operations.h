#ifndef FUNCTIONAL_H
#define FUNCTIONAL_H

#include "list.h"
#include "function.h"

list_t* map(function_t* fn, list_t* list);
extern function_t* map_fn;

list_t* filter(function_t* predicate, list_t* list);
extern function_t* filter_fn;

void* fold(function_t* reducer, void* seed, list_t* list);
extern function_t* fold_fn;

void* foldr(function_t* reducer, void* seed, list_t* list);
extern function_t* foldr_fn;

list_t* generic_range(int begin, int end, int step);
list_t* canonical_range(int begin, int end);

#define SELECT_RANGE(ARG1, ARG2, ARG3, IMPL, ...) IMPL
#define range(...) SELECT_RANGE(__VA_ARGS__, generic_range, canonical_range)(__VA_ARGS__)

function_t* composition_of(list_t* functions);
#define compose(...) composition_of(list(__VA_ARGS__))

#endif
