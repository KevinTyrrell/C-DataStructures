
#include "Vector.h"

#define DEFAULT_INITIAL_CAPACITY 10
#define VECTOR_GROW_AMOUNT 2

/* Vector structure. */
struct Vector
{
	const void** table;
	/* Start and end let us know where the data is. */
	unsigned int start, end;
	size_t size, capacity;

	/* Synchronization. */
	ReadWriteSync *rw_sync;

	/* Function pointers. */
	int(*compare)(const void*, const void*);
	char*(*toString)(const void*);
};

/* Convenience functions. */
static bool vect_full(const Vector* const vect);
static void vect_grow(Vector* const vect);
static void vect_swap(const Vector* const vect, const unsigned int i, const unsigned int h);
static void ptr_swap(const void** const v1, const void** const v2);
static void vect_merge_sort(const Vector* const vect, const unsigned int start, const size_t size);
static void vect_quick_sort(const Vector* const vect, const unsigned int index, const size_t size);
static void vect_shift(Vector* const vect, const unsigned int start, const unsigned int stop, const bool leftwards);
static unsigned int vect_index(const Vector* const vect, const unsigned int index);
static unsigned int wrap_add(unsigned int val, int dx, const unsigned int lower, const unsigned int upper);
static bool vect_in_domain(const Vector* const vect, const unsigned int arr_index);

/* Structure to assist in looping through Vector. */
typedef struct
{
	unsigned int index;
	const Vector* const ref;
} Iterator;

/* Iterator functionality. */
static void vect_iter_next(Iterator* const iter);
static void vect_iter_prev(Iterator* const iter);
static bool vect_iter_has_next(const Iterator* const iter);
static bool vect_iter_has_prev(const Iterator* const iter);

/* Constructor function. */
Vector* Vector_new(int(*compare)(const void*, const void*), char*(*toString)(const void*))
{
	Vector* const vect = mem_calloc(1, sizeof(Vector));
	vect->table = mem_calloc(DEFAULT_INITIAL_CAPACITY, sizeof(void*));
	vect->capacity = DEFAULT_INITIAL_CAPACITY;
	vect->compare = compare;
	vect->toString = toString;
    vect->rw_sync = ReadWriteSync_new();
	return vect;
}

/*
 * Returns the value at the given index.
 * Θ(1)
 */
void* vect_at(const Vector* const vect, const unsigned int index)
{
	/* Lock the data structure to future writers. */
	sync_read_start(vect->rw_sync);

	void *val = NULL;

	if (vect->size == 0)
		io_error(IO_MSG_EMPTY);
	else if (index >= vect->size)
		io_error(IO_MSG_OUT_OF_BOUNDS);
	else
	{
		/* Wrap around the table if the index exceeds the capacity. */
		val = (void*)vect->table[vect_index(vect, index)];
	}

	/* Unlock the data structure. */
	sync_read_end(vect->rw_sync);

	return val;
}

/*
 * Returns the value at the front of the Vector.
 * This function is synchronized.
 * Θ(1)
 */
void* vect_front(const Vector* const vect)
{
	/* Lock the data structure to future writers. */
	sync_read_start(vect->rw_sync);

	void *val = NULL;
    
	if (vect->size > 0)
		val = (void*)vect->table[vect->start];
	else io_error(IO_MSG_EMPTY);

	/* Unlock the data structure. */
	sync_read_end(vect->rw_sync);

	return val;
}

/*
 * Returns the value at the end of the Vector.
 * This function is synchronized.
 * Θ(1)
 */
void* vect_back(const Vector* const vect)
{
	/* Lock the data structure to future writers. */
	sync_read_start(vect->rw_sync);

	void *val = NULL;

	if (vect->size > 0)
		val = (void*)vect->table[vect->end];
	else io_error(IO_MSG_EMPTY);

	/* Unlock the data structure. */
	sync_read_end(vect->rw_sync);

	return val;
}

/*
 * Returns the size of the Vector.
 * Θ(1)
 */
size_t vect_size(const Vector* const vect)
{
	/* Lock the data structure to future writers. */
	sync_read_start(vect->rw_sync);

	const size_t size = vect->size;

	/* Unlock the data structure. */
	sync_read_end(vect->rw_sync);

	return size;
}

