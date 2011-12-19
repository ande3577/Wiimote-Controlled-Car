/*!
 \file

 \brief Definitions for tasks related to Wiimote car control.

 */

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <cwiid.h>
#include <unistd.h>
#if _MUTEX_ENABLE
#include <semaphore.h>
#endif
#include <pthread.h>
#include <error.h>
#include <errno.h>
#include <time.h>
#include <wiicarutility/timestamp.h>
#include <wiicarutility/error_message.h>
#include <wiicarutility/utility.h>
#include <controlboard/control_board.h>
#ifdef _PC_SIM
#include <wiicargui/wiicargui.h>
#endif

#include "wiicar.h"
#include "ControlTasks.h"

cwiid_mesg_callback_t cwiid_callback;

WiimoteStatusDataType wiimote_status_data =
{ 0 };

/// \bug Mutexes cause segmentation fault in OpenWRT
#if _MUTEX_ENABLE
pthread_cond_t cond =
PTHREAD_COND_INITIALIZER;
pthread_mutex_t mutex =
PTHREAD_MUTEX_INITIALIZER;
#else
volatile bool wiimote_data_ready = false;
#endif

const struct acc_cal DefaultAccelCalData =
{ //
		{ WIICAR_ZERO_G, //uint8 x0;
				WIICAR_ZERO_G, //uint8 y0;
				WIICAR_ZERO_G, }, //uint8 z0;

				{ WIICAR_NUMBER_OF_TOTAL_GS, //uint8 xg;
						WIICAR_NUMBER_OF_TOTAL_GS, //uint8 yg;
						WIICAR_NUMBER_OF_TOTAL_GS, } //uint8 zg;
		};

#define INFINITE_TIMEOUT -1

volatile bool debug_active = false;

#define MENU \
	" ^: turn left\n" \
	" v: turn right\n" \
	" A: enter accellerometer mode\n" \
	" B: enter infrared mode\n" \
	" 1: move forward\n" \
	" 2: move backward\n" \
	"hm: restart\n" \

#define MENU_ENTRIES 7
const char *lcd_menu_strings[MENU_ENTRIES] =
{ "^ - turn left", //
		"v - turn right", //
		"A - accel mode", //
		"B - infrared mode", //
		"1 - move fwd", //
		"2 - move back", //
		"hm - restart", //
		};

static ErrorID_t shutdown_all(cwiid_wiimote_t *wiimote);
static ErrorID_t main_menu(cwiid_wiimote_t *wiimote,
		volatile WiimoteStatusDataType *wiimote_status);
static ErrorID_t infrared_mode(cwiid_wiimote_t *wiimote,
		volatile WiimoteStatusDataType *wiimote_status);
static ErrorID_t acceleration_mode(cwiid_wiimote_t *wiimote,
		volatile WiimoteStatusDataType *wiimote_status);
static ErrorID_t button_mode(cwiid_wiimote_t *wiimote,
		volatile WiimoteStatusDataType *wiimote_status);
ErrorID_t error_mode(cwiid_wiimote_t *wiimote,
		volatile WiimoteStatusDataType *wiimote_status);
static ErrorID_t wait_for_wiimotedata(
		volatile WiimoteStatusDataType *wiimote_status, int32_t timeout);
static ErrorID_t signal_wiimote_data_ready(
		volatile WiimoteStatusDataType *wiimote_status, int32_t timeout);

int32_t sensor_cutoff_fwd, sensor_cutoff_rev;

cwiid_err_t err;

void err(cwiid_wiimote_t *wiimote, const char *s, va_list ap)
{
#if _DEBUG
	if (wiimote)
		printf("%d:", cwiid_get_id(wiimote));
	else
		printf("-1:");
	vprintf(s, ap);
	printf("\n");
#endif
}

/*!
 @brief High level task for wiimote.

 Handles initializing wiimote.
 */
