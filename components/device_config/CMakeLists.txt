SET (SOURCES)
SET (TXTFILES)

if (CONFIG_DEVICE_CONFIG_NVS_FACTORY_ENABLE)
    list (APPEND SOURCES "device_config_nvs.c")
else()
    list (APPEND SOURCES "device_config.c")
    if (CONFIG_DEVICE_CONFIG_CLIENT_CERT_ENABLE)
        list (APPEND TXTFILES "${CMAKE_SOURCE_DIR}/certs/client_certificate.pem"
                            "${CMAKE_SOURCE_DIR}/certs/client_private_key.pem")
    endif()
endif()

idf_component_register(SRCS "${SOURCES}"
                    INCLUDE_DIRS "include"
                    EMBED_TXTFILES "${TXTFILES}"
                    REQUIRES "nvs_flash")
