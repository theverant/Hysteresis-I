/*
 * Hysteresis - Magnetic Memory Lighting Controller
 * 2025-07-30
 * Version 0.5
 * Code by Theverant with essential help from ChatGPT
 * License: GPLv3
 *
 * Changelog:
 * - Crossfades colors with randomized fade durations.
 * - Rotates color assignments between physical LEDs to cycle colors through LEDs.
 */

#include <FastLED.h>

#define LED_PIN 6
#define NUM_LEDS 3
#define BRIGHTNESS 150

CRGB leds[NUM_LEDS];

unsigned long lastChange = 0;
unsigned long currentCrossfadeDuration = 0;
float progress = 0.0;

// Colors to cycle through
CRGB baseColors[3] = {CRGB::Red, CRGB::Green, CRGB::Blue};

// Rotation offset for which color is assigned to which LED
int colorRotation = 0;

void setup() {
  FastLED.addLeds<WS2812, LED_PIN, GRB>(leds, NUM_LEDS);
  FastLED.setBrightness(BRIGHTNESS);
  
  randomSeed(analogRead(A0));
  currentCrossfadeDuration = random(100, 500);
  lastChange = millis();

  fill_solid(leds, NUM_LEDS, CRGB::Black);
  FastLED.show();
}

void loop() {
  unsigned long currentMillis = millis();
  unsigned long elapsed = currentMillis - lastChange;
  progress = float(elapsed) / currentCrossfadeDuration;
  if (progress > 1.0) progress = 1.0;

  for (int i = 0; i < NUM_LEDS; i++) {
    // Current color index for this LED
    int currentColorIndex = (colorRotation + i) % NUM_LEDS;
    // Next color index for this LED
    int nextColorIndex = (colorRotation + i + 1) % NUM_LEDS;

    // Crossfade color for this LED between currentColor and nextColor
    leds[i] = blend(baseColors[currentColorIndex], baseColors[nextColorIndex], uint8_t(progress * 255));
  }

  FastLED.show();

  if (elapsed >= currentCrossfadeDuration) {
    lastChange = currentMillis;
    currentCrossfadeDuration = random(100, 500);
    colorRotation = (colorRotation + 1) % NUM_LEDS;
  }
}
