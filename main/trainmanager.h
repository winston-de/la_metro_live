#ifndef TRAINMANAGER
#define TRAINMANAGER
#include <string.h>
#include <stdlib.h>

void parse_train_data(uint8_t *buffer, size_t len);

void init_train_manager(void);
void clear_all_leds(void);

void clear_legend(void);
void draw_legend(void);

#endif
