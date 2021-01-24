#include <stdio.h>
#include <string.h>

#include "ota_update_https.h"

#include "esp_app_format.h"
#include "esp_https_ota.h"
#include "esp_log.h"
#include "esp_ota_ops.h"
#include "freertos/task.h"

#include "esp_sntp.h"

static const char *TAG = "ota_https";

extern const uint8_t ota_server_ca_certificate[] asm("_binary_ota_server_ca_pem_start");

static esp_https_ota_handle_t ota_handle;

static void abort_ota()
{
  ESP_LOGI(TAG, "Aborting OTA");
  esp_err_t result = esp_https_ota_abort(ota_handle);
  if (result != ESP_OK)
  {
    ESP_LOGE(TAG, "Error while aborting OTA: %i", result);
  }
  else
  {
    ESP_LOGI(TAG, "Success");
  }
}

void ota_update_https_start(const char *url)
{
  esp_err_t result;

  esp_https_ota_config_t config;
  const esp_http_client_config_t http_config = {
      .url = url,
      .cert_pem = (char *)ota_server_ca_certificate,
  };

  config.http_config = &http_config;
  config.http_client_init_cb = NULL;
  config.bulk_flash_erase = false;

  result = esp_https_ota_begin(&config, &ota_handle);

  if (result != ESP_OK)
  {
    ESP_LOGE(TAG, "Error while starting OTA update: %i", result);
    abort_ota();
    return;
  }
  ESP_LOGI(TAG, "Starting OTA update");

  esp_app_desc_t app_img_description;
  result = esp_https_ota_get_img_desc(ota_handle, &app_img_description);
  if (result != ESP_OK)
  {
    ESP_LOGE(TAG, "Error while getting image info: %i", result);
    abort_ota();
    return;
  }

  const esp_app_desc_t *current_app_description = esp_ota_get_app_description();
  ESP_LOGI(TAG, "New image date: %s - %s", app_img_description.date, app_img_description.time);
  ESP_LOGI(TAG, "New image version: %s", app_img_description.version);
  ESP_LOGI(TAG, "Local image version: %s", current_app_description->version);

  if (strcmp(app_img_description.version, current_app_description->version) == 0)
  {
    ESP_LOGW(TAG, "Local and remote version are equal, aborting upgrade");
    abort_ota();
    return;
  }

  ESP_LOGI(TAG, "Newer version available, beginning update process");
  result = esp_https_ota_perform(ota_handle);
  while (result == ESP_ERR_HTTPS_OTA_IN_PROGRESS)
  {
    ESP_LOGD(TAG, ".");
    result = esp_https_ota_perform(ota_handle);
  }
  ESP_LOGI(TAG, "");

  if (result != ESP_OK)
  {
    ESP_LOGE(TAG, "Error during OTA process: %i", result);
    abort_ota();
    return;
  }

  int image_length = esp_https_ota_get_image_len_read(ota_handle);
  if (image_length == -1)
  {
    ESP_LOGE(TAG, "Error while getting length of image");
    abort_ota();
    return;
  }
  ESP_LOGI(TAG, "Length of image: %i bytes", image_length);

  ESP_LOGI(TAG, "OTA update done, verifying if complete");

  bool complete = esp_https_ota_is_complete_data_received(ota_handle);
  if (!complete)
  {
    ESP_LOGE(TAG, "Image not completely downloaded");
    abort_ota();
    return;
  }
  ESP_LOGI(TAG, "Image received successfully");

  result = esp_https_ota_finish(ota_handle);
  if (result != ESP_OK)
  {
    ESP_LOGE(TAG, "Error while finishing OTA update: %i", result);
    abort_ota();
    return;
  }
  ESP_LOGI(TAG, "OTA applied successfully");
}

void ota_update_https_start_thread(void *pvParameter)
{
  while (sntp_get_sync_status() != SNTP_SYNC_STATUS_COMPLETED)
  {
    // wait until time is synced to avoid certification errors
    printf("...;\n");
    vTaskDelay(1000);
  }

  const char *url = (const char *)pvParameter;
  ota_update_https_start(url);
  vTaskDelete(NULL);
}