Data Structures in C
====================

The following are static libraries for use in my C coding projects. Ω, Θ, and O complexities should be relatively similar to their standard-library counterparts in C++/Java. Each function will have the three cases of complexity noted above their implementations. The structures were hand written without reference to the actual source code of professional implementations (excluding Red Black Tree as the whole structure is a set of rules and conditions). Each data structure will require a (sometimes optional) function pointer(s) in order to work properly. 


| Data Structure | Header File    | C File         | Constructor Function | Functions needed for the constructor                           |
| -------------- | -------------- | -------------- | -------------------- | -------------------------------------------------------------- |
| LinkedList     | LinkedList.h   | LinkedList.c   | LinkedList_new()	  | equals(void*, void*), toString(void*)                          |
| HashMap        | HashMap.h	  | HashMap.c      | HashMap_new()		  | unsigned int hash(void*), equals(void*, void*), toString(void*)|
| RedBlackTree   | RedBlackTree.h | RedBlackTree.c | RedBlackTree_new()   | int compare(void*, void*), toString(void*)                     |
| Vector         | Vector.h       | Vector.c       | Vector_new()         | int compare(void*, void*), toString(void*)                     |


## Example Uses:


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