/*
 * Returns true if the Vector is empty.
 * Θ(1)
 */
bool vect_empty(const Vector* const vect)
{
	/* Lock the data structure to future writers. */
	sync_read_start(vect->rw_sync);

	const bool empty = vect->size == 0;

	/* Unlock the data structure. */
	sync_read_end(vect->rw_sync);

	return empty;
}

/*
 * Returns true if the Vector contains the provided element.
 * Θ(n)
 */
bool vect_contains(const Vector* const vect, const void* const data)
{
	bool success = false;

	/* Lock the data structure to future writers. */
	sync_read_start(vect->rw_sync);

	for (unsigned int i = 0, size = vect->size; i < size; i++)
		if (vect->compare(vect_at(vect, i), data) == 0)
		{
			success = true;
			break;
		}

	/* Unlock the data structure. */
	sync_read_end(vect->rw_sync);

	return success;
}

/*
 * Returns an array of all elements inside the Vector.
 * Remember to call `free` on the dynamically allocated array.
 * Θ(n)
 */
void** vect_array(const Vector* const vect)
{
	/* Lock the data structure to future writers. */
	sync_read_start(vect->rw_sync);

	void** const arr = calloc(vect->size, sizeof(void*));
	for (unsigned int i = 0, size = vect->size; i < size; i++)
		arr[i] = vect_at(vect, i);

	/* Unlock the data structure. */
	sync_read_end(vect->rw_sync);

	return arr;
}

/*
 * Prints out the contents of the Vector using the toString function.
 * Θ(n)
 */
void vect_print(const Vector* const vect)
{
	/* Lock the data structure to future writers. */
	sync_read_start(vect->rw_sync);

	printf("%c", '[');
	for (unsigned int i = 0; i < vect->size; i++)
		printf("%s%s", vect->toString(vect_at(vect, i)),
			i + 1 < vect->size ? ", " : "");
	printf("]\n");

	/* Unlock the data structure. */
	sync_read_end(vect->rw_sync);
}

/*
 * Prints out the internal structure of the inner array.
 * TODO: Determine fate of this function.
 * Θ(n)
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
		printf("     ");
		const WORD oldColor = io_color(elementColor);
		for (unsigned int h = 0; h < INDEXES_PER_ROW && iter < vect->capacity; h++)
		{
			const void* const value = vect->table[iter++];
			printf(" %.1s  ", value != NULL ? vect->toString(value) : ".");
		}
		io_color(oldColor);

		printf("\nR%u   ", i % 10);
		/* Print indexes at the bottom of the row to indicate where we are in the array. */
		for (unsigned int h = 0; h < INDEXES_PER_ROW; h++)
			printf("[%u] ", h % 10);
		printf("\n");
	}

	printf("Vector Status -- Size: %zu, Capacity: %zu, Start: %u, End: %u.\n", vect->size, vect->capacity, vect->start, vect->end);
}

/*
 * Replaces an element in the Vector at a given index with a specified value.
 * Θ(1)
 */
void vect_assign(const Vector* const vect, const unsigned int index, const void* const data)
{
	if (data == NULL)
	{
		io_error(IO_MSG_NULL_PTR);
		return;
	}

	/* Lock the data structure to future readers/writers. */
	sync_write_start(vect->rw_sync);

	if (vect->size == 0)
		io_error(IO_MSG_EMPTY);
	else if (index >= vect->size)
		io_error(IO_MSG_OUT_OF_BOUNDS);
	else
		vect->table[vect_index(vect, index)] = data;

	/* Unlock the data structure. */
	sync_write_end(vect->rw_sync);
}

/*
 * Inserts the given element at the provided index.
 * Ω(1), O(n)
 */
