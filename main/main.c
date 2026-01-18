#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "led_strip.h"
#include "esp_log.h"
#include "esp_err.h"
#include "esp_timer.h"
#include "esp_mac.h"
#include "esp_crt_bundle.h"
#include "wifi_access.c"
#include "https_helper.c"
#include "esp_random.h"
#include "trainmanager.h"
#include "led_helper.h"
#include "esp_netif.h"
#include "esp_sntp.h"
#include "esp_netif_sntp.h"
#include "esp_log.h"
// #include "line_mapping.c"

#include "simulator.h"
#include "line_mapping.h"

// GPIO assignment
#define LED_STRIP_GPIO_PIN 26
// Numbers of the LED in the strip
#define LED_STRIP_LED_COUNT 6

#define LED_STRIP_RMT_RES_HZ (10 * 1000 * 1000)

// current time zone
#define TZ "PST8PDT,M3.2.0,M11.1.0"
// what time the board should turn on in the morning and off at night
#define ONTIME 7
#define OFFTIME 22

// how often the board should update data (seconds)
// API is only accurate to 3s, so don't make it less than that
#define UPDATE_PERIOD 6
// minutes
#define TIME_SYNC_PERIOD 60

// static const char *TAG = "example";

static led_strip_handle_t lgnd_strip;
static led_strip_handle_t ae_strip;
static led_strip_handle_t bd_strip;
static led_strip_handle_t ck_strip;

// void set_all_leds(led_strip_handle_t led_strip, uint32_t r, uint32_t g, uint32_t b, uint8_t max_leds) {
//     /* Set the LED pixel using RGB from 0 (0%) to 255 (100%) for each color */
//     for (uint8_t i = 0; i < max_leds; i++) {
//         ESP_ERROR_CHECK(led_strip_set_pixel(led_strip, i, r, g, b));
//     }
//     /* Refresh the strip to send data */
//     ESP_ERROR_CHECK(led_strip_refresh(led_strip));
// }

// void clear_all_leds(led_strip_handle_t led_strip) {
//     /* Set all LED off to clear all pixels */
//     ESP_ERROR_CHECK(led_strip_clear(led_strip));
//     ESP_ERROR_CHECK(led_strip_refresh(led_strip));
// }

// void set_lgnd_colors(void) {
//     ESP_ERROR_CHECK(led_strip_set_pixel(lgnd_strip, 5, A_LED_VAL));
//     ESP_ERROR_CHECK(led_strip_set_pixel(lgnd_strip, 4, B_LED_VAL));
//     ESP_ERROR_CHECK(led_strip_set_pixel(lgnd_strip, 3, C_LED_VAL));
//     ESP_ERROR_CHECK(led_strip_set_pixel(lgnd_strip, 2, D_LED_VAL));
//     ESP_ERROR_CHECK(led_strip_set_pixel(lgnd_strip, 1, E_LED_VAL));
//     ESP_ERROR_CHECK(led_strip_set_pixel(lgnd_strip, 0, K_LED_VAL));

//     ESP_ERROR_CHECK(led_strip_refresh(lgnd_strip));
// }

static int ae_i[20];
static int bd_i[10];
static int ck_i[8];

void chase_setup(void)
{
    // for(int i = 0; i < ARRL(ae_i); i++) {
    //     int res = (int)AE_LEDS / ((int)(ARRL(ae_i)));
    //     ESP_LOGI(TAG, "Value: %d    %d  %d", res, ARRL(ae_i), AE_LEDS);
    //     ae_i[i] = ((AE_LEDS / ((int)ARRL(ae_i)))*i) % AE_LEDS;
    // }

    // for(int i = 0; i < ARRL(bd_i); i++) {
    //     bd_i[i] = ((BD_LEDS / ((int)ARRL(bd_i)))*i) % BD_LEDS;
    // }

    // for(int i = 0; i < ARRL(ck_i); i++) {
    //     ck_i[i] = ((CK_LEDS / ((int)ARRL(ck_i)))*i) % CK_LEDS;
    // }
}

void sync_time(void)
{
    esp_sntp_config_t config = ESP_NETIF_SNTP_DEFAULT_CONFIG("pool.ntp.org");
    esp_netif_sntp_init(&config);
}

const bool do_chase = 1;

struct LineSim sims[6];

// void refresh_all_lines(void) {
//     ESP_ERROR_CHECK(led_strip_refresh(ae_strip));
//     ESP_ERROR_CHECK(led_strip_refresh(bd_strip));
//     ESP_ERROR_CHECK(led_strip_refresh(ck_strip));
// }

void run_on_all_sims(void (*f)(struct LineSim *))
{
    for (int i = 0; i < ARRL(sims); i++)
    {
        f(&sims[i]);
    }
}

static bool cleared = false;

void update_timer_callback(void *param)
{

    // this code turns the lights off during the night
    time_t now;
    struct tm timeinfo;

    time(&now);
    setenv("TZ", TZ, 1);
    tzset();

    localtime_r(&now, &timeinfo);

    if (timeinfo.tm_hour >= ONTIME && timeinfo.tm_hour <= OFFTIME)
    {
        https_with_url();
        cleared = false;
    }
    else if (!cleared)
    {
        printf("Clearing\n");
        clear_all_leds();
        cleared = true;
    }
    printf("Current time %d\n", timeinfo.tm_hour);
}

void sync_time_callback(void *param)
{
    ESP_LOGI(TAG, "Syncing time");
    sync_time();
}

void app_main(void)
{
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    init_wifi();
    init_train_manager();
    sync_time();

    const esp_timer_create_args_t led_timer_args = {
        .callback = &update_timer_callback,
        .name = "Train update"};
    esp_timer_handle_t update_timer_handler;
    ESP_ERROR_CHECK(esp_timer_create(&led_timer_args, &update_timer_handler));
    ESP_ERROR_CHECK(esp_timer_start_periodic(update_timer_handler, UPDATE_PERIOD * 1000 * 1000));

    // timer for time sync
    const esp_timer_create_args_t sync_timer_args = {
        .callback = &sync_time_callback,
        .name = "Time sync update"};
    esp_timer_handle_t sync_timer_handler;
    ESP_ERROR_CHECK(esp_timer_create(&sync_timer_args, &sync_timer_handler));
    ESP_ERROR_CHECK(esp_timer_start_periodic(sync_timer_handler, TIME_SYNC_PERIOD*(uint64_t)(60 * 1000 * 1000)));
    
    update_timer_callback(NULL);

}