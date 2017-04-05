
/*
Author: Kevin Tyrrell
Date: 2/27/2017
Version: 2.0
*/

#pragma once

#include "Tools.h"
#include "C-Random/Random.h"

#include <math.h>

/* Vector structure. */
struct Vector
{
	const void** table;
	/* Start and end let us know where the data is. */
	unsigned int start, end;
	size_t size, capacity;

	/* Function pointers.
	/* Compare function is needed to compare Keys of each Key/Value pair. */
	int(*compare)(const void* const, const void* const);
	/* Returns a given Key/Value pair as a String. */
	char*(*toString)(const void* const);
};

/* ~~~~~ Constructors ~~~~~ */

struct Vector* Vector_new(int(*compare)(const void *, const void *), char*(*toString)(const void* const));

/* ~~~~~ Accessors ~~~~~ */

/* Returns the value at the given index. */
void* vect_at(const struct Vector* const vect, const unsigned int index);
/* Returns the value at the front one the Vector. */
void* vect_front(const struct Vector* const vect);
/* Returns the value at the end of the Vector. */
void* vect_back(const struct Vector* const vect);
/* Returns the size of the Vector. */
size_t vect_size(const struct Vector* const vect);
/* Returns true if the Vector is empty. */
bool vect_empty(const struct Vector* const vect);
/* Returns true if the Vector contains the provided element. */
bool vect_contains(const struct Vector* const vect, const void* const data);
/* Returns an array of all elements inside the Vector. */
void** vect_array(const struct Vector* const vect);
/* Prints out the contents of the Vector using the toString function. */
void vect_print(const struct Vector* const vect);
/* Prints out the internal structure of the inner array. */
void vect_debug_print(const struct Vector* const vect);

/* ~~~~~ Mutators ~~~~~ */

/* Replaces an element in the Vector at a given index with a specified value. */
void vect_assign(const struct Vector* const vect, const unsigned int index, const void* const data);
/* Inserts the given element at the provided index. */
void vect_insert(struct Vector* const vect, const unsigned int index, const void* const data);
/* Attempts to remove a provided element from the Vector, if it exists. */
bool vect_remove(struct Vector* const vect, const void* const data);
/* Erases an element from the Vector at a given index. */
void vect_erase(struct Vector* const vect, const unsigned int index);
/* Inserts the given element at the end of the Vector. */
void vect_push_back(struct Vector* const vect, const void * const data);
/* Inserts the given element at the front of the Vector. */
void vect_push_front(struct Vector* const vect, const void * const data);
/* Removes the element at the end of the Vector. */
void vect_pop_back(struct Vector* const vect);
/* Removes the element at the front of the Vector. */
void vect_pop_front(struct Vector* const vect);
/* Append data from another Vector to the end of this Vector. */
void vect_append_vect(const struct Vector* const vect, const struct Vector* const data);
/* Append data from an array to the end of this Vector. */
void vect_append_array(const struct Vector* const vect, const void** const data, const size_t size);
/* Grows the underlying array to be able to store at least `min_size` elements. */
void vect_grow_to(struct Vector* const vect, const size_t min_size);
/* Removes all elements from the Vector while preserving the capacity. */
void vect_clear(struct Vector* const vect);
/* Sorts elements inside the Vector in descending order. */
void vect_sort(const struct Vector* const vect);
/* Shuffles the elements inside the Vector randomly. */
void vect_shuffle(const struct Vector* const vect);

/* ~~~~~ De-constructors ~~~~~ */

void vect_destroy(const struct Vector* const vect);
