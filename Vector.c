
/*
 * File: Vector.c
 * Date: Feb 27, 2017
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

#include "Vector.h"

/* Array capacity components. */
#define DEFAULT_INITIAL_CAPACITY 10
#define VECTOR_GROW_AMOUNT 2

#define INDEX_RIGHT(index, capacity) (index == capacity - 1) ? 0 : index + 1
#define INDEX_LEFT(index, capacity) (index == 0) ? capacity - 1 : index - 1

/* Vector structure. */
struct Vector
{
    const void** table;
    /* Start and end let us know where the data is. */
    unsigned int start, end;
    size_t size, capacity;

    /* Synchronization. */
    ReadWriteSync *rw_sync;

    /* Function pointers. */
    int(*compare)(const void*, const void*);
    char*(*toString)(const void*);
};

/* Structure to assist in looping through Vector. */
struct vect_Iterator
{
    /* Keep track of where we are inside the Vector. */
    unsigned int index, *bearing;
    /* Reference to the Vector that it is iterating through. */
    const Vector *ref;
};

/* Local functions. */
static bool vect_full(const Vector* const vect);
static void vect_grow(Vector* const vect);
static void vect_swap(const Vector* const vect, const unsigned int i, const unsigned int h);
static void vect_pswap(const void **const v1, const void **const v2);
static void vect_merge_sort(const Vector* const vect, const unsigned int start, const size_t size);
static void vect_quick_sort(const Vector* const vect, const unsigned int index, const size_t size);
static void vect_shift(Vector* const vect, const unsigned int start, const unsigned int stop, const bool leftwards);
static unsigned int vect_backend_index(const Vector *const vect, const unsigned int index);

/*
 * Constructor function.
 * Θ(1)
 */
Vector* Vector_new(int(*compare)(const void*, const void*), char*(*toString)(const void*))
{
    Vector* const vect = mem_calloc(1, sizeof(Vector));
    vect->table = mem_calloc(DEFAULT_INITIAL_CAPACITY, sizeof(void*));
    vect->capacity = DEFAULT_INITIAL_CAPACITY;
    vect->compare = compare;
    vect->toString = toString;
    vect->rw_sync = ReadWriteSync_new();
    return vect;
}

/*
 * Returns the element at the specified index.
 * Θ(1)
 */
void* vect_at(const Vector* const vect, const unsigned int index)
{
    io_assert(vect != NULL, IO_MSG_NULL_PTR);

    /* Lock the data structure to future writers. */
    sync_read_start(vect->rw_sync);

    io_assert(index < vect->size, IO_MSG_OUT_OF_BOUNDS);

    /* Wrap around the table if the index exceeds the capacity. */
    const void* const val = vect->table[vect_backend_index(vect, index)];

    /* Unlock the data structure. */
    sync_read_end(vect->rw_sync);

    return (void*)val;
}

/*
 * Returns the element at the front of the Vector.
 * Θ(1)
 */
void* vect_front(const Vector* const vect)
{
    io_assert(vect != NULL, IO_MSG_NULL_PTR);

    /* Lock the data structure to future writers. */
    sync_read_start(vect->rw_sync);

    io_assert(vect->size > 0, IO_MSG_EMPTY);
    const void* const val = vect->table[vect->start];

    /* Unlock the data structure. */
    sync_read_end(vect->rw_sync);

    return (void*)val;
}

/*
 * Returns the element at the back of the Vector.
 * Θ(1)
 */
void* vect_back(const Vector* const vect)
{
    io_assert(vect != NULL, IO_MSG_NULL_PTR);

    /* Lock the data structure to future writers. */
    sync_read_start(vect->rw_sync);

    io_assert(vect->size > 0, IO_MSG_EMPTY);
    const void* const val = vect->table[vect->end];

    /* Unlock the data structure. */
    sync_read_end(vect->rw_sync);

    return (void*)val;
}

/*
 * Returns the number of elements in the Vector.
 * Θ(1)
 */
