#include "trainmanager.h"
#include "gtfs-realtime.pb-c.h"
#include "station.h"
#include "line_mapping.h"
#include <string.h>
#include <stdlib.h>
#include "led_strip.h"
#include "led_helper.h"
#include "esp_log.h"

static char *TAG = "TrainManager";
MLine *lines;
int num_lines = 0;
led_strip_handle_t *led_strips;
LEDStrip *strips_data;
int num_strips = 0;

void init_train_manager(void)
{
    num_lines = get_all_lines(&lines);
    printf("initialized lines %d\n", num_lines);
    num_strips = get_all_strips(&strips_data);
    printf("obtained led strips_data %d \n", num_strips);

    led_strips = malloc(sizeof(led_strip_handle_t) * num_strips);

    printf("Pointers: %p %p\n", lines, strips_data);
    for (int i = 0; i < num_strips; i++)
    {
        printf("init LED strip %d \n", i);
        printf("init LED gpio %d num leds %d\n", strips_data[i].gpio, strips_data[i].num_leds);
        led_strips[i] = configure_led_strip(strips_data[i].gpio, strips_data[i].num_leds);
    }
} 

void clean_clear_all_leds(void)
{
    for (int i = 0; i < num_strips; i++)
    {
        for(int j = 0; j < strips_data[i].num_leds; j++) {
            led_strip_set_pixel(led_strips[i], j, 0, 0, 0);
        }
    }
}

void refresh_all_leds(void)
{
    for (int i = 0; i < num_strips; i++)
    {
        led_strip_refresh(led_strips[i]);
    }
}
void set_train_data(VehiclePosition *v)
{
    TripDescriptor *t = v->trip;
    // bool dir = t->direction_id;
    if (t)
    {
        uint16_t stop_id_i = (uint16_t)(strtol(v->stop_id, NULL, 10) % 10000);
        uint16_t route_id_i = (uint16_t)strtol(v->trip->route_id, NULL, 10);
        bool dir = (bool)t->direction_id;

        printf("Train on route %d heading to %d in dir %d\n", route_id_i, stop_id_i, dir);

        for (int i = 0; i < num_lines; i++)
        {
            if (route_id_i == lines[i].id)
            {
                for (int j = 0; j < lines[i].num_stations; j++)
                {
                    if (lines[i].stations[j].id == stop_id_i)
                    {
                        Station *st = &(lines[i].stations[j]);
                        // TODO: FIX DIRS
                        uint8_t node = st->nodes[!dir];
                        printf("LED strip %d node %d\n", lines[i].strip_num, node);
                        if(node != 255) {
                            ESP_ERROR_CHECK(led_strip_set_pixel(led_strips[lines[i].strip_num], node, lines[i].color_r, lines[i].color_g, lines[i].color_b));
                        }
                    }
                }
                break;
            }
        }
        // printf("Train: %s %s %d\n", route_id_i, stop_id_i, );
    }
    else
    {
        // printf("Invalid train info!\n");
    }
}

void parse_train_data(uint8_t *buffer, size_t len)
{
    FeedMessage *m = feed_message__unpack(NULL, len, buffer);
    if (m != NULL)
    {
        printf("clearing leds\n");
            clean_clear_all_leds();

        for (int i = 0; i < m->n_entity; i++)
        {
            VehiclePosition *v = m->entity[i]->vehicle;
            if (v)
            {
                set_train_data(v);
            }
            else
            {
                // printf("Invalid vehicle!");
            }
        }
        refresh_all_leds();
    }
    else
    {
        ESP_LOGE(TAG, "Invalid entity data received");
    }
}