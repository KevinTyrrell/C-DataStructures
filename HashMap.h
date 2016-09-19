
/*
Author: Kevin Tyrrell
Date: 8/18/2016
Version: 1.3
*/

#pragma once

#include <stdlib.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include "DataStructureTools.h"

/* HashMap structure. */
struct HashMap
{
	struct hm_Node **table;
	// HashMaps accept NULL keys.
	struct hm_Node *nullKeySet;
	size_t capacity, size;

	/* Function pointers.
	These are given to the HashMap to allow it to handle unknown data types. */
	bool(*equals)(void*, void*);
	char*(*toString)(void*);
	unsigned int(*hash)(void*);
};

/* Node structure. */
struct hm_Node
{
	void *key, *value;
	struct hm_Node *next;
};

/*
Constructor function.
Must pass in three function pointers.
The first function is the hash function for your key.
The second compares the two KEYS and returns 0 if they are equal.
The third function must return the String representation of the VALUE.
*/
struct HashMap* HashMap_new(size_t capacity, unsigned int(*hash)(void*), bool(*equals)(void*, void*), char*(*toString)(void*));

/* Place a Key/Value pair into the HashMap. */
void hm_put(struct HashMap *map, void *key, void *value);
/* Gets a value from the HashMap when provided with a key. */
void* hm_get(const struct HashMap *map, const void *key);
/* Removes a Key/Value pair from the HashMap when provided with a key. */
void* hm_remove(struct HashMap *map, void *key);
/* Check if a given element in the HashMap exists. */
bool hm_contains(struct HashMap *map, void *key);
/* Return an array with all Keys/Values currently in the HashMap. */
struct hm_Node** hm_KeySet(const struct HashMap *map);
/* Prints out the inner table of the HashMap. */
void hm_print(const struct HashMap *map);
/* Clear the HashMap of all Key/Value pairs. */
void hm_clear(struct HashMap *map);
/* Return a copy of the HashMap. */
struct HashMap* hm_clone(const struct HashMap *map);
/* De-constructor function. */
void hm_destroy(struct HashMap *map);