size_t vect_size(const Vector* const vect)
{
    io_assert(vect != NULL, IO_MSG_NULL_PTR);

    /* Lock the data structure to future writers. */
    sync_read_start(vect->rw_sync);

    const size_t size = vect->size;

    /* Unlock the data structure. */
    sync_read_end(vect->rw_sync);

    return size;
}

/*
 * Returns true if the Vector is empty.
 * Θ(1)
 */
bool vect_empty(const Vector* const vect)
{
    io_assert(vect != NULL, IO_MSG_NULL_PTR);

    /* Lock the data structure to future writers. */
    sync_read_start(vect->rw_sync);

    const bool empty = vect->size == 0;

    /* Unlock the data structure. */
    sync_read_end(vect->rw_sync);

    return empty;
}

/*
 * Returns the index in the Vector of the first occurrence of the specified element.
 * Returns false if no such element is found in the Vector.
 * The index parameter should be non-NULL and point to valid memory space.
 * The `compare` function must be defined to call this function.
 * Θ(n)
 */
bool vect_index(const Vector* const vect, const void* const data, unsigned int* const index)
{
    io_assert(vect != NULL, IO_MSG_NULL_PTR);
    io_assert(data != NULL, IO_MSG_NULL_PTR);
    io_assert(index != NULL, IO_MSG_NULL_PTR);
    io_assert(vect->compare != NULL, IO_MSG_NOT_SUPPORTED);

    bool found = false;

    /* Lock the data structure to future writers. */
    sync_read_start(vect->rw_sync);

    vect_Iterator* const iter = vect_iter(vect, 0);
    while (vect_iter_has_next(iter))
    {
        const void* const iterated = vect_iter_next(iter);
        if (iterated == data || vect->compare(iterated, data) == 0)
        {
            found = true;
            // TODO: Ensure that iter->index is always correct.
            *index = iter->index;
            break;
        }
    }

    /* Unlock the data structure. */
    sync_read_end(vect->rw_sync);

    vect_iter_destroy(iter);

    return found;
}

/*
 * Returns true if the Vector contains the specified element.
 * The `compare` function must be defined to call this function.
 * Θ(n)
 */
bool vect_contains(const Vector* const vect, const void* const data)
{
    io_assert(vect != NULL, IO_MSG_NULL_PTR);
    io_assert(data != NULL, IO_MSG_NULL_PTR);
    io_assert(vect->compare != NULL, IO_MSG_NOT_SUPPORTED);

    /* Lock the data structure to future writers. */
    sync_read_start(vect->rw_sync);

    unsigned int temp;
    const bool located = vect_index(vect, data, &temp);

    /* Unlock the data structure. */
    sync_read_end(vect->rw_sync);

    return located;
}

/*
 * Prints out the contents of the Vector to the console window.
 * The `toString` function must be defined to call this function.
 * Θ(n)
 */
void vect_print(const Vector* const vect)
{
    io_assert(vect != NULL, IO_MSG_NULL_PTR);
    io_assert(vect->toString != NULL, IO_MSG_NOT_SUPPORTED);

    /* Lock the data structure to future writers. */
    sync_read_start(vect->rw_sync);

    vect_Iterator* const iter = vect_iter(vect, 0);
    printf("%c", '[');
    while (vect_iter_has_next(iter))
    {
        char* value = vect->toString(vect_iter_next(iter));
        printf("%s%s", value, vect_iter_has_next(iter) ? ", " : "");
    }
    printf("]\n");

    /* Unlock the data structure. */
    sync_read_end(vect->rw_sync);

    vect_iter_destroy(iter);
}

/*
 * Returns a shallow copy of the Vector.
 * Θ(n)
 */
Vector* vect_clone(const Vector* const vect)
{
    io_assert(vect != NULL, IO_MSG_NULL_PTR);

    Vector* const copy = Vector_new(vect->compare, vect->toString);
    vect_append(copy, vect);

    /* Unlock the data structure. */
    sync_read_end(vect->rw_sync);

    return copy;
}

