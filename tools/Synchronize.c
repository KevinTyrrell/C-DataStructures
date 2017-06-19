
/*
 * File: Synchronize.c
 * Date: Jun 01, 2017
 * Name: Kevin Tyrrell
 * Version: 3.0.0
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

#define SYNC_SEMAPHORE_MAX 1
#define SYNC_MSG_NO_READERS "Unable to stop reading since there are no current readers!"
#define SYNC_MSG_NO_WRITERS "Unable to stop writing since there are no current writers!"

bool SYNC_DEBUG_MODE = false;
/* Performs a console log and includes the thread ID only in Debug Mode. */
#define SYNC_DEBUG_LOG(fmt, ...) do { if (SYNC_DEBUG_MODE) {\
    char buffer[256]; sprintf(buffer, "[Thread %lu] %s", GetCurrentThreadId(), fmt);\
    IO_CONSOLE_LOG(buffer, __VA_ARGS__); } } while (0)

/*
 * Structure to assist in synchronized reading/writing.
 * This structure will allow reader threads and writer threads to coexist.
 * Writers can only write when there are no readers or writers.
 * Readers can only read when there are no writers.
 * Multiple readers can read at the same time.
 * This implementation prioritizes writers.
 */
struct ReadWriteSync
{
    /* Number of reader threads who are waiting/currently reading. */
    unsigned int readers;
    /* The number of writer threads who are waiting to write. */
    unsigned int writers;
    /* Mutexes to protect `readers` and `writers`.  */
    HANDLE readers_mutex, writers_mutex, reader_bottleneck_mutex;
    /*
     * Protects writers from writing while there are readers.
     * Protects readers from reading while there are writers.
     */
    HANDLE reader_block_sem, writer_block_sem;
};

/* Local functions. */
void sem_signal(HANDLE semaphore);
void sem_wait(HANDLE semaphore);
void mutex_signal(HANDLE mutex);
void mutex_wait(HANDLE mutex);

/*
 * Constructor function.
 * Θ(1)
 */
ReadWriteSync* ReadWriteSync_new()
{
    ReadWriteSync* const rw_sync = mem_calloc(1, sizeof(ReadWriteSync));

    HANDLE mutexes[] = { rw_sync->readers_mutex, rw_sync->writers_mutex, rw_sync->reader_bottleneck_mutex };
    for (size_t i = 0, s = sizeof(mutexes) / sizeof(HANDLE); i < s; i++)
    {
        /* Default security, initially not owned, unnamed. */
        mutexes[i] = CreateMutex(NULL, false, NULL);
        if (mutexes[i] == NULL)
            SYNC_DEBUG_LOG("Error while creating Mutex! Error: %lu.\n", GetLastError());
    }

    HANDLE semaphores[] = { rw_sync->reader_block_sem, rw_sync->writer_block_sem };
    for (size_t i = 0, s = sizeof(semaphores) / sizeof(HANDLE); i < s; i++)
    {
        /* Default security, starting value, max value, unnamed. */
        semaphores[i] = CreateSemaphore(NULL, SYNC_SEMAPHORE_MAX, SYNC_SEMAPHORE_MAX, NULL);
        if (semaphores[i] == NULL)
            SYNC_DEBUG_LOG("Error while creating Semaphore! Error: %lu.\n", GetLastError());
    }

    return rw_sync;
}

/*
 * Adds this thread as a new synchronized reader.
 * Function `sync_read_end` must be called after reading is done.
 * Θ(1)
 */
void sync_read_start(ReadWriteSync* const rw_sync)
{
    io_assert(rw_sync != NULL, IO_MSG_NULL_PTR);

    /* Ensure that we are allowed to read. */
    mutex_wait(rw_sync->reader_bottleneck_mutex);
    sem_wait(rw_sync->reader_block_sem);
    mutex_wait(rw_sync->readers_mutex);

    /* We're the first reader, block any writing from occurring. */
    if (++rw_sync->readers == 1)
        sem_wait(rw_sync->writer_block_sem);

    mutex_signal(rw_sync->readers_mutex);
    sem_signal(rw_sync->reader_block_sem);
    mutex_signal(rw_sync->reader_bottleneck_mutex);
}

/*
 * Removes this thread as a new synchronized reader.
 * Function `sync_read_start` must be called before this function.
 * Θ(1)
 */
