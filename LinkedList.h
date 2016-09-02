
/*
Author: Kevin Tyrrell
Date: 7/21/2016
Version: 1.9
*/

#pragma once

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include "DataStructureTools.h"

/* LinkedList structure. */
struct LinkedList
{
	struct ll_Node *root, *tail;
	size_t size;

	/* Pointer functions */
	bool(*equals)(void*, void*);
	char*(*toString)(void*);
};

/* Node structure. */
struct ll_Node
{
	void *data;
	struct ll_Node *next, *prev;
};

/* Constructor function. */
struct LinkedList* LinkedList_new(bool(*equals)(void*, void*), char*(*toString)(void*));

/* Adds data at the front of the LinkedList. */
void ll_add(struct LinkedList *list, void *data);
/* Adds data at the given index in the LinkedList. */
void ll_addAt(struct LinkedList *list, const size_t index, void *data);
/* Adds data to the end of the LinkedList. */
void ll_addLast(struct LinkedList *list, void *data);
/* Attempts to remove a Node with matching data, if it exists. */
void* ll_remove(struct LinkedList *list, void *data);
/* Removes a Node at the given index. */
void* ll_removeAt(struct LinkedList *list, const size_t index);
/* Removes the first Node of the LinkedList. */
void* ll_removeFirst(struct LinkedList *list);
/* Removes the last Node of the LinkedList. */
void* ll_removeLast(struct LinkedList *list);
/* Destroys every Node of the LinkedList. */
void ll_clear(struct LinkedList *list);
/* Returns a copy of the LinkedList. */
struct LinkedList* ll_clone(const struct LinkedList *list);
/* Deconstructor function. */
void ll_destroy(struct LinkedList *list);
/* Prints out the LinkedList to the console window. */
void ll_print(const struct LinkedList *list);
