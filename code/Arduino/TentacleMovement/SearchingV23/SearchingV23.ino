/*
  Searching V24
  Two-tier emotional search system with fatigue-driven rest cycles
  Created by Theverant with essential help from Claude Opus 4
  Date: 2025-08-03
  License: GPLv3

  Changelog:
  - v24: Two-tier search with major/minor circles, dual memory systems, rest cycles
  - v23: Implemented hysteresis-based dwelling for lonely search behavior
  - v22: Added epicycloid pattern generation for circles within circles
  - v21: Multi-frequency sine wave exploration with organic movement
  - v20: Added Perlin-like noise generation for natural variations
  - v19: Improved smoothing algorithms for servo response
  - v18: Enhanced boundary constraints and limit handling
  - v17: Optimized timing intervals for consistent updates
  - v16: Refactored movement calculation pipeline
  - v15: Added debug output for position tracking
  - v14: Fixed servo pin assignments (X=3, Y=5)
  - v13: Renamed base/axial to X/Y for clarity
  - v12: Implemented smooth searching motion from startup
  - v11: Added manual override via serial terminal
  - v10: Basic random walk implementation
*/

#include <Servo.h>

Servo servoX;
Servo servoY;

const int servoXPin = 3;
const int servoYPin = 5;
const int buttonPin = 2;

// Current positions
float xAngle = 90.0;
float yAngle = 90.0;

// Major circle parameters
float majorPhase = 0.0;           // Position along major circle
float majorRadius = 55.0;         // Mean radius of major circle
float majorDirection = 1.0;       // 1 or -1 for direction
float wobblePhase = 0.0;          // For radius variation
float wobbleAmount = 15.0;        // How much radius varies

// Minor circle parameters  
float minorPhase = 0.0;           // Position in minor circle
float minorRadius = 12.0;         // Current radius of minor circles
float minorDirection = 1.0;       // Local search direction
float minorSpeed = 0.15;          // Much faster than major

// Memory systems
float searchFatigue = 0.0;        // Global fatigue from searching
float localMemory = 0.0;          // Local area memory
const float maxFatigue = 100.0;   // Rest threshold
const float maxLocalMemory = 5.0; // Local behavior change threshold

// Rest state
bool isResting = false;
float restTimer = 0.0;
const float restDuration = 150.0; // ~3 seconds at 20ms updates

// Movement tracking for fatigue
float lastX = 90.0;
float lastY = 90.0;

// Speed parameters
float majorSpeed = 0.003;         // Base speed for major circle
const float wobbleSpeed = 0.02;   // Speed of radius variation
const float fatigueRate = 0.05;   // How fast fatigue builds

// Servo limits
const float xMin = 15.0;
const float xMax = 165.0;
const float yMin = 35.0;
const float yMax = 145.0;

// Timing
unsigned long lastUpdate = 0;
const unsigned long updateInterval = 20;

// Button handling
bool joystickEnabled = false;
bool lastBtnState = HIGH;
unsigned long lastBtnPress = 0;
const unsigned long btnDebounceMs = 200;

void setup() {
  servoX.attach(servoXPin);
  servoY.attach(servoYPin);
  pinMode(buttonPin, INPUT_PULLUP);
  
  Serial.begin(9600);
  Serial.println("Searching V24 - Emotional Search System");
  Serial.println("Major/minor circles with fatigue-based rest");
}

void loop() {
  unsigned long now = millis();
  
  // Button handling
  bool btnState = digitalRead(buttonPin);
  if (btnState == LOW && lastBtnState == HIGH && now - lastBtnPress > btnDebounceMs) {
    joystickEnabled = !joystickEnabled;
    lastBtnPress = now;
    Serial.print("Mode: ");
    Serial.println(joystickEnabled ? "Manual Control" : "Emotional Search");
  }
  lastBtnState = btnState;
  
  handleSerial();
  
  if (!joystickEnabled && now - lastUpdate > updateInterval) {
    lastUpdate = now;
    if (isResting) {
      updateRest();
    } else {
      updateEmotionalSearch();
    }
  }
}

