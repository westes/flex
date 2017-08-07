#include "boxes.h"
#include "gc.h"

int* int_number(int value) {
	int* number = gc_malloc(sizeof *number);

	return number ? (*number = value, number) : NULL;
}

unsigned char* unsigned_symbol(unsigned char value) {
	unsigned char* symbol = gc_malloc(sizeof *symbol);

	return symbol ? (*symbol = value, symbol) : NULL;
}
