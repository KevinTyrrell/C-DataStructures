## Data Structures in C
=========================

The following are static libraries for use in my C coding projects. Ω, Θ, and O complexities should be relatively similar to their standard-library counterparts in C++/Java. Each function will have the three cases of complexity noted above their implementations. The structures were hand written without reference to the actual source code of professional implementations (excluding Red Black Tree as the whole structure is a set of rules and conditions). Each data structure will require a (sometimes optional) function pointer(s) in order to work properly. 


| Data Structure | Header File    | C File         | Constructor Function | Functions needed for the constructor                           |
| -------------- | -------------- | -------------- | -------------------- | -------------------------------------------------------------- |
| Vector         | Vector.h       | Vector.c       | Vector_new()         | int compare(void*, void*), toString(void*)                     |
| LinkedList     | LinkedList.h   | LinkedList.c   | LinkedList_new()	  | equals(void*, void*), toString(void*)                          |
| HashMap        | HashMap.h	  | HashMap.c      | HashMap_new()		  | unsigned int hash(void*), equals(void*, void*), toString(void*)|
| RedBlackTree   | RedBlackTree.h | RedBlackTree.c | RedBlackTree_new()   | int compare(void*, void*), toString(void*)                     |


## Vector

* Circular dynamically-resizing array.
* `O(1)` push, pop from both sides
* `O(1)` random access
* Doubles capacity once full

...

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


#### LinkedList

```c
#include "LinkedList.h"
#include <math.h>

int main()
{
	// NULL can be used for function pointers.
	// This prevents the LinkedList from using search(var) and toString()
	const struct LinkedList *queue = LinkedList_new(NULL, NULL);

	int *start = malloc(sizeof(int));
	*start = 5;
	ll_push_back(start);

	while(!ll_empty(queue))
	{
		int *ptr = ll_front(queue);
		ll_pop_front(queue);
		int val = *ptr;
		free(ptr);
		
		if (val % 5 == 0)
		{		
			int *weird = malloc(sizeof(int)), *other = malloc(sizeof(int));
			*wierd = (int)(sqrt((double)val * 2434));
			*other = (int)(pow((double)val, 7.0) + 99;
			ll_push_back(weird);
			ll_push_back(other);
		}
	}

	// Must be called before the LinkedList falls out of scope.
	ll_destroy(queue);
}
```



#### HashMap


```c
#include "HashMap.h"

#include <time.h>

unsigned int hash(void* var)
{
	char v = *(char*)var;
	unsigned int h = 4243;
	return (h << 5) + h * v;
}

bool equals(void* v1, void* v2)
{
	return *(char*)v1 == *(char*)v2;
}

int main()
{
	const struct HashMap *map = HashMap_new(16, &hash, &equals, NULL);
	srand(5);
	
	for (char i = '!'; i < '~'; i++)
	{
		char *key = malloc(sizeof(char));
		*key = i;
		int *value = malloc(sizeof(int));
		*value = rand();
		hm_put(map, key, value);
	}
	
	char search = '$';
	char *located = hm_get(map, &search);
	printf_s("Found: %d\n", *located);

	hm_destroy(map);
}
```