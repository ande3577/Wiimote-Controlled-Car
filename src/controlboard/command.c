#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <wiicarutility/error_message.h>

#include "comm.h"
#include "control_board.h"

char params[256];
char lcd_line_text[LCD_TEXT_LINES][256];

int32_t motor_level[NUMBER_OF_MOTOR_CHANNELS] =
{ 0 };

int32_t sensor_values[NUMBER_OF_SENSOR_CHANNELS] =
{ 0 };

bool ir_led_value = false;

struct led_flash_status_t status_led_status;

struct led_flash_status_t error_led_status;

int32_t motor_timeout;
bool motor_timeout_changed = true;
uint32_t timestamp;

error_info_t last_error;

char pgm_info[256];

int32_t write_motor_levels(int32_t channel1, int32_t channel2)
{
	int32_t ret_val = comm_query(params, "SML %d %d", channel1, channel2);
	if (ret_val == ERR_NONE)
	{
		motor_level[MOTOR_SPEED_CHANNEL] = channel1;
		motor_level[MOTOR_DIRECTION_CHANNEL] = channel2;
	}
	return ret_val;
}

int32_t read_motor_levels()
{
	int32_t ret_val;
	ret_val = comm_query(params, "GML");
	if (0 > ret_val)
		return ret_val;

	int32_t motor_channels_temp[NUMBER_OF_MOTOR_CHANNELS];
	ret_val = sscanf(params, "%d %d", &motor_channels_temp[0],
			&motor_channels_temp[1]);
	if (ret_val == NUMBER_OF_MOTOR_CHANNELS)
	{
		uint8_t i;
		for (i = 0; i < NUMBER_OF_MOTOR_CHANNELS; i++)
		{
			motor_level[i] = motor_channels_temp[i];
		}
		return ERR_NONE;
	}
	else
	{
		return ERR_PARAM;
	}
}

const int32_t *get_motor_levels()
{
	return motor_level;
}

int32_t read_sensor_values()
{
	int32_t ret_val;
	ret_val = comm_query(params, "GSV");
	if (0 > ret_val)
		return ret_val;

	int32_t sensor_values_temp[NUMBER_OF_SENSOR_CHANNELS];
	ret_val = sscanf(params, "%d %d %d %d %d", &sensor_values_temp[0],
			&sensor_values_temp[1], &sensor_values_temp[2],
			&sensor_values_temp[3], &sensor_values_temp[4]);

	if (ret_val == NUMBER_OF_SENSOR_CHANNELS)
		return ERR_NONE;
	else
		return ERR_PARAM;
}

const int32_t *get_sensor_values()
{
	return sensor_values;
}

int32_t set_motor_timeout(int32_t timeout)
{
	int32_t ret_val = comm_query(params, "SMT %d", timeout);
	if (ret_val == ERR_NONE)
		motor_timeout = timeout;
	return ret_val;
}

int32_t read_motor_timeout()
{
	int32_t ret_val;
	ret_val = comm_query(params, "GMT");
	if (0 > ret_val)
		return ret_val;

	int32_t timeout_temp;
	ret_val = sscanf(params, "%d", &timeout_temp);
	if (ret_val == 1)
	{
		motor_timeout = timeout_temp;
		return ERR_NONE;
	}
	else
	{
		return ERR_PARAM;
	}
}

const int32_t *get_motor_timeout()
{
	return &motor_timeout;
}

int32_t set_ir_led(bool on)
{
	int32_t ret_val;
	if (on)
		ret_val = comm_query(params, "SIL ON");
	else
		ret_val = comm_query(params, "SIL OFF");

	if (ret_val == ERR_NONE)
		ir_led_value = on;

	return ret_val;
}

int32_t read_ir_led()
{
	int32_t ret_val;
	char ir_state[16];
	ret_val = comm_query(params, "GIL");
	if (0 > ret_val)
		return ret_val;

	ret_val = sscanf(params, "%s", ir_state);
	if (ret_val != 1)
		return ERR_PARAM;

	if (!strncmp(ir_state, "OFF", strlen("OFF")))
		ir_led_value = false;
	else if (!strncmp(ir_state, "ON", strlen("ON")))
		ir_led_value = true;
	else
		return ERR_PARAM;

	return ERR_NONE;
}

const bool *get_ir_led()
{
	return &ir_led_value;
}

