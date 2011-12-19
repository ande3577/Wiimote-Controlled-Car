#include "wiicar.h"
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <controlboard/control_board.h>
#include <controlboard/hardware.h>
#include <wiicarutility/timestamp.h>
#include <wiicarutility/utility.h>
#include "wiicar_math.h"

#define MAX_FORWARD_PITCH (45 * DEGREE_SCALING)
#define MAX_REVERSE_PITCH (45 * DEGREE_SCALING)

#define MAX_RIGHT_ROLL (90 * DEGREE_SCALING)
#define MAX_LEFT_ROLL (90 * DEGREE_SCALING)

#define MAX_IR_DISTANCE (768 / 4)

int32_t stop_motors(void)
{
	return write_motor_levels(SPEED_NULL_VALUE, DIRECTION_NULL_VALUE);
}

/*!
 @brief Compute motor levels based on accelerometer data.

 The pitch forward and back controls the speed, the roll left-right controls the direction.

 */
int32_t computer_motor_levels_accel(
		volatile struct WiimoteStatusDataType *wiimote_status)
{
	int32_t speed;
	int32_t direction;

	normalize_accel(wiimote_status);

	speed = -wiimote_status->accel_computed_data.accel_normalized[Y_AXIS];
	speed *= 3;
	speed /= 2;
	if (speed > WIICAR_ACCEL_SCALING_VALUE)
		speed = WIICAR_ACCEL_SCALING_VALUE;

	direction = -wiimote_status->accel_computed_data.accel_normalized[X_AXIS];

	speed = rescale_range(speed, 0, 0 - WIICAR_ACCEL_SCALING_VALUE,
			WIICAR_ACCEL_SCALING_VALUE, 0, MAX_REVERSE_SPEED,
			MAX_FORWARD_SPEED);

	wiimote_status->accel_computed_data.pitch = determine_pitch(
			&wiimote_status->accel_computed_data);
	wiimote_status->accel_computed_data.roll = determine_roll(
			&wiimote_status->accel_computed_data);
	wiimote_status->accel_computed_data.yaw = determine_yaw(
			&wiimote_status->accel_computed_data);

#if _DEBUG
	printf("%u: Accel (p,r,y): %d %d %d\n", get_tick_count(), wiimote_status->accel_computed_data.pitch / 100,
			wiimote_status->accel_computed_data.roll / 100, wiimote_status->accel_computed_data.yaw / 100 );
#endif

	direction *= 3;
	direction /= 2;

	direction = rescale_range(direction, 0, -WIICAR_ACCEL_SCALING_VALUE,
			WIICAR_ACCEL_SCALING_VALUE, DIRECTION_NULL_VALUE,
			MIN_DIRECTION_MOTOR, MAX_DIRECTION_MOTOR);

	return write_motor_levels(speed, direction);
}

/*!
 \brief Motor control routines for using infrared based control.

 Uses three parameters: distance, theta, and phi, in order to determine motor direction and speed.

 Theta is the angle between the front of the wiimote and the front of the car.  Phi is the course
 change required to move the wiimote towards the center of the Wiimote's FOV.

 The car should first move towards the center of the camera, then align itself to the same
 orientation as the wiimote.  This means that for distances far from the center, phi will be
 the dominant control variable, while for points near the center, theta will dominate.


 \param WiimoteIrRawData pointer to data containing wiimote ir status.  Must contain distance, theta,
 phi and WiimoteIrStatus.

 \param WiimoteIrComputedData Will contain calculated IR values.

 \param LedData a pointer to the LedData, this function uses the Leds to inform the user if invalid
 infrared data is present (the camera is unable to see all three leds).

 \return bool returns true if the function succeeds in sending motor command, false if it fails.
 */
int32_t WiiComputeMotorLevelsInfrared(
		volatile struct WiimoteStatusDataType *wiimote_status,
		bool *valid_points)
{
	uint32_t distance;
	int32_t direction;
	int32_t speed;

	// compute derived data

	count_ir_points(&wiimote_status->ir_computed_data,
			&wiimote_status->ir_raw_data);

	/// @todo MED: change this function to be able to use 3 or 4 IR points. May have to sort data by spot size first
	if (wiimote_status->ir_computed_data.count == 3) // currently all IR calculations are only valid if exactly three IR points are seen.

	{
		wiimote_status->ir_computed_data.WiimoteIRStatus =
				WII_IR_STATUS_VALID_DATA;
	}
	else
	{
		wiimote_status->ir_computed_data.WiimoteIRStatus =
				WII_IR_STATUS_INVALID_DATA;
	}

	if (wiimote_status->ir_computed_data.WiimoteIRStatus
			== WII_IR_STATUS_INVALID_DATA)
	{
		return stop_motors();
	}
	else
	{
		compute_ir_data(&wiimote_status->ir_raw_data,
				&wiimote_status->ir_computed_data);

		distance = wiimote_status->ir_computed_data.distance;
		direction = wiimote_status->ir_computed_data.phi;

		if (distance > MAX_IR_DISTANCE)
		{
			distance = MAX_IR_DISTANCE;
		}

		if ((direction < (-90 * WIICAR_DEGREE_SCALING))
				|| (direction > (90 * WIICAR_DEGREE_SCALING))) // needs to be run in reverse

		{
			speed = (distance * MAX_REVERSE_SPEED) / MAX_IR_DISTANCE;
			direction += (180 * WIICAR_DEGREE_SCALING);
			direction = cap_angle(direction, 180, 360, WIICAR_DEGREE_SCALING);
		}
		else
		{
			speed = (distance * MAX_FORWARD_SPEED) / MAX_IR_DISTANCE;
		}
		direction = ComputeDirectionMotor(direction);
		return write_motor_levels(speed, direction);
	}
}

int32_t ComputeDirectionMotor(int32_t Direction)
{
	return rescale_range(Direction, 0, -MAX_LEFT_DIRECTION, MAX_RIGHT_DIRECTION,
			DIRECTION_NULL_VALUE, MIN_DIRECTION_MOTOR, MAX_DIRECTION_MOTOR);
}

