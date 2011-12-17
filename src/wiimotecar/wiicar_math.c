/*!
 \file

 \brief  Definitions of math routines used for computing IR and accel parameters.

 \todo Some of these functions update and use global data, others have pass by reference.

 */

#include "wiicar.h"
#include <stdint.h>
#include <math.h>

int32_t cap_angle(int32_t x, int32_t cap_value, int32_t circle_size,
		int32_t scaling)
{
	int32_t y = x;
	while (y > (cap_value * scaling))
	{
		y -= circle_size * scaling;
	}

	while (y < ((cap_value - circle_size) * scaling))
	{
		y += circle_size * scaling;
	}
	return y;
}

volatile struct cwiid_ir_src WiimoteMidpoint =
{ 0,
{ 1024 / 2, 768 / 2 }, 0 };

/*!
 \brief Normalize accleration values based on calibration data.

 Remove offset and setup scaling using factory calibration data read out of Wiimote.

 \param wiimote_status pointer to the wiimote status data
 */
void normalize_accel(volatile struct WiimoteStatusDataType *wiimote_status)
{
	int32_t accel;
	uint8_t i;
	int32_t one_g;

	for (i = 0; i < 3; i++)
	{
		accel = ((int32_t) wiimote_status->accel_cal_data.zero[i])
				- ((int32_t) wiimote_status->accel_raw_data[i]);

		accel *= WIICAR_ACCEL_SCALING_VALUE;

		one_g = wiimote_status->accel_cal_data.one[i]
				- wiimote_status->accel_cal_data.zero[i];

		if (accel >= 0)
			accel += one_g / 2;
		else
			accel -= one_g / 2;

		accel /= one_g;

		wiimote_status->accel_computed_data.accel_normalized[i] = accel;
	}

}

/*!
 @brief Given wiimite data, compute wiimote pitch.

 Pitch = arcsine y_accel / g

 \param WiimoteAccelComputedData must contain normalized accelerometers values.
 */
int32_t determine_pitch(
		volatile struct WiimoteAccelComputedData_t *WiimoteAccelComputedData)
{
	int32_t y_accel = WiimoteAccelComputedData->accel_normalized[Y_AXIS];
	float pitch;

	// cap the y_accel at 1 G
	if (y_accel > WIICAR_ACCEL_SCALING_VALUE)
	{
		y_accel = WIICAR_ACCEL_SCALING_VALUE;
	}
	else if (y_accel < -WIICAR_ACCEL_SCALING_VALUE)
	{
		y_accel = -WIICAR_ACCEL_SCALING_VALUE;
	}

	pitch = (float) y_accel / WIICAR_ACCEL_SCALING_VALUE;
	pitch = asin(pitch);
	pitch *= 180 / M_PI;
	pitch *= WIICAR_DEGREE_SCALING;

	if (WiimoteAccelComputedData->accel_normalized[Z_AXIS] > 0)
	{
		pitch = 18000 - pitch;
	}

	pitch = cap_angle(pitch, 180, 360, 100);

	return pitch;
}

/*!
 @brief Given wiimite data, compute wiimtoe roll.

 roll = arcsine (x_accel / g)

 \return int32_t Roll in degress * 100
 */

int32_t determine_roll(
		volatile struct WiimoteAccelComputedData_t *WiimoteAccelComputedData)
{
	int32_t x_accel = WiimoteAccelComputedData->accel_normalized[X_AXIS];
	int32_t roll;

	// cap the y_accel at 1 G
	if (x_accel > WIICAR_ACCEL_SCALING_VALUE)
	{
		x_accel = WIICAR_ACCEL_SCALING_VALUE;
	}
	else if (x_accel < -WIICAR_ACCEL_SCALING_VALUE)
	{
		x_accel = -WIICAR_ACCEL_SCALING_VALUE;
	}

	roll = WIICAR_DEGREE_SCALING * asin((float) x_accel
			/ WIICAR_ACCEL_SCALING_VALUE) * 180 / M_PI;

	if (WiimoteAccelComputedData->accel_normalized[Z_AXIS] > 0)
	{
		roll = 18000 - roll;
	}

	roll = cap_angle(roll, 180, 360, 100);

	return 0 - roll;
}

