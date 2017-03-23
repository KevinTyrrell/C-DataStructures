
#include "Vector.h"

#define DEFAULT_INITIAL_CAPACITY 10
#define VECTOR_GROW_AMOUNT 2

/* Local typedef for convenience. */
typedef struct Vector Vector;

/* Convenience functions. */
bool vect_full(const Vector* const vect);
void vect_grow(Vector* const vect);
void vect_swap(const Vector* const vect, const unsigned int i, const unsigned int h);
void ptr_swap(const void** const v1, const void** const v2);
void vect_merge_sort(const Vector* const vect, const unsigned int start, const size_t size);
unsigned int vect_index(const Vector* const vect, const unsigned int index);
unsigned int wrap_add(unsigned int val, int dx, const unsigned int lower, const unsigned int upper);

/* Constructor function. */
struct Vector* Vector_new(int(*compare)(const void*, const void*), char*(*toString)(const void*))
{
	Vector* const vect = ds_calloc(1, sizeof(Vector));
	vect->table = ds_calloc(DEFAULT_INITIAL_CAPACITY, sizeof(void*));
	vect->capacity = DEFAULT_INITIAL_CAPACITY;
	vect->compare = compare;
	vect->toString = toString;
	return vect;
}

/*
 * Returns the value at the given index.
 * Ω(1), Θ(1), O(1)
 */
void* vect_at(const struct Vector* const vect, const unsigned int index)
{
	/* Error checking before accessing. */
	bool error = true;
	if (vect_empty(vect))
		ds_error(DS_MSG_EMPTY);
	else if (index >= vect_size(vect))
		ds_error(DS_MSG_OUT_OF_BOUNDS);
	else error = false;
	if (error) return NULL;

	/* Wrap around the table if the index exceeds the capacity. */
	return (void*)vect->table[vect_index(vect, index)];
}

/*
 * Returns the value at the front of the Vector.
 * Ω(n), Θ(n), O(n)
 */
void* vect_front(const struct Vector* const vect)
{
	if (vect_empty(vect))
	{
		ds_error(DS_MSG_EMPTY);
		return NULL;
	}

	return (void*)vect->table[vect->start];
}

/*
 * Returns the value at the end of the Vector.
 * Ω(1), Θ(1), O(1)
 */
void* vect_back(const struct Vector* const vect)
{
	if (vect_empty(vect))
	{
		ds_error(DS_MSG_EMPTY);
		return NULL;
	}

	return (void*)vect->table[vect->end];
}

/*
 * Returns the size of the Vector.
 * Ω(1), Θ(1), O(1)
 */
size_t vect_size(const struct Vector* const vect)
{
	return vect->size;
}

/*
 * Returns true if the Vector is empty.
 * Ω(1), Θ(1), O(1)
 */
bool vect_empty(const struct Vector* const vect)
{
	return vect->size == 0;
}

/*
 * Returns true if the Vector contains the provided element.
 * Ω(1), Θ(1), O(1)
 */
bool vect_contains(const struct Vector* const vect, const void* const data)
{
	for (unsigned int i = 0, size = vect_size(vect); i < size; i++)
		if (vect->compare(vect_at(vect, i), data) == 0)
			return true;
	return false;
}

/*
 * Returns an array of all elements inside the Vector.
 * Remember to call `free` on the dynamically allocated array.
 * Ω(n), Θ(n), O(n)
 */
void** vect_array(const struct Vector* const vect)
{
	const void** const arr = calloc(vect_size(vect), sizeof(void*));
	for (unsigned int i = 0, size = vect_size(vect); i < size; i++)
		arr[i] = vect_at(vect, i);
	
	return arr;
}

/*
 * Prints out the contents of the Vector using the toString function.
 * Ω(n), Θ(n), O(n)
 */
void vect_print(const struct Vector* const vect)
{
	printf_s("%c", '[');
	for (unsigned int i = 0; i < vect->size; i++)
		printf_s("%s%s", vect->toString(vect_at(vect, i)),
			i + 1 < vect->size ? ", " : "");
	printf_s("]\n");
}

/*
 * Prints out the internal structure of the inner array.
 * Ω(n), Θ(n), O(n)
 */
void vect_debug_print(const struct Vector* const vect)
{
	/* Determine how many rows we will need to print and go through each one. */
	const unsigned int INDEXES_PER_ROW = 10;
	/* Color that the array's elements will be printed as. */
	const WORD elementColor = 13;
	const unsigned int rows = (unsigned int)ceil(vect->capacity / (double)INDEXES_PER_ROW);
	
	unsigned int iter = 0;
	for (unsigned int i = 0; i < rows; i++)
	{
		printf_s("     ");
		const WORD oldColor = ds_changeColor(elementColor);
		for (unsigned int h = 0; h < INDEXES_PER_ROW && iter < vect->capacity; h++)
		{
			const void* const value = vect->table[iter++];
			printf_s(" %.1s  ", value != NULL ? vect->toString(value) : ".");
		}
		ds_changeColor(oldColor);

		printf_s("\nR%u   ", i % 10);
		/* Print indexes at the bottom of the row to indicate where we are in the array. */
		for (unsigned int h = 0; h < INDEXES_PER_ROW; h++)
			printf_s("[%u] ", h % 10);
		printf_s("\n");
	}

	printf_s("Vector Status -- Size: %zu, Capacity: %zu, Start: %u, End: %u.\n", vect_size(vect), vect->capacity, vect->start, vect->end);
}

