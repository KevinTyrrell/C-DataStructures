#pragma once

#include <stdio.h>
#include <Windows.h>
#include <stdbool.h>
#include <conio.h>

#define DS_MSG_EXIT "Exit Runtime"
#define DS_MSG_OUT_OF_MEM "Not enough memory to allocate for this variable!"
#define DS_MSG_OUT_OF_BOUNDS "Requested index was out of bounds!"

/* Print out a formatted error message to the console window. */
void ds_Error(const char *message);
/* Prompt the user to press the Y or N key. */
bool ds_YesNo(const char *message);
/* Changes the color of the console window and returns the previous color. */
WORD ds_changeColor(WORD color);