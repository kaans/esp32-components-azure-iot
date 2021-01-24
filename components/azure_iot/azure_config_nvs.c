#include "azure_config.h"
#include "esp_log.h"
#include <sdkconfig.h>

#include "device_config.h"

static nvs_handle_t nvs_handle_config = -1;

static const char *TAG = "azure_config_nvs";

esp_err_t azure_config_init()
{
  ESP_LOGI(TAG, "Initializing azure config");

  if (nvs_handle_config == -1)
  {
    ESP_ERROR_CHECK(nvs_open_from_partition(CONFIG_DEVICE_CONFIG_NVS_FACTORY_PARTITION_NAME,
                                            CONFIG_AZURE_IOT_CONFIG_NVS_NAMESPACE,
                                            NVS_READONLY,
                                            &nvs_handle_config));
  }
  else
  {
    ESP_LOGW(TAG, "NVS factory handle has already been initialized");
  }

  return ESP_OK;
}

esp_err_t azure_config_close()
{
  ESP_LOGI(TAG, "Closing azure config");

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

#ifdef CONFIG_AZURE_IOT_HUB_CONNECTION_METHOD_DPS
static const char *global_prov_uri = "global.azure-devices-provisioning.net";

static char *id_scope = NULL;

const char *azure_config_dps_get_global_prov_uri()
{
  return global_prov_uri;
}

const char *azure_config_dps_get_id_scope()
{
  if (id_scope == NULL)
  {
    size_t required_size;
    ESP_ERROR_CHECK(nvs_get_str(nvs_handle_config, CONFIG_AZURE_DPS_ID_SCOPE_KEY,
                                NULL, &required_size));
    id_scope = malloc(required_size);
    if (id_scope != NULL)
    {
      ESP_ERROR_CHECK(nvs_get_str(nvs_handle_config, CONFIG_AZURE_DPS_ID_SCOPE_KEY,
                                  id_scope, &required_size));
    }
    else
    {
      ESP_LOGE(TAG, "Error while allocating memory for id scope, not enough memory");
    }
  }

  return id_scope;
}

#ifdef CONFIG_AZURE_DPS_CONNECTION_METHOD_CS

static char *cs_symmetric_key = NULL;
static char *cs_registration_name = NULL;

const char *azure_config_dps_get_cs_symmetric_key()
{
  if (cs_symmetric_key == NULL)
  {
    size_t required_size;
    ESP_ERROR_CHECK(nvs_get_str(nvs_handle_config, CONFIG_AZURE_DPS_CS_SYMMETRIC_KEY_KEY,
                                NULL, &required_size));
    cs_symmetric_key = malloc(required_size);
    if (cs_symmetric_key != NULL)
    {
      ESP_ERROR_CHECK(nvs_get_str(nvs_handle_config, CONFIG_AZURE_DPS_CS_SYMMETRIC_KEY_KEY,
                                  cs_symmetric_key, &required_size));
    }
    else
    {
      ESP_LOGE(TAG, "Error while allocating memory for cs symmetric key, not enough memory");
    }
  }

  return cs_symmetric_key;
}

const char *azure_config_dps_get_cs_registration_name()
{
  if (cs_registration_name == NULL)
  {
    size_t required_size;
    ESP_ERROR_CHECK(nvs_get_str(nvs_handle_config, CONFIG_AZURE_DPS_CS_REGISTRATION_NAME_KEY,
                                NULL, &required_size));
    cs_registration_name = malloc(required_size);
    if (cs_registration_name != NULL)
    {
      ESP_ERROR_CHECK(nvs_get_str(nvs_handle_config, CONFIG_AZURE_DPS_CS_REGISTRATION_NAME_KEY,
                                  cs_registration_name, &required_size));
    }
    else
    {
      ESP_LOGE(TAG, "Error while allocating memory for cs registration name, not enough memory");
    }
  }

  return cs_registration_name;
}

#endif

#elif CONFIG_AZURE_IOT_HUB_CONNECTION_METHOD_CS
static const char *connection_string = CONFIG_AZURE_IOT_HUB_CONNECTION_STRING;

static char *connection_string = NULL;

const char *azure_config_iot_get_connection_string()
{
  if (connection_string == NULL)
  {
    size_t required_size;
    ESP_ERROR_CHECK(nvs_get_str(nvs_handle_config, CONFIG_AZURE_IOT_HUB_CONNECTION_STRING_KEY,
                                NULL, &required_size));
    connection_string = malloc(required_size);
    if (connection_string != NULL)
    {
      ESP_ERROR_CHECK(nvs_get_str(nvs_handle_config, CONFIG_AZURE_IOT_HUB_CONNECTION_STRING_KEY,
                                  connection_string, &required_size));
    }
    else
    {
      ESP_LOGE(TAG, "Error while allocating memory for connection string, not enough memory");
    }
  }

  return connection_string;
}
#elif CONFIG_AZURE_IOT_HUB_CONNECTION_METHOD_X509

static char *host_name = NULL;

const char *azure_config_iot_get_host_name()
{
  if (host_name == NULL)
  {
    size_t required_size;
    ESP_ERROR_CHECK(nvs_get_str(nvs_handle_config, CONFIG_AZURE_IOT_HUB_HOST_NAME_KEY,
                                NULL, &required_size));
    host_name = malloc(required_size);
    if (host_name != NULL)
    {
      ESP_ERROR_CHECK(nvs_get_str(nvs_handle_config, CONFIG_AZURE_IOT_HUB_HOST_NAME_KEY,
                                  host_name, &required_size));
    }
    else
    {
      ESP_LOGE(TAG, "Error while allocating memory for host name, not enough memory");
    }
  }

  return host_name;
}

#endif
