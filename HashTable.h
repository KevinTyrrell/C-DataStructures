
/*
* Author: Kevin Tyrrell
* Date: 8/18/2016
* Version: 3.0
*/

#pragma once

#include "Tools.h"

/*
 * HashTable data structure.
 * HashTable uses the passed in Hash function to manage elements.
 * Each key of the key value pair are hashed and placed into the table.
 * The table grows as it needs space for more elements.
 * Constructor: Hashtable_new, De-constructor: table_destroy.
 */
typedef struct HashTable HashTable;

/* Entry structure for Key/Value pairs. */
struct table_Entry
{
	const void *key, *value;
};

/* ~~~~~ Constructors ~~~~~ */

/*
 * Constructs a new HashTable.
 * Hash - Converts your key into a unique unsigned integer.
 * Equals - Returns true if one key is the same as another.
 * toString - Returns the String representation of a Key/Value pair.
 */
HashTable* HashTable_new(
	unsigned int(*hash)(const void*),
	bool(*equals)(const void*, const void*),
	char*(*toString)(const struct table_Entry*));

/* ~~~~~ Accessors ~~~~~ */

/* Returns the value of an entry that corresponds to the provided key. */
void* table_get(const HashTable* const table, const void* const key);
/* Returns true if an entry with the provided key exists in the table. */
bool table_contains(const HashTable* const table, const void* const key);
/* Returns true if the table is empty. */
bool table_empty(const HashTable* const table);
/* Returns the amount of entries inside the table. */
size_t table_size(const HashTable* const table);
/* Returns the capacity of the table. */
size_t table_capacity(const HashTable* const table);
/* Returns an array of all entries inside the table. */
struct table_Entry** table_entries(const HashTable* const table);
/* Returns a shallow copy of the table. */
HashTable* table_clone(const HashTable* const table);
/* Prints out all entries inside the table to the console window. */
void table_print(const HashTable* const table);
/* Returns the current collision rate in the table. */
float table_collision_rate(const HashTable* const table);

/* ~~~~~ Mutators ~~~~~ */

/* Places a Key/Value entry into the table. */
void table_put(HashTable* const table, const void* const key, const void* const value);
/* Removes an entry from the table. */
bool table_remove(HashTable* const table, const void* const key);
/* Removes all entries from the table. */
void table_clear(HashTable* const table);
/* Grows the underlying array of the table to a specified capacity. */
void table_grow(HashTable* const table, const size_t capacity);
/* Shrinks the underlying array of the table to conserve memory. */
void table_shrink(HashTable* const table);

/* ~~~~~ De-constructors ~~~~~ */

void table_destroy(HashTable* const table);
