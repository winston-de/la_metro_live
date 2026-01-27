#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_err.h"
#include "esp_timer.h"
#include "wifi_access.c"
#include "https_helper.c"
#include "trainmanager.h"
#include "esp_netif_sntp.h"
#include "esp_log.h"
#include "driver/gpio.h"
#include "button_gpio.h"
#include "iot_button.h"

// current time zone
#define TZ "PST8PDT,M3.2.0,M11.1.0"
// what time the board should turn on in the morning and off at night
#define ONTIME 7
#define OFFTIME 22

// how often the board should update data (seconds)
// API is only accurate to 3s, so don't make it less than that
#define UPDATE_PERIOD 6

// How often the ESP should sync time with the server (minutes)
#define TIME_SYNC_PERIOD 60

void sync_time(void)
{
    esp_sntp_config_t config = ESP_NETIF_SNTP_DEFAULT_CONFIG("pool.ntp.org");
    esp_netif_sntp_init(&config);
}

static bool cleared = false;

void update_timer_callback(void *param)
{

    // this code turns the lights off and stops polling the API at night
    time_t now;
    struct tm timeinfo;

    time(&now);
    setenv("TZ", TZ, 1);
    tzset();

    localtime_r(&now, &timeinfo);

    if (timeinfo.tm_hour >= ONTIME && timeinfo.tm_hour < OFFTIME)
    {
        if (cleared)
        {
            draw_legend();
        }
        https_with_url();
        cleared = false;
    }
    else if (!cleared)
    {
        printf("Clearing\n");
        clear_legend();
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

// create gpio button
const button_config_t brightness_up_btn_cfg = {0};
const button_gpio_config_t brightness_up_btn_gpio_cfg = {
    .gpio_num = 0,
    .active_level = 0,
};

const button_config_t brightness_dn_btn_cfg = {0};
const button_gpio_config_t brightness_dn_btn_gpio_cfg = {
    .gpio_num = 4,
    .active_level = 0,
};

void brightness_up_pressed(void *arg,void *usr_data)
{
    ESP_LOGI(TAG, "Brightness up pressed");
    change_brightness(1);
}

void brightness_down_pressed(void *arg,void *usr_data)
{
    ESP_LOGI(TAG, "Brightness down pressed");
    change_brightness(-1);
}

void setup_buttons(void)
{
    button_handle_t bright_up_bttn = NULL;
    ESP_ERROR_CHECK(iot_button_new_gpio_device(&brightness_up_btn_cfg, &brightness_up_btn_gpio_cfg, &bright_up_bttn));
    iot_button_register_cb(bright_up_bttn, BUTTON_SINGLE_CLICK, NULL, brightness_up_pressed, NULL);

    button_handle_t bright_dn_bttn = NULL;
    ESP_ERROR_CHECK(iot_button_new_gpio_device(&brightness_dn_btn_cfg, &brightness_dn_btn_gpio_cfg, &bright_dn_bttn));
    iot_button_register_cb(bright_dn_bttn, BUTTON_SINGLE_CLICK, NULL, brightness_down_pressed, NULL);
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
    ESP_ERROR_CHECK(esp_timer_start_periodic(sync_timer_handler, TIME_SYNC_PERIOD * (uint64_t)(60 * 1000 * 1000)));

    setup_buttons();

    update_timer_callback(NULL);
}