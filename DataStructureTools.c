
#include "DataStructureTools.h"

#define DS_LIGHT_RED 12
#define VK_Y 0x59
#define VK_N 0x4E

#define Y_LC_KEY_CODE 121
#define Y_UC_KEY_CODE 89
#define N_LC_KEY_CODE 110
#define N_UC_KEY_CODE 78

/* Print out a formatted error message to the console window. */
void ds_Error(const char *message)
{
	WORD prevColor = ds_changeColor(DS_LIGHT_RED);
	fprintf_s(stderr, "\n\n%s\n\n", message);
	/* Revert back to the previous color. */
	ds_changeColor(prevColor);
	if (ds_YesNo(DS_MSG_EXIT))
		exit(EXIT_FAILURE);
}

/* Prompt the user with a yes/no message
and return whether the user pressed Y (yes) as
true or N (no) as false. */
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

/* Changes the color of the console window and returns the previous color. */
WORD ds_changeColor(WORD color)
{
	WORD prevColor = 0;

	CONSOLE_SCREEN_BUFFER_INFO info;
	HANDLE console = GetStdHandle(STD_OUTPUT_HANDLE);

	if (GetConsoleScreenBufferInfo(console, &info))
	{
		prevColor = info.wAttributes;
		SetConsoleTextAttribute(console, color);
	}

	return prevColor;
}