/*
 * Replaces an element in the Vector at a given index with a specified value.
 * Ω(1), Θ(1), O(1)
 */
void vect_assign(const struct Vector* const vect, const unsigned int index, const void* const data)
{
	/* Error checking before accessing. */
	bool error = true;
	if (data == NULL)
		ds_error(DS_MSG_NULL_PTR);
	else if (vect_empty(vect))
		ds_error(DS_MSG_EMPTY);
	else if (index >= vect_size(vect))
		ds_error(DS_MSG_OUT_OF_BOUNDS);
	else error = false;
	if (error) return;

	vect->table[vect_index(vect, index)] = data;
}

/*
 * TODO: Attempt to simplify algorithm.
 * Inserts the given element at the provided index.
 * Ω(1), Θ(n), O(n)
 */
void vect_insert(struct Vector * const vect, const unsigned int index, const void * const data)
{
	bool exit = true;
	if (data == NULL)
		ds_error(DS_MSG_NULL_PTR);
	else if (index > vect_size(vect))
		ds_error(DS_MSG_OUT_OF_BOUNDS);
	else if (index == 0)
		vect_push_front(vect, data);
	else if (index == vect_size(vect))
		vect_push_back(vect, data);
	else exit = false;
	if (exit) return;

	/* Check if we need to increase the array's capacity. */
	if (vect_full(vect))
		vect_grow(vect);
	
	unsigned int iter, target, dX;

	/* Check whether it is less expensive to shift rightwards. */
	if (vect->size - index - 1 <= index)
	{
		vect->end = wrap_add(vect->end, 1, 0, vect->capacity - 1);
		target = vect_index(vect, index);
		iter = vect->end;
		dX = -1;
	}
	else
	{
		/* Shifting backwards will make all n indexes into (n - 1), so subtract one from target. */
		target = wrap_add(vect_index(vect, index), -1, 0, vect->capacity - 1);
		vect->start = wrap_add(vect->start, -1, 0, vect->capacity - 1);
		iter = vect->start;
		dX = 1;
	}

	/* Shift leftward or rightward depending on the above conditions. */
	while (iter != target)
	{
		const unsigned int next = wrap_add(iter, dX, 0, vect->capacity - 1);
		ptr_swap(&vect->table[iter], &vect->table[next]);
		iter = next;
	}
	
	vect->size++;
	/* Set the given value as the new value. */
	vect_assign(vect, index, data);
}

/*
 * Inserts the given element at the end of the Vector.
 * Ω(1), Θ(1), O(n)
 */
void vect_push_back(Vector * const vect, const void * const data)
{
	if (data == NULL)
	{
		ds_error(DS_MSG_NULL_PTR);
		return;
	}

	/* Check if we need to increase the array's capacity. */
	if (vect_full(vect))
		vect_grow(vect);

	/* When Vector has one or less element(s), start and end must point to the same index. */
	if (!vect_empty(vect))
		/* Increment end and wrap. */
		vect->end = wrap_add(vect->end, 1, 0, vect->capacity - 1);
	
	vect->table[vect->end] = data;
	vect->size++;
}

/*
 * Inserts the given element at the front of the Vector.
 * Ω(1), Θ(1), O(n)
 */
void vect_push_front(Vector * const vect, const void * const data)
{
	if (data == NULL)
	{
		ds_error(DS_MSG_NULL_PTR);
		return;
	}

	/* Check if we need to increase the array's capacity. */
	if (vect_full(vect))
		vect_grow(vect);

	/* When Vector has one or less element(s), start and end must point to the same index. */
	if (!vect_empty(vect))
		/* Increment end and wrap. */
		vect->start = wrap_add(vect->start, -1, 0, vect->capacity - 1);

	vect->table[vect->start] = data;
	vect->size++;
}

/*
 * Removes the element at the end of the Vector.
 * Ω(1), Θ(1), O(n)
 */
void vect_pop_back(struct Vector* const vect)
{
	if (vect_empty(vect))
	{
		ds_error(DS_MSG_EMPTY);
		return;
	}
	
	/* When Vector has one or less element(s), start and end must point to the same index. */
	if (vect_size(vect) > 1)
		vect->end = wrap_add(vect->end, -1, 0, vect->capacity - 1);
	vect->size--;
}

