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
// #include "line_mapping.c"

#include "simulator.h"
#include "line_mapping.h"

// GPIO assignment
#define LED_STRIP_GPIO_PIN  26
// Numbers of the LED in the strip
#define LED_STRIP_LED_COUNT 6

#define LED_STRIP_RMT_RES_HZ  (10 * 1000 * 1000)


const uint8_t LGND_LEDS = 6;
const uint8_t AE_LEDS = 140;
const uint8_t BD_LEDS = 46;
const uint8_t CK_LEDS = 46;

// static const char *TAG = "example";

static led_strip_handle_t lgnd_strip;
static led_strip_handle_t ae_strip;
static led_strip_handle_t bd_strip;
static led_strip_handle_t ck_strip;


led_strip_handle_t configure_led_strip(uint8_t gpio, uint32_t leds)
{
    // LED strip general initialization, according to your led board design
    led_strip_config_t strip_config = {
        .strip_gpio_num = gpio, // The GPIO that connected to the LED strip's data line
        .max_leds = leds,      // The number of LEDs in the strip,
        .led_model = LED_MODEL_WS2812,        // LED strip model
        // set the color order of the strip: GRB
        .color_component_format = {
            .format = {
                .r_pos = 1, // red is the second byte in the color data
                .g_pos = 0, // green is the first byte in the color data
                .b_pos = 2, // blue is the third byte in the color data
                .num_components = 3, // total 3 color components
            },
        },
        .flags = {
            .invert_out = false, // don't invert the output signal
        }
    };

    // LED strip backend configuration: RMT
    led_strip_rmt_config_t rmt_config = {
        .clk_src = RMT_CLK_SRC_DEFAULT,        // different clock source can lead to different power consumption
        .resolution_hz = LED_STRIP_RMT_RES_HZ, // RMT counter clock frequency
        .mem_block_symbols = 0, // the memory block size used by the RMT channel
        .flags = {
            .with_dma = 0,     // Using DMA can improve performance when driving more LEDs
        }
    };

    // LED Strip object handle
    led_strip_handle_t led_strip;
    ESP_ERROR_CHECK(led_strip_new_rmt_device(&strip_config, &rmt_config, &led_strip));
    ESP_LOGI(TAG, "Created LED strip object with RMT backend");
    return led_strip;
}

void set_all_leds(led_strip_handle_t led_strip, uint32_t r, uint32_t g, uint32_t b, uint8_t max_leds) {
    /* Set the LED pixel using RGB from 0 (0%) to 255 (100%) for each color */
    for (uint8_t i = 0; i < max_leds; i++) {
        ESP_ERROR_CHECK(led_strip_set_pixel(led_strip, i, r, g, b));
    }
    /* Refresh the strip to send data */
    ESP_ERROR_CHECK(led_strip_refresh(led_strip));
}

void clear_all_leds(led_strip_handle_t led_strip) {
    /* Set all LED off to clear all pixels */
    ESP_ERROR_CHECK(led_strip_clear(led_strip));
    ESP_ERROR_CHECK(led_strip_refresh(led_strip));
}

void set_lgnd_colors(void) {
    ESP_ERROR_CHECK(led_strip_set_pixel(lgnd_strip, 5, A_LED_VAL));
    ESP_ERROR_CHECK(led_strip_set_pixel(lgnd_strip, 4, B_LED_VAL));
    ESP_ERROR_CHECK(led_strip_set_pixel(lgnd_strip, 3, C_LED_VAL));
    ESP_ERROR_CHECK(led_strip_set_pixel(lgnd_strip, 2, D_LED_VAL));
    ESP_ERROR_CHECK(led_strip_set_pixel(lgnd_strip, 1, E_LED_VAL));
    ESP_ERROR_CHECK(led_strip_set_pixel(lgnd_strip, 0, K_LED_VAL));

    ESP_ERROR_CHECK(led_strip_refresh(lgnd_strip));
}

static int ae_i[20];
static int bd_i[10];
static int ck_i[8];

void chase_setup(void) {
    for(int i = 0; i < ARRL(ae_i); i++) {
        int res = (int)AE_LEDS / ((int)(ARRL(ae_i)));
        ESP_LOGI(TAG, "Value: %d    %d  %d", res, ARRL(ae_i), AE_LEDS);
        ae_i[i] = ((AE_LEDS / ((int)ARRL(ae_i)))*i) % AE_LEDS;
    }

    for(int i = 0; i < ARRL(bd_i); i++) {
        bd_i[i] = ((BD_LEDS / ((int)ARRL(bd_i)))*i) % BD_LEDS;
    }

    for(int i = 0; i < ARRL(ck_i); i++) {
        ck_i[i] = ((CK_LEDS / ((int)ARRL(ck_i)))*i) % CK_LEDS;
    }
}