static int32_t write_led(char *write_command, StatusLedFlashState_t led_state,
		int32_t flash_rate, led_flash_status_t *flash_status_out)
{
	int32_t ret_val;
	switch (led_state)
	{
	case STATUS_LED_OFF:
		ret_val = comm_query(params, "%s OFF", write_command);
		break;
	case STATUS_LED_ON:
		ret_val = comm_query(params, "%s ON", write_command);
		break;
	case STATUS_LED_FLASH:
		ret_val = comm_query(params, "%s FLASH %d", write_command, flash_rate);
		break;
	default:
		ret_val = ERR_PARAM;
		break;
	}

	if (ret_val == ERR_NONE)
	{
		flash_status_out->state = led_state;
		flash_status_out->flash_rate = flash_rate;
	}

	return ret_val;

}

static int32_t read_led(char *read_command, led_flash_status_t *flash_status)
{
	int32_t ret_val;
	char led_string[16];
	int32_t flash_rate_temp;
	ret_val = comm_query(params, read_command);
	if (0 > ret_val)
		return ret_val;

	ret_val = sscanf(params, "%s %d", led_string, &flash_rate_temp);
	if ((ret_val == 1) || (ret_val == 2))
	{
		if (!strncmp(led_string, "OFF", strlen("OFF")))
		{
			flash_status->state = STATUS_LED_OFF;
			flash_status->flash_rate = 0;
		}
		else if (!strncmp(led_string, "ON", strlen("ON")))
		{
			flash_status->state = STATUS_LED_ON;
			flash_status->flash_rate = 0;
		}
		else if (!strncmp(led_string, "FLASH", strlen("FLASH"))
				&& (2 == ret_val))
		{
			flash_status->state = STATUS_LED_FLASH;
			flash_status->flash_rate = flash_rate_temp;
		}
		else
			return ERR_PARAM;
	}

	return ERR_NONE;
}

int32_t write_status_led(StatusLedFlashState_t led_state, int32_t flash_rate)
{
	return write_led("SSL", led_state, flash_rate, &status_led_status);
}

int32_t read_status_led()
{
	return read_led("GSL", &status_led_status);
}

const led_flash_status_t *get_status_led()
{
	return &status_led_status;
}

int32_t write_error_led(StatusLedFlashState_t led_state, int32_t flash_rate)
{
	return write_led("SEL", led_state, flash_rate, &error_led_status);
}

int32_t read_error_led()
{
	return read_led("GEL", &error_led_status);
}

const led_flash_status_t *get_error_led()
{
	return &error_led_status;
}

int32_t read_current_time()
{
	int32_t ret_val;
	ret_val = comm_query(params, "TIM");
	if (0 > ret_val)
		return ret_val;

	int32_t timestamp_temp;
	ret_val = sscanf(params, "%d", &timestamp_temp);
	if (ret_val == 1)
	{
		timestamp = timestamp_temp;
		return ERR_NONE;
	}
	else
	{
		return ERR_PARAM;
	}
}

const uint32_t *get_current_time()
{
	return &timestamp;
}

int32_t read_last_error()
{
	int32_t ret_val;
	ret_val = comm_query(params, "GLE");
	if (0 > ret_val)
		return ret_val;

	char error_id_string[16];
	uint32_t error_timestamp;

	ret_val = sscanf(params, "%s %d", error_id_string, &error_timestamp);
	if (ret_val == 2)
	{
		last_error.error_id = decode_error_response(error_id_string);
		last_error.timestamp = error_timestamp;
	}

	return ERR_NONE;
}

const error_info_t *get_last_error()
{
	return &last_error;
}

int32_t read_program_info()
{
	int32_t ret_val;
	ret_val = comm_query(params, "PGM");
	if (0 > ret_val)
		return ret_val;

	uint8_t i = 0;

	do
	{
		pgm_info[i] = params[i];
		i++;
	} while ((pgm_info[i - 1] != '\n') && (pgm_info[i - 1] != '\0'));

	pgm_info[i] = '\0';

	return ERR_NONE;
}

const char *get_program_info()
{
	return pgm_info;
}

int32_t send_password(char *password)
{
	return comm_query(params, "ICB %s", password);
}

int32_t send_jump_to_boot(void)
{
	return comm_query(params, "SDN");
}

int32_t shutdown()
{
	return comm_query(params, "SDN");
}

int32_t set_lcd(int32_t line, char *fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	vsprintf(lcd_line_text[line], fmt, args);

#if LCD_SUPPORTED
	return comm_query(params, "SLD %d \"%s\"", line, lcd_line_text[line]);
#else
#if _DEBUG
	printf("SLD %d \"%s\"\n", line, lcd_line_text[line]);
#endif
	return ERR_NONE;
#endif
}

const char *get_lcd(int32_t line)
{
	return lcd_line_text[line];
}
