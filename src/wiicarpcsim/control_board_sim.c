#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <pthread.h>
#include <gtk/gtk.h>
#include <unistd.h>

#include "timestamp.h"
#include "control_board.h"
#include "error_message.h"
#include "hardware.h"
#include "wiicargui.h"

/// TODO rework this so it can update GUI and talk to control board at the same time

pthread_t gtk_thread;
pthread_t draw_thread;

volatile bool is_open = false;

struct window_ref_t *wm;

GtkWidget* error_dialog;

#define GRAPHIC_UPDATE_DELAY 100

bool diagnostic_mode = false;

// dummy functions to prevent warnings
void on_get_button_clicked(GtkObject *object, gpointer wn)
{
}

void on_jump_to_boot_clicked(GtkObject *object, gpointer wn)
{
}

void on_set_button_clicked(GtkObject *object, gpointer wn)
{
}

void on_clear_button_clicked(GtkObject *object, gpointer wn)
{
}

void on_refresh_ports_clicked(GtkObject *object, gpointer wn)
{
}

void on_com_port_changed(GtkObject *object, gpointer wn)
{
}

void *gtk_launch_function(void *ptr)
{
	gtk_main();
	return NULL;
}

void *gtk_draw_function(void *ptr);

void set_comm_trace(bool enabled)
{
	comm_trace = enabled;
}

static bool timer_handler(struct window_ref_t *wm)
{
	gtk_draw_function(wm);
	return true;
}

int32_t comm_query(char *parameters, const char *fmt, ...)
{
	int32_t ret_val;
	va_list args;
	va_start(args, fmt);
	ret_val = vsprintf(tx_buffer, fmt, args);
	if (0 > ret_val)
		return ERR_UNKN;

	printf("%s\n", tx_buffer);
	return ERR_NONE;
}

int32_t comm_init(char *port_name)
{
	printf("Init: %s\n", port_name);

	is_open = true;

	int argc = 0;
	char **argv = NULL;

	wm = init_gui(argc, argv);
	gtk_widget_set_visible(GTK_WIDGET(wm->com_port_frame), false); // hide unused controls
	gtk_widget_set_visible(GTK_WIDGET(wm->com_port_label), false);
	gtk_widget_set_visible(GTK_WIDGET(wm->refresh_button), false);
	gtk_widget_set_visible(GTK_WIDGET(wm->get_button), false);
	gtk_widget_set_visible(GTK_WIDGET(wm->set_button), false);
	gtk_widget_set_visible(GTK_WIDGET(wm->jump_to_bootload_button), false);
	gtk_widget_set_visible(GTK_WIDGET(wm->clear_lcd_button), false);


	g_timeout_add(GRAPHIC_UPDATE_DELAY, (GSourceFunc) timer_handler, (gpointer) wm);
	timer_handler(wm);

	init_tick_count();
	set_comm_trace(true);

	pthread_create(&gtk_thread, NULL, gtk_launch_function, NULL);
	//	pthread_create(&draw_thread, NULL, gtk_draw_function, NULL);

	return 1;
}

int32_t comm_close()
{
	is_open = false;

	pthread_join(gtk_thread, NULL);
	//	pthread_join(draw_thread, NULL);
	return ERR_NONE;
}


int32_t write_motor_levels(int32_t channel1, int32_t channel2)
{
	motor_level[0] = channel1;
	motor_level[1] = channel2;
	motor_level_changed = true;
	return comm_query(params, "SML %d %d", channel1, channel2);
}

int32_t read_motor_levels(int32_t *channel1, int32_t *channel2)
{
	*channel1 = motor_level[0];
	*channel2 = motor_level[1];
	return comm_query(params, "GML");
}

int32_t read_sensor_values(int32_t *channel1, int32_t *channel2)
{
	/// \todo read value from control
	return comm_query(params, "GSV");;
}

int32_t set_motor_timeout(int32_t timeout)
{
	motor_timeout = timeout;
	motor_timeout_changed = true;
	return comm_query(params, "SMT %d", timeout);;
}

