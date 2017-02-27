
#include "HashMap.h"

/* Local typedefs. */
typedef struct HashMap Map;
typedef struct hm_KeySet KeySet;
typedef struct hm_Node Node;

/* Constructor functions. */
KeySet* hm_KeySet_new(const void* const key, const void* value);
Node* hm_Node_new(const KeySet* const set);
Node** hm_Table_new(const size_t capacity);

/* Deconstructor functions. */
/* If deep, destroy all members as well. */
void hm_Node_destroy(const Node* const node, const bool deep);

/* Convenience functions. */
KeySet* hm_find(const Map* const map, const void* const key);
size_t hm_tableSize(const Map* const map);
unsigned int hm_modulus(const unsigned int operandL, const unsigned int operandR);
bool hm_powerOf(const unsigned int n, const unsigned int base);

/* Default capacity of the HashMap. */
const size_t DEFAULT_INITIAL_CAPACITY = 16;
/* Map expands when the size / capacity is greater than the load factor. */
const float LOAD_FACTOR = 0.75;

/* Constructor function. */
Map* HashMap_new(const size_t capacity, unsigned int(*hash)(const void* const),
	bool(*equals)(const void* const, const void* const), char*(*toString)(const void* const))
{
	Map* const map = ds_calloc(1, sizeof(Map));
	/* Function pointers. */
	map->hash = hash;
	map->equals = equals;
	map->toString = toString;

	/* Capacity must be a power of 2. */
	const size_t fixedCap = hm_powerOf(capacity, 2) ? capacity : DEFAULT_INITIAL_CAPACITY;
	map->table = hm_Table_new(fixedCap);
	map->capacity = fixedCap;
	return map;
}

/*
Gets a value from the HashMap when provided with a key.

O(1) complexity.
Returns NULL if no such key exists.
*/
void* hm_get(const struct HashMap* const map, const void* const key)
{
	const KeySet* const retrieved = hm_find(map, key);
	return retrieved != NULL ? retrieved->value : NULL;
}

/* Returns true if the HashMap contains the specified key. */
bool hm_contains(const struct HashMap* const map, const void* const key)
{
	return hm_find(map, key) != NULL;
}

/* Prints out the inner table of the HashMap. */
void hm_print(const struct HashMap* const map)
{
	printf_s("Map Capacity: %zu, Map Size: %zu\n", map->capacity, map->size);
	if (map->nullSet != NULL)
		printf_s("%s%s\n", "[NULL]: ", map->toString(map->nullSet->value));
	for (size_t i = 0; i < map->capacity; i++)
	{
		printf_s("%c%zu%s", '[', i, "]: ");
		for (Node *iter = map->table[i]; iter != NULL; iter = iter->next)
		{
			printf_s("%s", map->toString(iter->set->value));
			if (iter->next != NULL)
				printf_s("%s", " -> ");
		}
		printf_s("\n");
	}
}

/*
Return an array with all Keys/Values currently in the HashMap.

Remember to free the array after use.
*/
struct hm_KeySet** hm_KeySets(const struct HashMap *map)
{
	const KeySet** const sets = ds_malloc(map->size + 1);
	unsigned int index = 0;

	for (unsigned int i = 0; i < map->capacity; i++)
		for (const Node* iter = map->table[i]; iter != NULL; iter = iter->next)
			sets[index++] = iter->set;
	if (map->nullSet != NULL)
		sets[index++] = map->nullSet;
	/* NULL terminating character. */
	sets[index] = NULL;
	return sets;
}

/* Returns a shallow copy of the HashMap. */
struct HashMap* hm_clone(const struct HashMap* const map)
{
	const Map* const copy = HashMap_new(map->capacity, map->hash, map->equals, map->toString);

	for (size_t i = 0; i < map->capacity; i++)
		for (const Node *iter = map->table[i]; iter != NULL; iter = iter->next)
			hm_put(copy, iter->set->key, iter->set->value);
	if (map->nullSet != NULL)
		hm_put(copy, map->nullSet->key, map->nullSet->value);

	return copy;
}

/*
Place a Key/Value pair into the HashMap.

O(1) complexity, assuming load factor is not reached.
*/
void hm_put(struct HashMap* const map, const void* const key, const void* const value)
{
	/* Double the size of the HashMap if it is getting too full. */
	if (hm_tableSize(map) / (double)map->capacity >= LOAD_FACTOR)
	{
		/* Temporary map that we will store our new table inside of. */
		const Map* const expanded = HashMap_new(map->capacity * 2, map->hash, map->equals, map->toString);

		/* Go through the old table and put each key/value pair
		into the new table. Destroy the old nodes. */
		for (unsigned int i = 0; i < map->capacity; i++)
		{
			const Node *iter = map->table[i];
			while (iter != NULL)
			{
				hm_put(expanded, iter->set->key, iter->set->value);
				const Node* const temp = iter;
				iter = iter->next;
				hm_Node_destroy(temp, true);
			}
		}

		/* Destroy the old table and replace it with the larger one. */
		ds_free(map->table, map->capacity * sizeof(Node*));
		map->table = expanded->table;
		/* Note that capacity changes but size doesn't! */
		map->capacity = expanded->capacity;
		ds_free(expanded, sizeof(Map));
	}

	/* HashMap is of correct size. Proceed to add the Key/Value to the Map. */
	if (key != NULL)
	{
		const size_t index = hm_modulus(map->hash(key), map->capacity);

		/* No collision. */
		if (map->table[index] == NULL)
			map->table[index] = hm_Node_new(hm_KeySet_new(key, value));
		else
		{
			Node *iter = map->table[index];
			while (iter->next != NULL)
			{
				/* Check if this key is a duplicate entry. */
				if (map->equals(iter->set->key, key))
				{
					iter->set->value = value;
					return;
				}
				iter = iter->next;
			}
			iter->next = hm_Node_new(hm_KeySet_new(key, value));
		}

		map->size++;
	}
	/* NULL is allowed as a key. */
	else
		if (map->nullSet != NULL)
		{
			map->nullSet->key = key;
			map->nullSet->value = value;
		}
		else
		{
			map->nullSet = hm_KeySet_new(key, value);
			map->size++;
		}
}

