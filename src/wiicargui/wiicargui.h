/*
 * wiicargui.h
 *
 *  Created on: Dec 19, 2010
 *      Author: desertfx5
 */

#ifndef WIICARGUI_H_
#define WIICARGUI_H_

#include "hardware.h"
#include "error_message.h"

typedef struct window_ref_t
{
	GtkWidget *window;
	GtkEntry *timestamp_control;
	GtkTextView *program_info_control;
	GtkRange* left_motor_slide;
	GtkRange* right_motor_slide;
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
void get_pushbuttons_from_control(GtkCheckButton **button_checkbox, bool *pressed);
void draw_lcd(GtkTextView *text_view, char *lcd_text);
int32_t get_motor_value_from_control(GtkRange *slider);
void draw_program_info(GtkTextView *text_view, char *pgm_info_string);
void draw_sensor_values(GtkTextView *text_view, int32_t *sensor_levels);
bool get_ir_led_from_control(GtkCheckButton *button);
char *get_lcd_text_from_control(GtkTextView *text_view, int32_t line);
int32_t get_motor_timeout_from_control(GtkSpinButton *motor_timeout_control);
void draw_error_info(GtkEntry *error_code_control, GtkEntry *timestamp_control,
		ErrorID_t error_id, uint32_t error_timestamp);
void draw_pushbuttons(GtkCheckButton **check_box, bool *pressed);


window_ref_t *init_gui(int argc, char *argv[]);

#endif /* PCSIM_H_ */
