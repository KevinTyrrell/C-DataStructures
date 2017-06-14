
/*
 * File: LinkedList.c
 * Date: Jul 21, 2016
 * Name: Kevin Tyrrell
 * Version: 4.0.0
 */

/*
Copyright © 2017 Kevin Tyrrell

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include "LinkedList.h"

/* Node structure. */
typedef struct list_Node
{
    const void *data;
    struct list_Node *next, *prev;
} list_Node;

/* LinkedList structure. */
struct LinkedList
{
    list_Node *head, *tail;
    size_t size;

    /* Synchronization. */
    ReadWriteSync *rw_sync;

    /* Function pointers. */
    int(*compare)(const void*, const void*);
    char*(*toString)(const void*);
};

/* Structure to assist in looping through List. */
struct list_Iterator
{
    /* Keep track of where we are inside the List. */
    unsigned int index;
    list_Node *left, *right, *last;
    /* Reference to the Vector that it is iterating through. */
    LinkedList *list;
};

/* Local functions. */
static list_Node* list_Node_new(const void* const data);
static list_Node* list_search(const LinkedList* const list, const size_t index);
static list_Node* list_locate(const LinkedList* const list, const void* const data, unsigned int* const index);
static void list_Node_destroy(list_Node* const node);
static void list_Node_clear(list_Node *head);
static void list_delete(LinkedList* const list, list_Node* const deleted);
static void list_link(list_Node* const left, list_Node* const right);
static void list_merge_sort(LinkedList* const list);
static void list_anti_merge_sort(LinkedList* const list);
static void list_separate(LinkedList* const to_be_emptied, LinkedList* const l1, LinkedList* const l2);
static void* list_pull_front(LinkedList* const list);
static void* list_pull_back(LinkedList* const list);

/*
 * Constructor function.
 * Θ(1)
 */
LinkedList* LinkedList_new(int(*compare)(const void*, const void*),
                           char*(*toString)(const void*))
{
    LinkedList* const list = mem_calloc(1, sizeof(LinkedList));
    list->compare = compare;
    list->toString = toString;
    list->rw_sync = ReadWriteSync_new();
    return list;
}

/*
 * Returns the element at the specified index.
 * see: list_search
 * Θ(n)
 */
void* list_at(const LinkedList* const list, const unsigned int index)
{
    io_assert(list != NULL, IO_MSG_NULL_PTR);

    /* Lock the data structure to future writers. */
    sync_read_start(list->rw_sync);

    io_assert(index < list->size, IO_MSG_OUT_OF_BOUNDS);

    const void *val = NULL;

    if (index == 0)
        val = list_front(list);
    else if (index == list->size - 1)
        val = list_back(list);
    else
        val = list_search(list, index)->data;

    /* Unlock the data structure. */
    sync_read_end(list->rw_sync);

    return (void*)val;
}

/*
 * Returns the element at the front of the List.
 * Θ(1)
 */
void* list_front(const LinkedList* const list)
{
    io_assert(list != NULL, IO_MSG_NULL_PTR);

    /* Lock the data structure to future writers. */
    sync_read_start(list->rw_sync);

    io_assert(list->size > 0, IO_MSG_EMPTY);
    
    const void* const val = list->head->data;

    /* Unlock the data structure. */
    sync_read_end(list->rw_sync);

    return (void*)val;
}

/*
 * Returns the element at the back of the List.
 * Θ(1)
 */
void* list_back(const LinkedList* const list)
{
    io_assert(list != NULL, IO_MSG_NULL_PTR);

    /* Lock the data structure to future writers. */
    sync_read_start(list->rw_sync);

    io_assert(list->size > 0, IO_MSG_EMPTY);

    const void* const val = list->tail->data;

    /* Unlock the data structure. */
    sync_read_end(list->rw_sync);

    return (void*)val;
}

/*
 * Returns the size of the List.
 * Θ(1)
 */
size_t list_size(const LinkedList* const list)
{
    io_assert(list != NULL, IO_MSG_NULL_PTR);

    /* Lock the data structure to future writers. */
    sync_read_start(list->rw_sync);

    const size_t size = list->size;

    /* Unlock the data structure. */
    sync_read_end(list->rw_sync);

    return size;
}

/*
 * Returns true if the List is empty.
 * Θ(1)
 */
