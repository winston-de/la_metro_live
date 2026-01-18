#include <stdio.h>
#include "typedefs.h"
#include "led_strip.h"

#ifndef LED_HELPER_H
#define LED_HELPER_H

led_strip_handle_t configure_led_strip(uint8_t gpio, uint32_t leds);

#endif