void vect_insert(Vector* const vect, const unsigned int index, const void* const data)
{
	if (data == NULL)
	{
		io_error(IO_MSG_NULL_PTR);
		return;
	}

	/* Lock the data structure to future readers/writers. */
	sync_write_start(vect->rw_sync);

	if (index > vect->size)
		io_error(IO_MSG_OUT_OF_BOUNDS);
	else if (index == 0)
		vect_push_front(vect, data);
	else if (index == vect->size)
		vect_push_back(vect, data);
	else
	{
		if (vect_full(vect))
			vect_grow(vect);

		/* Check if shifting right is quicker. */
		if (vect->size - 1 - index <= index)
		{
			vect->end = wrap_add(vect->end, 1, 0, vect->capacity - 1);
			vect_shift(vect, vect_index(vect, index), vect->end, false);
		}
		else
		{
			vect->start = wrap_add(vect->start, -1, 0, vect->capacity - 1);
			vect_shift(vect, vect_index(vect, index), vect->start, true);
		}

		vect->size++;
		vect_assign(vect, index, data);
	}

	/* Unlock the data structure. */
	sync_write_end(vect->rw_sync);
}

/*
 * Attempts to remove a provided element from the Vector, if it exists.
 * Θ(n)
 */
bool vect_remove(Vector* const vect, const void* const data)
{
	if (data == NULL)
	{
		io_error(IO_MSG_NULL_PTR);
		return false;
	}

	bool success = false;

	/* Lock the data structure to future readers/writers. */
	sync_write_start(vect->rw_sync);

	const unsigned size = vect->size;
	for (unsigned int i = 0; i < size; i++)
		/* Search for a data element that matches the parameter. */
		if (vect->compare(vect_at(vect, i), data) == 0)
		{
			vect_erase(vect, i);
			success = true;
			break;
		}

	/* Unlock the data structure. */
	sync_write_end(vect->rw_sync);

	return success;
}

/*
 * Erases an element from the Vector at a given index.
 * Ω(1), O(n)
 */
void vect_erase(Vector* const vect, const unsigned int index)
{
	/* Lock the data structure to future readers/writers. */
	sync_write_start(vect->rw_sync);

	if (index > vect->size)
		io_error(IO_MSG_OUT_OF_BOUNDS);
	else if (index == 0)
		vect_pop_front(vect);
	else if (index == vect->size - 1)
		vect_pop_back(vect);
	else
	{
		/* Check if shifting left is quicker. */
		if (vect->size - 1 - index <= index)
		{
			vect_shift(vect, vect->end, vect_index(vect, index), true);
			vect->end = wrap_add(vect->end, -1, 0, vect->capacity - 1);
		}
		else
		{
			vect_shift(vect, vect->start, vect_index(vect, index), false);
			vect->start = wrap_add(vect->start, 1, 0, vect->capacity - 1);
		}

		vect->size--;
	}

	/* Unlock the data structure. */
	sync_write_end(vect->rw_sync);
}

/*
 * Inserts the given element at the end of the Vector.
 * Ω(1), O(n)
 */
void vect_push_back(Vector * const vect, const void * const data)
{
	if (data == NULL)
	{
		io_error(IO_MSG_NULL_PTR);
		return;
	}

	/* Lock the data structure to future readers/writers. */
	sync_write_start(vect->rw_sync);

	/* Check if we need to increase the array's capacity. */
	if (vect_full(vect))
		vect_grow(vect);

	/* When Vector has one or less element(s), start and end must point to the same index. */
	if (!vect_empty(vect))
		/* Increment end and wrap. */
		vect->end = wrap_add(vect->end, 1, 0, vect->capacity - 1);
	
	vect->table[vect->end] = data;
	vect->size++;

	/* Unlock the data structure. */
	sync_write_end(vect->rw_sync);
}

/*
 * Inserts the given element at the front of the Vector.
 * Ω(1), O(n)
 */
void vect_push_front(Vector * const vect, const void *const data)
{
	if (data == NULL)
	{
		io_error(IO_MSG_NULL_PTR);
		return;
	}

	/* Lock the data structure to future readers/writers. */
	sync_write_start(vect->rw_sync);

	/* Check if we need to increase the array's capacity. */
	if (vect_full(vect))
		vect_grow(vect);

	/* When Vector has one or less element(s), start and end must point to the same index. */
	if (!vect_empty(vect))
		/* Increment end and wrap. */
		vect->start = wrap_add(vect->start, -1, 0, vect->capacity - 1);

	vect->table[vect->start] = data;
	vect->size++;

	/* Unlock the data structure. */
	sync_write_end(vect->rw_sync);
}