void control_tasks(char *dev_name)
{
	cwiid_wiimote_t *wiimote = NULL;
	WiimoteState_t WiimoteState = WII_PROMPT;
	bdaddr_t bdaddr = *BDADDR_ANY; /* bluetooth device address */

	init_tick_count();

#if _MUTEX_ENABLE
	pthread_mutex_lock(&mutex);
#endif

#if _PC_SIM
	int argc_dummy = 0;
	char **argv_dummy = NULL;
	init_gui(argc_dummy, argv_dummy);

	set_comm_trace(true);
	set_diagnostic_mode(true);
#else
	if (0 >= comm_init(dev_name))
	{
		debug_print("@%u: Cannot initialize %s\n", get_tick_count(), dev_name);
		exit(4);
	}
	else
	{
		debug_print("@%u: %s initialized.\n", get_tick_count(), dev_name);
	}

	do
	{
		debug_print("Connecting to controller board... ");
		if (0 > send_password("WIFIBOT123"))
		{
			debug_print("Error\n");
		}
		else
		{
			debug_print("Done\n");
			break;
		}
	}while (1);

#endif

	set_lcd(0, "%s", PACKAGE_NAME);
	set_lcd(1, "Rev: %s", PACKAGE_VERSION);

	// display startup for 1 s
	sleep(1);

	cwiid_set_err(err);

	for (;;)
	{

		switch (WiimoteState)
		{
		case WII_PROMPT:
			/* Connect to the wiimote */
			debug_print(
					"Put Wiimote in discoverable mode now (press 1+2)...\n");

			set_lcd(0, "Connecting...");
			set_lcd(1, "Press 1+2 now");
			WiimoteState = WII_WAIT_FOR_CONNECTION;
			break;
		case WII_WAIT_FOR_CONNECTION:
			if (!(wiimote = cwiid_open(&bdaddr, 0)))
			{
				fprintf(stderr, "Unable to connect to wiimote, retrying...\n");
				WiimoteState = WII_PROMPT;
			}
			else if (cwiid_set_mesg_callback(wiimote, cwiid_callback))
			{
				fprintf(stderr,
						"Unable to set message callback, retrying...\n");
				WiimoteState = WII_PROMPT;
			}
			else
			{
				WiimoteState = WII_OPERATE;
			}
			break;
		case WII_OPERATE:
		default:
			main_menu(wiimote, &wiimote_status_data);
			debug_print("exiting.\n");
#if _MUTEX_ENABLE
			pthread_mutex_destroy(&mutex);
#endif
#if _PC_SIM
			shutdown_gui();
#endif
			exit(0);
			break;
		}
	}
}

typedef enum WII_OPERATE_STATE
{
	WII_OPERATE_INIT_MENU,
	WII_OPERATE_WAIT_FOR_BUTTON_PRESS,
	WII_OPERATE_DISPLAY_ACCELEROMETER_INFO,
	WII_OPERATE_DISPLAY_IR_STATUS,
	WII_OPERATE_DISPLAY_BUTTON_STATUS,
	WII_OPERATE_ERROR_STATE
} WII_OPERATE_STATE;

/*!
 @brief Driver for Wiimote operation menu

 Handles parsing wiimote input to determine correct operating mode.

 This function will never exit once entered.
 */
