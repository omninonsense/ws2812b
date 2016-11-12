#include "ws2812b.h"

int WS2812B_init(WS2812B *self, int pin, Colour *pixels, int flags, unsigned int columns, unsigned int rows)
{
  if (digitalPinToPort(pin) == NOT_A_PIN) return 1;

  self->port = portOutputRegister(digitalPinToPort(pin));
  self->bit = digitalPinToBitMask(pin);
  self->pin = pin;
  self->count = columns*rows;
  self->pixels = pixels;
  self->flags = flags;
  self->columns = columns;
  self->rows = rows;

  return 0;
}

void WS2812B_sync(WS2812B *self)
{
  // https://cdn-shop.adafruit.com/datasheets/WS2812B.pdf
  volatile uint16_t count = self->count*3;
  uint8_t *ptr = (uint8_t*)self->pixels; // Next byte ptr (initialized to first here)
  uint8_t byte = *ptr++; // Current byte

  uint8_t hi = *self->port |  self->bit; // Cached high state
  uint8_t lo = *self->port & ~self->bit; // Cached low state

  volatile uint8_t next = lo;
  volatile uint8_t bit = 8;

  if (!count) return;

  cli(); // Disable interrupts

  // @TODO: Go through instruction set to see if we can relax the timing.
  // Note that the C code on the side is pseudo-pseudocode.
  // Some of the C code on the side would get optimsed, while some of it
  // can't be optimsed as effectively, which would significantly skew
  // the signal timing a WS281X chip expects. The timing here is on a very
  // tight schedule anyway, so we need to be careful.
  //
  //
  // See footnotes for extended descriptions.
  asm volatile (
    "__colour_bit:          \n\t"   // __colour_bit:
      "st %a[port], %[hi]   \n\t"   //   *port = hi;
      "sbrc %[byte], 7      \n\t\t" //   if (byte & 0x80)
        "mov %[next], %[hi] \n\t"   //     next = hi;
      "dec %[bit]           \n\t"   //   --bit;
      "st %a[port], %[next] \n\t"   //   *port = next;
      "mov %[next], %[lo]   \n\t"   //   next = lo;
      "breq __colour_byte   \n\t"   //   if (bit == 0) goto __colour_byte; // See [0]
      "lsl %[byte]          \n\t"   //   byte = (byte << 1);
      "nop; nop; nop        \n\t"   //   sleep_cycles(3);
      "st %a[port], %[lo]   \n\t"   //   *port = lo;
      "nop; nop; nop        \n\t"   //   sleep_cycles(3);
      "rjmp __colour_bit    \n"     //   goto __colour_bit;
                                    //
    "__colour_byte:         \n\t"   // __colour_byte:
      "ldi %[bit], 8        \n\t"   //   bit = 8;
      "ld %[byte], %a[ptr]+ \n\t"   //   byte = *ptr++;
      "st %a[port], %[lo]   \n\t"   //   *port = lo;
      "nop                  \n\t"   //   sleep_cycles(1);
      "sbiw %[count], 1     \n\t"   //   --count;
      "brne __colour_bit    \n"     //   if (bit != 0) goto __colour_bit; // See [1]

      // The benefit of extended ASM is that we can define registers and
      // operands elegantly:
    : [port]  "+e" (self->port),
      [count] "+w" (count),
      [bit]   "+r" (bit),
      [byte]  "+r" (byte),
      [next]  "+r" (next)
    : [ptr]    "e" (ptr),
      [hi]     "r" (hi),
      [lo]     "r" (lo)
  );
  // Enable interrupts
  sei();

  // Wait for *at least* 50µs; this causes the chip to latch the voltage onto
  // the individual LEDs contained within the RGB component.
  _delay_us(51);

  /**
   * # 1. `breq __colour_byte`
   * What this effectively does is test whether the Z flag in SREG register is
   * set to `1`, and if so, jumps to the defined symbol (`__colour_byte`).
   *
   * The Z flag (Zero flag) gets set when the previous arithmetic expression
   * evaluated to `0`. In our case, the last arithmetic expression was the
   * subtraction `dec %[bit]` (`--bit`).
   *
   * # 2. `brne __colour_bit`
   * This is similar to `breq`, but tests whether the Z flag in SREG is *not*
   * `1` before jumping. The flag gets cleared after the prevous arithmetic
   * expression evaluates to anything but `0`.
   *
   * ---
   * PS: "expression" is used as a synonym for "instruction"
   *
   * More detailed documentation can be found [here][doclink].
   *
   * [doclink]: http://www.atmel.com/webdoc/avrassembler/avrassembler.wb_instruction_list.html
   */
}

