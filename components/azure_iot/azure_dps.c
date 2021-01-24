#include "azure_dps.h"
#include "azure_config.h"
#include <stdio.h>

#include "azure_c_shared_utility/crt_abstractions.h"
#include "esp_log.h"
#include <azure_c_shared_utility/threadapi.h>
#include <azure_prov_client/prov_security_factory.h>

#ifdef CONFIG_AZURE_IOT_USE_TRUSTED_CERT
#include "certs.h"
#endif

#ifdef CONFIG_AZURE_DPS_TRANSPORT_MQTT
#include "azure_prov_client/prov_transport_mqtt_client.h"
#endif // CONFIG_AZURE_DPS_TRANSPORT_MQTT
#ifdef CONFIG_AZURE_DPS_TRANSPORT_MQTT_OVER_WEBSOCKETS
#include "azure_prov_client/prov_transport_mqtt_ws_client.h"
#endif // CONFIG_AZURE_DPS_TRANSPORT_MQTT_OVER_WEBSOCKETS
#ifdef CONFIG_AZURE_DPS_TRANSPORT_AMQP
#include "azure_prov_client/prov_transport_amqp_client.h"
#endif // CONFIG_AZURE_DPS_TRANSPORT_AMQP
#ifdef CONFIG_AZURE_DPS_TRANSPORT_AMQP_OVER_WEBSOCKETS
#include "azure_prov_client/prov_transport_amqp_ws_client.h"
#endif // CONFIG_AZURE_DPS_TRANSPORT_AMQP_OVER_WEBSOCKETS
#ifdef CONFIG_AZURE_DPS_TRANSPORT_HTTP
#include "azure_prov_client/prov_transport_http_client.h"
#endif // CONFIG_AZURE_DPS_TRANSPORT_HTTP

MU_DEFINE_ENUM_STRINGS_WITHOUT_INVALID(PROV_DEVICE_RESULT, PROV_DEVICE_RESULT_VALUE);
MU_DEFINE_ENUM_STRINGS_WITHOUT_INVALID(PROV_DEVICE_REG_STATUS, PROV_DEVICE_REG_STATUS_VALUES);

static const char *TAG = "az_dps";

typedef struct CLIENT_SAMPLE_INFO_TAG
{
  unsigned int sleep_time;
  char *iothub_uri;
  char *access_key_name;
  char *device_key;
  char *device_id;
  int registration_complete;
} CLIENT_SAMPLE_INFO;

static AZURE_DPS_CREDENTIALS credentials;

static void registration_status_callback(PROV_DEVICE_REG_STATUS reg_status, void *user_context)
{
  ESP_LOGI(TAG, "Provisioning Status: %s", MU_ENUM_TO_STRING(PROV_DEVICE_REG_STATUS, reg_status));
}

static void register_device_callback(PROV_DEVICE_RESULT register_result, const char *iothub_uri, const char *device_id, void *user_context)
{
  if (user_context == NULL)
  {
    ESP_LOGE(TAG, "user_context is NULL");
  }
  else
  {
    CLIENT_SAMPLE_INFO *user_ctx = (CLIENT_SAMPLE_INFO *)user_context;
    if (register_result == PROV_DEVICE_RESULT_OK)
    {
      ESP_LOGI(TAG, "Registration Information received from service: %s!", iothub_uri);
      mallocAndStrcpy_s(&(&credentials)->iothub_uri, iothub_uri);
      mallocAndStrcpy_s(&(&credentials)->device_id, device_id);

      user_ctx->registration_complete = 1;
    }
    else
    {
      ESP_LOGE(TAG, "Failure encountered on registration %s", MU_ENUM_TO_STRING(PROV_DEVICE_RESULT, register_result));
      user_ctx->registration_complete = 2;
    }
  }
}

