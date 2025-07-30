/*
 * Hysteresis - Magnetic Memory Lighting Controller
 * 2025-07-30
 * Version 0.12
 * Code by Theverant with essential help from ChatGPT
 * License: GPLv3
 *
 * Changelog:
 * - Increased color rotation speed ×5 (color rotate every 60ms)
 * - Color changes and fades follow hysteresis pattern steps for smoother, natural transitions
 * - Maintained staggered fade timing per LED for desynchronization
 */

#include <FastLED.h>

#define LED_PIN 6
#define NUM_LEDS 3
#define BRIGHTNESS 150

CRGB leds[NUM_LEDS];

// Hysteresis fading pattern (16 steps for smooth fade)
const uint8_t PATTERN_LENGTH = 16;
const float hysteresisPattern[PATTERN_LENGTH] = {
  0.00, 0.05, 0.10, 0.20, 0.30, 0.45, 0.60, 0.75,
  1.00, 0.75, 0.60, 0.45, 0.30, 0.20, 0.10, 0.05
};

// Magnetic domain base colors cycling through LEDs
CRGB magneticColors[NUM_LEDS] = {CRGB::Red, CRGB::Green, CRGB::Blue};

// Different step durations per LED (in ms) for desynchronized fades
const unsigned long ledStepDurations[NUM_LEDS] = {100, 140, 180};

unsigned long lastStepTimes[NUM_LEDS] = {0, 0, 0};
uint8_t patternIndices[NUM_LEDS] = {0, 5, 10};  // Offset start indices for variation

// Color rotation simulating magnetic domain rotation
uint8_t magneticRotation = 0;

void setup() {
  FastLED.addLeds<WS2812, LED_PIN, GRB>(leds, NUM_LEDS);
  FastLED.setBrightness(BRIGHTNESS);
  fill_solid(leds, NUM_LEDS, CRGB::Black);
  FastLED.show();
}

void loop() {
  unsigned long currentMillis = millis();

  // Update each LED independently
  for (uint8_t i = 0; i < NUM_LEDS; i++) {
    if (currentMillis - lastStepTimes[i] >= ledStepDurations[i]) {
      lastStepTimes[i] = currentMillis;

      // Advance pattern index with wrap-around
      patternIndices[i] = (patternIndices[i] + 1) % PATTERN_LENGTH;

      // Determine rotated color index based on magneticRotation
      uint8_t colorIndex = (i + magneticRotation) % NUM_LEDS;

      // Get brightness factor from hysteresis pattern
      float brightnessFactor = hysteresisPattern[patternIndices[i]];

      // Base color scaled by brightness factor
      CRGB baseColor = magneticColors[colorIndex];
      leds[i] = CRGB(
        uint8_t(baseColor.r * brightnessFactor),
        uint8_t(baseColor.g * brightnessFactor),
        uint8_t(baseColor.b * brightnessFactor)
      );
    }
  }

  // Rotate colors ×5 faster: every 60ms instead of 300ms
  static unsigned long lastRotation = 0;
  if (currentMillis - lastRotation >= 60) {
    lastRotation = currentMillis;
    magneticRotation = (magneticRotation + 1) % NUM_LEDS;
  }

  FastLED.show();
}
