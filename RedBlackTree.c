
#include "RedBlackTree.h"

#define RED true
#define BLACK false
#define LEFT true
#define RIGHT false

#define SPACING "                                                                                "
#define COLOR_RED 12
#define COLOR_BLACK 240
#define CHAR_NULL 254
#define CHAR_POINTER_L '/'
#define CHAR_POINTER_R '\\'

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
static unsigned int getHeight(const Tree *tree);
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
	trunk->compare = compare;
	return trunk;
}

/* Constructor function. */
static Node* Node_new(void *key, void *value, Node *parent, bool color, bool which)
{
	Node *child = calloc(1, sizeof(Node));
	if (child == NULL)
		ds_Error(DS_MSG_OUT_OF_MEM);
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
void* rbt_put(struct RedBlackTree *tree, const void *key, const void *value)
{
	if (tree->root == NULL)
		tree->root = Node_new((void*)key, (void*)value, NULL, BLACK, LEFT);
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
			parent->value = (void*)value;
			return val;
		}

		/* Create and insert the red node into the tree.
		Determine if it should be a left or right child. */
		Node *child = Node_new((void*)key, (void*)value, parent, RED, cmp < 0);
		//insertionCase1(child);

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
	
	if (isRoot(child) || (grandparent = getGrandparent(child)) == NULL || (uncle = getUncle(child)) == NULL)
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
	else if (!isleaf(var))
	{
		int cmp = tree->compare(key, var->key);
		if (cmp < 0 && var->left != NULL)
			return rbt_find(tree, var->left, key, strict);
		else if (cmp > 0 && var->right != NULL)
			return rbt_find(tree, var->right, key, strict);
		else
			return (Node*)var;
	}
	
	return (strict) ? NULL : (Node*)var;
}

/* Assign a child to a specific parent.
Inform the child of who its parent is and if it is a left or right child.
Inform the parent of if this is its left or right child.
Return the child that was overwritten, if one exists. NULL otherwise. */
static Node* assignChild(Node *parent, Node *child, bool which)
{
	Node *old = NULL;
	
	if (parent != NULL)
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
	if (child != NULL)
	{
		child->parent = parent;
		child->rl_child = which;
	}
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

/* Returns the height of the tree. */
static unsigned int getHeight(const Tree *tree)
{
	if (tree->root == NULL)
		return 0;
	
	struct LinkedList *queue = LinkedList_new(NULL, NULL);	
	ll_addLast(queue, tree->root);

	unsigned int height = 0;

	for (;queue->size > 0; height++)
		/* Loop over every Node on this row. */
		for (size_t nodes = queue->size; nodes > 0; nodes--)
		{
			Node *current = ll_removeFirst(queue);
			if (current->left != NULL)
				ll_addLast(queue, current->left);
			if (current->right != NULL)
				ll_addLast(queue, current->right);
		}

	ll_destroy(queue);
	return height;
}

/* Prints the Red Black Tree out to the console window. */
void rbt_print(const struct RedBlackTree *tree)
{
	if (tree->size == 0)
		return;

	/* We will use this to loop through the tree, row by row. */
	struct LinkedList *queue = LinkedList_new(NULL, NULL);
	/* Add the root to get us started. */
	ll_addLast(queue, tree->root);

	for (int height = getHeight(tree), row = 1; row <= height; row++)
	{
		/* Initial line spacing. */
		printf_s("%.*s", -2 + (int)pow(2, (double)height - (double)row + 1), SPACING);

		/* Enter the row of the tree. */
		for (size_t nodes = queue->size; nodes > 0; nodes--)
		{
			Node *current = ll_removeFirst(queue);
			
			/* Change the color and print this Node's data. */
			WORD prevColor = 0;			
			/* NULL key means this Node was a fake that we created. */
			if (current->key != NULL)
				prevColor = ds_changeColor(current->color == RED ? COLOR_RED : COLOR_BLACK);
			printf_s("%c", current->key == NULL ? CHAR_NULL : *((char*)current->key));
			if (prevColor != 0)
				ds_changeColor(prevColor);
			/* If another Node comes after this one, add spaces for it. */
			if (nodes > 1)
				printf_s("%.*s", -1 + (int)pow(2, (double)height - (double)row + 2), SPACING);
			
			/* Setup the children of this Node so we know what's underneath us.
			If we are on the bottom floor of the tree, there's not another row to be added. */
			if (row != height)
			{
				/* NULL children are going to mess up the printing very badly here.
				Create 'pretend' fake Nodes which will serve as placeholders 
				for what COULD have existed in this slot. That way, the tree in
				printed form will still look normal even though there gaps in it (NULL). */
				ll_addLast(queue, current->left == NULL ? Node_new(NULL, NULL, NULL, RED, LEFT) : current->left);
				ll_addLast(queue, current->right == NULL ? Node_new(NULL, NULL, NULL, RED, LEFT) : current->right);
			}
		}

		/* Add the arrows / arms which direct to the children of this row. */
		if (row != height)
		{
			/* Initial line spacing. */
			printf_s("\n%.*s", -1 + (int)pow(2, (double)height - (double)row), SPACING);
			for (size_t i = 0, sets = queue->size / 2; i < sets; i++)
			{
				/* Print out the  /     \  that go to the row below. */
				printf_s("%c%.*s%c", CHAR_POINTER_L, 
					(int)pow(2, (double)height - (double)row + 1) - 3, SPACING, CHAR_POINTER_R);
				/* Add spacing for the next set. */
				if (i + 1 != sets)
					printf_s("%.*s", (int)pow(2, (double)height - (double)row + 1) + 1, SPACING);
			}
		}
		printf_s("\n");
	}
	
	ll_destroy(queue);
}