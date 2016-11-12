#include "colour.h"
#include "ws2812b.h"

#define DATA_PIN 13

#define COLS 6
#define ROWS 5
#define COUNT (COLS*ROWS)
#define PALETTE_SIZE 5

#define LIGHTNESS 5

WS2812B driver;
Colour matrix[COLS*ROWS];
ColourPalette palette[PALETTE_SIZE];

void setup()
{
  Serial.begin(9600);
  pinMode(DATA_PIN, OUTPUT);

  if (WS2812B_init(&driver, DATA_PIN, matrix, WS2812B_ZIGZAG_EVEN|WS2812B_GRB, COLS, ROWS)) {
    Serial.println("Error intiailising driver");
    /* Error handling goes here. */
  } else {
    Serial.println("WS2812B Driver loaded");
    WS2812B_init_palette(&driver, palette, PALETTE_SIZE);

    WS2812B_add_colour(&driver, ' ', COLOUR_BLACK);
    WS2812B_add_colour(&driver, 'N', colour_hsl(  0, 100, LIGHTNESS));
    WS2812B_add_colour(&driver, 'i', colour_hsl( 50, 100, LIGHTNESS));
    WS2812B_add_colour(&driver, 'n', colour_hsl(100, 100, LIGHTNESS));
    WS2812B_add_colour(&driver, 'o', colour_hsl(150, 100, LIGHTNESS));
  }

  // Set all LEDs to black (off)
  WS2812B_set_all(&driver, {0,0,0});
  WS2812B_sync(&driver);
}

// This spells "Nino" in a loop on a 6x5 grid.
void loop()
{
  WS2812B_render(&driver,
    ///////|
    "N   N "
    "NN  N "
    "N N N "
    "N  NN "
    "N   N "
  );
  WS2812B_sync(&driver);
  delay(500);

  WS2812B_render(&driver,
    ///////|
    "  i   "
    "      "
    " iii  "
    "  i   "
    " iii  "
  );
  WS2812B_sync(&driver);
  delay(500);

  WS2812B_render(&driver,
    ///////|
    "      "
    "n nn  "
    "nn  n "
    "n   n "
    "n   n "
  );
  WS2812B_sync(&driver);
  delay(500);

  WS2812B_render(&driver,
    ///////|
    "      "
    " ooo  "
    "o   o "
    "o   o "
    " ooo  "
  );
  WS2812B_sync(&driver);
  delay(500);
}
