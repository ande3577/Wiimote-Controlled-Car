/*
 * main.c
 *
 *  Created on: Dec 11, 2010
 *      Author: desertfx5
 */

#include <gtk/gtk.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include "control_board.h"
#include "timestamp.h"
#include "error_message.h"
#include "hardware.h"
#include "wiicargui.h"

char *com_port_name = "";

struct window_ref_t *wm;

void update_port_names(GtkComboBox *combobox)
{
#if !_PCSIM
	gdk_threads_enter();
	int32_t fd;
	char port_name[32] = "";

	bool comm_trace = get_comm_trace();
	set_comm_trace(false); // mask printing errors opening port

	gtk_list_store_clear(wm->com_port_list);

	int32_t i = 0;
	// look for com ports
	for (i = 0; i < 10; i++)
	{
		sprintf(port_name, "/dev/ttyS%d", i);
		fd = comm_init(port_name);
		comm_close();
		if (0 < fd)
		gtk_combo_box_append_text(combobox, port_name);
	}

	// look for usb com ports
	for (i = 0; i < 10; i++)
	{
		sprintf(port_name, "/dev/ttyUSB%d", i);
		fd = comm_init(port_name);
		comm_close();
		if (0 < fd)
		gtk_combo_box_append_text(combobox, port_name);
	}

	/// TODO this currently resets com port name. Eventually this should retain value
	gtk_combo_box_set_active(combobox, 0);
	com_port_name = gtk_combo_box_get_active_text(combobox);
	set_comm_trace(comm_trace); // restore comm trace
	gdk_threads_leave();
#else

#endif

}

void on_get_button_clicked(GtkObject *object, gpointer wn)
{
	g_print("Get Button Clicked\n");

	int32_t error_code = ERR_NONE;

	if (0 > comm_init(com_port_name))
		error_code = ERR_PORT_INIT;

	if (ERR_NONE == error_code)
		error_code = send_password("WIFIBOT123");

	if (ERR_NONE == error_code)
	{
		uint32_t timestamp;
		error_code = read_current_time(&timestamp);
		if (ERR_NONE == error_code)
			draw_current_time(wm->timestamp_control, timestamp);
	}

	if (ERR_NONE == error_code)
	{
		char program_info[64] = "";
		error_code = read_program_info(program_info);
		if (ERR_NONE == error_code)
			draw_program_info(wm->program_info_control, program_info);
	}

	if (ERR_NONE == error_code)
	{
		int32_t motor_levels[NUMBER_OF_MOTOR_CHANNELS];
		error_code = read_motor_levels(&motor_levels[MOTOR_SPEED_CHANNEL],
				&motor_levels[MOTOR_DIRECTION_CHANNEL]);

		if (ERR_NONE == error_code)
		{
			draw_motor_control(wm->left_motor_slide, wm->left_motor_control,
					motor_levels[MOTOR_SPEED_CHANNEL]);
			draw_motor_control(wm->right_motor_slide, wm->right_motor_control,
					motor_levels[MOTOR_DIRECTION_CHANNEL]);
		}
	}

	if (ERR_NONE == error_code)
	{
		int32_t sensor_levels[NUMBER_OF_SENSOR_CHANNELS];
		error_code = read_sensor_values(&sensor_levels[SENSOR_FWD],
				&sensor_levels[SENSOR_REV]);

		if (ERR_NONE == error_code)
			draw_sensor_values(wm->sensor_value_control, sensor_levels);
	}

	if (ERR_NONE == error_code)
	{
		StatusLedFlashState_t led_state;
		int32_t flash_rate;

		error_code = read_status_led(&led_state, &flash_rate);

		if (ERR_NONE == error_code)
			update_led_control(wm->status_led_control, led_state, flash_rate);
	}

	if (ERR_NONE == error_code)
	{
		int32_t motor_timeout;

		error_code = read_motor_timeout(&motor_timeout);
		if (ERR_NONE == error_code)
			draw_motor_timeout(wm->motor_timeout_control, motor_timeout);
	}

	if (ERR_NONE == error_code)
	{
		bool on;
		error_code = read_ir_led(&on);
		if (ERR_NONE == error_code)
			draw_ir_led(wm->ir_led_control, on);
	}

	if (ERR_NONE == error_code)
	{
		int32_t error_id, error_timestamp;
		error_code = read_last_error(&error_id, &error_timestamp);
		if (ERR_NONE == error_code)
			draw_error_info(wm->error_code, wm->error_timestamp, error_id,
					error_timestamp);
	}

	if (ERR_NONE == error_code)
	{
		bool pressed[NUMBER_OF_PUSHBUTTONS];
		error_code = read_pushbuttons(pressed);
		if (ERR_NONE == error_code)
			draw_pushbuttons(wm->pushbutton, pressed);
	}

	comm_close();

	error_handler(wm->window, error_code);
}

