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
#include <stdlib.h>

#include "wiicargui.h"
#include "timestamp.h"
#include "error_message.h"
#include "hardware.h"
#include "wiicar.h"

struct window_ref_t wm;

GtkWidget* error_dialog;

void on_error_dialog_close_button_click(GtkObject *object, gpointer wn)
{
	gtk_widget_destroy(error_dialog);
}

void error_handler(GtkWidget *parent, int32_t error_code)
{
	if (ERR_NONE != error_code)
	{
		char error_string[64];
		format_error_string(error_code, error_string);

		error_dialog = gtk_message_dialog_new(NULL,
				GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
				GTK_MESSAGE_ERROR, GTK_BUTTONS_CLOSE, "Error %d: %s\nat %u ms",
				error_code, error_string, get_tick_count());
		g_signal_connect(error_dialog, "response",
				G_CALLBACK(on_error_dialog_close_button_click), error_dialog);

		gtk_widget_show(GTK_WIDGET(error_dialog));

	}
}

void on_window_destroy(GtkObject *object, gpointer user_data)
{
	g_print("Window Closed\n");
	gtk_main_quit();
	exit(1);
}

void update_led_control(GtkComboBox *combobox, StatusLedFlashState_t led_state,
		int32_t flash_rate)
{
	gdk_threads_enter();
	gtk_combo_box_remove_text(combobox, 2);

	switch (led_state)
	{
	case STATUS_LED_OFF:
		gtk_combo_box_set_active(combobox, 0);
		break;
	case STATUS_LED_ON:
		gtk_combo_box_set_active(combobox, 1);
		break;
	case STATUS_LED_FLASH:
	{
		char flash_rate_string[32] = "";
		sprintf(flash_rate_string, "%d", flash_rate);
		gtk_combo_box_append_text(combobox, flash_rate_string);
		gtk_combo_box_set_active(combobox, 2);
	}
		break;
	default:
		break;
	}
	gdk_threads_leave();
}

void init_led_combo_box(GtkComboBox *combobox)
{
	gdk_threads_enter();
	GtkListStore *store;

	/* Create a new GtkListStore with one string column... */
	store = gtk_list_store_new(1, G_TYPE_STRING);
	/* ...and set it as the combo box's model */
	gtk_combo_box_set_model(GTK_COMBO_BOX(combobox), GTK_TREE_MODEL(store));
	/* Now we can set the text column to the one we just made */
	gtk_combo_box_entry_set_text_column(GTK_COMBO_BOX_ENTRY(combobox), 0);

	gtk_combo_box_append_text(combobox, "OFF");
	gtk_combo_box_append_text(combobox, "ON");
	gtk_combo_box_set_active(combobox, 0);
	gdk_threads_leave();
}

void draw_program_info(GtkTextView *text_view, char *pgm_info_string)
{
	gdk_threads_enter();
	GtkTextBuffer *buffer = gtk_text_view_get_buffer(text_view);
	gtk_text_buffer_set_text(buffer, pgm_info_string, strlen(pgm_info_string));
	gtk_text_view_set_buffer(text_view, buffer);
	gdk_threads_leave();
}

void draw_sensor_values(GtkTextView *text_view, int32_t *sensor_levels)
{
	gdk_threads_enter();
	char sensor_channel[64] = "";
	sprintf(sensor_channel, "%d\n%d\n", sensor_levels[SENSOR_FWD],
			sensor_levels[SENSOR_REV]);
	GtkTextBuffer *buffer = gtk_text_view_get_buffer(text_view);
	gtk_text_buffer_set_text(buffer, sensor_channel, strlen(sensor_channel));
	gtk_text_view_set_buffer(text_view, buffer);
	gdk_threads_leave();
}

void draw_error_info(GtkEntry *error_code_control, GtkEntry *timestamp_control,
		ErrorID_t error_id, uint32_t error_timestamp)
{
	gdk_threads_enter();
	char error_id_string[32] = "", error_timestamp_string[32] = "";
	format_error_string(error_id, error_id_string);
	gtk_entry_set_text(error_code_control, error_id_string);
	sprintf(error_timestamp_string, "%d", error_timestamp);
	gtk_entry_set_text(timestamp_control, error_timestamp_string);
	gdk_threads_leave();
}