bool list_empty(const LinkedList* const list)
{
    io_assert(list != NULL, IO_MSG_NULL_PTR);

    /* Lock the data structure to future writers. */
    sync_read_start(list->rw_sync);

    const bool val = list->size == 0;

    /* Unlock the data structure. */
    sync_read_end(list->rw_sync);

    return val;
}

/*
 * Returns the index in the List of the first occurrence of the specified element.
 * Returns false if no such element is found in the List.
 * The index parameter should be non-NULL and point to valid memory space.
 * The `compare` function must be defined to call this function.
 * see: list_locate
 * Θ(n)
 */
bool list_index(const LinkedList* const list, const void* const data, unsigned int* const index)
{
    io_assert(list != NULL, IO_MSG_NULL_PTR);
    io_assert(data != NULL, IO_MSG_NULL_PTR);
    io_assert(index != NULL, IO_MSG_NULL_PTR);
    io_assert(list->compare != NULL, IO_MSG_NOT_SUPPORTED);

    /* Lock the data structure to future writers. */
    sync_read_start(list->rw_sync);

    const bool found = list_locate(list, data, index) != NULL;

    /* Unlock the data structure. */
    sync_read_end(list->rw_sync);

    return found;
}

/*
 * Returns true if the List contains the specified element.
 * The `compare` function must be defined to call this function.
 * Θ(n)
 */
bool list_contains(const LinkedList* const list, const void* const data)
{
    io_assert(list != NULL, IO_MSG_NULL_PTR);
    io_assert(data != NULL, IO_MSG_NULL_PTR);
    io_assert(list->compare != NULL, IO_MSG_NOT_SUPPORTED);

    /* Lock the data structure to future writers. */
    sync_read_start(list->rw_sync);

    unsigned int temp;
    const bool success = list_locate(list, data, &temp) != NULL;

    /* Unlock the data structure. */
    sync_read_end(list->rw_sync);

    return success;
}

/*
 * Returns an array of all elements inside the List.
 * Remember to call `free` on the dynamically allocated array.
 * Θ(n)
 */
void** list_array(const LinkedList* const list)
{
    io_assert(list != NULL, IO_MSG_NULL_PTR);

    void **arr = calloc(list->size, sizeof(void*));

    /* Lock the data structure to future writers. */
    sync_read_start(list->rw_sync);

    list_Iterator* const iter = list_iter((LinkedList*)list, 0);
    for (unsigned int i = 0; list_iter_has_next(iter); i++)
        arr[i] = list_iter_next(iter);

    /* Unlock the data structure. */
    sync_read_end(list->rw_sync);

    list_iter_destroy(iter);
    return arr;
}

/*
 * Prints out the contents of the List to the console window.
 * The `toString` function must be defined to call this function.
 * Θ(n)
 */
void list_print(const LinkedList* const list)
{
    io_assert(list != NULL, IO_MSG_NULL_PTR);
    io_assert(list->toString != NULL, IO_MSG_NOT_SUPPORTED);

    /* Lock the data structure to future writers. */
    sync_read_start(list->rw_sync);

    list_Iterator* const iter = list_iter((LinkedList*)list, 0);
    printf("%c", '[');
    while (list_iter_has_next(iter))
    {
        char* value = list->toString(list_iter_next(iter));
        printf("%s%s", value, list_iter_has_next(iter) ? ", " : "");
    }
    printf("]\n");

    /* Unlock the data structure. */
    sync_read_end(list->rw_sync);

    list_iter_destroy(iter);
}

/*
 * Returns a shallow copy of the List.
 * Θ(n)
 */
LinkedList* list_clone(const LinkedList* const list)
{
    io_assert(list != NULL, IO_MSG_NULL_PTR);

    LinkedList* const copy = LinkedList_new(list->compare, list->toString);

    /* Lock the data structure to future writers. */
    sync_read_start(list->rw_sync);

    list_Iterator* const iter = list_iter((LinkedList*)list, 0);
    while (list_iter_has_next(iter))
        list_push_back(copy, list_iter_next(iter));

    /* Unlock the data structure. */
    sync_read_end(list->rw_sync);

    list_iter_destroy(iter);
    return copy;
}

/*
 * Replaces an element in the List at a specified index.
 * see: list_search
 * Θ(n)
 */
