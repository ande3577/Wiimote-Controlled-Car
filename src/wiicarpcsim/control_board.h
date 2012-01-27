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

#ifndef CONTROL_BOARD_H_
#define CONTROL_BOARD_H_

#include <stdint.h>
#include <stdbool.h>
#include "hardware.h"


extern volatile bool timer_flag;

char *get_rx_buffer(void);
char *get_tx_buffer(void);

int32_t comm_init(char *port_name);
int32_t comm_close(void);

int32_t write_motor_levels(int32_t channel1, int32_t channel2);
int32_t read_motor_levels(int32_t *channel1, int32_t *channel2);
int32_t get_motor_pwm_counts(int32_t *channel1, int32_t *channel2);
int32_t read_sensor_values(int32_t *channel1, int32_t *channel2);
int32_t set_motor_timeout(int32_t timeout);
int32_t read_motor_timeout(int32_t *timeout);
int32_t set_ir_led(bool on);
int32_t read_ir_led(bool *on);
int32_t write_status_led(StatusLedFlashState_t led_state, int32_t flash_rate);
int32_t read_status_led(StatusLedFlashState_t *led_state, int32_t *flash_rate);
int32_t read_current_time(uint32_t *time);
int32_t read_program_info(char *pgm_info);
int32_t send_password(char *password);
int32_t read_last_error(int32_t *error_id, int32_t *timestamp);
int32_t read_pushbuttons(bool *pressed);
int32_t send_jump_to_boot(void);
int32_t clear_lcd(void);
int32_t set_lcd(int32_t line, char *lcd_text, ...);
int32_t lcd_putchars(int32_t line, int32_t ch, char *fmt, ...);
int32_t shutdown();

bool get_comm_trace(void);
void set_comm_trace(bool enabled);

#endif /* CONTROL_BOARD_H_ */
