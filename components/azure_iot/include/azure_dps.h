#ifndef AZURE_DPS_H
#define AZURE_DPS_H

#include <azure_prov_client/prov_device_ll_client.h>
#include <sdkconfig.h>

typedef struct AZURE_DPS_CREDENTIALS_TAG
{
  char *iothub_uri;
  char *device_id;
} AZURE_DPS_CREDENTIALS;

int azure_dps_provision_device();
AZURE_DPS_CREDENTIALS *azure_dps_get_credentials();
void azure_dps_deinit();

#endif