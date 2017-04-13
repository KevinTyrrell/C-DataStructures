
/*
Author: Kevin Tyrrell
Date: 7/21/2016
Version: 3.0
*/

#pragma once

#include "Tools.h"
#include "C-Random/Random.h"

/* Private structures. */
typedef struct LinkedList LinkedList;
typedef struct ll_Iterator ll_Iterator;

/* ~~~~~ Constructors ~~~~~ */

/*
Must pass in two function pointers.
The first compares two data elements and returns true if they are the same.
The second function must return the String representation of the data.
*/
LinkedList* LinkedList_new(int(*compare)(const void*, const void*),
	char*(*toString)(const void*));

/* ~~~~~ Accessors ~~~~~ */

/* Returns the element at the front of the list. */
void* ll_front(const LinkedList* const list);
/* Returns the element at the back of the list. */
void* ll_back(const LinkedList* const list);
/* Returns the element at the specified index in the list. */
void* ll_at(const LinkedList* const list, const size_t index);
/* Returns true if the list contains the specified element. */
bool ll_contains(const LinkedList* const list, const void* const data);
/* Returns the amount of elements inside the list. */
size_t ll_size(const LinkedList* const list);
/* Returns true if the list is empty. */
bool ll_empty(const LinkedList* const list);
/* Returns a shallow copy of the list. */
LinkedList* ll_clone(const LinkedList* const list);
/* Prints the list to the console window. */
void ll_print(const LinkedList* const list);
/* Returns an array of all elements inside the list. */
void** ll_array(const LinkedList* const list);

/* ~~~~~ Mutators ~~~~~ */

/* Inserts an element at the front of the list. */
void ll_push_front(LinkedList* const list, const void* const data);
/* Appends an element at the end of the list. */
void ll_push_back(LinkedList* const list, const void* const data);
/* Overwrites an element in the list at a given index. */
void ll_assign(const LinkedList* const list, const size_t index, const void* const data);
/* Inserts an element at a specific position in the list. */
void ll_insert(LinkedList* const list, const unsigned int index, const void* const data);
/* Removes an element at the front of the list. */
void ll_pop_front(LinkedList* const list);
/* Removes an element at the end of the list. */
void ll_pop_back(LinkedList* const list);
/* Removes the first occurrence of an element from the list. */
bool ll_remove(LinkedList* const list, const void* const data);
/* Removes the element at the specified position in the list. */
void ll_erase(LinkedList* const list, const size_t index);
/* Removes all elements inside the list. */
void ll_clear(LinkedList* const list);
/* Sorts the elements inside the list in ascending order. */
void ll_sort(LinkedList* const list);
/* Shuffles the elements in the list pseudo-randomly. */
void ll_shuffle(LinkedList* const list);

/* ~~~~~ De-constructors ~~~~~ */

void ll_destroy(LinkedList* const list);

/* ~~~~~ Iterator ~~~~~ */

/* Constructor function. */
ll_Iterator* ll_iter(const LinkedList* const list, const bool front);
/* Returns the iterator's current element and advances it forward. */
void* ll_iter_next(ll_Iterator* const iter);
/* Returns the iterator's current element and advances it backward. */
void* ll_iter_prev(ll_Iterator* const iter);
/* Returns true if the iterator has a next element. */
bool ll_iter_has_next(const ll_Iterator* const iter);
/* Returns true if the iterator has a previous element. */
bool ll_iter_has_prev(const ll_Iterator* const iter);
/* Returns the index at which this iterator is over. */
unsigned int ll_iter_index(const ll_Iterator* const iter);
/* Inserts an element at the iterator's current position. */
void ll_iter_insert(ll_Iterator* const iter, const void* const data);
/* Removes the last iterated element from the list. */
void ll_iter_remove(ll_Iterator* const iter);
/* De-constructor function. */
void ll_iter_destroy(const ll_Iterator* const iter);
