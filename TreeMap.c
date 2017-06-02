
#include "TreeMap.h"

#define RED true
#define BLACK false

#define SPACING "                                                                                "
#define COLOR_RED 12
#define COLOR_BLACK 240
#define CHAR_NULL 254
#define CHAR_POINTER_L '/'
#define CHAR_POINTER_R '\\'

/*
 * Red Black Tree must maintain the four basic properties.
 * 1) Each Node is either RED or BLACK.
 * 2) The root must always be BLACK.
 * 3) RED Nodes can only have BLACK children.
 * 4) Every path of the tree must have the same number of BLACK Nodes.
 */

 /* TreeMap structure. */
struct TreeMap
{
	struct map_Node *root;
	size_t size;

	/* Keep track of any iterators that are made. */
	LinkedList *iterators;

	/* Function pointers. */
	int(*compare)(const void*, const void*);
	char*(*toString)(const struct map_Entry*);
};

typedef struct map_Entry map_Entry;
typedef enum certificate certificate;

/* Node structure. */
typedef struct map_Node
{
	struct map_Node *parent, *left, *right;
	certificate status;
	map_Entry *data;
	bool color;
} map_Node;

/* Structure to help navigate through tree. */
struct map_Iterator
{
	const TreeMap *map;
	map_Node *current, *last;
};

/* Birth certificate for the Node. */
enum certificate
{
	LEFT,
	RIGHT,
	ORPHAN
};

/* Local functions. */
static map_Entry* map_Entry_new(const void* const key, const void* const value);
static map_Node* map_Node_new(map_Entry* const data, const bool color);
static map_Node* map_search(const TreeMap* const map, const map_Entry* const entry);
static map_Node* map_grandparent(const map_Node* const node);
static map_Node* map_uncle(const map_Node* const node);
static map_Node* map_child(const map_Node* const parent, const certificate which);
static map_Node* map_leftmost(map_Node* const node);
static map_Node* map_rightmost(map_Node* const node);
static void map_Entry_destroy(map_Entry* const entry);
static void map_Node_destroy(map_Node* const node);
static bool map_leaf(const map_Node* const node);
static bool map_root(const map_Node* const node);
static void map_custody(map_Node* const parent, map_Node* const child, const certificate status);
static void map_orphan(map_Node* const child);


static void map_iter_invalidate(const TreeMap* const map);
static int map_iter_compare(const void *i1, const void *i2);


static map_Node* map_Node_search(map_Node* const root, const map_Entry* const entry);

/*
 * Constructor function.
 * Θ(1)
 */
static map_Entry* map_Entry_new(const void* const key, const void* const value)
{
	map_Entry* const entry = ds_malloc(sizeof(map_Entry));
	entry->key = key;
	entry->value = value;
	return entry;
}

/*
 * Constructor function.
 * Θ(1)
 */
map_Node* map_Node_new(map_Entry* const data, const bool color)
{
	map_Node* const node = ds_calloc(1, sizeof(map_Node));
	node->color = color;
	node->data = data;
	node->status = ORPHAN;
	return node;
}

/*
 * Searches the map for a entry that matches the parameter.
 * The node returned is not guaranteed to be the node sought after.
 * Use map->compare afterwards to double check it is the correct node.
 * Θ(log(n))
 */
map_Node* map_search(const TreeMap* const map, const map_Entry* const entry)
{
	map_Node *i = map->root;
	while (true)
	{
		const int cmp = map->compare(i->data, entry);
		if (cmp < 0 && i->left != NULL)
			i = i->left;
		else if (cmp > 0 && i->right != NULL)
			i = i->right;
		else break;
	}

	return i;
}

/*
 * Returns the Node's grandparent.
 * NULL will be returned if the Node does not have a grandparent.
 * Θ(1)
 */
map_Node* map_grandparent(const map_Node* const node)
{
	/*if (node->status == ORPHAN)
	{
		ds_error(DS_MSG_NULL_PTR);
		return NULL;
	}*/

	return node->parent->parent;
}

