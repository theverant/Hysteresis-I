/*
 * Hysteresis - Magnetic Memory Lighting Controller
 * 2025-07-30
 * Version 0.15
 * Code by Theverant with essential help from ChatGPT
 * License: GPLv3
 *
 * Changelog:
 * - Reintegrated discrete hysteresis pattern fading and speed cycling
 * - Embedded data modulation fully obfuscated within brightness and color fades
 * - Removed explicit references to messaging or Morse code
 */

#include <FastLED.h>

#define LED_PIN 6
#define NUM_LEDS 3
#define BRIGHTNESS 150

CRGB leds[NUM_LEDS];

// Discrete hysteresis brightness pattern (rising/falling)
const uint8_t PATTERN_LENGTH = 16;
const float hysteresisPattern[PATTERN_LENGTH] = {
  0.00, 0.05, 0.10, 0.20, 0.30, 0.45, 0.60, 0.75,
  1.00, 0.75, 0.60, 0.45, 0.30, 0.20, 0.10, 0.05
};

// Base magnetic-like colors cycling through LEDs
CRGB magneticColors[NUM_LEDS] = {CRGB::Red, CRGB::Green, CRGB::Blue};

// Pattern controlling fade speed (hysteresis-like step timings)
const unsigned long fadeSpeedPattern[] = {50, 70, 90, 110, 130, 150, 130, 110, 90, 70};
const uint8_t SPEED_PATTERN_LENGTH = sizeof(fadeSpeedPattern) / sizeof(fadeSpeedPattern[0]);

// Obfuscated data modulation patterns ("domainPatterns") encrypted with magnetic field keys
const uint8_t domainPatterns[11] = {
  0x5A ^ 0xAB, 0x3C ^ 0xBC, 0xE1 ^ 0xCD, 0x2F ^ 0xDE, 0x84 ^ 0xEF,
  0xD7 ^ 0xFA, 0x91 ^ 0xAC, 0x4B ^ 0xBD, 0x76 ^ 0xCE, 0xA9 ^ 0xDF, 0x12 ^ 0xE0
};
// Magnetic field keys for decrypting domainPatterns (rotated per fluxCoefficients)
const uint8_t fieldKeys[4] = {0xAB, 0xBC, 0xCD, 0xDE};

// Obfuscated flux coefficients sequence indexing domainPatterns, guiding modulation timing
const uint8_t fluxCoefficients[] = {2, 8, 5, 10, 10, 7, 5, 7, 9, 5, 3};

uint8_t patternIndices[NUM_LEDS] = {0, 5, 10};
int8_t patternDirections[NUM_LEDS] = {1, 1, 1};

uint8_t speedIndices[NUM_LEDS] = {0, 3, 6};
int8_t speedDirections[NUM_LEDS] = {1, 1, 1};

unsigned long lastStepTimes[NUM_LEDS] = {0, 0, 0};
unsigned long lastColorRotate = 0;
uint8_t magneticRotation = 0;

unsigned long lastSpeedStepTime[NUM_LEDS] = {0, 0, 0};

// Modulation state variables to cycle through obfuscated data pattern
uint8_t fluxCursor = 0;
uint8_t domainIndex = 0;
bool magnetizationActive = true;
unsigned long lastDomainChange = 0;
bool hysteresisMode = false;
unsigned long hysteresisStart = 0;

// Return decrypted domain bit for modulation (simulating domain switching)
bool getDomainBit() {
  if (!magnetizationActive) return false;

  if (hysteresisMode) {
    if (millis() - hysteresisStart >= 80) { // Stabilization period
      hysteresisMode = false;
      lastDomainChange = millis();
    }
    return false;
  }

  if (domainIndex >= sizeof(fluxCoefficients)) {
    magnetizationActive = false;
    return false;
  }

  uint8_t patternIndex = fluxCoefficients[domainIndex];
  uint8_t encryptedPattern = domainPatterns[patternIndex % 11];
  uint8_t key = fieldKeys[domainIndex % 4];
  uint8_t decrypted = encryptedPattern ^ key;

  uint8_t bitPos = 7 - (fluxCursor % 8);
  bool bit = (decrypted & (1 << bitPos)) != 0;

  unsigned long now = millis();
  unsigned long interval = bit ? 80 : 240; // 80ms or 240ms per bit duration

  if (now - lastDomainChange >= interval) {
    fluxCursor++;
    if (fluxCursor >= 8) {
      fluxCursor = 0;
      domainIndex++;
      hysteresisMode = true;
      hysteresisStart = now;
    }
    lastDomainChange = now;
  }

  return bit;
}

void setup() {
  FastLED.addLeds<WS2812, LED_PIN, GRB>(leds, NUM_LEDS);
  FastLED.setBrightness(BRIGHTNESS);
  fill_solid(leds, NUM_LEDS, CRGB::Black);
  FastLED.show();
}

unsigned long getCurrentFadeSpeed(uint8_t ledIndex) {
  return fadeSpeedPattern[speedIndices[ledIndex]];
}

void updatePatternIndex(uint8_t &index, int8_t &direction, uint8_t length) {
  index = index + direction;
  if (index == 0 || index == length - 1) {
    direction = -direction;
  }
}

void loop() {
  unsigned long currentMillis = millis();

  // Advance speed indices per hysteresis speed pattern
  for (uint8_t i = 0; i < NUM_LEDS; i++) {
    if (currentMillis - lastSpeedStepTime[i] >= 200) {
      lastSpeedStepTime[i] = currentMillis;
      updatePatternIndex(speedIndices[i], speedDirections[i], SPEED_PATTERN_LENGTH);
    }
  }

  for (uint8_t i = 0; i < NUM_LEDS; i++) {
    // Update brightness index based on LED's current fade speed
    unsigned long fadeSpeed = getCurrentFadeSpeed(i);

    if (currentMillis - lastStepTimes[i] >= fadeSpeed) {
      lastStepTimes[i] = currentMillis;

      // Step brightness index with hysteresis direction
      updatePatternIndex(patternIndices[i], patternDirections[i], PATTERN_LENGTH);

      // Compute base brightness from hysteresis pattern
      float brightnessFactor = hysteresisPattern[patternIndices[i]];

      // Add domain modulation effect (small amplitude)
      bool domainBit = getDomainBit();
      float modulation = domainBit ? 0.07f : -0.07f; // ±7% brightness change hidden in pattern

      brightnessFactor += modulation;
      brightnessFactor = constrain(brightnessFactor, 0.0f, 1.0f);

      // Rotate base color with magneticRotation offset
      uint8_t colorIndex = (i + magneticRotation) % NUM_LEDS;
      CRGB baseColor = magneticColors[colorIndex];

      leds[i] = CRGB(
        uint8_t(baseColor.r * brightnessFactor),
        uint8_t(baseColor.g * brightnessFactor),
        uint8_t(baseColor.b * brightnessFactor)
      );
    }
  }

  // Rotate LED colors every 60ms for dynamic effect
  if (currentMillis - lastColorRotate >= 60) {
    lastColorRotate = currentMillis;
    magneticRotation = (magneticRotation + 1) % NUM_LEDS;
  }

  FastLED.show();
}
