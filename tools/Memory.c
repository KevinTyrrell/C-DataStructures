
/*
 * File: Memory.c
 * Date: Jun 01, 2017
 * Name: Kevin Tyrrell
 * Version: 2.0.0
 */

/*
Copyright © 2017 Kevin Tyrrell

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include "Memory.h"

#define MEM_MSG_INVALID_BLOCK_SIZE "Memory block size was invalid!"
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
    io_assert(size > 0, MEM_MSG_INVALID_BLOCK_SIZE);

    void* const block = malloc(size);
    io_assert(block != NULL, MEM_MSG_BLOCKS_UNAVAILABLE);

    MEM_CURRENT_ALLOCATIONS++;
    MEM_TOTAL_ALLOCATIONS++;
    MEM_BLOCKS_ALLOCATED += size;
    return block;
}

/*
 * Memory allocation replacement.
 * This function should be used over `calloc`.
 * Updates memory usage variables to detect memory leaks.
 * Θ(1)
 */
void* mem_calloc(const size_t items, const size_t size)
{
    io_assert(items > 0, MEM_MSG_INVALID_BLOCK_SIZE);
    io_assert(size > 0, MEM_MSG_INVALID_BLOCK_SIZE);

    void* const block = calloc(items, size);
    io_assert(block != NULL, MEM_MSG_BLOCKS_UNAVAILABLE);

    MEM_CURRENT_ALLOCATIONS++;
    MEM_TOTAL_ALLOCATIONS++;
    MEM_BLOCKS_ALLOCATED += size * items;
    return block;
}

/*
 * Memory reallocation replacement.
 * This function should be used over `realloc`.
 * Updates memory usage variables to detect memory leaks.
 * The new/old size of the pointer being must be specified.
 * Θ(1)
 */
void* mem_realloc(void *const ptr, const size_t oldSize, const size_t newSize)
{
    io_assert(ptr != NULL, IO_MSG_NULL_PTR);
    io_assert(MEM_BLOCKS_ALLOCATED >= oldSize, MEM_MSG_INVALID_MEMORY);

    void* const block = realloc(ptr, newSize);
    io_assert(block != NULL, MEM_MSG_BLOCKS_UNAVAILABLE);

    MEM_BLOCKS_ALLOCATED -= oldSize;
    MEM_BLOCKS_ALLOCATED += newSize;

    return block;
}

/*
 * Memory de-allocation replacement.
 * This function should be used over `free`.
 * Updates memory usage variables to detect memory leaks.
 * The size of the pointer being freed must be specified.
 * Θ(1)
 */
void mem_free(void *const ptr, const size_t size)
{
    io_assert(ptr != NULL, IO_MSG_NULL_PTR);
    io_assert(size > 0, MEM_MSG_INVALID_BLOCK_SIZE);
    io_assert(MEM_BLOCKS_ALLOCATED >= size, MEM_MSG_INVALID_MEMORY);
    io_assert(MEM_CURRENT_ALLOCATIONS > 0, MEM_MSG_INVALID_MEMORY);

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
    printf("Active allocations %-5zu Blocks allocated: %-10zu Leakage: %.2f%%\n",
           MEM_CURRENT_ALLOCATIONS, MEM_BLOCKS_ALLOCATED,
           100.0 * MEM_CURRENT_ALLOCATIONS / MEM_TOTAL_ALLOCATIONS);
}