/*
 * Returns the Node's uncle.
 * NULL will be returned if the Node does not have an uncle.
 * Θ(1)
 */
map_Node* map_uncle(const map_Node* const node)
{
	map_Node* grandparent = map_grandparent(node);
	if (node->parent->status == LEFT)
		return grandparent->right;
	return grandparent->left;

	/*const map_Node* const grandparent = map_Node_grandparent(node);
	if (grandparent == NULL)
	{
		ds_error(DS_MSG_NULL_PTR);
		return NULL;
	}

	/* Uncle is the grandparents other child.
	return map_Node_child(grandparent, !node->parent->status);*/
}

/*
 * Returns the parent's left or right child.
 * Θ(1)
 */
map_Node* map_child(const map_Node* const parent, const certificate which)
{
	return which == LEFT ? parent->left : parent->right;
}

/*
 * Returns the left-most Node from this Node.
 * Θ(log(n))
 */
map_Node* map_leftmost(map_Node* const node)
{
	if (node->left != NULL)
		return map_leftmost(node);
	return node;
}

/*
 * Returns the right-most Node from this Node.
 * Θ(log(n))
 */
map_Node* map_rightmost(map_Node* const node)
{
	if (node->right != NULL)
		return map_rightmost(node);
	return node;
}

/*
 * De-constructor function.
 * Θ(1)
 */
void map_Entry_destroy(map_Entry* const entry)
{
	ds_free(entry, sizeof(map_Entry));
}

/*
 * De-constructor function.
 * Θ(1)
 */
void map_Node_destroy(map_Node* const node)
{
	map_Entry_destroy(node->data);
	ds_free(node, sizeof(map_Node*));
}

/*
 * Returns true if this Node is a leaf.
 * Θ(1)
 */
bool map_leaf(const map_Node* const node)
{
	return node->left == NULL && node->right == NULL;
}

/*
 * Returns true if this Node is the root.
 * Θ(1)
 */
bool map_root(const map_Node* const node)
{
	return node->parent == NULL;
}

/*
 * Assigns custody of a child node to a parent.
 * The child will be left or right depending on the certificate.
 * If a previous child of that certificate exists, it is overwritten.
 * Θ(1)
 */
void map_custody(map_Node* const parent, map_Node* const child, const certificate status)
{
	/* Orphan the two children that have their parents modified. */
	map_orphan(child);
	map_Node *orphaned = map_child(parent, status);
	map_orphan(orphaned);

	/* Assign this child to the parent. */
	child->parent = parent;
	child->status = status;
	*(&orphaned) = child;
}

/*
 * Orphans the child, removing his parent.
 * Informs the former parent that he no longer has that child.
 * Θ(1)
 */
void map_orphan(map_Node* const child)
{
	/* Child is already orphan. */
	if (child->status == ORPHAN) return;
	/* Inform the parent that he does not have a child anymore. */
	*(child->status == LEFT ? 
		&child->parent->left : &child->parent->right) = NULL;
	child->parent = NULL;
	child->status = ORPHAN;
}

/*
* Invalidates all iterators currently iterating over the TreeMap.
* Iterators are not allowed to run if changes were made to the map.
* Θ(n)
*/
void map_iter_invalidate(const TreeMap* const map)
{
	while (!ll_empty(map->iterators))
	{
		/* Invalidate and pop all iterators from the list. */
		((map_Iterator*)ll_front(map->iterators))->map = NULL;
		ll_pop_front(map->iterators);
	}
}

/*
 * Returns true if two iterators are the same.
 * Θ(1)
 */
int map_iter_compare(const void *i1, const void *i2)
{
	return i1 == i2 ? 0 : 1;
}

