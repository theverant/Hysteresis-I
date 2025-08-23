/*
  LED Hysteresis Lighting System
  2025-07-30
  Version: 0.16
  Code by Theverant with essential help from ChatGPT
  License: GPLv3

  Hardware:
    - Microcontroller (e.g., Arduino Uno)
    - 3 WS2812-compatible RGB LEDs

  Pin Hookup:
    DATA_PIN -> WS2812 DIN
    VCC      -> 5V
    GND      -> GND

  Changelog:
    - v0.16: Adjusted fade ranges to reduce pastel tones
    - v0.14 and earlier: Developed hysteresis-controlled RGB transitions
*/

#include <FastLED.h>

#define NUM_LEDS 3
#define DATA_PIN 6
#define COLOR_CHANGE_SPEED 5  // Increase for faster transitions
#define FADE_STEP 3           // Intensity change per update

CRGB leds[NUM_LEDS];

// Hysteresis curve simulation parameters
float domain_state[NUM_LEDS] = {0.0, 0.0, 0.0};
float domain_velocity[NUM_LEDS] = {0.02, 0.017, 0.023};

// Color palette (high saturation only)
const CRGB palette[] = {
  CRGB::Red, CRGB::Green, CRGB::Blue,
  CRGB::Yellow, CRGB::Cyan, CRGB::Magenta
};

const int paletteSize = sizeof(palette) / sizeof(CRGB);
int currentColorIndex[NUM_LEDS] = {0, 2, 4};
int nextColorIndex[NUM_LEDS] = {1, 3, 5};
uint8_t crossfadeStep[NUM_LEDS] = {0, 0, 0};

uint8_t vivid(uint8_t value) {
  return map(value, 0, 255, 100, 255);
}

CRGB vividColor(const CRGB& color) {
  return CRGB(vivid(color.r), vivid(color.g), vivid(color.b));
}

void updateDomain(int i) {
  // Simple hysteresis-like bounce
  domain_state[i] += domain_velocity[i];
  if (domain_state[i] > 1.0 || domain_state[i] < 0.0) {
    domain_velocity[i] = -domain_velocity[i];
    domain_state[i] += domain_velocity[i];
  }
}

void crossfadeColor(int i) {
  CRGB from = vividColor(palette[currentColorIndex[i]]);
  CRGB to   = vividColor(palette[nextColorIndex[i]]);

  uint8_t t = crossfadeStep[i];
  leds[i] = blend(from, to, t);

  if (t < 255) {
    crossfadeStep[i] += COLOR_CHANGE_SPEED;
  } else {
    crossfadeStep[i] = 0;
    currentColorIndex[i] = nextColorIndex[i];
    nextColorIndex[i] = (nextColorIndex[i] + 2) % paletteSize;
  }
}

void setup() {
  FastLED.addLeds<WS2812, DATA_PIN, GRB>(leds, NUM_LEDS);
  FastLED.clear();
  FastLED.show();
}

void loop() {
  for (int i = 0; i < NUM_LEDS; i++) {
    updateDomain(i);
    crossfadeColor(i);
  }

  FastLED.show();
  delay(20);  // Control overall speed
}