/*
 * Replaces an element in the Vector at a specified index.
 * Θ(1)
 */
void vect_assign(const Vector* const vect, const unsigned int index, const void* const data)
{
    io_assert(vect != NULL, IO_MSG_NULL_PTR);
    io_assert(data != NULL, IO_MSG_NULL_PTR);

    /* Lock the data structure to future readers/writers. */
    sync_write_start(vect->rw_sync);

    io_assert(index < vect->size, IO_MSG_OUT_OF_BOUNDS);

    vect->table[vect_backend_index(vect, index)] = data;

    /* Unlock the data structure. */
    sync_write_end(vect->rw_sync);
}

/*
 * Inserts an element at the specified index in the Vector.
 * Ω(1), O(n)
 */
void vect_insert(Vector* const vect, const unsigned int index, const void* const data)
{
    io_assert(vect != NULL, IO_MSG_NULL_PTR);
    io_assert(data != NULL, IO_MSG_NULL_PTR);

    /* Lock the data structure to future readers/writers. */
    sync_write_start(vect->rw_sync);

    io_assert(index <= vect->size, IO_MSG_OUT_OF_BOUNDS);

    if (index == 0)
        vect_push_front(vect, data);
    else if (index == vect->size)
        vect_push_back(vect, data);
    else
    {
        if (vect_full(vect))
            vect_grow(vect);

        /* Check if shifting right is quicker. */
        if (vect->size - 1 - index <= index)
        {
            vect->end = INDEX_RIGHT(vect->end, vect->capacity);
            vect_shift(vect, vect_backend_index(vect, index), vect->end, false);
        }
        else
        {
            vect->start = INDEX_LEFT(vect->start, vect->capacity);
            vect_shift(vect, vect_backend_index(vect, index), vect->start, true);
        }

        vect->size++;
        vect_assign(vect, index, data);
    }

    /* Unlock the data structure. */
    sync_write_end(vect->rw_sync);
}

/*
 * Removes an element from the Vector and returns true if the removal was successful.
 * The `compare` function must be defined to call this function.
 * Θ(n)
 */
bool vect_remove(Vector* const vect, const void* const data)
{
    io_assert(vect != NULL, IO_MSG_NULL_PTR);
    io_assert(data != NULL, IO_MSG_NULL_PTR);
    io_assert(vect->compare != NULL, IO_MSG_NOT_SUPPORTED);

    /* Lock the data structure to future readers/writers. */
    sync_write_start(vect->rw_sync);

    unsigned int index;
    bool success;
    if ((success = vect_index(vect, data, &index)))
        vect_erase(vect, index);

    /* Unlock the data structure. */
    sync_write_end(vect->rw_sync);

    return success;
}

/*
 * Removes an element from the Vector at a specified index.
 * Ω(1), O(n)
 */
void vect_erase(Vector* const vect, const unsigned int index)
{
    io_assert(vect != NULL, IO_MSG_NULL_PTR);

    /* Lock the data structure to future readers/writers. */
    sync_write_start(vect->rw_sync);

    io_assert(index < vect->size, IO_MSG_OUT_OF_BOUNDS);

    if (index == 0)
        vect_pop_front(vect);
    else if (index == vect->size - 1)
        vect_pop_back(vect);
    else
    {
        /* Check if shifting left is quicker. */
        if (vect->size - 1 - index <= index)
        {
            vect_shift(vect, vect->end, vect_backend_index(vect, index), true);
            vect->end = INDEX_LEFT(vect->end, vect->capacity);
        }
        else
        {
            vect_shift(vect, vect->start, vect_backend_index(vect, index), false);
            vect->start = INDEX_RIGHT(vect->start, vect->capacity);
        }

        vect->size--;
    }

    /* Unlock the data structure. */
    sync_write_end(vect->rw_sync);
}

/*
 * Appends an element at the end of the Vector.
 * Ω(1), O(n)
 */