void chase(void) {
    ESP_ERROR_CHECK(led_strip_clear(ae_strip));
    ESP_ERROR_CHECK(led_strip_clear(bd_strip));
    ESP_ERROR_CHECK(led_strip_clear(ck_strip));

    for(int i = 0; i < ARRL(ae_i); i++) {
        ESP_ERROR_CHECK(led_strip_set_pixel(ae_strip, ae_i[i], A_LED_VAL));
        ae_i[i]++;
        if(ae_i[i] >= AE_LEDS) ae_i[i] = 0;
    }
    
    for(int i = 0; i < ARRL(bd_i); i++) {
        ESP_ERROR_CHECK(led_strip_set_pixel(bd_strip, bd_i[i], B_LED_VAL));
        bd_i[i]++;
        if(bd_i[i] >= BD_LEDS) bd_i[i] = 0;
    }
    
    for(int i = 0; i < ARRL(ck_i); i++) {
        ESP_ERROR_CHECK(led_strip_set_pixel(ck_strip, ck_i[i], C_LED_VAL));
        ck_i[i]++;
        if(ck_i[i] >= CK_LEDS) ck_i[i] = 0;
    }

    ESP_ERROR_CHECK(led_strip_refresh(ae_strip));
    ESP_ERROR_CHECK(led_strip_refresh(bd_strip));
    ESP_ERROR_CHECK(led_strip_refresh(ck_strip));

}

void simulator(void) {

}

const bool do_chase = 1;

struct LineSim sims[6];

void refresh_all_lines(void) {
    ESP_ERROR_CHECK(led_strip_refresh(ae_strip));
    ESP_ERROR_CHECK(led_strip_refresh(bd_strip));
    ESP_ERROR_CHECK(led_strip_refresh(ck_strip));
}

void run_on_all_sims(void (*f)(struct LineSim *)) {
    for(int i = 0; i < ARRL(sims); i++) {
        f(&sims[i]);
    }
} 


void timer_callback(void *param)
{
    if(do_chase) {chase();} else {
        run_on_all_sims(clear_all_stations);
        run_on_all_sims(simulate_update);
        refresh_all_lines();
    }

}

void app_main(void)
{
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    lgnd_strip = configure_led_strip(26, LGND_LEDS);
    ae_strip = configure_led_strip(27, AE_LEDS);
    bd_strip = configure_led_strip(14, BD_LEDS);
    ck_strip = configure_led_strip(12, CK_LEDS);

    const esp_timer_create_args_t my_timer_args = {
      .callback = &timer_callback,
      .name = "LED Timer"};
    esp_timer_handle_t timer_handler;
    
    uint64_t period = 5000000;
    if(do_chase) {
        period = 500000;
    }

    ESP_ERROR_CHECK(esp_timer_create(&my_timer_args, &timer_handler));
    ESP_ERROR_CHECK(esp_timer_start_periodic(timer_handler, period));
    init_wifi();
    // https_with_url();

    TaskHandle_t xHandle = NULL;
    static uint8_t ucParameterToPass = 1;
    // BaseType_t xReturned = xTaskCreate(https_with_url, "NAME", 11000, &ucParameterToPass, tskIDLE_PRIORITY + 1, &xHandle);
    https_with_url();
    // if(xReturned == pdPASS) {
    //     printf("task created\n");
    // } else {
    //     printf("task failed\n");
    // }

    set_lgnd_colors();
    clear_all_leds(ae_strip);
    clear_all_leds(bd_strip);
    clear_all_leds(ck_strip);


    if(do_chase) {
        chase_setup();
    } else {
        // sims[0] = initialize_line(ae_strip, get_a_line(), 23, 2);
        // sims[1] = initialize_line(ae_strip, get_e_line(), 9, 1);
        // sims[2] = initialize_line(bd_strip, get_b_line(), 6, 3);
        // sims[3] = initialize_line(bd_strip, get_d_line(), 3, 0);
        // sims[4] = initialize_line(ck_strip, get_c_line(), 7, 0);
        // sims[5] = initialize_line(ck_strip, get_k_line(), 6, 0);
        // refresh_all_lines();
    }
    // init_wifi();
    // set_all_leds(ae_strip, A_LED_VAL, AE_LEDS);
    // set_all_leds(bd_strip, B_LED_VAL, BD_LEDS);
    // set_all_leds(ck_strip, C_LED_VAL, CK_LEDS);
}