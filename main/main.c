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

#define TEST 0


#define REQUEST_UPDATE_EVENT_ID 20
ESP_EVENT_DECLARE_BASE(CUSTOM_EVENTS);
ESP_EVENT_DEFINE_BASE(CUSTOM_EVENTS);

void sync_time(void)
{
    esp_sntp_config_t config = ESP_NETIF_SNTP_DEFAULT_CONFIG("pool.ntp.org");
    esp_netif_sntp_init(&config);
}

static bool cleared = false;
static uint8_t overrideState = 0; // 0 = no override, 1 = override off, 2 = override on

void core_task(void *pvParameters)
{
    static bool running = false;
    if (!running)
    {
        running = true;
        https_with_url();
        // refresh_all_leds();
        running = false;
    }
    else
    {
        ESP_LOGI(TAG, "Already running update, skipping");
    }

    vTaskDelete(NULL);
}

void update_timer_callback(void *param)
{

    static bool running = false;
    // this code turns the lights off and stops polling the API at night
    time_t now;
    struct tm timeinfo;

    time(&now);
    setenv("TZ", TZ, 1);
    tzset();

    localtime_r(&now, &timeinfo);

    if ((timeinfo.tm_hour >= ONTIME && timeinfo.tm_hour < OFFTIME && overrideState != 1) || overrideState == 2)
    {
        if (cleared)
        {
            set_state(true);
        }

        if(!running) {
            running = true;
            https_with_url();
            running = false;
        } else {
            ESP_LOGI(TAG, "Already in progress");
        }
        // refresh_all_leds();
        // xTaskCreatePinnedToCore(
        //     core_task,      // Function to implement the task
        //     "Update Board", // Name of the task
        //     DEFAULT_THREAD_STACKSIZE*4,          // Stack size in words (or bytes depending on IDF version)
        //     NULL,           // Task input parameter
        //     1,              // Priority of the task
        //     NULL,           // Task handle (can be NULL if not needed)
        //     1               // Core to pin to (0 for PRO_CPU, 1 for APP_CPU)
        // );

        cleared = false;
    }
    else if (!cleared)
    {
        printf("Clearing\n");
        set_state(false);
        cleared = true;
    }

    printf("Current time %d\n", timeinfo.tm_hour);
}

void request_force_update(void) {
    esp_event_post(CUSTOM_EVENTS, REQUEST_UPDATE_EVENT_ID, NULL, 0, portMAX_DELAY);
}

void force_update(void)
{
    update_timer_callback(NULL);
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

const button_config_t power_btn_cfg = {0};
const button_gpio_config_t power_btn_gpio_cfg = {
    .gpio_num = 2,
    .active_level = 0,
};

void brightness_up_pressed(void *arg, void *usr_data)
{
    ESP_LOGI(TAG, "Brightness up pressed");
    change_brightness(1);
    // request_force_update();
}

void brightness_down_pressed(void *arg, void *usr_data)
{
    ESP_LOGI(TAG, "Brightness down pressed");
    change_brightness(-1);
    // request_force_update();
}

void power_pressed(void *arg, void *usr_data)
{
    ESP_LOGI(TAG, "Power pressed");
    if (cleared)
    {
        overrideState = 2;
    }
    else
    {
        overrideState = 1;
    }
    // request_force_update();
}

void power_double_pressed(void *arg, void *usr_data)
{
    ESP_LOGI(TAG, "Power double");
    overrideState = 0;
    // request_force_update();
}

void button_task(void *param)
{
    button_handle_t bright_up_bttn = NULL;
    ESP_ERROR_CHECK(iot_button_new_gpio_device(&brightness_up_btn_cfg, &brightness_up_btn_gpio_cfg, &bright_up_bttn));
    iot_button_register_cb(bright_up_bttn, BUTTON_SINGLE_CLICK, NULL, brightness_up_pressed, NULL);

    button_handle_t bright_dn_bttn = NULL;
    ESP_ERROR_CHECK(iot_button_new_gpio_device(&brightness_dn_btn_cfg, &brightness_dn_btn_gpio_cfg, &bright_dn_bttn));
    iot_button_register_cb(bright_dn_bttn, BUTTON_SINGLE_CLICK, NULL, brightness_down_pressed, NULL);

    button_handle_t power_bttn = NULL;
    ESP_ERROR_CHECK(iot_button_new_gpio_device(&power_btn_cfg, &power_btn_gpio_cfg, &power_bttn));
    iot_button_register_cb(power_bttn, BUTTON_SINGLE_CLICK, NULL, power_pressed, NULL);
    iot_button_register_cb(power_bttn, BUTTON_DOUBLE_CLICK, NULL, power_double_pressed, NULL);

    // while (true)
    // {
    //     // if (iot_button_get_event(bright_up_bttn) == BUTTON_SINGLE_CLICK)
    //     //     brightness_up_pressed();
    //     // if (iot_button_get_event(bright_dn_bttn) == BUTTON_SINGLE_CLICK)
    //     //     brightness_down_pressed();
    //     // button_event_t power = iot_button_get_event(power_bttn);
    //     // switch (power)
    //     // {
    //     // case BUTTON_SINGLE_CLICK:
    //     //     power_pressed();
    //     //     break;

    //     // case BUTTON_DOUBLE_CLICK:
    //     //     power_double_pressed();
    //     //     break;
    //     // default:
    //     //     break;
    //     // }

    //     vTaskDelay(pdMS_TO_TICKS(20));
    // }
    vTaskDelete(NULL);
}

void update_request_handler(void* handler_arg, esp_event_base_t base, int32_t id, void* event_data) {
    ESP_LOGI(TAG, "Manual update requested");
    update_timer_callback(NULL);
}

void app_main(void)
{
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    init_train_manager();
    ESP_ERROR_CHECK(esp_event_handler_register(CUSTOM_EVENTS, REQUEST_UPDATE_EVENT_ID, update_request_handler, NULL));

#if TEST
    while (true)
    {
        clear_all_leds();
        clear_legend();
        sleep(1);
        run_test();
        sleep(1);
    }
#else
    init_wifi();
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

    xTaskCreatePinnedToCore(
        button_task,      // Function to implement the task
        "Watch Buttons", // Name of the task
        DEFAULT_THREAD_STACKSIZE,          // Stack size in words (or bytes depending on IDF version)
        NULL,           // Task input parameter
        1,              // Priority of the task
        NULL,           // Task handle (can be NULL if not needed)
        0               // Core to pin to (0 for PRO_CPU, 1 for APP_CPU)
    );

    // setup_buttons();
    force_update();

#endif
}