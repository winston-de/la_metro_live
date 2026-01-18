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
led_strip_handle_t lgnd_strip;

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


    LEDStrip lgnd_data = get_lgnd_strip();
    lgnd_strip = configure_led_strip(lgnd_data.gpio, lgnd_data.num_leds);
    for(int i = 0; i < lgnd_data.num_leds; i++) {
        led_strip_set_pixel(lgnd_strip,  lgnd_data.num_leds - i - 1, lines[i].color_r, lines[i].color_g, lines[i].color_b);
    }
    led_strip_refresh(lgnd_strip);
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

    // trains that are being moved on the system will show up in the API response, but will not have an associated trip
    // and will not have an associated direction or next stop
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
                        // printf("found station %d\n", lines[i].id);
                        Station *st = &(lines[i].stations[j]);
                        uint8_t node = st->nodes[dir];
                        printf("LED strip %d node %d\n", lines[i].strip_num, node);

                        // a node of 255 indicates that the section of track is one-way, and there is no associated LED for that direction
                        // This should never happen, as the API would have to return a train that's on nonexistent tracks
                        if(node != 255) {
                            ESP_ERROR_CHECK(led_strip_set_pixel(led_strips[lines[i].strip_num], node, lines[i].color_r, lines[i].color_g, lines[i].color_b));
                        }
                        break;
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

        feed_message__free_unpacked(m, NULL);
    }
    else
    {
        ESP_LOGE(TAG, "Invalid entity data received");
    }
}