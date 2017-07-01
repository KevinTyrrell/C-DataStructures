
/*
 * File: HashTable.c
 * Date: Aug 18, 2016
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

#include "HashTable.h"
#include <math.h>

/* Array capacity components. */
#define DEFAULT_INITIAL_CAPACITY 16
#define LOAD_FACTOR 0.75f
#define GROW_FACTOR 2

/* Modulus of (a, b) where b is a positive base 2 integer. */
#define MODULUS(operand, base_2_num) (operand & (base_2_num - 1))
#define LOG_BX(num, base) (log((double)num)/log((double)base))
/* v = A_B2(num) where v >= num >= DIC and is a power of GF and factor of DIC. */
#define ATLEAST_BASE_2(num) ((unsigned int)pow((double)GROW_FACTOR,\
    ceil(max(LOG_BX(num, GROW_FACTOR), sqrt((double)DEFAULT_INITIAL_CAPACITY)))))

/* HashTable structure. */
struct HashTable
{
    struct table_Bucket **buckets;
    size_t capacity, size;

    /* Synchronization. */
    ReadWriteSync *rw_sync;

    /* Function pointers. */
    bool(*equals)(const void*, const void*);
    unsigned int(*hash)(const void*);
    char*(*toString)(const void*, const void*);
};

/* Bucket structure. */
typedef struct table_Bucket
{
    const void *key, *value;
    struct table_Bucket *next;
    unsigned int hash;
} table_Bucket;

/* Structure to assist in looping through Table. */
struct table_Iterator
{
    /* Keep track of where we are inside the Table. */
    unsigned int index;
    const table_Bucket *current;
    size_t visited;
    /* Reference to the Table that it is iterating through. */
    const HashTable *ref;
};

/* Local functions. */
static table_Bucket* table_Bucket_new(const void* const key, void* const value, const unsigned int hash);
static table_Bucket* table_iter_next_bucket(table_Iterator* const iter);
static table_Bucket* table_search(const HashTable* const table, const void* const key,
                                  const unsigned int hash, bool* const found);
static void table_Bucket_destroy(table_Bucket* const bucket);
static bool table_design_load(const HashTable* const table);
static bool table_Bucket_match(const table_Bucket* const bucket, const void* const key, const unsigned int hash,
                               bool(*equals)(const void*, const void*));

/*
 * Constructor function.
 * The `hash` function must be defined to call this function.
 * The `equals` function must be defined to call this function.
 * Θ(1)
 */
HashTable* HashTable_new(unsigned int(*hash)(const void*),
                         bool(*equals)(const void*, const void*),
                         char*(*toString)(const void*, const void*))
{
    io_assert(hash != NULL, IO_MSG_NOT_SUPPORTED);
    io_assert(equals != NULL, IO_MSG_NOT_SUPPORTED);

    HashTable* const table = mem_calloc(1, sizeof(HashTable));
    /* Note: Capacity must always be a power of 2. */
    table->buckets = mem_calloc(DEFAULT_INITIAL_CAPACITY, sizeof(table_Bucket*));
    table->capacity = DEFAULT_INITIAL_CAPACITY;
    table->hash = hash;
    table->equals = equals;
    table->toString = toString;
    table->rw_sync = ReadWriteSync_new();
    return table;
}

/*
 * Returns the value of a mapping whose key matches the specified key.
 * Returns NULL if no such mapping exists.
 * Ω(1), O(n)
 */
void* table_get(const HashTable* const table, const void* const key)
{
    io_assert(table != NULL, IO_MSG_NULL_PTR);
    io_assert(key != NULL, IO_MSG_NULL_PTR);

    const void *value = NULL;

    /* Lock the data structure to future writers. */
    sync_read_start(table->rw_sync);

    bool exists;
    const table_Bucket *bucket = table_search(table, key, table->hash(key), &exists);
    if (exists) value = bucket->value;

    /* Unlock the data structure. */
    sync_read_end(table->rw_sync);

    return (void*)value;
}

/*
 * Returns the number of mappings in the Table.
 * Θ(1)
 */
size_t table_size(const HashTable* const table)
{
    io_assert(table != NULL, IO_MSG_NULL_PTR);

    /* Lock the data structure to future writers. */
    sync_read_start(table->rw_sync);

    const size_t size = table->size;

    /* Unlock the data structure. */
    sync_read_end(table->rw_sync);

    return size;
}

