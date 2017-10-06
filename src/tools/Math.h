
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
 * File Name:       Math.h
 * File Author:     Kevin Tyrrell
 * Date Created:    02/09/2017
 */

#pragma once

#include "Math.h"

#define MATH_DIV_CEIL(dividend, divisor) (1 + ((dividend - 1) / divisor))

/* Returns the base to the power of the exponent. */
unsigned long long math_pow(unsigned long long base, unsigned int exp);
/* Returns the max of two integers. */
unsigned int math_max(const unsigned int a, const unsigned int b);
/* Returns the smallest power of the base which is greater than or equal to the specified value. */
unsigned int math_min_power_gt(const unsigned int base, const unsigned int greater_than);
