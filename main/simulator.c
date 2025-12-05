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

const uint8_t time_target = 10;


struct LineSim initialize_line(led_strip_handle_t led_strip, MLine line, uint8_t num_trains) {
    struct Train* trains = malloc(sizeof(struct Train) * num_trains);

    uint8_t num_stations = ARRL((Station[])(*line.stations));

    int diff = num_stations * 2 / num_trains;
    for(int i = 0; i < num_trains; i++) {
        bool dir = 0;
        uint8_t station_index = i * diff;
        if(station_index >= num_stations) {
            station_index = (station_index - num_stations) % num_stations;
            dir = 1;
        }

        if(station_index > num_stations) {
            station_index = 0;
        }

        struct Train t = {
            .station_index = station_index,
            .dir = dir,
            .timer_val = esp_random() % time_target
        };

        trains[i] = t;
        uint8_t node = line.stations[t.station_index].nodes[t.dir];
        ESP_ERROR_CHECK(led_strip_set_pixel(led_strip, node, line.color_r, line.color_g, line.color_b));
        ESP_LOGI("simulator", "%d %d", t.station_index, t.dir);
    }

    struct LineSim ls = {
        ._strip = led_strip, 
        ._num_trains = num_trains,
        ._line = line,
        ._trains = trains,
        ._num_stations = num_stations,
    };

    return ls;
}

void simulate_train(struct Train *t, struct LineSim *ls) {
    if(t->timer_val >= time_target) {
        t->timer_val = 0;
        int prv_st = t->station_index;
        uint8_t prv_node = ls->_line.stations[t->station_index].nodes[t->dir];
        ESP_ERROR_CHECK(led_strip_set_pixel(ls->_strip, prv_node, 0, 0, 0));
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
        uint8_t node = ls->_line.stations[t->station_index].nodes[t->dir];
        ESP_LOGI("simulator up", "%c line: cur st: %d prev st: %d dir: %d node: %d", ls->_line.id, t->station_index, prv_st, t->dir, node);

        if(node == 255) {
            goto NEXT_STATION;
        }
        ESP_ERROR_CHECK(led_strip_set_pixel(ls->_strip, node, ls->_line.color_r, ls->_line.color_g, ls->_line.color_b));
    } else {
        t->timer_val += 1;
    }
}

void simulate_update(struct LineSim *ls) {
    for(int i = 0; i < ls->_num_trains; i++) {
        simulate_train(&(ls->_trains[i]), ls);
    }
}