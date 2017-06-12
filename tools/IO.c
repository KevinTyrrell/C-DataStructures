
/*
 * File: IO.c
 * Date: Jun 01, 2017
 * Name: Kevin Tyrrell
 * Version: 1.0.0
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
