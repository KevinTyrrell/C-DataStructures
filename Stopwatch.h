#pragma once

#include "Tools.h"

#include <time.h>

struct Stopwatch
{
	clock_t *start, *end;
};

/* ~~~~~ Constructors ~~~~~ */

struct Stopwatch* Stopwatch_new();

/* ~~~~~ Accessors ~~~~~ */

/* Returns the elapsed time taken by the Stopwatch. */
clock_t sw_elapsed(const struct Stopwatch* const sw);
/* Returns the elapsed time taken by the Stopwatch in milliseconds. */
unsigned long sw_elapsed_millis(const struct Stopwatch * const sw);
/* Returns the difference of time between the given two Stopwatches. */
clock_t sw_difference(const struct Stopwatch* const sw1, const struct Stopwatch* const sw2);

/* ~~~~~ Mutators ~~~~~ */

/* Starts the Stopwatch. */
void sw_start(struct Stopwatch* const sw);
/* Stops the Stopwatch and returns the amount of clocks taken. */
clock_t sw_stop(struct Stopwatch* const sw);
/* Resets this Stopwatch to its' default state. */
void sw_reset(struct Stopwatch* const sw);

/* ~~~~~ De-constructors ~~~~~ */

void sw_destroy(const struct Stopwatch* const sw);
