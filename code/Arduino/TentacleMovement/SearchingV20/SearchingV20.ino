/*
  Searching V21 - Enhanced Wandering
  Improved organic search patterns
  Date: 2025-08-03
  License: GPLv3
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

// Movement parameters
float xPhase = 0.0;
float yPhase = 0.0;
float wanderTime = 0.0;

// Speed and scale parameters
const float baseSpeed = 0.02;      // Base speed for phase advancement
const float moveScale = 40.0;      // Maximum deviation from center
const float smoothing = 0.05;      // Smoothing factor for position updates

// Perlin-like noise state
float xNoise = 0.0;
float yNoise = 0.0;
const float noiseStep = 0.01;

// Servo limits
const float xMin = 20.0;
const float xMax = 160.0;
const float yMin = 30.0;
const float yMax = 150.0;

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
  
  // Initialize random seed
  randomSeed(analogRead(0));
  xNoise = random(1000) / 1000.0;
  yNoise = random(1000) / 1000.0;
  
  Serial.begin(9600);
  Serial.println("Tentacle V15 - Enhanced Wandering");
}

void loop() {
  unsigned long now = millis();
  
  // Button handling
  bool btnState = digitalRead(buttonPin);
  if (btnState == LOW && lastBtnState == HIGH && now - lastBtnPress > btnDebounceMs) {
    joystickEnabled = !joystickEnabled;
    lastBtnPress = now;
    Serial.print("Mode: ");
    Serial.println(joystickEnabled ? "Joystick" : "Wandering");
  }
  lastBtnState = btnState;
  
  handleSerial();
  
  if (!joystickEnabled && now - lastUpdate > updateInterval) {
    lastUpdate = now;
    updateWandering();
  }
}

void updateWandering() {
  // Advance time
  wanderTime += updateInterval / 1000.0;
  
  // Create complex movement using multiple sine waves
  float xBase = 90.0 + sin(wanderTime * 0.3) * 30.0;  // Slow large sweep
  float yBase = 90.0 + cos(wanderTime * 0.4) * 25.0;  // Different frequency
  
  // Add medium frequency components
  xBase += sin(wanderTime * 1.1) * 15.0;
  yBase += cos(wanderTime * 0.9) * 12.0;
  
  // Add high frequency wobble
  xBase += sin(wanderTime * 3.7) * 5.0;
  yBase += cos(wanderTime * 4.3) * 4.0;
  
  // Apply pseudo-Perlin noise for organic feel
  xNoise += noiseStep;
  yNoise += noiseStep;
  float xNoiseVal = (sin(xNoise * 6.28) + sin(xNoise * 12.56) * 0.5) * 10.0;
  float yNoiseVal = (sin(yNoise * 6.28) + sin(yNoise * 12.56) * 0.5) * 8.0;
  
  // Calculate target positions
  float xTarget = xBase + xNoiseVal;
  float yTarget = yBase + yNoiseVal;
  
  // Constrain to limits
  xTarget = constrain(xTarget, xMin, xMax);
  yTarget = constrain(yTarget, yMin, yMax);
  
  // Smooth approach to targets
  xAngle += (xTarget - xAngle) * smoothing;
  yAngle += (yTarget - yAngle) * smoothing;
  
  // Write to servos
  servoX.write(constrain(xAngle, 0, 180));
  servoY.write(constrain(yAngle, 0, 180));
  
  // Debug output
  static unsigned long lastPrint = 0;
  if (now - lastPrint > 250) {
    lastPrint = now;
    Serial.print("X: "); Serial.print((int)xAngle);
    Serial.print("  Y: "); Serial.print((int)yAngle);
    Serial.print("  (T: "); Serial.print(xTarget, 1);
    Serial.print(", "); Serial.print(yTarget, 1);
    Serial.println(")");
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