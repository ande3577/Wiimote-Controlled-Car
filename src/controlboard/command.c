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
char lcd_text[256];

int32_t set_motor_levels(int32_t channel1, int32_t channel2)
{
	return comm_query(params, "SML %d %d", channel1, channel2);
}

int32_t get_motor_levels(int32_t *channel1, int32_t *channel2)
{
	int32_t ret_val;
	ret_val = comm_query(params, "GML");
	if (0 > ret_val)
		return ret_val;

	ret_val = sscanf(params, "%d %d", channel1, channel2);
	if (ret_val == 2)
		return ERR_NONE;
	else
		return ERR_PARAM;
}

int32_t get_motor_pwm_counts(int32_t *channel1, int32_t *channel2)
{
	int32_t ret_val;
	ret_val = comm_query(params, "GMP");
	if (0 > ret_val)
		return ret_val;

	ret_val = sscanf(params, "%d %d", channel1, channel2);
	if (ret_val == 2)
		return ERR_NONE;
	else
		return ERR_PARAM;
}

int32_t get_sensor_value(int32_t *channel1, int32_t *channel2)
{
	int32_t ret_val;
	ret_val = comm_query(params, "GSV");
	if (0 > ret_val)
		return ret_val;

	ret_val = sscanf(params, "%d %d", channel1, channel2);
	if (ret_val == 2)
		return ERR_NONE;
	else
		return ERR_PARAM;
}

int32_t set_motor_timeout(int32_t timeout)
{
	return comm_query(params, "SMT %d", timeout);
}

int32_t get_motor_timeout(int32_t *timeout)
{
	int32_t ret_val;
	ret_val = comm_query(params, "GMT");
	if (0 > ret_val)
		return ret_val;

	ret_val = sscanf(params, "%d", timeout);
	if (ret_val == 1)
		return ERR_NONE;
	else
		return ERR_PARAM;
}

int32_t set_ir_led(bool on)
{
	if (on)
		return comm_query(params, "SIL ON");
	else
		return comm_query(params, "SIL OFF");
}

int32_t get_ir_led(bool *on)
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
		*on = false;
	else if (!strncmp(ir_state, "ON", strlen("ON")))
		*on = true;
	else
		return ERR_PARAM;

	return ERR_NONE;
}

int32_t set_status_led(StatusLedFlashState_t led_state, int32_t flash_rate)
{
	switch (led_state)
	{
	case STATUS_LED_OFF:
		return comm_query(params, "SSL OFF");
		break;
	case STATUS_LED_ON:
		return comm_query(params, "SSL ON");
		break;
	case STATUS_LED_FLASH:
		return comm_query(params, "SSL FLASH %d", flash_rate);
		break;
	default:
		return ERR_PARAM;
	}

}

int32_t get_status_led(StatusLedFlashState_t *led_state, int32_t *flash_rate)
{
	int32_t ret_val;
	char led_string[16];
	int32_t flash_rate_temp;
	ret_val = comm_query(params, "GSL");
	if (0 > ret_val)
		return ret_val;

	ret_val = sscanf(params, "%s %d", led_string, &flash_rate_temp);
	if ((ret_val == 1) || (ret_val == 2))
	{
		if (!strncmp(led_string, "OFF", strlen("OFF")))
		{
			*led_state = STATUS_LED_OFF;
			*flash_rate = 0;
		}
		else if (!strncmp(led_string, "ON", strlen("ON")))
		{
			*led_state = STATUS_LED_ON;
			*flash_rate = 0;
		}
		else if (!strncmp(led_string, "FLASH", strlen("FLASH")) && (2
				== ret_val))
		{
			*led_state = STATUS_LED_FLASH;
			*flash_rate = flash_rate_temp;
		}
		else
			return ERR_PARAM;
	}

	return ERR_NONE;
}

int32_t get_current_time(uint32_t *time)
{
	int32_t ret_val;
	ret_val = comm_query(params, "TIM");
	if (0 > ret_val)
		return ret_val;

	ret_val = sscanf(params, "%d", time);
	if (ret_val == 1)
		return ERR_NONE;
	else
		return ERR_PARAM;
}

int32_t get_last_error(int32_t *error_id, int32_t *timestamp)
{
	int32_t ret_val;
	char error_id_string[16];
	ret_val = comm_query(params, "GLE");
	if (0 > ret_val)
		return ret_val;

	ret_val = sscanf(params, "%s %d", error_id_string, timestamp);
	if (ret_val == 2)
	{
		*error_id = decode_error_response(error_id_string);
	}

	return ERR_NONE;
}

int32_t get_program_info(char *pgm_info)
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

int32_t send_password(char *password)
{
	return comm_query(params, "ICB %s", password);
}

int32_t send_jump_to_boot(void)
{
	return comm_query(params, "SDN");
}

int32_t get_pushbuttons(bool *pressed)
{
#if PUSHBUTTONS_SUPPORTED
	int32_t ret_val, i;
	int32_t button_int[NUMBER_OF_PUSHBUTTONS];

	ret_val = comm_query(params, "GPB");
	if (0 > ret_val)
		return ret_val;
	sscanf(params, "%d %d %d %d %d", &button_int[0], &button_int[1],
			&button_int[2], &button_int[3], &button_int[4]);

	for (i = 0; i < NUMBER_OF_PUSHBUTTONS; i++)
		pressed[i] = (button_int[i] != 0);

	return ret_val;
#else
	return ERR_NONE;
#endif
}

int32_t clear_lcd(void)
{
#if LCD_SUPPORTED
	return comm_query(params, "CLD");
#else
	return ERR_NONE;
#endif
}

int32_t set_lcd(int32_t line, char *fmt, ...)
{
#if LCD_SUPPORTED
	va_list args;
	va_start(args,fmt);
	vsprintf(lcd_text, fmt, args);

	return comm_query(params, "SLD %d \"%s\"", line, lcd_text);
#else
	return ERR_NONE;
#endif
}

int32_t lcd_putchars(int32_t line, int32_t ch, char *fmt, ...)
{
#if LCD_SUPPORTED
	va_list args;
	va_start(args,fmt);
	vsprintf(lcd_text, fmt, args);

	return comm_query(params, "PLC %d %d \"%s\"", line, ch, lcd_text);
#else
	return ERR_NONE;
#endif
}

int32_t shutdown()
{
	return comm_query(params, "SDN");
}