void draw_pushbuttons(GtkCheckButton **check_box, bool *pressed)
{
	gdk_threads_enter();
	int32_t i;
	for (i = 0; i < NUMBER_OF_PUSHBUTTONS; i++)
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check_box[i]),
				pressed[i]);
	gdk_threads_leave();
}

/// TODO need handling for error led here (not implemented on control board)

int32_t get_motor_value_from_control(GtkRange *slider)
{
	gdk_threads_enter();
	int32_t value = gtk_range_get_value(slider);
	gdk_threads_leave();
	return value;
}

bool get_ir_led_from_control(GtkCheckButton *button)
{
	gdk_threads_enter();
	bool on = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(button));
	gdk_threads_leave();
	return on;
}

int32_t get_motor_timeout_from_control(GtkSpinButton *motor_timeout_control)
{
	gdk_threads_enter();
	int32_t val = gtk_spin_button_get_value(motor_timeout_control);
	gdk_threads_leave();
	return val;
}

void on_left_motor_slide_value_changed(GtkObject *object, gpointer user_data)
{
	gdk_threads_enter();
	gdouble motor_value = gtk_range_get_value(GTK_RANGE(wm.left_motor_slide));

	gtk_spin_button_set_value(wm.left_motor_control, motor_value);
	gdk_threads_leave();
}

void on_left_motor_value_value_changed(GtkObject *object, gpointer user_data)
{
	gdk_threads_enter();
	gtk_range_set_value(wm.left_motor_slide,
			gtk_spin_button_get_value(wm.left_motor_control));
	gdk_threads_leave();
}

void on_right_motor_slide_value_changed(GtkObject *object, gpointer user_data)
{
	gdk_threads_enter();
	gdouble motor_value = gtk_range_get_value(GTK_RANGE(wm.right_motor_slide));

	gtk_spin_button_set_value(wm.right_motor_control, motor_value);
	gdk_threads_leave();
}

void on_right_motor_value_value_changed(GtkObject *object, gpointer user_data)
{
	gdk_threads_enter();
	gtk_range_set_value(wm.right_motor_slide,
			gtk_spin_button_get_value(wm.right_motor_control));
	gdk_threads_leave();
}

void on_quit_menu_item_activate(GtkObject *object, gpointer user_data)
{
	gtk_widget_destroy(wm.window);
}

void on_about_menu_item_activate(GtkObject *object, gpointer user_data)
{
	static const gchar copyright[] = "Copyright \xc2\xa9 2010 David Anderson";

	static const char comments[] =
			"A demonstration/test application for the controller board built on GTK/Glade";

	gtk_show_about_dialog(GTK_WINDOW (wm.window), "comments", comments,
			"copyright", copyright, "version", "0.1", "website",
			"http://www.google.com", "program-name",
			"Controller Board Test GUI", "logo-icon-name", GTK_STOCK_EDIT,
			NULL);
}

void draw_motor_control(GtkRange *slider, GtkSpinButton *numeric_control,
		int32_t channel)
{
	gdk_threads_enter();
	gtk_range_set_value(slider, channel);
	gtk_spin_button_set_value(numeric_control, channel);
	gdk_threads_leave();
}

void draw_motor_timeout(GtkSpinButton *timeout_control, int32_t timeout)
{
	gdk_threads_enter();
	gtk_spin_button_set_value(timeout_control, timeout);
	gdk_threads_leave();
}

void draw_ir_led(GtkCheckButton *button, bool on)
{
	gdk_threads_enter();
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(button), on);
	gdk_threads_leave();
}

void get_led_state_from_control(GtkComboBox *combobox,
		StatusLedFlashState_t *flash_state, int32_t *flash_rate)
{
	gdk_threads_enter();
	const char *state_string = gtk_combo_box_get_active_text(combobox);

	if (!strcmp(state_string, "ON"))
	{
		*flash_state = STATUS_LED_ON;
		*flash_rate = 0;
	}
	else if (!strcmp(state_string, "OFF"))
	{
		*flash_state = STATUS_LED_OFF;
		*flash_rate = 0;
	}
	else
	{
		*flash_state = STATUS_LED_FLASH;
		sscanf(state_string, "%d", flash_rate);
	}
	gdk_threads_leave();

}

void draw_current_time(GtkEntry *timestamp_control, uint32_t time)
{
	gdk_threads_enter();
	char timestamp_string[64];
	sprintf(timestamp_string, "%d", time);
	gtk_entry_set_text(timestamp_control, timestamp_string);
	gdk_threads_leave();
}

