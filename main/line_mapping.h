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

MLine get_e_line();
MLine get_a_line();
MLine get_b_line();
MLine get_d_line();
MLine get_c_line();
MLine get_k_line();

#endif