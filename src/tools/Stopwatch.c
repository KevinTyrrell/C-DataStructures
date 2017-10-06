
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
 * File Name:       Stopwatch.c
 * File Author:     Kevin Tyrrell
 * Date Created:    03/07/2017
 */

#include "Stopwatch.h"

#define SW_TO_MILLIS(val) val * 1000

#define SW_MSG_NOT_STARTED "The Stopwatch must have been started in order to perform this operation!"
#define SW_MSG_ALREADY_RUNNING "Unable to perform this operation while the Stopwatch is running!"
#define SW_MSG_ALREADY_ENDED "The Stopwatch has already been ended!"

/* Stopwatch structure. */
struct Stopwatch
{
    clock_t *start, *end;
};

/* Constructor function. */
Stopwatch* Stopwatch_new()
{
    return mem_calloc(1, sizeof(Stopwatch));
}

/*
 * Returns the amount of time clocked by the watch in milliseconds.
 * The watch must have been started and stopped before calling this function.
 * Θ(1)
 */
clock_t sw_elapsed(const Stopwatch* const sw)
{
    io_assert(sw != NULL, IO_MSG_NULL_PTR);
    io_assert(sw->start != NULL, SW_MSG_NOT_STARTED);
    io_assert(sw->end != NULL, SW_MSG_ALREADY_RUNNING);

    return SW_TO_MILLIS(*sw->end - *sw->start) / CLOCKS_PER_SEC;
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
    io_assert(sw != NULL, IO_MSG_NULL_PTR);
    io_assert(sw->start == NULL, SW_MSG_ALREADY_RUNNING);

    const clock_t time = clock();
    sw->start = mem_malloc(sizeof(clock_t));
    *sw->start = time;
}

/*
 * Ends the stopwatch and returns the elapsed time in milliseconds.
 * The watch should be started before calling this function.
 * See: sw_elapsed
 * Θ(1)
 */
clock_t sw_stop(struct Stopwatch* const sw)
{
    io_assert(sw != NULL, IO_MSG_NULL_PTR);
    io_assert(sw->start != NULL, SW_MSG_NOT_STARTED);
    io_assert(sw->end != NULL, SW_MSG_ALREADY_ENDED);

    const clock_t time = clock();
    sw->end = mem_malloc(sizeof(clock_t));
    *sw->end = time;
    return sw_elapsed(sw);
}

/*
 * Resets the stopwatch to its original state.
 * Θ(1)
 */
void sw_reset(Stopwatch* const sw)
{
    io_assert(sw != NULL, IO_MSG_NULL_PTR);
    /* No sense in resetting the watch if it hasn't been used. */
    io_assert(sw->start != NULL, SW_MSG_NOT_STARTED);

    mem_free(sw->start, sizeof(clock_t));
    sw->start = NULL;

    /* The user is allow to reset the watch before stopping it. */
    if (sw->end != NULL)
    {
        mem_free(sw->end, sizeof(clock_t));
        sw->end = NULL;
    }
}

/* De-constructor function. */
void sw_destroy(Stopwatch* const sw)
{
    io_assert(sw != NULL, IO_MSG_NULL_PTR);

    if (sw->start != NULL)
        mem_free(sw->start, sizeof(clock_t));
    if (sw->end != NULL)
        mem_free(sw->end, sizeof(clock_t));
    mem_free(sw, sizeof(Stopwatch));
}