///* Constructor function. */
//struct TreeMap* TreeMap_new(int(*compare)(const void* const, const void* const), char*(*toString)(const void* const))
//{
//	TreeMap* const tree = ds_calloc(1, sizeof(Tree));
//	tree->compare = compare;
//	tree->toString = toString;
//	return tree;
//}
//
///* Returns the value associated with the key inside the tree. NULL if one doesn't exist. */
//void* map_get(const struct TreeMap* const tree, const void* const key)
//{
//	const map_Node* const found = map_search(tree, tree->root, key);
//	return found != NULL ? found->set->value : NULL;
//}
//
///* Returns true if the tree contains the specified key. */
//bool map_contains(const struct TreeMap* const tree, const void* const key)
//{
//	const map_Node* const located = map_search(tree, tree->root, key);
//	return tree->compare(located->set->key, key) == 0;
//}
//
///* Returns tree if the Tree is empty. */
//bool map_isEmpty(const struct TreeMap* const tree)
//{
//	return tree->size == 0;
//}
//
///* Returns the vertical height of the tree. */
//unsigned int map_height(const struct TreeMap* const tree)
//{
//	if (map_isEmpty(tree))
//		return 0;
//
//	const struct LinkedList* const queue = LinkedList_new(NULL, NULL);
//	ll_push_back(queue, tree->root);
//
//	unsigned int height = 0;
//	while (!ll_empty(queue))
//	{
//		/* Loop over every Node on this row. */
//		for (unsigned int nodes = queue->size; nodes > 0; nodes--)
//		{
//			const map_Node* const current = ll_front(queue);
//			ll_pop_front(queue);
//			if (current->left != NULL)
//				ll_push_back(queue, current->left);
//			if (current->right != NULL)
//				ll_push_back(queue, current->right);
//		}
//		height++;
//	}
//
//	ll_destroy(queue);
//	return height;
//}
//
///* Prints the Red Black Tree out to the console window. */
//void map_print(const struct TreeMap* const tree)
//{
//	if (map_isEmpty(tree))
//		return;
//
//	/* We will use this to loop through the tree, row by row. */
//	struct LinkedList* const queue = LinkedList_new(NULL, NULL);
//	/* Add the root to get us started. */
//	ll_push_back(queue, tree->root);
//	printf("%s%zu\n", "Red Black Tree - Size: ", tree->size);
//
//	for (int currentHeight = map_height(tree), row = 1; row <= currentHeight; row++)
//	{
//		/* Initial line spacing. */
//		printf("%.*s", -2 + (int)pow(2, (double)currentHeight - (double)row + 1), SPACING);
//
//		/* Enter the row of the tree. */
//		for (unsigned int nodes = queue->size; nodes > 0; nodes--)
//		{
//			const map_Node* const current = ll_front(queue);
//			ll_pop_front(queue);
//			const KeySet* const set = current->set;
//
//			/* Change the color and print this Node's data. */
//			WORD prevColor = 0;
//
//			/* NULL key means this Node was a fake that we created. */
//			if (set != NULL)
//			{
//				prevColor = ds_changeColor(current->color == RED ? COLOR_RED : COLOR_BLACK);
//				printf(" %c ", (char)CHAR_NULL);
//			}
//			else
//				printf("%.3s", tree->toString(set));
//
//			if (prevColor != 0)
//				ds_changeColor(prevColor);
//			/* If another Node comes after this one, add spaces for it. */
//			if (nodes > 1)
//				printf("%.*s", -1 + (int)pow(2, (double)currentHeight - (double)row + 2), SPACING);
//
//			/* Setup the children of this Node so we know what's underneath us.
//			If we are on the bottom floor of the tree, there's not another row to be added. */
//			if (row != currentHeight)
//			{
//				/* NULL children are going to mess up the printing very badly here.
//				Create 'pretend' fake Nodes which will serve as placeholders
//				for what COULD have existed in this slot. That way, the tree in
//				printed form will still look normal even though there gaps in it (NULL). */
//				ll_push_back(queue, current->left == NULL ? map_Node_new(NULL, NULL, RED, LEFT) : current->left);
//				ll_push_back(queue, current->right == NULL ? map_Node_new(NULL, NULL, RED, LEFT) : current->right);
//			}
//		}
//
//		/* Add the arrows / arms which direct to the children of this row. */
//		if (row != currentHeight)
//		{
//			/* Initial line spacing. */
//			printf("\n%.*s", -1 + (int)pow(2, (double)currentHeight - (double)row), SPACING);
//			for (size_t i = 0, sets = queue->size / 2; i < sets; i++)
//			{
//				/* Print out the  /     \  that go to the row below. */
//				printf("%c%.*s%c", CHAR_POINTER_L,
//					(int)pow(2, (double)currentHeight - (double)row + 1) - 3, SPACING, CHAR_POINTER_R);
//				/* Add spacing for the next set. */
//				if (i + 1 != sets)
//					printf("%.*s", (int)pow(2, (double)currentHeight - (double)row + 1) + 1, SPACING);
//			}
//		}
//		printf("\n");
//	}
//
//	ll_destroy(queue);
//}
//
///* Places a Key/Value pair into the tree. */
//void map_put(struct TreeMap* const tree, const void* const key, const void* const value)
//{
//	if (key == NULL)
//	{
//		ds_error(RBT_ERR_NULL_KEY);
//		return;
//	}
//
//	if (!map_isEmpty(tree))
//	{
//		const map_Node* const parent = map_search(tree, tree->root, key);
//		const int cmp = tree->compare(key, parent->set->key);
//
//		/* Duplicate key entered */
//		if (cmp == 0)
//		{
//			parent->set->value = value;
//			return;
//		}
//
//		const map_Node* const child = map_Node_new(map_KeySet_new(key, value), parent, BLACK, cmp < 0);
//	}
//	else
//		tree->root = map_Node_new(map_KeySet_new(key, value), NULL, BLACK, LEFT);
//
//	tree->size++;
//}
//
///* De-constructor function. */
//void map_destroy(const struct TreeMap* const tree)
//{
//	if (!map_isEmpty(tree))
//	{
//		const struct LinkedList* const queue = LinkedList_new(NULL, NULL);
//		ll_push_back(queue, tree->root);
//
//		while (!ll_empty(queue))
//			/* Go through all the Nodes on this row. */
//			for (unsigned int nodes = queue->size; nodes > 0; nodes--)
//			{
//				/* Prepare the next row. */
//				const map_Node* const current = ll_front(queue);
//				ll_pop_front(queue);
//				if (current->left != NULL)
//					ll_push_back(queue, current->left);
//				if (current->right != NULL)
//					ll_push_back(queue, current->right);
//				map_Node_destroy(current, true);
//			}
//
//		ll_destroy(queue);
//	}
//
//	ds_free(tree, sizeof(Tree));
//}