/*
 * Removes the element at the end of the Vector.
 * Θ(1)
 */
void vect_pop_back(Vector* const vect)
{
	/* Lock the data structure to future readers/writers. */
	sync_write_start(vect->rw_sync);

	if (vect->size > 0)
	{
		/* When Vector has one or less element(s), start and end must point to the same index. */
		if (vect->size-- > 1)
			vect->end = wrap_add(vect->end, -1, 0, vect->capacity - 1);
	}
	else io_error(IO_MSG_EMPTY);

	/* Unlock the data structure. */
	sync_write_end(vect->rw_sync);
}

/*
* Removes the element at the front of the Vector.
* Θ(1)
*/
void vect_pop_front(Vector* const vect)
{
	/* Lock the data structure to future readers/writers. */
	sync_write_start(vect->rw_sync);

	if (vect->size > 0)
	{
		/* When Vector has one or less element(s), start and end must point to the same index. */
		if (vect->size-- > 1)
			vect->start = wrap_add(vect->start, 1, 0, vect->capacity - 1);
	}
	else io_error(IO_MSG_EMPTY);

	/* Unlock the data structure. */
	sync_write_end(vect->rw_sync);
}

/*
 * Append data from another Vector to the end of this Vector.
 * Θ(n)
 */
void vect_append(Vector* const vect, const Vector* const other)
{
	/* Lock the other structure to future readers/writers. */
	sync_write_start(vect->rw_sync);

	const size_t combined = vect->size + other->size;
	if (vect->capacity < combined)
		vect_grow_to(vect, combined);
	for (unsigned int i = 0, size = other->size; i < size; i++)
		vect_push_back(vect, vect_at(other, i));

	/* Unlock the other structure. */
	sync_write_end(vect->rw_sync);
}

/*
* Append data from an array to the end of this Vector.
* Θ(n)
*/
void vect_append_array(Vector* const vect, const void** const data, const size_t size)
{
	/* Lock the data structure to future readers/writers. */
	sync_write_start(vect->rw_sync);

	/* Grow the vector to accommodate current and appended elements. */
	const size_t combined = vect->size + size;
	if (vect->capacity < combined)
		vect_grow_to(vect, combined);
	for (unsigned int i = 0; i < size; i++)
		vect_push_back(vect, data[i]);

	/* Unlock the data structure. */
	sync_write_end(vect->rw_sync);
}

/*
 * Removes all elements from the Vector while preserving the capacity.
 * Θ(1)
 */
void vect_clear(Vector* const vect)
{
	/* Lock the data structure to future readers/writers. */
	sync_write_start(vect->rw_sync);

	vect->start = 0;
	vect->end = 0;
	vect->size = 0;

	/* Unlock the data structure. */
	sync_write_end(vect->rw_sync);
}

/*
 * Sorts elements inside the Vector in ascending order.
 * Uses the `compare` function provided to the Vector.
 * See: `vect_quick_sort`
 */
void vect_sort(const Vector* const vect)
{
	/* Lock the data structure to future readers/writers. */
	sync_write_start(vect->rw_sync);

	vect_quick_sort(vect, 0, vect->size);

	/* Unlock the data structure. */
	sync_write_end(vect->rw_sync);
}

/*
 * Shuffles the elements inside the Vector randomly.
 * Utilizes the Fisher-Yates Shuffling Algorithm:
 * (https://en.wikipedia.org/wiki/Fisher%E2%80%93Yates_shuffle)
 * Θ(n)
 */
void vect_shuffle(const Vector* const vect)
{
    /* Lock the data structure to future readers/writers. */
	sync_write_start(vect->rw_sync);

	const size_t size = vect->size;
	for (unsigned int i = size - 1; i > 0; i--)
	{
		const unsigned int swap_location = rand_limit(i + 1);
		vect_swap(vect, i, swap_location);
	}

    /* Unlock the data structure. */
    sync_write_end(vect->rw_sync);
}

