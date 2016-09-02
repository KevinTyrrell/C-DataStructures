
#include "DataStructureTools.h"

#define Y_LC_KEY_CODE 121
#define Y_UC_KEY_CODE 89
#define N_LC_KEY_CODE 110
#define N_UC_KEY_CODE 78

/* Print out an error to the Console window in color. */
void ds_Error(const char *message)
{
	// Attempt to change the color of the Console window.
	bool colorFlag = false;
	WORD prevColor;
	CONSOLE_SCREEN_BUFFER_INFO info;
	HANDLE console = GetStdHandle(STD_OUTPUT_HANDLE);
	if (GetConsoleScreenBufferInfo(console, &info))
	{
		colorFlag = true;
		prevColor = info.wAttributes;
		SetConsoleTextAttribute(console, DS_LIGHT_RED);
	}

	fprintf_s(stderr, "\n\n%s\n\n", message);
	// Revert the color to the previous color.
	if (colorFlag)
		SetConsoleTextAttribute(console, prevColor);

	if (ds_YesNo(DS_MSG_EXIT))
		exit(EXIT_FAILURE);
}

/*
Prompt the user to press the Y or N key.

Returns true if the user presses Y or false if N.
*/
bool ds_YesNo(const char *message)
{
	printf_s("%s (Y/N)?\n", message);

	int input;
	do
		input = _getch();
	while (input != Y_LC_KEY_CODE && input != Y_UC_KEY_CODE 
		&& input != N_LC_KEY_CODE && input != N_UC_KEY_CODE);

	return input == Y_LC_KEY_CODE || input == Y_UC_KEY_CODE;
}