/*
 * Returns true if the Table is empty.
 * Θ(1)
 */
bool table_empty(const HashTable* const table)
{
    io_assert(table != NULL, IO_MSG_NULL_PTR);

    /* Lock the data structure to future writers. */
    sync_read_start(table->rw_sync);

    const bool val = table->size == 0;

    /* Unlock the data structure. */
    sync_read_end(table->rw_sync);

    return val;
}

/*
 * Returns true if the Dictionary contains a mapping with the specified key.
 * Ω(1), O(n)
 */
bool table_contains(const HashTable* const table, const void* const key)
{
    io_assert(table != NULL, IO_MSG_NULL_PTR);
    io_assert(key != NULL, IO_MSG_NULL_PTR);

    /* Lock the data structure to future writers. */
    sync_read_start(table->rw_sync);

    bool exists;
    table_search(table, key, table->hash(key), &exists);

    /* Unlock the data structure. */
    sync_read_end(table->rw_sync);

    return exists;
}

/*
 * Prints out the contents of the Table to the console window.
 * Θ(n)
 */
void table_print(const HashTable* const table)
{
    io_assert(table != NULL, IO_MSG_NULL_PTR);

    /* Lock the data structure to future writers. */
    sync_read_start(table->rw_sync);

    table_Iterator* const iter = table_iter(table);
    printf("%c", '[');
    while (table_iter_has_next(iter))
    {
        void* value;
        printf("%s", table->toString(table_iter_next(iter, &value), value));
        if (table_iter_has_next(iter)) printf(", ");
    }
    printf("]\n");
    table_iter_destroy(iter);

    /* Unlock the data structure. */
    sync_read_end(table->rw_sync);
}

/*
 * Returns a shallow copy of the Table.
 * Θ(n)
 */
HashTable* table_clone(const HashTable* const table)
{
    io_assert(table != NULL, IO_MSG_NULL_PTR);

    HashTable* const copy = HashTable_new(table->hash, table->equals, table->toString);

    /* Lock the data structure to future writers. */
    sync_read_start(table->rw_sync);

    /* The new table needs to have the same capacity as the old one. */
    table_grow(copy, table->capacity);
    table_Iterator* const iter = table_iter(table);
    while (table_iter_has_next(iter))
    {
        void *value, *key = table_iter_next(iter, &value);
        table_put(copy, key, value);
    }
    table_iter_destroy(iter);

    /* Unlock the data structure. */
    sync_read_end(table->rw_sync);

    return copy;
}

/*
 * Inserts a mapping into the Table.
 * If the Table already contained a mapping for the key, the old value is replaced.
 * Returns the replaced value or NULL if this is a new mapping.
 * Ω(1), O(n)
 */
void* table_put(HashTable* const table, const void* const key, const void* const value)
{
    io_assert(table != NULL, IO_MSG_NULL_PTR);
    io_assert(key != NULL, IO_MSG_NULL_PTR);
    io_assert(value != NULL, IO_MSG_NULL_PTR);

    const void *replaced = NULL;
    const unsigned int hash = table->hash(key);

    /* Lock the data structure to future readers/writers. */
    sync_write_start(table->rw_sync);

    /* Expand the Table automatically if we are at design load. */
    if (table_design_load(table))
        table_grow(table, table->capacity * GROW_FACTOR);

    bool already_exists;
    table_Bucket* const located = table_search(table, key, hash, &already_exists);
    if (!already_exists)
    {
        table_Bucket* const inserted = table_Bucket_new(key, (void*)value, hash);
        /* Check if a collision occurred. */
        if (located != NULL)
            located->next = inserted;
        /* This is a new bucket, place it directly into the array. */
        else table->buckets[MODULUS(hash, table->capacity)] = inserted;

        table->size++;
    }
    /* Duplicate key entered; update the value. */
    else
    {
        replaced = located->value;
        located->value = value;
    }

    /* Unlock the data structure. */
    sync_write_end(table->rw_sync);

    return (void*)replaced;
}

/*
 * Removes a key/value pair from the Table and returns true if the removal was successful.
 * Ω(1), O(n)
 */
