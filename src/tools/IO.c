
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
 * File Name:       IO.c
 * File Author:     Kevin Tyrrell
 * Date Created:    02/01/2017
 */

#include "IO.h"

#define IO_TIMESTAMP_FORMAT "%s%d/%s%d/%d %s%d:%s%d:%s%d"
#define IO_CONVERT_YEAR(year) (year + 1900)
#define IO_LEADING_ZERO(value) (value < 10 ? "0" : "")

/*
 * Returns the current system timestamp in String form.
 * Return value is replaced if multiple timestamp calls are made.
 * Used for testing purposes.
 * Θ(1)
 */
char* io_timestamp()
{
    /* Get the system time, parse it into month/day/year. */
    const time_t t = time(NULL);
    const struct tm time = *localtime(&t);

    /* Concatenate the month, day, year, hour, minute, and second with leading zeroes. */
    static char buffer[20];
    sprintf(buffer, IO_TIMESTAMP_FORMAT, IO_LEADING_ZERO(time.tm_mon + 1), time.tm_mon + 1,
            IO_LEADING_ZERO(time.tm_mday), time.tm_mday, IO_CONVERT_YEAR(time.tm_year),
            IO_LEADING_ZERO(time.tm_hour), time.tm_hour, IO_LEADING_ZERO(time.tm_min),
            time.tm_min, IO_LEADING_ZERO(time.tm_sec), time.tm_sec);

    return buffer;
}
