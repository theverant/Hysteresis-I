/*
 * Hysteresis - Magnetic Memory Lighting Controller
 * 2025-07-30
 * Version 0.14
 * Code by Theverant with essential help from ChatGPT
 * License: GPLv3
 *
 * Changelog:
 * - Fade speed for each LED cycles according to discrete hysteresis-like curve with direction memory
 * - Each LED has different step rate through the hysteresis speed pattern
 * - Color rotation and independent brightness fades per LED maintained
 */

#include <FastLED.h>

#define LED_PIN 6
#define NUM_LEDS 3
#define BRIGHTNESS 150

CRGB leds[NUM_LEDS];

// Hysteresis pattern for fade brightness (rising and falling)
const uint8_t PATTERN_LENGTH = 16;
const float hysteresisPattern[PATTERN_LENGTH] = {
  0.00, 0.05, 0.10, 0.20, 0.30, 0.45, 0.60, 0.75,
  1.00, 0.75, 0.60, 0.45, 0.30, 0.20, 0.10, 0.05
};

// Base colors cycling through LEDs
CRGB magneticColors[NUM_LEDS] = {CRGB::Red, CRGB::Green, CRGB::Blue};

// Discrete hysteresis fade speed values (in ms)
const unsigned long fadeSpeedPattern[] = {50, 70, 90, 110, 130, 150, 130, 110, 90, 70};
const uint8_t SPEED_PATTERN_LENGTH = sizeof(fadeSpeedPattern) / sizeof(fadeSpeedPattern[0]);

// Each LED state for fade brightness
uint8_t patternIndices[NUM_LEDS] = {0, 5, 10};
int8_t patternDirections[NUM_LEDS] = {1, 1, 1};

// Each LED state for speed pattern
uint8_t speedIndices[NUM_LEDS] = {0, 3, 6};
int8_t speedDirections[NUM_LEDS] = {1, 1, 1};

unsigned long lastStepTimes[NUM_LEDS] = {0, 0, 0};
unsigned long lastColorRotate = 0;
uint8_t magneticRotation = 0;

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
    direction = -direction; // Reverse direction at edges
  }
}

void loop() {
  unsigned long currentMillis = millis();

  for (uint8_t i = 0; i < NUM_LEDS; i++) {
    unsigned long fadeSpeed = getCurrentFadeSpeed(i);
    
    if (currentMillis - lastStepTimes[i] >= fadeSpeed) {
      lastStepTimes[i] = currentMillis;

      // Step brightness index with hysteresis direction
      updatePatternIndex(patternIndices[i], patternDirections[i], PATTERN_LENGTH);

      // Step speed index with hysteresis direction (slower stepping than brightness)
      static unsigned long lastSpeedStepTime[NUM_LEDS] = {0,0,0};
      if (currentMillis - lastSpeedStepTime[i] >= 200) {  // speed changes every 200ms
        lastSpeedStepTime[i] = currentMillis;
        updatePatternIndex(speedIndices[i], speedDirections[i], SPEED_PATTERN_LENGTH);
      }

      // Compute brightness factor from hysteresis pattern
      float brightnessFactor = hysteresisPattern[patternIndices[i]];

      // Rotate colors between LEDs
      uint8_t colorIndex = (i + magneticRotation) % NUM_LEDS;
      CRGB baseColor = magneticColors[colorIndex];

      leds[i] = CRGB(
        uint8_t(baseColor.r * brightnessFactor),
        uint8_t(baseColor.g * brightnessFactor),
        uint8_t(baseColor.b * brightnessFactor)
      );
    }
  }

  // Rotate LED colors every 60ms
  if (currentMillis - lastColorRotate >= 60) {
    lastColorRotate = currentMillis;
    magneticRotation = (magneticRotation + 1) % NUM_LEDS;
  }

  FastLED.show();
}
