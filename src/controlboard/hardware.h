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

#ifndef HARDWARE_H_
#define HARDWARE_H_

#define NUMBER_OF_MOTOR_CHANNELS 2
#define NUMBER_OF_SENSOR_CHANNELS 5
#define NUMBER_OF_PUSHBUTTONS 5
#define LCD_TEXT_LINES 2

#define MAX_SENSOR_LEVEL 100
#define MAX_SENSOR_ADC 4095

#define SPEED_NULL_VALUE 0
#define DIRECTION_NULL_VALUE 511

#define MAX_FORWARD_SPEED (256)
#define MAX_REVERSE_SPEED (0 - MAX_FORWARD_SPEED)

#define MAX_LEFT_DIRECTION (90 * WIICAR_DEGREE_SCALING)
#define MAX_RIGHT_DIRECTION MAX_LEFT_DIRECTION

#define MIN_DIRECTION_MOTOR 0
#define MAX_DIRECTION_MOTOR 1024


typedef enum MotorChannel_t
{
	MOTOR_SPEED_CHANNEL, //
	MOTOR_DIRECTION_CHANNEL,
} MotorChannel_t;

typedef enum StatusLedFlashState_t
{
	STATUS_LED_OFF, //
	STATUS_LED_ON, //
	STATUS_LED_FLASH, //
	STATUS_LED_UNITIALIZED = -1,
} StatusLedFlashState_t;

typedef enum MotorDirection_t
{
	MOTOR_OFF = 0x00, //
	MOTOR_FORWARD = 0x01, //
	MOTOR_REVERSE = 0x02, //
	MOTOR_BRAKE = 0x03,
//
} MotorDirection_t;

typedef enum SensorChannelName_t
{
	SENSOR_FWD = 0, //
	SENSOR_REV = 1,
} SensorChannelName_t;

typedef enum IrLedEnableFlagType
{
	IR_LED_ENABLE_NONE = 0x00,
	IR_LED_ENABLE_LED_1 = (1 << 0),
	IR_LED_ENABLE_LED_2 = (1 << 1),
	IR_LED_ENABLE_LED_3 = (1 << 2),
	IR_LED_ENABLE_ALL = (IR_LED_ENABLE_LED_1 | IR_LED_ENABLE_LED_2
			| IR_LED_ENABLE_LED_3),
} IrLedEnableFlagType;

typedef enum PushbuttonName_t
{
	JOY_CENTER = 0, //
	JOY_UP = 1, //
	JOY_DOWN = 2, //
	JOY_LEFT = 3, //
	JOY_RIGHT = 4,
} PushbuttonName_t;

#endif /* HARDWARE_H_ */
