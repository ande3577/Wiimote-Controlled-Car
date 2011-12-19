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

#include <wiicarutility/timestamp.h>
#include <wiicarutility/error_message.h>
#include <controlboard/hardware.h>
#include <controlboard/control_board.h>
#include <pthread.h>

#include "wiicargui.h"

#define GRAPHIC_UPDATE_DELAY 100

char *com_port_name = "";

typedef struct window_ref_t
{
	GtkWidget *window;
	GtkEntry *timestamp_control;
	GtkTextView *program_info_control;
	GtkRange* left_motor_slide;
	GtkRange* right_motor_slide;
	GtkLabel* left_motor_min_label;
	GtkLabel* left_motor_null_label;
	GtkLabel* left_motor_max_label;
	GtkLabel* right_motor_min_label;
	GtkLabel* right_motor_null_label;
	GtkLabel* right_motor_max_label;
	GtkSpinButton* left_motor_control;
	GtkSpinButton* right_motor_control;
	GtkTextView *sensor_value_control;
	GtkComboBox *status_led_control;
	GtkComboBox *error_led_control;
	GtkSpinButton *motor_timeout_control;
	GtkCheckButton *ir_led_control;
	GtkEntry *error_code;
	GtkEntry *error_timestamp;
	GtkCheckButton *pushbutton[NUMBER_OF_PUSHBUTTONS];
	GtkTextView *lcd_text;
	GtkComboBox *mode;
	GtkListStore *mode_list;
	GtkComboBox *com_port;
	GtkListStore *com_port_list;
	GtkWidget *com_port_frame;
	GtkWidget *com_port_label;
	GtkButton *refresh_button;
	GtkButton *get_button;
	GtkButton *set_button;
	GtkButton *jump_to_bootload_button;
	GtkButton *clear_lcd_button;
} window_ref_t;

struct window_ref_t wm;

pthread_t gtk_thread;
pthread_t draw_thread;

GtkWidget* error_dialog;

void error_handler(GtkWidget *parent, int32_t error_code);
void draw_motor_control(GtkRange *slider, GtkSpinButton *numeric_control,
		int32_t channel);
void draw_motor_timeout(GtkSpinButton *timeout_control, int32_t timeout);
void draw_ir_led(GtkCheckButton *button, bool on);
void update_led_control(GtkComboBox *combobox, StatusLedFlashState_t led_state,
		int32_t flash_rate);
void get_led_state_from_control(GtkComboBox *combobox,
		StatusLedFlashState_t *flash_state, int32_t *flash_rate);
void draw_current_time(GtkEntry *timestamp_control, uint32_t time);
void get_pushbuttons_from_control(GtkCheckButton **button_checkbox,
		bool *pressed);
void draw_lcd(GtkTextView *text_view, const char *lcd_text);
int32_t get_motor_value_from_control(GtkRange *slider);
void draw_program_info(GtkTextView *text_view, const char *pgm_info_string);
void draw_sensor_values(GtkTextView *text_view, const int32_t *sensor_levels);
bool get_ir_led_from_control(GtkCheckButton *button);
char *get_lcd_text_from_control(GtkTextView *text_view, int32_t line);
int32_t get_motor_timeout_from_control(GtkSpinButton *motor_timeout_control);
void draw_error_info(GtkEntry *error_code_control, GtkEntry *timestamp_control,
		ErrorID_t error_id, uint32_t error_timestamp);
void draw_pushbuttons(GtkCheckButton **check_box, bool *pressed);

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

void draw_program_info(GtkTextView *text_view, const char *pgm_info_string)
{
	gdk_threads_enter();
	GtkTextBuffer *buffer = gtk_text_view_get_buffer(text_view);
	gtk_text_buffer_set_text(buffer, pgm_info_string, strlen(pgm_info_string));
	gtk_text_view_set_buffer(text_view, buffer);
	gdk_threads_leave();
}