ErrorID_t main_menu(cwiid_wiimote_t *wiimote,
		volatile WiimoteStatusDataType *wiimote_status)
{
	ErrorID_t error;
	uint8_t menu_count;
	uint16_t last_button_state = 0;
	WII_OPERATE_STATE wii_operate_state = WII_OPERATE_INIT_MENU;

	write_status_led(STATUS_LED_OFF, 0);

	for (;;)
	{
		switch (wii_operate_state)
		{
		case WII_OPERATE_INIT_MENU:
			menu_count = 0;
			last_button_state = 0;
			wii_operate_state = WII_OPERATE_WAIT_FOR_BUTTON_PRESS;
			write_status_led(STATUS_LED_FLASH, 500);

			if (0 > cwiid_set_led(wiimote, CWIID_LED1_ON))
				return WII_ERROR_TRANSMIT;
			if (0 > cwiid_enable(wiimote, CWIID_FLAG_MESG_IFC))
				return WII_ERROR_TRANSMIT;
			if (0 > cwiid_set_rpt_mode(wiimote, CWIID_RPT_BTN))
				return WII_ERROR_TRANSMIT;

			/* Menu */
			debug_print("%s", MENU);
			set_lcd(0, "Connected.");
			set_lcd(1, "Select Mode.");
			break;

		case WII_OPERATE_WAIT_FOR_BUTTON_PRESS:
			error = wait_for_wiimotedata(wiimote_status, 2000);
			if (error == ERR_WII_DATA_TIMEOUT)
			{
				set_lcd(0, "Select Mode");
				set_lcd(1, (char *) lcd_menu_strings[menu_count]);
				if (++menu_count >= MENU_ENTRIES)
					menu_count = 0;
			}
			else if (0 < error)
			{
				exit(-3);
			}
			else if (wiimote_status->button_data != last_button_state)
			{
				last_button_state = wiimote_status->button_data;
				if (last_button_state & CWIID_BTN_A) // collect acceleration data
				{
					wii_operate_state = WII_OPERATE_DISPLAY_ACCELEROMETER_INFO;
				}
				else if (last_button_state & CWIID_BTN_B) // collect IR data
				{
					wii_operate_state = WII_OPERATE_DISPLAY_IR_STATUS;
				}
				else if (last_button_state & CWIID_BTN_HOME) // reset
				{
					shutdown_all(wiimote);
					return true;
				}
				else if (last_button_state & CWIID_BTN_PLUS)
				{
					sensor_cutoff_fwd += 5;
					if (sensor_cutoff_fwd > 100)
						sensor_cutoff_fwd = 100;
#if REVERSE_SENSOR_PRESENT
					sensor_cutoff_rev += 5;
					if (sensor_cutoff_rev > 100)
					sensor_cutoff_rev = 100;
#endif

				}
				else if (last_button_state & CWIID_BTN_MINUS)
				{
					sensor_cutoff_fwd -= 5;
					if (sensor_cutoff_fwd < 0)
						sensor_cutoff_fwd = 0;
#if REVERSE_SENSOR_PRESENT
					sensor_cutoff_rev -= 5;
					if (sensor_cutoff_rev < 0)
					sensor_cutoff_rev = 0;
#endif
				}
				else if (last_button_state & (CWIID_BTN_2 | CWIID_BTN_1))
				{
					wii_operate_state = WII_OPERATE_DISPLAY_BUTTON_STATUS;
				}
			}
			break;

		case WII_OPERATE_DISPLAY_ACCELEROMETER_INFO:
			if (ERR_NONE != acceleration_mode(wiimote, wiimote_status))
				wii_operate_state = WII_OPERATE_ERROR_STATE;
			else
				wii_operate_state = WII_OPERATE_INIT_MENU;
			break;

		case WII_OPERATE_DISPLAY_IR_STATUS:
			if (ERR_NONE != infrared_mode(wiimote, wiimote_status))
				wii_operate_state = WII_OPERATE_ERROR_STATE;
			else
				wii_operate_state = WII_OPERATE_INIT_MENU;
			break;

		case WII_OPERATE_DISPLAY_BUTTON_STATUS:
			if (ERR_NONE != button_mode(wiimote, wiimote_status))
				wii_operate_state = WII_OPERATE_ERROR_STATE;
			else
				wii_operate_state = WII_OPERATE_INIT_MENU;
			break;

		case WII_OPERATE_ERROR_STATE:
			if (ERR_NONE == error_mode(wiimote, wiimote_status))
				wii_operate_state = WII_OPERATE_INIT_MENU;
			break;
		}
	}
	return ERR_NONE;
}

