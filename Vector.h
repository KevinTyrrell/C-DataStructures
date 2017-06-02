
/*
 * Author: Kevin Tyrrell
 * Date: 2/27/2017
 * Version: 3.0
 */

#pragma once

#include "tools/Memory.h"
#include "tools/Synchronize.h"
#include "C-Random/Random.h"

#include <math.h>

/* Anonymous structure. */
typedef struct Vector Vector;

/* ~~~~~ Constructors ~~~~~ */

Vector* Vector_new(int(*compare)(const void*, const void*), char*(*toString)(const void*));

/* ~~~~~ Accessors ~~~~~ */

/* Returns the value at the given index. */
void* vect_at(const Vector* const vect, const unsigned int index);
/* Returns the value at the front one the Vector. */
void* vect_front(const Vector* const vect);
/* Returns the value at the end of the Vector. */
void* vect_back(const Vector* const vect);
/* Returns the size of the Vector. */
size_t vect_size(const Vector* const vect);
/* Returns true if the Vector is empty. */
bool vect_empty(const Vector* const vect);
/* Returns true if the Vector contains the provided element. */
bool vect_contains(const Vector* const vect, const void* const data);
/* Returns an array of all elements inside the Vector. */
void** vect_array(const Vector* const vect);
/* Prints out the contents of the Vector using the toString function. */
void vect_print(const Vector* const vect);

/* ~~~~~ Mutators ~~~~~ */

/* Replaces an element in the Vector at a given index with a specified value. */
void vect_assign(const Vector* const vect, const unsigned int index, const void* const data);
/* Inserts the given element at the provided index. */
void vect_insert(Vector* const vect, const unsigned int index, const void* const data);
/* Attempts to remove a provided element from the Vector, if it exists. */
bool vect_remove(Vector* const vect, const void* const data);
/* Erases an element from the Vector at a given index. */
void vect_erase(Vector* const vect, const unsigned int index);
/* Inserts the given element at the end of the Vector. */
void vect_push_back(Vector* const vect, const void * const data);
/* Inserts the given element at the front of the Vector. */
void vect_push_front(Vector* const vect, const void * const data);
/* Removes the element at the end of the Vector. */
void vect_pop_back(Vector* const vect);
/* Removes the element at the front of the Vector. */
void vect_pop_front(Vector* const vect);
/* Append data from another Vector to the end of this Vector. */
void vect_append(Vector* const vect, const Vector* const other);
/* Append data from an array to the end of this Vector. */
void vect_append_array(Vector* const vect, const void** const data, const size_t size);
/* Grows the underlying array to be able to store at least `min_size` elements. */
void vect_grow_to(Vector* const vect, const size_t min_size);
/* Removes all elements from the Vector while preserving the capacity. */
void vect_clear(Vector* const vect);
/* Sorts elements inside the Vector in descending order. */
void vect_sort(const Vector* const vect);
/* Shuffles the elements inside the Vector randomly. */
void vect_shuffle(const Vector* const vect);

/* ~~~~~ De-constructors ~~~~~ */

void vect_destroy(const Vector* const vect);
