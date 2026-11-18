/*
 * SPDX-FileCopyrightText: 2026 Aakash
 * SPDX-License-Identifier: Apache-2.0
 */
/* Private: handle layout shared by ht16k33.c (I2C) and ht16k33_frame.c (pure frame logic). */
#pragma once

#include "ht16k33.h"

struct ht16k33_t {
    i2c_master_dev_handle_t dev;
    uint8_t ram[HT16K33_COMS * 2]; /* byte 2c = ROW0-7 of COM c, byte 2c+1 = ROW8-15 */
    bool on;
    ht16k33_blink_t blink;
};
