
#include "HashMap.h"

/* Local typedef for convenience. */
typedef struct HashMap Map;
typedef struct map_Entry map_Entry;
typedef struct map_Node Node;

/* Constructor functions. */
map_Entry* KeySet_new(const void* const key, const void* value);
Node* Node_new(const map_Entry* const set);
Node** Table_new(const size_t capacity);

/* Deconstructor functions. */
/* If deep, destroy all members as well. */
void Node_destroy(const Node* const node, const bool deep);

/* Convenience functions. */
map_Entry* map_search(const Map* const map, const void* const key);
size_t map_tableSize(const Map* const map);
unsigned int map_modulus(const unsigned int operandL, const unsigned int operandR);
bool map_powerOf(const unsigned int n, const unsigned int base);

/* Default capacity of the HashMap. */
const size_t DEFAULT_INITIAL_CAPACITY = 16;
/* Map expands when the size / capacity is greater than the load factor. */
const float LOAD_FACTOR = 0.75;

/*
* Hash function. Must return a value larger than the capacity of the Map.
* Ideally, hash should convert unique keys into unique integers.
*
* Equals function. It's possible for two unique keys to hash to the same value.
* Due to this, an equals function is needed to distinguish different elements in the map.
*
* toString function. Converts a Key/Value entry into a string for the print function.
* The first parameter for the function is the key and the second is the value.
*/
Map* HashMap_new(unsigned int(*hash)(const void* const),
	bool(*equals)(const void* const, const void* const), 
		char*(*toString)(const void* const, const void* const))
{
	Map* const map = ds_calloc(1, sizeof(Map));
	/* Function pointers. */
	map->hash = hash;
	map->equals = equals;
	map->toString = toString;

	/* Note: Capacity must be a power of 2. */
	map->table = Table_new(DEFAULT_INITIAL_CAPACITY);
	map->capacity = DEFAULT_INITIAL_CAPACITY;
	return map;
}

/*
 * Returns the value corresponding to the provided key in the map. NULL if no such key exists.
 * Ω(1), Θ(1), O(1)
 */
void* map_get(const struct HashMap* const map, const void* const key)
{
	const map_Entry* const retrieved = map_search(map, key);
	return retrieved != NULL ? retrieved->value : NULL;
}

/*
 * Returns true if a provided key exists within the Map.
 * Ω(1), Θ(1), O(1)
 */
bool map_contains(const struct HashMap* const map, const void* const key)
{
	return map_search(map, key) != NULL;
}

/*
* Returns true if the map is empty. False if otherwise.
* Ω(1), Θ(1), O(1)
*/
bool map_empty(const struct HashMap* const map)
{
	return map_size(map) == 0;
}

/*
 * Returns the amount of elements inside the Map.
 * Ω(1), Θ(1), O(1)
 */
size_t map_size(const struct HashMap* const map)
{
	return map->size;
}

/*
* Prints all elements inside the Map to the console window.
* Ω(n), Θ(n), O(n)
*/
void map_print(const struct HashMap* const map)
{
	printf_s("{ ");

	/* Loop until either all elements are found or we've reached capacity. */
	for (unsigned int i = 0, h = 0; i < map->capacity && h < map_size(map); i++)
		for (const Node *iter = map->table[i]; iter != NULL; iter = iter->next)
		{
			printf_s("%s ", map->toString(iter->set->key, iter->set->value));
			h++;
		}

	printf_s("}\n");
}

/*
 * Returns an array of all Key/Value entries inside the Map.
 * Remember to free the array after use.
 * Ω(n), Θ(n), O(n)
 */
struct map_Entry** map_keyset(const struct HashMap *map)
{
	/* Use regular malloc here since it's user's job to clear the memory used. */
	const map_Entry** const entries = malloc(map_size(map));
	unsigned int index = 0;

	/* Place the NULL key inside the array, if one exists. */
	if (map->nullSet != NULL)
		entries[index++] = map->nullSet;

	/* Loop until either all elements are found or we've reached capacity. */
	for (unsigned int i = 0, h = 0; i < map->capacity && h < map_size(map); i++)
		for (const Node *iter = map->table[i]; iter != NULL; iter = iter->next)
		{
			entries[index++] = iter->set;
			h++;
		}

	return entries;
}

/*
* Returns a shallow copy of the Map.
* Ω(n), Θ(n), O(n^2)
*/
struct HashMap* map_clone(const struct HashMap* const map)
{
	const Map* const copy = HashMap_new(map->hash, map->equals, map->toString);

	if (map->nullSet != NULL)
		map_put(copy, map->nullSet->key, map->nullSet->value);

	/* Loop until either all elements are found or we've reached capacity. */
	for (unsigned int i = 0, h = 0; i < map->capacity && h < map_size(map); i++)
		for (const Node *iter = map->table[i]; iter != NULL; iter = iter->next)
		{
			const map_Entry* const entry = iter->set;
			map_put(copy, entry->key, entry->value);
			h++;
		}

	return copy;
}

/*
 * Associates a provided key with a provided value into the Map.
 * The map will expand dynamically if the map is becoming too full.
 * Ω(1), Θ(1), O(n)
 */
