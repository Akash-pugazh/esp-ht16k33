/*
 * SPDX-FileCopyrightText: 2026 Aakash
 * SPDX-License-Identifier: Apache-2.0
 */
/* Frame buffer editing. No I2C here, so it builds and tests on a PC. */
#include <string.h>

#include "ht16k33_priv.h"

void ht16k33_set_led(ht16k33_handle_t h, uint8_t row, uint8_t com, bool on)
{
    if (row >= HT16K33_ROWS || com >= HT16K33_COMS) return;
    uint8_t *b = &h->ram[2 * com + row / 8];
    uint8_t bit = (uint8_t)(1 << (row % 8));
    *b = on ? (*b | bit) : (*b & (uint8_t)~bit);
}

bool ht16k33_get_led(ht16k33_handle_t h, uint8_t row, uint8_t com)
{
    if (row >= HT16K33_ROWS || com >= HT16K33_COMS) return false;
    return h->ram[2 * com + row / 8] & (1 << (row % 8));
}

void ht16k33_clear(ht16k33_handle_t h)
{
    memset(h->ram, 0, sizeof h->ram);
}
