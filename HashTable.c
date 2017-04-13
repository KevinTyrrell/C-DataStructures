
#include "HashTable.h"
#include <math.h>

#define DEFAULT_INITIAL_CAPACITY 16
#define LOAD_FACTOR 0.75f
#define GROW_FACTOR 2

/* HashTable structure. */
struct HashTable
{
	struct table_Node **buckets;
	size_t capacity, size;

	/* Function pointers. */
	bool(*equals)(const void*, const void*);
	unsigned int(*hash)(const void*);
	char*(*toString)(const struct table_Entry*);
};

/* Local typedefs. */
typedef struct table_Entry table_Entry;

/* Node structure. */
typedef struct table_Node
{
	struct table_Node *next;
	table_Entry *data;
} table_Node;

/* Local functions. */
static table_Entry* table_Entry_new(const void* const key, const void* value);
static table_Node* table_Node_new(table_Entry* const entry);
static table_Node** table_Array_new(const size_t size);
static table_Node* table_search(const HashTable* const table, const void* const key);
static void table_mutate(HashTable* const table, const size_t capacity);
static void table_Entry_destroy(table_Entry* const entry);
static void table_Node_destroy(table_Node* const node);
static void table_Array_destroy(HashTable* const table);
static unsigned int table_modulus(const unsigned int operand, const unsigned int power_of_two);
static size_t table_regulate_capacity(const size_t desired);
static bool table_full_load(const HashTable* const table);

/*
* Hash function. Must return a value larger than the capacity of the Map.
* Ideally, hash should convert unique keys into unique integers.
*
* Equals function. It's possible for two unique keys to hash to the same value.
* Due to this, an equals function is needed to distinguish different elements in the map.
*
* toString function. Converts a Key/Value entry into a string for the print function.
*/
HashTable* HashTable_new(
	unsigned int(*hash)(const void*),
		bool(*equals)(const void*, const void*), 
			char*(*toString)(const struct table_Entry*))
{
	HashTable* const table = ds_calloc(1, sizeof(HashTable));

	/* Function pointers. */
	table->hash = hash;
	table->equals = equals;
	table->toString = toString;

	/* Note: Capacity must be a power of 2 for modulus to work later on. */
	table->buckets = table_Array_new(DEFAULT_INITIAL_CAPACITY);
	table->capacity = DEFAULT_INITIAL_CAPACITY;
	return table;
}

/*
 * Returns the value of an entry that corresponds to the provided key.
 * If no key exists in the table, returns NULL.
 * The Hash and Equals functions must defined before calling this function.
 * See: table_search
 * Ω(1), O(n)
 */
void* table_get(const HashTable* const table, const void* const key)
{
	const table_Node* const located = table_search(table, key);
	return located != NULL ? (void*)located->data->value : NULL;
}

/*
 * Returns true if an entry with the provided key exists in the table.
 * The Hash and Equals functions must defined before calling this function.
 * See: table_search
 * Ω(1), O(n)
 */
bool table_contains(const HashTable* const table, const void* const key)
{
	return table_search(table, key) != NULL;
}

/*
 * Returns true if the table is empty.
 * Θ(1)
 */
bool table_empty(const HashTable* const table)
{
	return table_size(table) == 0;
}

/*
 * Returns the amount of entries inside the table.
 * Θ(1)
 */
size_t table_size(const HashTable* const table)
{
	return table->size;
}

/*
* Returns the capacity of the table.
* Θ(1)
*/
size_t table_capacity(const HashTable* const table)
{
	return table->capacity;
}

/*
 * Returns an array of all entries inside the table.
 * Remember to free the memory allocated by this dynamic array.
 * The array can be correctly freed by: sizeof(struct table_Entry) * size.
 * Θ(n)
 */
struct table_Entry** table_entries(const HashTable* const table)
{
	if (table_empty(table))
	{
		ds_error(DS_MSG_EMPTY);
		return NULL;
	}