int32_t read_motor_timeout(int32_t *timeout)
{
	*timeout = motor_timeout;
	return comm_query(params, "GMT");
}

bool get_comm_trace(void)
{
	return comm_trace;
}

int32_t set_ir_led(bool on)
{
	ir_led_value = on;
	ir_led_changed = true;
	if (on)
		return comm_query(params, "SIL ON");
	else
		return comm_query(params, "SIL OFF");
}

int32_t read_ir_led(bool *on)
{
	*on = ir_led_value;
	return comm_query(params, "GIL");
}

int32_t write_status_led(StatusLedFlashState_t led_state, int32_t flash_rate)
{
	status_led_flash_state = led_state;
	status_led_flash_rate = flash_rate;
	status_led_changed = true;
	//	update_led_control(wm->status_led_control, led_state, flash_rate);
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

int32_t read_status_led(StatusLedFlashState_t *led_state, int32_t *flash_rate)
{
	*led_state = status_led_flash_state;
	*flash_rate = status_led_flash_rate;
	return comm_query(params, "GSL");
}

int32_t read_current_time(uint32_t *time)
{
	*time = get_tick_count();
	return comm_query(params, "TIM");
}

int32_t read_last_error(int32_t *error_id, int32_t *timestamp)
{
	return comm_query(params, "GLE");
}

int32_t read_program_info(char *pgm_info)
{
	pgm_info = "Controller Board PC Sim";
	return comm_query(params, "PGM");
}

int32_t send_password(char *password)
{
	return comm_query(params, "ICB %s", password);
}

int32_t send_jump_to_boot(void)
{
	return comm_query(params, "SDN");
}

int32_t read_pushbuttons(bool *pressed)
{
	get_pushbuttons_from_control(wm->pushbutton, pressed);

	return comm_query(params, "GPB");
}

int32_t clear_lcd(void)
{
	*lcd_line_text[0] = '\0';
	*lcd_line_text[1] = '\0';
	lcd_changed = true;
	return comm_query(params, "CLD");
}

int32_t set_lcd(int32_t line, char *fmt, ...)
{
	va_list args;
	va_start(args,fmt);
	vsprintf(lcd_line_text[line], fmt, args);
	lcd_changed = true;

	return comm_query(params, "SLD %d \"%s\"", line, lcd_line_text[line]);
}

int32_t lcd_putchars(int32_t line, int32_t ch, char *fmt, ...)
{
	va_list args;
	va_start(args,fmt);
	vsprintf(&lcd_line_text[line][ch], fmt, args);
	lcd_changed = true;

	return comm_query(params, "PLC %d \"%s\"", line, lcd_line_text[line]);
}

void *gtk_draw_function(void *ptr)
{
	if (motor_level_changed)
	{
		motor_level_changed = false;
		draw_motor_control(wm->left_motor_slide, wm->left_motor_control,
				motor_level[0]);
		draw_motor_control(wm->right_motor_slide, wm->right_motor_control,
				motor_level[1]);
	}
	if (motor_timeout_changed)
	{
		motor_timeout_changed = false;
		draw_motor_timeout(wm->motor_timeout_control, motor_timeout);
	}
	if (ir_led_changed)
	{
		ir_led_changed = false;
		draw_ir_led(wm->ir_led_control, ir_led_value);
	}

	draw_current_time(wm->timestamp_control, get_tick_count());
	if (lcd_changed)
	{
		lcd_changed = false;
		int32_t i;
		strcpy(lcd_text,"");
		for (i = 0; i < LCD_TEXT_LINES; i++)
		{
			strcat(lcd_text, lcd_line_text[i]);
			strcat(lcd_text, "\n");
		}
		draw_lcd(wm->lcd_text, lcd_text);
	}

	if (status_led_changed)
	{
		status_led_changed = false;
		update_led_control(wm->status_led_control, status_led_flash_state,
				status_led_flash_rate);
	}
	return NULL;
}

int32_t shutdown()
{
	return comm_query(params, "SDN");
}
