#include <stdio.h>
#include <stdbool.h>
#define ARRL(arr) sizeof(arr) / sizeof(arr[0])

#ifndef STATION_H
#define STATION_H

typedef struct {
    uint32_t id;
    uint8_t nodes[2];
} Station;

typedef struct {
    Station* stations;
    uint16_t id;
    uint8_t num_stations;
    uint8_t strip_num;
    uint8_t color_r;
    uint8_t color_g;
    uint8_t color_b;
} MLine;

typedef struct {
    int16_t routeId;
    bool dir;
    int16_t stopId; 

} VehicleData;

typedef struct {
    uint8_t num_leds;
    uint8_t gpio;
} LEDStrip;

#endif