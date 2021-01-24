#include "example_gpio.h"
#include <stdio.h>

#include "esp_log.h"
#include "driver/gpio.h"

static int led_blink_delay_ms = CONFIG_EXAMPLE_GPIO_LED_BLINK_DELAY_MS;
static example_gpio_led_mode_t led_mode = OFF;
static ButtonCallbackHandler_t button_callback_handler = NULL;
static TaskHandle_t button_task_notify_handle = NULL;
static TaskHandle_t led_task_notify_handle = NULL;

static const char *TAG = "example_gpio";

esp_err_t example_gpio_led_set_delay(uint32_t delay_ms)
{
    if (delay_ms == 0)
    {
        return ESP_ERR_INVALID_ARG;
    }

    led_blink_delay_ms = delay_ms;
    example_gpio_led_set_mode(BLINK);

    return ESP_OK;
}

void example_gpio_led_set_mode(example_gpio_led_mode_t mode)
{
    led_mode = mode;

    if (led_task_notify_handle != NULL)
    {
        xTaskNotifyGive(led_task_notify_handle);
    }
}

int example_gpio_led_get_state()
{
    return !gpio_get_level(CONFIG_EXAMPLE_GPIO_LED_PIN);
}

void example_gpio_led_set_state(int state)
{
    gpio_set_level(CONFIG_EXAMPLE_GPIO_LED_PIN, !state);
}

void example_gpio_led_toggle_state()
{
    example_gpio_led_set_state(!example_gpio_led_get_state());
}

int example_gpio_button_get_state()
{
    return gpio_get_level(CONFIG_EXAMPLE_GPIO_LED_PIN);
}

void example_gpio_button_isr_handler()
{
    vTaskNotifyGiveFromISR(button_task_notify_handle, NULL);
}

void example_gpio_button_set_callback_handler(ButtonCallbackHandler_t callback_handler)
{
    button_callback_handler = callback_handler;
}

void example_gpio_led_init(example_gpio_led_mode_t mode)
{
    gpio_pad_select_gpio(CONFIG_EXAMPLE_GPIO_LED_PIN);
    ESP_ERROR_CHECK(gpio_set_direction(CONFIG_EXAMPLE_GPIO_LED_PIN, GPIO_MODE_INPUT_OUTPUT));
    example_gpio_led_set_mode(mode);
}

void example_gpio_button_init()
{
    ESP_ERROR_CHECK(gpio_install_isr_service(ESP_INTR_FLAG_EDGE));
    ESP_ERROR_CHECK(gpio_isr_handler_add(CONFIG_EXAMPLE_GPIO_BUTTON_PIN, example_gpio_button_isr_handler, NULL));
    gpio_pad_select_gpio(CONFIG_EXAMPLE_GPIO_BUTTON_PIN);
    ESP_ERROR_CHECK(gpio_set_direction(CONFIG_EXAMPLE_GPIO_BUTTON_PIN, GPIO_MODE_INPUT));
    ESP_ERROR_CHECK(gpio_set_pull_mode(CONFIG_EXAMPLE_GPIO_BUTTON_PIN, GPIO_PULLUP_ONLY));
    ESP_ERROR_CHECK(gpio_set_intr_type(CONFIG_EXAMPLE_GPIO_BUTTON_PIN, GPIO_INTR_ANYEDGE));
    ESP_ERROR_CHECK(gpio_intr_enable(CONFIG_EXAMPLE_GPIO_BUTTON_PIN));
}

#ifdef CONFIG_EXAMPLE_GPIO_DEBUG_ENABLE
static void print_task_debug_info(TaskHandle_t handle)
{
    configASSERT(handle);
    TaskStatus_t task_details;

    vTaskGetInfo(handle, &task_details, pdTRUE, eInvalid);

    ESP_LOGI(TAG, "Task name: %s\nStack high water mark: %u",
             (unsigned char *)task_details.pcTaskName, task_details.usStackHighWaterMark);
}
#endif

void example_gpio_led_blink_task(void *pvParameter)
{
    ESP_LOGI(TAG, "Example blink task started");

    led_task_notify_handle = xTaskGetCurrentTaskHandle();

    do
    {
        switch (led_mode)
        {
        case ON:
            example_gpio_led_set_state(1);
            break;
        case OFF:
            example_gpio_led_set_state(0);
            break;
        case BLINK:
            example_gpio_led_toggle_state();
            break;
        default:
            break;
        }
        ulTaskNotifyTake(pdTRUE, led_blink_delay_ms / portTICK_PERIOD_MS);
    } while (1);
}

void example_gpio_button_task(void *pvParameter)
{
    ESP_LOGI(TAG, "Example button task started");

    button_task_notify_handle = xTaskGetCurrentTaskHandle();
    example_gpio_button_init();

    do
    {
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

        ESP_LOGI(TAG, "BUTTON pressed %i", example_gpio_button_get_state());

        if (button_callback_handler != NULL)
        {
            button_callback_handler(example_gpio_button_get_state());
        }
    } while (1);
}

esp_err_t example_gpio_led_blink_task_start()
{
    if (xTaskCreate(&example_gpio_led_blink_task, "blink_task", 1024 * 2, NULL, 5, NULL) != pdPASS)
    {
        ESP_LOGI(TAG, "create led blink task failed");
        return ESP_ERR_NO_MEM;
    }

    return ESP_OK;
}

esp_err_t example_gpio_button_task_start()
{
    if (xTaskCreate(&example_gpio_button_task, "button_task", 1024 * 2, NULL, 5, NULL) != pdPASS)
    {
        ESP_LOGI(TAG, "create button task failed");
        return ESP_ERR_NO_MEM;
    }

    return ESP_OK;
}
