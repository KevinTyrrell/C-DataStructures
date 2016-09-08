
#include "HashMap.h"

/* Static functions. */
static struct hm_Node* hm_Node_new(void *key, void *value);
// Note: capacity is a pointer so we can change its value in between functions.
static struct hm_Node** hm_Table_new(size_t *capacity);

/* Default capacity of the HashMap. */
const static size_t DEFAULT_INITIAL_CAPACITY = 16;
/* Load factor. Map expands when this percentage is reached. */
const static float LOAD_FACTOR = 0.75;
/* Gets the size of the HashMap depending on whether the NULL keyset is involved or not. */
static size_t hm_getSize(const struct HashMap *map, bool includingNull);
/* Locates a given Node in the HashMap, if it exists, by key. */
static struct hm_Node* hm_find(const struct HashMap *map, void* key);

/* Constructor function. */
struct HashMap* HashMap_new(size_t capacity, unsigned int(*hash)(void*),
	bool(*equals)(void*, void*), char*(*toString)(void*))
{
	struct HashMap *map = calloc(1, sizeof(struct HashMap));
	if (map == NULL)
		ds_Error(DS_MSG_OUT_OF_MEM);
	// Function pointers.
	map->hash = hash;
	map->equals = equals;
	map->toString = toString;

	map->table = hm_Table_new(&capacity);
	map->capacity = capacity;
	return map;
}

/* Constructor function. */
static struct hm_Node* hm_Node_new(void *key, void *value)
{
	struct hm_Node *entry = calloc(1, sizeof(struct hm_Node));
	if (entry == NULL)
		ds_Error(DS_MSG_OUT_OF_MEM);
	entry->key = key;
	entry->value = value;
	return entry;
}

/*
Constructor helper function.

Creates an array of Nodes.
*/
static struct hm_Node** hm_Table_new(size_t *capacity)
{
	/*
	Make sure the capacity is a power of 2.
	If capacity is not, the bitwise operator &
	will NOT work. Otherwise, just use a default capacity.
	*/
	double val = log((double)*capacity) / log(2.0);
	*capacity = (val - floor(val) < 1e-8) ? *capacity : DEFAULT_INITIAL_CAPACITY;

	struct hm_Node** table = calloc(*capacity, sizeof(struct Node*));
	if (table == NULL)
		ds_Error(DS_MSG_OUT_OF_MEM);
	return table;
}

/*
Helper function.
Locates a given Node in the HashMap, if it exists, by key.

Returns NULL if it cannot be located.
*/
static struct hm_Node* hm_find(const struct HashMap *map, void* key)
{
	// Null keys must be handled differently.
	if (key != NULL)
	{
		// Bitwise modulus.
		size_t index = map->hash(key) & (map->capacity - 1);

		for (struct hm_Node *iter = map->table[index]; iter != NULL; iter = iter->next)
			// Key matches this node, return the value associated with it.
			if (map->equals(iter->key, key))
				return iter;
	}
	else if (map->nullKeySet != NULL && map->equals(map->nullKeySet->key, key))
		return map->nullKeySet;

	return NULL;
}

/*
Place a Key/Value pair into the HashMap.

O(1) complexity, assuming load factor is not reached.
*/
void hm_put(struct HashMap *map, void *key, void *value)
{
	// Double the size of the HashMap if it is getting too full.
	if (hm_getSize(map, false) / (double)map->capacity >= LOAD_FACTOR)
	{
		// Temporary map that we will store our new table inside of.
		struct HashMap *expanded = HashMap_new(map->capacity * 2, map->hash, map->equals, map->toString);

		/* Go through the old table and put each key/value pair
		into the new table. Destroy the old nodes. */
		for (size_t i = 0; i < map->capacity; i++)
			for (struct hm_Node *iter = map->table[i], *prev = NULL; iter != NULL;)
			{
				hm_put(expanded, iter->key, iter->value);
				prev = iter;
				iter = iter->next;
				free(prev);
			}

		// Destroy the old table and replace it with the larger one.
		free(map->table);
		map->table = expanded->table;
		map->capacity = expanded->capacity;
		free(expanded);
	}

	// Map is correct size, proceed with insertion.
	struct hm_Node *insert = hm_Node_new(key, value);

	// Null keys are handled differently.
	// Make sure that we are not dealing with one.
	if (insert->key != NULL)
	{
		// Bitwise modulus.
		size_t index = map->hash(insert->key) & (map->capacity - 1);

		// No collision.
		if (map->table[index] == NULL)
			map->table[index] = insert;
		else
		{
			struct hm_Node *iter = map->table[index];
			for (; iter->next != NULL; iter = iter->next)
			{
				// Duplicate key was entered, overwrite old value.
				if (map->equals(iter->key, key))
				{
					iter->value = value;
					return;
				}
			}
			iter->next = insert;
		}

		map->size++;
	}
	else
	{
		// Overwrite old KeySet.
		if (map->nullKeySet != NULL)
			free(map->nullKeySet);
		else
			map->size++;
		map->nullKeySet = insert;
	}
}

