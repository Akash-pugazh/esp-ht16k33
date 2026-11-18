# HT16K33 driver for ESP-IDF

Driver for the Holtek HT16K33, a 16 × 8 LED matrix controller with a 13 × 3 key scanner, built on the ESP-IDF `i2c_master` driver (ESP-IDF ≥ 5.2).

- Shares an I²C bus you create, so it coexists with other sensors on the same bus
- Buffers the whole display in RAM; `ht16k33_flush()` sends it in one 17-byte write
- Brightness (16 levels), blink (off / 2 / 1 / 0.5 Hz), display on/off
- Reads the debounced key matrix
- Includes a 7-segment helper that takes your wiring as data, so common-cathode, common-anode and irregular boards all work without patching the driver

## Add to a project

```yaml
# main/idf_component.yml
dependencies:
  akash-pugazh/ht16k33: "^0.1.0"
```

or `idf.py add-dependency "akash-pugazh/ht16k33^0.1.0"`.

## Use

```c
#include "ht16k33.h"
#include "ht16k33_seg.h"

i2c_master_bus_config_t bus_cfg = {
    .i2c_port = -1, .sda_io_num = 6, .scl_io_num = 7,
    .clk_source = I2C_CLK_SRC_DEFAULT, .glitch_ignore_cnt = 7,
    .flags.enable_internal_pullup = true,
};
i2c_master_bus_handle_t bus;
ESP_ERROR_CHECK(i2c_new_master_bus(&bus_cfg, &bus));

ht16k33_handle_t disp;
ht16k33_config_t cfg = HT16K33_CONFIG_DEFAULT();   /* 0x70, 400 kHz */
ESP_ERROR_CHECK(ht16k33_new(bus, &cfg, &disp));

ht16k33_set_led(disp, 2, 5, true);                 /* ROW2 (A2) x COM5 (C5) */
ESP_ERROR_CHECK(ht16k33_flush(disp));
```

### 7-segment displays

Describe where segments a–g of each digit sit as `{row, com}` pairs. For a common-cathode display with digit *d* on COM *d* and segments on ROW0–6:

```c
#define DIGIT(d) {{{0, d}, {1, d}, {2, d}, {3, d}, {4, d}, {5, d}, {6, d}}}
static const ht16k33_seg_digit_t digits[4] = {DIGIT(0), DIGIT(1), DIGIT(2), DIGIT(3)};

ht16k33_seg_number(disp, digits, 4, -42);   /* " -42", returns false and shows "----" if it does not fit */
ht16k33_seg_text(disp, digits, 4, "Err");
ht16k33_flush(disp);
```

For a common-anode display, wire the digit commons to ROW pins and the segments to COM pins, then swap the pair order (`{d, s}` instead of `{s, d}`). Every entry is independent, so a board where each digit is wired differently needs only a different table.

### Keys

Connect each key between a scan line KS0–KS2 (COM1–COM3) and an input K1–K13 (ROW3–ROW15) through a diode, anode on the COM side. Pins that also drive LEDs keep working; the chip multiplexes scanning with display refresh.

```c
uint16_t keys[HT16K33_KEY_ROWS];
ESP_ERROR_CHECK(ht16k33_read_keys(disp, keys));   /* keys[i] bit n = Kn+1 on KSi */
```

The chip debounces keys (~20 ms) and clears its key memory after each read. Poll every 20–50 ms.

## Hardware notes

- The datasheet rates VDD at 4.5–5.5 V. Most modules also run at 3.3 V with dimmer LEDs. At 5 V, the module's I²C pull-ups lift SDA/SCL to 5 V, which most ESP32 pins do not tolerate; use a level shifter or 3.3 V pull-ups.
- Handles are not thread-safe. Use one task per handle or add your own lock.

## Test

`sh test/host/run.sh` builds and runs the frame and 7-segment tests on a PC with `cc`; no board needed.

## Example

`examples/basic`: lights every LED, blinks, then counts on a common-cathode 4-digit display and prints key presses.

## License

Apache-2.0. See `LICENSE`.
