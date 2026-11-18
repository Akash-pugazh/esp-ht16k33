/*
 * SPDX-FileCopyrightText: 2026 Aakash
 * SPDX-License-Identifier: Apache-2.0
 */
#include "ht16k33_seg.h"

#include <stdio.h>

#define A HT16K33_SEG_A
#define B HT16K33_SEG_B
#define C HT16K33_SEG_C
#define D HT16K33_SEG_D
#define E HT16K33_SEG_E
#define F HT16K33_SEG_F
#define G HT16K33_SEG_G

static const uint8_t DIGITS[10] = {
    A | B | C | D | E | F, B | C, A | B | D | E | G, A | B | C | D | G, B | C | F | G,
    A | C | D | F | G, A | C | D | E | F | G, A | B | C, A | B | C | D | E | F | G, A | B | C | D | F | G,
};

/* Letters A-Z; 0 where a 7-segment shape would be misread (K, M, V, W, X). */
static const uint8_t LETTERS[26] = {
    A | B | C | E | F | G, /* A */ C | D | E | F | G,     /* b */ A | D | E | F,     /* C */
    B | C | D | E | G,     /* d */ A | D | E | F | G,     /* E */ A | E | F | G,     /* F */
    A | C | D | E | F,     /* G */ B | C | E | F | G,     /* H */ E | F,             /* I */
    B | C | D | E,         /* J */ 0,                     /* K */ D | E | F,         /* L */
    0,                     /* M */ C | E | G,             /* n */ C | D | E | G,     /* o */
    A | B | E | F | G,     /* P */ A | B | C | F | G,     /* q */ E | G,             /* r */
    A | C | D | F | G,     /* S */ D | E | F | G,         /* t */ B | C | D | E | F, /* U */
    0,                     /* V */ 0,                     /* W */ 0,                 /* X */
    B | C | D | F | G,     /* y */ A | B | D | E | G,     /* Z */
};

uint8_t ht16k33_seg_font(char c)
{
    if (c >= '0' && c <= '9') return DIGITS[c - '0'];
    if (c >= 'a' && c <= 'z') c = (char)(c - 'a' + 'A');
    if (c >= 'A' && c <= 'Z') return LETTERS[c - 'A'];
    if (c == '-') return G;
    if (c == '_') return D;
    return 0;
}

void ht16k33_seg_set(ht16k33_handle_t h, const ht16k33_seg_digit_t *digit, uint8_t segs)
{
    for (int s = 0; s < 7; s++) ht16k33_set_led(h, digit->seg[s].row, digit->seg[s].com, segs & (1 << s));
}

void ht16k33_seg_text(ht16k33_handle_t h, const ht16k33_seg_digit_t *digits, size_t n, const char *text)
{
    for (size_t i = 0; i < n; i++) {
        uint8_t segs = (text && *text) ? ht16k33_seg_font(*text++) : 0;
        ht16k33_seg_set(h, &digits[i], segs);
    }
}

bool ht16k33_seg_number(ht16k33_handle_t h, const ht16k33_seg_digit_t *digits, size_t n, int value)
{
    char buf[16];
    int len = snprintf(buf, sizeof buf, "%*d", (int)n, value);
    bool fits = n < sizeof buf && len == (int)n;
    if (!fits) {
        for (size_t i = 0; i < n; i++) ht16k33_seg_set(h, &digits[i], G);
        return false;
    }
    ht16k33_seg_text(h, digits, n, buf);
    return true;
}
