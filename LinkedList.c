
#include "LinkedList.h"
#include "C-Random/Random.h"

#define START true
#define END false

/* Node structure. */
typedef struct ll_Node
{
	void *data;
	struct ll_Node *next, *prev;
} ll_Node;

/* LinkedList structure. */
struct LinkedList
{
	ll_Node *root, *tail;
	size_t size;

	// TODO: Invalidation listener.

	/* Function pointers. */
	int(*compare)(const void*, const void*);
	char*(*toString)(const void*);
};

/* Structure to help navigate through list. */
struct ll_Iterator
{
	unsigned int index;
	ll_Node *current, *last;
	LinkedList* list;
};

/* Local functions. */
static ll_Node* Node_new(void* const data);
static ll_Node* ll_search(const LinkedList* const list, const size_t index);
static void ll_link(ll_Node* const left, ll_Node* const right);
static void ll_separate(LinkedList* const list, LinkedList* const l1, LinkedList* const l2);
static void* ll_pull_front(LinkedList* const list);
static void* ll_pull_back(LinkedList* const list);

/* Constructor function. */
LinkedList* LinkedList_new(int(*compare)(const void*, const void*),
	char*(*toString)(const void*))
{
	LinkedList *list = ds_calloc(1, sizeof(LinkedList));
	/* Pointer functions. */
	list->compare = compare;
	list->toString = toString;
	return list;
}

/* Constructor function. */
ll_Node* Node_new(void* const data)
{
	ll_Node *node = ds_calloc(1, sizeof(ll_Node));
	node->data = data;
	return node;
}

/*
 * Returns the element at the front of the list.
 * Θ(1)
 */
void* ll_front(const LinkedList* const list)
{
	if (ll_empty(list))
		ds_error(DS_MSG_EMPTY);
	return list->root->data;
}

/*
 * Returns the element at the back of the list.
 * Θ(1)
 */
void* ll_back(const LinkedList* const list)
{
	if (ll_empty(list))
		ds_error(DS_MSG_EMPTY);
	return list->tail->data;
}

/*
 * Returns the element at the specified index in the list.
 * Θ(n)
 */
void* ll_at(const LinkedList* const list, const size_t index)
{
	if (ll_empty(list))
	{
		ds_error(DS_MSG_EMPTY);
		return NULL;
	}
	if (index == 0)
		return ll_front(list);
	if (index == list->size - 1)
		return ll_back(list);
	return ll_search(list, index)->data;
}

/*
 * Returns true if the list contains the specified element.
 * Θ(n)
 */
bool ll_contains(const LinkedList* const list, const void* const data)
{
	const ll_Iterator *iter = ll_iter(list, START);
	while (ll_iter_has_next(iter))
		if (list->compare(ll_iter_next(iter), data) == 0)
		{
			ll_iter_destroy(iter);
			return true;
		}

	ll_iter_destroy(iter);
	return false;
}

/*
 * Returns the amount of elements inside the list.
 * Θ(1)
 */
size_t ll_size(const LinkedList* const list)
{
	return list->size;
}

/*
 * Returns true if the list is empty.
 * Θ(1)
 */
bool ll_empty(const LinkedList* const list)
{
	return ll_size(list) == 0;
}

/*
 * Returns a shallow copy of the list.
 * Θ(n)
 */
LinkedList* ll_clone(const LinkedList* const list)
{
	LinkedList* const copy = LinkedList_new(list->compare, list->toString);
	ll_Iterator* const iter = ll_iter(list, START);

	while (ll_iter_has_next(iter))
		ll_push_back(copy, ll_iter_next(iter));
	ll_iter_destroy(iter);

	return copy;
}

/*
 * Prints out the list to the console window.
 * Θ(n)
 */
void ll_print(const LinkedList* const list)
{
	printf("{ ");

	ll_Iterator* const iter = ll_iter(list, true);
	while (ll_iter_has_next(iter))
	{
		printf("%s", list->toString(ll_iter_next(iter)));
		if (ll_iter_has_next(iter))
			printf(", ");
	}

	ll_iter_destroy(iter);
	printf(" }\n");
}

