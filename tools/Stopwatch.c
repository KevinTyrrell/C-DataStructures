
#include "Stopwatch.h"

#define SW_MSG_NOT_STARTED "The Stopwatch must have been started in order to perform this operation!"
#define SW_MSG_ALREADY_RUNNING "Unable to perform this operation while the Stopwatch is running! "
#define SW_MSG_ALREADY_ENDED "The Stopwatch has already been ended!"

#define TO_MILLISECONDS 1000

/* Stopwatch structure. */
struct Stopwatch
{
	clock_t *start, *end;
};

/* Constructor function. */
Stopwatch* Stopwatch_new()
{
	Stopwatch* const sw = mem_calloc(1, sizeof(Stopwatch));
	return sw;
}

/*
 * Returns the amount of time clocked in milliseconds.
 * The watch must have been started and stopped before calling this function.
 * Θ(1)
 */
clock_t sw_elapsed(const Stopwatch* const sw)
{
	bool error = TRUE;
	if (sw->start == NULL)
		io_error(SW_MSG_NOT_STARTED);
	else if (sw->end == NULL)
		io_error(SW_MSG_ALREADY_RUNNING);
	else error = FALSE;
	if (error) return -1;
	
	return TO_MILLISECONDS * (*sw->end - *sw->start) / CLOCKS_PER_SEC;
}

/*
 * Returns the difference between two stopwatches in milliseconds.
 * The duration returned will be positive if the first watch
 * clocked more time than the second, and vice-versa.
 * Both watches must be started and stopped before calling this function.
 * Θ(1)
 */
clock_t sw_difference(const Stopwatch* const sw1, const Stopwatch* const sw2)
{
	return sw_elapsed(sw1) - sw_elapsed(sw2);
}

/*
 * Starts the stopwatch.
 * A stopwatch can only be started again after being reset.
 * Θ(1)
 */
void sw_start(struct Stopwatch * const sw)
{
	if (sw->start != NULL)
	{
		io_error(SW_MSG_ALREADY_RUNNING);
		return;
	}
	sw->start = mem_malloc(sizeof(clock_t));
	*sw->start = clock();
}

/*
 * Ends the stopwatch and returns the elapsed time in milliseconds.
 * The watch should be started before calling this function.
 * For the return value, see: Elapsed.
 * Θ(1)
 */
clock_t sw_stop(struct Stopwatch * const sw)
{
	bool error = TRUE;
	if (sw->start == NULL)
		io_error(SW_MSG_NOT_STARTED);
	else if (sw->end != NULL)
		io_error(SW_MSG_ALREADY_ENDED);
	else error = FALSE;
	if (error) return -1;

	sw->end = mem_malloc(sizeof(clock_t));
	*sw->end = clock();
	return sw_elapsed(sw);
}

/*
 * Resets the stopwatch to its original state.
 * Θ(1)
 */
void sw_reset(Stopwatch* const sw)
{
	if (sw->start == NULL)
	{
		io_error(SW_MSG_NOT_STARTED);
		return;
	}
	mem_free(sw->start, sizeof(clock_t));
	sw->start = NULL;
	
	if (sw->end != NULL)
	{
		mem_free(sw->end, sizeof(clock_t));
		sw->end = NULL;
	}
}

/* De-constructor function. */
void sw_destroy(Stopwatch* const sw)
{
	if (sw->start != NULL)
		mem_free(sw->start, sizeof(clock_t));
	if (sw->end != NULL)
		mem_free(sw->end, sizeof(clock_t));
	mem_free(sw, sizeof(Stopwatch));
}
