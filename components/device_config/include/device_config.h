#ifndef DEVICE_CONFIG_H
#define DEVICE_CONFIG_H

#include "esp_err.h"
#include "nvs_flash.h"

esp_err_t device_config_init();

const char *device_config_get_device_name();

#ifdef CONFIG_DEVICE_CONFIG_CLIENT_CERT_ENABLE
const char *device_config_get_client_certificate();
const char *device_config_get_client_private_key();
#endif

#endif