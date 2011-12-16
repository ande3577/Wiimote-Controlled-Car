/*
 * utility.h
 *
 *  Created on: Dec 15, 2011
 *      Author: dsanderson
 */

#ifndef UTILITY_H_
#define UTILITY_H_

#include <stdint.h>

extern int32_t rescale_range(int32_t x, int32_t null_x, int32_t min_x,
		int32_t max_x, int32_t null_y, int32_t min_y, int32_t max_y);

extern int32_t coerce(int32_t x, int32_t min, int32_t max);

#endif /* UTILITY_H_ */