/*
 * De-constructor function. 
 * Θ(1)
 */
void vect_destroy(const Vector* const vect)
{
	mem_free(vect->table, vect->capacity * sizeof(void*));
	sync_destroy(vect->rw_sync);
	mem_free((void*)vect, sizeof(Vector));
}

/*
 * Returns true if the Vector is full.
 * Θ(1)
 */
bool vect_full(const Vector* const vect)
{
	return vect->size == vect->capacity;
}

/*
 * Grows the underlying array by a factor of `VECTOR_GROW_AMOUNT`.
 * Θ(n)
 */
void vect_grow(Vector* const vect)
{
	/* Expand the underlying array by a set amount. */
	vect_grow_to(vect, vect->capacity * VECTOR_GROW_AMOUNT);
}

/*
 * Grows the underlying array to be able to store at least `min_size` elements.
 * The Vector's capacity will always be of the form `capacity = 10 * 2^n`
 * Θ(n)
 */
void vect_grow_to(Vector* const vect, const size_t min_size)
{
	/* Lock the data structure to future readers/writers. */
	sync_write_start(vect->rw_sync);

	/* Disregard invalid sizes. */
	if (min_size <= vect->capacity)
	{
		/* Unlock the data structure. */
		sync_write_end(vect->rw_sync);
		return;
	}

	/* 
	 * Solve for the expanded capacity.
	 * The Vector only expands in increments of base_capacity * 2^n.
	 * Ex. `min_size` of 73 means expanded capacity of 80 if base_capacity is 10.
	 */
	const size_t expanded_capacity = DEFAULT_INITIAL_CAPACITY * 
		(unsigned int)pow(VECTOR_GROW_AMOUNT, 1 + floor(
			log((double)min_size / DEFAULT_INITIAL_CAPACITY) / log(VECTOR_GROW_AMOUNT)));

	const void** const expanded_table = mem_calloc(expanded_capacity, sizeof(void*));
	for (unsigned int i = 0, len = vect->size; i < len; i++)
		expanded_table[i] = vect_at(vect, i);

	/* Destroy the old table. */
	mem_free(vect->table, vect->capacity * sizeof(void*));
	/* Update the Vector's properties. */
	vect->table = expanded_table;
	vect->capacity = expanded_capacity;
	vect->start = 0;
	vect->end = !vect_empty(vect) ? vect->size - 1 : 0;

	/* Unlock the data structure. */
	sync_write_end(vect->rw_sync);
}

/*
* Swap function for sorting and shifting algorithms.
* Θ(1)
*/
void vect_swap(const Vector* const vect, const unsigned int i, const unsigned int h)
{
	const void* const temp = vect_at(vect, i);
	vect_assign(vect, i, vect_at(vect, h));
	vect_assign(vect, h, temp);
}

/*
* Swap function for sorting and shifting algorithms.
* Θ(1)
*/
void ptr_swap(const void** const v1, const void** const v2)
{
	const void* const temp = *v1;
	*v1 = *v2;
	*v2 = temp;
}

/*
 * Converts regular indexes from 0->(capacity - 1) into indexes that wrap around the Vector. 
 * Θ(1)
 */
unsigned int vect_index(const Vector* const vect, const unsigned int index)
{
	return (vect->start + index) % vect->capacity;
}

/*
 * Given a value and a change in that value (dX), find the sum and wrap it between lower and upper.
 * Θ(n)
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
 * Returns true if the given array index is within start and end.
 * Θ(1)
 */
bool vect_in_domain(const Vector* const vect, const unsigned int arr_index)
{
	if (vect_empty(vect))
		return false;
	/* Standard array form. */
	if (vect->start <= vect->end)
		return arr_index >= vect->start && arr_index <= vect->end;
	/* Circular property is in effect. */
	return arr_index >= vect->start || arr_index <= vect->end;
}

/*
 * Sorts the Vector in ascending order using the Quicksort algorithm.
 * Ω(n * log(n)), O(n^2)
 */
