// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include <hsm_client_data.h>
#include <sdkconfig.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <azure_c_shared_utility/crt_abstractions.h>

#include "azure_config.h"
#include "device_config.h"

#include "esp_log.h"

// Provided for sample only, canned values
static const unsigned char EK[] = {0x45, 0x6e, 0x64, 0x6f, 0x72, 0x73, 0x65, 0x6d, 0x65, 0x6e, 0x74, 0x20, 0x6b, 0x65, 0x79, 0x0d, 0x0a};
static const size_t EK_LEN = sizeof(EK) / sizeof(EK[0]);
static const unsigned char SRK[] = {0x53, 0x74, 0x6f, 0x72, 0x65, 0x20, 0x72, 0x6f, 0x6f, 0x74, 0x20, 0x6b, 0x65, 0x79, 0x0d, 0x0a};
static const size_t SRK_LEN = sizeof(SRK) / sizeof(SRK[0]);

typedef struct CUSTOM_HSM_SAMPLE_INFO_TAG
{
  const char *certificate;
  const char *common_name;
  const char *key;
  const unsigned char *endorsment_key;
  size_t ek_length;
  const unsigned char *storage_root_key;
  size_t srk_len;
  char *symm_key;
  char *registration_name;
} CUSTOM_HSM_SAMPLE_INFO;

int hsm_client_x509_init()
{
  return 0;
}

void hsm_client_x509_deinit()
{
}

HSM_CLIENT_HANDLE custom_hsm_create()
{
  HSM_CLIENT_HANDLE result;
  CUSTOM_HSM_SAMPLE_INFO *hsm_info = malloc(sizeof(CUSTOM_HSM_SAMPLE_INFO));
  if (hsm_info == NULL)
  {
    (void)printf("Failued allocating hsm info\r\n");
    result = NULL;
  }
  else
  {
    hsm_info->certificate = device_config_get_client_certificate();
    hsm_info->key = device_config_get_client_private_key();
    hsm_info->common_name = device_config_get_device_name();
    hsm_info->endorsment_key = EK;
    hsm_info->ek_length = EK_LEN;
    hsm_info->storage_root_key = SRK;
    hsm_info->srk_len = SRK_LEN;

#ifdef AZURE_DPS_CONNECTION_METHOD_CS
    hsm_info->symm_key = (char *)azure_config_dps_get_cs_symmetric_key();
    hsm_info->registration_name = (char *)azure_config_dps_get_cs_registration_name();
#endif
    result = hsm_info;
  }
  return result;
}

void custom_hsm_destroy(HSM_CLIENT_HANDLE handle)
{
  if (handle != NULL)
  {
    CUSTOM_HSM_SAMPLE_INFO *hsm_info = (CUSTOM_HSM_SAMPLE_INFO *)handle;
    // Free anything that has been allocated in this module
    free(hsm_info);
  }
}

char *custom_hsm_get_certificate(HSM_CLIENT_HANDLE handle)
{
  char *result;
  if (handle == NULL)
  {
    (void)printf("Invalid handle value specified\r\n");
    result = NULL;
  }
  else
  {
    // TODO: Malloc the certificate for the iothub sdk to free
    // this value will be sent unmodified to the tlsio
    // layer to be processed
    CUSTOM_HSM_SAMPLE_INFO *hsm_info = (CUSTOM_HSM_SAMPLE_INFO *)handle;
    size_t len = strlen(hsm_info->certificate);
    if ((result = (char *)malloc(len + 1)) == NULL)
    {
      (void)printf("Failure allocating certificate\r\n");
      result = NULL;
    }
    else
    {
      strcpy(result, hsm_info->certificate);
    }
  }
  return result;
}

char *custom_hsm_get_key(HSM_CLIENT_HANDLE handle)
{
  char *result;
  if (handle == NULL)
  {
    (void)printf("Invalid handle value specified\r\n");
    result = NULL;
  }
  else
  {
    // TODO: Malloc the private key for the iothub sdk to free
    // this value will be sent unmodified to the tlsio
    // layer to be processed
    CUSTOM_HSM_SAMPLE_INFO *hsm_info = (CUSTOM_HSM_SAMPLE_INFO *)handle;
    size_t len = strlen(hsm_info->key);
    if ((result = (char *)malloc(len + 1)) == NULL)
    {
      (void)printf("Failure allocating certificate\r\n");
      result = NULL;
    }
    else
    {
      strcpy(result, hsm_info->key);
    }
  }
  return result;
}

