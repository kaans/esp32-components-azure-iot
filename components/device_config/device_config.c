#include "device_config.h"
#include <stdio.h>

#include "esp_log.h"

static const char *TAG = "device_config";

extern const char client_certificate_pem_start[] asm("_binary_client_certificate_pem_start");
extern const char client_private_key_pem_start[] asm("_binary_client_private_key_pem_start");

esp_err_t device_config_init()
{
  ESP_LOGI(TAG, "Device config init");
  return ESP_OK;
}

esp_err_t device_config_close()
{
  ESP_LOGI(TAG, "Device config close");
  return ESP_OK;
}

const char *device_config_get_device_name()
{
  return CONFIG_DEVICE_NAME;
}

#ifdef CONFIG_DEVICE_CONFIG_CLIENT_CERT_ENABLE
const char *device_config_get_client_certificate()
{
  return client_certificate_pem_start;
}

const char *device_config_get_client_private_key()
{
  return client_private_key_pem_start;
}
#endif