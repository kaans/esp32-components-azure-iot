#include "azure_iot_hub.h"
#include "azure_config.h"
#include <stdio.h>

#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "freertos/task.h"

#include <azure_c_shared_utility/http_proxy_io.h>
#include <azure_c_shared_utility/shared_util_options.h>
#include <azure_c_shared_utility/threadapi.h>
#include <azure_c_shared_utility/tickcounter.h>

#include "esp_log.h"
#include "esp_wifi.h"

#ifdef CONFIG_AZURE_IOT_USE_TRUSTED_CERT
#include "certs.h"
#endif

#ifdef CONFIG_AZURE_IOT_HUB_CONNECTION_METHOD_X509
#include "device_config.h"
#include <azure_prov_client/prov_security_factory.h>
#elif CONFIG_AZURE_IOT_HUB_CONNECTION_METHOD_DPS
#include "azure_dps.h"
#endif

#ifdef CONFIG_AZURE_IOT_HUB_TRANSPORT_MQTT
#include "iothubtransportmqtt.h"
#endif // CONFIG_AZURE_IOT_HUB_TRANSPORT_MQTT
#ifdef CONFIG_AZURE_IOT_HUB_TRANSPORT_MQTT_OVER_WEBSOCKETS
#include "iothubtransportmqtt_websockets.h"
#endif // CONFIG_AZURE_IOT_HUB_TRANSPORT_MQTT_OVER_WEBSOCKETS
#ifdef CONFIG_AZURE_IOT_HUB_TRANSPORT_AMQP
#include "iothubtransportamqp.h"
#endif // CONFIG_AZURE_IOT_HUB_TRANSPORT_AMQP
#ifdef CONFIG_AZURE_IOT_HUB_TRANSPORT_AMQP_OVER_WEBSOCKETS
#include "iothubtransportamqp_websockets.h"
#endif // CONFIG_AZURE_IOT_HUB_TRANSPORT_AMQP_OVER_WEBSOCKETS
#ifdef CONFIG_AZURE_IOT_HUB_TRANSPORT_HTTP
#include "iothubtransportmqtt.h"
#endif // CONFIG_AZURE_IOT_HUB_TRANSPORT_HTTP

static const char *TAG = "az_iot";

static IOTHUB_CLIENT_SAMPLE_INFO iothub_info;
static IOTHUB_DEVICE_CLIENT_LL_HANDLE device_ll_handle = NULL;

static IOTHUB_CLIENT_MESSAGE_CALLBACK_ASYNC message_callback = NULL;
static IOTHUB_CLIENT_DEVICE_TWIN_CALLBACK device_twin_callback = NULL;
static IOTHUB_CLIENT_DEVICE_METHOD_CALLBACK_ASYNC device_method_callback = NULL;

IOTHUB_CLIENT_RESULT azure_iot_hub_set_message_callback(IOTHUB_CLIENT_MESSAGE_CALLBACK_ASYNC callback, void *user_context_callback)
{
  message_callback = callback;

  if (device_ll_handle != NULL)
  {
    return IoTHubDeviceClient_LL_SetMessageCallback(device_ll_handle, callback, user_context_callback);
  }

  return IOTHUB_CLIENT_ERROR;
}

IOTHUB_CLIENT_RESULT azure_iot_hub_set_device_twin_callback(IOTHUB_CLIENT_DEVICE_TWIN_CALLBACK callback, void *user_context_callback)
{
  device_twin_callback = callback;

  if (device_ll_handle != NULL)
  {
    return IoTHubDeviceClient_LL_SetDeviceTwinCallback(device_ll_handle, callback, user_context_callback);
  }

  return IOTHUB_CLIENT_ERROR;
}

IOTHUB_CLIENT_RESULT azure_iot_hub_send_reported_state(const unsigned char *reported_state,
                                                       size_t size,
                                                       IOTHUB_CLIENT_REPORTED_STATE_CALLBACK reported_state_callback,
                                                       void *user_context_callback)
{
  if (device_ll_handle != NULL)
  {
    return IoTHubDeviceClient_LL_SendReportedState(device_ll_handle, reported_state, size, reported_state_callback, user_context_callback);
  }

  return IOTHUB_CLIENT_ERROR;
}

IOTHUB_CLIENT_RESULT azure_iot_hub_get_device_twin(IOTHUB_CLIENT_DEVICE_TWIN_CALLBACK callback, void *user_context_callback)
{
  if (device_ll_handle != NULL)
  {
    return IoTHubDeviceClient_LL_GetTwinAsync(device_ll_handle, callback, user_context_callback);
  }

  return IOTHUB_CLIENT_ERROR;
}