void updateEmotionalSearch() {
  // Update wobble for major circle radius variation
  wobblePhase += wobbleSpeed;
  float currentMajorRadius = majorRadius + sin(wobblePhase) * wobbleAmount;
  
  // Update major circle position
  majorPhase += majorSpeed * majorDirection;
  
  // Check for major circle direction reversal based on fatigue
  if (searchFatigue > maxFatigue * 0.7 && random(100) < 5) {
    majorDirection *= -1;
    Serial.println("Major direction reversed due to fatigue");
  }
  
  // Update minor circle - much faster, always searching
  minorPhase += minorSpeed * minorDirection;
  
  // Local memory affects minor circle behavior
  localMemory += 0.1;  // Build up when searching same area
  
  // Change minor circle patterns based on local memory
  if (localMemory > maxLocalMemory) {
    minorDirection *= -1;  // Reverse local search
    minorRadius = 8.0 + random(100) / 10.0;  // Vary radius 8-18
    localMemory = 0;
    Serial.println("Local pattern change");
  }
  
  // Calculate combined position
  // Major circle with wobble
  float majorX = cos(majorPhase) * currentMajorRadius;
  float majorY = sin(majorPhase) * currentMajorRadius * 0.8;  // Elliptical
  
  // Minor circles on top of major
  float minorX = cos(minorPhase) * minorRadius;
  float minorY = sin(minorPhase) * minorRadius;
  
  // Combined target position
  float xTarget = 90.0 + majorX + minorX;
  float yTarget = 90.0 + majorY + minorY;
  
  // Constrain to servo limits
  xTarget = constrain(xTarget, xMin, xMax);
  yTarget = constrain(yTarget, yMin, yMax);
  
  // Calculate movement for fatigue
  float deltaX = abs(xTarget - lastX);
  float deltaY = abs(yTarget - lastY);
  float totalMovement = sqrt(deltaX * deltaX + deltaY * deltaY);
  
  // Accumulate fatigue based on movement
  searchFatigue += totalMovement * fatigueRate;
  
  // Extra fatigue at extremes
  float distFromCenter = sqrt(pow(xTarget - 90, 2) + pow(yTarget - 90, 2));
  if (distFromCenter > 70) {
    searchFatigue += 0.1;  // Extra cost at edges
  }
  
  // Check for exhaustion
  if (searchFatigue >= maxFatigue) {
    isResting = true;
    restTimer = 0;
    Serial.println("Exhausted - entering rest cycle");
  }
  
  // Smooth approach to target
  const float smoothing = 0.12;
  xAngle += (xTarget - xAngle) * smoothing;
  yAngle += (yTarget - yAngle) * smoothing;
  
  // Update last positions
  lastX = xTarget;
  lastY = yTarget;
  
  // Write to servos
  servoX.write(constrain(xAngle, 0, 180));
  servoY.write(constrain(yAngle, 0, 180));
  
  // Debug output
  static unsigned long lastPrint = 0;
  if (millis() - lastPrint > 500) {
    lastPrint = millis();
    Serial.print("Fatigue: "); Serial.print(searchFatigue, 1);
    Serial.print("/"); Serial.print(maxFatigue);
    Serial.print(" | Major R: "); Serial.print(currentMajorRadius, 1);
    Serial.print(" | X: "); Serial.print((int)xAngle);
    Serial.print(" Y: "); Serial.println((int)yAngle);
  }
  
  // Keep phases bounded
  if (majorPhase > 6.28318) majorPhase -= 6.28318;
  if (majorPhase < 0) majorPhase += 6.28318;
  if (minorPhase > 6.28318) minorPhase -= 6.28318;
  if (wobblePhase > 6.28318) wobblePhase -= 6.28318;
}

void updateRest() {
  restTimer += 1.0;
  
  // During rest: very subtle minor movements only
  float restX = 90.0 + cos(minorPhase * 0.3) * 3.0;
  float restY = 90.0 + sin(minorPhase * 0.3) * 2.0;
  
  minorPhase += 0.02;  // Very slow
  
  // Gentle approach to rest position
  xAngle += (restX - xAngle) * 0.05;
  yAngle += (restY - yAngle) * 0.05;
  
  servoX.write(constrain(xAngle, 0, 180));
  servoY.write(constrain(yAngle, 0, 180));
  
  // Check if rest is complete
  if (restTimer >= restDuration) {
    isResting = false;
    searchFatigue = 0;
    localMemory = 0;
    Serial.println("Rest complete - resuming search");
  }
  
  // Debug during rest
  static unsigned long lastPrint = 0;
  if (millis() - lastPrint > 1000) {
    lastPrint = millis();
    Serial.print("Resting... "); 
    Serial.print((int)((restTimer / restDuration) * 100));
    Serial.println("%");
  }
}

void handleSerial() {
  if (Serial.available() >= 2) {
    char axis = Serial.read();
    int val = Serial.parseInt();
    if (val >= 0 && val <= 180) {
      if (axis == 'X') {
        xAngle = val;
        Serial.print("Manual X: "); Serial.println(val);
      }
      else if (axis == 'Y') {
        yAngle = val;
        Serial.print("Manual Y: "); Serial.println(val);
      }
    }
  }
}