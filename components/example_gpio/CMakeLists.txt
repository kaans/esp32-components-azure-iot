set (SOURCES)

if (CONFIG_EXAMPLE_GPIO_ENABLE)
    list (APPEND SOURCES "example_gpio.c")
endif()

idf_component_register(SRCS "${SOURCES}"
                    INCLUDE_DIRS "include")
