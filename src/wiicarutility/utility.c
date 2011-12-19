/*
 * utility.c
 *
 *  Created on: Dec 15, 2011
 *      Author: dsanderson
 */

#include "utility.h"
#include <stdarg.h>
#if _DEBUG
#include <stdio.h>
#endif

int32_t rescale_range(int32_t x, int32_t null_x, int32_t min_x, int32_t max_x,
		int32_t null_y, int32_t min_y, int32_t max_y)
{
	int32_t y = x - null_x;
	if (x > null_x)
	{
		y *= (max_y - null_y);
		if (max_y == null_y)
			return max_y;
		y /= (max_x - null_x);
	}
	else
	{
		y *= (null_y - min_y);
		if (min_y == null_y)
			return min_y;
		y /= (null_x - min_x);
	}
	y += null_y;
	return coerce(y, min_y, max_y);
}

int32_t coerce(int32_t x, int32_t min, int32_t max)
{
	if (x < min)
		return min;
	else if (x > max)
		return max;
	else
		return x;
}


void debug_print(const char *fmt, ...)
{
#if _DEBUG
	va_list args;
	va_start(args, fmt);
	vprintf(fmt, args);
#endif
}
