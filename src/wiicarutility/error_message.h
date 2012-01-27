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

#ifndef ERROR_MESSAGE_H_
#define ERROR_MESSAGE_H_

#include <stdint.h>

/// TODO resync this with controller board firmware
typedef enum ErrorID_t {
	ERR_NONE = 0, //
	ERR_MOTOR_TIMEOUT = -1, //
	ERR_PARAM = -2, //
	ERR_CMD = -3, //
	ERR_EXEC = -4, //
	ERR_BUFFER_FULL = -5, //
	ERR_BUFFER_EMPTY = -6, //
	ERR_COMMAND_MISMATCH = -7, //
	ERR_COMM_TIMEOUT = -8, //
	ERR_INVALID_RESPONSE = -9, //
	ERR_WRITE = -10, //
	ERR_READ = -11, //
	ERR_FRAME = -12, //
	ERR_PORT_INIT = -13, //
	ERR_WII_DATA_TIMEOUT = -14, //
	ERR_UNKN = -15, //
	WII_ERROR_TRANSMIT = -5000,
	WII_ERROR_ACCEL_CAL = -5001,
	WII_ERROR_DATA_TIMEOUT = -5002,
	WII_ERROR_ABORT_PRESSED = -5003,
	WII_ERROR_MOTOR_COMMAND_ERROR = -5004,
	WII_ERROR_SENSOR_ERROR = -5005,
	WII_ERROR_QUEUE_ERROR = -5006,
} ErrorID_t;

int32_t decode_error_response(char *response);

void format_error_string(ErrorID_t error_code, char *error_string);

#endif /* ERROR_MESSAGE_H_ */