void list_assign(const LinkedList* const list, const unsigned int index, const void* const data)
{
    io_assert(list != NULL, IO_MSG_NULL_PTR);
    io_assert(data != NULL, IO_MSG_NULL_PTR);

    /* Lock the data structure to future readers/writers. */
    sync_write_start(list->rw_sync);

    io_assert(index < list->size, IO_MSG_OUT_OF_BOUNDS);

    list_Node* const located = list_search(list, index);
    located->data = data;

    /* Unlock the data structure. */
    sync_write_end(list->rw_sync);
}

/*
 * Inserts an element at the specified index in the List.
 * see: list_search
 * Ω(1), O(n)
 */
void list_insert(LinkedList* const list, const unsigned int index, const void* const data)
{
    io_assert(list != NULL, IO_MSG_NULL_PTR);
    io_assert(data != NULL, IO_MSG_NULL_PTR);

    /* Lock the data structure to future readers/writers. */
    sync_write_start(list->rw_sync);

    io_assert(index <= list->size, IO_MSG_OUT_OF_BOUNDS);

    if (index == 0)
        list_push_front(list, data);
    else if (index == list->size)
        list_push_back(list, data);
    else
    {
        list_Node* const inserted = list_Node_new(data);
        list_Node* const neighbor = list_search(list, index);
        list_link(neighbor->prev, inserted);
        list_link(inserted, neighbor);
        list->size++;
    }

    /* Unlock the data structure. */
    sync_write_end(list->rw_sync);
}

/*
 * Removes an element from the List and returns true if the removal was successful.
 * The `compare` function must be defined to call this function.
 * Θ(n)
 */
bool list_remove(LinkedList* const list, const void* const data)
{
    io_assert(list != NULL, IO_MSG_NULL_PTR);
    io_assert(data != NULL, IO_MSG_NULL_PTR);
    io_assert(list->compare != NULL, IO_MSG_NOT_SUPPORTED);
    
    /* Lock the data structure to future readers/writers. */
    sync_write_start(list->rw_sync);

    bool success;
    unsigned int temp;
    list_Node* const to_be_removed = list_locate(list, data, &temp);
    if ((success = to_be_removed != NULL))
        list_delete(list, to_be_removed);

    /* Unlock the data structure. */
    sync_write_end(list->rw_sync);

    return success;
}

/*
 * Removes an element from the List at a specified index.
 * see: list_search
 * Θ(1), O(n)
 */
void list_erase(LinkedList* const list, const size_t index)
{
    io_assert(list != NULL, IO_MSG_NULL_PTR);

    /* Lock the data structure to future readers/writers. */
    sync_write_start(list->rw_sync);

    io_assert(index < list->size, IO_MSG_OUT_OF_BOUNDS);

    list_delete(list, list_search(list, index));

    /* Unlock the data structure. */
    sync_write_end(list->rw_sync);
}

/*
 * Appends an element at the end of the List.
 * Θ(1)
 */
void list_push_back(LinkedList* const list, const void* const data)
{
    io_assert(list != NULL, IO_MSG_NULL_PTR);
    io_assert(data != NULL, IO_MSG_NULL_PTR);

    list_Node* const insert = list_Node_new(data);

    /* Lock the data structure to future readers/writers. */
    sync_write_start(list->rw_sync);

    if (list->size > 0)
        list_link(list->tail, insert);
    else
        list->head = insert;

    list->tail = insert;
    list->size++;

    /* Unlock the data structure. */
    sync_write_end(list->rw_sync);
}

/*
 * Inserts an element at the front of the List.
 * Θ(1)
 */
void list_push_front(LinkedList* const list, const void* const data)
{
    io_assert(list != NULL, IO_MSG_NULL_PTR);
    io_assert(data != NULL, IO_MSG_NULL_PTR);

    list_Node* const insert = list_Node_new(data);

    /* Lock the data structure to future readers/writers. */
    sync_write_start(list->rw_sync);

    if (list->size > 0)
        list_link(insert, list->head);
    else
        list->tail = insert;

    list->head = insert;
    list->size++;

    /* Unlock the data structure. */
    sync_write_end(list->rw_sync);
}

/*
 * Removes the element at the end of the List.
 * Θ(1)
 */
void list_pop_back(LinkedList* const list)
{
    io_assert(list != NULL, IO_MSG_NULL_PTR);

    /* Lock the data structure to future readers/writers. */
    sync_write_start(list->rw_sync);

    io_assert(list->size > 0, IO_MSG_EMPTY);

    list_Node* const tail = list->tail;
    if (list->size == 1)
    {
        list->head = NULL;
        list->tail = NULL;
    }
    else
    {
        list->tail = tail->prev;
        list->tail->next = NULL;
    }

    list->size--;

    /* Unlock the data structure. */
    sync_write_end(list->rw_sync);

    list_Node_destroy(tail);
}