void vect_push_back(Vector * const vect, const void* const data)
{
    io_assert(vect != NULL, IO_MSG_NULL_PTR);
    io_assert(data != NULL, IO_MSG_NULL_PTR);

    /* Lock the data structure to future readers/writers. */
    sync_write_start(vect->rw_sync);

    /* Check if we need to increase the array's capacity. */
    if (vect_full(vect))
        vect_grow(vect);

    /* When Vector has one or less element(s), start and end must point to the same index. */
    if (!vect_empty(vect))
        /* Increment end and wrap. */
        vect->end = INDEX_RIGHT(vect->end, vect->capacity);

    vect->table[vect->end] = data;
    vect->size++;

    /* Unlock the data structure. */
    sync_write_end(vect->rw_sync);
}

/*
 * Inserts an element at the front of the Vector.
 * Ω(1), O(n)
 */
void vect_push_front(Vector* const vect, const void* const data)
{
    io_assert(vect != NULL, IO_MSG_NULL_PTR);
    io_assert(data != NULL, IO_MSG_NULL_PTR);

    /* Lock the data structure to future readers/writers. */
    sync_write_start(vect->rw_sync);

    /* Check if we need to increase the array's capacity. */
    if (vect_full(vect))
        vect_grow(vect);

    /* When Vector has one or less element(s), start and end must point to the same index. */
    if (!vect_empty(vect))
        /* Increment end and wrap. */
        vect->start = INDEX_LEFT(vect->start, vect->capacity);

    vect->table[vect->start] = data;
    vect->size++;

    /* Unlock the data structure. */
    sync_write_end(vect->rw_sync);
}

/*
 * Removes the element at the end of the Vector.
 * Θ(1)
 */
void vect_pop_back(Vector* const vect)
{
    io_assert(vect != NULL, IO_MSG_NULL_PTR);

    /* Lock the data structure to future readers/writers. */
    sync_write_start(vect->rw_sync);

    io_assert(vect->size > 0, IO_MSG_EMPTY);

    /* When Vector has one or less element(s), start and end must point to the same index. */
    if (vect->size-- > 1)
        vect->end = INDEX_LEFT(vect->end, vect->capacity);

    /* Unlock the data structure. */
    sync_write_end(vect->rw_sync);
}

/*
* Removes the element at the front of the Vector.
* Θ(1)
*/
void vect_pop_front(Vector* const vect)
{
    io_assert(vect != NULL, IO_MSG_NULL_PTR);

    /* Lock the data structure to future readers/writers. */
    sync_write_start(vect->rw_sync);

    io_assert(vect->size > 0, IO_MSG_EMPTY);

    /* When Vector has one or less element(s), start and end must point to the same index. */
    if (vect->size-- > 1)
        vect->start = INDEX_RIGHT(vect->start, vect->capacity);

    /* Unlock the data structure. */
    sync_write_end(vect->rw_sync);
}

/*
 * Appends all data from another Vector to the end of this Vector.
 * Θ(n)
 */
void vect_append(Vector* const vect, const Vector* const other)
{
    io_assert(vect != NULL, IO_MSG_NULL_PTR);
    io_assert(other != NULL, IO_MSG_NULL_PTR);

    /* Lock the data structure to future readers/writers. */
    sync_write_start(vect->rw_sync);
    /* Lock the other data structure to future writers. */
    sync_read_start(other->rw_sync);

    const size_t combined = vect->size + other->size;
    if (vect->capacity < combined)
        vect_grow_to(vect, combined);

    vect_Iterator* const iter = vect_iter(other, 0);
    while (vect_iter_has_next(iter))
        vect_push_back(vect, vect_iter_next(iter));
    vect_iter_destroy(iter);

    /* Unlock the data structure. */
    sync_write_end(vect->rw_sync);
    /* Unlock the other data structure. */
    sync_read_end(other->rw_sync);
}

/*
 * Grows the underlying array to be able to store at least `min_size` elements.
 * The Vector's capacity will always be of the form `capacity = 10 * 2^n`
 * Θ(n)
 */
