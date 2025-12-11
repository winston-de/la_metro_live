#include <stdio.h>

#define ARRL(arr) sizeof(arr) / sizeof(arr[0])

#ifndef STATION_H
#define STATION_H

typedef struct {
    uint32_t id;
    uint8_t nodes[2];
} Station;

typedef struct {
    Station* stations;
    char id;
    uint8_t num_stations;
    uint8_t color_r;
    uint8_t color_g;
    uint8_t color_b;
} MLine;


#endif