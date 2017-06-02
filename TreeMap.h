
/*
 * Author: Kevin Tyrrell
 * Date: 9/01/2016
 * Version: 3.0
 */

#pragma once

#include "Tools.h"
#include "LinkedList.h"

 /*
 * TreeMap data structure.
 * TreeMap uses the passed in Compare function to manage elements.
 * Each key of the key value pair are compared with existing elements.
 * The tree balances itself when it becomes imbalanced.
 * Constructor: TreeMap_new, De-constructor: map_destroy.
 */
typedef struct TreeMap TreeMap;
typedef struct map_Iterator map_Iterator;

/* Entry structure for Key/Value pairs. */
struct map_Entry
{
	const void *key, *value;
};

/* ~~~~~ Constructors ~~~~~ */

TreeMap* TreeMap_new(int(*compare)(const void*, const void*), char*(*toString)(const struct map_Entry*));

/* ~~~~~ Accessors ~~~~~ */

/* Returns the amount of entries inside the table. */
size_t map_size(const TreeMap* const map);

/* ~~~~~ Mutators ~~~~~ */

/* Places a Key/Value entry into the map. */
void map_put(TreeMap* const map, void* const key, void* const value);

/* ~~~~~ De-constructors ~~~~~ */



/* ~~~~~ Iterator ~~~~~ */

/* Constructor function. */
map_Iterator* map_iter(const TreeMap* const map, const bool front);
/* Returns the iterator's current element and advances it forward. */
struct map_Entry* map_iter_next(map_Iterator* const iter);
/* Returns true if the iterator has a next element. */
bool map_iter_has_next(const map_Iterator* const iter);
/* Returns true if the iterator has a previous element. */
bool map_iter_has_prev(const map_Iterator* const iter);
/* De-constructor function. */
void map_iter_destroy(map_Iterator* const iter);