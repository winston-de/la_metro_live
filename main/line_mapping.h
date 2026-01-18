#include <stdio.h>
#include "station.h"

#ifndef LINE_MAPPING_H
#define LINE_MAPPING_H

#define A_LED_VAL 0, 0, 17
#define B_LED_VAL 17, 0, 0
#define C_LED_VAL 0, 17, 0
#define D_LED_VAL 34, 0, 4
#define E_LED_VAL 17, 16, 0
#define K_LED_VAL 17, 7, 8

#define AE_STRIP_ID 1
#define BD_STRIP_ID 2
#define CK_STRIP_ID 3

int get_all_lines(MLine **lines);
int get_all_strips(LEDStrip **strips);

#endif