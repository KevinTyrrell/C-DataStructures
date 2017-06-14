
/*
 * File: LinkedList.h
 * Date: Jul 21, 2016
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
#include "C-Random/Random.h"

/*
* LinkedList data structure.
* LinkedList manages elements with pointers and references.
* Elements are added dynamically without wasted memory.
* Constructor: LinkedList_new, De-constructor: list_destroy.
*/

/* Anonymous structures. */
typedef struct LinkedList LinkedList;
typedef struct list_Iterator list_Iterator;

/* ~~~~~ Constructors ~~~~~ */

/*
 * Constructs a new LinkedList.
 * Compare - Compares two elements. Returns -1, 0, or 1 based on how they compare.
 * toString - Returns the String representation of a specified element.
 *
 * NOTE: The LinkedList must be de-constructed after its usable life-span.
 */
LinkedList* LinkedList_new(int(*compare)(const void*, const void*),
                           char*(*toString)(const void*));

/* ~~~~~ Accessors ~~~~~ */

/* Returns the element at the specified index. */
void* list_at(const LinkedList* const list, const unsigned int index);
/* Returns the element at the front of the List. */
void* list_front(const LinkedList* const list);
/* Returns the element at the back of the List. */
void* list_back(const LinkedList* const list);
/* Returns the size of the List. */
size_t list_size(const LinkedList* const list);
/* Returns true if the List is empty. */
bool list_empty(const LinkedList* const list);
/* Returns the index in the List of the first occurrence of the specified element. */
bool list_index(const LinkedList* const list, const void* const data, unsigned int* const index);
/* Returns true if the List contains the specified element. */
bool list_contains(const LinkedList* const list, const void* const data);
/* Returns an array of all elements inside the List. */
void** list_array(const LinkedList* const list);
/* Prints out the contents of the List to the console window. */
void list_print(const LinkedList* const list);
/* Returns a shallow copy of the List. */
LinkedList* list_clone(const LinkedList* const list);

/* ~~~~~ Mutators ~~~~~ */

/* Replaces an element in the List at a specified index. */
void list_assign(const LinkedList* const list, const unsigned int index, const void* const data);
/* Inserts an element at the specified index in the List. */
void list_insert(LinkedList* const list, const unsigned int index, const void* const data);
/* Removes an element from the List and returns true if the removal was successful. */
bool list_remove(LinkedList* const list, const void* const data);
/* Removes an element from the List at a specified index. */
void list_erase(LinkedList* const list, const size_t index);
/* Appends an element at the end of the List. */
void list_push_back(LinkedList* const list, const void* const data);
/* Inserts an element at the front of the List. */
void list_push_front(LinkedList* const list, const void* const data);
/* Removes the element at the end of the List. */
void list_pop_back(LinkedList* const list);
/* Removes the element at the front of the List. */
void list_pop_front(LinkedList* const list);
/* Removes all elements from the List. */
void list_clear(LinkedList* const list);
/* Sorts the elements inside the List in ascending order. */
void list_sort(LinkedList* const list);
/* Shuffles the elements in the List pseudo-randomly. */
void list_shuffle(LinkedList* const list);

/* ~~~~~ De-constructors ~~~~~ */

void list_destroy(LinkedList* const list);

/* ~~~~~ Iterator ~~~~~ */

/*
 * Constructs a new Iterator for the List.
 *
 * NOTE: The Iterator must be de-constructed after its usable life-span.
 * NOTE: During the life-span of the Iterator, DO NOT modify the List.
 * NOTE: The Iterator is NOT thread-safe. Do not share the Iterator across threads.
 */
list_Iterator* list_iter(LinkedList* const list, const unsigned int index);

/* Returns the iterator's current element and advances it forward. */
void* list_iter_next(list_Iterator* const iter);
/* Returns the iterator's current element and retracts it backward. */
void* list_iter_prev(list_Iterator* const iter);
/* Returns true if the iterator has a next element. */
bool list_iter_has_next(const list_Iterator* const iter);
/* Returns true if the iterator has a previous element. */
bool list_iter_has_prev(const list_Iterator* const iter);
/* Inserts an element at the iterator's current position. */
void list_iter_insert(list_Iterator* const iter, const void* const data);
/* Removes the last iterated element from the List. */
void list_iter_remove(list_Iterator* const iter);
/* Returns the index of the last iterated element. */
unsigned int list_iter_index(const list_Iterator* const iter);
/* De-constructor function. */
void list_iter_destroy(list_Iterator* const iter);
