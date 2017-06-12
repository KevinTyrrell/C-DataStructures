
/*
 * File: IO.h
 * Date: Jun 01, 2017
 * Name: Kevin Tyrrell
 * Version: 1.0.0
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

#pragma once

#include <conio.h>
#include <windows.h>
#include <stdbool.h>
#include <stdio.h>
#include <assert.h>

/* Assert Macro. */
#define io_assert(condition, msg) assert(condition && msg)

/* Error messages. */
#define IO_MSG_OUT_OF_BOUNDS "Requested index was out of bounds!"
#define IO_MSG_NULL_PTR "Unable to perform operation on NULL pointer!"
#define IO_MSG_EMPTY "Unable to perform this operation while the container is empty!"
#define IO_MSG_NOT_SUPPORTED "Unable to perform this operation with a NULL user-provided member function!"
#define IO_MSG_INVALID_SIZE "Unable to perform this operation with an invalid sizing parameter!"

#define IO_LIGHT_TEAL 11
#define IO_LIGHT_RED 12

/* ~~~~~ Input/Output ~~~~~ */

/* Displays an error message to the user. */
void io_error(const char* const message);
/* Returns true if the user selects yes to the displayed prompt. */
bool io_prompt(const char *const message);
/* Changes the color of the console window and returns the previous color. */
WORD io_color(const WORD color);