
/*
* Author: Kevin Tyrrell
* Date: 8/18/2016
* Version: 2.1
*/

#pragma once

#include <stdlib.h>
#include <stdbool.h>

#include "Tools.h"

/* HashMap structure. */
struct HashMap
{
	struct map_Node **table;
	/* HashMap accepts a NULL key. */
	struct map_Entry *nullSet;
	size_t capacity, size;

	/* Function pointers.
	These are given to the HashMap to allow it to handle unknown data types. */
	bool(*equals)(const void* const, const void* const);
	unsigned int(*hash)(const void* const);
	char*(*toString)(const void* const, const void* const);
};

/* Entry structure for Key/Value pairs. */
struct map_Entry
{
	void *key, *value;
};

/* Node structure. */
struct map_Node
{
	struct map_Node *next;
	struct map_Entry *set;
};

/* ~~~~~ Constructors ~~~~~ */

/*
* Hash function. Must return a value larger than the capacity of the Map.
* Ideally, hash should convert unique keys into unique integers.
*
* Equals function. It's possible for two unique keys to hash to the same value.
* Due to this, an equals function is needed to distinguish different elements in the map.
*
* toString function. Converts a Key/Value entry into a string for the print function.
* The first parameter for the function is the key and the second is the value.
*/
struct HashMap* HashMap_new(unsigned int(*hash)(const void* const),
	bool(*equals)(const void* const, const void* const),
	char*(*toString)(const void* const, const void* const));

/* ~~~~~ Accessors ~~~~~ */

/* Returns the value corresponding to the provided key in the map. NULL if no such key exists. */
void* map_get(const struct HashMap* const map, const void* const key);
/* Returns true if a provided key exists within the Map. */
bool map_contains(const struct HashMap* const map, const void* const key);
/* Returns true if the map is empty. False if otherwise. */
bool map_empty(const struct HashMap* const map);
/* Returns the amount of elements inside the Map. */
size_t map_size(const struct HashMap* const map);
/* Prints all elements inside the Map to the console window. */
void map_print(const struct HashMap* const map);
/* Returns an array of all Key/Value entries inside the Map. */
struct map_Entry** map_keyset(const struct HashMap *map);
/* Returns a shallow copy of the Map. */
struct HashMap* map_clone(const struct HashMap* const map);

/* ~~~~~ Mutators ~~~~~ */

/* Associates a provided key with a provided value into the Map. */
void map_put(struct HashMap* const map, const void* const key, const void* const value);
/* Removes a Key/Value pair from the HashMap when provided with a key. */
void* map_remove(struct HashMap* const map, const void* const key);
/* Clear the HashMap of all Key/Value pairs. */
void map_clear(struct HashMap* const map);

/* ~~~~~ De-constructors ~~~~~ */

void map_destroy(const struct HashMap* const map);
void map_entry_destroy(const struct map_Entry* const set);
