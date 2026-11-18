/*
 * SPDX-FileCopyrightText: 2026 Aakash
 * SPDX-License-Identifier: Apache-2.0
 */
/*
 * 7-segment helper for HT16K33. The caller describes where each segment sits as a (row, com)
 * pair, so any wiring works: common-cathode digits on COM lines, common-anode digits on ROW
 * lines, or a board where every digit is wired differently.
 */
#pragma once

#include <stddef.h>

#include "ht16k33.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Segment bits in the usual abcdefg order. */
#define HT16K33_SEG_A (1 << 0) /* top */
#define HT16K33_SEG_B (1 << 1) /* top right */
#define HT16K33_SEG_C (1 << 2) /* bottom right */
#define HT16K33_SEG_D (1 << 3) /* bottom */
#define HT16K33_SEG_E (1 << 4) /* bottom left */
#define HT16K33_SEG_F (1 << 5) /* top left */
#define HT16K33_SEG_G (1 << 6) /* middle */

typedef struct {
    uint8_t row;
    uint8_t com;
} ht16k33_led_t;

/* Position of segments a..g of one digit. */
typedef struct {
    ht16k33_led_t seg[7];
} ht16k33_seg_digit_t;

/** Segment mask for a character: 0-9, A-Z/a-z where readable, '-', '_', ' '. Unknown -> 0 (blank). */
uint8_t ht16k33_seg_font(char c);

/** Set one digit's segments (abcdefg mask) in the frame. */
void ht16k33_seg_set(ht16k33_handle_t h, const ht16k33_seg_digit_t *digit, uint8_t segs);

/** Write text left-aligned across n digits; extra digits blank, extra characters dropped. */
void ht16k33_seg_text(ht16k33_handle_t h, const ht16k33_seg_digit_t *digits, size_t n, const char *text);

/** Write an integer right-aligned. If it does not fit, fills with '-' and returns false. */
bool ht16k33_seg_number(ht16k33_handle_t h, const ht16k33_seg_digit_t *digits, size_t n, int value);

#ifdef __cplusplus
}
#endif
