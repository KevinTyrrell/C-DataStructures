#pragma once
#pragma warning( disable : 4090 )

#include <stdio.h>
#include <Windows.h>
#include <conio.h>
#include <stdbool.h>

#define DS_MSG_OUT_OF_BOUNDS "Requested index was out of bounds!"
#define DS_MSG_NULL_PTR "Unable to perform operation on NULL pointer!"
#define DS_MSG_EMPTY "Cannot perform this operation while the container is empty!"
#define DS_MSG_INVALID "Cannot perform this operation as the structure is marked as invalid!"

/* Print out a formatted error message to the console window. */
void ds_error(const char* const message);
/* Prompt the user to press the Y or N key. */
bool ds_yesNo(const char* const message);
/* Changes the color of the console window and returns the previous color. */
WORD ds_changeColor(const WORD color);

/* Malloc function which tracks memory usage. */
void* ds_malloc(const size_t size);
/* Calloc function which tracks memory usage. */
void* ds_calloc(const size_t nitems, const size_t size);
/* Realloc function which tracks memory usage. */
void* ds_realloc(const void *ptr, const size_t oldSize, const size_t newSize);
/* Free function which tracks memory usage. */
void ds_free(void *ptr, const size_t size);
/* Print the current status of the memory management. */
void ds_printMemStatus();