/*
 * Returns an array of all elements inside the list.
 * The array is allocated dynamically and must be freed after use.
 * Θ(n)
 */
void** ll_array(const LinkedList* const list)
{
	if (ll_empty(list))
	{
		ds_error(DS_MSG_EMPTY);
		return NULL;
	}

	void** const arr = malloc(ll_size(list) * sizeof(void*));
	
	unsigned int index = 0;
	ll_Iterator* const iter = ll_iter(list, START);
	while (ll_iter_has_next(iter))
		arr[index++] = ll_iter_next(iter);
	ll_iter_destroy(iter);

	return arr;
}

/*
 * Inserts an element at the front of the list.
 * Θ(1)
 */
void ll_push_front(LinkedList* const list, const void* const data)
{
	ll_Node* const insert = Node_new(data);

	if (!ll_empty(list))
		ll_link(insert, list->root);
	else
		list->tail = insert;

	list->root = insert;
	list->size++;
}

/*
 * Appends an element at the end of the list.
 * Θ(1)
 */
void ll_push_back(LinkedList* const list, const void* const data)
{
	ll_Node* const insert = Node_new(data);

	if (!ll_empty(list))
		ll_link(list->tail, insert);
	else
		list->root = insert;

	list->tail = insert;
	list->size++;
}

/*
 * Overwrites an element in the list at a given index.
 * Θ(n)
 */
void ll_assign(const LinkedList* const list, const size_t index, const void* const data)
{
	ll_Node* const located = ll_search(list, index);
	located->data = data;
}

/*
 * Inserts an element at a specific position in the list.
 * Ω(1), O(n)
 */
void ll_insert(LinkedList* const list, const unsigned int index, const void* const data)
{
	if (index > ll_size(list))
		ds_error(DS_MSG_OUT_OF_BOUNDS);
	else if (ll_empty(list) || index == 0)
		ll_push_front(list, data);
	else if (index == ll_size(list))
		ll_push_back(list, data);
	else
	{
		ll_Node* const i = ll_search(list, index),
			*const inserted = Node_new(data);
		ll_link(i->prev, inserted);
		ll_link(inserted, i);
		list->size++;
	}
}

/*
 * Removes an element at the front of the list.
 * Θ(1)
 */
void ll_pop_front(LinkedList* const list)
{
	if (ll_empty(list))
	{
		ds_error(DS_MSG_EMPTY);
		return;
	}

	const ll_Node* const root = list->root;
	if (ll_size(list) == 1)
	{
		list->root = NULL;
		list->tail = NULL;
	}
	else
	{
		ll_Node* const neighbor = root->next;
		neighbor->prev = NULL;
		list->root = neighbor;
	}

	ds_free(root, sizeof(ll_Node));
	list->size--;
}

/*
 * Removes an element at the end of the list.
 * Θ(1)
 */
void ll_pop_back(LinkedList* const list)
{
	if (ll_empty(list))
	{
		ds_error(DS_MSG_EMPTY);
		return;
	}

	const ll_Node* const tail = list->tail;
	if (ll_size(list) == 1)
	{
		list->root = NULL;
		list->tail = NULL;
	}
	else
	{
		ll_Node* const neighbor = tail->prev;
		neighbor->next = NULL;
		list->tail = neighbor;
	}

	ds_free(tail, sizeof(ll_Node));
	list->size--;
}

/*
 * Removes the first occurrence of an element from the list.
 * Returns true if the removal was successful, false if it could not be found.
 * Θ(n)
 */
bool ll_remove(LinkedList* const list, const void* const data)
{
	if (ll_empty(list))
	{
		ds_error(DS_MSG_EMPTY);
		return false;
	}

	ll_Iterator *iter = ll_iter(list, START);
	while (ll_iter_has_next(iter))
	{
		ll_Node *temp = iter->last;
		if (list->compare(temp->data, data) == 0)
		{
			if (ll_iter_index(iter) == 0)
				ll_pop_front(list);
			else if (ll_iter_index(iter) == ll_size(list) - 1)
				ll_pop_back(list);
			else
			{
				ll_link(temp->prev, temp->next);
				ds_free(temp, sizeof(ll_Node));
				list->size--;
			}

			ll_iter_destroy(iter);
			return true;
		}

		ll_iter_next(iter);
	}

	ll_iter_destroy(iter);
	return false;
}

