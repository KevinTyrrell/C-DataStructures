
/*
Author: Kevin Tyrrell
Date: 7/21/2016
Version: 2.0
*/

#pragma once

#include <time.h>

#include "Tools.h"
#include "Vector.h"

/* LinkedList structure. */
struct LinkedList
{
	struct ll_Node *root, *tail;
	size_t size;
	/* Pointer functions */
	bool(*equals)(void*, void*);
	char*(*toString)(void*);
	bool thread_lock;
};

/* Node structure. */
struct ll_Node
{
	void *data;
	struct ll_Node *next, *prev;
};

/* ~~~~~ Constructors ~~~~~ */

/*
Must pass in two function pointers.
The first compares two data elements and returns true if they are the same.
The second function must return the String representation of the data.
*/
struct LinkedList* LinkedList_new(bool(*equals)(void*, void*), char*(*toString)(void*));

/* ~~~~~ Accessors ~~~~~ */

/* Returns the data at the front of the LinkedList. */
void* ll_front(const struct LinkedList* const list);
/* Returns the data at the end of the LinkedList. */
void* ll_back(const struct LinkedList* const list);
/* Returns the data at the specified position in the LinkedList. */
void* ll_at(const struct LinkedList* const list, const size_t index);
/* Returns true if the LinkedList contains the specified data. */
bool ll_contains(const struct LinkedList* const list, const void* const data);
/* Returns true if the LinkedList is empty. */
bool ll_empty(const struct LinkedList* const list);
/* Returns a shallow copy of this LinkedList. */
struct LinkedList* ll_clone(const struct LinkedList* const list);
/* Prints the LinkedList to the console window. */
void ll_print(const struct LinkedList* const list);
// TODO:
void** ll_array(const struct LinkedList* const list);
struct Vector* ll_vector(const struct LinkedList* const list);

/* ~~~~~ Mutators ~~~~~ */

/* Inserts data at the front of the LinkedList. */
void ll_push_front(struct LinkedList* const list, const void* const data);
/* Inserts data at the end of the LinkedList. */
void ll_push_back(struct LinkedList* const list, const void* const data);
/* Overwrites a value in the LinkedList at a given index. */
void ll_assign(const struct LinkedList* const list, const size_t index, const void* const data);
/* Inserts data at a specific position in the LinkedList. */
void ll_insert(struct LinkedList* const list, const size_t index, const void* const data);
/* Removes the data at the front of the LinkedList and returns it. */
void ll_pop_front(struct LinkedList* const list);
/* Removes the data at the end of the LinkedList and returns it. */
void ll_pop_back(struct LinkedList* const list);
/* Removes the first occurrence of the data from the LinkedList, if it exists. */
bool ll_remove(struct LinkedList* const list, const void* const data);
/* Removes the data at the specified position in the LinkedList. */
void ll_erase(struct LinkedList* const list, const size_t index);
/* Removes all data inside the LinkedList. */
void ll_clear(struct LinkedList* const list);
/* Randomizes the position of all elements inside the LinkedList. */
void ll_shuffle(const struct LinkedList* const list);

/* ~~~~~ De-constructors ~~~~~ */

void ll_destroy(const struct LinkedList* const list);