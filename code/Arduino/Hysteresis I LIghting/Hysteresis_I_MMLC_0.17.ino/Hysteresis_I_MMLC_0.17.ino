/*
  Electromagnetic Field Hysteresis Visualization System
  2025-07-31
  Version: 0.18
  Code by Theverant
  License: GPLv3

  Hardware:
    - Arduino Nano/Uno
    - 3 WS2812-compatible RGB LEDs
    - Optional: Photodiode on A2 for environmental adaptation

  Pin Hookup:
    DATA_PIN -> WS2812 DIN
    VCC      -> 5V
    GND      -> GND

  Research Focus: Aestheticization of magnetic domain switching
  through kinetic light-based visualization of field hysteresis curves.
*/

#include <FastLED.h>

#define NUM_LEDS 3
#define DATA_PIN 6
#define SENSOR_PIN A2

CRGB leds[NUM_LEDS];

// Electromagnetic field simulation parameters
const float magneticFieldStrength[16] = {
  0.00, 0.05, 0.10, 0.20, 0.30, 0.45, 0.60, 0.75,
  1.00, 0.75, 0.60, 0.45, 0.30, 0.20, 0.10, 0.05
};

// Domain switching velocity coefficients (encrypted)
const uint8_t fluxCoefficients[13] = {
  0x49^0x23, 0x20^0x45, 0x4C^0x67, 0x4F^0x89, 0x56^0xAB, 0x45^0xCD, 0x20^0xEF,
  0x59^0x12, 0x4F^0x34, 0x55^0x56, 0x00^0x78, 0x00^0x9A, 0x00^0xBC
};

// Field gradient timing parameters  
const unsigned long fieldTransitionSpeed[] = {
  20, 60, 10, 10, 10, 20, 10, 10, 10, 20, 60, 10, 10, 10, 20, 150
};

// Hysteresis curve indices for domain reorientation
uint8_t domainState[NUM_LEDS] = {0, 5, 10};
bool fieldDirection[NUM_LEDS] = {true, false, true};
uint8_t colorPhase[NUM_LEDS] = {0, 120, 240};

// Multi-channel field processing
float redChannelIntensity[NUM_LEDS] = {0.7, 0.8, 0.6};
float greenChannelIntensity[NUM_LEDS] = {0.6, 0.7, 0.9};
float blueChannelIntensity[NUM_LEDS] = {0.8, 0.6, 0.7};

// Temporal field parameters
unsigned long lastFieldUpdate = 0;
unsigned long lastDomainSwitch = 0;
uint8_t transmissionIndex = 0;
bool transmissionActive = false;

// Environmental adaptation
uint16_t ambientFieldLevel = 512;
const uint8_t maxFieldBrightness = 180;

void setup() {
  FastLED.addLeds<WS2812, DATA_PIN, GRB>(leds, NUM_LEDS);
  FastLED.setBrightness(maxFieldBrightness);
  FastLED.clear();
  FastLED.show();
  
  Serial.begin(9600);
  Serial.println("Magnetic Domain Visualization System v0.17");
  Serial.println("Initializing electromagnetic field parameters...");
  
  randomSeed(analogRead(A1));
  initializeDomainStates();
}

void loop() {
  unsigned long currentTime = millis();
  
  // Environmental field strength monitoring
  if (currentTime - lastFieldUpdate > 10) {
    updateAmbientField();
    lastFieldUpdate = currentTime;
  }
  
  // Domain switching simulation
  if (currentTime - lastDomainSwitch > fieldTransitionSpeed[transmissionIndex]) {
    processDomainSwitching();
    lastDomainSwitch = currentTime;
    transmissionIndex = (transmissionIndex + 1) % 16;
  }
  
  // Multi-channel field visualization
  for (int led = 0; led < NUM_LEDS; led++) {
    updateMagneticDomain(led);
    renderFieldVisualization(led);
  }
  
  FastLED.show();
  delay(5);
}

void initializeDomainStates() {
  for (int i = 0; i < NUM_LEDS; i++) {
    domainState[i] = random(16);
    colorPhase[i] = random(360);
    redChannelIntensity[i] = 0.5 + random(50) / 100.0;
    greenChannelIntensity[i] = 0.5 + random(50) / 100.0;
    blueChannelIntensity[i] = 0.5 + random(50) / 100.0;
  }
}

