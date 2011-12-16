/*
 * hardware.h
 *
 *  Created on: Dec 19, 2010
 *      Author: desertfx5
 */

#ifndef HARDWARE_H_
#define HARDWARE_H_

#define NUMBER_OF_MOTOR_CHANNELS 2
#define NUMBER_OF_SENSOR_CHANNELS 2
#define NUMBER_OF_PUSHBUTTONS 5
#define LCD_TEXT_LINES 2

#define MAX_SENSOR_LEVEL 100
#define MAX_SENSOR_ADC 4095

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
