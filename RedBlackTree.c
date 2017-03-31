
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

#define RBT_ERR_NULL_KEY "Key for Key/Value pair must not be NULL!\n"

/*
Four rules:
1) Each Node is either red or black
2) Root is always black.
3) Red Nodes can only have black children.
4) Every path of the tree must have the exact same number of black Nodes.
*/

/* Local typedef for convenience. */
typedef struct RedBlackTree Tree;
typedef struct rbt_Node Node;
typedef struct rbt_KeySet KeySet;

/* Constructor functions. */
Node* rbt_Node_new(const KeySet* const set, const Node* const parent, const bool color, const bool rl_child);
KeySet* rbt_KeySet_new(const void* const key, const void* value);

/* De-constructor functions. */
/* If deep, destroy all members as well. */
void rbt_Node_destroy(const Node* const val, const bool deep);
void rbt_KeySet_destroy(const KeySet* const set);

/* Convenience functions. */
/* Recursively search through the Tree in search for a Node holding a specific Key. */
Node* rbt_search(const Tree* const tree, const Node* const var, const void* const key);
/* Assign a child to a specific parent. */
void assignChild(Node* const parent, Node* const child, const bool leftRight);
/* Returns the uncle Node of the parameter Node if it exists, otherwise NULL. */
Node* getUncle(const Node* const child);
/* Returns the grandparent Node of the parameter Node if it exists, otherwise NULL. */
Node* getGrandparent(const Node* const child);
/* Check if a given Node is a leaf. */
bool isLeaf(const Node* const subject);
/* Returns whether or not the parameter Node is the root of the tree. */
bool isRoot(const Node* const subject);

/* Constructor function. */
struct RedBlackTree* RedBlackTree_new(int(*compare)(const void* const, const void* const), char*(*toString)(const void* const))
{
	Tree* const tree = ds_calloc(1, sizeof(Tree));
	tree->compare = compare;
	tree->toString = toString;
	return tree;
}

/* Returns the value associated with the key inside the tree. NULL if one doesn't exist. */
void* rbt_get(const struct RedBlackTree* const tree, const void* const key)
{
	const Node* const found = rbt_search(tree, tree->root, key);
	return found != NULL ? found->set->value : NULL;
}

/* Returns true if the tree contains the specified key. */
bool rbt_contains(const struct RedBlackTree* const tree, const void* const key)
{
	const Node* const located = rbt_search(tree, tree->root, key);
	return tree->compare(located->set->key, key) == 0;
}

/* Returns tree if the Tree is empty. */
bool rbt_isEmpty(const struct RedBlackTree* const tree)
{
	return tree->size == 0;
}

/* Returns the vertical height of the tree. */
unsigned int rbt_height(const struct RedBlackTree* const tree)
{
	if (rbt_isEmpty(tree))
		return 0;

	const struct LinkedList* const queue = LinkedList_new(NULL, NULL);
	ll_push_back(queue, tree->root);

	unsigned int height = 0;
	while (!ll_empty(queue))
	{
		/* Loop over every Node on this row. */
		for (unsigned int nodes = queue->size; nodes > 0; nodes--)
		{
			const Node* const current = ll_front(queue);
			ll_pop_front(queue);
			if (current->left != NULL)
				ll_push_back(queue, current->left);
			if (current->right != NULL)
				ll_push_back(queue, current->right);
		}
		height++;
	}

	ll_destroy(queue);
	return height;
}

