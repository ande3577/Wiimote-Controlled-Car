/**
 * @author David S Anderson
 *
 *
 * Copyright (C) 2011 David S Anderson
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
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