	/* Keep track of how many elements we've seen. */
	unsigned int elements = 0,
		capacity = table_capacity(table),
		size = table_size(table);

	/* Array that will contain the entries. */
	table_Entry** arr = calloc(table_size(table), sizeof(table_Entry*));

	/* Keep searching until we've located every non-NULL bucket. */
	for (unsigned int i = 0; i < capacity && elements < size; i++)
	{
		const table_Node *iter = table->buckets[i];
		while (iter != NULL)
		{
			arr[elements++] = iter->data;
			iter = iter->next;
		}
	}

	return arr;
}

/*
* Returns a shallow copy of the table.
* The Hash and Equals functions must defined before calling this function.
* Θ(n)
*/
HashTable* table_clone(const HashTable* const table)
{
	/* Create a copy and grow its size to store the entries. */
	HashTable* const copy = HashTable_new(table->hash, table->equals, table->toString);
	table_grow(copy, table_capacity(table));

	/* Keep track of how many elements we've seen. */
	unsigned int elements = 0,
		capacity = table_capacity(table),
		size = table_size(table);

	/* Keep searching until we've located every non-NULL bucket. */
	for (unsigned int i = 0; i < capacity && elements < size; i++)
	{
		const table_Node *iter = table->buckets[i];
		while (iter != NULL)
		{
			/* Place a shallow reference of the Key/Value into the clone. */
			const table_Entry* const entry = iter->data;
			table_put(copy, entry->key, entry->value);
			iter = iter->next;
			elements++;
		}
	}

	return copy;
}

/*
 * Prints out all entries inside the table to the console window.
 * The toString function must defined before calling this function.
 * Θ(n)
 */
void table_print(const HashTable* const table)
{
	printf("{ ");

	/* Keep track of how many elements we've seen. */
	unsigned int elements = 0,
		capacity = table_capacity(table),
		size = table_size(table);

	/* Keep searching until we've located every non-NULL bucket. */
	for (unsigned int i = 0; i < capacity && elements < size; i++)
	{
		const table_Node *iter = table->buckets[i];
		while (iter != NULL)
		{
			/* Print out each individual entry. */
			printf("%s%s ", table->toString(iter->data), ++elements < size ? "," : "");
			iter = iter->next;
		}
	}

	printf("}\n");
}

/*
 * Returns the current collision rate in the table.
 * The return value will be between 0.0 (no collisions) and 1.0 (all collisions).
 * A strong Hash function will make collisions far less likely.
 * This function is provided to test the robustness of the Hash function.
 * Θ(n)
 */
float table_collision_rate(const HashTable* const table)
{
	/* A table with size of 0 or 1 cannot have collisions. */
	if (table_size(table) <= 1)
		return 0.0f;

	unsigned int counter = 0;
	const unsigned int capacity = table_capacity(table),
		size = table_size(table);

	/* Determine how many non-NULL buckets there are. */
	for (unsigned int i = 0; i < capacity; i++)
		if (table->buckets[i] != NULL)
			counter++;

	return 1 - (float)(counter - 1) / (size - 1);
}

/*
 * Places a Key/Value entry into the table.
 * The Hash and Equals functions must defined before calling this function.
 * The key provided must not be NULL.
 * The table will grow dynamically as more space is needed.
 * Ω(1), O(n)
 */
void table_put(HashTable* const table, const void* const key, const void* const value)
{
	if (key == NULL)
	{
		ds_error(DS_MSG_NULL_PTR);
		return;
	}

	/* If the table is at capacity, grow it dynamically. */
	if (table_full_load(table))
		table_grow(table, table_size(table) + 1);

	/* Hash the key and locate the bucket that will contain this entry. */
	const size_t index = table_modulus(table->hash(key), table_capacity(table));
	table_Node *bucket = table->buckets[index];

	/* No collision. */
	if (bucket == NULL)
		table->buckets[index] = table_Node_new(table_Entry_new(key, value));
	else
	{
		while (true)
		{
			/* Duplicate key -- Overwrite the old value. */
			if (table->equals(bucket->data->key, key))
			{
				bucket->data->value = value;
				return;
			}

			if (bucket->next != NULL)
				bucket = bucket->next;
			else break;
		}

		/* Append the value to the end of the bucket. */
		bucket->next = table_Node_new(table_Entry_new(key, value));
	}

	table->size++;
}