/*!
 @brief Determines motor parameters from IR data.
 */
ErrorID_t infrared_mode(cwiid_wiimote_t *wiimote,
		volatile WiimoteStatusDataType *wiimote_status)
{

	typedef enum WiimoteInfraredStateType
	{
		WIIMOTE_INFRARED_WAIT_FOR_START,
		WIIMOTE_INFRARED_ENABLE,
		WIIMOTE_INFRARED_READ_DATA,
		WIIMOTE_INFRARED_WAIT_FOR_EXIT,
		WIIMOTE_INFRARED_EXIT,
	} WiimoteInfraredStateType;

	ErrorID_t error_flag = ERR_NONE;

	bool last_valid = false;
	bool valid_points = false;

	WiimoteInfraredStateType state = WIIMOTE_INFRARED_WAIT_FOR_START;

	write_status_led(STATUS_LED_OFF, 0);

	for (;;)
	{
		switch (state)
		{
		case WIIMOTE_INFRARED_WAIT_FOR_START:
			if (0 < wait_for_wiimotedata(wiimote_status, 500))
			{
				error_flag = WII_ERROR_DATA_TIMEOUT;
				state = WIIMOTE_INFRARED_EXIT;
			}
			else if (0 == (wiimote_status->button_data & CWIID_BTN_B))
			{
				state = WIIMOTE_INFRARED_ENABLE;
			}
			break;

		case WIIMOTE_INFRARED_ENABLE:
			set_ir_led(STATUS_LED_ON);

			debug_print("Setting IR Report\n");

			if (0 < cwiid_set_rpt_mode(wiimote, CWIID_RPT_IR | CWIID_RPT_BTN))
			{
				debug_print("Cannot set report mode to IR\n");
				return false;
			}

			set_lcd(0, "Infrared Mode");
			set_lcd(1, "Point wiimote at car");

			write_status_led(STATUS_LED_ON, 0);

			state = WIIMOTE_INFRARED_READ_DATA;
			break;

		case WIIMOTE_INFRARED_READ_DATA:
			if (0 < wait_for_wiimotedata(wiimote_status, 500))
			{
				error_flag = WII_ERROR_DATA_TIMEOUT;
				state = WIIMOTE_INFRARED_EXIT;
			}
			else if (wiimote_status->button_data & CWIID_BTN_B)
			{
				state = WIIMOTE_INFRARED_WAIT_FOR_EXIT;
			}
			else
			{
				error_flag = WiiComputeMotorLevelsInfrared(wiimote_status,
						&valid_points);
				if (valid_points != last_valid)
				{
					write_status_led(
							valid_points ? STATUS_LED_OFF : STATUS_LED_ON, 0);
					set_lcd(
							1,
							valid_points ?
									"LED's seen" : "Point wiimote at car");
				}

				last_valid = valid_points;
				if (ERR_EXEC == error_flag)
					cwiid_set_rumble(wiimote, true);
				else
				{
					cwiid_set_rumble(wiimote, false);
					if (error_flag < 0)
						state = WIIMOTE_INFRARED_EXIT;
				}
			}
			break;

		case WIIMOTE_INFRARED_WAIT_FOR_EXIT:
			if (0 < wait_for_wiimotedata(wiimote_status, 500))
			{
				state = WIIMOTE_INFRARED_EXIT;
			}
			else if (0 == (wiimote_status->button_data & CWIID_BTN_B))
			{
				state = WIIMOTE_INFRARED_EXIT;
			}
			break;
		case WIIMOTE_INFRARED_EXIT:
			stop_motors();
			write_status_led(STATUS_LED_OFF, 0);
			set_ir_led(false);
			return error_flag;
			break;

			break;
		}
	}
	return true;
}

/*!
 @brief Computes motor parameters from Accel Data.
 */