/*!
 @brief Given wiimite data, compute wiimtoe yaw.

 yaw = arcsine (z_accel / g)
 */

int32_t determine_yaw(
		volatile struct WiimoteAccelComputedData_t *WiimoteAccelComputedData)
{
	int32_t z_accel = WiimoteAccelComputedData->accel_normalized[Z_AXIS];
	int32_t yaw;

	// cap the y_accel at 1 G
	if (z_accel > WIICAR_ACCEL_SCALING_VALUE)
	{
		z_accel = WIICAR_ACCEL_SCALING_VALUE;
	}
	else if (z_accel < -WIICAR_ACCEL_SCALING_VALUE)
	{
		z_accel = -WIICAR_ACCEL_SCALING_VALUE;
	}

	yaw = WIICAR_DEGREE_SCALING * asin((float) z_accel
			/ WIICAR_ACCEL_SCALING_VALUE) * 180 / M_PI;

	yaw += 9000;

	if (WiimoteAccelComputedData->accel_normalized[X_AXIS] > 0)
	{
		yaw = 18000 - yaw;
	}

	yaw = cap_angle(yaw, 180, 360, 100);

	return yaw;
}

/*!
 @brief Compute distance between two points.

 Good old d^2 = x^2 + y^2
 */

uint32_t compute_distance(int16_t x_1, int16_t x_2, int16_t y_1, int16_t y_2)
{
	int32_t x_dist, y_dist;

	x_dist = x_2 - x_1;
	y_dist = y_2 - y_1;

	return ((uint32_t) sqrt(x_dist * x_dist + y_dist * y_dist));
}

/*!
 @brief Compute the angle of the lane made by two points.

 Theta = atan (dy/dx)

 \return int32_t the resulting angle in degrees * 100.  Straight up is 0, with CW positive,
 CCW negative.

 */

int32_t compute_angle(int16_t x_1, int16_t x_2, int16_t y_1, int16_t y_2)
{
	int32_t theta1;
	int32_t tan_theta;
	int16_t dx, dy;

	dx = (int16_t) x_2 - (int16_t) x_1;
	dy = (int16_t) y_2 - (int16_t) y_1;

	if (dx == 0)
	{
		if (dy >= 0)
		{
			return 90 * WIICAR_DEGREE_SCALING;
		}
		else
		{
			return -90 * WIICAR_DEGREE_SCALING;
		}
	}

	tan_theta = ((int32_t) WIICAR_ACCEL_SCALING_VALUE * dy) / dx;

	theta1 = atan((float) tan_theta / WIICAR_ACCEL_SCALING_VALUE) * 180 / M_PI
			* WIICAR_DEGREE_SCALING;

	if (dx < 0)
	{
		theta1 += (180 * WIICAR_DEGREE_SCALING);
	}

	theta1 = cap_angle(theta1, 180, 360, WIICAR_DEGREE_SCALING);

	return theta1;
}

/*!
 \brief Counts the number of IR points the wiimote sees.

 A point is considered invalid of x = y = 0x03FF.

 Note that this requires the three valid points to be the first points, as this is the only way the later processing will work.


 \param WiimoteIRComputedData pointer to wiimote infrared computeddata structure.  When function finished, will contain count of IR point.

 */
void count_ir_points(
		volatile struct WiimoteIRComputedData_t *WiimoteIRComputedData,
		volatile struct WiimoteIrRawData_t *WiimoteIrRawData)
{
	uint8_t i;
	uint8_t count = 0;

	for (i = 0; i < 4; i++)
	{
		if (WiimoteIrRawData->WiimoteIRPoint[i].valid)
		{
			count++;
		}
		else
		{
			break;
		}

	}

	WiimoteIRComputedData->count = count;
}