/*
 * Removes the element at the specified position in the list.
 * Ω(1), O(n)
 */
void ll_erase(LinkedList* const list, const size_t index)
{
	if (index >= ll_size(list))
		ds_error(DS_MSG_OUT_OF_BOUNDS);
	else if (ll_empty(list))
		ds_error(DS_MSG_EMPTY);
	else if (index == 0)
		ll_pop_front(list);
	else if (index == ll_size(list) - 1)
		ll_pop_back(list);
	else
	{
		const ll_Node* const located = ll_search(list, index);
		ll_link(located->prev, located->next);
		ds_free(located, sizeof(ll_Node));
		list->size--;
	}
}

/*
 * Removes all elements inside the list.
 * Θ(n)
 */
void ll_clear(LinkedList* const list)
{
	ll_Iterator* const iter = ll_iter(list, START);
	
	while (ll_iter_has_next(iter))
	{
		ll_iter_next(iter);
		bool leave = !ll_iter_has_next(iter);
		ds_free(iter->last, sizeof(ll_Node));
		if (leave) break;
	}
	ll_iter_destroy(iter);

	list->root = NULL;
	list->tail = NULL;
	list->size = 0;
}

/*
 * Sorts the elements inside the list in ascending order.
 * Implementation uses Mergesort algorithm.
 * Θ(n * log(n))
 */
void ll_sort(LinkedList* const list)
{
	if (ll_size(list) <= 1)
		return;

	/* Create two sub-lists of the main list. */
	LinkedList* const left = LinkedList_new(list->compare, list->toString),
		*const right = LinkedList_new(list->compare, list->toString);
	ll_separate(list, left, right);

	/* Recursively sort the sub-lists. */
	ll_sort(left);
	ll_sort(right);

	while (!ll_empty(left) && !ll_empty(right))
		/* Place the lowest element from each sub-list back into the main list. */
		ll_push_back(list, list->compare(ll_front(left), ll_front(right)) <= 0 ?
			ll_pull_front(left) : ll_pull_front(right));

	/* Dump any remaining elements back into the list. */
	LinkedList* const both[] = { left, right };
	for (int i = 0; i < 2; i++)
		while (!ll_empty(both[i]))
			ll_push_back(list, ll_pull_front(both[i]));

	ll_destroy(left);
	ll_destroy(right);
}

/*
 * Shuffles the elements in the list pseudo-randomly.
 * Shuffle algorithm is based on the Mergesort algorithm.
 * Random is based on the Random implementation (see Random.h).
 * TODO: Determine true O-complexity.
 * Ω(n * log(n)), O(n * log(n) + n)
 */
#define DUMMY_SIZE 1
void ll_shuffle(LinkedList* const list)
{
	if (ll_size(list) <= 1)
		return;

	/* Create two sub-lists of the main list. */
	LinkedList* const left = LinkedList_new(list->compare, list->toString),
		*const right = LinkedList_new(list->compare, list->toString);
	ll_separate(list, left, right);

	/* Recursively shuffle the sub-lists. */
	ll_shuffle(left);
	ll_shuffle(right);

	/*
	 * In order to ensure random distribution, both lists must be the same size.
	 * If size of the original list is not 2^n, then list size of 1 and 2 will occur.
	 * In that situation, we need to place a dummy inside the list to maintain
	 * random distribution. We must also delete the dummy ll_Node after merging.
	 */
	void* dummy = NULL;
	if (ll_size(left) > ll_size(right))
	{
		dummy = ds_malloc(DUMMY_SIZE);
		const unsigned int dummy_loc = rand_limit(ll_size(right) + 1);
		ll_insert(right, dummy_loc, dummy);
	}

	/* Merge the two lists back together randomly. */
	while (!ll_empty(left) && !ll_empty(right))
		/* Flip a coin and merge based on the result of the toss. */
		ll_push_back(list, rand_bool() ?
			ll_pull_front(left) : ll_pull_front(right));

	/* Dump any remaining elements back into the list. */
	LinkedList* const both[] = { left, right };
	for (int i = 0; i < 2; i++)
		while (!ll_empty(both[i]))
			ll_push_back(list, ll_pull_front(both[i]));
		
	ll_destroy(left);
	ll_destroy(right);

	if (dummy == NULL) return;
	/* Remove the dummy variable if we needed one. */
	ll_Iterator* const iter = ll_iter(list, START);
	while (ll_iter_next(iter) != dummy);
	ll_iter_remove(iter);
	ds_free(dummy, DUMMY_SIZE);
	ll_iter_destroy(iter);
}