/*
 * Removes the element at the front of the List.
 * Θ(1)
 */
void list_pop_front(LinkedList* const list)
{
    io_assert(list != NULL, IO_MSG_NULL_PTR);

    /* Lock the data structure to future readers/writers. */
    sync_write_start(list->rw_sync);

    io_assert(list->size > 0, IO_MSG_EMPTY);

    list_Node* const head = list->head;
    if (list->size == 1)
    {
        list->head = NULL;
        list->tail = NULL;
    }
    else
    {
        list->head = head->next;
        list->head->prev = NULL;
    }

    list->size--;

    /* Unlock the data structure. */
    sync_write_end(list->rw_sync);

    list_Node_destroy(head);
}

/*
 * Removes all elements from the List.
 * De-construction of the Node chain takes place in an alternate thread.
 * Θ(n)
 */
void list_clear(LinkedList* const list)
{
    io_assert(list != NULL, IO_MSG_NULL_PTR);

    /* Lock the data structure to future readers/writers. */
    sync_write_start(list->rw_sync);

    if (list->size > 0)
    {
        DWORD thread_id;
        const HANDLE cleanup_thread = CreateThread(
                NULL, 0, (LPTHREAD_START_ROUTINE)list_Node_clear, list->head, 0, &thread_id);
        // TODO: Determine cleaner solution for this.
        if (cleanup_thread == NULL)
            printf("Thread could not be created! Error: %lu.\n", GetLastError());
        CloseHandle(cleanup_thread);

        list->head = list->tail = NULL;
        list->size = 0;
    }

    /* Unlock the data structure. */
    sync_write_end(list->rw_sync);
}

/*
 * Sorts the elements inside the List in ascending order.
 * Implementation uses Merge-sort algorithm.
 * The `compare` function must be defined to call this function.
 * see: list_merge_sort
 * Θ(n * log(n))
 */
void list_sort(LinkedList* const list)
{
    io_assert(list != NULL, IO_MSG_NULL_PTR);
    io_assert(list->compare != NULL, IO_MSG_NOT_SUPPORTED);

    /* Lock the data structure to future readers/writers. */
    sync_write_start(list->rw_sync);

    list_merge_sort(list);

    /* Unlock the data structure. */
    sync_write_end(list->rw_sync);
}

/*
 * Shuffles the elements inside the List pseudo-randomly.
 * Implementation uses Merge-sort algorithm.
 * For the Random implementation, see: C-Random
 * see: list_merge_sort
 * Θ(n * log(n))
 */
void list_shuffle(LinkedList* const list)
{
    io_assert(list != NULL, IO_MSG_NULL_PTR);

    /* Lock the data structure to future readers/writers. */
    sync_write_start(list->rw_sync);

    list_anti_merge_sort(list);

    /* Unlock the data structure. */
    sync_write_end(list->rw_sync);
}

/*
 * De-constructor function.
 * Θ(n)
 */
void list_destroy(LinkedList* const list)
{
    io_assert(list != NULL, IO_MSG_NULL_PTR);
    list_clear(list);
    sync_destroy(list->rw_sync);
    mem_free(list, sizeof(LinkedList));
}

/*
 * Constructor function.
 * Θ(1)
 */
list_Iterator* list_iter(LinkedList* const list, const unsigned int index)
{
    io_assert(list != NULL, IO_MSG_NULL_PTR);
    /* If the List is empty, there is nothing to iterate over. */
    io_assert(list->size > 0, IO_MSG_EMPTY);
    io_assert(index < list->size, IO_MSG_OUT_OF_BOUNDS);

    list_Iterator* const iter = mem_calloc(1, sizeof(list_Iterator));
    iter->list = list;

    /* Prepare to navigate to the index that the user wanted. */
    void*(*navigate)(list_Iterator* const);
    /* Determine if seeking from the head is faster. */
    if (index + 1 <= list->size - index)
    {
        iter->right = iter->last = list->head;
        iter->index = 0;
        navigate = &list_iter_next;
    }
    else
    {
        iter->left = iter->last = list->tail;
        iter->index = list->size;
        navigate = &list_iter_prev;
    }

    while (list_iter_index(iter) != index)
        navigate(iter);
    return iter;
}

