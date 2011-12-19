/*
 * control_board.h
 *
 *  Created on: Sep 16, 2010
 *      Author: dsanderson
 */

#ifndef CONTROL_BOARD_H_
#define CONTROL_BOARD_H_

#include <stdint.h>
#include <stdbool.h>

#include "hardware.h"

typedef struct led_flash_status_t
{
	StatusLedFlashState_t state;
	int32_t flash_rate;
} led_flash_status_t;

typedef struct error_info_t
{
	int32_t error_id;
	uint32_t timestamp;
} error_info_t;

extern volatile bool timer_flag;

char *get_rx_buffer(void);
char *get_tx_buffer(void);

int32_t comm_init(char *port_name);
int32_t comm_close(void);

int32_t write_motor_levels(int32_t channel1, int32_t channel2);
int32_t read_motor_levels();
const int32_t *get_motor_levels();

int32_t read_sensor_values();
const int32_t *get_sensor_values();

int32_t set_motor_timeout(int32_t timeout);
int32_t read_motor_timeout();
const int32_t *get_motor_timeout();

int32_t set_ir_led(bool on);
int32_t read_ir_led();
const bool *get_ir_led();

int32_t write_status_led(StatusLedFlashState_t led_state, int32_t flash_rate);
int32_t read_status_led();
const led_flash_status_t *get_status_led();

int32_t write_error_led(StatusLedFlashState_t led_state, int32_t flash_rate);
int32_t read_error_led();
const led_flash_status_t *get_error_led();

int32_t read_current_time();
const uint32_t *get_current_time();

int32_t read_last_error();
const error_info_t *get_last_error();

int32_t read_program_info();
const char *get_program_info();

int32_t send_password(char *password);

int32_t send_jump_to_boot(void);
int32_t shutdown();

int32_t set_lcd(int32_t line, char *lcd_text, ...);
const char *get_lcd(int32_t line);

bool get_comm_trace(void);
void set_comm_trace(bool enabled);

bool get_diagnostic_mode();
void set_diagnostic_mode(bool enabled);

#endif /* CONTROL_BOARD_H_ */