void map_put(struct HashMap* const map, const void* const key, const void* const value)
{
	/* Double the size of the HashMap if it is getting too full. */
	if (map_tableSize(map) / (double)map->capacity >= LOAD_FACTOR)
	{
		/* Temporary map that we will store our new table inside of. */
		const Map* const expanded = HashMap_new(map->hash, map->equals, map->toString);

		/* Loop until either all elements are found or we've reached capacity. */
		for (unsigned int i = 0, h = 0; i < map->capacity && h < map_size(map); i++)
		{
			const Node *iter = map->table[i];
			while (iter != NULL)
			{
				map_put(expanded, iter->set->key, iter->set->value);
				h++;
				/* Destroy the current Node and advance to the next. */
				const Node* const temp = iter;
				iter = iter->next;
				Node_destroy(temp, true);
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
		const size_t index = map_modulus(map->hash(key), map->capacity);

		/* No collision. */
		if (map->table[index] == NULL)
			map->table[index] = Node_new(KeySet_new(key, value));
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
			iter->next = Node_new(KeySet_new(key, value));
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
			map->nullSet = KeySet_new(key, value);
			map->size++;
		}
}


bool map_remove(struct HashMap* const map, const void* const key)
{
	/* NULL key must be handled in a different way. */
	if (key == NULL)
	{
		if (map->nullSet != NULL)
		{
			ds_free(map->nullSet, sizeof(map_Entry));
			map->nullSet = NULL;
			map->size--;
			return true;
		}
		
		return false;
	}

	/* Attempt to locate and delete the Node with the provided key. */
	const size_t index = map_modulus(map->hash(key), map->capacity);
	for (Node *iter = map->table[index], *prev = NULL; iter != NULL; prev = iter, iter = iter->next)
		if (map->equals(iter->set->key, key))
		{
			/* Remove the old value from the Node list. */
			*(prev != NULL ? &prev->next : &map->table[index]) = iter->next;
			removed = iter->set->value;
			Node_destroy(iter, true);
			break;
		}


	/* HashMap accepts NULL as a key. */
	if (key != NULL)
	{
		
	}
	else if (map->nullSet != NULL && map->equals(map->nullSet->key, key))
	{
		removed = map->nullSet->value;
		map_entry_destroy(map->nullSet);
		map->nullSet = NULL;
	}

	if (removed != NULL)
		map->size--;
	return true;
}

/* Clear the HashMap of all Key/Value pairs. */
void map_clear(struct HashMap* const map)
{
	/* Destroy the entire table. */
	for (size_t i = 0; i < map->capacity; i++)
	{
		const Node *iter = map->table[i], *temp = NULL;
		while (iter != NULL)
		{
			temp = iter->next;
			Node_destroy(iter, true);
			iter = temp;
		}
	}
	ds_free(map->table, map->capacity * sizeof(Node*));

	/* Destroy the NULL key as well. */
	if (map->nullSet != NULL)
	{
		map_entry_destroy(map->nullSet);
		map->nullSet = NULL;
	}

	/* Reconstruct the table. */
	map->table = Table_new(DEFAULT_INITIAL_CAPACITY);
	map->capacity = DEFAULT_INITIAL_CAPACITY;
	map->size = 0;
}

/* De-constructor function. */
void map_destroy(const struct HashMap* const map)
{
	/* Destroy the entire table. */
	for (size_t i = 0; i < map->capacity; i++)
	{
		const Node *iter = map->table[i], *temp = NULL;
		while (iter != NULL)
		{
			temp = iter->next;
			Node_destroy(iter, true);
			iter = temp;
		}
	}
	ds_free(map->table, map->capacity * sizeof(Node*));

	/* Destroy the NULL key as well. */
	if (map->nullSet != NULL)
		map_entry_destroy(map->nullSet);
	ds_free(map, sizeof(Map));
}

/* Deconstructor function. */
void map_entry_destroy(const struct map_Entry* const set)
{
	ds_free(set, sizeof(map_Entry));
}

/* Constructor function. */
map_Entry* KeySet_new(const void* const key, const void* value)
{
	map_Entry* const set = ds_malloc(sizeof(map_Entry));
	set->key = key;
	set->value = value;
	return set;
}

/* Constructor function. */
Node* Node_new(const map_Entry* const set)
{
	Node* const entry = ds_calloc(1, sizeof(Node));
	entry->set = set;
	return entry;
}

/* Constructor function. */
Node** Table_new(const size_t capacity)
{
	const Node** const table = ds_calloc(capacity, sizeof(Node*));
	return table;
}

/* Deconstructor function.
If deep, destroy all members as well. */
void Node_destroy(const Node* const node, const bool deep)
{
	if (deep)
		map_entry_destroy(node->set);
	ds_free(node, sizeof(Node));
}

/*
Helper function.
Locates a given map_Entry in the HashMap.

Returns NULL if it cannot be located.
*/
map_Entry* map_search(const Map* const map, const void* const key)
{
	/* NULL keys must be handled differently. */
	if (key == NULL)
		return map->nullSet;
	
	const size_t index = map_modulus(map->hash(key), map->capacity);
	for (const Node *iter = map->table[index]; iter != NULL; iter = iter->next)
		/* Key matches this node, return the value associated with it. */
		if (map->equals(iter->set->key, key))
			return iter->set;
	return NULL;
}

/*
Returns the amount of elements inside the table of the HashMap.
*/
size_t map_tableSize(const Map* const map)
{
	return (map->nullSet == NULL) ? map->size : map->size - 1;
}

/* Performs bitwise modulus. */
unsigned int map_modulus(const unsigned int operandL, const unsigned int operandR)
{
	return operandL & (operandR - 1);
}

/* Determines of n is a power of base. */
bool map_powerOf(const unsigned int n, const unsigned int base)
{
	const double exponent = log((double)n) / log((double)base);
	return floor(exponent) == exponent;
}