void vect_grow_to(Vector* const vect, const size_t min_size)
{
    io_assert(vect != NULL, IO_MSG_NULL_PTR);

    /* Lock the data structure to future readers/writers. */
    sync_write_start(vect->rw_sync);

    io_assert(min_size > vect->capacity, IO_MSG_INVALID_SIZE);

    /*
     * Solve for the expanded capacity.
     * The Vector only expands in increments of base_capacity * 2^n.
     * Ex. `min_size` of 73 means expanded capacity of 80 if base_capacity is 10.
     */
    const size_t expanded_capacity = DEFAULT_INITIAL_CAPACITY
                                     * (unsigned int)math_pow(VECTOR_GROW_AMOUNT, 1 + (unsigned int)floor(
            log((double)min_size / DEFAULT_INITIAL_CAPACITY) / log(VECTOR_GROW_AMOUNT)));

    /* Create a larger table and add the old table's data into it. */
    const void** const expanded_table = mem_calloc(expanded_capacity, sizeof(void*));
    for (unsigned int i = 0; i < vect->size; i++)
        expanded_table[i] = vect_at(vect, i);

    /* Destroy the old table. */
    mem_free(vect->table, vect->capacity * sizeof(void*));
    /* Update the Vector's properties. */
    vect->table = expanded_table;
    vect->capacity = expanded_capacity;
    vect->start = 0;
    vect->end = !vect_empty(vect) ? vect->size - 1 : 0;

    /* Unlock the data structure. */
    sync_write_end(vect->rw_sync);
}

/*
 * Removes all elements from the Vector while preserving the capacity.
 * Θ(1)
 */
void vect_clear(Vector* const vect)
{
    io_assert(vect != NULL, IO_MSG_NULL_PTR);

    /* Lock the data structure to future readers/writers. */
    sync_write_start(vect->rw_sync);

    vect->start = vect->end = vect->size = 0;

    /* Unlock the data structure. */
    sync_write_end(vect->rw_sync);
}

/*
 * Sorts elements inside the Vector in ascending order.
 * Uses the `compare` function specified to the Vector.
 * See: `vect_quick_sort`
 */
void vect_sort(const Vector* const vect)
{
    io_assert(vect != NULL, IO_MSG_NULL_PTR);

    /* Lock the data structure to future readers/writers. */
    sync_write_start(vect->rw_sync);

    vect_quick_sort(vect, 0, vect->size);

    /* Unlock the data structure. */
    sync_write_end(vect->rw_sync);
}

/*
 * Shuffles the elements inside the Vector pseudo-randomly.
 * Utilizes the Fisher-Yates Shuffling Algorithm:
 * (https://en.wikipedia.org/wiki/Fisher%E2%80%93Yates_shuffle)
 * Θ(n)
 */
void vect_shuffle(const Vector* const vect)
{
    io_assert(vect != NULL, IO_MSG_NULL_PTR);

    /* Lock the data structure to future readers/writers. */
    sync_write_start(vect->rw_sync);

    for (unsigned int i = vect->size - 1; i > 0; i--)
    {
        const unsigned int swap_location = rand_limit(i + 1);
        vect_swap(vect, i, swap_location);
    }

    /* Unlock the data structure. */
    sync_write_end(vect->rw_sync);
}

/*
 * De-constructor function.
 * Θ(1)
 */
void vect_destroy(Vector* const vect)
{
    io_assert(vect != NULL, IO_MSG_NULL_PTR);

    mem_free(vect->table, vect->capacity * sizeof(void*));
    sync_destroy(vect->rw_sync);
    mem_free(vect, sizeof(Vector));
}

/*
 * Constructor function.
 * Θ(1)
 */
vect_Iterator* vect_iter(const Vector* const vect, const unsigned int index)
{
    io_assert(vect != NULL, IO_MSG_NULL_PTR);
    io_assert(index < vect->size, IO_MSG_OUT_OF_BOUNDS);

    vect_Iterator* const iter = mem_calloc(1, sizeof(vect_Iterator));

    iter->ref = vect;
    iter->index = vect_backend_index(vect, index);

    if (index == 0)
        iter->bearing = &((Vector*)vect)->start;
    else if (index == vect->size - 1)
        iter->bearing = &((Vector*)vect)->end;

    return iter;
}

