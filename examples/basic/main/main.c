/*
 * SPDX-FileCopyrightText: 2026 Aakash
 * SPDX-License-Identifier: Apache-2.0
 */
/* HT16K33 basic example: all LEDs on, blink, then count on a common-cathode 4-digit display
 * (digit d on COM d, segments a-g on ROW0-6) and print key presses. Change the pins below. */
#include <stdio.h>

#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "ht16k33.h"
#include "ht16k33_seg.h"

#define PIN_SDA 6
#define PIN_SCL 7

#define DIGIT(d) {{{0, d}, {1, d}, {2, d}, {3, d}, {4, d}, {5, d}, {6, d}}}
static const ht16k33_seg_digit_t digits[4] = {DIGIT(0), DIGIT(1), DIGIT(2), DIGIT(3)};

void app_main(void)
{
    i2c_master_bus_config_t bus_cfg = {
        .i2c_port = -1,
        .sda_io_num = PIN_SDA,
        .scl_io_num = PIN_SCL,
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .glitch_ignore_cnt = 7,
        .flags.enable_internal_pullup = true,
    };
    i2c_master_bus_handle_t bus;
    ESP_ERROR_CHECK(i2c_new_master_bus(&bus_cfg, &bus));

    ht16k33_handle_t disp;
    ht16k33_config_t cfg = HT16K33_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(ht16k33_new(bus, &cfg, &disp));

    for (int row = 0; row < HT16K33_ROWS; row++)
        for (int com = 0; com < HT16K33_COMS; com++) ht16k33_set_led(disp, row, com, true);
    ESP_ERROR_CHECK(ht16k33_flush(disp));
    ESP_ERROR_CHECK(ht16k33_set_blink(disp, HT16K33_BLINK_2HZ));
    vTaskDelay(pdMS_TO_TICKS(3000));
    ESP_ERROR_CHECK(ht16k33_set_blink(disp, HT16K33_BLINK_OFF));
    ht16k33_clear(disp);

    for (int n = 0;; n = (n + 1) % 10000) {
        ht16k33_seg_number(disp, digits, 4, n);
        ESP_ERROR_CHECK(ht16k33_flush(disp));

        uint16_t keys[HT16K33_KEY_ROWS];
        if (ht16k33_read_keys(disp, keys) == ESP_OK && (keys[0] | keys[1] | keys[2]))
            printf("keys KS0=%04x KS1=%04x KS2=%04x\n", keys[0], keys[1], keys[2]);
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}