void draw_sensor_values(GtkTextView *text_view, const int32_t *sensor_levels)
{
	gdk_threads_enter();
	char sensor_channel[64] = "";
	sprintf(sensor_channel, "%d\n%d\n%d\n%d\n%d", sensor_levels[0],
			sensor_levels[1], sensor_levels[2], sensor_levels[4],
			sensor_levels[4]);
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

void update_port_names(GtkComboBox *combobox)
{
#if !_PCSIM
	gdk_threads_enter();
	int32_t fd;
	char port_name[32] = "";

	bool comm_trace = get_comm_trace();
	set_comm_trace(false); // mask printing errors opening port

	gtk_list_store_clear(wm.com_port_list);

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
		error_code = read_current_time();
		if (ERR_NONE == error_code)
			draw_current_time(wm.timestamp_control, *get_current_time());
	}

	if (ERR_NONE == error_code)
	{
		error_code = read_program_info();
		if (ERR_NONE == error_code)
			draw_program_info(wm.program_info_control, get_program_info());
	}

	if (ERR_NONE == error_code)
	{
		error_code = read_motor_levels();

		if (ERR_NONE == error_code)
		{
			draw_motor_control(wm.left_motor_slide, wm.left_motor_control,
					get_motor_levels()[MOTOR_SPEED_CHANNEL]);
			draw_motor_control(wm.right_motor_slide, wm.right_motor_control,
					get_motor_levels()[MOTOR_DIRECTION_CHANNEL]);
		}
	}

	if (ERR_NONE == error_code)
	{
		error_code = read_sensor_values();

		if (ERR_NONE == error_code)
			draw_sensor_values(wm.sensor_value_control, get_sensor_values());
	}

	if (ERR_NONE == error_code)
	{
		StatusLedFlashState_t led_state;
		int32_t flash_rate;

		error_code = read_status_led(&led_state, &flash_rate);

		if (ERR_NONE == error_code)
			update_led_control(wm.status_led_control, led_state, flash_rate);
	}

	if (ERR_NONE == error_code)
	{
		StatusLedFlashState_t led_state;
		int32_t flash_rate;

		error_code = read_status_led(&led_state, &flash_rate);

		if (ERR_NONE == error_code)
			update_led_control(wm.status_led_control, led_state, flash_rate);
	}

	if (ERR_NONE == error_code)
	{
		int32_t motor_timeout;

		error_code = read_motor_timeout(&motor_timeout);
		if (ERR_NONE == error_code)
			draw_motor_timeout(wm.motor_timeout_control, motor_timeout);
	}

	if (ERR_NONE == error_code)
	{
		bool on;
		error_code = read_ir_led(&on);
		if (ERR_NONE == error_code)
			draw_ir_led(wm.ir_led_control, on);
	}

	if (ERR_NONE == error_code)
	{
		int32_t error_id, error_timestamp;
		error_code = read_last_error(&error_id, &error_timestamp);
		if (ERR_NONE == error_code)
			draw_error_info(wm.error_code, wm.error_timestamp, error_id,
					error_timestamp);
	}

	/// TODO need handling for error led here (not implemented on control board)

	comm_close();

	error_handler(wm.window, error_code);
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
				wm.left_motor_slide);
		motor_levels[MOTOR_DIRECTION_CHANNEL] = get_motor_value_from_control(
				wm.right_motor_slide);
		error_code = write_motor_levels(motor_levels[MOTOR_SPEED_CHANNEL],
				motor_levels[MOTOR_DIRECTION_CHANNEL]);
	}

	if (ERR_NONE == error_code)
	{
		error_code = set_ir_led(get_ir_led_from_control(wm.ir_led_control));
	}

	if (ERR_NONE == error_code)
	{
		error_code = set_motor_timeout(
				get_motor_timeout_from_control(wm.motor_timeout_control));
	}

	if (ERR_NONE == error_code)
	{
		StatusLedFlashState_t flash_state;
		int32_t flash_rate;

		get_led_state_from_control(wm.status_led_control, &flash_state,
				&flash_rate);
		write_status_led(flash_state, flash_rate);
	}

	if (ERR_NONE == error_code)
	{
		int32_t i;
		for (i = 0; i < LCD_TEXT_LINES; i++)
		{
			gchar * lcd_line_text = get_lcd_text_from_control(wm.lcd_text, i);
			set_lcd(i, lcd_line_text);
		}

	}

	/// TODO need handling for error led here (not implemented on control board)

	comm_close();

	error_handler(wm.window, error_code);
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

	error_handler(wm.window, error_code);
}

void on_clear_button_clicked(GtkObject *object, gpointer user_data)
{
	int32_t error_code = ERR_NONE;

	if (0 > comm_init(com_port_name))
		error_code = ERR_PORT_INIT;

	if (ERR_NONE == error_code)
		error_code = send_password("WIFIBOT123");

//	if (ERR_NONE == error_code)
//		error_code = clear_lcd();
//
//	if (ERR_NONE == error_code)
//	{
//		draw_lcd(wm.lcd_text, "");
//	}

	comm_close();

	error_handler(wm.window, error_code);
}

void on_refresh_ports_clicked(GtkObject *object, gpointer user_data)
{
	update_port_names(wm.com_port);
}