/*
 * Returns the iterator's current element and advances it forward.
 * Θ(1)
 */
void* list_iter_next(list_Iterator* const iter)
{
    io_assert(iter != NULL, IO_MSG_NULL_PTR);
    io_assert(list_iter_has_next(iter), IO_MSG_OUT_OF_BOUNDS);

    /* Travel over the right-ward Node and save it into `Last`. */
    iter->last = iter->right;
    iter->right = iter->right->next;
    iter->left = iter->last;

    iter->index++;
    return (void*)iter->last->data;
}

/*
 * Returns the iterator's current element and retracts it backward.
 * Θ(1)
 */
void* list_iter_prev(list_Iterator* const iter)
{
    io_assert(iter != NULL, IO_MSG_NULL_PTR);
    io_assert(list_iter_has_prev(iter), IO_MSG_OUT_OF_BOUNDS);

    /* Travel over the right-ward Node and save it into `Last`. */
    iter->last = iter->left;
    iter->left = iter->left->prev;
    iter->right = iter->last;

    iter->index--;
    return (void*)iter->last->data;
}

/*
 * Returns true if the iterator has a next element.
 * Θ(1)
 */
bool list_iter_has_next(const list_Iterator* const iter)
{
    io_assert(iter != NULL, IO_MSG_NULL_PTR);
    return iter->right != NULL;
}

/*
 * Returns true if the iterator has a previous element.
 * Θ(1)
 */
bool list_iter_has_prev(const list_Iterator* const iter)
{
    io_assert(iter != NULL, IO_MSG_NULL_PTR);
    return iter->left != NULL;
}

/*
 * Inserts an element at the iterator's current position.
 * The iterator will treat the inserted element as its 'last' returned element.
 * The element will always be inserted behind the iterator.
 *
 * Before insertion.
 * <- O -> <- O ->
 *        ^I
 *
 * After insertion (if `next` was called most recently)
 * <- O -> <- N -> <- O ->
 *                ^I
 *
 * After insertion (if `prev` was called most recently)
 * <- O -> <- N -> <- O ->
 *        ^I
 * Θ(1)
 */
void list_iter_insert(list_Iterator* const iter, const void* const data)
{
    io_assert(iter != NULL, IO_MSG_NULL_PTR);
    io_assert(data != NULL, IO_MSG_NULL_PTR);
    /* If the List is empty, there is nothing to iterate over. */
    io_assert(iter->left != NULL || iter->right != NULL, IO_MSG_EMPTY);

    list_Node* const inserted = list_Node_new(data);
    if (!list_iter_has_prev(iter))
    {
        list_link(inserted, iter->last);
        iter->left = inserted;
        iter->list->head = inserted;
        /* Inserted behind us, so adjust index. */
        iter->index++;
    }
    else if (!list_iter_has_next(iter))
    {
        list_link(iter->last, inserted);
        iter->right = inserted;
        iter->list->tail = inserted;
    }
    else
    {
        list_link(iter->left, inserted);
        list_link(inserted, iter->right);
        /* Depending on which way the iterator is heading,
         * place the new Node behind `last`. */
        if (iter->left == iter->last)
        {
            iter->left = iter->last = inserted;
            iter->index++;
        }
        else iter->right = iter->last = inserted;
    }

    iter->list->size++;
}

/*
 * Removes the last iterated element from the List.
 * When the iterator is on the edge of the List, the last
 * iterated element is considered to be the head or tail.
 * Θ(1)
 */
void list_iter_remove(list_Iterator* const iter)
{
    io_assert(iter != NULL, IO_MSG_NULL_PTR);
    /* If the List is empty, there is nothing to iterate over. */
    io_assert(iter->left != NULL || iter->right != NULL, IO_MSG_EMPTY);

    list_Node* const removed = iter->last;

    /*
     * Depending on where the iterator is and what direction it was heading in,
     * move the three pointers to ensure 'last' is never NULL.
     * TODO: It may be possible to simplify this.
     */
    if (iter->left == NULL)
        iter->last = iter->right = iter->right->next;
    else if (iter->right == NULL)
    {
        iter->last = iter->left = iter->left->prev;
        iter->index--;
    }
    else if (iter->last == iter->left)
    {
        iter->left = iter->left->prev;
        iter->last = (iter->left != NULL) ? iter->left : iter->right;
        iter->index--;
    }
    else
    {
        iter->right = iter->right->next;
        iter->last = (iter->right != NULL) ? iter->right : iter->left;
    }

    list_delete(iter->list, removed);
}