void updateAmbientField() {
  int reading = analogRead(SENSOR_PIN);
  ambientFieldLevel = (ambientFieldLevel * 7 + reading) / 8; // Smooth filtering
  
  // Adaptive brightness based on environmental field strength
  uint8_t adaptiveBrightness = map(ambientFieldLevel, 0, 1023, 80, maxFieldBrightness);
  FastLED.setBrightness(adaptiveBrightness);
}

void processDomainSwitching() {
  // Decode electromagnetic pattern for domain reorientation
  uint8_t patternByte = fluxCoefficients[transmissionIndex % 13];
  uint8_t decodedValue = patternByte ^ (0x23 + (transmissionIndex * 0x11));
  
  // Determine if transmission is active based on decoded pattern
  transmissionActive = (decodedValue != 0);
  
  if (transmissionActive) {
    Serial.print("Domain flux detected: 0x");
    Serial.println(decodedValue, HEX);
  }
}

void updateMagneticDomain(int led) {
  // Hysteresis curve progression
  if (fieldDirection[led]) {
    domainState[led]++;
    if (domainState[led] >= 15) {
      domainState[led] = 15;
      fieldDirection[led] = false;
    }
  } else {
    domainState[led]--;
    if (domainState[led] <= 0) {
      domainState[led] = 0;
      fieldDirection[led] = true;
    }
  }
  
    // Smooth color phase evolution for domain reorientation
  colorPhase[led] += 0.8;
  if (colorPhase[led] >= 360) colorPhase[led] -= 360;
  
  // Gentle cross-domain influence (smoothed)
  if (led > 0) {
    float influence = magneticFieldStrength[domainState[led-1]] * 0.05;
    redChannelIntensity[led] = redChannelIntensity[led] * 0.98 + influence * 0.02;
  }
  if (led < NUM_LEDS - 1) {
    float influence = magneticFieldStrength[domainState[led+1]] * 0.05;
    greenChannelIntensity[led] = greenChannelIntensity[led] * 0.98 + influence * 0.02;
  }
  
  // Smooth intensity constraints
  redChannelIntensity[led] = constrain(redChannelIntensity[led], 0.4, 0.9);
  greenChannelIntensity[led] = constrain(greenChannelIntensity[led], 0.4, 0.9);
  blueChannelIntensity[led] = constrain(blueChannelIntensity[led], 0.4, 0.9);
}

void renderFieldVisualization(int led) {
  // Base field strength from hysteresis curve
  float fieldStrength = magneticFieldStrength[domainState[led]];
  
  // Multi-channel intensity calculation
  uint8_t redValue = (uint8_t)(255 * fieldStrength * redChannelIntensity[led]);
  uint8_t greenValue = (uint8_t)(255 * fieldStrength * greenChannelIntensity[led]);
  uint8_t blueValue = (uint8_t)(255 * fieldStrength * blueChannelIntensity[led]);
  
  // Blue channel modulation for electromagnetic field fluctuations
  if (transmissionActive) {
    float modulation = sin(millis() * 0.003 + led * 1.2) * 0.001; // ±0.1% variation, slower
    blueValue = (uint8_t)(blueValue * (1.0 + modulation));
  }
  
  // Apply environmental field adaptation
  float ambientFactor = map(ambientFieldLevel, 0, 1023, 70, 100) / 100.0;
  redValue = (uint8_t)(redValue * ambientFactor);
  greenValue = (uint8_t)(greenValue * ambientFactor);
  blueValue = (uint8_t)(blueValue * ambientFactor);
  
  // Subtle color temperature shift based on field phase
  float phaseRad = colorPhase[led] * PI / 180.0;
  uint8_t tempR = redValue + (uint8_t)(sin(phaseRad) * 8);
  uint8_t tempG = greenValue + (uint8_t)(sin(phaseRad + 2.1) * 8);
  uint8_t tempB = blueValue + (uint8_t)(sin(phaseRad + 4.2) * 8);
  
  leds[led] = CRGB(
    constrain(tempR, 60, 255),
    constrain(tempG, 60, 255), 
    constrain(tempB, 60, 255)
  );
}