bool table_remove(HashTable* const table, const void* const key)
{
    io_assert(table != NULL, IO_MSG_NULL_PTR);
    io_assert(key != NULL, IO_MSG_NULL_PTR);

    const unsigned int hash = table->hash(key);
    bool removed = false;

    /* Lock the data structure to future readers/writers. */
    sync_write_start(table->rw_sync);

    /* Iterate over the bucket chain at the hashed index. */
    const unsigned int index = MODULUS(hash, table->capacity);
    table_Bucket *prev = NULL, *current = table->buckets[index];
    while (current != NULL)
    {
        removed = table_Bucket_match(current, key, hash, table->equals);
        if (removed)
        {
            /* Determine if this bucket is root of the chain. */
            if (prev != NULL)
                prev->next = current->next;
            else table->buckets[index] = NULL;
            table_Bucket_destroy(current);
            table->size--;
            break;
        }

        prev = current;
        current = current->next;
    }

    /* Unlock the data structure. */
    sync_write_end(table->rw_sync);

    return removed;
}

/*
 * Grows the underlying array to be able to store at least `min_size` elements.
 * The Table's new capacity will be the smallest power of 2 which is >= `min_size`.
 * The Table's capacity will always be a factor of DEFAULT_INITIAL_CAPACITY.
 * If the new capacity is not larger than the current capacity, no changes are made.
 * Ω(1), O(n)
 */
void table_grow(HashTable* const table, const size_t min_size)
{
    io_assert(table != NULL, IO_MSG_NULL_PTR);

    const size_t expanded_capacity = ATLEAST_BASE_2(min_size);

    /* Lock the data structure to future readers/writers. */
    sync_write_start(table->rw_sync);

    /* No need to expand if the table if there is no size improvement. */
    if (expanded_capacity > table->capacity)
    {
        /* Create a temporary Table on the Stack. */
        HashTable expanded =
        {
            mem_calloc(expanded_capacity, sizeof(table_Bucket*)), expanded_capacity, 0,
            table->rw_sync, table->equals, table->hash, table->toString
        };

        /* Re-hash all key/value pairs into the new Table's array. */
        table_Iterator* const iter = table_iter(table);
        while (table_iter_has_next(iter))
        {
            table_Bucket* const bucket = table_iter_next_bucket(iter);
            table_put(&expanded, bucket->key, bucket->value);
            table_Bucket_destroy(bucket);
        }
        table_iter_destroy(iter);

        /* Destroy the old array and replace it with the expanded one. */
        mem_free(table->buckets, table->capacity * sizeof(table_Bucket*));
        table->buckets = expanded.buckets;
        table->capacity = expanded_capacity;
        /* The expanded table was made on the stack, so no need to destroy it. */
    }

    /* Unlock the data structure. */
    sync_write_end(table->rw_sync);
}

/*
 * Removes all key/value pairs from the Table while preserving the capacity.
 * Θ(n)
 */
void table_clear(HashTable* const table)
{
    io_assert(table != NULL, IO_MSG_NULL_PTR);

    /* Lock the data structure to future readers/writers. */
    sync_write_start(table->rw_sync);

    table_Iterator* const iter = table_iter(table);
    while (table_iter_has_next(iter))
        table_Bucket_destroy(table_iter_next_bucket(iter));
    /* NULL out the memory inside the Table for future use. */
    memset(table->buckets, 0, sizeof(table_Bucket*) * table->capacity);
    table->size = 0;

    /* Unlock the data structure. */
    sync_write_end(table->rw_sync);

    table_iter_destroy(iter);
}

/*
 * De-constructor function.
 * Θ(n)
 */
void table_destroy(HashTable* const table)
{
    io_assert(table != NULL, IO_MSG_NULL_PTR);

    table_clear(table);
    mem_free(table->buckets, table->capacity * sizeof(table_Bucket*));
    sync_destroy(table->rw_sync);
    mem_free(table, sizeof(HashTable));
}

/*
 * Constructor function.
 * Θ(1)
 */
table_Iterator* table_iter(const HashTable* const table)
{
    io_assert(table != NULL, IO_MSG_NULL_PTR);

    table_Iterator* const iter = mem_calloc(1, sizeof(table_Iterator));

    iter->ref = table;
    return iter;
}

/*
 * Returns the iterator's current key/value pair and advances it forward.
 * The key will be returned and the value will be assigned to the data of the parameter.
 * Ω(1), O(n)
 */