void on_com_port_changed(GtkObject *object, gpointer user_data)
{
	gdk_threads_enter();
	com_port_name = gtk_combo_box_get_active_text(GTK_COMBO_BOX(object));
	gdk_threads_leave();
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

void draw_lcd(GtkTextView *text_view, const char *lcd_text)
{
	gdk_threads_enter();
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

void *gtk_draw_function(void *ptr)
{
	draw_motor_control(wm.left_motor_slide, wm.left_motor_control,
			get_motor_levels()[0]);
	draw_motor_control(wm.right_motor_slide, wm.right_motor_control,
			get_motor_levels()[1]);

	draw_motor_timeout(wm.motor_timeout_control, *get_motor_timeout());

	draw_ir_led(wm.ir_led_control, *get_ir_led());

	draw_current_time(wm.timestamp_control, get_tick_count());
	int32_t i;
	char lcd_text[256] = "";
	for (i = 0; i < LCD_TEXT_LINES; i++)
	{
		strcat(lcd_text, get_lcd(i));
		strcat(lcd_text, "\n");
	}
	draw_lcd(wm.lcd_text, lcd_text);

	update_led_control(wm.status_led_control, get_status_led()->state,
			get_status_led()->flash_rate);

	update_led_control(wm.error_led_control, get_error_led()->state,
			get_error_led()->flash_rate);

	return NULL;
}

static bool timer_handler(struct window_ref_t *wm)
{
	gtk_draw_function(wm);
	return true;
}

void *gtk_launch_function(void *ptr)
{
	gtk_main();
	return NULL;
}

void init_gui(int argc, char *argv[])
{
	GtkBuilder *builder;

	gtk_init(&argc, &argv);

	builder = gtk_builder_new();

	gtk_builder_add_from_file(builder, "cboard_test_gui.glade", NULL);

	wm.window = GTK_WIDGET(gtk_builder_get_object(builder, "window"));
	wm.timestamp_control =
			GTK_ENTRY(gtk_builder_get_object(builder, "timestamp"));
	wm.program_info_control = GTK_TEXT_VIEW(gtk_builder_get_object(builder,
					"program_info"));
	wm.left_motor_slide = GTK_RANGE(gtk_builder_get_object(builder,
					"left_motor_slide"));
	wm.left_motor_control = GTK_SPIN_BUTTON(gtk_builder_get_object(builder,
					"left_motor_value"));

	wm.left_motor_min_label =
			GTK_LABEL(gtk_builder_get_object(builder, "left_motor_min_label"));
	wm.left_motor_null_label =
			GTK_LABEL(gtk_builder_get_object(builder, "left_motor_null_label"));
	wm.left_motor_max_label =
			GTK_LABEL(gtk_builder_get_object(builder, "left_motor_max_label"));

	wm.right_motor_min_label =
			GTK_LABEL(gtk_builder_get_object(builder, "right_motor_min_label"));
	wm.right_motor_null_label =
			GTK_LABEL(gtk_builder_get_object(builder, "right_motor_null_label"));
	wm.right_motor_max_label =
			GTK_LABEL(gtk_builder_get_object(builder, "right_motor_max_label"));

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

	wm.lcd_text = GTK_TEXT_VIEW(gtk_builder_get_object(builder,
					"lcd_text"));

	wm.com_port = GTK_COMBO_BOX(gtk_builder_get_object(builder,"com_port"));
	wm.com_port_list =
			GTK_LIST_STORE(gtk_builder_get_object(builder, "com_port_list"));
	wm.com_port_frame =
			GTK_WIDGET(gtk_builder_get_object(builder, "com_port_frame"));
	wm.com_port_label =
			GTK_WIDGET(gtk_builder_get_object(builder, "com_port_label"));
	wm.mode = GTK_COMBO_BOX(gtk_builder_get_object(builder,"mode"));
	wm.mode_list = GTK_LIST_STORE(gtk_builder_get_object(builder, "mode_list"));

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

	sprintf(label_string, "%d", MIN_DIRECTION_MOTOR);
	gtk_label_set_text(GTK_LABEL(wm.right_motor_min_label), label_string);

	sprintf(label_string, "%d", DIRECTION_NULL_VALUE);
	gtk_label_set_text(GTK_LABEL(wm.right_motor_null_label), label_string);

	sprintf(label_string, "%d", MAX_DIRECTION_MOTOR);
	gtk_label_set_text(GTK_LABEL(wm.right_motor_max_label), label_string);

	sprintf(label_string, "%d", MAX_REVERSE_SPEED);
	gtk_label_set_text(GTK_LABEL(wm.left_motor_min_label), label_string);

	sprintf(label_string, "%d", SPEED_NULL_VALUE);
	gtk_label_set_text(GTK_LABEL(wm.left_motor_null_label), label_string);

	sprintf(label_string, "%d", MAX_FORWARD_SPEED);
	gtk_label_set_text(GTK_LABEL(wm.right_motor_max_label), label_string);

	gtk_spin_button_set_range(wm.motor_timeout_control, -1, 65535);
	gtk_spin_button_set_value(wm.motor_timeout_control, 200);

	init_led_combo_box(wm.status_led_control);
	init_led_combo_box(wm.error_led_control);

	gtk_builder_connect_signals(builder, NULL);

	g_object_unref(G_OBJECT(builder));

	init_tick_count();
	gtk_widget_show(wm.window);

	g_timeout_add(GRAPHIC_UPDATE_DELAY, (GSourceFunc) timer_handler,
			(gpointer) & wm);
	timer_handler(&wm);
	pthread_create(&gtk_thread, NULL, gtk_launch_function, NULL);

}

void shutdown_gui()
{
	g_print("Window Closed\n");
	pthread_join(gtk_thread, NULL);
	gtk_main_quit();
}

