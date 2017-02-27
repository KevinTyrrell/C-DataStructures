
#include "LinkedList.h"

/* Local typedef for convenience. */
typedef struct ll_Node Node;
typedef struct LinkedList List;

/* Local functions. */
Node* Node_new(const void* const data);
Node* ll_search(const struct LinkedList* const list, const size_t index);

/* Constructor function. */
struct LinkedList* LinkedList_new(bool(*equals)(void*, void*), char*(*toString)(void*))
{
	List* const list = ds_calloc(1, sizeof(List));
	/* Pointer functions. */
	list->equals = equals;
	list->toString = toString;
	return list;
}

/* Constructor function. */
Node* Node_new(const void* const data)
{
	Node *node = ds_calloc(1, sizeof(Node));
	node->data = data;
	return node;
}

/* Returns the data at the front of the LinkedList. */
void* ll_front(const struct LinkedList* const list)
{
	return list->root->data;
}

/* Returns the data at the end of the LinkedList. */
void* ll_back(const struct LinkedList* const list)
{
	return list->tail->data;
}

/* Returns the data at the specified position in the LinkedList. */
void* ll_at(const struct LinkedList* const list, const size_t index)
{
	if (ll_empty(list))
		return NULL;
	if (index == 0)
		return ll_front(list);
	if (index == list->size - 1)
		return ll_back(list);
	return ll_search(list, index)->data;
}

/* Returns true if the LinkedList contains the specified data. */
bool ll_contains(const struct LinkedList* const list, const void* const data)
{
	for (const Node *iter = list->root; iter != NULL; iter = iter->next)
		if (list->equals(iter->data, data))
			return true;
	return false;
}

/* Returns true if the LinkedList is empty. */
bool ll_empty(const struct LinkedList* const list)
{
	return list->size == 0;
}

/* Returns a shallow copy of the given LinkedList. */
struct LinkedList* ll_clone(const struct LinkedList* const list)
{
	const List* const copy = LinkedList_new(list->equals, list->toString);
	for (const Node *iter = list->root; iter != NULL; iter = iter->next)
		ll_push_back(copy, iter->data);
	return copy;
}

/*
Prints out the LinkedList to the console window.

O(n) complexity.
Attempts to cast the void* data into char*.
*/
void ll_print(const struct LinkedList* const list)
{
	printf_s("%s%zu%s", "List Size: ", list->size, " { ");

	for (const Node *iter = list->root; iter != NULL; iter = iter->next)
	{
		printf_s("%s", list->toString(iter->data));
		if (iter->next != NULL)
			printf_s(", ");
	}

	printf(" }\n");
}

/* Inserts data at the front of the LinkedList. */
void ll_push_front(struct LinkedList* const list, const void* const data)
{
	Node* const insert = Node_new(data);

	if (ll_empty(list))
		list->tail = insert;
	else
	{
		insert->next = list->root;
		list->root->prev = insert;
	}

	list->root = insert;
	list->size++;
}

/* Inserts data at the end of the LinkedList. */
void ll_push_back(struct LinkedList* const list, const void* const data)
{
	if (ll_empty(list))
		ll_push_front(list, data);
	else
	{
		Node* const insert = Node_new(data);
		insert->prev = list->tail;
		list->tail->next = insert;
		list->tail = insert;
		list->size++;
	}
}

/* Overwrites a value in the LinkedList at a given index. */
void ll_assign(const struct LinkedList* const list, const size_t index, const void * const data)
{
	Node* const located = ll_search(list, index);
	located->data = data;
}

/* Inserts data at a specific position in the LinkedList. */
void ll_insert(struct LinkedList* const list, const size_t index, const void* const data)
{
	if (ll_empty(list) || index == 0)
		ll_push_front(list, data);
	else if (index == list->size)
		ll_push_back(list, data);
	else
	{
		Node* const located = ll_search(list, index);
		if (located != NULL)
		{
			Node* const insert = Node_new(data);
			located->prev->next = insert;
			insert->prev = located->prev;
			insert->next = located;
			located->prev = insert;
			list->size++;
		}
	}
}