void* table_iter_next(table_Iterator* const iter, void **value)
{
    io_assert(iter != NULL, IO_MSG_NULL_PTR);
    io_assert(table_iter_has_next(iter), IO_MSG_OUT_OF_BOUNDS);

    const table_Bucket* const current = table_iter_next_bucket(iter);
    *value = (void*)current->value;

    return (void*)current->key;
}

/*
 * Returns true if the iterator has a next key/value pair.
 * Ω(1), O(n)
 */
bool table_iter_has_next(const table_Iterator* const iter)
{
    io_assert(iter != NULL, IO_MSG_NULL_PTR);
    /* If we've visited all the pairs, there are no more to iterate over. */
    if (iter->visited >= iter->ref->size) return false;
    const table_Bucket *current = iter->current;
    for (unsigned int i = iter->index; current == NULL && i < iter->ref->capacity; i++)
        current = iter->ref->buckets[i];
    return current != NULL;
}

/*
 * De-constructor function.
 * Θ(1)
 */
void table_iter_destroy(table_Iterator* const iter)
{
    io_assert(iter != NULL, IO_MSG_NULL_PTR);
    mem_free(iter, sizeof(table_Iterator));
}

/*
 * Constructor function.
 * Θ(1)
 */
table_Bucket* table_Bucket_new(const void* const key, void* const value, const unsigned int hash)
{
    io_assert(key != NULL, IO_MSG_NULL_PTR);
    io_assert(value != NULL, IO_MSG_NULL_PTR);

    table_Bucket* const bucket = mem_calloc(1, sizeof(table_Bucket));
    bucket->key = key;
    bucket->value = value;
    bucket->hash = hash;
    return bucket;
}

/*
 * Returns the iterator's current bucket and advances it forward.
 * Ω(1), O(n)
 */
table_Bucket* table_iter_next_bucket(table_Iterator* const iter)
{
    io_assert(iter != NULL, IO_MSG_NULL_PTR);
    io_assert(table_iter_has_next(iter), IO_MSG_OUT_OF_BOUNDS);

    while (iter->current == NULL && iter->index < iter->ref->capacity)
        iter->current = iter->ref->buckets[iter->index++];

    const table_Bucket* const current = iter->current;
    iter->visited++;

    iter->current = iter->current->next;

    return (table_Bucket*)current;
}

/*
 * Returns a bucket in the Table whose hash matches the specified hash.
 * If no such bucket exists, either the closest bucket will be returned or NULL.
 * The parameter `found` will be set to true if an exact match is located.
 * Ω(1), O(n)
 */
table_Bucket* table_search(const HashTable* const table, const void* const key,
                           const unsigned int hash, bool* const found)
{
    io_assert(table != NULL, IO_MSG_NULL_PTR);
    io_assert(key != NULL, IO_MSG_NULL_PTR);
    io_assert(found != NULL, IO_MSG_NULL_PTR);

    *found = false;
    const table_Bucket *current = table->buckets[MODULUS(hash, table->capacity)];

    while (current != NULL)
    {
        // TODO: It may be possible to refactor this code.
        *found = table_Bucket_match(current, key, hash, table->equals);
        if (*found || current->next == NULL) break;
        current = current->next;
    }

    return (table_Bucket*)current;
}

/*
 * De-constructor function.
 * Θ(1)
 */
void table_Bucket_destroy(table_Bucket* const bucket)
{
   mem_free(bucket, sizeof(table_Bucket));
}

/*
 * Returns true if the Table is or is exceeding maximum design load.
 * Design load is when the ratio of elements to capacity exceeds the load factor.
 * Θ(1)
 */
static bool table_design_load(const HashTable* const table)
{
    return (double)table->size / table->capacity >= LOAD_FACTOR;
}

/*
 * Returns true if a Bucket matches a specified hash and key.
 * Θ(1)
 */
static bool table_Bucket_match(const table_Bucket* const bucket, const void* const key, const unsigned int hash,
                               bool(*equals)(const void*, const void*))
{
    io_assert(bucket != NULL, IO_MSG_NULL_PTR);
    io_assert(key != NULL, IO_MSG_NULL_PTR);
    io_assert(equals != NULL, IO_MSG_NOT_SUPPORTED);
    return bucket->hash == hash && (bucket->key == key || equals(key, bucket->key));
}