/*
 * TODO: Document
 */
TreeMap* TreeMap_new(int(*compare)(const void*, const void*), char*(*toString)(const struct map_Entry*))
{
	TreeMap* const map = ds_calloc(1, sizeof(TreeMap));
	map->iterators = LinkedList_new(&map_iter_compare, NULL);

	/* Function pointers. */
	map->compare = compare;
	map->toString = toString;
	return map;
}

/*
 * Returns the amount of entries inside the map.
 * Θ(1)
 */
size_t map_size(const TreeMap* const map)
{
	return map->size;
}

void map_put(TreeMap* const map, void* const key, void* const value)
{
}

/*
 * Constructor function.
 * Front indicates whether the iterator will
 * start at the front or end of the tree.
 * Θ(log(n))
 */
map_Iterator* map_iter(const TreeMap* const map, const bool front)
{
	if (map == NULL || map->root == NULL)
	{
		ds_error(DS_MSG_NULL_PTR);
		return NULL;
	}

	map_Iterator* const iter = ds_calloc(1, sizeof(map_Iterator));
	/* Add a reference to this iterator to the map's iterator list. */
	iter->map = map;
	ll_push_back(map->iterators, iter);
	iter->last = front ? map_leftmost(map->root) : map_rightmost(map->root);

	return iter;
}

