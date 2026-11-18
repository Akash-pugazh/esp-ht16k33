/*
 * SPDX-FileCopyrightText: 2026 Aakash
 * SPDX-License-Identifier: Apache-2.0
 */
#include "ht16k33.h"
#include "ht16k33_priv.h"

#include <stdlib.h>
#include <string.h>

#include "esp_check.h"

#define TIMEOUT_MS 50

/* Commands, datasheet v1.10 */
#define CMD_SYSTEM   0x20 /* | 1 = oscillator on */
#define CMD_DISPLAY  0x80 /* | blink << 1 | on */
#define CMD_ROW_INT  0xA0 /* | 0 = ROW15 is a row output */
#define CMD_DIMMING  0xE0 /* | level */
#define ADDR_RAM     0x00
#define ADDR_KEYS    0x40

static const char *TAG = "ht16k33";

static esp_err_t cmd(ht16k33_handle_t h, uint8_t c)
{
    return i2c_master_transmit(h->dev, &c, 1, TIMEOUT_MS);
}

static esp_err_t display_cmd(ht16k33_handle_t h)
{
    return cmd(h, CMD_DISPLAY | (uint8_t)(h->blink << 1) | (h->on ? 1 : 0));
}

esp_err_t ht16k33_new(i2c_master_bus_handle_t bus, const ht16k33_config_t *cfg, ht16k33_handle_t *out)
{
    ESP_RETURN_ON_FALSE(bus && cfg && out, ESP_ERR_INVALID_ARG, TAG, "null argument");
    ESP_RETURN_ON_ERROR(i2c_master_probe(bus, cfg->addr, TIMEOUT_MS), TAG, "no HT16K33 at 0x%02X", cfg->addr);

    ht16k33_handle_t h = calloc(1, sizeof *h);
    ESP_RETURN_ON_FALSE(h, ESP_ERR_NO_MEM, TAG, "no memory");
    i2c_device_config_t dev_cfg = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = cfg->addr,
        .scl_speed_hz = cfg->scl_speed_hz,
    };
    esp_err_t err = i2c_master_bus_add_device(bus, &dev_cfg, &h->dev);
    if (err) {
        free(h);
        return err;
    }
    h->on = true;
    if ((err = cmd(h, CMD_SYSTEM | 1)) || (err = cmd(h, CMD_ROW_INT)) ||
        (err = ht16k33_set_brightness(h, HT16K33_BRIGHTNESS_MAX)) || (err = ht16k33_flush(h)) ||
        (err = display_cmd(h))) {
        i2c_master_bus_rm_device(h->dev);
        free(h);
        return err;
    }
    *out = h;
    return ESP_OK;
}

esp_err_t ht16k33_del(ht16k33_handle_t h)
{
    ESP_RETURN_ON_FALSE(h, ESP_ERR_INVALID_ARG, TAG, "null handle");
    cmd(h, CMD_SYSTEM); /* standby; ignore failure, we are tearing down anyway */
    esp_err_t err = i2c_master_bus_rm_device(h->dev);
    free(h);
    return err;
}

esp_err_t ht16k33_set_brightness(ht16k33_handle_t h, uint8_t level)
{
    return cmd(h, CMD_DIMMING | (level > HT16K33_BRIGHTNESS_MAX ? HT16K33_BRIGHTNESS_MAX : level));
}

esp_err_t ht16k33_set_blink(ht16k33_handle_t h, ht16k33_blink_t rate)
{
    h->blink = rate & 3;
    return display_cmd(h);
}

esp_err_t ht16k33_set_display(ht16k33_handle_t h, bool on)
{
    h->on = on;
    return display_cmd(h);
}

esp_err_t ht16k33_flush(ht16k33_handle_t h)
{
    uint8_t buf[1 + sizeof h->ram] = {ADDR_RAM};
    memcpy(buf + 1, h->ram, sizeof h->ram);
    return i2c_master_transmit(h->dev, buf, sizeof buf, TIMEOUT_MS);
}

esp_err_t ht16k33_read_keys(ht16k33_handle_t h, uint16_t keys[HT16K33_KEY_ROWS])
{
    uint8_t reg = ADDR_KEYS, raw[2 * HT16K33_KEY_ROWS];
    ESP_RETURN_ON_ERROR(i2c_master_transmit_receive(h->dev, &reg, 1, raw, sizeof raw, TIMEOUT_MS), TAG, "key read");
    for (int i = 0; i < HT16K33_KEY_ROWS; i++) keys[i] = (uint16_t)((raw[2 * i] | raw[2 * i + 1] << 8) & 0x1FFF);
    return ESP_OK;
}
