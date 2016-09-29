
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
static Node* getGrandparent(const Node *child);
static Node* getUncle(const Node *child);
static bool isleaf(const Node *var);
static bool isRoot(const Node *subject);

static void insertionCase1(Tree *tree, Node *child);
static void insertionCase2(Tree *tree, Node *child);
static void insertionCase3(Tree *tree, Node *child);

static unsigned int getHeight(const struct RedBlackTree *tree);

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

		/* Fix up the Tree if it's in violation. */
		insertionCase1(tree, child);
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
static void insertionCase1(Tree *tree, Node *child)
{
	Node *parent = child->parent, *uncle, *grandparent;

	/* Violation only occurs when child and parent are both red. */
	if (child->color == RED && parent->color == RED)
		if ((uncle = getUncle(child)) != NULL && uncle->color == RED)
		{
			grandparent = getGrandparent(child);
			uncle->color = BLACK;
			parent->color = BLACK;
			if (!isRoot(grandparent))
				grandparent->color = RED;
			/* Re-check this case further up the tree. */
			insertionCase1(tree, grandparent);
		}
		else
			insertionCase2(tree, child);
}

/*
Case 2: Uncle is black. Child is either a left child of a
right parent or a right child of a left parent.
Child becomes the parent of parent and also inherits
the parent's old other child.
Proceed to case 3 with the parent (who is now a child).
*/
static void insertionCase2(Tree *tree, Node *child)
{
	Node *parent = child->parent, *uncle = getUncle(child), *grandparent;
	
	if ((uncle == NULL || uncle->color == BLACK) && parent->rl_child != child->rl_child)
	{
		grandparent = getGrandparent(child);
		bool direction = parent->rl_child;
		Node *temp = (direction == LEFT) ? child->left : child->right;
		assignChild(grandparent, child, direction);
		assignChild(child, parent, direction);
		assignChild(child, temp, !direction);
		insertionCase3(tree, parent);
	}
	else
		insertionCase3(tree, child);
}

/*
Case 3: Uncle is black. Child is either a left child of a
left parent or a right child of a right parent.
Right or left rotation is performed. 
*/
static void insertionCase3(Tree *tree, Node *child)
{
	Node *parent = child->parent, *uncle = getUncle(child),
		*grandparent;

	if ((uncle == NULL || uncle->color == BLACK) && child->rl_child == parent->rl_child)
	{
		grandparent = getGrandparent(child);
		bool direction = parent->rl_child, rootChanged;
		/* If there is more of the tree above us, connect it to parent. */
		if ((rootChanged = isRoot(grandparent)) == false)
			assignChild(grandparent->parent, parent, grandparent->rl_child);
		Node *temp = (direction == LEFT) ? parent->right : parent->left;
		assignChild(parent, grandparent, !direction);
		assignChild(grandparent, temp, direction);
		grandparent->color = RED;
		parent->color = BLACK;
		/* It's possible we messed up the root, fix it. */
		if (rootChanged)
		{
			tree->root = parent;
			tree->root->parent = NULL;
		}
	}
}

/* Returns the value associated with the key inside the tree. NULL if one doesn't exist. */
void* rbt_get(struct RedBlackTree *tree, const void *key)
{
	Node *keyset = rbt_find(tree, tree->root, key, true);
	return (keyset != NULL) ? keyset->value : NULL;
}

/* Returns true if the tree contains the specified key. */
bool rbt_contains(struct RedBlackTree *tree, const void *key)
{
	return rbt_find(tree, tree->root, key, true) != NULL;
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
	{
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
		/* Inform the overwritten child that he no longer has a parent. */
		if (old != NULL)
			old->parent = NULL;
	}
	if (child != NULL)
	{
		/* Inform the previous parent that he no longer owns this child. */
		if (child->parent != NULL)
			if (child->rl_child == LEFT)
				child->parent->left = NULL;
			else
				child->parent->right = NULL;
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
static unsigned int getHeight(const struct RedBlackTree *tree)
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

/* De-constructor function. */
void rbt_destroy(struct RedBlackTree *tree)
{
	if (tree->size > 0)
	{
		struct LinkedList *queue = LinkedList_new(NULL, NULL);
		ll_addLast(queue, tree->root);

		while (queue->size > 0)
			/* Go through all the Nodes on this row. */
			for (size_t nodes = queue->size; nodes > 0; nodes--)
			{
				/* Prepare the next row. */
				Node *current = ll_removeFirst(queue);
				if (current->left != NULL)
					ll_addLast(queue, current->left);
				if (current->right != NULL)
					ll_addLast(queue, current->right);
				free(current);
			}

		ll_destroy(queue);
	}
	
	free(tree);
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
	printf_s("%s%zu\n", "Red Black Tree - Size: ", tree->size);

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
