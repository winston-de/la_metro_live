#include <stdio.h>
#include "typedefs.h"

#ifndef LINE_MAPPING_H
#define LINE_MAPPING_H

#define A_LED_VAL 0, 0, 16
#define B_LED_VAL 16, 0, 0
#define C_LED_VAL 0, 16, 0
#define D_LED_VAL 30, 0, 8
#define E_LED_VAL 16, 15, 0
#define K_LED_VAL 16, 7, 8

// #define A_LED_VAL 0, 0, 255
// #define B_LED_VAL 255, 0, 0
// #define C_LED_VAL 0, 255, 0
// #define D_LED_VAL 255, 0, 63
// #define E_LED_VAL 255, 240, 0
// #define K_LED_VAL 255, 115, 125

#define AE_STRIP_ID 0
#define BD_STRIP_ID 1
#define CK_STRIP_ID 2

int get_all_lines(MLine **lines);
int get_all_strips(LEDStrip **strips);
LEDStrip get_lgnd_strip();

#endif