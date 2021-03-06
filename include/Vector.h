
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

/*
 * File Name:       Vector.h
 * File Author:     Kevin Tyrrell
 * Date Created:    02/27/2017
 */

#pragma once

#include "../tools/Memory.h"
#include "../tools/Math.h"
#include "../tools/Synchronize.h"
#include "C-Random/Random.h"

/* Anonymous structures. */
typedef struct Vector Vector;
typedef struct vect_Iterator vect_Iterator;

/* ~~~~~ Constructors ~~~~~ */

/*
 * Constructs a new Vector.
 * Compare - Compares two elements. Returns -1, 0, or 1 based on how they compare.
 * toString - Returns the String representation of a specified element.
 *
 * NOTE: The Vector must be de-constructed after its usable life-span.
 */
Vector* Vector_new(int(*compare)(const void*, const void*),
                   char*(*toString)(const void*));

/* ~~~~~ Accessors ~~~~~ */

/* Returns the element at the specified index. */
void* vect_at(const Vector* const vect, const unsigned int index);
/* Returns the element at the front one the Vector. */
void* vect_front(const Vector* const vect);
/* Returns the element at the back of the Vector. */
void* vect_back(const Vector* const vect);
/* Returns the number of elements in the Vector. */
size_t vect_size(const Vector* const vect);
/* Returns true if the Vector is empty. */
bool vect_empty(const Vector* const vect);
/* Returns the index in the Vector of the first occurrence of the specified element. */
bool vect_index(const Vector* const vect, const void* const data, unsigned int* const index);
/* Returns true if the Vector contains the specified element. */
bool vect_contains(const Vector* const vect, const void* const data);
/* Prints out the contents of the Vector to the console window. */
void vect_print(const Vector* const vect);
/* Returns a shallow copy of the Vector. */
Vector* vect_clone(const Vector* const vect);

/* ~~~~~ Mutators ~~~~~ */

/* Replaces an element in the Vector at a specified index. */
void vect_assign(const Vector* const vect, const unsigned int index, const void* const data);
/* Inserts an element at the specified index in the Vector. */
void vect_insert(Vector* const vect, const unsigned int index, const void* const data);
/* Removes an element from the Vector and returns true if the removal was successful. */
bool vect_remove(Vector* const vect, const void* const data);
/* Removes an element from the Vector at a specified index. */
void vect_erase(Vector* const vect, const unsigned int index);
/* Appends an element at the end of the Vector. */
void vect_push_back(Vector* const vect, const void * const data);
/* Inserts an element at the front of the Vector. */
void vect_push_front(Vector* const vect, const void * const data);
/* Removes the element at the end of the Vector. */
void vect_pop_back(Vector* const vect);
/* Removes the element at the front of the Vector. */
void vect_pop_front(Vector* const vect);
/* Appends all data from another Vector to the end of this Vector. */
void vect_append(Vector* const vect, const Vector* const other);
/* Changes the Vector's capacity to accommodate at least the specified number of elements. */
void vect_resize(Vector *const vect, const size_t min_size);
/* Removes all elements from the Vector while preserving the capacity. */
void vect_clear(Vector* const vect);
/* Sorts elements inside the Vector in ascending order. */
void vect_sort(const Vector* const vect);
/* Shuffles the elements inside the Vector pseudo-randomly. */
void vect_shuffle(const Vector* const vect);

/* ~~~~~ De-constructors ~~~~~ */

void vect_destroy(Vector* const vect);

/* ~~~~~ Iterator ~~~~~ */

/*
 * Constructs a new Iterator for the Vector.
 *
 * NOTE: The Iterator must be de-constructed after its usable life-span.
 * NOTE: During the life-span of the Iterator, DO NOT modify the Vector.
 * NOTE: The Iterator is NOT thread-safe. Do not share the Iterator across threads.
 * // TODO: Implement this as a full ListIterator.
 */
vect_Iterator* vect_iter(const Vector* const vect, const unsigned int index);

/* Returns the iterator's current element and advances it forward. */
void* vect_iter_next(vect_Iterator* const iter);
/* Returns the iterator's current element and retracts it backward. */
void* vect_iter_prev(vect_Iterator* const iter);
/* Returns true if the iterator has a next element. */
bool vect_iter_has_next(const vect_Iterator* const iter);
/* Returns true if the iterator has a previous element. */
bool vect_iter_has_prev(const vect_Iterator* const iter);
/* De-constructor function. */
void vect_iter_destroy(vect_Iterator* const iter);