/*
* Removes the element at the front of the Vector.
* Ω(1), Θ(1), O(n)
*/
void vect_pop_front(struct Vector* const vect)
{
	if (vect_empty(vect))
	{
		ds_error(DS_MSG_EMPTY);
		return;
	}

	/* When Vector has one or less element(s), start and end must point to the same index. */
	if (vect_size(vect) > 1)
		vect->start = wrap_add(vect->start, 1, 0, vect->capacity - 1);
	vect->size--;
}

/*
 * Removes all elements from the Vector while preserving the capacity.
 * Ω(1), Θ(1), O(1)
 */
void vect_clear(struct Vector* const vect)
{
	vect->start = 0;
	vect->end = 0;
	vect->size = 0;
}

/*
 * Sorts elements inside the Vector in descending order.
 * Uses the `compare` function provided to the Vector.
 * Implementation uses Merge Sort.
 * Ω(n log(n)), Θ(n log(n)), O(n log(n))
 */
void vect_sort(const struct Vector* const vect)
{
	vect_merge_sort(vect, 0, vect->size);
}

/*
 * De-constructor function. 
 * Ω(1), Θ(1), O(1)
 */
void vect_destroy(const struct Vector* const vect)
{
	ds_free(vect->table, vect->capacity * sizeof(void*));
	ds_free((void*)vect, sizeof(Vector));
}

/*
 * Returns true if the Vector is full.
 * Ω(1), Θ(1), O(1)
 */
bool vect_full(const Vector* const vect)
{
	return vect->size == vect->capacity;
}

/*
 * Grows the underlying array by a factor of `VECTOR_GROW_AMOUNT`.
 * Ω(n), Θ(n), O(n)
 */
void vect_grow(Vector * const vect)
{
	/* Expand the underlying array by a set amount. */
	const size_t expanded_capacity = vect->capacity * VECTOR_GROW_AMOUNT;
	const void** const expanded_table = ds_calloc(expanded_capacity, sizeof(void*));

	for (unsigned int i = 0, size = vect_size(vect); i < size; i++)
		expanded_table[i] = vect_at(vect, i);

	/* Destroy the old table. */
	ds_free(vect->table, vect->capacity * sizeof(void*));
	/* Update the Vector's properties. */
	vect->table = expanded_table;
	vect->capacity = expanded_capacity;
	vect->start = 0;
	vect->end = vect_size(vect) - 1;
}

/*
* Swap function for sorting and shifting algorithms.
* Ω(1), Θ(1), O(1)
*/
void vect_swap(const Vector* const vect, const unsigned int i, const unsigned int h)
{
	const void* const temp = vect_at(vect, i);
	vect_assign(vect, i, vect_at(vect, h));
	vect_assign(vect, h, temp);
}

/*
* Swap function for sorting and shifting algorithms.
* Ω(1), Θ(1), O(1)
*/
void ptr_swap(const void** const v1, const void** const v2)
{
	const void* const temp = *v1;
	*v1 = *v2;
	*v2 = temp;
}

/*
 * Converts regular indexes from 0->(capacity - 1) into indexes that wrap around the Vector. 
 * Ω(1), Θ(1), O(1)
 */
unsigned int vect_index(const Vector* const vect, const unsigned int index)
{
	return (vect->start + index) % vect->capacity;
}

/*
 * Given a value and a change in that value (dX), find the sum and wrap it between lower and upper.
 * Ω(n), Θ(n), O(n)
 */
unsigned int wrap_add(unsigned int val, int dx, const unsigned int lower, const unsigned int upper)
{
	while (dx < 0)
	{
		val = (val == lower) ? upper : val - 1;
		++dx;
	}

	while (dx > 0) 
	{
		val = (val == upper) ? lower : val + 1;
		--dx;
	}

	return val;
}

/*
 * See: vect_sort.
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
	const void** const arr_left = ds_calloc(size_left, sizeof(void*));
	const void** const arr_right = ds_calloc(size_right, sizeof(void*));
	for (unsigned int i = 0; i < size_left; i++)
		arr_left[i] = vect_at(vect, start + i);
	for (unsigned int i = 0; i < size_right; i++)
		arr_right[i] = vect_at(vect, start_right + i);
	
	/* Maintain track of an iterator for the combined array and the two sub-arrays. */
	unsigned int iter = start, iter_left = 0, iter_right = 0;

	/* Merge the two sub-arrays back into the primary array. */
	while (iter_left < size_left && iter_right < size_right)
		if (vect->compare(arr_left[iter_left], arr_right[iter_right]) <= 0)
			vect_assign(vect, iter++, arr_left[iter_left++]);
		else
			vect_assign(vect, iter++, arr_right[iter_right++]);
	while (iter_left < size_left)
		vect_assign(vect, iter++, arr_left[iter_left++]);
	while (iter_right < size_right)
		vect_assign(vect, iter++, arr_right[iter_right++]);

	/* Clean up memory and return the sorted array. */
	ds_free(arr_left, size_left * sizeof(void*));
	ds_free(arr_right, size_right * sizeof(void*));
}