/*!
 \brief Function computes the distance between two IR points.

 This is basically a forwarding function for ComputeDistance.  Although it may be changed to elimiate the squareroot using approx:


 distance = sqrt(dx^2 + dy^2) ~ dx + dy

 This would significantly reduce processing time, and probably still allow for finding front and back points.

 \param WiimotePoint1 pointer the first IR point.
 \param WiimotePoint2 pointer to the second IR point.

 \return uint32_t The distance between WiimotePoint1 and WiimotePoint2
 */
uint32_t compute_ir_distance(volatile struct cwiid_ir_src *WiimotePoint1,
		volatile struct cwiid_ir_src *WiimotePoint2)
{
	return compute_distance(WiimotePoint1->pos[0], WiimotePoint2->pos[0],
			WiimotePoint1->pos[1], WiimotePoint2->pos[1]);
}

/*!
 \brief Determines IR points are front and which are back.

 Checks the distance between all three points, the shortest distance is the front, the other point is the back.

 \param WiimoteIRPositions this will be filled with the front and back point.  Front is the midpoint of the two front LEDS.
 \param WiimoteIRData this contains the input IR data to the function.
 */
void determine_ir_front_back(
		volatile struct WiimoteIrRawData_t *WiimoteIrRawData,
		volatile struct WiimoteIRPositions_t *WiimoteIRPositions)
{
	uint32_t distance_01, distance_12, distance_02;

	distance_01 = compute_ir_distance(&WiimoteIrRawData->WiimoteIRPoint[0],
			&WiimoteIrRawData->WiimoteIRPoint[1]);
	distance_12 = compute_ir_distance(&WiimoteIrRawData->WiimoteIRPoint[1],
			&WiimoteIrRawData->WiimoteIRPoint[2]);
	distance_02 = compute_ir_distance(&WiimoteIrRawData->WiimoteIRPoint[0],
			&WiimoteIrRawData->WiimoteIRPoint[2]);

	if ((distance_01 < distance_12) && (distance_01 < distance_02)) // 01 is shortest distance -> 01 are front points, 2 is back
	{
		WiimoteIRPositions->WiimoteCarPosition[WII_CAR_POSITION_FRONT].pos[0]
				= (WiimoteIrRawData->WiimoteIRPoint[0].pos[0]
						+ WiimoteIrRawData->WiimoteIRPoint[1].pos[0] + 1) / 2;

		WiimoteIRPositions->WiimoteCarPosition[WII_CAR_POSITION_FRONT].pos[1]
				= (WiimoteIrRawData->WiimoteIRPoint[0].pos[1]
						+ WiimoteIrRawData->WiimoteIRPoint[1].pos[1] + 1) / 2;

		WiimoteIRPositions->WiimoteCarPosition[WII_CAR_POSITION_FRONT].size
				= (WiimoteIrRawData->WiimoteIRPoint[0].size
						+ WiimoteIrRawData->WiimoteIRPoint[1].size + 1) / 2;

		WiimoteIRPositions->WiimoteCarPosition[WII_CAR_POSITION_BACK]
				= WiimoteIrRawData->WiimoteIRPoint[2];
	}
	else if ((distance_12 < distance_01) && (distance_12 < distance_02)) // 12 is shortest distance -> 12 are front points, 0 is back
	{
		WiimoteIRPositions->WiimoteCarPosition[WII_CAR_POSITION_FRONT].pos[0]
				= (WiimoteIrRawData->WiimoteIRPoint[1].pos[0]
						+ WiimoteIrRawData->WiimoteIRPoint[2].pos[0] + 1) / 2;

		WiimoteIRPositions->WiimoteCarPosition[WII_CAR_POSITION_FRONT].pos[1]
				= (WiimoteIrRawData->WiimoteIRPoint[1].pos[1]
						+ WiimoteIrRawData->WiimoteIRPoint[2].pos[1] + 1) / 2;

		WiimoteIRPositions->WiimoteCarPosition[WII_CAR_POSITION_FRONT].size
				= (WiimoteIrRawData->WiimoteIRPoint[1].size
						+ WiimoteIrRawData->WiimoteIRPoint[2].size + 1) / 2;

		WiimoteIRPositions->WiimoteCarPosition[WII_CAR_POSITION_BACK]
				= WiimoteIrRawData->WiimoteIRPoint[0];
	}
	else // 02 is shortest distance -> 02 are front points, 1 is back
	{
		WiimoteIRPositions->WiimoteCarPosition[WII_CAR_POSITION_FRONT].pos[0]
				= (WiimoteIrRawData->WiimoteIRPoint[0].pos[0]
						+ WiimoteIrRawData->WiimoteIRPoint[2].pos[0] + 1) / 2;

		WiimoteIRPositions->WiimoteCarPosition[WII_CAR_POSITION_FRONT].pos[1]
				= (WiimoteIrRawData->WiimoteIRPoint[0].pos[1]
						+ WiimoteIrRawData->WiimoteIRPoint[2].pos[1] + 1) / 2;

		WiimoteIRPositions->WiimoteCarPosition[WII_CAR_POSITION_FRONT].size
				= (WiimoteIrRawData->WiimoteIRPoint[0].size
						+ WiimoteIrRawData->WiimoteIRPoint[2].size + 1) / 2;

		WiimoteIRPositions->WiimoteCarPosition[WII_CAR_POSITION_BACK]
				= WiimoteIrRawData->WiimoteIRPoint[1];
	}

}