/* Removes the data at the front of the LinkedList and returns it. */
void ll_pop_front(struct LinkedList* const list)
{
	if (ll_empty(list))
		return;

	const Node* const root = list->root;
	if (list->size == 1)
	{
		list->root = NULL;
		list->tail = NULL;
	}
	else
	{
		Node* const neighbor = root->next;
		neighbor->prev = NULL;
		list->root = neighbor;
	}

	ds_free(root, sizeof(Node));
	list->size--;
}

/* Removes the data at the end of the LinkedList and returns it. */
void ll_pop_back(struct LinkedList* const list)
{
	if (ll_empty(list))
		return;

	const Node* const tail = list->tail;
	if (list->size == 1)
	{
		list->root = NULL;
		list->tail = NULL;
	}
	else
	{
		Node* const neighbor = tail->prev;
		neighbor->next = NULL;
		list->tail = neighbor;
	}

	ds_free(tail, sizeof(Node));
	list->size--;
}

/* Removes the first occurrence of the data from the LinkedList, if it exists. */
bool ll_remove(struct LinkedList* const list, const void* const data)
{
	for (const Node *iter = list->root; iter != NULL; iter = iter->next)
		if (list->equals(iter->data, data))
		{
			if (iter == list->root)
				ll_pop_front(list);
			else if (iter == list->tail)
				ll_pop_back(list);
			else
			{
				iter->prev->next = iter->next;
				iter->next->prev = iter->prev;
				ds_free(iter, sizeof(Node));
				list->size--;
			}

			return true;
		}

	return false;
}

/* Removes the data at the specified position in the LinkedList. */
void ll_erase(struct LinkedList* const list, const size_t index)
{
	if (ll_empty(list))
		return;
	if (index == 0)
		ll_pop_front(list);
	else if (index == list->size - 1)
		ll_pop_back(list);
	else
	{
		const Node* const located = ll_search(list, index);
		located->prev->next = located->next;
		located->next->prev = located->prev;
		ds_free(located, sizeof(Node));
		list->size--;
	}
}

/* Removes all data inside the LinkedList. */
void ll_clear(struct LinkedList* const list)
{
	const Node* iter = list->root;
	while (iter != NULL)
	{
		const Node* const temp = iter->next;
		ds_free(iter, sizeof(Node));
		iter = temp;
	}

	list->root = NULL;
	list->tail = NULL;
	list->size = 0;
}

/* Randomizes the position of all elements inside the LinkedList. */
void ll_shuffle(const struct LinkedList* const list)
{
	if (list->size <= 1)
		return;

	srand((unsigned int)time(NULL));
	rand();
	rand();

	Node *end = list->tail;
	for (size_t i = list->size; i > 1; i--)
	{
		/* Generate a massive random number. */
		unsigned long long random =
			(((unsigned long long)rand() << 0) & 0x00000000FFFFFFFFull) |
			(((unsigned long long)rand() << 32) & 0xFFFFFFFF00000000ull);
		Node* const swap = ll_search(list, (size_t)(random % i));
		const void* const temp = swap->data;
		swap->data = end->data;
		end->data = temp;
		end = end->prev;
	}
}

/* De-constructor function. */
void ll_destroy(const struct LinkedList* const list)
{
	ll_clear(list);
	ds_free(list, sizeof(List));
}

/*
Helper function. Locates a Node by index in the LinkedList.
Prints an error if the index was out of bounds.
Returns NULL if the Node cannot be located.
*/
Node* ll_search(const struct LinkedList* const list, const size_t index)
{
	if (index >= list->size)
	{
		ds_error(DS_MSG_OUT_OF_BOUNDS);
		return NULL;
	}

	/* Search from the head or the tail depending on which is closer. */
	const bool seek_front = (double)index / list->size <= 0.5;
	const Node *iter = NULL;
	if (seek_front)
	{
		iter = list->root;
		for (unsigned int i = 0; i < index; i++)
			iter = iter->next;
	}
	else
	{
		iter = list->tail;
		for (unsigned int i = list->size - 1; i > index; i--)
			iter = iter->prev;
	}

	return iter;
}