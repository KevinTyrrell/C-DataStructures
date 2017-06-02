
#include "IO.h"

#define Y_LC_KEY_CODE 121
#define Y_UC_KEY_CODE 89
#define N_LC_KEY_CODE 110
#define N_UC_KEY_CODE 78

#define IO_EXIT "Exit Runtime"

/*
 * Displays an error message to the user.
 * The user will also be prompted to terminate runtime or continue.
 * See: io_prompt
 */
void io_error(const char* const message)
{
    const WORD stdColor = io_color(IO_LIGHT_RED);
    fprintf(stderr, "\n\n%s\n\n", message);
    /* Revert back to the previous color. */
    io_color(stdColor);
    if (io_prompt(IO_EXIT))
        exit(EXIT_FAILURE);
}

/*
 * Returns true if the user selects yes to the displayed prompt.
 * The program's runtime will halt and wait for a Y/N key-press.
 * A provided message will be displayed along with the prompt.
 * Θ(1)
 */
bool io_prompt(const char *const message)
{
    printf("%s (Y/N)?\n", message);

    int input;
    do
        input = _getch();
        /* Wait for uppercase or lowercase Y/N to be pressed. */
    while (input != Y_LC_KEY_CODE && input != Y_UC_KEY_CODE
           && input != N_LC_KEY_CODE && input != N_UC_KEY_CODE);

    return input == Y_LC_KEY_CODE || input == Y_UC_KEY_CODE;
}

/*
 * Changes the color of the text in the console window.
 * Returns the previous color that was overwritten.
 * Θ(1)
 */
WORD io_color(const WORD color)
{
    WORD prevColor = 0;
    CONSOLE_SCREEN_BUFFER_INFO info;
    const HANDLE console = GetStdHandle(STD_OUTPUT_HANDLE);

    if (GetConsoleScreenBufferInfo(console, &info))
    {
        prevColor = info.wAttributes;
        SetConsoleTextAttribute(console, color);
    }

    return prevColor;
}
