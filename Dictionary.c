
#include "Dictionary.h"

#define DICT_NODE_CHILDREN 4
#define DICT_NODE_ELEMENTS DICT_NODE_CHILDREN - 1
#define LEFT_CHILD(i) i
#define RIGHT_CHILD(i) ((i + 1) / 2)

typedef struct KeySet KeySet;

/* Node structure. */
typedef struct dict_Node
{
	const Vector *key_sets;
	struct dict_Node* children[DICT_NODE_CHILDREN];
} dict_Node;

/* Dictionary structure. */
struct Dictionary
{
	dict_Node *root;
	size_t size;

	/* Function pointers. */
	int(*compare)(const void*, const void*);
	char*(*toString)(const KeySet*);
};

/* Local functions. */
static dict_Node* dict_Node_new();
dict_Node *dict_search(const Dictionary* const dict, const void* const key,
					   dict_Node* const child, dict_Node* const parent);
static bool dict_Node_leaf(const dict_Node* const subject);
static bool dict_Node_full(const dict_Node* const subject);
static void dict_Node_destroy(dict_Node* const deceased);

/*
 * Constructor function.
 */
Dictionary* Dictionary_new(int(*compare)(const void*, const void*), char*(*toString)(const KeySet*))
{
	Dictionary* const dict = ds_calloc(1, sizeof(Dictionary));
	dict->compare = compare;
	dict->toString = toString;
    /* Compare is required for functionality of the Dictionary. */
	if (compare == NULL)
	{
		ds_error(DS_MSG_NULL_PTR);
		return NULL;
	}

	return dict;
}

void dict_put(const struct Dictionary* const dict, const void* const key, const void* value)
{

}

/*
 * Constructor function.
 */
static dict_Node* dict_Node_new()
{
	dict_Node* const node = ds_calloc(1, sizeof(struct dict_Node));
    node->key_sets = Vector_new(NULL, NULL);
    return node;
}

/*
 *
 */
dict_Node* dict_search(const Dictionary* const dict, const void* const key,
					   dict_Node* const child, dict_Node* const parent)
{
	// TODO: Check for splitting.
	const Vector* const vect = child->key_sets;
	for (unsigned int i = 0, s = vect_size(vect); i < s; i++)
	{
		const int cmp = dict->compare(key, vect_at(vect, i));
		/* Key located. Return the parent. */
		if (cmp == 0) return parent;
		if (cmp > 0)
		{
			/* If there's another element, keep going rightwards. */
			if (i + 1 < s)
			{
				dict_Node* right_child = child->children[RIGHT_CHILD(i)];
				return (right_child != NULL) ? dict_search(dict, key, right_child, child) : parent;
			}
		}
		else
		{
			dict_Node* const left_child = child->children[LEFT_CHILD(i)];
			return (left_child != NULL) ? dict_search(dict, key, left_child, child) : parent;
		}
	}
}

/*
 * Returns true if the Node doesn't have any children.
 * Θ(1)
 */
bool dict_Node_leaf(const dict_Node *const subject)
{
    return subject->children[0] == NULL;
}

/*
 * Returns true if the Node is full with children.
 * Θ(1)
 */
bool dict_Node_full(const dict_Node* const subject)
{
	return vect_size(subject->key_sets) == DICT_NODE_ELEMENTS;
}

/*
 * De-constructor function.
 */
static void dict_Node_destroy(dict_Node* const deceased)
{
	vect_destroy(deceased->key_sets);
	ds_free(deceased, sizeof(dict_Node));
}
