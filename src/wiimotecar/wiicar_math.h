#ifndef WIICAR_MATH_H_
#define WIICAR_MATH_H_

#include <stdint.h>
#include "wiicar.h"

void normalize_accel(volatile struct WiimoteStatusDataType *wiimote_status);
int32_t determine_pitch(volatile struct WiimoteAccelComputedData_t *WiimoteAccelComputedData);
int32_t determine_roll(volatile struct WiimoteAccelComputedData_t *WiimoteAccelComputedData);
int32_t determine_yaw(volatile struct WiimoteAccelComputedData_t *WiimoteAccelComputedData);

void count_ir_points(volatile struct WiimoteIRComputedData_t *WiimoteIRComputedData, volatile struct WiimoteIrRawData_t *WiimoteIrRawData);
uint32_t compute_distance(int16_t x_1, int16_t x_2, int16_t y_1, int16_t y_2);
int32_t compute_angle(int16_t x_1, int16_t x_2, int16_t y_1, int16_t y_2);

uint32_t compute_ir_distance(volatile struct cwiid_ir_src *WiimotePoint1, volatile struct cwiid_ir_src *WiimotePoint2);
void determine_ir_front_back(volatile  struct WiimoteIrRawData_t *WiimoteIrRawData, volatile struct WiimoteIRPositions_t *WiimoteIRPositions);
void determine_car_midpoint(volatile struct WiimoteIRPositions_t *WiimoteIRPositions);
uint32_t compute_ir_point_distance(volatile struct cwiid_ir_src *WiimoteIRPoint);
int32_t compute_ir_angle(volatile struct cwiid_ir_src *WiimotePoint1, volatile struct cwiid_ir_src *WiimotePoint2);
int32_t compute_ir_theta(volatile struct WiimoteIRPositions_t *WiimoteIRPositions);
int32_t compute_ir_temp_phi(volatile struct WiimoteIRPositions_t *WiimoteIRPositions);
int32_t compute_ir_phi(int32_t theta, int32_t temp_phi);
void compute_ir_data(volatile struct WiimoteIrRawData_t *WiimoteIrRawData, volatile struct WiimoteIRComputedData_t *WiimoteIRComputedData);


int32_t cap_angle(int32_t x, int32_t cap_value, int32_t circle_size, int32_t scaling);


#endif /*WIIMOTEMATH_H_*/
