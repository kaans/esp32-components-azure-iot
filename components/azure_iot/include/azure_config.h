#ifndef AZURE_CONFIG_H
#define AZURE_CONFIG_H

#include "esp_err.h"

esp_err_t azure_config_init();
esp_err_t azure_config_close();

#ifdef CONFIG_AZURE_IOT_HUB_CONNECTION_METHOD_DPS
const char *azure_config_dps_get_global_prov_uri();
const char *azure_config_dps_get_id_scope();
#ifdef CONFIG_AZURE_DPS_CONNECTION_METHOD_CS
const char *azure_config_dps_get_cs_symmetric_key();
const char *azure_config_dps_get_cs_registration_name();
#endif
#endif

#ifdef CONFIG_AZURE_IOT_HUB_CONNECTION_METHOD_CS
const char *azure_config_iot_get_connection_string();
#endif

#ifdef CONFIG_AZURE_IOT_HUB_CONNECTION_METHOD_X509
const char *azure_config_iot_get_host_name();
#endif

#endif