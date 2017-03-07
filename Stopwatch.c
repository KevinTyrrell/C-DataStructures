#include "Stopwatch.h"

#define SW_MSG_NOT_STARTED "The Stopwatch must have been started in order to perform this operation!"
#define SW_MSG_ALREADY_RUNNING "Unable to perform this operation while the Stopwatch is running! "
#define SW_MSG_ALREADY_ENDED "The Stopwatch has already been ended!"

/* Local typedef for convenience. */
typedef struct Stopwatch Stopwatch;

/* Constructor function. */
struct Stopwatch * Stopwatch_new()
{
	Stopwatch* const sw = ds_calloc(1, sizeof(Stopwatch));
	return sw;
}

/* Returns the elapsed time taken by the Stopwatch. */
clock_t sw_elapsed(const Stopwatch* const sw)
{
	bool error = true;
	if (sw->start == NULL)
		ds_error(SW_MSG_NOT_STARTED);
	else if (sw->end == NULL)
		ds_error(SW_MSG_ALREADY_RUNNING);
	else error = false;
	if (error) return 0;
	
	return *sw->end - *sw->start;
}

/* Returns the elapsed time taken by the Stopwatch in milliseconds. */
unsigned long sw_elapsed_millis(const Stopwatch* const sw)
{
	return 1000 * sw_elapsed(sw) / CLOCKS_PER_SEC;
}

/* Returns the difference of time between the given two Stopwatches. */
clock_t sw_difference(const Stopwatch * const sw1, const Stopwatch * const sw2)
{
	bool error = true;
	if (sw1->start == NULL || sw2->start == NULL)
		ds_error(SW_MSG_NOT_STARTED);
	else if (sw2->end == NULL || sw2->end == NULL)
		ds_error(SW_MSG_ALREADY_RUNNING);
	else error = false;
	if (error) return 0;

	return sw_elapsed(sw1) - sw_elapsed(sw2);
}

/* Starts the Stopwatch. */
void sw_start(struct Stopwatch * const sw)
{
	if (sw->start != NULL)
	{
		ds_error(SW_MSG_ALREADY_RUNNING);
		return;
	}
	sw->start = ds_malloc(sizeof(clock_t));
	*sw->start = clock();
}

/* Stops the Stopwatch and returns the amount of clocks taken. */
clock_t sw_stop(struct Stopwatch * const sw)
{
	bool error = true;
	if (sw->start == NULL)
		ds_error(SW_MSG_NOT_STARTED);
	else if (sw->end != NULL)
		ds_error(SW_MSG_ALREADY_ENDED);
	else error = false;
	if (error) return 0;

	sw->end = ds_malloc(sizeof(clock_t));
	*sw->end = clock();
	return sw_elapsed(sw);
}

/* Resets this Stopwatch to its' default state. */
void sw_reset(struct Stopwatch * const sw)
{
	if (sw->start == NULL)
	{
		ds_error(SW_MSG_NOT_STARTED);
		return;
	}
	ds_free(sw->start, sizeof(clock_t));
	sw->start = NULL;
	
	if (sw->end != NULL)
	{
		ds_free(sw->end, sizeof(clock_t));
		sw->end = NULL;
	}
}

/* De-constructor function. */
void sw_destroy(const struct Stopwatch * const sw)
{
	if (sw->start != NULL)
		ds_free(sw->start, sizeof(clock_t));
	if (sw->end != NULL)
		ds_free(sw->end, sizeof(clock_t));
	ds_free(sw, sizeof(Stopwatch));
}
