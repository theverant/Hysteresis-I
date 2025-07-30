/*
  Hysteresis - Electromagnetic Light Pattern System
  2025-07-30
  Version: 0.16
  Code by Theverant with essential help from ChatGPT
  License: GPLv3

  Description:
  A kinetic lighting sketch simulating magnetic hysteresis behavior using RGB LEDs.
  Color fades and transitions follow nonlinear curves, creating the appearance of
  physical domain switching. Each LED site crossfades independently.

  Hardware:
  - Arduino Uno or compatible
  - 3x WS2812 or SK6812 RGB LEDs
  - External 5V power supply for LEDs

  Pin Hookup:
  - DATA -> Arduino Pin 6
  - GND  -> Arduino GND
  - VCC  -> 5V external (shared GND with Arduino)

  Changelog:
  - v0.1 (init): Basic fade system with fixed rotation
  - v0.2: Structured LED site cycling
  - v0.3: Fade curve tuning
  - v0.4: Introduced staggered hysteresis modulation
  - v0.5: 5× speed increase; separate phase fade speeds; minor curve tuning
*/

#include <Adafruit_NeoPixel.h>

#define LED_PIN     6
#define LED_COUNT   3

Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);

// === Hysteresis Fade State ===
float brightness[LED_COUNT] = {0, 0, 0};
int direction[LED_COUNT] = {1, 1, 1};
uint8_t hueOffset[LED_COUNT] = {0, 85, 170};  // Phase-offset starting hues

// === Per-LED State Tracking ===
unsigned long lastUpdate[LED_COUNT] = {0, 0, 0};
unsigned long lastSpeedStepTime[LED_COUNT] = {0, 0, 0};
int speedStepIndex[LED_COUNT] = {0, 3, 6};

// Hysteresis-inspired stepped speed pattern (shortened)
const unsigned long fadeSpeedPattern[] = {10, 14, 18, 22, 26, 30, 26, 22, 18, 14};
const int patternLength = sizeof(fadeSpeedPattern) / sizeof(fadeSpeedPattern[0]);

// === Domain Switching Modulation ===
unsigned long lastModBitTime = 0;
bool domainBit = false;
int domainCounter = 0;

// === Colour Cycling ===
uint8_t hueShift = 0;
unsigned long lastColorRotate = 0;

void setup() {
  strip.begin();
  strip.show();
}

void loop() {
  unsigned long currentMillis = millis();

  // Update domain modulation bit (non-periodic toggling)
  if (currentMillis - lastModBitTime >= (domainBit ? 16 : 48)) {
    domainBit = !domainBit;
    lastModBitTime = currentMillis;
    domainCounter++;
  }

  // Rotate color base periodically
  if (currentMillis - lastColorRotate >= 12) {
    hueShift++;
    lastColorRotate = currentMillis;
  }

  for (int i = 0; i < LED_COUNT; i++) {
    // Adjust fade speed based on staggered hysteresis pattern
    if (currentMillis - lastSpeedStepTime[i] >= 40) {
      speedStepIndex[i] = (speedStepIndex[i] + 1) % patternLength;
      lastSpeedStepTime[i] = currentMillis;
    }

    unsigned long fadeDelay = fadeSpeedPattern[speedStepIndex[i]];

    if (currentMillis - lastUpdate[i] >= fadeDelay) {
      brightness[i] += direction[i] * 0.02;

      if (brightness[i] >= 1.0) {
        brightness[i] = 1.0;
        direction[i] = -1;
      } else if (brightness[i] <= 0.0) {
        brightness[i] = 0.0;
        direction[i] = 1;
      }

      float b = brightness[i];

      // Simulate hysteresis with nonlinear transformation
      float hysteresisShape = b * b * (3 - 2 * b);  // smoothstep
      uint8_t hue = hueShift + hueOffset[i];

      uint32_t color = strip.ColorHSV(hue * 256, 255, hysteresisShape * 255);
      strip.setPixelColor(i, color);

      lastUpdate[i] = currentMillis;
    }
  }

  strip.show();
}