/*
 * De-constructor function.
 * Θ(n)
 */
void ll_destroy(LinkedList* const list)
{
	ll_clear(list);
	ds_free(list, sizeof(LinkedList));
}

/*
 * Constructor function.
 * list provided is the data the iterator will iterate over.
 * Boolean provided indicates which side of the list the iterator begins at.
 */
ll_Iterator* ll_iter(const LinkedList* const list, const bool front)
{
	ll_Iterator* const iter = ds_calloc(1, sizeof(ll_Iterator));

	if (front)
		iter->last = list->root;
	else
	{
		iter->last = list->tail;
		iter->index = ll_size(list) - 1;
	}

	iter->list = list;
	return iter;
}

/*
 * Returns the iterator's current element and advances it forward.
 * Ω(1)
 */
void* ll_iter_next(ll_Iterator* const iter)
{
	if (!ll_iter_has_next(iter))
	{
		ds_error(DS_MSG_OUT_OF_BOUNDS);
		return NULL;
	}

	const void* data;

	/* If current is out of bounds, utilize our temporary pointer. */
	if (iter->current == NULL)
	{
		data = iter->last->data;
		iter->current = iter->last->next;
	}
	else
	{
		data = iter->current->data;
		iter->last = iter->current;
		iter->current = iter->current->next;
	}

	iter->index++;
	return data;
}

/*
 * Returns the iterator's current element and advances it backward.
 * Ω(1)
 */
void* ll_iter_prev(ll_Iterator* const iter)
{
	if (!ll_iter_has_prev(iter))
	{
		ds_error(DS_MSG_OUT_OF_BOUNDS);
		return NULL;
	}

	const void* data;

	/* If current is out of bounds, utilize our temporary pointer. */
	if (iter->current == NULL)
	{
		data = iter->last->data;
		iter->current = iter->last->prev;
	}
	else
	{
		data = iter->current->data;
		iter->last = iter->current;
		iter->current = iter->current->prev;
	}

	iter->index--;
	return data;
}

/*
 * Returns true if the iterator has a next element.
 * Ω(1)
 */
bool ll_iter_has_next(const ll_Iterator* const iter)
{
	/* Current is NULL if it goes out of bounds. Last is NULL if the list is empty. */
	return iter->current != NULL || (iter->last != NULL && ll_iter_index(iter) == 0);
}

/*
 * Returns true if the iterator has a previous element.
 * Ω(1)
 */
bool ll_iter_has_prev(const ll_Iterator* const iter)
{
	/* Current is NULL if it goes out of bounds. Last is NULL if the list is empty. */
	return iter->current != NULL || (iter->last != NULL && ll_iter_index(iter) > 0);
}

/*
 * Returns the index at which this iterator is over.
 * Ω(1)
 */
unsigned ll_iter_index(const ll_Iterator* const iter)
{
	return iter->index;
}

/*
 * De-constructor function.
 * Ω(1)
 */
void ll_iter_destroy(const ll_Iterator* const iter)
{
	ds_free(iter, sizeof(ll_Iterator));
}

/*
 * Inserts an element at the iterator's current position.
 * The new element will become the iterator's current element.
 * Ω(1)
 */
