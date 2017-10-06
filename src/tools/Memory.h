
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

/*
 * File Name:       Memory.h
 * File Author:     Kevin Tyrrell
 * Date Created:    06/01/2017
 */

#pragma once

#include "IO.h"

#include <stdlib.h>

/* ~~~~~ Memory Management ~~~~~ */

/* Memory allocation function. */
void* mem_malloc(const size_t size);
/* Memory allocation function.*/
void* mem_calloc(const size_t items, const size_t size);
/* Memory reallocation function. */
void* mem_realloc(void *const ptr, const size_t oldSize, const size_t newSize);
/* Memory de-allocation function. */
void mem_free(void *const ptr, const size_t size);
/* Prints out the status of the program's memory management. */
void mem_status();
