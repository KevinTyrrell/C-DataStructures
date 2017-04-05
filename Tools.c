
#include "Tools.h"
#include <math.h>

#define DS_LIGHT_TEAL 11
#define DS_LIGHT_RED 12
#define VK_Y 0x59
#define VK_N 0x4E
#define Y_LC_KEY_CODE 121
#define Y_UC_KEY_CODE 89
#define N_LC_KEY_CODE 110
#define N_UC_KEY_CODE 78

#define DS_MSG_DEALLOCATE "Cannot de-allocate blocks that were not reported to have been allocated!\n"
#define DS_MSG_OUT_OF_MEM "Not enough memory to allocate for this variable!"

#define DS_EXIT "Exit Runtime"

/* Track memory usage in order to make sure we free all allocated memory. */
size_t MEM_CURRENT_ALLOCATIONS = 0, MEM_TOTAL_ALLOCATIONS = 0, MEM_BLOCKS_ALLOCATED = 0;

/* Print out a formatted error message to the console window. */
void ds_error(const char* const message)
{
	WORD prevColor = ds_changeColor(DS_LIGHT_RED);
	fprintf(stderr, "\n\n%s\n\n", message);
	/* Revert back to the previous color. */
	ds_changeColor(prevColor);
	if (ds_yesNo(DS_EXIT))
		exit(EXIT_FAILURE);
}

/* Prompt the user with a yes/no message
and return whether the user pressed Y (yes) as
true or N (no) as false. */
bool ds_yesNo(const char* const message)
{
	printf("%s (Y/N)?\n", message);

	int input;
	do
		input = _getch();
	while (input != Y_LC_KEY_CODE && input != Y_UC_KEY_CODE 
		&& input != N_LC_KEY_CODE && input != N_UC_KEY_CODE);

	return input == Y_LC_KEY_CODE || input == Y_UC_KEY_CODE;
}

/* Changes the color of the console window and returns the previous color. */
WORD ds_changeColor(const WORD color)
{
	WORD prevColor = 0;

	CONSOLE_SCREEN_BUFFER_INFO info;
	HANDLE console = GetStdHandle(STD_OUTPUT_HANDLE);

	if (GetConsoleScreenBufferInfo(console, &info))
	{
		prevColor = info.wAttributes;
		SetConsoleTextAttribute(console, color);
	}

	return prevColor;
}

/* Malloc function which tracks memory usage. */
void * ds_malloc(const size_t size)
{
	const void* const block = malloc(size);
	if (block == NULL)
		ds_error(DS_MSG_OUT_OF_MEM);
	MEM_CURRENT_ALLOCATIONS++;
	MEM_TOTAL_ALLOCATIONS++;
	MEM_BLOCKS_ALLOCATED += size;
	return block;
}

/* Calloc function which tracks memory usage. */
void * ds_calloc(const size_t nitems, const size_t size)
{	
	const void* const block = calloc(nitems, size);
	if (block == NULL)
		ds_error(DS_MSG_OUT_OF_MEM);
	MEM_CURRENT_ALLOCATIONS++;
	MEM_TOTAL_ALLOCATIONS++;
	MEM_BLOCKS_ALLOCATED += size * nitems;
	return block;
}

/* Realloc function which tracks memory usage. */
void * ds_realloc(const void * ptr, const size_t oldSize, const size_t newSize)
{
	const void* const block = realloc(ptr, newSize);
	if (block == NULL)
		ds_error(DS_MSG_OUT_OF_MEM);
	MEM_BLOCKS_ALLOCATED += newSize;
	MEM_BLOCKS_ALLOCATED -= oldSize;
	return block;
}

/* Free function which tracks memory usage. */
void ds_free(void * ptr, const size_t size)
{
	if (ptr == NULL)
	{
		ds_error(DS_MSG_NULL_PTR);
		return;
	}
	free(ptr);
	if (MEM_CURRENT_ALLOCATIONS == 0 || MEM_BLOCKS_ALLOCATED < size)
	{
		ds_error(DS_MSG_DEALLOCATE);
		return;
	}
	MEM_CURRENT_ALLOCATIONS--;
	MEM_BLOCKS_ALLOCATED -= size;
}

/* Print the current status of the memory management. */
void ds_printMemStatus()
{
	const WORD prevColor = ds_changeColor(DS_LIGHT_TEAL);
	printf("Active Allocations %-5zu Blocks Allocated: %-10zu Leakage: %.2f%%\n",
		MEM_CURRENT_ALLOCATIONS, MEM_BLOCKS_ALLOCATED, 100.0 * MEM_CURRENT_ALLOCATIONS / MEM_TOTAL_ALLOCATIONS);
	ds_changeColor(prevColor);
}