void sync_read_end(ReadWriteSync* const rw_sync)
{
    io_assert(rw_sync != NULL, IO_MSG_NULL_PTR);

    mutex_wait(rw_sync->readers_mutex);
    io_assert(rw_sync->readers > 0, SYNC_MSG_NO_READERS);

    /* We're the last reader, let the writers write again. */
    if (--rw_sync->readers == 0)
        sem_signal(rw_sync->writer_block_sem);

    mutex_signal(rw_sync->readers_mutex);
}


/*
 * Adds this thread as a new synchronized writer.
 * Function `sync_write_end` must be called after writing is done.
 * Θ(1)
 */
void sync_write_start(ReadWriteSync* const rw_sync)
{
    io_assert(rw_sync != NULL, IO_MSG_NULL_PTR);

    mutex_wait(rw_sync->writers_mutex);

    /* We're the first writer, block any reading from occurring. */
    if (++rw_sync->writers == 1)
        sem_wait(rw_sync->reader_block_sem);

    mutex_signal(rw_sync->writers_mutex);
    /* Only one writer can write at a time. */
    sem_wait(rw_sync->writer_block_sem);
}

/*
 * Removes this thread as a new synchronized writer.
 * Function `sync_write_start` must be called before this function.
 * Θ(1)
 */
void sync_write_end(ReadWriteSync* const rw_sync)
{
    io_assert(rw_sync != NULL, IO_MSG_NULL_PTR);

    /* Allow other readers/writers to continue. */
    sem_signal(rw_sync->writer_block_sem);

    mutex_wait(rw_sync->writers_mutex);
    io_assert(rw_sync->writers > 0, SYNC_MSG_NO_WRITERS);

    /* We're the last writer, let the readers read again. */
    if (--rw_sync->writers == 0)
        sem_signal(rw_sync->reader_block_sem);

    mutex_signal(rw_sync->writers_mutex);
}

/*
 * De-constructor function.
 * Θ(1)
 */
void sync_destroy(ReadWriteSync* const rw_sync)
{
    io_assert(rw_sync != NULL, IO_MSG_NULL_PTR);

    HANDLE resources[] = { rw_sync->readers_mutex, rw_sync->writers_mutex,
                           rw_sync->reader_bottleneck_mutex, rw_sync->reader_block_sem,
                           rw_sync->writer_block_sem };
    for (size_t i = 0, s = sizeof(resources) / sizeof(HANDLE); i < s; i++)
        CloseHandle(resources[i]);

    mem_free(rw_sync, sizeof(ReadWriteSync));
}

/*
 * Waits to obtain the lock from the binary semaphore.
 * Θ(1)
 */
void sem_wait(HANDLE semaphore)
{
    SYNC_DEBUG_LOG("Waiting for Semaphore (%p).\n", semaphore);
    /* Wait for semaphore with unlimited time-out. */
    WaitForSingleObject(semaphore, INFINITE);
    SYNC_DEBUG_LOG("Obtained Semaphore (%p).\n", semaphore);
}

/*
 * Signals the semaphore that this thread is done using it.
 * Θ(1)
 */
void sem_signal(HANDLE semaphore)
{
    /* Semaphore, increment count by 1, no previous count. */
    if (!ReleaseSemaphore(semaphore, 1, NULL))
        SYNC_DEBUG_LOG("Error while releasing Semaphore (%p)! Error: %lu.\n", GetLastError());
    SYNC_DEBUG_LOG("Released Semaphore (%p).\n", semaphore);
}

/*
 * Waits to obtain the lock from the mutex.
 * Θ(1)
 */
void mutex_wait(HANDLE mutex)
{
    SYNC_DEBUG_LOG("Waiting for Mutex (%p).\n", mutex);
    /* Wait for mutex with unlimited time-out. */
    WaitForSingleObject(mutex, INFINITE);
    SYNC_DEBUG_LOG("Obtained Mutex (%p).\n", mutex);
}

/*
 * Signals the mutex that this thread is done using it.
 * Θ(1)
 */
void mutex_signal(HANDLE mutex)
{
    if (!ReleaseMutex(mutex))
        SYNC_DEBUG_LOG("Error while releasing Mutex (%p)! Error: %lu.\n", GetLastError());
    SYNC_DEBUG_LOG("Released Mutex (%p).\n", mutex);
}
