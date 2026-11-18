/*
 * SPDX-FileCopyrightText: 2026 Aakash
 * SPDX-License-Identifier: Apache-2.0
 */
/*
 * HT16K33 driver: 16x8 LED matrix controller with 13x3 key scan, on the ESP-IDF i2c_master driver.
 *
 * Datasheet terms: ROW0-ROW15 source current (breakout pins A0-A15), COM0-COM7 sink it (C0-C7).
 * An LED is addressed by (row, com). How rows/coms map to digits or icons is board knowledge and
 * stays in the application; see ht16k33_seg.h for a 7-segment helper that takes such a map.
 *
 * LED writes go to a RAM frame inside the handle; ht16k33_flush() sends the frame in one transfer.
 * A handle is not thread-safe: use it from one task or guard it with your own lock.
 */
#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "driver/i2c_master.h"
#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

#define HT16K33_ADDR_DEFAULT   0x70 /* A0-A2 address pads open; 0x70-0x77 with pads */
#define HT16K33_ROWS           16
#define HT16K33_COMS           8
#define HT16K33_BRIGHTNESS_MAX 15
#define HT16K33_KEY_ROWS       3    /* KS0-KS2 (COM1-COM3) */

typedef struct ht16k33_t *ht16k33_handle_t;

typedef struct {
    uint16_t addr;         /* 7-bit I2C address */
    uint32_t scl_speed_hz; /* up to 400 kHz */
} ht16k33_config_t;

#define HT16K33_CONFIG_DEFAULT() { .addr = HT16K33_ADDR_DEFAULT, .scl_speed_hz = 400000 }

typedef enum {
    HT16K33_BLINK_OFF    = 0,
    HT16K33_BLINK_2HZ    = 1,
    HT16K33_BLINK_1HZ    = 2,
    HT16K33_BLINK_0_5HZ  = 3,
} ht16k33_blink_t;

/**
 * Probe the chip on an existing bus and start it: oscillator on, ROW15 as a row output,
 * brightness max, RAM cleared, display on, no blink.
 * Returns ESP_ERR_NOT_FOUND if nothing answers at cfg->addr.
 */
esp_err_t ht16k33_new(i2c_master_bus_handle_t bus, const ht16k33_config_t *cfg, ht16k33_handle_t *out);

/** Put the chip in standby (oscillator off), remove it from the bus and free the handle. */
esp_err_t ht16k33_del(ht16k33_handle_t h);

esp_err_t ht16k33_set_brightness(ht16k33_handle_t h, uint8_t level); /* 0..15, clamped */
esp_err_t ht16k33_set_blink(ht16k33_handle_t h, ht16k33_blink_t rate);
esp_err_t ht16k33_set_display(ht16k33_handle_t h, bool on);

/* Frame editing: no I2C traffic until ht16k33_flush(). Out-of-range row/com is ignored. */
void ht16k33_set_led(ht16k33_handle_t h, uint8_t row, uint8_t com, bool on);
bool ht16k33_get_led(ht16k33_handle_t h, uint8_t row, uint8_t com);
void ht16k33_clear(ht16k33_handle_t h);
esp_err_t ht16k33_flush(ht16k33_handle_t h);

/**
 * Read the debounced key matrix. keys[i] bit n = key Kn+1 on scan line KSi (COMi+1).
 * Keys connect a KS line (COM1-COM3) to a K line (ROW3-ROW15) through a diode, anode on COM.
 * The chip clears its key RAM after each read.
 */
esp_err_t ht16k33_read_keys(ht16k33_handle_t h, uint16_t keys[HT16K33_KEY_ROWS]);

#ifdef __cplusplus
}
#endif
