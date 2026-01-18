#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_err.h"
#include "esp_timer.h"
#include "esp_mac.h"
#include "wifi_access.c"
#include "https_helper.c"
#include "esp_random.h"
#include "trainmanager.h"
#include "led_helper.h"
#include "esp_netif.h"
#include "esp_sntp.h"
#include "esp_netif_sntp.h"
#include "esp_log.h"
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

void sync_time(void)
{
    esp_sntp_config_t config = ESP_NETIF_SNTP_DEFAULT_CONFIG("pool.ntp.org");
    esp_netif_sntp_init(&config);
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

    if (timeinfo.tm_hour >= ONTIME && timeinfo.tm_hour < OFFTIME)
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