ErrorID_t acceleration_mode(cwiid_wiimote_t *wiimote,
		volatile WiimoteStatusDataType *wiimote_status)
{
	typedef enum WiimoteAccelStateType
	{
		WIIMOTE_ACCEL_WAIT_FOR_START,
		WIIMOTE_ACCEL_READ_CAL_DATA,
		WIIMOTE_ACCEL_READ_DATA,
		WIIMOTE_ACCEL_WAIT_FOR_EXIT,
		WIIMOTE_ACCEL_EXIT,
	} WiimoteAccelStateType;

	const uint8_t RETRY_VALUE = 2;
	uint8_t retry_count = 0;
	ErrorID_t error_flag = ERR_NONE;

	WiimoteAccelStateType state = WIIMOTE_ACCEL_WAIT_FOR_START;

	debug_print("Entering acceleration mode...\n");
	write_status_led(STATUS_LED_OFF, 0);

	for (;;)
	{
		switch (state)
		{
		case WIIMOTE_ACCEL_WAIT_FOR_START:
			if (0 > wait_for_wiimotedata(wiimote_status, INFINITE_TIMEOUT))
			{
				debug_print("No start signal received\n");
				error_flag = WII_ERROR_DATA_TIMEOUT;
				state = WIIMOTE_ACCEL_EXIT;
			}
			else if (0 == (wiimote_status->button_data & CWIID_BTN_A))
			{
				state = WIIMOTE_ACCEL_READ_CAL_DATA;
			}
			break;

		case WIIMOTE_ACCEL_READ_CAL_DATA:
			error_flag = WII_ERROR_ACCEL_CAL;
			do
			{
				if (0
						== cwiid_get_acc_cal(
								wiimote,
								CWIID_EXT_NONE,
								(struct acc_cal *) &wiimote_status->accel_cal_data))
				{
					error_flag = ERR_NONE;
					break;
				}
			} while (retry_count++ < RETRY_VALUE);

			if (error_flag != ERR_NONE)
			{
				debug_print("Cannot read cal data\n");
				return error_flag;
			}

			if (0 > cwiid_set_rpt_mode(wiimote, CWIID_RPT_ACC | CWIID_RPT_BTN))
			{
				debug_print("Cannot set report mode to ACCEL\n");
				return false;
			}

			set_lcd(0, "Accel Mode...");
			set_lcd(1, "P = %4d R = %4d", 0, 0);
			state = WIIMOTE_ACCEL_READ_DATA;
			break;

		case WIIMOTE_ACCEL_READ_DATA:
			if (0 > wait_for_wiimotedata(wiimote_status, INFINITE_TIMEOUT))
			{
				debug_print("@%u: WII_ERROR_DATA_TIMEOUT\n", get_tick_count());
				error_flag = WII_ERROR_DATA_TIMEOUT;
				state = WIIMOTE_ACCEL_EXIT;
			}
			else
			{
				if (wiimote_status->button_data & CWIID_BTN_A)
				{
					state = WIIMOTE_ACCEL_WAIT_FOR_EXIT;
				}
				else
				{
					error_flag = computer_motor_levels_accel(wiimote_status);
					set_lcd(1, "P = %4d R = %4d",
							wiimote_status->accel_computed_data.pitch / 100,
							wiimote_status->accel_computed_data.roll / 100);

					if (ERR_EXEC == error_flag)
						cwiid_set_rumble(wiimote, true);
					else
					{
						cwiid_set_rumble(wiimote, false);
						if (error_flag < 0)
						{
							debug_print("@%u: CONTROL CAR COMM ERROR: %d\n",
									get_tick_count(), error_flag);
							state = WIIMOTE_ACCEL_EXIT;
						}
					}

				}
			}
			break;

		case WIIMOTE_ACCEL_WAIT_FOR_EXIT:
			stop_motors();

			if (0 < wait_for_wiimotedata(wiimote_status, INFINITE_TIMEOUT))
			{
				error_flag = WII_ERROR_DATA_TIMEOUT;
				state = WIIMOTE_ACCEL_EXIT;
			}
			else if (0 == (wiimote_status->button_data & CWIID_BTN_A))
			{
				state = WIIMOTE_ACCEL_EXIT;
			}
			break;
		case WIIMOTE_ACCEL_EXIT:
			debug_print("Exiting acceleration mode.\n\n");
			return error_flag;
		}
	}
	return ERR_NONE;
}

