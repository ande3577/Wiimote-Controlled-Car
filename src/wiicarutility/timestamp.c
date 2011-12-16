/*
 * timestamp.c
 *
 *  Created on: Dec 13, 2010
 *      Author: desertfx5
 */

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <sys/time.h>

uint64_t start_time = 0;

static uint64_t timeval_to_ms(struct timeval *tv)
{
	return (uint64_t) tv->tv_sec*1000 + tv->tv_usec/1000;
}

static uint64_t get_raw_ms(void)
{
	struct timeval tv;
	gettimeofday(&tv, NULL);
	return timeval_to_ms(&tv);
}

void init_tick_count(void)
{
	start_time = get_raw_ms();
}

uint32_t get_tick_count(void)
{
	return get_raw_ms() - start_time;
}

bool check_for_timeout(uint32_t current_time, uint32_t start_time, int32_t timeout)
{
	if (timeout < 0)
		return false;
	else
		return (current_time - start_time > timeout);
}



