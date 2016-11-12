/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2016 Nino Miletich
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#ifndef __WS2812B_H__
#define __WS2812B_H__
#include <inttypes.h>
#include "Arduino.h"

// Should be defined in Arduino.h
// We define these to supress warnings and errors
#ifndef Arduino_h

#warning Seems like Arduino.h is not included; this will cause issues!

#define NOT_A_PIN 0

#define digitalPinToPort(x)    (x)
#define digitalPinToBitMask(x) (x)
#define portOutputRegister(x)  ((volatile uint8_t*)x)

#define cli()
#define sei()
#define _delay_us(x) ((volatile int)x)
#endif // Arduino_h

#ifndef MIN
#define MIN(a,b) (((a)<(b))?(a):(b))
#endif

#ifndef MAX
#define MAX(a,b) (((a)>(b))?(a):(b))
#endif

#define WS2812B_ZIGZAG_ODD  (1 << 0)
#define WS2812B_ZIGZAG_EVEN (1 << 1)

#define WS2812B_RGB (1 << 2)
#define WS2812B_RBG (1 << 3)
#define WS2812B_GRB (1 << 4)
#define WS2812B_GBR (1 << 5)
#define WS2812B_BRG (1 << 6)
#define WS2812B_BGR (1 << 7)

#define WS2812B_SCHEMES (WS2812B_RGB|WS2812B_RBG|WS2812B_GRB|WS2812B_GBR|WS2812B_BRG|WS2812B_BGR)

#ifndef __COLOUR_H__
typedef struct {
  uint8_t red;
  uint8_t green;
  uint8_t blue;
} Colour;
#endif

typedef struct {
  char id;
  Colour col;
} ColourPalette;

typedef struct {
  volatile uint8_t *port;
  uint8_t bit;
  int pin;
  uint16_t count;
  Colour *pixels;
  int flags;
  unsigned int columns;
  unsigned int rows;
  unsigned int palette_size;
  unsigned int palette_used;
  ColourPalette *palette;
} WS2812B;

int WS2812B_init(WS2812B *self, int pin, Colour *pixels, int flags, unsigned int columns, unsigned int rows);
void WS2812B_sync(WS2812B *self);

int WS2812B_init_palette(WS2812B *self, ColourPalette* palette, unsigned int size);
int WS2812B_add_colour(WS2812B *self, char id, Colour c);

int WS2812B_set_pixel(WS2812B *self, unsigned int x, unsigned int y, Colour c);
int WS2812B_set_all(WS2812B *self, Colour c);
int WS2812B_set_pixel_at_offset(WS2812B *self, uint16_t offset, Colour c);
Colour WS2812B_get_pixel(WS2812B *self, unsigned int x, unsigned int y);
int WS2812B_set_row(WS2812B *self, unsigned int row, Colour c);
int WS2812B_set_column(WS2812B *self, unsigned int col, Colour c);
int WS2812B_render(WS2812B *self, const char *ascii_art);

// Helper functions
int _WS2812B_addr(unsigned int x, unsigned int y, unsigned int c, unsigned int *addr); // get addr of (x,y) inside flat array; dereferencing twice is slower
int _WS2812B_spf(WS2812B *self, unsigned int x, unsigned int y, Colour c); // set pixel fast
unsigned int _WS2812B_x(unsigned int addr, unsigned int c); // get x
unsigned int _WS2812B_y(unsigned int addr, unsigned int c); // get y
Colour _WS2812B_lupc(WS2812B *self, char id); // look up palette colour

Colour _WS2812B_cc(Colour c, int order); // convert for output

#endif
