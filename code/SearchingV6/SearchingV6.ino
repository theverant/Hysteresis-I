/*
  Code: Searching Tentacle
  Version: V8
  Date: 2025-07-25
  Author: Theverant (with essential help from ChatGPT)
  License: GPLv3

  Changelog:
  - V1: Basic two-servo circular motion
  - V2: Randomized weaving movement added
  - V3: Adjustable speed, axial range, and randomness
  - V4: Added joystick toggle and recenter feature
  - V5: Larger decision distances to reduce chatter
  - V6: Axial motion adjusted to keep posture lower
  - V7: (BROKEN) Attempted continuous motion; caused jitter
  - V8: Fixed jitter, added smooth serial control, restored adjustable smooth motion
*/

#include <Servo.h>

// === CONFIGURABLE PARAMETERS ===
const int servoXPin = 3;
const int servoYPin = 5;

// Motion tuning
float movementSpeed = 0.4;        // Lower = slower motion
float randomnessAmount = 0.3;     // 0–1 range
float baseCircleSize = 30;        // Size of axial circle
float axialVariation = 15;        // Max +/- variation from baseCircleSize
int angleThreshold = 2;           // Degrees of min movement to avoid jitter

// Servo limits
const int minAngle = 30;
const int maxAngle = 150;

// === STATE VARIABLES ===
Servo servoX, servoY;
float angleX = 90, angleY = 90;   // Current positions
float targetX = 90, targetY = 90;
bool joystickMode = false;
bool lastButtonState = false;

// Time tracking
unsigned long lastUpdate = 0;
const int updateInterval = 20; // ms

// Joystick
const int joystickXPin = A0;
const int joystickYPin = A1;
const int joystickButtonPin = 2;
const int deadzone = 20;
const int joyCenter = 512;
const int joyRange = 512;

// Sweep variables
float theta = 0.0;

void setup() {
  Serial.begin(9600);
  servoX.attach(servoXPin);
  servoY.attach(servoYPin);
  pinMode(joystickButtonPin, INPUT_PULLUP);
  centerServos();
}

void loop() {
  handleSerial();
  handleJoystickToggle();

  unsigned long now = millis();
  if (now - lastUpdate >= updateInterval) {
    lastUpdate = now;
    if (joystickMode) {
      handleJoystick();
    } else {
      updateSweep();
    }
    smoothMove();
  }
}

// === CORE FUNCTIONS ===

void updateSweep() {
  theta += movementSpeed;
  if (theta > TWO_PI) theta -= TWO_PI;

  float axialRadius = baseCircleSize + random(-axialVariation, axialVariation);
  float baseX = cos(theta);
  float baseY = sin(theta);

  float bendX = sin(theta * 1.5 + random(-10, 10) * 0.01);
  float bendY = cos(theta * 1.3 + random(-10, 10) * 0.01);

  // Blend base sweep with axial pattern
  float mixedX = baseX * 60 + bendX * axialRadius * randomnessAmount;
  float mixedY = baseY * 30 + bendY * axialRadius;

  targetX = constrain(90 + mixedX, minAngle, maxAngle);
  targetY = constrain(90 + mixedY, minAngle, maxAngle);
}

void smoothMove() {
  if (abs(angleX - targetX) > angleThreshold) {
    angleX += (targetX - angleX) * 0.05;
    servoX.write(angleX);
  }
  if (abs(angleY - targetY) > angleThreshold) {
    angleY += (targetY - angleY) * 0.05;
    servoY.write(angleY);
  }
}

void centerServos() {
  angleX = angleY = 90;
  targetX = targetY = 90;
  servoX.write(angleX);
  servoY.write(angleY);
}

// === JOYSTICK CONTROL ===

void handleJoystickToggle() {
  bool buttonState = !digitalRead(joystickButtonPin);
  if (buttonState && !lastButtonState) {
    joystickMode = !joystickMode;
    if (!joystickMode) centerServos();
  }
  lastButtonState = buttonState;
}

void handleJoystick() {
  int xRaw = analogRead(joystickXPin);
  int yRaw = analogRead(joystickYPin);

  if (abs(xRaw - joyCenter) > deadzone)
    targetX = map(xRaw, 0, 1023, minAngle, maxAngle);
  if (abs(yRaw - joyCenter) > deadzone)
    targetY = map(yRaw, 0, 1023, minAngle, maxAngle);
}

// === SERIAL CONTROL ===

void handleSerial() {
  if (Serial.available() > 1) {
    char axis = Serial.read();
    int val = Serial.parseInt();
    val = constrain(val, minAngle, maxAngle);
    if (axis == 'X' || axis == 'x') {
      targetX = val;
    } else if (axis == 'Y' || axis == 'y') {
      targetY = val;
    }
  }
}
