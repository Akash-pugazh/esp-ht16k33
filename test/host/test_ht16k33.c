/*
 * SPDX-FileCopyrightText: 2026 Aakash
 * SPDX-License-Identifier: Apache-2.0
 */
/* Host test: frame layout and 7-segment helper. Run: sh test/host/run.sh */
#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "ht16k33_priv.h"
#include "ht16k33_seg.h"

static struct ht16k33_t dev;
static ht16k33_handle_t h = &dev;

/* Common-cathode style: digit d on COM d, segment s on ROW s. */
static const ht16k33_seg_digit_t CC[4] = {
#define CC_DIGIT(d) {{{0, d}, {1, d}, {2, d}, {3, d}, {4, d}, {5, d}, {6, d}}}
    CC_DIGIT(0), CC_DIGIT(1), CC_DIGIT(2), CC_DIGIT(3),
};

static uint8_t cc_digit(int d) { return dev.ram[2 * d]; } /* ROW0-6 of COM d = abcdefg */

int main(void)
{
    /* Frame layout: byte 2*com holds ROW0-7, byte 2*com+1 holds ROW8-15. */
    ht16k33_clear(h);
    ht16k33_set_led(h, 3, 5, true);
    ht16k33_set_led(h, 12, 0, true);
    uint8_t want[16] = {0};
    want[10] = 1 << 3;
    want[1] = 1 << 4;
    assert(memcmp(dev.ram, want, 16) == 0);
    assert(ht16k33_get_led(h, 3, 5) && !ht16k33_get_led(h, 4, 5));
    ht16k33_set_led(h, 3, 5, false);
    assert(dev.ram[10] == 0);
    ht16k33_set_led(h, 16, 0, true); /* out of range: ignored */
    ht16k33_set_led(h, 0, 8, true);
    assert(!ht16k33_get_led(h, 16, 0) && !ht16k33_get_led(h, 0, 8));
    ht16k33_clear(h);
    assert(memcmp(dev.ram, (uint8_t[16]){0}, 16) == 0);

    /* Font */
    assert(ht16k33_seg_font('8') == 0x7F);
    assert(ht16k33_seg_font('1') == (HT16K33_SEG_B | HT16K33_SEG_C));
    assert(ht16k33_seg_font('E') == ht16k33_seg_font('e'));
    assert(ht16k33_seg_font('-') == HT16K33_SEG_G);
    assert(ht16k33_seg_font(' ') == 0 && ht16k33_seg_font('?') == 0 && ht16k33_seg_font('M') == 0);

    /* seg_set clears segments it does not set */
    ht16k33_seg_set(h, &CC[0], 0x7F);
    ht16k33_seg_set(h, &CC[0], HT16K33_SEG_A);
    assert(cc_digit(0) == HT16K33_SEG_A);

    /* Numbers: right-aligned, blank padding, sign, overflow */
    ht16k33_clear(h);
    assert(ht16k33_seg_number(h, CC, 4, 1707));
    assert(cc_digit(0) == ht16k33_seg_font('1') && cc_digit(1) == ht16k33_seg_font('7') &&
           cc_digit(2) == ht16k33_seg_font('0') && cc_digit(3) == ht16k33_seg_font('7'));
    assert(ht16k33_seg_number(h, CC, 4, 0));
    assert(cc_digit(0) == 0 && cc_digit(1) == 0 && cc_digit(2) == 0 && cc_digit(3) == ht16k33_seg_font('0'));
    assert(ht16k33_seg_number(h, CC, 4, -17));
    assert(cc_digit(0) == 0 && cc_digit(1) == HT16K33_SEG_G && cc_digit(3) == ht16k33_seg_font('7'));
    assert(ht16k33_seg_number(h, CC, 4, -999));
    assert(!ht16k33_seg_number(h, CC, 4, 10000));
    for (int d = 0; d < 4; d++) assert(cc_digit(d) == HT16K33_SEG_G);
    assert(!ht16k33_seg_number(h, CC, 4, -1000));

    /* Text: left-aligned, short text blanks the rest, long text truncated */
    ht16k33_seg_text(h, CC, 4, "Er");
    assert(cc_digit(0) == ht16k33_seg_font('E') && cc_digit(1) == ht16k33_seg_font('r') && cc_digit(2) == 0 && cc_digit(3) == 0);
    ht16k33_seg_text(h, CC, 4, "HELLO");
    assert(cc_digit(3) == ht16k33_seg_font('L'));
    ht16k33_seg_text(h, CC, 4, NULL);
    for (int d = 0; d < 4; d++) assert(cc_digit(d) == 0);

    /* Segments touch only their own LEDs */
    ht16k33_clear(h);
    ht16k33_set_led(h, 7, 0, true); /* e.g. a decimal point on ROW7 */
    ht16k33_seg_number(h, CC, 4, 8888);
    assert(ht16k33_get_led(h, 7, 0));

    puts("test_ht16k33: ok");
    return 0;
}