/*
 * Returns the iterator's current element and advances it forward.
 * Θ(1)
 */
void* vect_iter_next(vect_Iterator* const iter)
{
    io_assert(iter != NULL, IO_MSG_NULL_PTR);
    io_assert(vect_iter_has_next(iter), IO_MSG_OUT_OF_BOUNDS);

    const void* const val = iter->ref->table[iter->index];
    Vector* const vect = (Vector*)iter->ref;

    if (iter->index != vect->end)
    {
        iter->index = INDEX_RIGHT(iter->index, vect->capacity);
        iter->bearing = NULL;
    }
    else iter->bearing = &vect->end;

    return (void*)val;
}

/*
 * Returns the iterator's current element and retracts it backward.
 * Θ(1)
 */
void* vect_iter_prev(vect_Iterator* const iter)
{
    io_assert(iter != NULL, IO_MSG_NULL_PTR);
    io_assert(vect_iter_has_prev(iter), IO_MSG_OUT_OF_BOUNDS);

    const void* const val = iter->ref->table[iter->index];
    Vector* const vect = (Vector*)iter->ref;

    if (iter->index != vect->start)
    {
        iter->index = INDEX_LEFT(iter->index, vect->capacity);
        iter->bearing = NULL;
    }
    else iter->bearing = &vect->start;

    return (void*)val;
}

/*
 * Returns true if the iterator has a next element.
 * Θ(1)
 */
bool vect_iter_has_next(const vect_Iterator* const iter)
{
    io_assert(iter != NULL, IO_MSG_NULL_PTR);
    /* Start and End can be on the same index, so an extra check is needed. */
    return iter->index != iter->ref->end || iter->bearing != &iter->ref->end;
}

/*
 * Returns true if the iterator has a previous element.
 * Θ(1)
*/
bool vect_iter_has_prev(const vect_Iterator* const iter)
{
    io_assert(iter != NULL, IO_MSG_NULL_PTR);
    /* Start and End can be on the same index, so an extra check is needed. */
    return iter->index != iter->ref->start || iter->bearing != &iter->ref->start;
}

/*
 * De-constructor function.
 * Θ(1)
 */
void vect_iter_destroy(vect_Iterator* const iter)
{
    io_assert(iter != NULL, IO_MSG_NULL_PTR);
    mem_free(iter, sizeof(vect_Iterator));
}

/*
 * Returns true if the Vector is full.
 * Θ(1)
 */
bool vect_full(const Vector* const vect)
{
    return vect->size == vect->capacity;
}

/*
 * Grows the underlying array by a factor of `VECTOR_GROW_AMOUNT`.
 * Θ(n)
 */
void vect_grow(Vector* const vect)
{
    /* Expand the underlying array by a set amount. */
    vect_grow_to(vect, vect->capacity * VECTOR_GROW_AMOUNT);
}

/*
* Swap function for sorting and shifting algorithms.
* Θ(1)
*/
void vect_swap(const Vector* const vect, const unsigned int i, const unsigned int h)
{
    const void* const temp = vect_at(vect, i);
    vect_assign(vect, i, vect_at(vect, h));
    vect_assign(vect, h, temp);
}

/*
* Swap function for sorting and shifting algorithms.
* Θ(1)
*/
void vect_pswap(const void **const v1, const void **const v2)
{
    const void* const temp = *v1;
    *v1 = *v2;
    *v2 = temp;
}

/*
 * Converts regular indexes from 0->(capacity - 1) into indexes that wrap around the Vector.
 * Θ(1)
 */
unsigned int vect_backend_index(const Vector *const vect, const unsigned int index)
{
    return (vect->start + index) % vect->capacity;
}

/*
 * Sorts the Vector in ascending order using the Quicksort algorithm.
 * Ω(n * log(n)), O(n^2)
 */
