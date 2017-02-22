
#include "LinkedList.h"

/* Local typedef for convenience. */
typedef struct ll_Node Node;
typedef struct LinkedList List;

/* Local functions. */
static Node* Node_new(void *data);
static Node* ll_find(const struct LinkedList *list, const size_t index);

/* Constructor function. */
List* LinkedList_new(bool(*equals)(void*, void*), char*(*toString)(void*))
{
	List *list = calloc(1, sizeof(List));
	if (list == NULL)
		ds_Error(DS_MSG_OUT_OF_MEM);
	// Pointer functions.
	list->equals = equals;
	list->toString = toString;
	return list;
}

/* Constructor function. */
static Node* Node_new(void *data)
{
	Node *node = calloc(1, sizeof(Node));
	if (node == NULL)
		ds_Error(DS_MSG_OUT_OF_MEM);
	node->data = data;
	return node;
}

/* Inserts data at a specific position in the LinkedList. */
void ll_add(struct LinkedList *list, const size_t index, void *data)
{
	if (list->size == 0 || index == 0)
		ll_addFirst(list, data);
	else if (index == list->size)
		ll_addLast(list, data);
	else
	{
		Node *iter = ll_find(list, index);
		if (iter != NULL)
		{
			Node *insert = Node_new(data);
			iter->prev->next = insert;
			insert->prev = iter->prev;
			insert->next = iter;
			iter->prev = insert;
			list->size++;
		}
	}
}

/* Inserts data at the front of the LinkedList. */
void ll_addFirst(struct LinkedList *list, void *data)
{
	Node *insert = Node_new(data);
	
	if (list->root == NULL)
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
void ll_addLast(struct LinkedList *list, void *data)
{
	if (list->size == 0)
		ll_addFirst(list, data);
	else
	{
		Node *insert = Node_new(data);
		insert->prev = list->tail;
		list->tail->next = insert;
		list->tail = insert;
		list->size++;
	}
}

/* Removes all data inside the LinkedList. */
void ll_clear(struct LinkedList *list)
{
	for (Node *iter = list->root, *prev = NULL; iter != NULL; prev = iter, iter = iter->next)
		free(prev);
	list->root = NULL;
	list->tail = NULL;
	list->size = 0;
}

/* Returns a shallow copy of this LinkedList. */
List* ll_clone(const struct LinkedList *list)
{
	List *copy = LinkedList_new(list->equals, list->toString);

	for (Node *iter = list->root; iter != NULL; iter = iter->next)
		ll_addLast(copy, iter->data);
	return copy;
}

bool ll_isEmpty(const struct LinkedList * list)
{
	return list->size == 0;
}

/* Returns true if the LinkedList contains the specified data. */
bool ll_contains(const struct LinkedList *list, void *data)
{
	for (Node *iter = list->root; iter != NULL; iter = iter->next)
		if (list->equals(iter->data, data))
			return true;
	return false;
}

/* Returns the data at the specified position in the LinkedList. */
void* ll_get(const struct LinkedList *list, const size_t index)
{
	if (list->size == 0)
		return NULL;
	if (index == 0)
		return ll_getFirst(list);
	if (index == list->size - 1)
		return ll_getLast(list);
	return ll_find(list, index);
}

/* Returns the data at the front of the LinkedList. */
void* ll_getFirst(const struct LinkedList *list)
{
	return (list->size == 0) ? NULL : list->root->data;
}

/* Returns the data at the end of the LinkedList. */
void* ll_getLast(const struct LinkedList *list)
{
	return (list->size == 0) ? NULL : list->tail->data;
}

/* Removes the data at the front of the LinkedList and returns it. */
void* ll_removeFirst(struct LinkedList *list)
{
	if (list->size == 0)
		return NULL;

	void *val = list->root->data;
	Node *temp = list->root->next;
	free(list->root);
	
	if (list->size == 1)
	{
		list->root = NULL;
		list->tail = NULL;
	}
	else
	{
		temp->prev = NULL;
		list->root = temp;
	}

	list->size--;
	return val;
}

/* Removes the data at the end of the LinkedList and returns it. */
void* ll_removeLast(struct LinkedList *list)
{
	if (list->size <= 1)
		return ll_removeFirst(list);
	
	void *val = list->tail->data;
	Node *temp = list->tail->prev;
	free(list->tail);
	temp->next = NULL;
	list->tail = temp;
	list->size--;

	return val;
}

/* Removes the data at the specified position in the LinkedList. */
void* ll_removeAt(struct LinkedList *list, const size_t index)
{
	if (index == 0)
		return ll_removeFirst(list);
	if (index == list->size - 1)
		return ll_removeLast(list);
	
	Node *located = ll_find(list, index);
	if (located == NULL)
		return NULL;

	void *val = located->data;
	located->prev->next = located->next;
	located->next->prev = located->prev;
	free(located);
	list->size--;
	return val;
}

/* Removes the first occurrence of the data from the LinkedList, if it exists. */
bool ll_remove(struct LinkedList *list, void *data)
{
	for (Node *iter = list->root; iter != NULL; iter = iter->next)
		if (list->equals(iter->data, data))
		{
			if (iter == list->root)
				ll_removeFirst(list);
			else if (iter == list->tail)
				ll_removeLast(list);
			else
			{
				iter->prev->next = iter->next;
				iter->next->prev = iter->prev;
				free(iter);
				list->size--;
			}

			return true;
		}

	return false;
}

/* Randomizes the position of all elements inside the LinkedList. */
void ll_shuffle(struct LinkedList *list)
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
		Node *swap = ll_find(list, (size_t)(random % i));
		void *temp = swap->data;
		swap->data = end->data;
		end->data = temp;
		end = end->prev;
	}
}

/* De-constructor function. */
void ll_destroy(struct LinkedList *list)
{
	ll_clear(list);
	free(list);
}

/*
Prints out the LinkedList to the console window.

O(n) complexity.
Attempts to cast the void* data into char*.
*/
void ll_print(const struct LinkedList *list)
{
	printf_s("%s%zu%s", "List Size: ", list->size, " { ");

	for (Node *iter = list->root; iter != NULL; iter = iter->next)
	{
		printf_s("%s", list->toString(iter->data));
		if (iter->next != NULL)
			printf_s(", ");
	}

	printf(" }\n");
}

/*
Helper function. Locates a Node by index in the LinkedList.
Prints an error if the index was out of bounds.
Returns NULL if the Node cannot be located.
*/
static Node* ll_find(const struct LinkedList *list, const size_t index)
{
	if (index >= list->size)
		ds_Error(DS_MSG_OUT_OF_BOUNDS);
	else
	{
		/* Seek from the head or the tail. */
		bool rootSeek = (double)index / list->size <= 0.5;
		Node *iter = (rootSeek ? list->root : list->tail);

		for (size_t i = rootSeek ? 0 : list->size - 1; rootSeek ? i < index : i > index;) 
		{
			rootSeek ? i++ : i--;
			iter = rootSeek ? iter->next : iter->prev;
		}

		return iter;
	}

	return NULL;
}