/* 
 * Returns the index of the last iterated element.
 * Θ(1)
 */
unsigned int list_iter_index(const list_Iterator* const iter)
{
    io_assert(iter != NULL, IO_MSG_NULL_PTR);
    io_assert(iter->left != NULL || iter->right != NULL, IO_MSG_EMPTY);
    /* The iterator lies BETWEEN Nodes, not on them. Thus index depend on iter position. */
    return (iter->right == iter->last) ? iter->index : iter->index - 1;
}

/*
 * De-constructor function.
 * Θ(1)
 */
void list_iter_destroy(list_Iterator* const iter)
{
    mem_free(iter, sizeof(list_Iterator));
}

/*
 * Constructor function.
 * Θ(1)
 */
list_Node* list_Node_new(const void* const data)
{
    io_assert(data != NULL, IO_MSG_NULL_PTR);
    list_Node* const node = mem_calloc(1, sizeof(list_Node));
    node->data = data;
    return node;
}

/*
 * Returns the Node at the specified index.
 * see: list_iter
 * Θ(n)
 */
list_Node* list_search(const LinkedList* const list, const size_t index)
{
    io_assert(list != NULL, IO_MSG_NULL_PTR);
    io_assert(index < list->size, IO_MSG_OUT_OF_BOUNDS);

    list_Iterator* const iter = list_iter((LinkedList*)list, index);
    const list_Node* const found = iter->last;
    list_iter_destroy(iter);

    return (list_Node*)found;
}

/*
 * Returns a Node in the List whose data matches the specified data.
 * If the Node is located, the value which `index` points to will reflect its index.
 * Returns NULL if no such Node exists in the List.
 * The `compare` function must be defined to call this function.
 * Θ(n)
 */
static list_Node* list_locate(const LinkedList* const list, const void* const data, unsigned int* const index)
{
    io_assert(list != NULL, IO_MSG_NULL_PTR);
    io_assert(data != NULL, IO_MSG_NULL_PTR);
    io_assert(index != NULL, IO_MSG_NULL_PTR);
    io_assert(list->compare != NULL, IO_MSG_NOT_SUPPORTED);

    const list_Node *located = NULL;

    list_Iterator* const iter = list_iter((LinkedList*)list, 0);
    while (list_iter_has_next(iter))
    {
        list_iter_next(iter);
        const list_Node* const iterated = iter->last;
        if (iterated->data == data || list->compare(iterated->data, data) == 0)
        {
            located = iterated;
            *index = list_iter_index(iter);
            break;
        }
    }

    list_iter_destroy(iter);
    return (list_Node*)located;
}

/*
 * De-constructor function.
 * Θ(1)
 */
void list_Node_destroy(list_Node *const node)
{
    io_assert(node != NULL, IO_MSG_NULL_PTR);
    mem_free(node, sizeof(list_Node));
}

/*
 * De-constructs all Nodes starting from the head Node.
 * Θ(n)
 */
void list_Node_clear(list_Node *head)
{
    io_assert(head != NULL, IO_MSG_NULL_PTR);

    do
    {
        list_Node* const temp = head;
        head = head->next;
        list_Node_destroy(temp);
    } while (head != NULL);
}

/*
 * Removes a specified Node from the List.
 * Θ(1)
 */
static void list_delete(LinkedList* const list, list_Node* const deleted)
{
    io_assert(list != NULL, IO_MSG_NULL_PTR);
    io_assert(deleted != NULL, IO_MSG_NULL_PTR);

    /* Remove the Node from the List. */
    if (deleted->prev != NULL)
        deleted->prev->next = deleted->next;
    if (deleted->next != NULL)
        deleted->next->prev = deleted->prev;

    /* Manage the List's properties. */
    if (deleted == list->head)
        list->head = deleted->next;
    if (deleted == list->tail)
        list->tail = deleted->prev;
    list->size--;

    list_Node_destroy(deleted);
}

/*
 * Links the pointers of a left-ward Node with a right-ward neighbor.
 * 								(left)	(right)
 * Before function call:		O		O
 * After function call:			O -> <- O
 * Θ(1)
 */
void list_link(list_Node* const left, list_Node* const right)
{
    io_assert(left != NULL, IO_MSG_NULL_PTR);
    io_assert(right != NULL, IO_MSG_NULL_PTR);
    left->next = right;
    right->prev = left;
}