void get_pushbuttons_from_control(GtkCheckButton **button_checkbox,
		bool *pressed)
{
	uint32_t i;
	gdk_threads_enter();
	for (i = 0; i < NUMBER_OF_PUSHBUTTONS; i++)
		pressed[i] = gtk_toggle_button_get_active(
				GTK_TOGGLE_BUTTON(button_checkbox[i]));
	gdk_threads_leave();
}

void draw_lcd(GtkTextView *text_view, char *lcd_text)
{
	gdk_threads_enter();
	strcat(lcd_text, "\n");
	GtkTextBuffer *buffer = gtk_text_view_get_buffer(text_view);
	gtk_text_buffer_set_text(buffer, lcd_text, strlen(lcd_text));
	gdk_threads_leave();
}

char *get_lcd_text_from_control(GtkTextView *text_view, int32_t line)
{
	gdk_threads_enter();
	GtkTextBuffer *buffer = gtk_text_view_get_buffer(text_view);
	GtkTextIter line_start, line_end;
	gtk_text_buffer_get_iter_at_line(buffer, &line_start, line);
	line_end = line_start;
	gtk_text_iter_forward_to_line_end(&line_end);
	char *lcd_text = gtk_text_buffer_get_text(buffer, &line_start, &line_end,
			FALSE);
	gdk_threads_leave();

	return lcd_text;
}

