
/*
Author: Kevin Tyrrell
Date: 8/18/2016
Version: 2.0
*/

#pragma once

#include <stdlib.h>
#include <math.h>
#include <stdbool.h>
#include <string.h>

#include "Tools.h"

/* HashMap structure. */
struct HashMap
{
	struct hm_Node **table;
	/* HashMap accepts a NULL key. */
	struct hm_KeySet *nullSet;
	size_t capacity, size;

	/* Function pointers.
	These are given to the HashMap to allow it to handle unknown data types. */
	bool(*equals)(const void* const, const void* const);
	char*(*toString)(const void* const);
	unsigned int(*hash)(const void* const);
};

/* KeySet for Key/Value entries. */
struct hm_KeySet
{
	void *key, *value;
};

/* Node structure. */
struct hm_Node
{
	struct hm_Node *next;
	struct hm_KeySet *set;
};

/* ~~~~~ Constructors ~~~~~ */

/*
Must pass in three function pointers.
The first function is the hash function for your key.
The second compares the two KEYS and returns 0 if they are equal.
The third function must return the String representation of the VALUE.
*/
struct HashMap* HashMap_new(const size_t capacity, unsigned int(*hash)(const void* const),
	bool(*equals)(const void* const, const void* const), char*(*toString)(const void* const));

/* ~~~~~ Accessors ~~~~~ */

/* Gets a value from the HashMap when provided with a key. */
void* hm_get(const struct HashMap* const map, const void* const key);
/* Returns true if the HashMap contains the specified key. */
bool hm_contains(const struct HashMap* const map, const void* const key);
/* Prints out the inner table of the HashMap. */
void hm_print(const struct HashMap* const map);
/* Return an array with all Keys/Values currently in the HashMap. */
struct hm_KeySet** hm_KeySets(const struct HashMap *map);
/* Returns a shallow copy of the HashMap. */
struct HashMap* hm_clone(const struct HashMap* const map);

/* ~~~~~ Mutators ~~~~~ */

/* Place a Key/Value pair into the HashMap. */
void hm_put(struct HashMap* const map, const void* const key, const void* const value);
/* Removes a Key/Value pair from the HashMap when provided with a key. */
void* hm_remove(struct HashMap* const map, const void* const key);
/* Clear the HashMap of all Key/Value pairs. */
void hm_clear(struct HashMap* const map);

/* ~~~~~ De-constructors ~~~~~ */

void hm_destroy(const struct HashMap* const map);
void hm_KeySet_destroy(const struct hm_KeySet* const set);






