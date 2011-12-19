#ifndef WIIMOTE_H_
#define WIIMOTE_H_

#include <stdbool.h>
#include <stdint.h>
#include "cwiid.h"

/// \brief Number of points the wiimote is capable of tracking.
#define WIICAR_NUMBER_OF_MAX_IR_POINTS 4

/// \brief Number of infrared LEDs on the wiimote car.
#define WIICAR_IR_NUMBER_OF_POINTS 3

#define WIICAR_DEGREE_SCALING (100)

#define WIICAR_NUMBER_OF_TOTAL_GS (32)
#define WIICAR_ZERO_G (0x80)

#define WIICAR_RUMBLE_ON_VALUE 5

#define WIICAR_ACCEL_SCALING_VALUE (4096)

typedef enum WiiCalIndex_t
{
	X_AXIS = 0, //
	Y_AXIS = 1,
	Z_AXIS = 2
} WiiCalIndex_t;


typedef enum WiimoteState_t
{
	WII_PROMPT, WII_SETUP, WII_WAIT_FOR_CONNECTION, WII_OPERATE,
} WiimoteState_t;

typedef enum WiimoteIRStatus_t
{
	WII_IR_STATUS_INVALID_DATA, WII_IR_STATUS_VALID_DATA,
} WiimoteIRStatus_t;

typedef enum CarPositionSelection_t
{
	WII_CAR_POSITION_FRONT = 0,
	WII_CAR_POSITION_BACK = 1,
	WII_CAR_POSITION_CENTER = 2,
} CarPositionSelection_t;

typedef struct WiimoteIrRawData_t
{
	struct cwiid_ir_src WiimoteIRPoint[WIICAR_NUMBER_OF_MAX_IR_POINTS];
} WiimoteIrRawData_t;


typedef struct WiimoteAccelComputedData_t
{
	int16_t accel_normalized[3];
	int32_t yaw;
	int32_t pitch;
	int32_t roll;
} WiimoteAccelComputedData_t;

typedef struct WiimoteIRPositions_t
{
	struct cwiid_ir_src WiimoteCarPosition[WIICAR_IR_NUMBER_OF_POINTS];
} WiimoteIRPositions_t;

typedef struct WiimoteIRComputedData_t
{
	WiimoteIRStatus_t WiimoteIRStatus;
	uint8_t count;
	struct WiimoteIRPositions_t WiimoteIRPositions;
	uint16_t distance;
	int32_t theta;
	int32_t temp_phi;
	int32_t phi;
} WiimoteIRComputedData_t;

typedef struct WiimoteStatusDataType
{
	int16_t battery_level;
	uint16_t button_data;
	uint8_t accel_raw_data[3];
	struct WiimoteIrRawData_t ir_raw_data;

	// wiimote data
	struct WiimoteAccelComputedData_t accel_computed_data;
	struct acc_cal accel_cal_data;
	struct WiimoteIRComputedData_t ir_computed_data;
} WiimoteStatusDataType;

typedef enum SensorStatusType
{
	SENSOR_CLEAR = 0x00, /// no objects detected
	SENSOR_FORWARD_OBJECT = 0x01, /// object detected ahead of car
	SENSOR_REVERSE_OBJECT = 0x02, /// object detected behind car
	SENSOR_SURROUNDED_OBJECT = 0x03, /// object surrounding car
	SENSOR_ERROR = 0xFF,
} SensorStatusType;

int32_t stop_motors(void);

int32_t computer_motor_levels_accel(
		volatile struct WiimoteStatusDataType *wiimote_status);

int32_t WiiComputeMotorLevelsInfrared(
		volatile struct WiimoteStatusDataType *wiimote_status, bool *valid_points);

int32_t ComputeDirectionMotor(int32_t Direction);

#endif /*WIIMOTE_H_*/
