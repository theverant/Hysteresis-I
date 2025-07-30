/*
 * Hysteresis - Magnetic Memory Lighting Controller
 * 2025-07-30
 * Version 0.4
 * Code by Theverant with essential help from ChatGPT
 * License: GPLv3
 *
 * Changelog:
 * - Added randomized crossfade duration between 100ms and 500ms for LED color transitions.
 * - Updated fading logic to use variable crossfade duration per transition.
 */

#include <FastLED.h>

#define LED_PIN 6
#define NUM_LEDS 3
#define BRIGHTNESS 150

CRGB leds[NUM_LEDS];

unsigned long lastChange = 0;
int ledIndex = 0;

// Randomized crossfade duration (ms)
unsigned long currentCrossfadeDuration = 0;

void setup() {
  FastLED.addLeds<WS2812, LED_PIN, GRB>(leds, NUM_LEDS);
  FastLED.setBrightness(BRIGHTNESS);

  // Initialize random seed (use analog noise)
  randomSeed(analogRead(A0));

  // Start with a random crossfade duration
  currentCrossfadeDuration = random(100, 500);
  lastChange = millis();
  fill_solid(leds, NUM_LEDS, CRGB::Black);
  FastLED.show();
}

void loop() {
  unsigned long currentMillis = millis();
  unsigned long elapsed = currentMillis - lastChange;

  float progress = float(elapsed) / currentCrossfadeDuration;
  if (progress > 1.0) progress = 1.0;

  // Current LED color (rotating RGB)
  CRGB currentColor;
  switch (ledIndex % 3) {
    case 0: currentColor = CRGB::Red; break;
    case 1: currentColor = CRGB::Green; break;
    case 2: currentColor = CRGB::Blue; break;
  }

  // Next LED color (rotating RGB)
  CRGB nextColor;
  switch ((ledIndex + 1) % 3) {
    case 0: nextColor = CRGB::Red; break;
    case 1: nextColor = CRGB::Green; break;
    case 2: nextColor = CRGB::Blue; break;
  }

  // Crossfade between current and next LED
  for (int i = 0; i < NUM_LEDS; i++) {
    if (i == ledIndex) {
      leds[i] = currentColor;
      leds[i].fadeToBlackBy(uint8_t(progress * 255));
    } else if (i == (ledIndex + 1) % NUM_LEDS) {
      leds[i] = nextColor;
      leds[i].fadeToBlackBy(uint8_t((1.0 - progress) * 255));
    } else {
      leds[i] = CRGB::Black;
    }
  }

  FastLED.show();

  if (elapsed >= currentCrossfadeDuration) {
    lastChange = currentMillis;
    ledIndex = (ledIndex + 1) % NUM_LEDS;
    currentCrossfadeDuration = random(100, 500);
  }
}
