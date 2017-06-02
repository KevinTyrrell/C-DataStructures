
/*
 * Author: Kevin Tyrrell
 * Date: 6/01/2017
 * Version: 1.0
 */

#pragma once

#include "Memory.h"

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

void sync_destroy(ReadWriteSync *rw_sync);
