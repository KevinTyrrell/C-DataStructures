
#include "RedBlackTree.h"

#define RED true
#define BLACK false
#define LEFT true
#define RIGHT false

/*
Four rules:
1) Each Node is either red or black
2) Root is always black.
3) Red Nodes can only have black children.
4) Every path of the tree must have the exact same number of black Nodes.
*/

/* Local typedef for convenience. */
typedef struct rbt_Node Node;
typedef struct RedBlackTree Tree;

/* Local functions. */
static Node* Node_new(void *key, void *value, Node *parent, bool color, bool rl_child);
static Node* rbt_find(const Tree *tree, const Node *var, const void *key, const bool strict);
static Node* assignChild(Node *parent, Node *child, bool which);
static Node* getGrandparent(const Node *child);
static Node* getUncle(const Node *child);
static bool isleaf(const Node *var);
static bool isRoot(const Node *subject);

static void insertionCase1(Node *child);

/* Constructor function. */
struct RedBlackTree* RedBlackTree_new(int(*compare)(const void*, const void*))
{
	Tree *trunk = calloc(1, sizeof(Tree));
	if (trunk == NULL)
		ds_Error(DS_MSG_OUT_OF_MEM);
	return trunk;
}

/* Constructor function. */
static Node* Node_new(void *key, void *value, Node *parent, bool color, bool which)
{
	Node *child = calloc(1, sizeof(Node));
	child->key = key;
	child->value = value;
	child->color = color;
	assignChild(parent, child, which);
	return child;
}

/*
Inserts a Key/Value set into the tree.
Returns the previous value associated with the key,
or NULL if there was no previous mapping for the key.
*/
void* rbt_put(Tree *tree, const void *key, const void *value)
{
	if (tree->root == NULL)
		tree->root = Node_new(key, value, NULL, BLACK, LEFT);
	else
	{
		/* Locate the parent of where this Key/Value set SHOULD be. */
		Node *parent = rbt_find(tree, tree->root, key, false);
		int cmp = tree->compare(key, parent->key);

		/* Provided key is exactly the same as this key.
		Overwrite previous value, return the old value. */
		if (cmp == 0)
		{
			void *val = parent->value;
			parent->value = value;
			return val;
		}

		/* Create and insert the red node into the tree.
		Determine if it should be a left or right child. */
		Node *child = Node_new(key, value, parent, RED, cmp < 0);
		insertionCase1(child);

		/* 
		/* Case 3: Uncle is black or NULL and Child is a left child.
			Parent takes the place of Grandparent while Grandparent becomes
			the right child of Parent and inherits Parent's	previous right
			child as his new left child. Parent becomes black and Grandparent red.
		if ((uncle == NULL || uncle->color == BLACK) && child->rl_child == LEFT)
		{
			/* Temporary variable as we are doing swapping.
			Node *temp = parent->right;

			/* Parent becomes the new 'grandparent'.
			if (isRoot(grandparent))
				tree->root = parent;
			else
				assignChild(grandparent->parent, parent, grandparent->rl_child);

			assignChild(parent, grandparent, RIGHT);
			assignChild(grandparent, temp, LEFT);
			parent->color = BLACK;
			grandparent->color = RED;
		}*/
	}

	tree->size++;
	return NULL;
}

/*
Case 1: Uncle is red. Re-color key Nodes.
Re-color the parent and uncle to black.
If the grandparent is not the root, re-color him to red.
Recursively call cases on the grandparent.
*/
static void insertionCase1(Node *child)
{
	Node *grandparent, *uncle;
	
	if (isRoot(child) || (grandparent = getGrandparent) == NULL || (uncle = getUncle(child)) == NULL)
		return;
	if (uncle->color == RED)
	{
		uncle->color = BLACK;
		child->parent->color = BLACK;
		if (!isRoot(grandparent))
			grandparent->color = RED;
		insertionCase1(grandparent);
	}
	//else
		//insertionCase2(child);
}

/* 
Helper function. 
Recursively search through the RedBlackTree in search for a given key.
If the key does not exist in the tree, return depending on 'strict'.
If strict is true, return null meaning it doesn't exist.
If strict is false, return the would-be parent of if it were to exist.
*/
static Node* rbt_find(const Tree *tree, const Node *var, const void *key, const bool strict)
{
	if (var == NULL)
		return NULL;
	else if (!rbt_isleaf(var))
	{
		int cmp = tree->compare(key, var->key);
		if (cmp < 0 && var->left != NULL)
			return rbt_find(tree, var->left, key, strict);
		else if (cmp > 0 && var->right != NULL)
			return rbt_find(tree, var->right, key, strict);
		else
			return var;
	}
	
	return (strict) ? NULL : var;
}

/* Assign a new child to this parent, based on direction.
Return the child that was overwritten, if one exists. NULL otherwise. */
static Node* assignChild(Node *parent, Node *child, bool which)
{
	Node *old;
	if (which == LEFT)
	{
		old = parent->left;
		parent->left = child;
	}
	else
	{
		old = parent->right;
		parent->right = child;
	}
	/* Tell the child if it is a right child or a left child. */
	if (child != NULL)
		child->rl_child = which;
	return old;
}

/* Check if a given Node is a leaf. */
static bool isleaf(const Node *subject)
{
	return subject->left == NULL && subject->right == NULL;
}

/* Returns whether or not the parameter Node is the root of the tree. */
static bool isRoot(const Node *subject)
{
	return subject->parent == NULL;
}

/* Returns the grandparent Node of the parameter Node if it exists, otherwise NULL. */
static Node* getGrandparent(const Node *child)
{
	return (child != NULL && child->parent != NULL) ? child->parent->parent : NULL;
}

/* Returns the uncle Node of the parameter Node if it exists, otherwise NULL. */
static Node* getUncle(const Node *child)
{
	Node *grandparent = getGrandparent(child);
	if (grandparent == NULL)
		return NULL;
	return (child->parent->rl_child == RIGHT) ? grandparent->left : grandparent->right;
}

static size_t getHeight(Tree *tree)
{
	for (struct LinkedList *list = LinkedList_new(NULL, NULL);;)
	{

	}
	
	// ll_destroy(list);
	return 0;
}

/* Prints the Red Black Tree out to the console window. */
void rbt_Print(const Tree *tree)
{

}