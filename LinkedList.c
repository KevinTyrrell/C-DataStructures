
#include "LinkedList.h"

/* Constructor function. */
static struct ll_Node* ll_Node_new(void *data);
/* Helper function. Locates a Node by index in the LinkedList. */
static struct ll_Node* ll_find(const struct LinkedList *list, const size_t index);

/* Constructor function. */
struct LinkedList* LinkedList_new(bool(*equals)(void*, void*), char*(*toString)(void*))
{
	struct LinkedList *list = calloc(1, sizeof(struct LinkedList));
	if (list == NULL)
		ds_Error(DS_MSG_OUT_OF_MEM);
	// Pointer functions.
	list->equals = equals;
	list->toString = toString;
	return list;
}

/* Constructor function. */
static struct ll_Node* ll_Node_new(void *data)
{
	struct ll_Node *node = calloc(1, sizeof(struct ll_Node));
	if (node == NULL)
		ds_Error(DS_MSG_OUT_OF_MEM);
	node->data = data;
	return node;
}

/*
Helper function. Locates a Node by index in the LinkedList.

O(n / 2) complexity.
Prints an error if the index was out of bounds.
Returns NULL if the Node cannot be located.
*/
static struct ll_Node* ll_find(const struct LinkedList *list, const size_t index)
{
	if (index >= list->size)
		ds_Error(DS_MSG_OUT_OF_BOUNDS);
	else
	{
		bool rootSeek = (double)index / list->size <= 0.5;
		struct ll_Node *iter = (rootSeek ? list->root : list->tail);

		for (size_t i = rootSeek ? 0 : list->size - 1; rootSeek ? i < index : i > index; 
			rootSeek ? i++ : i--, iter = rootSeek ? iter->next : iter->prev) {}
		return iter;
	}

	return NULL;
}

/*
Adds data at the front of the LinkedList.

O(1) complexity.
*/
void ll_add(struct LinkedList *list, void *data)
{
	struct ll_Node *insert = ll_Node_new(data);
	
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

/*
Adds data at the given index in the LinkedList.

O(log n) complexity (worst case).
Prints an error if the index is out of bounds.
*/
void ll_addAt(struct LinkedList *list, const size_t index, void *data)
{
	if (list->size == 0 || index == 0)
		ll_add(list, data);
	else if (index == list->size)
		ll_addLast(list, data);
	else
	{
		struct ll_Node *iter = ll_find(list, index);
		if (iter != NULL)
		{
			struct ll_Node *insert = ll_Node_new(data);
			if (iter->prev != NULL)
				iter->prev->next = insert;
			insert->prev = iter->prev;
			insert->next = iter;
			iter->prev = insert;
			list->size++;
		}
	}
}

/*
Adds data to the end of the LinkedList.

O(1) complexity.
*/
void ll_addLast(struct LinkedList *list, void *data)
{
	if (list->size == 0)
		ll_add(list, data);
	else
	{
		struct ll_Node *insert = ll_Node_new(data);
		insert->prev = list->tail;
		list->tail->next = insert;
		list->tail = insert;
		list->size++;
	}
}

/*
Removes a Node at the given index.

O(n/2) complexity.
Returns the value of the removed Node.
Returns NULL if the Node could not be found.
*/
void* ll_removeAt(struct LinkedList *list, const size_t index)
{
	if (index == 0)
		return ll_removeFirst(list);
	else if (index == list->size - 1)
		return ll_removeLast(list);
	else
	{
		struct ll_Node *iter = ll_find(list, index);
		iter->prev->next = iter->next;
		iter->next->prev = iter->prev;
		void *val = iter->data;
		free(iter);
		list->size--;
		return val;
	}

	return NULL;
}

/*
Attempts to remove a Node with matching data, if it exists.

O(n) complexity.
Returns the value of the removed Node. NULL if the operation failed.
*/
void* ll_remove(struct LinkedList *list, void *data)
{
	size_t i = 0;
	for (struct ll_Node *iter = list->root; iter != NULL; iter = iter->next, i++)
		// This Node matches what we are searching for.
		if (list->equals(iter->data, data))
			if (i == 0)
				return ll_removeFirst(list);
			else if (i == list->size - 1)
				return ll_removeLast(list);
			else
			{
				iter->prev->next = iter->next;
				iter->next->prev = iter->prev;
				void *val = iter->data;
				free(iter);
				list->size--;
				return val;
			}

	return NULL;
}

/*
Removes the first Node of the LinkedList.

O(1) complexity.
Returns the removed value, NULL if operation failed.
*/
void* ll_removeFirst(struct LinkedList *list)
{
	if (list->size > 0)
	{
		void* val = list->root->data;
		struct ll_Node *temp = (list->size > 1) ? list->root->next : NULL;
		if (temp != NULL)
			temp->prev = NULL;

		if (list->size == 1)
			list->tail = NULL;
		else if (list->size == 2)
			list->tail = temp;

		free(list->root);
		list->root = temp;
		list->size--;
		return val;
	}

	return NULL;
}

/*
Removes the last Node of the LinkedList.

O(1) complexity.
Returns the removed value, NULL if the operation failed.
*/
void* ll_removeLast(struct LinkedList *list)
{
	if (list->size == 1)
		return ll_removeFirst(list);
	else if (list->size > 1)
	{
		void* val = list->tail->data;
		struct ll_Node *temp = list->tail->prev;
		
		if (list->size == 2)
			list->root = temp;
		
		temp->next = NULL;
		free(list->tail);
		list->tail = temp;
		list->size--;
		return val;
	}

	return NULL;
}

/*
Destroys every Node of the LinkedList.

O(n) complexity.
*/
void ll_clear(struct LinkedList *list)
{
	for (struct ll_Node *iter = list->root, *prev = NULL; iter != NULL; prev = iter, iter = iter->next)
		free(prev);
	list->root = NULL;
	list->tail = NULL;
	list->size = 0;
}

/*
Returns a copy of the LinkedList.

O(n) complexity.
*/
struct LinkedList* ll_clone(const struct LinkedList *list)
{
	struct LinkedList *copy = LinkedList_new(list->equals, list->toString);

	for (struct ll_Node *iter = list->root; iter != NULL; iter = iter->next)
		ll_addLast(copy, iter->data);

	return copy;
}

/*
Deconstructor function.

O(n) complexity.
*/
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

	for (struct ll_Node *iter = list->root; iter != NULL; iter = iter->next)
	{
		printf_s("%s", list->toString(iter->data));
		if (iter->next != NULL)
			printf_s(", ");
	}

	printf(" }\n");
}