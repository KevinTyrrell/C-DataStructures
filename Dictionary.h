#pragma once

#include "Vector.h"

/*
 * TreeMap data structure.
 * Map manages key/value pairs called "KeySets".
 * The map is always sorted based on the keys.
 * Each value can only be accessed/mutated with its key.
 * Constructor: Dictionary_new, De-constructor: dict_destroy.
 */
typedef struct Dictionary Dictionary;
struct KeySet
{
	void *key, *value;
};

/* ~~~~~ Constructors ~~~~~ */

Dictionary* Dictionary_new(int(*compare)(const void*, const void*),
	char*(*toString)(const struct KeySet*));


/* ~~~~~ Accessors ~~~~~ */

/* ~~~~~ Mutators ~~~~~ */

void dict_put(const Dictionary* const dict, const void* const key, const void* value);

/* ~~~~~ De-constructors ~~~~~ */

/* ~~~~~ Iterator ~~~~~ */
