
/*
 * File: Dictionary.h
 * Date: Sep 01, 2016
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
#include "tools/Math.h"
#include "Vector.h"

/* Anonymous structures. */
typedef struct Dictionary Dictionary;
typedef struct dict_Iterator dict_Iterator;
/* Enum to be used on Iterator creation. */
enum dict_iter_traversal { IN_ORDER, PRE_ORDER, POST_ORDER };

/* ~~~~~ Constructors ~~~~~ */

/*
 * Constructs a new Dictionary.
 * Compare - Compares two keys. Returns -1, 0, or 1 based on how they compare.
 * toString - Returns the String representation of a specified key/value pair.
 *
 * NOTE: The Compare function MUST be defined.
 * NOTE: The Dictionary must be de-constructed after its usable life-span.
 */
Dictionary* Dictionary_new(int(*compare)(const void*, const void*),
                           char*(*toString)(const void*, const void*));

/* ~~~~~ Accessors ~~~~~ */

/* Returns the number of elements in the Dictionary. */
size_t dict_size(const Dictionary* const dict);
/* Returns true if the Dictionary is empty. */
bool dict_empty(const Dictionary* const dict);
/* Prints out the structure of the Dictionary to the console window. */
void dict_print_tree(const Dictionary* const dict);

/* ~~~~~ Mutators ~~~~~ */

/* Inserts a key/value pair into the Dictionary. */
void dict_put(Dictionary* const dict, const void* const key, const void* const value);
/* Removes all key/value pairs from the Dictionary. */
void dict_clear(Dictionary* const dict);

/* ~~~~~ De-constructors ~~~~~ */

void dict_destroy(Dictionary* const dict);

/* ~~~~~ Iterator ~~~~~ */

/*
 * Constructs a new Iterator for the Dictionary.
 *
 * NOTE: The Iterator must be de-constructed after its usable life-span.
 * NOTE: During the life-span of the Iterator, DO NOT modify the Dictionary.
 * NOTE: The Iterator is NOT thread-safe. Do not share the Iterator across threads.
 */
dict_Iterator* dict_iter(const Dictionary* const dict, const enum dict_iter_traversal traverse_type);

/* Returns the iterator's current key/value pair and advances it forward. */
void* dict_iter_next(dict_Iterator* const iter, void** value);
/* Returns true if the iterator has a next key/value pair. */
bool dict_iter_has_next(const dict_Iterator* const iter);
/* De-constructor function. */
void dict_iter_destroy(dict_Iterator* const iter);