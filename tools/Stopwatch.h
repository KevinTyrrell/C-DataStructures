
/*
 * Author: Kevin Tyrrell
 * Date: 3/07/2016
 * Version: 2.0
 */

#pragma once

#include "Memory.h"

#include <time.h>

/* Private structure. */
typedef struct Stopwatch Stopwatch;

/* ~~~~~ Constructors ~~~~~ */

struct Stopwatch* Stopwatch_new();

/* ~~~~~ Accessors ~~~~~ */

/* Returns the amount of time clocked in milliseconds. */
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