/* Prints the Red Black Tree out to the console window. */
void rbt_print(const struct RedBlackTree* const tree)
{
	if (rbt_isEmpty(tree))
		return;

	/* We will use this to loop through the tree, row by row. */
	struct LinkedList* const queue = LinkedList_new(NULL, NULL);
	/* Add the root to get us started. */
	ll_push_back(queue, tree->root);
	printf("%s%zu\n", "Red Black Tree - Size: ", tree->size);

	for (int currentHeight = rbt_height(tree), row = 1; row <= currentHeight; row++)
	{
		/* Initial line spacing. */
		printf("%.*s", -2 + (int)pow(2, (double)currentHeight - (double)row + 1), SPACING);

		/* Enter the row of the tree. */
		for (unsigned int nodes = queue->size; nodes > 0; nodes--)
		{
			const Node* const current = ll_front(queue);
			ll_pop_front(queue);
			const KeySet* const set = current->set;

			/* Change the color and print this Node's data. */
			WORD prevColor = 0;

			/* NULL key means this Node was a fake that we created. */
			if (set != NULL)
			{
				prevColor = ds_changeColor(current->color == RED ? COLOR_RED : COLOR_BLACK);
				printf(" %c ", (char)CHAR_NULL);
			}
			else
				printf("%.3s", tree->toString(set));

			if (prevColor != 0)
				ds_changeColor(prevColor);
			/* If another Node comes after this one, add spaces for it. */
			if (nodes > 1)
				printf("%.*s", -1 + (int)pow(2, (double)currentHeight - (double)row + 2), SPACING);

			/* Setup the children of this Node so we know what's underneath us.
			If we are on the bottom floor of the tree, there's not another row to be added. */
			if (row != currentHeight)
			{
				/* NULL children are going to mess up the printing very badly here.
				Create 'pretend' fake Nodes which will serve as placeholders
				for what COULD have existed in this slot. That way, the tree in
				printed form will still look normal even though there gaps in it (NULL). */
				ll_push_back(queue, current->left == NULL ? rbt_Node_new(NULL, NULL, RED, LEFT) : current->left);
				ll_push_back(queue, current->right == NULL ? rbt_Node_new(NULL, NULL, RED, LEFT) : current->right);
			}
		}

		/* Add the arrows / arms which direct to the children of this row. */
		if (row != currentHeight)
		{
			/* Initial line spacing. */
			printf("\n%.*s", -1 + (int)pow(2, (double)currentHeight - (double)row), SPACING);
			for (size_t i = 0, sets = queue->size / 2; i < sets; i++)
			{
				/* Print out the  /     \  that go to the row below. */
				printf("%c%.*s%c", CHAR_POINTER_L,
					(int)pow(2, (double)currentHeight - (double)row + 1) - 3, SPACING, CHAR_POINTER_R);
				/* Add spacing for the next set. */
				if (i + 1 != sets)
					printf("%.*s", (int)pow(2, (double)currentHeight - (double)row + 1) + 1, SPACING);
			}
		}
		printf("\n");
	}

	ll_destroy(queue);
}

/* Places a Key/Value pair into the tree. */
void rbt_put(struct RedBlackTree* const tree, const void* const key, const void* const value)
{
	if (key == NULL)
	{
		ds_error(RBT_ERR_NULL_KEY);
		return;
	}

	if (!rbt_isEmpty(tree))
	{
		const Node* const parent = rbt_search(tree, tree->root, key);
		const int cmp = tree->compare(key, parent->set->key);

		/* Duplicate key entered */
		if (cmp == 0)
		{
			parent->set->value = value;
			return;
		}

		const Node* const child = rbt_Node_new(rbt_KeySet_new(key, value), parent, BLACK, cmp < 0);
	}
	else
		tree->root = rbt_Node_new(rbt_KeySet_new(key, value), NULL, BLACK, LEFT);

	tree->size++;
}

/* De-constructor function. */
void rbt_destroy(const struct RedBlackTree* const tree)
{
	if (!rbt_isEmpty(tree))
	{
		const struct LinkedList* const queue = LinkedList_new(NULL, NULL);
		ll_push_back(queue, tree->root);

		while (!ll_empty(queue))
			/* Go through all the Nodes on this row. */
			for (unsigned int nodes = queue->size; nodes > 0; nodes--)
			{
				/* Prepare the next row. */
				const Node* const current = ll_front(queue);
				ll_pop_front(queue);
				if (current->left != NULL)
					ll_push_back(queue, current->left);
				if (current->right != NULL)
					ll_push_back(queue, current->right);
				rbt_Node_destroy(current, true);
			}

		ll_destroy(queue);
	}

	ds_free(tree, sizeof(Tree));
}

/* Constructor function. */
Node* rbt_Node_new(const KeySet* const set, const Node* const parent, const bool color, const bool rl_child)
{
	Node* const child = ds_calloc(1, sizeof(Node));
	child->color = color;
	child->set = set;
	assignChild(parent, child, rl_child);
	return child;
}

/* Constructor function. */
KeySet* rbt_KeySet_new(const void* const key, const void* value)
{
	KeySet* const set = ds_malloc(sizeof(KeySet));
	set->key = key;
	set->value = value;
	return set;
}

/* De-constructor function.
If deep, destroy all members as well. */
void rbt_Node_destroy(const Node* const val, const bool deep)
{
	if (deep)
		rbt_KeySet_destroy(val->set);
	ds_free(val, sizeof(Node));
}

