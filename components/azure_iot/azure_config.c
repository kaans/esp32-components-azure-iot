#include "azure_config.h"
#include "esp_log.h"
#include <sdkconfig.h>

static const char *TAG = "azure_config";

esp_err_t azure_config_init()
{
  ESP_LOGI(TAG, "Initializing azure config");

  return ESP_OK;
}

esp_err_t azure_config_close()
{
  ESP_LOGI(TAG, "Closing azure config");

  return ESP_OK;
}

#ifdef CONFIG_AZURE_IOT_HUB_CONNECTION_METHOD_DPS

const char *azure_config_dps_get_global_prov_uri()
{
  return "global.azure-devices-provisioning.net";
}

const char *azure_config_dps_get_id_scope()
{
  return CONFIG_AZURE_DPS_ID_SCOPE;
}

#ifdef CONFIG_AZURE_DPS_CONNECTION_METHOD_CS

const char *azure_config_dps_get_cs_symmetric_key()
{
  return CONFIG_AZURE_DPS_CS_SYMMETRIC_KEY;
}

const char *azure_config_dps_get_cs_registration_name()
{
  return CONFIG_AZURE_DPS_CS_REGISTRATION_NAME;
}

#endif

#elif CONFIG_AZURE_IOT_HUB_CONNECTION_METHOD_CS

const char *azure_config_iot_get_connection_string()
{
  return CONFIG_AZURE_IOT_HUB_CONNECTION_STRING;
}

#elif CONFIG_AZURE_IOT_HUB_CONNECTION_METHOD_X509

const char *azure_config_iot_get_host_name()
{
  return CONFIG_AZURE_IOT_HUB_HOST_NAME;
}

#endif