/*
Removes a Key/Value pair from the HashMap when provided with a key.

O(1) complexity.
Returns the value of the removed Node, NULL if operation failed.
*/
void* hm_remove(struct HashMap* const map, const void* const key)
{
	const void *removed = NULL;

	/* HashMap accepts NULL as a key. */
	if (key != NULL)
	{
		const size_t index = hm_modulus(map->hash(key), map->capacity);
		for (Node *iter = map->table[index], *prev = NULL; iter != NULL; prev = iter, iter = iter->next)
			if (map->equals(iter->set->key, key))
			{
				/* Remove the old value from the Node list. */
				*(prev != NULL ? &prev->next : &map->table[index]) = iter->next;
				removed = iter->set->value;
				hm_Node_destroy(iter, true);
				break;
			}
	}
	else if (map->nullSet != NULL && map->equals(map->nullSet->key, key))
	{
		removed = map->nullSet->value;
		hm_KeySet_destroy(map->nullSet);
		map->nullSet = NULL;
	}

	if (removed != NULL)
		map->size--;
	return removed;
}

/* Clear the HashMap of all Key/Value pairs. */
void hm_clear(struct HashMap* const map)
{
	/* Destroy the entire table. */
	for (size_t i = 0; i < map->capacity; i++)
	{
		const Node *iter = map->table[i], *temp = NULL;
		while (iter != NULL)
		{
			temp = iter->next;
			hm_Node_destroy(iter, true);
			iter = temp;
		}
	}
	ds_free(map->table, map->capacity * sizeof(Node*));

	/* Destroy the NULL key as well. */
	if (map->nullSet != NULL)
	{
		hm_KeySet_destroy(map->nullSet);
		map->nullSet = NULL;
	}

	/* Reconstruct the table. */
	map->table = hm_Table_new(DEFAULT_INITIAL_CAPACITY);
	map->capacity = DEFAULT_INITIAL_CAPACITY;
	map->size = 0;
}

/* De-constructor function. */
void hm_destroy(const struct HashMap* const map)
{
	/* Destroy the entire table. */
	for (size_t i = 0; i < map->capacity; i++)
	{
		const Node *iter = map->table[i], *temp = NULL;
		while (iter != NULL)
		{
			temp = iter->next;
			hm_Node_destroy(iter, true);
			iter = temp;
		}
	}
	ds_free(map->table, map->capacity * sizeof(Node*));

	/* Destroy the NULL key as well. */
	if (map->nullSet != NULL)
		hm_KeySet_destroy(map->nullSet);
	ds_free(map, sizeof(Map));
}

/* Deconstructor function. */
void hm_KeySet_destroy(const struct hm_KeySet* const set)
{
	ds_free(set, sizeof(KeySet));
}

/* Constructor function. */
KeySet* hm_KeySet_new(const void* const key, const void* value)
{
	KeySet* const set = ds_malloc(sizeof(KeySet));
	set->key = key;
	set->value = value;
	return set;
}

/* Constructor function. */
Node* hm_Node_new(const KeySet* const set)
{
	Node* const entry = ds_calloc(1, sizeof(Node));
	entry->set = set;
	return entry;
}

/* Constructor function. */
Node** hm_Table_new(const size_t capacity)
{
	const Node** const table = ds_calloc(capacity, sizeof(Node*));
	return table;
}

/* Deconstructor function.
If deep, destroy all members as well. */
void hm_Node_destroy(const Node* const node, const bool deep)
{
	if (deep)
		hm_KeySet_destroy(node->set);
	ds_free(node, sizeof(Node));
}

/*
Helper function.
Locates a given KeySet in the HashMap.

Returns NULL if it cannot be located.
*/
KeySet* hm_find(const Map* const map, const void* const key)
{
	/* NULL keys must be handled differently. */
	if (key != NULL)
	{
		const size_t index = hm_modulus(map->hash(key), map->capacity);
		for (const Node *iter = map->table[index]; iter != NULL; iter = iter->next)
			/* Key matches this node, return the value associated with it. */
			if (map->equals(iter->set->key, key))
				return iter->set;
	}
	else if (map->nullSet != NULL && map->equals(map->nullSet->key, key))
		return map->nullSet;

	return NULL;
}

/*
Returns the amount of elements inside the table of the HashMap.
*/
size_t hm_tableSize(const Map* const map)
{
	return (map->nullSet == NULL) ? map->size : map->size - 1;
}

/* Performs bitwise modulus. */
unsigned int hm_modulus(const unsigned int operandL, const unsigned int operandR)
{
	return operandL & (operandR - 1);
}

/* Determines of n is a power of base. */
bool hm_powerOf(const unsigned int n, const unsigned int base)
{
	const double exponent = log((double)n) / log((double)base);
	return floor(exponent) == exponent;
}