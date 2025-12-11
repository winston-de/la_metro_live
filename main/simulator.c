#include <stdio.h>
#include "led_strip.h"
#include "simulator.h"
#include "esp_timer.h"
#include "esp_random.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "led_strip.h"
#include "station.h"
#include "esp_log.h"

const uint8_t time_target = 20;

#define indx(t, ls) ls->_line.stations[t->station_index].nodes[t->dir]

struct LineSim initialize_line(led_strip_handle_t led_strip, MLine line, uint8_t num_trains, uint8_t start_offset) {
    struct Train* trains = malloc(sizeof(struct Train) * num_trains);

    int diff = (line.num_stations * 2 - start_offset) / num_trains;
    for(int i = 0; i < num_trains; i++) {
        uint8_t station_index = start_offset + i * diff;
    CHANGE_NODE:
        bool dir = 0;

        if(station_index >= line.num_stations) {
            station_index = line.num_stations - (station_index % line.num_stations) - 1;
            dir = 1;
        }

        // if(station_index > line.num_stations) {
        //     station_index = 0;
        // }

        struct Train t = {
            .station_index = station_index,
            .dir = dir,
            .timer_val = esp_random() % time_target
        };

        trains[i] = t;
        uint8_t node = line.stations[t.station_index].nodes[t.dir];
        if(node == 255) {
            station_index += 1;
            goto CHANGE_NODE;
        }
        ESP_LOGI("simulator", "%c %d %d %d", line.id , node, t.station_index, t.dir);
        ESP_ERROR_CHECK(led_strip_set_pixel(led_strip, node, line.color_r, line.color_g, line.color_b));
    }

    struct LineSim ls = {
        ._strip = led_strip, 
        ._num_trains = num_trains,
        ._line = line,
        ._trains = trains,
        ._num_stations = line.num_stations,
    };

    return ls;
}

// the built in clear API refreshes the strip, so clear LEDs with no refresh
void clear_all_stations(struct LineSim *ls) {
    for(int i = 0; i < ls->_num_trains; i++) {
        ESP_ERROR_CHECK(led_strip_set_pixel(ls->_strip, indx((&ls->_trains[i]), ls), 0, 0, 0));
    }
}

void simulate_train(struct Train *t, struct LineSim *ls) {
    if(t->timer_val >= time_target) {
        t->timer_val = 0;
        int prv_st = t->station_index;
        // uint8_t prv_node = ls->_line.stations[t->station_index].nodes[t->dir];
        // ESP_ERROR_CHECK(led_strip_set_pixel(ls->_strip, prv_node, 0, 0, 0));
NEXT_STATION:
        if(t->dir) {
            if(t->station_index  == 0) {
                t->dir = 0;
                t->station_index += 1;
            } else {
                t->station_index -= 1;
            }
        } else {
            if(t->station_index >= ls->_num_stations -1 ) {
                t->dir = 1;
                t->station_index = ls->_num_stations -1;
            } else {
                t->station_index += 1;
            }   
        }
    } else {
        t->timer_val += 1;
    }

    uint8_t node = indx(t, ls);
    // ESP_LOGI("sim", "BLAH");
    // ESP_LOGI("simulator up", "%c line: cur st: %d prev st: %d dir: %d node: %d", ls->_line.id, t->station_index, prv_st, t->dir, node);

    if(node == 255) {
        goto NEXT_STATION;
    }
    ESP_ERROR_CHECK(led_strip_set_pixel(ls->_strip, node, ls->_line.color_r, ls->_line.color_g, ls->_line.color_b));
}

void simulate_update(struct LineSim *ls) {
    for(int i = 0; i < ls->_num_trains; i++) {
        simulate_train(&(ls->_trains[i]), ls);
    }
}