/*
Gets a value from the HashMap when provided with a key.

O(1) complexity.
Returns NULL if no such key exists.
*/
void* hm_get(const struct HashMap *map, void *key)
{
	struct hm_Node *retrieved = hm_find(map, key);
	return retrieved != NULL ? retrieved->value : NULL;
}

/*
Removes a Key/Value pair from the HashMap when provided with a key.

O(1) complexity.
Returns the value of the removed Node, NULL if operation failed.
*/
void* hm_remove(struct HashMap *map, void *key)
{
	if (key != NULL)
	{
		// Bitwise modulus.
		size_t index = map->hash(key) & (map->capacity - 1);

		for (struct hm_Node *iter = map->table[index], *prev = NULL; iter != NULL; prev = iter, iter = iter->next)
			// Keys match. Delete this Node.
			if (map->equals(iter->key, key))
			{
				if (prev != NULL)
					prev->next = iter->next;
				else
					map->table[index] = iter->next;
				void *val = iter->value;
				free(iter);
				map->size--;
				return val;
			}
	}
	else if (map->nullKeySet != NULL && map->equals(map->nullKeySet->key, key))
	{
		void *val = map->nullKeySet->value;
		free(map->nullKeySet);
		map->nullKeySet = NULL;
		map->size--;
		return val;
	}

	return NULL;
}

/* Check if a given element in the HashMap exists. */
bool hm_contains(struct HashMap *map, void *key)
{
	return hm_find(map, key) != NULL;
}

/* Clear the HashMap of all Key/Value pairs. */
void hm_clear(struct HashMap *map)
{
	// Destroy the table.
	for (size_t i = 0; i < map->capacity; i++)
		for (struct hm_Node *iter = map->table[i], *temp = NULL; iter != NULL; iter = temp)
		{
			temp = iter->next;
			free(iter);
		}
	free(map->nullKeySet);
	free(map->table);
	size_t tableSize = 0;
	map->table = hm_Table_new(&tableSize);
	map->size = 0;
	map->capacity = DEFAULT_INITIAL_CAPACITY;	
}

/* 
Return an array with all Keys/Values currently in the HashMap.

Remember to free the returned array of hm_Nodes.
*/
struct hm_Node** hm_KeySet(const struct HashMap *map)
{
	// +1 to include the NULL terminating character.
	struct hm_Node **keySet = malloc((hm_getSize(map, true) + 1) * sizeof(struct hm_Node*));
	if (keySet == NULL)
		ds_Error(DS_MSG_OUT_OF_MEM);

	size_t h = 0;
	for (size_t i = 0; i < map->capacity; i++)
		for (struct hm_Node *iter = map->table[i]; iter != NULL; iter = iter->next)
			keySet[h++] = iter;
	if (map->nullKeySet != NULL)
		keySet[h++] = map->nullKeySet;
	// Terminating index.
	keySet[h] = NULL;
	return keySet;
}

/* Prints out the inner table of the HashMap. */
void hm_print(const struct HashMap *map)
{
	printf_s("Map Capacity: %zu, Map Size: %zu\n", map->capacity, map->size);
	if (map->nullKeySet != NULL)
		printf_s("%s%s\n", "[NULL]: ", map->toString(map->nullKeySet->value));
	for (size_t i = 0; i < map->capacity; i++)
	{
		printf_s("%c%zu%s", '[', i, "]: ");
		for (struct hm_Node *iter = map->table[i]; iter != NULL; iter = iter->next)
		{
			printf_s("%s", map->toString(iter->value));
			if (iter->next != NULL)
				printf_s("%s", " -> ");
		}
		printf_s("\n");
	}
}

/* 
Return a copy of the HashMap. 

Remember to free the returned HashMap.
*/
struct HashMap* hm_clone(const struct HashMap *map)
{
	struct HashMap *copy = HashMap_new(map->capacity, map->hash, map->equals, map->toString);

	for (size_t i = 0; i < map->capacity; i++)
		for (struct hm_Node *iter = map->table[i]; iter != NULL; iter = iter->next)
			hm_put(copy, iter->key, iter->value);
	
	if (map->nullKeySet != NULL)
	{
		copy->nullKeySet = map->nullKeySet;
		copy->size++;
	}

	return copy;
}

/* 
Gets the size of the HashMap. 

If parameter is true, gets the table size + if a null key is present.
If false, only gets the table size.
*/
static size_t hm_getSize(const struct HashMap *map, bool includingNull)
{
	return map->size - (includingNull ? (map->nullKeySet != NULL ? 1 : 0) : 0);
}

/* Deconstructor function. */
void hm_destroy(struct HashMap *map)
{
	// Destroy the table.
	for (size_t i = 0; i < map->capacity; i++)
		for (struct hm_Node *iter = map->table[i], *temp = NULL; iter != NULL; iter = temp) 
		{
			temp = iter->next;
			free(iter);
		}
	free(map->table);
	free(map->nullKeySet);
	free(map);
}