ErrorID_t button_mode(cwiid_wiimote_t *wiimote,
		volatile WiimoteStatusDataType *wiimote_status)
{
	int32_t speed = SPEED_NULL_VALUE;
	int32_t direction = 0;
	bool run = true;

	cwiid_set_rpt_mode(wiimote, CWIID_RPT_BTN);

	debug_print("Entering button mode:\n");
	write_status_led(STATUS_LED_OFF, 0);

	set_lcd(0, "Button Mode");
	set_lcd(1, "2-fwd,1-back,dpad-steer");

	do
	{
		wait_for_wiimotedata(wiimote_status, 25);

		if ((wiimote_status->button_data & CWIID_BTN_2)
				&& (wiimote_status->button_data & CWIID_BTN_1))
		{
			if (speed > 0)
			{
				speed -= MAX_FORWARD_SPEED / 10;
				if (speed < 0)
					speed = 0;
			}
			else if (speed < 0)
			{
				speed -= MAX_REVERSE_SPEED / 10;
				if (speed > 0)
					speed = 0;
			}
		}
		else if (wiimote_status->button_data & CWIID_BTN_2)
		{
			speed += MAX_FORWARD_SPEED / 10;
			if (speed > MAX_FORWARD_SPEED)
				speed = MAX_FORWARD_SPEED;
		}
		else if (wiimote_status->button_data & CWIID_BTN_1)
		{
			speed += MAX_REVERSE_SPEED / 10;
			if (speed < MAX_REVERSE_SPEED)
				speed = MAX_REVERSE_SPEED;
		}
		else
		{
			speed = 0;
			direction = 0;
			run = false;
		}

		if (wiimote_status->button_data & CWIID_BTN_UP)
		{
			direction -= (MAX_LEFT_DIRECTION / 10);
			if (direction < -MAX_LEFT_DIRECTION)
				direction = -MAX_LEFT_DIRECTION;
		}
		else if (wiimote_status->button_data & CWIID_BTN_DOWN)
		{
			direction += (MAX_RIGHT_DIRECTION / 10);
			if (direction > MAX_RIGHT_DIRECTION)
				direction = MAX_RIGHT_DIRECTION;
		}
		else
			direction = 0;

		debug_print("@%u: Button (speed, heading) = %d %d\n", get_tick_count(),
				speed, direction);

		write_motor_levels(speed, ComputeDirectionMotor(direction));
	} while (run);
	return ERR_NONE;
}

int32_t shutdown_all(cwiid_wiimote_t *wiimote)
{
	stop_motors();
	set_ir_led(false);
	write_status_led(STATUS_LED_OFF, 0);
	return cwiid_close(wiimote);
}

/* Prototype cwiid_callback with cwiid_callback_t, define it with the actual
 * type - this will cause a compile error (rather than some undefined bizarre
 * behavior) if cwiid_callback_t changes */
/* cwiid_mesg_callback_t has undergone a few changes lately, hopefully this
 * will be the last.  Some programs need to know which messages were received
 * simultaneously (e.g. for correlating accelerometer and IR data), and the
 * sequence number mechanism used previously proved cumbersome, so we just
 * pass an array of messages, all of which were received at the same time.
 * The id is to distinguish between multiple wiimotes using the same callback.
 * */
void cwiid_callback(cwiid_wiimote_t *wiimote, int mesg_count,
		union cwiid_mesg mesg[], struct timespec *timestamp)
{
	int i, j;
	int valid_source;

#if _DEBUG >= 2
	debug_print("@%u: msg received\n",get_tick_count());
#endif

