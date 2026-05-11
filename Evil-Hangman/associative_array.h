#ifndef ASSOCIATIVE_ARRAY_H
#define ASSOCIATIVE_ARRAY_H

#include "status.h"
#include "my_string.h"
#include "generic_vector.h"

typedef void* ASSOCIATIVE_ARRAY;

ASSOCIATIVE_ARRAY associative_array_init_default(void);

Status associative_array_insert(ASSOCIATIVE_ARRAY hArray, MY_STRING key, MY_STRING word);

GENERIC_VECTOR associative_array_find(ASSOCIATIVE_ARRAY hArray, MY_STRING key);

GENERIC_VECTOR associative_array_get_largest_family(ASSOCIATIVE_ARRAY hArray, MY_STRING winning_key);

void associative_array_destroy(ASSOCIATIVE_ARRAY* phArray);

#endif
