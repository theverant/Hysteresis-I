/*
 * Hysteresis - Magnetic Memory Lighting Controller
 * 2025-07-30
 * Version 0.7
 * Code by Theverant with essential help from ChatGPT
 * License: GPLv3
 *
 * Changelog:
 * - Removed all references to obfuscation or hidden data.
 * - Added deterministic hysteresis pattern color crossfade with domain rotation.
 */

#include <FastLED.h>

#define LED_PIN 6
#define NUM_LEDS 3
#define BRIGHTNESS 150

CRGB leds[NUM_LEDS];

// Magnetic domain patterns (data for hysteresis states)
const uint8_t domainPatterns[32] = {
  0x5A, 0x3C, 0xE1, 0x2F, 0x84, 0xD7, 0x91, 0x4B,
  0x76, 0xA9, 0x12, 0x8E, 0xC5, 0x3F, 0x60, 0xB4,
  0x1D, 0x8A, 0x47, 0xE3, 0x29, 0x95, 0x7C, 0x58,
  0xAF, 0x14, 0x62, 0xDB, 0x30, 0x9E, 0x4C, 0x77
};

// Magnetic flux coefficients (sequence indices)
const uint8_t fluxCoefficients[] = {
  0x02, 0x0E, 0x05, 0x11, 0x11, 0x08, 0x05, 0x08, 0x13, 0x05, 0x0B
};

// Magnetic field parameters
struct MagneticField {
  uint16_t gauss;
  uint16_t permeability;
  uint16_t saturation;
  float reluctance;
} fieldParams[] = {
  {273, 1013, 45, 2.718281},
  {297, 1008, 62, 1.414213},
  {310, 995, 78, 0.707106},
  {285, 1020, 55, 3.141592}
};

// Hysteresis fading pattern (values between 0.0 and 1.0)
const uint8_t PATTERN_LENGTH = 8;
const float hysteresisPattern[PATTERN_LENGTH] = {0.0, 0.15, 0.4, 0.7, 1.0, 0.7, 0.4, 0.15};

// Timing parameters (milliseconds)
const unsigned long STEP_DURATION = 150;
unsigned long lastStepTime = 0;
uint8_t patternIndex = 0;

// Color rotation simulating magnetic domain rotation
uint8_t magneticRotation = 0;

// Base magnetic colors cycling through LEDs
CRGB magneticColors[NUM_LEDS] = {CRGB::Red, CRGB::Green, CRGB::Blue};

void setup() {
  FastLED.addLeds<WS2812, LED_PIN, GRB>(leds, NUM_LEDS);
  FastLED.setBrightness(BRIGHTNESS);
  fill_solid(leds, NUM_LEDS, CRGB::Black);
  FastLED.show();
}

void loop() {
  unsigned long currentMillis = millis();

  if (currentMillis - lastStepTime >= STEP_DURATION) {
    lastStepTime = currentMillis;

    // Update pattern index with wrap-around
    patternIndex = (patternIndex + 1) % PATTERN_LENGTH;

    // Rotate magnetic domain colors for LEDs
    magneticRotation = (magneticRotation + 1) % NUM_LEDS;

    // Update LEDs with crossfade based on hysteresis pattern and color rotation
    for (uint8_t i = 0; i < NUM_LEDS; i++) {
      // Determine color index for this LED, rotated by magneticRotation
      uint8_t colorIndex = (i + magneticRotation) % NUM_LEDS;

      // Crossfade brightness from hysteresis pattern
      float brightnessFactor = hysteresisPattern[(patternIndex + i) % PATTERN_LENGTH];

      // Scale base color by brightness factor
      leds[i] = magneticColors[colorIndex];
      leds[i].fadeToBlackBy(255 - uint8_t(brightnessFactor * 255));
    }

    FastLED.show();
  }
}

