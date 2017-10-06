
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
 * File Name:       Synchronize.c
 * File Author:     Kevin Tyrrell
 * Date Created:    06/01/2017
 */

#pragma once

#include "Memory.h"

#include <windows.h>
#include <stdbool.h>
#include <string.h>

/* Anonymous structure. */
typedef struct ReadWriteSync ReadWriteSync;

/* ~~~~~ Constructors ~~~~~ */

ReadWriteSync* ReadWriteSync_new();

/* ~~~~~ Mutators ~~~~~ */

/* Adds a reader to the scheduler, forbidding any future writing. */
void sync_read_start(ReadWriteSync* const rw_sync);
/* Removes a reader that was previously reading. */
void sync_read_end(ReadWriteSync* const rw_sync);
/* Adds a writer to the scheduler, forbidding any future reading/writing. */
void sync_write_start(ReadWriteSync* const rw_sync);
/* Removes a writer that was previously writing. */
void sync_write_end(ReadWriteSync* const rw_sync);

/* ~~~~~ De-constructors ~~~~~ */

void sync_destroy(ReadWriteSync* const rw_sync);