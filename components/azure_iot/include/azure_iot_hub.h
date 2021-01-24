#ifndef AZURE_IOT_HUB_H
#define AZURE_IOT_HUB_H

#include <sdkconfig.h>
#include <iothub.h>
#include <iothub_client_options.h>
#include <iothub_client_version.h>
#include <iothub_device_client_ll.h>
#include <iothub_message.h>

#include "azure_config.h"

typedef struct IOTHUB_CLIENT_SAMPLE_INFO_TAG
{
    int connected;
    int stop_running;
    int exited;
} IOTHUB_CLIENT_SAMPLE_INFO;

void azure_iot_hub_task(void *pvParameter);
void azure_iot_hub_stop();
IOTHUB_CLIENT_SAMPLE_INFO *azure_iot_hub_get_connection_info();

IOTHUB_CLIENT_RESULT azure_iot_hub_set_message_callback(IOTHUB_CLIENT_MESSAGE_CALLBACK_ASYNC callback, void *user_context_callback);
void azure_iot_hub_send_message(char *msgText);

IOTHUB_CLIENT_RESULT azure_iot_hub_get_device_twin(IOTHUB_CLIENT_DEVICE_TWIN_CALLBACK callback, void *user_context_callback);
IOTHUB_CLIENT_RESULT azure_iot_hub_set_device_twin_callback(IOTHUB_CLIENT_DEVICE_TWIN_CALLBACK callback, void *user_context_callback);
IOTHUB_CLIENT_RESULT azure_iot_hub_send_reported_state(const unsigned char *reported_state,
                                                       size_t size,
                                                       IOTHUB_CLIENT_REPORTED_STATE_CALLBACK reported_state_callback,
                                                       void *user_context_callback);

IOTHUB_CLIENT_RESULT azure_iot_hub_set_device_method_callback(IOTHUB_CLIENT_DEVICE_METHOD_CALLBACK_ASYNC callback, void *user_context_callback);

#endif