#ifndef TRAINMANAGER
#define TRAINMANAGER
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

void parse_train_data(uint8_t *buffer, size_t len);

void init_train_manager(void);
void clear_all_leds(void);

void clear_legend(void);
void draw_legend(void);
void show_error_status(void);
void change_brightness(int8_t diff);
void set_state(bool enabled);
void refresh_all_leds(void);
void run_test(void);
#endif
