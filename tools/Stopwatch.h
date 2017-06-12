
/*
 * File: Stopwatch.h
 * Date: Mar 07, 2017
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

#pragma once

#include "Memory.h"

#include <time.h>

/* Private structure. */
typedef struct Stopwatch Stopwatch;

/* ~~~~~ Constructors ~~~~~ */

struct Stopwatch* Stopwatch_new();

/* ~~~~~ Accessors ~~~~~ */

/* Returns the amount of time clocked by the watch in milliseconds. */
clock_t sw_elapsed(const Stopwatch* const sw);
/* Returns the difference between two stopwatches in milliseconds. */
clock_t sw_difference(const Stopwatch* const sw1, const Stopwatch* const sw2);

/* ~~~~~ Mutators ~~~~~ */

/* Starts the stopwatch. */
void sw_start(Stopwatch* const sw);
/* Ends the stopwatch and returns the elapsed time in milliseconds. */
clock_t sw_stop(Stopwatch* const sw);
/* Resets the stopwatch to its original state. */
void sw_reset(Stopwatch* const sw);

/* ~~~~~ De-constructors ~~~~~ */

void sw_destroy(Stopwatch* const sw);