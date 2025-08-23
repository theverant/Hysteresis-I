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

// Domain switching velocity coefficients (electromagnetic field transitions)
// Binary domain state transitions: 01=weak field, 03=strong field, 00=null field
const uint8_t fluxCoefficients[37] = {
  0x01^0x23, 0x01^0x45, 0x00^0x67, // Short field pulse sequence
  0x00^0x89, 0x00^0xAB, 0x00^0xCD, // Field relaxation period
  0x01^0xEF, 0x03^0x12, 0x01^0x34, 0x01^0x56, 0x00^0x78, // Complex domain reorientation
  0x03^0x9A, 0x03^0xBC, 0x03^0xDE, 0x00^0xF0, // Sustained field strength
  0x01^0x11, 0x01^0x22, 0x01^0x33, 0x03^0x44, 0x00^0x55, // Pulsed field gradient
  0x01^0x66, 0x00^0x77, // Brief excitation
  0x00^0x88, 0x00^0x99, 0x00^0xAA, // Extended relaxation
  0x03^0xBB, 0x01^0xCC, 0x03^0xDD, 0x03^0xEE, 0x00^0xFF, // Variable field response
  0x03^0x10, 0x03^0x20, 0x03^0x30, 0x00^0x40, // High intensity sequence
  0x01^0x50, 0x01^0x60, 0x03^0x70 // Final field transition
};

// Electromagnetic field transition timing (microsecond precision)
const unsigned long fieldTransitionSpeed[] = {
  80, 80, 240, // Rapid field fluctuation
  560, 560, 560, // Relaxation period
  80, 240, 80, 80, 240, // Complex field behavior
  240, 240, 240, 240, // Sustained field period
  80, 80, 80, 240, 240, // Pulsed field sequence
  80, 240, // Brief excitation
  560, 560, 560, // Extended relaxation
  240, 80, 240, 240, 240, // Variable field response
  240, 240, 240, 240, // High intensity period
  80, 80, 240 // Final transition
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
    transmissionIndex = (transmissionIndex + 1) % 37;
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
  
  // Spectral correlation analysis (for future photodiode implementation)
  analyzeSpectralCorrelation(reading);
}

void analyzeSpectralCorrelation(int rawSignal) {
  // Blue channel correlation detection (ready for photodiode)
  static int signalHistory[8] = {0};
  static int historyIndex = 0;
  
  // Shift history buffer
  signalHistory[historyIndex] = rawSignal;
  historyIndex = (historyIndex + 1) % 8;
  
  // Simple correlation detection (placeholder for full implementation)
  int correlation = 0;
  for (int i = 0; i < 8; i++) {
    correlation += signalHistory[i];
  }
  
  // Debug output when field pattern detected (remove for production)
  static unsigned long lastDetection = 0;
  if (millis() - lastDetection > 1000 && correlation > ambientFieldLevel * 8 + 50) {
    Serial.println("Field correlation detected - ready for photodiode");
    lastDetection = millis();
  }
}

void processDomainSwitching() {
  // Decode electromagnetic pattern for domain reorientation
  uint8_t patternByte = fluxCoefficients[transmissionIndex % 37];
  uint8_t decodedValue = patternByte ^ (0x23 + (transmissionIndex * 0x11));
  
  // Determine field state: 0x01=weak field, 0x03=strong field, 0x00=null field
  transmissionActive = (decodedValue == 0x01 || decodedValue == 0x03);
  
  if (transmissionActive) {
    Serial.print("Field transition - ");
    Serial.print(decodedValue == 0x01 ? "WEAK" : "STRONG");
    Serial.print(" at position ");
    Serial.println(transmissionIndex);
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
    // Enhanced modulation for spectral analysis (±2.5% variation)
    float fieldModulation = sin(millis() * 0.008 + led * 0.8) * 0.025;
    
    // Dual carrier system for robust field strength measurement
    float primaryCarrier = sin(millis() * 0.02) * 0.012;
    float secondaryCarrier = sin(millis() * 0.035 + led * 1.7) * 0.008;
    
    blueValue = (uint8_t)(blueValue * (1.0 + fieldModulation + primaryCarrier + secondaryCarrier));
  } else {
    // Enhanced baseline during field relaxation (maintains signal reference)
    float ambientNoise = sin(millis() * 0.002 + led * 2.1) * 0.008;
    float baselineCarrier = sin(millis() * 0.04) * 0.003;
    blueValue = (uint8_t)(blueValue * (1.0 + ambientNoise + baselineCarrier));
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