#include "Vector.h"

#define DEFAULT_INITIAL_CAPACITY 10
#define VECTOR_GROW_AMOUNT 2

/* Local typedef for convenience. */
typedef struct Vector Vector;

/* Convenience functions. */
bool vect_full(const Vector* const vect);
void vect_grow(Vector* const vect);
unsigned int vect_middle(const Vector* const vect);
unsigned int vect_index(const Vector* const vect, const unsigned int index);
unsigned int wrap_add(unsigned int val, int dx, const unsigned int lower, const unsigned int upper);
void swap(const void** const v1, const void** const v2);
// TODO: void** vect_merge_sort(int(*compare)(const void*, const void*), void** const arr, const size_t size);

/* Constructor function. */
struct Vector * Vector_new(int(*compare)(const void*, const void*), char*(*toString)(const void*))
{
	Vector* const vect = ds_calloc(1, sizeof(Vector));
	vect->table = ds_calloc(DEFAULT_INITIAL_CAPACITY, sizeof(void*));
	vect->capacity = DEFAULT_INITIAL_CAPACITY;
	const unsigned int middle = vect_middle(vect);
	vect->start = middle;
	vect->end = middle;
	vect->compare = compare;
	vect->toString = toString;
	return vect;
}

/* 
Returns the value at the given index.
Ω(1), Θ(1), O(1)
*/
void* vect_at(const Vector * const vect, const unsigned int index)
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
void * vect_front(const Vector * const vect)
{
	if (vect_empty(vect))
	{
		ds_error(DS_MSG_EMPTY);
		return NULL;
	}

	return vect_at(vect, 0);
}

/*
 * Returns the value at the end of the Vector.
 * Ω(1), Θ(1), O(1)
 */
void * vect_back(const Vector * const vect)
{
	if (vect_empty(vect))
	{
		ds_error(DS_MSG_EMPTY);
		return NULL;
	}

	return vect_at(vect, vect->size - 1);
}

/*
 * Returns the size of the Vector.
 * Ω(1), Θ(1), O(1)
 */
size_t vect_size(const Vector * const vect)
{
	return vect->size;
}

/*
 * Returns true if the Vector is empty.
 * Ω(1), Θ(1), O(1)
 */
bool vect_empty(const Vector * const vect)
{
	return vect->size == 0;
}

/* 
 * TODO:
Returns an array of all contents inside the Vector.
Remember to call `ds_free(array, size of(void*) * vect_size(vector))` to destroy the array.
Ω(n), Θ(n), O(n)
*/
void ** vect_array(const struct Vector * const vect)
{
	const void** const arr = ds_calloc(vect_size(vect), sizeof(void*));
	for (unsigned int i = 0; i < vect_size(vect); i++)
		arr[i] = vect_at(vect, i);
	
	return arr;
}

/* 
 * TODO:
Prints out the contents of the Vector using the toString function.
Ω(n), Θ(n), O(n)
*/
void vect_print(const Vector * const vect)
{
	printf_s("%c", '[');
	for (unsigned int i = 0; i < vect->size; i++)
		printf_s("%s%s", vect->toString(vect_at(vect, i)),
			i + 1 < vect->size ? ", " : "");
	printf_s("]\n");
}

/*
 * Prints out the contents of the Vector (including NULL indexes) using the toString function.
 * Ω(n), Θ(n), O(n)
 */
void vect_debug_print(const Vector* const vect)
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
}

/*
 * Replaces an element in the Vector at a given index with a specified value.
 * Ω(1), Θ(1), O(1)
 */
void vect_assign(const Vector * const vect, const unsigned int index, const void * const data)
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
 *TODO:
Inserts the given element at the provided index.
Ω(1), Θ(n), O(n)
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
	
	/* Determine if we should left leftwards or rightwards.
	Shift the array in the direction where there is the most space. */
	const bool shift_left = vect->start >= vect->capacity - vect->end - 1;

	/* If there is no space to shift, grow the array. */
	if ((shift_left && vect->start == 0) || (!shift_left && vect->end + 1 == vect->capacity))
		vect_grow(vect);

	/* Shift elements before or after the index by one. */
	const unsigned int target_index = vect->start + index;
	if (shift_left)
		for (unsigned int i = --vect->start; i < target_index; i++)
			vect->table[i] = vect->table[i + 1];
	else
		for (unsigned int i = vect->end++; i >= target_index; i--)
			vect->table[i + 1] = vect->table[i];
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
 * TODO:
Inserts the given element at the front of the Vector.
Ω(1), Θ(1), O(n)
*/
void vect_push_front(Vector * const vect, const void * const data)
{
	if (data == NULL)
	{
		ds_error(DS_MSG_NULL_PTR);
		return;
	}

	/* Check if we need to increase the array's capacity. */
	if (vect->start == 0)
		vect_grow(vect);

	/* If the Vector is empty, we can't let the decrement happen.
	Otherwise, our `start` will be over NULL indexes. */
	if (!vect_empty(vect))
		vect->start--;
	vect->table[vect->start] = data;
	vect->size++;
}

/* 
 * TODO:
Removes the element at the end of the Vector.
Ω(1), Θ(1), O(n)
*/
void vect_pop_back(Vector * const vect)
{
	if (vect_empty(vect))
	{
		ds_error(DS_MSG_EMPTY);
		return;
	}
	
	if (vect->end > vect->start)
		vect->end--;
	vect->size--;
}