/* De-constructor function. */
void rbt_KeySet_destroy(const KeySet* const set)
{
	ds_free(set, sizeof(KeySet));
}

/* Recursively search through the Tree in search for a Node holding a specific Key.
If `exact` is true, either get the exact Node or return NULL. If false, get the Node
or return the parent of where the Node would have been. */
Node* rbt_search(const Tree* const tree, const Node* const var, const void* const key)
{
	if (var == NULL)
		return NULL;
	if (isLeaf(var))
		return var;
	const int cmp = tree->compare(key, var->set->key);
	if (cmp < 0 && var->left != NULL)
		return rbt_search(tree, var->left, key);
	else if (cmp > 0 && var->right != NULL)
		return rbt_search(tree, var->right, key);
	else
		return var;
}

/* Assign a child to a specific parent.
Inform the child of who its parent is and if it is a left or right child.
Inform the parent of if this is its left or right child. */
void assignChild(Node* const parent, Node* const child, const bool leftRight)
{
	if (parent == NULL)
		return;

	/* Assign the child to the parent. */
	*(leftRight == LEFT ? &parent->left : &parent->right) = child;

	if (child != NULL)
	{
		/* Inform the previous parent that he no longer owns this child. */
		if (child->parent != NULL)
			*(child->rl_child == LEFT ? &child->parent->left : &child->parent->right) = NULL;
		child->parent = parent;
		child->rl_child = leftRight;
	}
}

/* Returns the uncle Node of the parameter Node if it exists, otherwise NULL. */
Node* getUncle(const Node* const child)
{
	Node *grandparent = getGrandparent(child);
	if (grandparent == NULL)
		return NULL;
	return (child->parent->rl_child == RIGHT) ? grandparent->left : grandparent->right;
}

/* Returns the grandparent Node of the parameter Node if it exists, otherwise NULL. */
Node* getGrandparent(const Node* const child)
{
	return (child != NULL && child->parent != NULL) ? child->parent->parent : NULL;
}

/* Check if a given Node is a leaf. */
bool isLeaf(const Node* const subject)
{
	return subject->left == NULL && subject->right == NULL;
}

/* Returns whether or not the parameter Node is the root of the tree. */
bool isRoot(const Node* const subject)
{
	return subject->parent == NULL;
}

/*
Case 1: Uncle is red. Re-color key Nodes.
Re-color the parent and uncle to black.
If the grandparent is not the root, re-color him to red.
Recursively call cases on the grandparent.

static void insertionCase1(Tree *tree, Node *child)
{
	Node *parent = child->parent, *uncle, *grandparent;

	/* Violation only occurs when child and parent are both red. 
	if (child->color == RED && parent->color == RED)
		if ((uncle = getUncle(child)) != NULL && uncle->color == RED)
		{
			grandparent = getGrandparent(child);
			uncle->color = BLACK;
			parent->color = BLACK;
			if (!isRoot(grandparent))
				grandparent->color = RED;
			/* Re-check this case further up the tree. 
			insertionCase1(tree, grandparent);
		}
		else
			insertionCase2(tree, child);
}


Case 2: Uncle is black. Child is either a left child of a
right parent or a right child of a left parent.
Child becomes the parent of parent and also inherits
the parent's old other child.
Proceed to case 3 with the parent (who is now a child).

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


Case 3: Uncle is black. Child is either a left child of a
left parent or a right child of a right parent.
Right or left rotation is performed. 

static void insertionCase3(Tree *tree, Node *child)
{
	Node *parent = child->parent, *uncle = getUncle(child),
		*grandparent;

	if ((uncle == NULL || uncle->color == BLACK) && child->rl_child == parent->rl_child)
	{
		grandparent = getGrandparent(child);
		bool direction = parent->rl_child, rootChanged;
		/* If there is more of the tree above us, connect it to parent. 
		if ((rootChanged = isRoot(grandparent)) == false)
			assignChild(grandparent->parent, parent, grandparent->rl_child);
		Node *temp = (direction == LEFT) ? parent->right : parent->left;
		assignChild(parent, grandparent, !direction);
		assignChild(grandparent, temp, direction);
		grandparent->color = RED;
		parent->color = BLACK;*/
		/* It's possible we messed up the root, fix it. */
		/*if (rootChanged)
		{
			tree->root = parent;
			tree->root->parent = NULL;
		}
	}
}*/