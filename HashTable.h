
/*
 * File: HashTable.h
 * Date: Aug 18, 2016
 * Name: Kevin Tyrrell
 * Version: 4.0.0
 */

/*
Copyright © 2017 Kevin Tyrrell

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#pragma once

#include "tools/Memory.h"
#include "tools/Synchronize.h"

/* Anonymous structures. */
typedef struct HashTable HashTable;
typedef struct table_Iterator table_Iterator;

/* ~~~~~ Constructors ~~~~~ */

/*
 * Constructs a new HashTable.
 * Hash - Returns a (preferably) unique and large integer value from a specified key.
 *        Data used to calculate Hash/Equals should not change while the key is in the Table.
 * Equals - Returns true if two keys are equivalent.
 *          Two different keys in the Table may share the same hash result but cannot be equal.
 * toString - Returns the String representation of a specified key/value pair.
 *
 * NOTE: The Hash and Equals functions MUST be defined.
 * NOTE: The Table must be de-constructed after its usable life-span.
 */
HashTable* HashTable_new(unsigned int(*hash)(const void*),
                         bool(*equals)(const void*, const void*),
                         char*(*toString)(const void*, const void*));

/* ~~~~~ Accessors ~~~~~ */

/* Returns the value of an entry that corresponds to the specified key. */
void* table_get(const HashTable* const table, const void* const key);
/* Returns the number of elements in the Table. */
size_t table_size(const HashTable* const table);
/* Returns true if the Table is empty. */
bool table_empty(const HashTable* const table);
/* Returns true if the Table contains the specified key. */
bool table_contains(const HashTable* const table, const void* const key);
/* Prints out the contents of the Table to the console window. */
void table_print(const HashTable* const table);
/* Returns a shallow copy of the Table. */
HashTable* table_clone(const HashTable* const table);

/* ~~~~~ Mutators ~~~~~ */

/* Inserts a key/value pair into the Table. */
void table_put(HashTable* const table, const void* const key, const void* const value);
/* Removes a key/value pair from the Table and returns true if the removal was successful. */
bool table_remove(HashTable* const table, const void* const key);
/* Grows the underlying array to be able to store at least `min_size` elements. */
void table_grow(HashTable *const table, const size_t min_size);
/* Removes all key/value pairs from the Table while preserving the capacity. */
void table_clear(HashTable* const table);

/* ~~~~~ De-constructors ~~~~~ */

void table_destroy(HashTable* const table);

/* ~~~~~ Iterator ~~~~~ */

/*
 * Constructs a new Iterator for the Table.
 *
 * NOTE: There is no guarantee of order among iterated elements.
 * NOTE: The Iterator must be de-constructed after its usable life-span.
 * NOTE: During the life-span of the Iterator, DO NOT modify the Table.
 * NOTE: The Iterator is NOT thread-safe. Do not share the Iterator across threads.
 */
table_Iterator* table_iter(const HashTable* const table);

/* Returns the iterator's current key/value pair and advances it forward. */
void* table_iter_next(table_Iterator* const iter, const void** const value);
/* Returns true if the iterator has a next key/value pair. */
bool table_iter_has_next(const table_Iterator* const iter);
/* De-constructor function. */
void table_iter_destroy(table_Iterator* const iter);
