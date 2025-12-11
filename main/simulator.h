#include <stdio.h>
#include "led_strip.h"
#include "line_mapping.h"

#ifndef SIMULATOR_H
#define SIMULATOR_H

struct Train {
    bool dir;
    uint8_t station_index;
    uint8_t timer_val;
};

struct LineSim {
    led_strip_handle_t _strip;
    struct Train *_trains;
    uint8_t _num_trains;
    uint8_t _num_stations;
    MLine _line;
};

struct LineSim initialize_line(led_strip_handle_t led_strip, MLine line, uint8_t num_trains, uint8_t start_offset);
void simulate_update(struct LineSim *ls);

void clear_all_stations(struct LineSim *ls);

#endif