void vect_quick_sort(const Vector* const vect, const unsigned int index, const size_t size)
{
    /* Array size of 1 means the partition is sorted. */
    if (size <= 1)
        return;
    /* Pivot in this implementation is the right element. */
    const unsigned int pivot_index = index + size - 1;
    const void* const pivot = vect_at(vect, pivot_index);

    /* Left and right iterators. */
    unsigned int left = index, right = pivot_index;

    while (true)
    {
        /* Move the indexes until they cross OR find swappable values. */
        while (left < right && vect->compare(vect_at(vect, left), pivot) < 0)
            left++;
        while (left < right && vect->compare(vect_at(vect, --right), pivot) > 0);

        if (left >= right)
            break;

        vect_swap(vect, left, right);
        left++;
    }

    vect_swap(vect, left, pivot_index);
    vect_quick_sort(vect, index, left - index);
    vect_quick_sort(vect, left + 1, index + size - left - 1);
}

/*
 * Sorts the Vector in ascending order using the Mergesort Algorithm.
 * An addition N space of memory will be needed for the merge step.
 * Θ(n * log(n))
 */
void vect_merge_sort(const Vector* const vect, const unsigned int start, const size_t size)
{
    /* Base case. */
    if (size <= 1)
        return;

    /* Split the Vector into half using integer pointers. */
    const size_t size_right = size / 2;
    const size_t size_left = size - size_right;
    const unsigned int start_right = start + size_left;

    /* Sort the left and right sub-arrays recursively. */
    vect_merge_sort(vect, start, size_left);
    vect_merge_sort(vect, start_right, size_right);

    /* Create two sub-arrays to assist in the merge process. */
    const void** const arr_left = mem_calloc(size_left, sizeof(void*));
    const void** const arr_right = mem_calloc(size_right, sizeof(void*));
    for (unsigned int i = 0; i < size_left; i++)
        arr_left[i] = vect_at(vect, start + i);
    for (unsigned int i = 0; i < size_right; i++)
        arr_right[i] = vect_at(vect, start_right + i);

    /* Maintain track of an iterator for the combined array and the two sub-arrays. */
    unsigned int iter = start, vect_iter_left = 0, vect_iter_right = 0;

    /* Merge the two sub-arrays back into the primary array. */
    while (vect_iter_left < size_left && vect_iter_right < size_right)
        if (vect->compare(arr_left[vect_iter_left], arr_right[vect_iter_right]) <= 0)
            vect_assign(vect, iter++, arr_left[vect_iter_left++]);
        else
            vect_assign(vect, iter++, arr_right[vect_iter_right++]);
    while (vect_iter_left < size_left)
        vect_assign(vect, iter++, arr_left[vect_iter_left++]);
    while (vect_iter_right < size_right)
        vect_assign(vect, iter++, arr_right[vect_iter_right++]);

    /* Clean up memory and return the sorted array. */
    mem_free(arr_left, size_left * sizeof(void*));
    mem_free(arr_right, size_right * sizeof(void*));
}

/*
 * Shifts the elements in the Vector left or right based on `leftwards`.
 * The shift will begin from `start` and the furthest element will be moved to `stop`.
 * Shifts may leave holes in the array and will not update `start` and `end` pointers of the Vector.
 * Θ(n)
 */
void vect_shift(Vector* const vect, const unsigned int start, const unsigned int stop, const bool leftwards)
{
    /* Iterator will aid in safely looping through array. */
    vect_Iterator* const iter = vect_iter(vect, stop);
    /* Shifting left means iterating rightwards and vice versa.
     * Function pointer makes this process much easier to read. */
    void*(*iterate)(vect_Iterator* const);
    iterate = leftwards ? &vect_iter_next : &vect_iter_prev;

    /* Iterate and swap. */
    while (iter->index != start)
    {
        /* Swap the data with the iterator and its neighboring element. */
        const unsigned int a = iter->index;
        iterate(iter);
        vect_pswap(&vect->table[a], &vect->table[iter->index]);
    }

    vect_iter_destroy(iter);
}