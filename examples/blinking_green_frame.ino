#include "colour.h"
#include "ws2812b.h"

#define DATA_PIN 13

#define COLS 6
#define ROWS 5
#define COUNT (COLS*ROWS)

WS2812B driver;
Colour matrix[COLS*ROWS];

void setup()
{
  Serial.begin(9600);
  pinMode(DATA_PIN, OUTPUT);

  if (WS2812B_init(&driver, DATA_PIN, matrix, WS2812B_ZIGZAG_EVEN|WS2812B_GRB, COLS, ROWS)) {
    Serial.println("Error intiailising driver");
    /* Error handling goes here. */
  } else {
    Serial.println("WS2812B Driver loaded");
  }

  // Set all LEDs to black (off)
  WS2812B_set_all(&driver, {0,0,0});
  WS2812B_sync(&driver);
}

void loop()
{
    Colour c = colour_hsl(90, 100, 5);
    WS2812B_set_row(&driver, 0, c);
    WS2812B_set_row(&driver, 4, c);
    WS2812B_set_column(&driver, 0, c);
    WS2812B_set_column(&driver, 5, c);
    WS2812B_sync(&driver);

    delay(500);

    WS2812B_set_all(&driver, COLOUR_BLACK);
    WS2812B_sync(&driver);
    delay(500);
}