void ll_iter_insert(ll_Iterator* const iter, const void* const data)
{
	if (!ll_iter_has_prev(iter))
	{
		ll_push_front(iter->list, data);
		iter->current = iter->list->root;
	}
	else if (!ll_iter_has_next(iter))
	{
		ll_push_back(iter->list, data);
		iter->current = iter->list->tail;
	}
	else
	{
		ll_Node* const inserted = Node_new(data);

		/* Iterator was heading rightwards. */
		if (iter->current == iter->last->next)
		{
			ll_link(iter->last, inserted);
			ll_link(inserted, iter->current);
		}
		else
		{
			ll_link(iter->current, inserted);
			ll_link(inserted, iter->last);
		}
		iter->current = inserted;
		iter->list->size++;
	}

	iter->index++;
}

/*
 * Removes the last iterated element from the list.
 * The removed element will be the one last returned by
 * prev() or next(). If neither have been called, it will
 * remove the starting element (root or tail).
 * Ω(1)
 */
void ll_iter_remove(ll_Iterator* const iter)
{
	if (ll_empty(iter->list))
		ds_error(DS_MSG_EMPTY);
	/* Last element iterated was the root. */
	else if (iter->last->prev == NULL)
	{
		iter->last = iter->last->next;
		/* Make sure current remains up to date. */
		if (iter->last != NULL)
			iter->current = iter->last->next;
		/* If last becomes NULL, reset the index to zero. */
		else iter->index = 0;
		ll_pop_front(iter->list);
	}
	/* Last element iterated was the tail. */
	else if (iter->last->next == NULL)
	{
		iter->last = iter->last->prev;
		if (iter->current != NULL)
			iter->current = iter->current->prev;
		ll_pop_back(iter->list);
		/* Index must go back as we are removing from the tail. */
		iter->index--;
	}
	else
	{
		const ll_Node* const deceased = iter->last;

		/* Last is behind current. */
		if (iter->last->next == iter->current)
		{
			iter->last = iter->last->prev;
			ll_link(iter->last, iter->current);
			iter->index--;
		}
		/* Last is ahead of current. */
		else
		{
			iter->last = iter->last->next;
			ll_link(iter->current, iter->last);
		}

		ds_free(deceased, sizeof(ll_Node));
		iter->list->size--;
	}
}

/*
 * Locates a Node in the list based on the index value.
 * Seeks from the head or tail based on which is closer.
 * This means that worse case, will traverse n / 2 Nodes.
 * Ω(n)
 */
ll_Node* ll_search(const LinkedList* const list, const size_t index)
{
	if (index >= list->size)
	{
		ds_error(DS_MSG_OUT_OF_BOUNDS);
		return NULL;
	}

	void*(*navigate)(ll_Iterator* const);
	bool location;

	/* Determine if seeking from the root is faster. */
	if ((double)index / (list->size - 1) <= 0.5)
	{
		location = START;
		navigate = &ll_iter_next;
	}
	else
	{
		location = END;
		navigate = &ll_iter_prev;
	}

	ll_Iterator* const iter = ll_iter(list, location);
	while (ll_iter_index(iter) != index)
		navigate(iter);

	/* If current is NULL, last is the one that corresponds to the correct Node. */
	ll_Node* const located = iter->current == NULL ? iter->last : iter->current;
	ll_iter_destroy(iter);
	return located;
}

/*
 * Links the pointers of two Nodes together.
 * Θ(1)
 */
void ll_link(ll_Node* const left, ll_Node* const right)
{
	left->next = right;
	right->prev = left;
}

/*
 * Separates a list into two smaller lists.
 * The main list will be emptied into the two smaller lists.
 * The two smaller lists should be initialized before calling this function.
 * Ω(n)
 */
void ll_separate(LinkedList* const list, LinkedList* const l1, LinkedList* const l2)
{
	while (!ll_empty(list))
	{
		ll_push_back(l1, ll_pull_front(list));
		if (!ll_empty(list))
			ll_push_back(l2, ll_pull_front(list));
	}
}

/*
 * Pops an element from the front of the list and returns it.
 * Ω(1)
 */
void* ll_pull_front(LinkedList* const list)
{
	void* const data = ll_front(list);
	ll_pop_front(list);
	return data;
}

/*
* Pops an element from the back of the list and returns it.
* Ω(1)
*/
void* ll_pull_back(LinkedList* const list)
{
	void* const data = ll_back(list);
	ll_pop_back(list);
	return data;
}
