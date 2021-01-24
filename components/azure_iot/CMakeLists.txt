set (SOURCES)

if (CONFIG_AZURE_IOT_ENABLE)
    list (APPEND SOURCES
    "azure_iot_hub.c")
endif()

if (CONFIG_AZURE_IOT_HUB_CONNECTION_METHOD_DPS OR CONFIG_AZURE_IOT_HUB_CONNECTION_METHOD_X509)
    list (APPEND SOURCES
        "custom_hsm_x509.c")
        
    # CONFIG_DEVICE_COMMON_NAME is used by azure-iot-sdk-c to enable device provisioning
    # as well as to include the custom HSM module for x509 certificates.
    # Thus, this option is enabled here by default
    SET(CONFIG_DEVICE_COMMON_NAME CONFIG_DEVICE_NAME CACHE INTERNAL "device name")
else()
    UNSET(CONFIG_DEVICE_COMMON_NAME CACHE)
endif()

if (CONFIG_AZURE_IOT_CONFIG_USE_NVS)
    list (APPEND SOURCES "azure_config_nvs.c")
else()
    list (APPEND SOURCES "azure_config.c")
endif()

if (CONFIG_AZURE_IOT_HUB_CONNECTION_METHOD_DPS)
    list (APPEND SOURCES 
        "azure_dps.c")
endif()

idf_component_register(SRCS "${SOURCES}"
                    INCLUDE_DIRS "include"
                    PRIV_REQUIRES "port"
                                  "device_config")

