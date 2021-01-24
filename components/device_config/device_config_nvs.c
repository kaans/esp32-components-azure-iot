#include "device_config.h"
#include <stdio.h>

#include "esp_log.h"

static const char *TAG = "device_config_nvs";

static nvs_handle_t nvs_handle_config = -1;

static char *device_name = NULL;
static char *client_certificate = NULL;
static char *client_key = NULL;

esp_err_t device_config_init()
{
  esp_err_t ret = nvs_flash_init_partition(CONFIG_DEVICE_CONFIG_NVS_FACTORY_PARTITION_NAME);

  if (ret != ESP_OK)
  {
    ESP_LOGE(TAG, "Error while initializing partition %s for device config: %s",
             CONFIG_DEVICE_CONFIG_NVS_FACTORY_PARTITION_NAME,
             esp_err_to_name(ret));
  }
  else
  {
    if (nvs_handle_config == -1)
    {
      ESP_ERROR_CHECK(nvs_open_from_partition(CONFIG_DEVICE_CONFIG_NVS_FACTORY_PARTITION_NAME,
                                              CONFIG_DEVICE_CONFIG_NVS_FACTORY_NAMESPACE,
                                              NVS_READONLY,
                                              &nvs_handle_config));

      ESP_LOGI(TAG, "NVS partition %s initialized successfully",
               CONFIG_DEVICE_CONFIG_NVS_FACTORY_PARTITION_NAME);
    }
    else
    {
      ESP_LOGW(TAG, "NVS handle for partition %s has already been initialized",
               CONFIG_DEVICE_CONFIG_NVS_FACTORY_PARTITION_NAME);
    }
  }
  return ret;
}

esp_err_t device_config_close()
{
  ESP_LOGI(TAG, "Closing device config");

  if (nvs_handle_config != -1)
  {
    nvs_close(nvs_handle_config);
  }
  else
  {
    ESP_LOGW(TAG, "NVS factory handle has already been closed");
  }

  return ESP_OK;
}

const char *device_config_get_device_name()
{
  if (device_name == NULL)
  {
    size_t required_size;
    ESP_ERROR_CHECK(nvs_get_str(nvs_handle_config, CONFIG_DEVICE_NAME_KEY,
                                NULL, &required_size));
    device_name = malloc(required_size);
    if (device_name != NULL)
    {
      ESP_ERROR_CHECK(nvs_get_str(nvs_handle_config, CONFIG_DEVICE_NAME_KEY,
                                  device_name, &required_size));
    }
    else
    {
      ESP_LOGE(TAG, "Error while allocating memory for device name, not enough memory");
    }
  }

  return device_name;
}

#ifdef CONFIG_DEVICE_CONFIG_CLIENT_CERT_ENABLE
const char *device_config_get_client_certificate()
{
  if (client_certificate == NULL)
  {
    size_t required_size;
    ESP_ERROR_CHECK(nvs_get_blob(nvs_handle_config, CONFIG_DEVICE_CONFIG_CLIENT_CERT_KEY,
                                 NULL, &required_size));
    client_certificate = malloc(required_size);
    if (client_certificate != NULL)
    {
      ESP_ERROR_CHECK(nvs_get_blob(nvs_handle_config, CONFIG_DEVICE_CONFIG_CLIENT_CERT_KEY,
                                   client_certificate, &required_size));
    }
    else
    {
      ESP_LOGE(TAG, "Error while allocating memory for client certificate, not enough memory");
    }
  }

  return client_certificate;
}

const char *device_config_get_client_private_key()
{
  if (client_key == NULL)
  {
    size_t required_size;
    ESP_ERROR_CHECK(nvs_get_blob(nvs_handle_config, CONFIG_DEVICE_CONFIG_CLIENT_PRIV_KEY_KEY,
                                 NULL, &required_size));
    client_key = malloc(required_size);
    if (client_key != NULL)
    {
      ESP_ERROR_CHECK(nvs_get_blob(nvs_handle_config, CONFIG_DEVICE_CONFIG_CLIENT_PRIV_KEY_KEY,
                                   client_key, &required_size));
    }
    else
    {
      ESP_LOGE(TAG, "Error while allocating memory for client provate key, not enough memory");
    }
  }

  return client_key;
}
#endif