int WS2812B_set_pixel(WS2812B *self, unsigned int x, unsigned int y, Colour c)
{
  return _WS2812B_spf(self, x, y, _WS2812B_cc(c, self->flags & WS2812B_SCHEMES));
}

int WS2812B_set_pixel_at_offset(WS2812B *self, uint16_t offset, Colour c)
{
  if (offset >= self->count) return 1;
  self->pixels[offset] = _WS2812B_cc(c, self->flags & WS2812B_SCHEMES);
  return 0;
}

int WS2812B_set_all(WS2812B *self, Colour c)
{
  Colour _c = _WS2812B_cc(c, self->flags & WS2812B_SCHEMES);
  uint16_t count = self->count;

  while (count) self->pixels[--count] = _c;

  return 0;
}

Colour WS2812B_get_pixel(WS2812B *self, unsigned int x, unsigned int y)
{
  uint16_t offset = 0;
  if (x >= self->columns || y >= self->rows) return {0,0,0};

  if (self->flags & WS2812B_ZIGZAG_ODD && y % 2 != 0)
    x = self->columns - x - 1;

  if (self->flags & WS2812B_ZIGZAG_EVEN && y % 2 == 0)
    x = self->columns - x - 1;

  if(_WS2812B_addr(x, y, self->columns, &offset)) return {0,0,0};
  return self->pixels[offset];
}

int WS2812B_set_row(WS2812B *self, int row, Colour c)
{
  if (row >= self->rows || row < 0) return 1;
  Colour _c = _WS2812B_cc(c, self->flags & WS2812B_SCHEMES);

  for (unsigned int i = 0; i < self->columns; i++)
    _WS2812B_spf(self, i, row, _c);

  return 0;
}

int WS2812B_set_column(WS2812B *self, int col, Colour c)
{
  if (col >= self->columns || col < 0) return 1;
  Colour _c = _WS2812B_cc(c, self->flags & WS2812B_SCHEMES);

  for (unsigned int i = 0; i < self->rows; i++)
    _WS2812B_spf(self, col, i, _c);

  return 0;
}

/* Helper functions */
int _WS2812B_addr(unsigned int x, unsigned int y, unsigned int c, unsigned int *addr)
{
  if (x >= c) return 1;
  *addr = c * y + x;

  return 0;
}

int _WS2812B_spf(WS2812B *self, unsigned int x, unsigned int y, Colour c)
{
  uint16_t offset = 0;
  if (x >= self->columns || y >= self->rows) return 1;

  if (self->flags & WS2812B_ZIGZAG_ODD && y % 2 != 0)
    x = self->columns - x - 1;

  if (self->flags & WS2812B_ZIGZAG_EVEN && y % 2 == 0)
    x = self->columns - x - 1;

  if(_WS2812B_addr(x, y, self->columns, &offset)) return 1;

  self->pixels[offset] = c;
  return 0;
}

unsigned int _WS2812B_x(unsigned int addr, unsigned int c) {
  return addr % c;
}

unsigned int _WS2812B_y(unsigned int addr, int c) {
  return addr / c;
}

Colour _WS2812B_cc(Colour c, int order)
{
  switch (order) {
    case WS2812B_RBG:  return {c.red, c.blue, c.green};
    case WS2812B_GRB:  return {c.green, c.red, c.blue};
    case WS2812B_GBR:  return {c.green, c.blue, c.red};
    case WS2812B_BRG:  return {c.blue, c.red, c.green};
    case WS2812B_BGR:  return {c.blue, c.green, c.red};
    default /* RGB */: return c;
  }
}
