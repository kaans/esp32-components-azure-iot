#ifndef EXAMPLE_GPIO_H
#define EXAMPLE_GPIO_H

#include <stdint.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_err.h"
#include "esp_system.h"

typedef void (*ButtonCallbackHandler_t)(int button_state);

typedef enum example_gpio_led_mode
{
    OFF,
    ON,
    BLINK
} example_gpio_led_mode_t;

void example_gpio_led_init(example_gpio_led_mode_t state);

esp_err_t example_gpio_led_blink_task_start();
esp_err_t example_gpio_button_task_start();
void example_gpio_led_blink_task(void *pvParameter);
void example_gpio_button_task(void *pvParameter);

esp_err_t example_gpio_led_set_delay(uint32_t delay_ms);
void example_gpio_led_set_mode(example_gpio_led_mode_t state);
void example_gpio_led_set_state(int state);
int example_gpio_led_get_state();
void example_gpio_led_toggle_state();

int example_gpio_button_get_state();
void example_gpio_button_set_callback_handler(ButtonCallbackHandler_t callback_handler);

#endif