	for (i = 0; i < mesg_count; i++)
	{
		switch (mesg[i].type)
		{
		case CWIID_MESG_STATUS:
#if _DEBUG >= 2
			debug_print("@%u: Status Report: battery=%d extension=", get_tick_count(),
					mesg[i].status_mesg.battery);
#endif
			wiimote_status_data.battery_level = mesg[i].status_mesg.battery;
			switch (mesg[i].status_mesg.ext_type)
			{
			case CWIID_EXT_NONE:
#if _DEBUG >= 2
				debug_print("none\n");
#endif
				break;
			case CWIID_EXT_NUNCHUK:
#if _DEBUG >= 2
				debug_print("Nunchuk\n");
#endif
				break;
			case CWIID_EXT_CLASSIC:
#if _DEBUG >= 2
				debug_print("Classic Controller\n");
#endif
				break;
			default:
#if _DEBUG >= 2
				debug_print("Unknown Extension\n");
#endif
				break;
			}
			break;
		case CWIID_MESG_BTN:
#if _DEBUG >= 2
			debug_print("@%u: Button Report: %.4X\n", get_tick_count(), mesg[i].btn_mesg.buttons);
#endif
			wiimote_status_data.button_data = mesg[i].btn_mesg.buttons;

			break;
		case CWIID_MESG_ACC:
#if _DEBUG >= 2
			debug_print("@%u: Acc Report: x=%d, y=%d, z=%d\n", get_tick_count(),
					mesg[i].acc_mesg.acc[CWIID_X],
					mesg[i].acc_mesg.acc[CWIID_Y],
					mesg[i].acc_mesg.acc[CWIID_Z]);
#endif
			for (j = 0; j < 3; j++)
			{
				wiimote_status_data.accel_raw_data[j] = mesg[i].acc_mesg.acc[j];
			}
			break;
		case CWIID_MESG_IR:
#if _DEBUG >= 2
			debug_print("@%u: IR Report: ", get_tick_count());
#endif
			valid_source = 0;
			for (j = 0; j < CWIID_IR_SRC_COUNT; j++)
			{
#if _DEBUG >= 2
				if (mesg[i].ir_mesg.src[j].valid)
				{
					valid_source = 1;
					debug_print("(%d,%d) ", mesg[i].ir_mesg.src[j].pos[CWIID_X],
							mesg[i].ir_mesg.src[j].pos[CWIID_Y]);

				}
#endif
				wiimote_status_data.ir_raw_data.WiimoteIRPoint[j].valid =
						mesg[i].ir_mesg.src[j].valid;
				wiimote_status_data.ir_raw_data.WiimoteIRPoint[j].size =
						mesg[i].ir_mesg.src[j].size;
				wiimote_status_data.ir_raw_data.WiimoteIRPoint[j].pos[0] =
						mesg[i].ir_mesg.src[j].pos[CWIID_X];
				wiimote_status_data.ir_raw_data.WiimoteIRPoint[j].pos[1] =
						mesg[i].ir_mesg.src[j].pos[CWIID_Y];
			}
#if _DEBUG >= 2
			if (!valid_source)
			{
				debug_print("@%u: no sources detected", get_tick_count());
			}
			printf("\n");
#endif
			break;
		case CWIID_MESG_NUNCHUK:
#if _DEBUG >= 2
			debug_print("@%u: Nunchuk Report: btns=%.2X stick=(%d,%d) acc.x=%d acc.y=%d "
					"acc.z=%d\n", get_tick_count(), mesg[i].nunchuk_mesg.buttons,
					mesg[i].nunchuk_mesg.stick[CWIID_X],
					mesg[i].nunchuk_mesg.stick[CWIID_Y],
					mesg[i].nunchuk_mesg.acc[CWIID_X],
					mesg[i].nunchuk_mesg.acc[CWIID_Y],
					mesg[i].nunchuk_mesg.acc[CWIID_Z]);
#endif
			break;
		case CWIID_MESG_CLASSIC:
#if _DEBUG >= 2
			debug_print("@%u: Classic Report: btns=%.4X l_stick=(%d,%d) r_stick=(%d,%d) "
					"l=%d r=%d\n", get_tick_count(), mesg[i].classic_mesg.buttons,
					mesg[i].classic_mesg.l_stick[CWIID_X],
					mesg[i].classic_mesg.l_stick[CWIID_Y],
					mesg[i].classic_mesg.r_stick[CWIID_X],
					mesg[i].classic_mesg.r_stick[CWIID_Y],
					mesg[i].classic_mesg.l, mesg[i].classic_mesg.r);
#endif
			break;
		case CWIID_MESG_ERROR:
			if (cwiid_close(wiimote))
			{
				fprintf(stderr, "@%u: Error on wiimote disconnect\n",
						get_tick_count());
				exit(-1);

			}
			exit(0);
			break;
		default:
#if _DEBUG > 2
			debug_print("@%u: Unknown Report", get_tick_count());
#endif
			break;
		}
	}
	signal_wiimote_data_ready(&wiimote_status_data, 100);
}

