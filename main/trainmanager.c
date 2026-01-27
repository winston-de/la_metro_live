#include "trainmanager.h"
#include "gtfs-realtime.pb-c.h"
#include "typedefs.h"
#include "line_mapping.h"
#include <string.h>
#include <stdlib.h>
#include "led_strip.h"
#include "led_helper.h"
#include "esp_log.h"

static char *TAG = "TrainManager";

#define MIN(X, Y) (((X) < (Y)) ? (X) : (Y))
#define MAX(X, Y) (((X) > (Y)) ? (X) : (Y))


#define SAFE_BRIGHTNESS(c) MIN(c*brightness_factor, 255)

MLine *lines;
int num_lines = 0;
led_strip_handle_t *led_strips;
led_strip_handle_t lgnd_strip;

static uint8_t brightness_factor = 2;
const uint8_t max_brightness = 10;

LEDStrip *strips_data;
LEDStrip lgnd_data;

int num_strips = 0;

void init_train_manager(void)
{
    num_lines = get_all_lines(&lines);
    num_strips = get_all_strips(&strips_data);

    led_strips = malloc(sizeof(led_strip_handle_t) * num_strips);

    for (int i = 0; i < num_strips; i++)
    {
        led_strips[i] = configure_led_strip(strips_data[i].gpio, strips_data[i].num_leds);
    }
    clear_all_leds();

    lgnd_data = get_lgnd_strip();
    lgnd_strip = configure_led_strip(lgnd_data.gpio, lgnd_data.num_leds);
    draw_legend();
}

void clear_legend(void)
{
    ESP_ERROR_CHECK(led_strip_clear(lgnd_strip));
}

void change_brightness(int8_t diff) {
    brightness_factor = MAX(MIN(max_brightness, brightness_factor + diff), 1);
    draw_legend();
}

void draw_legend(void)
{
    for (int i = 0; i < lgnd_data.num_leds; i++)
    {
        ESP_ERROR_CHECK(led_strip_set_pixel(lgnd_strip, lgnd_data.num_leds - i - 1, 
            SAFE_BRIGHTNESS(lines[i].color_r), SAFE_BRIGHTNESS(lines[i].color_g), SAFE_BRIGHTNESS(lines[i].color_b)));
    }
    ESP_ERROR_CHECK(led_strip_refresh(lgnd_strip));
}

void clean_clear_all_leds(void)
{
    for (int i = 0; i < num_strips; i++)
    {
        for (int j = 0; j < strips_data[i].num_leds; j++)
        {
            ESP_ERROR_CHECK(led_strip_set_pixel(led_strips[i], j, 0, 0, 0));
        }
    }
}

static bool leds_enabled = true;

void set_state(bool enabled) {
    leds_enabled = enabled;
    if(!enabled) {
        clear_all_leds();
        clear_legend();
    } else {
        draw_legend();
    }
}

void clear_all_leds(void)
{
    for (int i = 0; i < num_strips; i++)
    {
        ESP_ERROR_CHECK(led_strip_clear(led_strips[i]));
    }
}

void refresh_all_leds(void)
{
    for (int i = 0; i < num_strips; i++)
    {
        ESP_ERROR_CHECK(led_strip_refresh(led_strips[i]));
    }
}

static bool error_shown = false;
void show_error_status(void)
{
    // clear the lines, and display the legend as red on exception
    error_shown = true;
    clear_all_leds();
    for (int i = 0; i < lgnd_data.num_leds; i++)
    {
        ESP_ERROR_CHECK(led_strip_set_pixel(lgnd_strip, i, 120, 0, 0));
    }

    ESP_ERROR_CHECK(led_strip_refresh(lgnd_strip));
}

void clear_error_status(void)
{
    if (error_shown)
    {
        draw_legend();
        error_shown = false;
    }
}

void set_train_data(VehiclePosition *v)
{
    TripDescriptor *t = v->trip;

    // out-of-service trains that are being moved on the system will show up in the API response, but will not have an associated trip
    // or next stop data
    if (!t || !v->stop_id || !t->route_id)
    {
        ESP_LOGE(TAG, "Invalid train data received");
        return;
    }

    uint16_t stop_id_i = (uint16_t)(strtol(v->stop_id, NULL, 10) % 10000);
    // the first digit of the stop ID is the same for all stations, so we cut it out to save some memory in the mappings
    // eg 80207 -> 207, 801103 -> 1103

    uint16_t route_id_i = (uint16_t)strtol(t->route_id, NULL, 10);
    bool dir = (bool)t->direction_id;

    ESP_LOGI(TAG, "Train %s on route %d heading to %d in dir %d", v->vehicle->label, route_id_i, stop_id_i, dir);

    for (int i = 0; i < num_lines; i++)
    {
        if (route_id_i == lines[i].id)
        {
            for (int j = 0; j < lines[i].num_stations; j++)
            {
                if (lines[i].stations[j].id == stop_id_i)
                {
                    Station *st = &(lines[i].stations[j]);
                    uint8_t node = st->nodes[dir];

                    // a node of 255 indicates that the section of track is one-way, and there is no associated LED for that direction
                    // This should never happen, as the API would have to return a train that's on nonexistent tracks
                    if (node != 255)
                    {
                        ESP_ERROR_CHECK(led_strip_set_pixel(led_strips[lines[i].strip_num], node, 
                            SAFE_BRIGHTNESS(lines[i].color_r), SAFE_BRIGHTNESS(lines[i].color_g), SAFE_BRIGHTNESS(lines[i].color_b)));
                    }
                    break;
                }
            }
            break;
        }
    }
}

void parse_train_data(uint8_t *buffer, size_t len)
{
    static uint8_t failed_parse_attempts = 0;

    FeedMessage *m = feed_message__unpack(NULL, len, buffer);
    if (m != NULL)
    {
        clean_clear_all_leds();

        failed_parse_attempts = 0;
        clear_error_status();

        for (int i = 0; i < m->n_entity; i++)
        {
            VehiclePosition *v = m->entity[i]->vehicle;
            if (v)
            {
                set_train_data(v);
            }
        }
        // if(leds_enabled) {
        if(!leds_enabled) {
            clean_clear_all_leds();
        }

        feed_message__free_unpacked(m, NULL);
    }
    else
    {
        ESP_LOGE(TAG, "Invalid entity data received");
        failed_parse_attempts++;
        if (failed_parse_attempts > 5)
        {
            show_error_status();
        }
    }
}