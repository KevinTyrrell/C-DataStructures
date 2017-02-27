
/*
Author: Kevin Tyrrell
Date: 9/01/2016
Version: 2.0
*/

#pragma once

#include "Tools.h"
#include "LinkedList.h"

#include <stdbool.h>
#include <math.h>

/* Red/Black Tree structure. */
struct RedBlackTree
{
	struct rbt_Node *root;
	size_t size;

	/* Function pointers.
	/* Compare function is needed to compare Keys of each Key/Value pair. */
	int(*compare)(const void* const, const void* const);
	/* Returns a given Key/Value pair as a String. */
	char*(*toString)(const void* const);
};

/* KeySet for Key/Value entries. */
struct rbt_KeySet
{
	void *key, *value;
};

/* Node of the RedBlackTree. */
struct rbt_Node
{
	/* Key Value of this Node. */
	struct rbt_KeySet *set;
	/* color	-	Color of the Node. True = red, False = black.
	rl_child	-	Which child is the Node. True = left child, False = right child. */
	bool color, rl_child;
	/* Pointers to both children and the parent Nodes. */
	struct rbt_Node *parent, *left, *right;
};

/* Constructor function. */
struct RedBlackTree* RedBlackTree_new(int(*compare)(const void*, const void*), char*(*toString)(void*));

/* Places a Key/Value pair into the tree. */
void rbt_put(struct RedBlackTree* const tree, const void* const key, const void* const value);
/* Returns the value associated with the key inside the tree. NULL if one doesn't exist. */
void* rbt_get(const struct RedBlackTree* const tree, const void* const key);
/* Returns tree if the Tree is empty. */
bool rbt_isEmpty(const struct RedBlackTree* const tree);
/* Returns true if the tree contains the specified key. */
bool rbt_contains(const struct RedBlackTree* const tree, const void* const key);
/* Returns the vertical height of the tree. */
unsigned int getHeight(const struct RedBlackTree* const tree);
/* Prints the Red Black Tree out to the console window. */
void rbt_print(const struct RedBlackTree* const tree);
/* De-constructor function. */
void rbt_destroy(const struct RedBlackTree* const tree);