/*!
 \brief Computes the location of the cars center when the front and back point are known.

 \param WiimoteIRPositions must have already calculated the cars front and rear point.  Center position will be located
 in this data structure.
 */
void determine_car_midpoint(
		volatile struct WiimoteIRPositions_t *WiimoteIRPositions)
{
	WiimoteIRPositions->WiimoteCarPosition[WII_CAR_POSITION_CENTER].pos[0]
			= (WiimoteIRPositions->WiimoteCarPosition[WII_CAR_POSITION_FRONT].pos[0]
					+ WiimoteIRPositions->WiimoteCarPosition[WII_CAR_POSITION_BACK].pos[0]
					+ 1) / 2;

	WiimoteIRPositions->WiimoteCarPosition[WII_CAR_POSITION_CENTER].pos[1]
			= (WiimoteIRPositions->WiimoteCarPosition[WII_CAR_POSITION_FRONT].pos[1]
					+ WiimoteIRPositions->WiimoteCarPosition[WII_CAR_POSITION_BACK].pos[1]
					+ 1) / 2;

	WiimoteIRPositions->WiimoteCarPosition[WII_CAR_POSITION_CENTER].size
			= (WiimoteIRPositions->WiimoteCarPosition[WII_CAR_POSITION_FRONT].size
					+ WiimoteIRPositions->WiimoteCarPosition[WII_CAR_POSITION_BACK].size
					+ 1) / 2;
}

/*!
 \brief Computes between a WiimoteIRPointType, and the center of the car.

 \param WiimoteIRPoint the point for the computation.

 \return uint32_t distance of the two points.
 */
uint32_t compute_ir_point_distance(volatile struct cwiid_ir_src *WiimoteIRPoint)
{
	return compute_ir_distance(WiimoteIRPoint, &WiimoteMidpoint);
}

/*!
 \brief Computes the angle between two wiimote points.

 \param WiimotePoint1 the first wiimote point.
 \param WiimotePoint2 the second wiimote point.

 \return int32_t the resulting angle in degrees * 100.
 */
int32_t compute_ir_angle(volatile struct cwiid_ir_src *WiimotePoint1,
		volatile struct cwiid_ir_src *WiimotePoint2)
{
	return compute_angle(WiimotePoint1->pos[0], WiimotePoint2->pos[0], 768
			- WiimotePoint1->pos[1], 768 - WiimotePoint2->pos[1]); // reverse y coord
}