int azure_dps_provision_device()
{
  SECURE_DEVICE_TYPE hsm_type;
  //hsm_type = SECURE_DEVICE_TYPE_TPM; // not yet supported
#ifdef CONFIG_AZURE_DPS_CONNECTION_METHOD_X509
  hsm_type = SECURE_DEVICE_TYPE_X509;
#elif CONFIG_AZURE_DPS_CONNECTION_METHOD_CS
  hsm_type = SECURE_DEVICE_TYPE_SYMMETRIC_KEY;
#endif

#ifdef CONFIG_AZURE_IOT_TRACE_ENABLE
  bool traceOn = true;
#else
  bool traceOn = false;
#endif

  printf("PROV DEV SEC INITIALIZED PRE START");

  prov_dev_security_init(hsm_type);

  printf("PROV DEV SEC INITIALIZED DONE");

  prov_dev_set_symmetric_key_info(azure_config_dps_get_cs_registration_name(),
                                  azure_config_dps_get_cs_symmetric_key());

  PROV_DEVICE_TRANSPORT_PROVIDER_FUNCTION prov_transport;
  CLIENT_SAMPLE_INFO user_ctx;

  memset(&user_ctx, 0, sizeof(CLIENT_SAMPLE_INFO));

  // Protocol to USE - HTTP, AMQP, AMQP_WS, MQTT, MQTT_WS
#ifdef CONFIG_AZURE_DPS_TRANSPORT_MQTT
  prov_transport = Prov_Device_MQTT_Protocol;
#endif // CONFIG_AZURE_DPS_TRANSPORT_MQTT
#ifdef CONFIG_AZURE_DPS_TRANSPORT_MQTT_OVER_WEBSOCKETS
  prov_transport = Prov_Device_MQTT_WS_Protocol;
#endif // CONFIG_AZURE_DPS_TRANSPORT_MQTT_OVER_WEBSOCKETS
#ifdef CONFIG_AZURE_DPS_TRANSPORT_AMQP
  prov_transport = Prov_Device_AMQP_Protocol;
#endif // CONFIG_AZURE_DPS_TRANSPORT_AMQP
#ifdef CONFIG_AZURE_DPS_TRANSPORT_AMQP_OVER_WEBSOCKETS
  prov_transport = Prov_Device_AMQP_WS_Protocol;
#endif // CONFIG_AZURE_DPS_TRANSPORT_AMQP_OVER_WEBSOCKETS
#ifdef CONFIG_AZURE_DPS_TRANSPORT_HTTP
  prov_transport = Prov_Device_HTTP_Protocol;
#endif // CONFIG_AZURE_DPS_TRANSPORT_HTTP

  // Set ini
  user_ctx.registration_complete = 0;
  user_ctx.sleep_time = 10;

  ESP_LOGD(TAG, "Provisioning API Version: %s", Prov_Device_LL_GetVersionString());

  PROV_DEVICE_LL_HANDLE handle;
  PROV_DEVICE_RESULT result = PROV_DEVICE_RESULT_INVALID_STATE;
  if ((handle = Prov_Device_LL_Create(azure_config_dps_get_global_prov_uri(),
                                      azure_config_dps_get_id_scope(), prov_transport)) == NULL)
  {
    ESP_LOGE(TAG, "failed calling Prov_Device_LL_Create");
  }
  else
  {
    ESP_LOGI(TAG, "prov device handle created");
    Prov_Device_LL_SetOption(handle, PROV_OPTION_LOG_TRACE, &traceOn);
#ifdef CONFIG_AZURE_IOT_USE_TRUSTED_CERT
    // Setting the Trusted Certificate.  This is only necessary on system with without
    // built in certificate stores.
    Prov_Device_LL_SetOption(handle, OPTION_TRUSTED_CERT, certificates);
#endif // CONFIG_AZURE_IOT_USE_TRUSTED_CERT

    // This option sets the registration ID it overrides the registration ID that is
    // set within the HSM so be cautious if setting this value
    //Prov_Device_SetOption(prov_device_handle, PROV_REGISTRATION_ID, "[REGISTRATION ID]");
    result = Prov_Device_LL_Register_Device(handle, register_device_callback, &user_ctx, registration_status_callback, &user_ctx);

    if (result != PROV_DEVICE_RESULT_OK)
    {
      ESP_LOGE(TAG, "failed calling Prov_Device_LL_Register_Device: %s ", MU_ENUM_TO_STRING(PROV_DEVICE_RESULT, result));
    }
    else
    {
      do
      {
        Prov_Device_LL_DoWork(handle);
        ThreadAPI_Sleep(user_ctx.sleep_time);
      } while (user_ctx.registration_complete == 0);
    }
    Prov_Device_LL_Destroy(handle);
  }

  ESP_LOGI(TAG, "Provisioning done\n");

  return user_ctx.registration_complete;
}

AZURE_DPS_CREDENTIALS *azure_dps_get_credentials()
{
  return &credentials;
}

void azure_dps_deinit()
{
  prov_dev_security_deinit();
}