#if _MUTEX_ENABLE
void update_timeout_value(struct timespec *ts, int32_t timeout)
{
	clock_gettime(CLOCK_REALTIME, ts);
	if (1000 <= timeout)
	ts->tv_sec += timeout / 1000;
	else
	{
		ts->tv_nsec += timeout * 1000 * 1000; // one second timeout
		if (1000000000 <= ts->tv_nsec)
		{
			ts->tv_nsec -= 1000000000;
			ts->tv_sec++;
		}
	}
}
#endif

ErrorID_t wait_for_wiimotedata(volatile WiimoteStatusDataType *wiimote_status,
		int32_t timeout)
{
	ErrorID_t ret_val = ERR_NONE;

#if _MUTEX_ENABLE
	static struct timespec abstime;
	if (0 < timeout)
	{
		update_timeout_value(&abstime, timeout);
		ret_val = pthread_cond_timedwait(&cond, &mutex, &abstime);
		if (ETIMEDOUT == ret_val)
		{
			fprintf(stderr, "@%u: Wiimote data timeout.\n", get_tick_count());
		}

	}
	else
	{
		ret_val = pthread_cond_wait(&cond, &mutex);
	}

	if (0 <= ret_val)
	{
#if (_DEBUG >= 2)
		fprintf(stderr, "@%u: Wiimote data received.\n", get_tick_count());
#endif
	}
#else
	uint32_t start_time = get_tick_count();
	wiimote_data_ready = false;
	while (!wiimote_data_ready)
	{
		if (check_for_timeout(get_tick_count(), start_time, timeout))
		{
			debug_print("%u: wiimote data timeout, timeout = %d\n",
					get_tick_count(), timeout);
			return ERR_WII_DATA_TIMEOUT;
		}

		usleep(25 * 1000);
	}
#endif
	return ret_val;
}

int32_t signal_wiimote_data_ready(
		volatile WiimoteStatusDataType *wiimote_status, int32_t timeout)
{
#if _MUTEX_ENABLE
	int32_t ret_val;
	ret_val = pthread_cond_broadcast(&cond);
	if (0 > ret_val)
	return ret_val;
	return pthread_mutex_unlock(&mutex);
#if _DEBUG >= 2
	debug_print("@%u: Wiimote data received\n", get_tick_count());
#endif
	return ret_val;
#else
	wiimote_data_ready = true;
	return ERR_NONE;
#endif
}

ErrorID_t error_mode(cwiid_wiimote_t *wiimote,
		volatile WiimoteStatusDataType *wiimote_status)
{
	debug_print("@%u: An error has occurred\n", get_tick_count());
	shutdown_all(wiimote);
	exit(-1);
}