/*!
 \brief Computes the current wiimotes heading.

 Theta is defined as being the difference between the wiimotes current heading and verticle relative to the Wiimote.

 0 Means the the front of the wiimote car is pointing directly towards the top of the wiimote.

 This value will tell the car how it needs to turn to align with the orientation of the wiimote.

 \param WiimoteIRPositions data containing the location of the front and back of the car.

 \return int32_t the resulting angle in degrees * 100.
 */
int32_t compute_ir_theta(
		volatile struct WiimoteIRPositions_t *WiimoteIRPositions)
{
	int32_t theta1 = compute_ir_angle(
			&WiimoteIRPositions->WiimoteCarPosition[WII_CAR_POSITION_BACK],
			&WiimoteIRPositions->WiimoteCarPosition[WII_CAR_POSITION_FRONT]);

	theta1 += (90 * WIICAR_DEGREE_SCALING);
	theta1 = cap_angle(theta1, 180, 360, WIICAR_DEGREE_SCALING);

	return 0 - theta1;
}

/*!
 \brief Computes the angle between the center of the car and the center of the wiimote camera.


 \param WiimoteIRPositions position inof containing the location of the cars center.

 \return int32_t the resuling angle in degrees * 100
 */
int32_t compute_ir_temp_phi(
		volatile struct WiimoteIRPositions_t *WiimoteIRPositions)
{
	int32_t phi1 = compute_ir_angle(
			&WiimoteIRPositions->WiimoteCarPosition[WII_CAR_POSITION_CENTER],
			&WiimoteMidpoint);

	phi1 += (90 * WIICAR_DEGREE_SCALING);
	phi1 = cap_angle(phi1, 180, 360, WIICAR_DEGREE_SCALING);

	return 0 - phi1;
}

/*!
 \brief Compute the difference between the cars heading and the heading toward the center of the wiimote.

 This will tell the wiimote how it needs to turn in order to go to the center of the wiimote camera.

 \param theta current heading of the wiimote car.
 \param temp_phi heading towards the center of wiimote camera's FOV.

 \return int32_t returns the needed course change in degrees * 100.
 */
int32_t compute_ir_phi(int32_t theta, int32_t temp_phi)
{
	int32_t phi;

	phi = theta - temp_phi;
	phi = cap_angle(phi, 180, 360, WIICAR_DEGREE_SCALING);

	return phi;
}

/*!
 \brief controls computing IR parameters.

 Final values are:

 <ul>
 <li>distance - distance between center of car and center of camera FOV.

 <li>theta - orientation of car relative to wiimote.

 <li>phi - orientation of car relative to the heading to the center of camer FOV.

 </ul>

 \param WiimoteIrRawData header to wiimote IR data.  must already be populated with IR points.
 \param WiimoteIRComputedData Pointer to computed data struct, will fill in computed IR data.
 */
void compute_ir_data(volatile struct WiimoteIrRawData_t *WiimoteIrRawData,
		volatile struct WiimoteIRComputedData_t *WiimoteIRComputedData)
{

	determine_ir_front_back(WiimoteIrRawData,
			&WiimoteIRComputedData->WiimoteIRPositions);

	determine_car_midpoint(&WiimoteIRComputedData->WiimoteIRPositions);

	WiimoteIRComputedData->distance
			= compute_ir_point_distance(
					&WiimoteIRComputedData->WiimoteIRPositions.WiimoteCarPosition[WII_CAR_POSITION_CENTER]);
	WiimoteIRComputedData->theta = compute_ir_theta(
			&WiimoteIRComputedData->WiimoteIRPositions);
	WiimoteIRComputedData->temp_phi = compute_ir_temp_phi(
			&WiimoteIRComputedData->WiimoteIRPositions);
	WiimoteIRComputedData->phi = compute_ir_phi(WiimoteIRComputedData->theta,
			WiimoteIRComputedData->temp_phi);
}