/* 
Removes the element at the front of the Vector.
Ω(1), Θ(1), O(1)
*/
void vect_pop_front(Vector * const vect)
{
	if (vect_empty(vect))
	{
		ds_error(DS_MSG_EMPTY);
		return;
	}

	if (vect->start < vect->end)
		vect->start++;
	vect->size--;
}

/*
 * Removes all elements from the Vector while preserving the capacity.
 * Ω(1), Θ(1), O(1)
 */
void vect_clear(Vector * const vect)
{
	const unsigned int middle = vect_middle(vect);
	vect->start = middle;
	vect->end = middle;
	// TODO: vect->table[middle] = NULL;
	vect->size = 0;
}

/*
 *TODO:
Sorts the array in descending order using the `compare` function.
Implementation uses Merge Sort.
Ω(n log(n)), Θ(n log(n)), O(n log(n))
*/
void vect_sort(Vector * const vect)
{
	void** arr = vect_array(vect);
	void** sorted = NULL;// vect_merge_sort(vect->compare, arr, vect->size);
	ds_free(arr, vect->size * sizeof(void*));
	 
	/* Place all of the sorted values into the middle of the Vector. */
	vect->start = vect_middle(vect) - (vect->size - 1) / 2;
	vect->end = vect->start + vect->size - 1;
	for (unsigned int i = vect->start, h = 0; i <= vect->end; i++)
		vect->table[i] = sorted[h++];
}

/* 
De-constructor function. 
Ω(1), Θ(1), O(1)
*/
void vect_destroy(const Vector * const vect)
{
	ds_free(vect->table, vect->capacity * sizeof(void*));
	ds_free(vect, sizeof(Vector));
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
 * TODO:
Grows the underlying array by a factor of `VECTOR_GROW_AMOUNT`.
Values from the previous Vector are placed in the middle of the new Vector.
Ω(n), Θ(n), O(n)
*/
void vect_grow(Vector * const vect)
{
	/* Expand the underlying array by a set amount. */
	const size_t expanded_capacity = vect->capacity * VECTOR_GROW_AMOUNT;
	const void** const expanded_table = ds_calloc(expanded_capacity, sizeof(void*));

	/* Place the values from the old table into the middle of the larger one. */
	const unsigned int expanded_start = (expanded_capacity - 1) / 2 - (vect->size - 1) / 2;
	unsigned int expanded_iter = !vect_empty(vect) ? expanded_start - 1 : expanded_start;
	for (unsigned int i = 0; i < vect_size(vect); i++)
		expanded_table[++expanded_iter] = vect_at(vect, i);

	/* Destroy the old table. */
	ds_free(vect->table, vect->capacity * sizeof(void*));
	/* Update the Vector's properties. */
	vect->table = expanded_table;
	vect->capacity = expanded_capacity;
	vect->start = expanded_start;
	vect->end = expanded_iter;
}

/*
 * Locates the center of the Vector, preferring the left in even capacities.
 * Ω(1), Θ(1), O(1)
 */
unsigned int vect_middle(const Vector * const vect)
{
	return (vect->capacity - 1) / 2;
}

/*
 * Converts regular indexes from 0->(capacity - 1) into indexes that wrap around the Vector. 
 * Ω(1), Θ(1), O(1)
 */
unsigned int vect_index(const Vector * const vect, const unsigned int index)
{
	return (vect->start + index) % vect->capacity;
}

/*
 * Given a value and a change in that value (dX), find the sum and wrap it between lower and upper.
 * Ω(1), Θ(1), O(1)
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
* Swap function for sorting and shifting algorithms.
* Ω(1), Θ(1), O(1)
*/
void swap(const void** const v1, const void** const v2)
{
	const void* const temp = *v1;
	*v1 = *v2;
	*v2 = temp;
}

/* TODO: Helper function for vect_sort. */
void** vect_merge_sort(int(*compare)(const void*, const void*), void** const arr, const size_t size)
{
	if (size <= 1)
		return arr;
	/* Divison ceiling for unsigned int. */
	const size_t left_size = (size + 2 - 1) / 2;
	/* Right array will default to be smaller if odd. */
	const size_t right_size = size / 2;

	/* Split the parameter array into two smaller arrays. */
	void **left_arr = ds_malloc(sizeof(void*) * left_size);
	void **right_arr = ds_malloc(sizeof(void*) * right_size);
	for (unsigned int i = 0; i < size; i++)
		if (i < left_size)
			left_arr[i] = arr[i];
		else
			right_arr[i - left_size] = arr[i];

	/* Sort the sub-arrays recursively. */
	void** const left_sorted = vect_merge_sort(compare, left_arr, left_size);
	void** const right_sorted = vect_merge_sort(compare, right_arr, right_size);

	/* Place the sorted elements back into the parameter array in sorted order. */
	unsigned int i = 0, left_iter = 0, right_iter = 0;
	while (left_iter < left_size && right_iter < right_size)
		if (compare(left_arr[left_iter], right_arr[right_iter]) <= 0)
			arr[i++] = left_arr[left_iter++];
		else
			arr[i++] = right_arr[right_iter++];
	while (left_iter < left_size)
		arr[i++] = left_arr[left_iter++];
	while (right_iter < right_size)
		arr[i++] = right_arr[right_iter++];

	/* Clean up memory and return the sorted array. */
	ds_free(left_sorted, left_size * sizeof(void*));
	ds_free(right_sorted, right_size * sizeof(void*));
	return arr;
}
