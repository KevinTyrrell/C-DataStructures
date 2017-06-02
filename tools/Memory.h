
/*
 * Author: Kevin Tyrrell
 * Date: 6/01/2017
 * Version: 1.0
 */

#pragma once

#include "IO.h"

#include <stdlib.h>

/* ~~~~~ Memory Management ~~~~~ */

/* Memory allocation function. */
void* mem_malloc(const size_t size);
/* Memory allocation function.*/
void* mem_calloc(const size_t items, const size_t size);
/* Memory allocation function. */
void* mem_realloc(void *const ptr, const size_t oldSize, const size_t newSize);
/* Memory de-allocation function. */
void mem_free(void *const ptr, const size_t size);
/* Prints out the status of the program's memory management. */
void mem_status();