/*
 * Removes an entry from the table.
 * The Hash and Equals functions must defined before calling this function.
 * Ω(1), O(n)
 */
bool table_remove(HashTable* const table, const void* const key)
{
	if (table_empty(table))
	{
		ds_error(DS_MSG_EMPTY);
		return false;
	}

	/* Hash the key and locate the bucket that will contain this entry. */
	const size_t index = table_modulus(table->hash(key), table->capacity);
	table_Node *bucket = table->buckets[index];

	/* Not found. */
	if (bucket == NULL)
		return false;
	/* No collision. */
	if (bucket->next == NULL)
	{
		table_Node_destroy(bucket);
		table->buckets[index] = NULL;
	}
	else
	{
		while (!table->equals(bucket->next->data->key, key))
			bucket = bucket->next;
		table_Node_destroy(bucket->next);
		bucket->next = NULL;
	}

	table->size--;
	return true;
}

/*
 * Removes all entries from the table.
 * This function will not resize the underlying array capacity.
 * To completely reset the table to default capacity, either call table_clear
 * and table_shrink or destroy the current table and make a new one.
 * Θ(n)
 */
void table_clear(HashTable* const table)
{
	/* Keep track of how many elements we've seen. */
	unsigned int elements = 0,
		capacity = table_capacity(table),
		size = table_size(table);

	/* Keep searching until we've located every non-NULL bucket. */
	for (unsigned int i = 0; i < capacity && elements < size; i++)
	{
		table_Node *iter = table->buckets[i];
		while (iter != NULL)
		{
			table_Node *temp = iter;
			iter = iter->next;
			table_Node_destroy(temp);
			elements++;
		}
	}

	table->size = 0;
}

/*
 * Grows the underlying array of the table to a specified capacity.
 * All entries will be re-hashed and find new locations in the table.
 * Due to specific requirements of the table, it is only guaranteed that
 * the final capacity will be at least as large as the parameter.
 * The Hash and Equals functions must defined before calling this function.
 * See: vect_regulate_capacity.
 * Ω(n), O(n^2)
 */
void table_grow(HashTable* const table, const size_t capacity)
{
	table_mutate(table, capacity);
}

/*
 * Shrinks the underlying array of the table to conserve memory.
 * All entries will be re-hashed and find new locations in the table.
 * Due to specific requirements of the table, it is only guaranteed that
 * the final capacity will be at least as large as the current table size.
 * The Hash and Equals functions must defined before calling this function.
 * See: vect_regulate_capacity.
 * Ω(n), O(n^2)
 */
void table_shrink(HashTable* const table)
{
	table_mutate(table, table_size(table));
}

/*
 * De-constructor function.
 * See: table_Array_destroy
 * Θ(n)
 */
void table_destroy(HashTable* const table)
{
	table_Array_destroy(table);
	ds_free(table, sizeof(HashTable));
}

/*
 * Constructor function.
 * Θ(1)
 */
table_Entry* table_Entry_new(const void* const key, const void* value)
{
	table_Entry* const entry = ds_malloc(sizeof(table_Entry));
	entry->key = key;
	entry->value = value;
	return entry;
}

/*
 * Constructor function.
 * Θ(1)
 */
table_Node* table_Node_new(table_Entry* const entry)
{
	table_Node* const node = ds_calloc(1, sizeof(table_Node));
	node->data = entry;
	return node;
}

/*
 * Constructor function.
 * Θ(1)
 */
table_Node** table_Array_new(const size_t size)
{
	const table_Node** const buckets = ds_calloc(size, sizeof(table_Node*));
	return buckets;
}