IOTHUB_CLIENT_RESULT azure_iot_hub_set_device_method_callback(IOTHUB_CLIENT_DEVICE_METHOD_CALLBACK_ASYNC callback, void *user_context_callback)
{
  device_method_callback = callback;

  if (device_ll_handle != NULL)
  {
    return IoTHubDeviceClient_LL_SetDeviceMethodCallback(device_ll_handle, callback, user_context_callback);
  }

  return IOTHUB_CLIENT_ERROR;
}

static void iothub_connection_status(IOTHUB_CLIENT_CONNECTION_STATUS result, IOTHUB_CLIENT_CONNECTION_STATUS_REASON reason, void *user_context)
{
  (void)reason;
  if (user_context == NULL)
  {
    ESP_LOGE(TAG, "iothub_connection_status user_context is NULL");
  }
  else
  {
    IOTHUB_CLIENT_SAMPLE_INFO *iothub_info = (IOTHUB_CLIENT_SAMPLE_INFO *)user_context;
    if (result == IOTHUB_CLIENT_CONNECTION_AUTHENTICATED)
    {
      iothub_info->connected = 1;
    }
    else
    {
      iothub_info->connected = 0;
      iothub_info->stop_running = 1;
    }
  }
}

IOTHUB_CLIENT_SAMPLE_INFO *azure_iot_hub_get_connection_info()
{
  return &iothub_info;
}

const IO_INTERFACE_DESCRIPTION *socketio_get_interface_description(void)
{
  return NULL;
}