void vect_quick_sort(const Vector* const vect, const unsigned int index, const size_t size)
{
	/* Array size of 1 means the partition is sorted. */
	if (size <= 1)
		return;
	/* Pivot in this implementation is the right element. */
	const unsigned int pivot_index = index + size - 1;
	const void* const pivot = vect_at(vect, pivot_index);

	/* Left and right iterators. */
	unsigned int left = index, right = pivot_index;
	
	while (true)
	{
		/* Move the indexes until they cross OR find swappable values. */
		while (left < right && vect->compare(vect_at(vect, left), pivot) < 0)
			left++;
		while (left < right && vect->compare(vect_at(vect, --right), pivot) > 0);

		if (left >= right)
			break;

		vect_swap(vect, left, right);
		left++;
	}
	
	vect_swap(vect, left, pivot_index);
	vect_quick_sort(vect, index, left - index);
	vect_quick_sort(vect, left + 1, index + size - left - 1);
}

/*
 * Sorts the Vector in ascending order using the Mergesort Algorithm.
 * An addition N space of memory will be needed for the merge step.
 * Θ(n * log(n))
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
	const void** const arr_left = mem_calloc(size_left, sizeof(void*));
	const void** const arr_right = mem_calloc(size_right, sizeof(void*));
	for (unsigned int i = 0; i < size_left; i++)
		arr_left[i] = vect_at(vect, start + i);
	for (unsigned int i = 0; i < size_right; i++)
		arr_right[i] = vect_at(vect, start_right + i);
	
	/* Maintain track of an iterator for the combined array and the two sub-arrays. */
	unsigned int iter = start, vect_iter_left = 0, vect_iter_right = 0;

	/* Merge the two sub-arrays back into the primary array. */
	while (vect_iter_left < size_left && vect_iter_right < size_right)
		if (vect->compare(arr_left[vect_iter_left], arr_right[vect_iter_right]) <= 0)
			vect_assign(vect, iter++, arr_left[vect_iter_left++]);
		else
			vect_assign(vect, iter++, arr_right[vect_iter_right++]);
	while (vect_iter_left < size_left)
		vect_assign(vect, iter++, arr_left[vect_iter_left++]);
	while (vect_iter_right < size_right)
		vect_assign(vect, iter++, arr_right[vect_iter_right++]);

	/* Clean up memory and return the sorted array. */
	mem_free(arr_left, size_left * sizeof(void*));
	mem_free(arr_right, size_right * sizeof(void*));
}

/*
 * Shifts the elements in the Vector left or right based on `leftwards`.
 * The shift will begin from `start` and the furthest element will be moved to `stop`.
 * Shifts may leave holes in the array and will not update `start` and `end` pointers of the Vector.
 * Θ(n)
 */
void vect_shift(Vector* const vect, const unsigned int start, const unsigned int stop, const bool leftwards)
{
	/* Iterator will aid in safely looping through array. */
	Iterator iter = { stop, vect };
	/* Shifting left means iterating rightwards and vice versa.
	 * Function pointer makes this process much easier to read. */
	void(*iterate)(Iterator* const);
	iterate = leftwards ? &vect_iter_next : &vect_iter_prev;
		
	/* Iterate and swap. */
	while (iter.index != start)
	{
		/* Swap the data with the iterator and its neighboring element. */
		const unsigned int a = iter.index;
		iterate(&iter);
		ptr_swap(&vect->table[a], &vect->table[iter.index]);
	}
}

/*
 * Moves the Iterator forward one index.
 * Θ(1)
 */
void vect_iter_next(Iterator* const iter)
{
	iter->index = wrap_add(iter->index, 1, 0, iter->ref->capacity - 1);		
}

/*
 * Moves the Iterator backward one index.
 * Θ(1)
 */
void vect_iter_prev(Iterator* const iter)
{
	iter->index = wrap_add(iter->index, -1, 0, iter->ref->capacity - 1);
}

/*
 * Returns true if the Iterator can move forward.
 * Θ(1)
 */
bool vect_iter_has_next(const Iterator* const iter)
{
	return iter->index != iter->ref->end;
}

/*
* Returns true if the Iterator can move backwards.
* Θ(1)
*/
bool vect_iter_has_prev(const Iterator* const iter)
{
	return iter->index != iter->ref->start;
}
