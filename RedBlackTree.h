#pragma once

#include "DataStructureTools.h"
#include "LinkedList.h"
#include <stdbool.h>
#include <stdlib.h>
#include <Windows.h>
#include <math.h>

/* Red/Black Tree -- Key/Value data structure. */
struct RedBlackTree
{
	struct rbt_Node *root;
	size_t size;

	/* Function pointers.
	/* Compare function is needed to compare Keys of each Key/Value pair. */
	int(*compare)(const void*, const void*);
};

/* Node of the RedBlackTree. */
struct rbt_Node
{
	/* Key Value of this Node. */
	void *key, *value;
	/* color	-	Color of the Node. True = red, False = black.
	rl_child	-	Which child is the Node. True = left child, False = right child. */
	bool color, rl_child;
	/* Pointers to both children and the parent Nodes. */
	struct rbt_Node *parent, *left, *right;
};

/* Constructor function. */
struct RedBlackTree* RedBlackTree_new(int(*compare)(const void*, const void*));
/* Places a key/value pair into the tree. */
void* rbt_put(struct RedBlackTree *tree, const void *key, const void *value);
/* Prints the Red Black Tree out to the console window. */
void rbt_print(const struct RedBlackTree *tree);
