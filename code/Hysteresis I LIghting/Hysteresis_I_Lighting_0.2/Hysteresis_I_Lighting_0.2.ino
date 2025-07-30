/*
  WS2812 Solid Red Test with Adafruit NeoPixel
  July 30, 2025
  Code by Theverant with essential help from ChatGPT
  License: GPLv3
*/

#include <Adafruit_NeoPixel.h>

#define LED_PIN    6
#define NUM_LEDS   3

Adafruit_NeoPixel strip(NUM_LEDS, LED_PIN, NEO_GRB + NEO_KHZ800);

void setup() {
  strip.begin();
  strip.show(); // Initialize all pixels to 'off'
  strip.setBrightness(128);
  for (int i = 0; i < NUM_LEDS; i++) {
    strip.setPixelColor(i, strip.Color(255, 0, 0)); // Red color
  }
  strip.show();
}

void loop() {
  // Solid red, no changes
}