char *custom_hsm_get_common_name(HSM_CLIENT_HANDLE handle)
{
  char *result;
  if (handle == NULL)
  {
    (void)printf("Invalid handle value specified\r\n");
    result = NULL;
  }
  else
  {
    // TODO: Malloc the common name for the iothub sdk to free
    // this value will be sent to dps
    CUSTOM_HSM_SAMPLE_INFO *hsm_info = (CUSTOM_HSM_SAMPLE_INFO *)handle;
    size_t len = strlen(hsm_info->common_name);
    if ((result = (char *)malloc(len + 1)) == NULL)
    {
      (void)printf("Failure allocating certificate\r\n");
      result = NULL;
    }
    else
    {
      strcpy(result, hsm_info->common_name);
    }
  }
  return result;
}

// Defining the v-table for the x509 hsm calls
static const HSM_CLIENT_X509_INTERFACE x509_interface =
    {
        custom_hsm_create,
        custom_hsm_destroy,
        custom_hsm_get_certificate,
        custom_hsm_get_key,
        custom_hsm_get_common_name};

const HSM_CLIENT_X509_INTERFACE *hsm_client_x509_interface()
{
  // x509 interface pointer
  return &x509_interface;
}

/*********
 * Symmetric key
 **************/

char *custom_hsm_symm_key(HSM_CLIENT_HANDLE handle)
{
  char *result;
  if (handle == NULL)
  {
    (void)printf("Invalid handle value specified\r\n");
    result = NULL;
  }
  else
  {
    // TODO: Malloc the symmetric key for the iothub
    // The SDK will call free() this value
    CUSTOM_HSM_SAMPLE_INFO *hsm_info = (CUSTOM_HSM_SAMPLE_INFO *)handle;
    size_t len = strlen(hsm_info->symm_key);
    if ((result = (char *)malloc(len + 1)) == NULL)
    {
      (void)printf("Failure allocating symm_key\r\n");
      result = NULL;
    }
    else
    {
      strcpy(result, hsm_info->symm_key);
    }
  }
  return result;
}

char *custom_hsm_get_registration_name(HSM_CLIENT_HANDLE handle)
{
  char *result;
  if (handle == NULL)
  {
    (void)printf("Invalid handle value specified\r\n");
    result = NULL;
  }
  else
  {
    // TODO: Malloc the registration name for the iothub
    // The SDK will call free() this value
    CUSTOM_HSM_SAMPLE_INFO *hsm_info = (CUSTOM_HSM_SAMPLE_INFO *)handle;
    size_t len = strlen(hsm_info->registration_name);
    if ((result = (char *)malloc(len + 1)) == NULL)
    {
      (void)printf("Failure allocating registration_name\r\n");
      result = NULL;
    }
    else
    {
      strcpy(result, hsm_info->registration_name);
    }
  }
  return result;
}

int custom_hsm_set_symm_key_info(HSM_CLIENT_HANDLE handle, const char *reg_name, const char *symm_key)
{
  /*
  int result = 0;

  if (reg_name == NULL)
  {
    printf("Regname is null\n");
  }
  else if (symm_key == NULL)
  {
    printf("Symmkey is null\n");
  }
  else
  {
    printf("CUSTOM HSM SET SYMMETRIC KEEEEYYY: %s    %s\n", reg_name, symm_key);
  }

  CUSTOM_HSM_SAMPLE_INFO *hsm_info = (CUSTOM_HSM_SAMPLE_INFO *)handle;

  printf("Existing values: %s     %s\n", hsm_info->registration_name, hsm_info->symm_key);
  printf("Existing values: %p     %p\n", &hsm_info->registration_name, &hsm_info->symm_key);

  if ((result = mallocAndStrcpy_s(&hsm_info->registration_name, reg_name)) != 0)
  {
    (void)printf("Failure allocating registration_name: %i\r\n", result);
    result = -1;
  }

  if ((result = mallocAndStrcpy_s(&hsm_info->symm_key, symm_key)) != 0)
  {
    (void)printf("Failure allocating symm_key: %i\r\n", result);
    result = -1;
  }

  return result;
  */

  return 0;
}

static const HSM_CLIENT_KEY_INTERFACE symm_key_interface =
    {
        custom_hsm_create,
        custom_hsm_destroy,
        custom_hsm_symm_key,
        custom_hsm_get_registration_name,
        custom_hsm_set_symm_key_info};

const HSM_CLIENT_KEY_INTERFACE *hsm_client_key_interface()
{
  return &symm_key_interface;
}