window_ref_t *init_gui(int argc, char *argv[])
{
	GtkBuilder *builder;

	gtk_init(&argc, &argv);

	builder = gtk_builder_new();

	gtk_builder_add_from_file(builder, "cboard_test_gui.ui", NULL);

	wm.window = GTK_WIDGET(gtk_builder_get_object(builder, "window"));
	wm.timestamp_control =
			GTK_ENTRY(gtk_builder_get_object(builder, "timestamp"));
	wm.program_info_control = GTK_TEXT_VIEW(gtk_builder_get_object(builder,
					"program_info"));
	wm.left_motor_slide = GTK_RANGE(gtk_builder_get_object(builder,
					"left_motor_slide"));
	wm.left_motor_control = GTK_SPIN_BUTTON(gtk_builder_get_object(builder,
					"left_motor_value"));

	wm.left_motor_min_label = GTK_LABEL(gtk_builder_get_object(builder, "left_motor_min_label"));
	wm.left_motor_null_label = GTK_LABEL(gtk_builder_get_object(builder, "left_motor_null_label"));
	wm.left_motor_max_label = GTK_LABEL(gtk_builder_get_object(builder, "left_motor_max_label"));

	wm.right_motor_min_label = GTK_LABEL(gtk_builder_get_object(builder, "right_motor_min_label"));
	wm.right_motor_null_label = GTK_LABEL(gtk_builder_get_object(builder, "right_motor_null_label"));
	wm.right_motor_max_label = GTK_LABEL(gtk_builder_get_object(builder, "right_motor_max_label"));

	wm.right_motor_slide = GTK_RANGE(gtk_builder_get_object(builder,
					"right_motor_slide"));
	wm.right_motor_control = GTK_SPIN_BUTTON(gtk_builder_get_object(builder,
					"right_motor_value"));
	wm.sensor_value_control = GTK_TEXT_VIEW(gtk_builder_get_object(builder,
					"sensor_values"));
	wm.status_led_control = GTK_COMBO_BOX(gtk_builder_get_object(builder,
					"status_led"));
	wm.error_led_control = GTK_COMBO_BOX(gtk_builder_get_object(builder,
					"error_led"));
	wm.motor_timeout_control = GTK_SPIN_BUTTON(gtk_builder_get_object(builder,
					"motor_timeout"));
	wm.ir_led_control = GTK_CHECK_BUTTON(gtk_builder_get_object(builder,
					"ir_led"));
	wm.error_code = GTK_ENTRY(gtk_builder_get_object(builder,
					"last_error_code"));
	wm.error_timestamp = GTK_ENTRY(gtk_builder_get_object(builder,
					"last_error_time"));
	wm.pushbutton[JOY_CENTER] = GTK_CHECK_BUTTON(gtk_builder_get_object(builder,
					"joy_center_button"));
	wm.pushbutton[JOY_UP] = GTK_CHECK_BUTTON(gtk_builder_get_object(builder,
					"joy_up_button"));
	wm.pushbutton[JOY_DOWN] = GTK_CHECK_BUTTON(gtk_builder_get_object(builder,
					"joy_down_button"));
	wm.pushbutton[JOY_LEFT] = GTK_CHECK_BUTTON(gtk_builder_get_object(builder,
					"joy_left_button"));
	wm.pushbutton[JOY_RIGHT] = GTK_CHECK_BUTTON(gtk_builder_get_object(builder,
					"joy_right_button"));

#if !PUSHBUTTONS_SUPPORTED
	for (uint8_t i = 0; i < NUMBER_OF_PUSHBUTTONS; i++)
		gtk_widget_set_sensitive(GTK_WIDGET(wm.pushbutton[i]), false);
#endif

#if !LCD_SUPPORTED
//	gtk_widget_set_sensitive(GTK_WIDGET(wm.lcd_text), false);
//	gtk_widget_set_sensitive(GTK_WIDGET(wm.clear_lcd_button), false);
#endif

	wm.lcd_text = GTK_TEXT_VIEW(gtk_builder_get_object(builder,
					"lcd_text"));

#if !LCD_SUPPORTED

#endif

	wm.com_port = GTK_COMBO_BOX(gtk_builder_get_object(builder,"com_port"));
	wm.com_port_list =
			GTK_LIST_STORE(gtk_builder_get_object(builder, "com_port_list"));
	wm.com_port_frame =
			GTK_WIDGET(gtk_builder_get_object(builder, "com_port_frame"));
	wm.com_port_label =
			GTK_WIDGET(gtk_builder_get_object(builder, "com_port_label"));
	wm.refresh_button =
			GTK_BUTTON(gtk_builder_get_object(builder, "refresh_ports"));
	wm.get_button = GTK_BUTTON(gtk_builder_get_object(builder, "get_button"));
	wm.set_button = GTK_BUTTON(gtk_builder_get_object(builder, "set_button"));
	wm.jump_to_bootload_button =
			GTK_BUTTON(gtk_builder_get_object(builder, "jump_to_boot"));
	wm.clear_lcd_button =
			GTK_BUTTON(gtk_builder_get_object(builder, "clear_button"));

	gtk_range_set_range(GTK_RANGE(wm.left_motor_slide), MAX_REVERSE_SPEED,
			MAX_FORWARD_SPEED);
	gtk_range_set_range(GTK_RANGE(wm.right_motor_slide), MIN_DIRECTION_MOTOR,
			MAX_DIRECTION_MOTOR);

	gtk_spin_button_set_range(wm.left_motor_control, MAX_REVERSE_SPEED,
			MAX_FORWARD_SPEED);
	gtk_spin_button_set_range(wm.right_motor_control, MIN_DIRECTION_MOTOR,
			MAX_DIRECTION_MOTOR);

	char label_string[32] = "";

	sprintf(label_string, "%d",MIN_DIRECTION_MOTOR);
	gtk_label_set_text(GTK_LABEL(wm.right_motor_min_label), label_string);

	sprintf(label_string, "%d",DIRECTION_NULL_VALUE);
	gtk_label_set_text(GTK_LABEL(wm.right_motor_null_label), label_string);

	sprintf(label_string, "%d",MAX_DIRECTION_MOTOR);
	gtk_label_set_text(GTK_LABEL(wm.right_motor_max_label), label_string);

	sprintf(label_string, "%d",MAX_REVERSE_SPEED);
	gtk_label_set_text(GTK_LABEL(wm.left_motor_min_label), label_string);

	sprintf(label_string, "%d",SPEED_NULL_VALUE);
	gtk_label_set_text(GTK_LABEL(wm.left_motor_null_label), label_string);

	sprintf(label_string, "%d",MAX_FORWARD_SPEED);
	gtk_label_set_text(GTK_LABEL(wm.right_motor_max_label), label_string);

	gtk_spin_button_set_range(wm.motor_timeout_control, -1, 65535);
	gtk_spin_button_set_value(wm.motor_timeout_control, 200);

	init_led_combo_box(wm.status_led_control);
	init_led_combo_box(wm.error_led_control);

	gtk_builder_connect_signals(builder, NULL);

	g_object_unref(G_OBJECT(builder));

	init_tick_count();
	gtk_widget_show(wm.window);

	return &wm;
}

