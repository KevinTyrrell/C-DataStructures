
#include "Memory.h"

#define MEM_MSG_INVALID_MEMORY "Memory blocks allocated does not match expected values for this operation!"
#define MEM_MSG_BLOCKS_UNAVAILABLE "Not enough memory to allocate for this variable!"

/* Track memory usage in order to make sure we free all allocated memory. */
size_t MEM_CURRENT_ALLOCATIONS = 0, MEM_TOTAL_ALLOCATIONS = 0, MEM_BLOCKS_ALLOCATED = 0;

/*
 * Memory allocation function.
 * Use this function instead of `malloc`.
 * Updates memory usage variables to show memory leaks.
 * Θ(1)
 */
void* mem_malloc(const size_t size)
{
    void* const block = malloc(size);
    if (block == NULL)
    {
        io_error(MEM_MSG_BLOCKS_UNAVAILABLE);
        return NULL;
    }
    MEM_CURRENT_ALLOCATIONS++;
    MEM_TOTAL_ALLOCATIONS++;
    MEM_BLOCKS_ALLOCATED += size;
    return block;
}

/*
 * Memory allocation function.
 * Use this function instead of `calloc`.
 * Updates memory usage variables to show memory leaks.
 * Θ(1)
 */
void* mem_calloc(const size_t items, const size_t size)
{
    void* const block = calloc(items, size);
    if (block == NULL)
    {
        io_error(MEM_MSG_BLOCKS_UNAVAILABLE);
        return NULL;
    }
    MEM_CURRENT_ALLOCATIONS++;
    MEM_TOTAL_ALLOCATIONS++;
    MEM_BLOCKS_ALLOCATED += size * items;
    return block;
}

/*
 * Memory allocation function.
 * Use this function instead of `realloc`.
 * Updates memory usage variables to show memory leaks.
 * Θ(1)
 */
void* mem_realloc(void *const ptr, const size_t oldSize, const size_t newSize)
{
    void* const block = realloc(ptr, newSize);
    if (block == NULL)
        io_error(MEM_MSG_BLOCKS_UNAVAILABLE);
    MEM_BLOCKS_ALLOCATED += newSize;
    MEM_BLOCKS_ALLOCATED -= oldSize;
    return block;
}

/*
 * Memory de-allocation function.
 * Use this function instead of `free`.
 * Updates memory usage variables to show memory leaks.
 * size - Size of the pointer being freed.
 * Θ(1)
 */
void mem_free(void *const ptr, const size_t size)
{
    bool error = TRUE;
    if (ptr == NULL)
        io_error(IO_MSG_NULL_PTR);
    else if (MEM_CURRENT_ALLOCATIONS == 0 || size > MEM_BLOCKS_ALLOCATED)
        io_error(MEM_MSG_INVALID_MEMORY);
    else error = FALSE;
    if (error) return;

    free(ptr);
    MEM_CURRENT_ALLOCATIONS--;
    MEM_BLOCKS_ALLOCATED -= size;
}

/*
 * Prints out the status of the program's memory management.
 * Active allocations: calls to 'alloc'/'calloc', minus calls to `free`.
 * Blocks allocated: bytes of memory currently allocated.
 * Leakage: percentage of total memory that was allocated without being de-allocated.
 * Θ(1)
 */
void mem_status()
{
    const WORD stdColor = io_color(IO_LIGHT_TEAL);
    printf("Active allocations %-5lu Blocks allocated: %-10lu Leakage: %.2f%%\n",
           MEM_CURRENT_ALLOCATIONS, MEM_BLOCKS_ALLOCATED, 100.0 * MEM_CURRENT_ALLOCATIONS / MEM_TOTAL_ALLOCATIONS);
    io_color(stdColor);
}