/*
 * Hysteresis - Magnetic Memory Lighting Controller
 * 2025-07-30
 * Version 0.13
 * Code by Theverant with essential help from ChatGPT
 * License: GPLv3
 *
 * Changelog:
 * - Fade speed for each LED cycles smoothly between slow and fast, each at different rates
 * - Fade speeds modulated by hysteresis-like sine curves to simulate natural magnetic behavior
 * - Maintains color rotation and independent brightness fades per LED
 */

#include <FastLED.h>

#define LED_PIN 6
#define NUM_LEDS 3
#define BRIGHTNESS 150

CRGB leds[NUM_LEDS];

// Hysteresis fading pattern (16 steps)
const uint8_t PATTERN_LENGTH = 16;
const float hysteresisPattern[PATTERN_LENGTH] = {
  0.00, 0.05, 0.10, 0.20, 0.30, 0.45, 0.60, 0.75,
  1.00, 0.75, 0.60, 0.45, 0.30, 0.20, 0.10, 0.05
};

// Magnetic domain base colors cycling through LEDs
CRGB magneticColors[NUM_LEDS] = {CRGB::Red, CRGB::Green, CRGB::Blue};

// Base fade speed bounds (ms)
const unsigned long FADE_SPEED_MIN = 50;
const unsigned long FADE_SPEED_MAX = 250;

// Phase offsets for each LED's speed modulation (to cycle differently)
const float phaseOffsets[NUM_LEDS] = {0.0, 2.1, 4.2};

unsigned long lastStepTimes[NUM_LEDS] = {0, 0, 0};
uint8_t patternIndices[NUM_LEDS] = {0, 5, 10};

uint8_t magneticRotation = 0;

void setup() {
  FastLED.addLeds<WS2812, LED_PIN, GRB>(leds, NUM_LEDS);
  FastLED.setBrightness(BRIGHTNESS);
  fill_solid(leds, NUM_LEDS, CRGB::Black);
  FastLED.show();
}

unsigned long computeFadeSpeed(unsigned long currentMillis, uint8_t ledIndex) {
  // Normalize time for smooth oscillation (cycles approx every 5 seconds)
  float t = (currentMillis % 5000) / 5000.0f;
  
  // Phase-shifted sine wave to simulate hysteresis-like cycle (0 to 1)
  float sineVal = 0.5f + 0.5f * sinf(2 * 3.14159f * t + phaseOffsets[ledIndex]);

  // Map sine output to fade speed range
  return FADE_SPEED_MIN + (unsigned long)((FADE_SPEED_MAX - FADE_SPEED_MIN) * sineVal);
}

void loop() {
  unsigned long currentMillis = millis();

  for (uint8_t i = 0; i < NUM_LEDS; i++) {
    unsigned long fadeSpeed = computeFadeSpeed(currentMillis, i);
    
    if (currentMillis - lastStepTimes[i] >= fadeSpeed) {
      lastStepTimes[i] = currentMillis;

      patternIndices[i] = (patternIndices[i] + 1) % PATTERN_LENGTH;

      uint8_t colorIndex = (i + magneticRotation) % NUM_LEDS;

      float brightnessFactor = hysteresisPattern[patternIndices[i]];

      CRGB baseColor = magneticColors[colorIndex];
      leds[i] = CRGB(
        uint8_t(baseColor.r * brightnessFactor),
        uint8_t(baseColor.g * brightnessFactor),
        uint8_t(baseColor.b * brightnessFactor)
      );
    }
  }

  // Rotate colors every 60ms
  static unsigned long lastRotation = 0;
  if (currentMillis - lastRotation >= 60) {
    lastRotation = currentMillis;
    magneticRotation = (magneticRotation + 1) % NUM_LEDS;
  }

  FastLED.show();
}