/*
 * Returns a Node in the table whose entry's key matches the provided key.
 * If the key does not exist in the table, the return value of search will be NULL.
 * The Hash and Equals functions must defined before calling this function.
 * Ω(1), O(n)
 */
table_Node* table_search(const HashTable* const table, const void* const key)
{
	/* Hash the key and locate the correct bucket index. */
	const size_t index = table_modulus(table->hash(key), table->capacity);
	
	table_Node *iter = table->buckets[index];
	while (iter != NULL)
	{
		if (table->equals(iter->data->key, key))
			break;
		iter = iter->next;
	}

	return iter;
}

/*
 * See: table_grow
 * Ω(n), O(n^2)
 */
void table_mutate(HashTable* const table, const size_t capacity)
{
	/* Prepare the smaller bucket array to be transfered into the expanded version. */
	const size_t expanded_capacity = table_regulate_capacity(capacity),
		old_size = table_size(table), old_capacity = table_capacity(table);
	if (expanded_capacity == old_capacity) return;

	table_Node** const expanded = table_Array_new(expanded_capacity),
		**const old = table->buckets;

	/* Modify the table structure so it functions with the new bucket array. */
	table->buckets = expanded;
	table->capacity = expanded_capacity;
	table->size = 0;

	/* Keep searching until we've located every non-NULL bucket. */
	unsigned int elements = 0;
	for (unsigned int i = 0; i < old_capacity && elements < old_size; i++)
	{
		table_Node *iter = old[i];
		while (iter != NULL)
		{
			/* Place every entry from the old array to the new table. */
			table_put(table, iter->data->key, iter->data->value);
			table_Node* const temp = iter;
			iter = iter->next;
			table_Node_destroy(temp);
			elements++;
		}
	}

	/* Destroy the old bucket array. */
	ds_free(old, old_capacity * sizeof(table_Node*));
}

/*
 * De-constructor function.
 * Θ(1)
 */
void table_Entry_destroy(table_Entry* const entry)
{
	ds_free(entry, sizeof(table_Entry));
}

/*
 * De-constructor function.
 * Destroys the inner table_Entry as well as the Node.
 * Θ(1)
 */
void table_Node_destroy(table_Node* const node)
{
	table_Entry_destroy(node->data);
	ds_free(node, sizeof(table_Node));
}

/*
 * De-constructor function.
 * See: table_clear
 * Θ(n)
 */
void table_Array_destroy(HashTable* const table)
{
	table_clear(table);
	ds_free(table->buckets, table_capacity(table) * sizeof(table_Node*));
}

/*
 * Returns the result of the first value modulus by the second.
 * The second parameter MUST be a power of 2.
 * Modulus is performed with a bitwise AND.
 * Θ(1)
 */
unsigned int table_modulus(const unsigned int operand, const unsigned int power_of_two)
{
	return operand & (power_of_two - 1);
}

/*
 * Regulates the capacity passed in, returning one that is HashTable-safe.
 * The capacity of the table must be of the form: INIT_CAP * GROW^n.
 * In addition, the regulated capacity must be at least: desired / LOADF.
 * The returned value is guaranteed to be at least as large as the parameter.
 * Θ(1)
 */
size_t table_regulate_capacity(const size_t desired)
{
	/* Max prevents value from being zero if desired is zero. */
	return max(DEFAULT_INITIAL_CAPACITY,
		DEFAULT_INITIAL_CAPACITY * (size_t)ceil(pow((double)GROW_FACTOR,
			ceil(log((double)desired / (DEFAULT_INITIAL_CAPACITY * LOAD_FACTOR)) /
				log((double)GROW_FACTOR)))));
}

/*
 * Returns true if the table is at full load capacity.
 * Full load capacity is when the amount of elements in the table
 * exceeds the LOAD_FACTOR percentage of the capacity.
 * For example, size:8, capacity:10, LOAD_FACTOR: 3/4 is at full load.
 * Θ(1)
 */
bool table_full_load(const HashTable* const table)
{
	return table_size(table) / (double)table_capacity(table) >= LOAD_FACTOR;
}
