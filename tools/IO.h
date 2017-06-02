
/*
 * Author: Kevin Tyrrell
 * Date: 6/01/2017
 * Version: 1.0
 */

#pragma once

#include <conio.h>
#include <windows.h>
#include <stdbool.h>
#include <stdio.h>

/* Error messages. */
#define IO_MSG_OUT_OF_BOUNDS "Requested index was out of bounds!"
#define IO_MSG_NULL_PTR "Unable to perform operation on NULL pointer!"
#define IO_MSG_EMPTY "Cannot perform this operation while the container is empty!"
#define IO_MSG_INVALID "Cannot perform this operation as the structure is marked as invalid!"

#define IO_LIGHT_TEAL 11
#define IO_LIGHT_RED 12

/* ~~~~~ Input/Output ~~~~~ */

/* Displays an error message to the user. */
void io_error(const char* const message);
/* Returns true if the user selects yes to the displayed prompt. */
bool io_prompt(const char *const message);
/* Changes the color of the console window and returns the previous color. */
WORD io_color(const WORD color);