void azure_iot_hub_task(void *pvParameter)
{
  iothub_info.exited = 0;
  iothub_info.connected = 0;
  iothub_info.stop_running = 0;

  IOTHUB_CLIENT_TRANSPORT_PROVIDER iothub_transport;

  // Protocol to USE - HTTP, AMQP, AMQP_WS, MQTT, MQTT_WS
#if defined(CONFIG_AZURE_IOT_HUB_TRANSPORT_MQTT) || defined(CONFIG_AZURE_IOT_HUB_TRANSPORT_HTTP) // HTTP CONFIG_AZURE_IOT_HUB_TRANSPORT will use mqtt protocol
  iothub_transport = MQTT_Protocol;
#endif // CONFIG_AZURE_IOT_HUB_TRANSPORT_MQTT
#ifdef CONFIG_AZURE_IOT_HUB_TRANSPORT_MQTT_OVER_WEBSOCKETS
  iothub_transport = MQTT_WebSocket_Protocol;
#endif // CONFIG_AZURE_IOT_HUB_TRANSPORT_MQTT_OVER_WEBSOCKETS
#ifdef CONFIG_AZURE_IOT_HUB_TRANSPORT_AMQP
  iothub_transport = AMQP_Protocol;
#endif // CONFIG_AZURE_IOT_HUB_TRANSPORT_AMQP
#ifdef CONFIG_AZURE_IOT_HUB_TRANSPORT_AMQP_OVER_WEBSOCKETS
  iothub_transport = AMQP_Protocol_over_WebSocketsTls;
#endif // CONFIG_AZURE_IOT_HUB_TRANSPORT_AMQP_OVER_WEBSOCKETS

  ESP_LOGI(TAG, "Iothub API Version: %s", IoTHubClient_GetVersionString());

  IoTHub_Init();

  ESP_LOGI(TAG, "Creating IoTHub Device handle");
#ifdef CONFIG_AZURE_IOT_HUB_CONNECTION_METHOD_DPS

  int prov_result = azure_dps_provision_device();
  if (prov_result != 1)
  {
    ESP_LOGE(TAG, "Could not provision device, exiting thread");
  }
  else
  {
    ESP_LOGI(TAG, "Device provisioned successfully, creating auth");
    AZURE_DPS_CREDENTIALS *dps_credentials = azure_dps_get_credentials();
    device_ll_handle = IoTHubDeviceClient_LL_CreateFromDeviceAuth(dps_credentials->iothub_uri, dps_credentials->device_id, iothub_transport);
  }
#elif CONFIG_AZURE_IOT_HUB_CONNECTION_METHOD_CS
  device_ll_handle = IoTHubDeviceClient_LL_CreateFromConnectionString(azure_config_iot_get_connection_string(), iothub_transport);
#elif CONFIG_AZURE_IOT_HUB_CONNECTION_METHOD_X509
  SECURE_DEVICE_TYPE hsm_type;
  hsm_type = SECURE_DEVICE_TYPE_X509;
  ESP_LOGI(TAG, "-------------------- USING X509 for Azure IoT Handle");

  prov_dev_security_init(hsm_type);
  device_ll_handle = IoTHubDeviceClient_LL_CreateFromDeviceAuth(azure_config_iot_get_host_name(), device_config_get_device_name(), iothub_transport);
#endif
  if (device_ll_handle == NULL)
  {
#ifdef CONFIG_AZURE_IOT_HUB_CONNECTION_METHOD_DPS
    ESP_LOGE(TAG, "failed create IoTHub client through dps!");
#elif CONFIG_AZURE_IOT_HUB_CONNECTION_METHOD_CS
    ESP_LOGE(TAG, "failed create IoTHub client from connection string %s!",
             azure_config_iot_get_connection_string());
#elif CONFIG_AZURE_IOT_HUB_CONNECTION_METHOD_X509
    ESP_LOGE(TAG, "failed create IoTHub client for host %s with device name %s!",
             azure_config_iot_get_host_name(), device_config_get_device_name());
#endif
  }
  else
  {
    ESP_LOGI(TAG, "Auth created successfully");

    iothub_info.stop_running = 0;
    iothub_info.connected = 0;

#ifdef CONFIG_AZURE_IOT_TRACE_ENABLE
    bool traceOn = true;
#else
    bool traceOn = false;
#endif

    azure_iot_hub_set_message_callback(message_callback, NULL);
    azure_iot_hub_set_device_twin_callback(device_twin_callback, NULL);
    azure_iot_hub_set_device_method_callback(device_method_callback, NULL);

    (void)IoTHubDeviceClient_LL_SetConnectionStatusCallback(device_ll_handle, iothub_connection_status, &iothub_info);

    // Set any option that are neccessary.
    // For available options please see the iothub_sdk_options.md documentation

    IoTHubDeviceClient_LL_SetOption(device_ll_handle, OPTION_LOG_TRACE, &traceOn);

#ifdef CONFIG_AZURE_IOT_USE_TRUSTED_CERT
    // Setting the Trusted Certificate.  This is only necessary on system with without
    // built in certificate stores.
    IoTHubDeviceClient_LL_SetOption(device_ll_handle, OPTION_TRUSTED_CERT, certificates);
#endif // CONFIG_AZURE_IOT_USE_TRUSTED_CERT

#if CONFIG_AZURE_IOT_HUB_CONNECTION_METHOD_X509
    IoTHubDeviceClient_LL_SetOption(device_ll_handle, OPTION_X509_CERT, device_config_get_client_certificate());
    IoTHubDeviceClient_LL_SetOption(device_ll_handle, OPTION_X509_PRIVATE_KEY, device_config_get_client_private_key());
#endif

    ESP_LOGD(TAG, "Starting iot hub loop");
    do
    {
      IoTHubDeviceClient_LL_DoWork(device_ll_handle);
      ThreadAPI_Sleep(100);
    } while (iothub_info.stop_running == 0);

    // Clean up the iothub sdk handle
    IoTHubDeviceClient_LL_Destroy(device_ll_handle);
  }

#ifdef CONFIG_AZURE_IOT_HUB_CONNECTION_METHOD_DPS
  azure_dps_deinit();
#endif

  IoTHub_Deinit();

  iothub_info.exited = 1;
  ESP_LOGI(TAG, "Azure IoT hub task exited");

  vTaskDelete(NULL);
}

void azure_iot_hub_stop()
{
  iothub_info.stop_running = 1;
}

void azure_iot_hub_send_message(char *msgText)
{
  static size_t msg_count = 0;

  if (iothub_info.connected != 0)
  {
    IOTHUB_MESSAGE_HANDLE msg_handle = IoTHubMessage_CreateFromByteArray((const unsigned char *)msgText, strlen(msgText));
    if (msg_handle == NULL)
    {
      ESP_LOGE(TAG, "ERROR: iotHubMessageHandle is NULL!");
    }
    else
    {
      IOTHUB_CLIENT_RESULT ret = IoTHubDeviceClient_LL_SendEventAsync(device_ll_handle, msg_handle, NULL, NULL);
      if (ret != IOTHUB_CLIENT_OK)
      {
        ESP_LOGE(TAG, "ERROR: IoTHubClient_LL_SendEventAsync..........FAILED! %s ", MU_ENUM_TO_STRING(IOTHUB_CLIENT_RESULT, ret));
      }
      else
      {
        ESP_LOGD(TAG, "IoTHubClient_LL_SendEventAsync accepted message [%zu] for transmission to IoT Hub.", msg_count);
      }
      IoTHubMessage_Destroy(msg_handle);
    }
  }
}
