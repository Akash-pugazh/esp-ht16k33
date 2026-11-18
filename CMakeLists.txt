include($ENV{IDF_PATH}/tools/cmake/version.cmake)

# i2c_master moved from "driver" to "esp_driver_i2c" in ESP-IDF 5.3
if("${IDF_VERSION_MAJOR}.${IDF_VERSION_MINOR}" VERSION_GREATER_EQUAL "5.3")
    set(i2c_component esp_driver_i2c)
else()
    set(i2c_component driver)
endif()

idf_component_register(SRCS "src/ht16k33.c" "src/ht16k33_frame.c" "src/ht16k33_seg.c"
                       INCLUDE_DIRS "include"
                       REQUIRES ${i2c_component})
