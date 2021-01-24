set (SOURCES)
SET (TXTFILES)

if (CONFIG_OTA_UPDATE_HTTPS_ENABLE)
    list (APPEND SOURCES "ota_update_https.c")

    list (APPEND TXTFILES "${CMAKE_SOURCE_DIR}/certs/ota_server_ca.pem")
endif()

idf_component_register(SRCS "${SOURCES}"
                    INCLUDE_DIRS "include"
                    EMBED_TXTFILES "${TXTFILES}"
                    REQUIRES "esp_https_ota"
                        "app_update")
