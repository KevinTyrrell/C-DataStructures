## Data Structures in C
=========================

Static library implementations of popular data structures in Computer Science. These were developed for use in my own projects, as I wanted a way to abstract the C language a bit to make more complex projects easier to work with.
Each data structure's implementation should closely match the correct algorithms for each operation. Complexities written in the documentation should be accurate. Each structure may need function pointers in order to work properly. It is up to the user of this library to provide those functions to the structures. The data types for each of the structures is of type `void*`, thus the strucutres need to know how to handle the types you provide it with.

|Data Structure|Uses|Sorted?|Functions Required|Thread Safe|
|-|:-:|:-:|-|:-:
|Vector| Random Access, Deque, Stack, Queue|On Demand Ω(n * log(n))|**compare** (optional, used for *sort*)<br>**toString** (optional, used for *print*)|No
|LinkedList|Deque, Stack, Queue|On Demand Θ(n * log(n))|**compare** (optional, used for *sort*)<br>**toString** (optional, used for *print*)|No
|HashTable|Map, Set|No|**hash** (mandatory)<br>**equals** (mandatory)<br>**toString** (optional, used for *print*)|No



## Example Uses:


#### Vector

```c
#include "Vector.h"

char* toString(const void* const v)
{
	static char ar[1];
	ar[0] = *(char*)v;
	return ar;
}

int compare(const void* const p1, const void* const p2)
{
	const char c1 = *(char*)p1, c2 = *(char*)p2;
	if (c1 < c2) return -1;
	if (c1 > c2) return 1;
	return 0;
}

int main(int argc, char* argv[])
{
	struct Vector *vect = Vector_new(&compare, &toString);

	const char letters[] = "ABCDEFGHIJK";
	for (unsigned int i = 0, len = strlen(letters); i < len; i++)
		if (i % 2 == 0)
			vect_push_back(vect, &letters[i]);
		else
			vect_push_front(vect, &letters[i]);
	
	vect_print(vect); // Prints [J, H, F, D, B, A, C, E, G, I, K]
	while (vect_size(vect) > 5)
		vect_pop_back(vect);
	vect_sort(vect);
	vect_print(vect); // Prints [B, D, F, H, J]

	/* Remember to free the Vector after use. */
	vect_destroy(vect);
    return 0;-
}
```

#### HashTable


```c
#include "HashTable.h"

#include <stdbool.h>
#include <stdio.h>
#include <string.h>

/* djb2 algorithm by Dan Bernstein. */
unsigned int hash(const void *key)
{
	/* Our key in this case is a String. */
	const char* str = (char*)key;
	unsigned int hash = 5381;

	for (int c; c = *str++;)
		hash = ((hash << 5) + hash) + c; /* hash * 33 + c */

	return hash;
}

/* Check if two Strings are equal. */
bool equals(const void *k1, const void *k2)
{
	return strcmp(k1, k2) == 0;
}

/* Only used for printing out Key/Value pairs. */
char* toString(const struct table_Entry *entry)
{
	static char ar[50];
	sprintf(ar, "<%s,%.2f>", (char*)entry->key, *(float*)entry->value);
	return ar;
}

int main(int argc, char* argv[])
{
	HashTable* const gradebook = HashTable_new(&hash, &equals, &toString);
	
	size_t size = 13;
	const char* const students[] = {
		"Jessie", "James", "Brock", "Misty", "Ash", "Gary", "Oak", "Giovanni",
		"Jenny", "Tracey", "May", "Max", "Dawn"
	};
	const float const grades[] = {
		3.25f, 1.98f, 3.90f, 2.0f, 1.12f, 1.8f, 2.0f, 3.33f,
		4.0f, 3.0f, 0.5f, 2.9f, 2.1f
	};

	// HashTable grows automatically, table_grow call is not needed.
	// If you do know how many elements you want to add, call grow to speed up complexity.
	table_grow(gradebook, size);

	// Place all Key/Value pairs into the map.
	for (int i = 0; i < size; i++)
		table_put(gradebook, students[i], &grades[i]);
	
	// Get the value associated with the key "Ash".
	printf("The student %s has a GPA of %.2f.\n", "Ash",
		*(float*)table_get(gradebook, "Ash"));

	// Print all Key/Value entries.
	table_print(gradebook);

 	/* Remember to free the HashTable after use. */
	table_destroy(gradebook);
	return 0;
}
```