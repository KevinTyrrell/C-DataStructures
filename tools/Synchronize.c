
/*
 * File: Synchronize.c
 * Date: Jun 01, 2017
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

#include "Synchronize.h"

#define SYNC_MSG_MUTEX_FAIL "Mutex creation failed: "
#define SYNC_MSG_NO_READERS "Unable to stop reading as there are no readers!"

const bool SYNC_DEBUG_MODE = false;

/*
 * Structure to assist in synchronized reading/writing.
 * Each data structure will contain this structure.
 * This structure controls the ability for functions
 * to proceed depending on if they are readers or writers.
 */
struct ReadWriteSync
{
    /* Synchronization variables. */
    HANDLE mutex_primary, mutex_auxiliary;
    /* How many readers are reading this structure. */
    unsigned int readers;
};

/* Constructor function. */
ReadWriteSync* ReadWriteSync_new()
{
    ReadWriteSync* const rw_sync = mem_calloc(1, sizeof(ReadWriteSync));

    /* Default security, initially not owned, unnamed. */
    rw_sync->mutex_primary = CreateMutex(NULL, FALSE, NULL);
    if (rw_sync->mutex_primary == NULL)
        fprintf(stderr, "%s%lu", SYNC_MSG_MUTEX_FAIL, GetLastError());
    rw_sync->mutex_auxiliary = CreateMutex(NULL, FALSE, NULL);
    if (rw_sync->mutex_auxiliary == NULL)
        fprintf(stderr, "%s%lu", SYNC_MSG_MUTEX_FAIL, GetLastError());

    io_assert(rw_sync->mutex_primary != NULL, IO_MSG_NULL_PTR);
    io_assert(rw_sync->mutex_auxiliary != NULL, IO_MSG_NULL_PTR);

    return rw_sync;
}

/*
 * Adds a reader to the scheduler, forbidding any future writing.
 * Remember to call `sync_read_end` to release the mutex.
 * Θ(1)
 */
void sync_read_start(ReadWriteSync* const rw_sync)
{
    io_assert(rw_sync != NULL, IO_MSG_NULL_PTR);

    if (SYNC_DEBUG_MODE) printf("Thread %lu is waiting for aux mutex.\n", GetCurrentThreadId());
    /* Grab mutex so we can increment readers. */
    WaitForSingleObject(rw_sync->mutex_auxiliary, INFINITE);
    if (SYNC_DEBUG_MODE) printf("Thread %lu has obtained the aux mutex.\n", GetCurrentThreadId());
    /* If we are the first reader, indicate that no one can write. */
    if (++rw_sync->readers == 1)
    {
        if (SYNC_DEBUG_MODE) printf("Thread %lu is waiting for main mutex.\n", GetCurrentThreadId());
        WaitForSingleObject(rw_sync->mutex_primary, INFINITE);
        if (SYNC_DEBUG_MODE) printf("Thread %lu has obtained the main mutex.\n", GetCurrentThreadId());
    }
    /* Release the mutex now that we have modified readers. */
    ReleaseMutex(rw_sync->mutex_auxiliary);
    if (SYNC_DEBUG_MODE) printf("Thread %lu has released aux mutex.\n", GetCurrentThreadId());
}

/*
 * Removes a reader that was previously reading.
 * Remember to call `sync_read_start` before calling this function.
 * Θ(1)
 */
void sync_read_end(ReadWriteSync* const rw_sync)
{
    io_assert(rw_sync != NULL, IO_MSG_NULL_PTR);

    if (SYNC_DEBUG_MODE) printf("Thread %lu is waiting for aux mutex.\n", GetCurrentThreadId());
    /* Grab mutex so we can decrement readers. */
    WaitForSingleObject(rw_sync->mutex_auxiliary, INFINITE);
    if (SYNC_DEBUG_MODE) printf("Thread %lu has obtained the aux mutex.\n", GetCurrentThreadId());
    if (rw_sync->readers == 0)
        io_error(SYNC_MSG_NO_READERS);
    /* If we are the last reader, allow others to write. */
    if (--rw_sync->readers == 0)
    {
        ReleaseMutex(rw_sync->mutex_primary);
        if (SYNC_DEBUG_MODE) printf("Thread %lu has released main mutex.\n", GetCurrentThreadId());
    }
    /* Release the mutex now that we have modified readers. */
    ReleaseMutex(rw_sync->mutex_auxiliary);
    if (SYNC_DEBUG_MODE) printf("Thread %lu has released aux mutex.\n", GetCurrentThreadId());
}

/*
 * Adds a writer to the scheduler, forbidding any future reading/writing.
 * Remember to call `sync_write_end` to release the mutex.
 * Θ(1)
 */
void sync_write_start(ReadWriteSync* const rw_sync)
{
    io_assert(rw_sync != NULL, IO_MSG_NULL_PTR);

    if (SYNC_DEBUG_MODE) printf("Thread %lu is waiting for main mutex.\n", GetCurrentThreadId());
    WaitForSingleObject(rw_sync->mutex_primary, INFINITE);
    if (SYNC_DEBUG_MODE) printf("Thread %lu has obtained the main mutex.\n", GetCurrentThreadId());
}

/*
 * Removes a writer that was previously writing.
 * Remember to call `sync_write_start` before calling this function.
 * Θ(1)
 */
void sync_write_end(ReadWriteSync* const rw_sync)
{
    io_assert(rw_sync != NULL, IO_MSG_NULL_PTR);

    ReleaseMutex(rw_sync->mutex_primary);
    if (SYNC_DEBUG_MODE) printf("Thread %lu has released main mutex.\n", GetCurrentThreadId());
}

/*
 * De-constructor function.
 * Θ(1)
 */
void sync_destroy(ReadWriteSync *rw_sync)
{
    io_assert(rw_sync != NULL, IO_MSG_NULL_PTR);

    CloseHandle(rw_sync->mutex_primary);
    CloseHandle(rw_sync->mutex_auxiliary);
    mem_free(rw_sync, sizeof(ReadWriteSync));
}