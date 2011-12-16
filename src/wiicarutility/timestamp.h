/*
 * timestamp.h
 *
 *  Created on: Dec 13, 2010
 *      Author: desertfx5
 */

#ifndef TIMESTAMP_H_
#define TIMESTAMP_H_

#include <stdint.h>
#include <stdbool.h>

void init_tick_count(void);
uint32_t get_tick_count(void);

bool check_for_timeout(uint32_t current_time, uint32_t start_time, int32_t timeout);

#endif /* TIMESTAMP_H_ */