/*
 * Returns the iterator's current element and advances it forward.
 * Θ(1)
 */
struct map_Entry* map_iter_next(map_Iterator* const iter)
{
	/* Error checking. */
	bool error = true;
	if (iter == NULL)
		ds_error(DS_MSG_NULL_PTR);
	else if (iter->map == NULL)
		ds_error(DS_MSG_INVALID);
	else if (!map_iter_has_next(iter))
		ds_error(DS_MSG_OUT_OF_BOUNDS);
	else error = false;
	if (error) return NULL;

	map_Entry *val;
	/* Last needs to move but this reference is where it was. */
	const map_Node* const lastRef = iter->last;

	/* Prepare the return value and update last's location. */
	if (iter->current == NULL)
	{
		val = iter->last->data;
		iter->current = iter->last;
	}
	else
	{
		val = iter->current->data;
		iter->last = iter->current;
	}

	/* An existing right elements always means go right. */
	if (iter->current->right != NULL)
		iter->current = iter->current->right;
	/* No parent means we've hit the end of the iteration. */
	else if (iter->current->parent == NULL)
		iter->current = NULL;
	/* Otherwise, head upwards. */
	else
		iter->current = iter->current->parent;

	return val;
}

/*
 * Returns true if the iterator has a next element.
 * Θ(1)
 */
bool map_iter_has_next(const map_Iterator* const iter)
{
	return iter->current != NULL || iter->last->right != NULL;
}

/*
 * Returns true if the iterator has a previous element.
 * Θ(1)
 */
bool map_iter_has_prev(const map_Iterator* const iter)
{
	return iter->current != NULL || iter->last->left != NULL;
}

/*
 * De-constructor function.
 * Θ(1)
 */
void map_iter_destroy(map_Iterator* const iter)
{
	/* Before this iterator goes, make sure the map knows we're gone. */
	if (iter->map != NULL)
		ll_remove(iter->map->iterators, iter);
	ds_free(iter, sizeof(map_Iterator));
}

///* Constructor function. */
//Node* map_Node_new(const KeySet* const set, const map_Node* const parent, const bool color, const bool rl_child)
//{
//	Node* const child = ds_calloc(1, sizeof(Node));
//	child->color = color;
//	child->set = set;
//	assignChild(parent, child, rl_child);
//	return child;
//}
//
///* Constructor function. */
//KeySet* map_KeySet_new(const void* const key, const void* value)
//{
//	KeySet* const set = ds_malloc(sizeof(KeySet));
//	set->key = key;
//	set->value = value;
//	return set;
//}
//
///* De-constructor function.
//If deep, destroy all members as well. */
//void map_Node_destroy(const map_Node* const val, const bool deep)
//{
//	if (deep)
//		map_KeySet_destroy(val->set);
//	ds_free(val, sizeof(Node));
//}
//
///* De-constructor function. */
//void map_KeySet_destroy(const KeySet* const set)
//{
//	ds_free(set, sizeof(KeySet));
//}
//
///* Recursively search through the Tree in search for a Node holding a specific Key.
//If `exact` is true, either get the exact Node or return NULL. If false, get the Node
//or return the parent of where the Node would have been. */
//Node* map_search(const TreeMap* const tree, const map_Node* const var, const void* const key)
//{
//	if (var == NULL)
//		return NULL;
//	if (isLeaf(var))
//		return var;
//	const int cmp = tree->compare(key, var->set->key);
//	if (cmp < 0 && var->left != NULL)
//		return map_search(tree, var->left, key);
//	else if (cmp > 0 && var->right != NULL)
//		return map_search(tree, var->right, key);
//	else
//		return var;
//}
//


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