void on_set_button_clicked(GtkObject *object, gpointer wn)
{
	g_print("Set Button Clicked\n");

	int32_t error_code = ERR_NONE;

	if (0 > comm_init(com_port_name))
		error_code = ERR_PORT_INIT;

	if (ERR_NONE == error_code)
		error_code = send_password("WIFIBOT123");

	if (ERR_NONE == error_code)
	{
		int32_t motor_levels[NUMBER_OF_MOTOR_CHANNELS];
		motor_levels[MOTOR_SPEED_CHANNEL] = get_motor_value_from_control(
				wm->left_motor_slide);
		motor_levels[MOTOR_DIRECTION_CHANNEL] = get_motor_value_from_control(
				wm->right_motor_slide);
		error_code = write_motor_levels(motor_levels[MOTOR_SPEED_CHANNEL],
				motor_levels[MOTOR_DIRECTION_CHANNEL]);
	}

	if (ERR_NONE == error_code)
	{
		error_code = set_ir_led(get_ir_led_from_control(wm->ir_led_control));
	}

	if (ERR_NONE == error_code)
	{
		error_code = set_motor_timeout(get_motor_timeout_from_control(
				wm->motor_timeout_control));
	}

	if (ERR_NONE == error_code)
	{
		StatusLedFlashState_t flash_state;
		int32_t flash_rate;

		get_led_state_from_control(wm->status_led_control, &flash_state,
				&flash_rate);
		write_status_led(flash_state, flash_rate);
	}

	if (ERR_NONE == error_code)
	{
		int32_t i;
		for (i = 0; i < LCD_TEXT_LINES; i++)
		{
			gchar * lcd_line_text = get_lcd_text_from_control(wm->lcd_text, i);
			set_lcd(i, lcd_line_text);
		}

	}

	comm_close();

	error_handler(wm->window, error_code);
}

void on_jump_to_boot_clicked(GtkObject *object, gpointer user_data)
{
	int32_t error_code = ERR_NONE;

	if (0 > comm_init(com_port_name))
		error_code = ERR_PORT_INIT;

	if (ERR_NONE == error_code)
		error_code = send_password("WIFIBOT123");

	if (ERR_NONE == error_code)
		error_code = send_jump_to_boot();

	comm_close();

	error_handler(wm->window, error_code);
}

void on_clear_button_clicked(GtkObject *object, gpointer user_data)
{
	int32_t error_code = ERR_NONE;

	if (0 > comm_init(com_port_name))
		error_code = ERR_PORT_INIT;

	if (ERR_NONE == error_code)
		error_code = send_password("WIFIBOT123");

	if (ERR_NONE == error_code)
		error_code = clear_lcd();

	if (ERR_NONE == error_code)
	{
		draw_lcd(wm->lcd_text, "");
	}

	comm_close();

	error_handler(wm->window, error_code);
}

void on_refresh_ports_clicked(GtkObject *object, gpointer user_data)
{
	update_port_names(wm->com_port);
}

void on_com_port_changed(GtkObject *object, gpointer user_data)
{
	gdk_threads_enter();
	com_port_name = gtk_combo_box_get_active_text(GTK_COMBO_BOX(object));
	gdk_threads_leave();
}

int main(int argc, char *argv[])
{
	wm = init_gui(argc, argv);

	update_port_names(wm->com_port);

	set_comm_trace(true);
	gtk_main();

	return 0;
}