/*
 * Recursively sorts the List using the Merge-sort algorithm.
 * Separates the list into sizes of one, then combines sorted
 * sub-lists back together into a single sorted list.
 * The `compare` function must be defined to call this function.
 * Θ(n * log(n))
 */
void list_merge_sort(LinkedList* const list)
{
    io_assert(list != NULL, IO_MSG_NULL_PTR);
    io_assert(list->compare != NULL, IO_MSG_NOT_SUPPORTED);

    if (list->size <= 1) return;

    /* Create two sub-lists of the main List. */
    LinkedList* const left = LinkedList_new(list->compare, list->toString),
            *const right = LinkedList_new(list->compare, list->toString);
    list_separate(list, left, right);

    /* Recursively sort the sub-lists. */
    list_merge_sort(left);
    list_merge_sort(right);

    while (left->size > 0 && right->size > 0)
        /* Place the lowest element from each sub-list back into the main List. */
        list_push_back(list, list->compare(list_front(left), list_front(right)) 
                             <= 0 ? list_pull_front(left) : list_pull_front(right));

    /* Dump any remaining elements back into the List. */
    LinkedList* const lists[] = { left, right };
    for (int i = 0; i < 2; i++)
        while (lists[i]->size > 0)
            list_push_back(list, list_pull_front(lists[i]));

    list_destroy(left);
    list_destroy(right);
}

/*
 * Recursively shuffles the elements inside the List pseudo-randomly.
 * The algorithm mirrors merge sort, except that merging is performed randomly.
 * For the Random implementation, see: C-Random
 * Θ(n * log(n))
 */
static void list_anti_merge_sort(LinkedList* const list)
{
    io_assert(list != NULL, IO_MSG_NULL_PTR);

    if (list->size <= 1) return;

    /* Create two sub-lists of the main List. */
    LinkedList* const left = LinkedList_new(list->compare, list->toString),
            *const right = LinkedList_new(list->compare, list->toString);
    list_separate(list, left, right);

    /* Recursively shuffle the sub-lists. */
    list_anti_merge_sort(left);
    list_anti_merge_sort(right);

    /*
     * In order to ensure random distribution, both lists must be the same size.
     * If size of the original list is not 2^n, then list size of 1 and 2 will occur.
     * In that situation, we need to place a dummy inside the List to maintain
     * random distribution. We must also delete the dummy list_Node after merging.
     */
    void *dummy = NULL;
    if (list_size(left) > list_size(right))
    {
        /* Make Dummy non-NULL so we can add it into the List. */
        dummy = &dummy;
        const unsigned int dummy_loc = rand_limit(right->size + 1);
        list_insert(right, dummy_loc, dummy);
    }

    /* Merge the two lists back together randomly. */
    while (left->size > 0 && right->size > 0)
        /* Flip a coin and merge based on the result of the toss. */
        list_push_back(list, rand_bool() ?
                             list_pull_front(left) : list_pull_front(right));

    /* Dump any remaining elements back into the List. */
    LinkedList* const lists[] = { left, right };
    for (int i = 0; i < 2; i++)
        while (lists[i]->size > 0)
            list_push_back(list, list_pull_front(lists[i]));

    list_destroy(left);
    list_destroy(right);

    /* Remove the dummy variable if we needed one. */
    if (dummy != NULL) list_remove(list, dummy);
}

/*
 * Empties a List evenly into two sub-lists.
 * Θ(n)
 */
void list_separate(LinkedList* const to_be_emptied, LinkedList* const l1, LinkedList* const l2)
{
    io_assert(to_be_emptied != NULL, IO_MSG_NULL_PTR);
    io_assert(l1 != NULL, IO_MSG_NULL_PTR);
    io_assert(l2 != NULL, IO_MSG_NULL_PTR);

    LinkedList* const lists[] = { l1, l2 };
    for (int toggle = 0; to_be_emptied->size > 0; toggle %= 2)
        list_push_back(lists[toggle++], list_pull_front(to_be_emptied));
}

/*
 * Pops an element from the front of the List and returns it.
 * Θ(1)
 */
void* list_pull_front(LinkedList *const list)
{
    void* const data = list_front(list);
    list_pop_front(list);
    return data;
}

/*
* Pops an element from the back of the List and returns it.
* Θ(1)
*/
void* list_pull_back(LinkedList *const list)
{
    void* const data = list_back(list);
    list_pop_back(list);
    return data;
}
