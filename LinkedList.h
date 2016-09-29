
/*
Author: Kevin Tyrrell
Date: 7/21/2016
Version: 1.9
*/

#pragma once

#include "DataStructureTools.h"
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <time.h>

/* Node structure. */
struct ll_Node
{
	void *data;
	struct ll_Node *next, *prev;
};

/* LinkedList structure. */
struct LinkedList
{
	struct ll_Node *root, *tail;
	size_t size;

	/* Pointer functions */
	bool(*equals)(void*, void*);
	char*(*toString)(void*);
};

/*
Constructor function.
Must pass in two function pointers.
The first compares two data elements and returns true if they are the same.
The second function must return the String representation of the data.
*/
struct LinkedList* LinkedList_new(bool(*equals)(void*, void*), char*(*toString)(void*));

/* Inserts data at a specific position in the LinkedList. */
void ll_add(struct LinkedList *list, const size_t index, void *data);
/* Inserts data at the front of the LinkedList. */
void ll_addFirst(struct LinkedList *list, void *data);
/* Inserts data at the end of the LinkedList. */
void ll_addLast(struct LinkedList *list, void *data);
/* Removes all data inside the LinkedList. */
void ll_clear(struct LinkedList *list);
/* Returns a shallow copy of this LinkedList. */
struct LinkedList* ll_clone(const struct LinkedList *list);
/* Returns true if the LinkedList contains the specified data. */
bool ll_contains(const struct LinkedList *list, void *data);
/* Returns the data at the specified position in the LinkedList. */
void* ll_get(const struct LinkedList *list, const size_t index);
/* Returns the data at the front of the LinkedList. */
void* ll_getFirst(const struct LinkedList *list);
/* Returns the data at the end of the LinkedList. */
void* ll_getLast(const struct LinkedList *list);
/* Removes the data at the front of the LinkedList and returns it. */
void* ll_removeFirst(struct LinkedList *list);
/* Removes the data at the end of the LinkedList and returns it. */
void* ll_removeLast(struct LinkedList *list);
/* Removes the data at the specified position in the LinkedList. */
void* ll_removeAt(struct LinkedList *list, const size_t index);
/* Removes the first occurrence of the data from the LinkedList, if it exists. */
bool ll_remove(struct LinkedList *list, void *data);
/* Randomizes the position of all elements inside the LinkedList. */
void ll_shuffle(struct LinkedList *list);
/* De-constructor function. */
void ll_destroy(struct LinkedList *list);
/* Prints the LinkedList to the console window. */
void ll_print(const struct LinkedList *list);
