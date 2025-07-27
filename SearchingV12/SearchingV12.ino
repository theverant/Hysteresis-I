/*
  Searching V14
  Created by Theverant with essential help from ChatGPT
  Date: 2025-07-27
  License: GPLv3

  Improvements:
  - Renamed base/axial to X/Y for clarity
  - Restored correct motor pins (X=3, Y=5)
  - Smooth searching motion from startup
  - Manual override via serial terminal (e.g., "X120", "Y45")
*/

#include <Servo.h>

Servo servoX;
Servo servoY;

const int servoXPin = 3;  // X = motor that affects base rotation
const int servoYPin = 5;  // Y = motor that affects axial tilt
const int buttonPin = 2;

float xAngle = 90.0;
float yAngle = 90.0;
float xTarget = 120.0;
float yTarget = 60.0;

float movementSpeed = 0.08;

const float xMin = 0.0;
const float xMax = 180.0;
const float yMin = 0.0;
const float yMax = 180.0;

unsigned long lastUpdate = 0;
unsigned long updateInterval = 20;

bool joystickEnabled = false;
bool lastBtnState = HIGH;
unsigned long lastBtnPress = 0;
const unsigned long btnDebounceMs = 200;

void setup() {
  servoX.attach(servoXPin);
  servoY.attach(servoYPin);
  pinMode(buttonPin, INPUT_PULLUP);
  Serial.begin(9600);
  Serial.println("Tentacle V14 starting...");
}

void loop() {
  unsigned long now = millis();

  bool btnState = digitalRead(buttonPin);
  if (btnState == LOW && lastBtnState == HIGH && now - lastBtnPress > btnDebounceMs) {
    joystickEnabled = !joystickEnabled;
    lastBtnPress = now;
    Serial.print("Mode toggled. Joystick enabled: ");
    Serial.println(joystickEnabled);
  }
  lastBtnState = btnState;

  handleSerial();

  if (!joystickEnabled && now - lastUpdate > updateInterval) {
    lastUpdate = now;
    updateTargets();
    updateMotion();
  }
}

void updateMotion() {
  xAngle += (xTarget - xAngle) * movementSpeed;
  yAngle += (yTarget - yAngle) * movementSpeed;

  servoX.write(constrain(xAngle, 0, 180));
  servoY.write(constrain(yAngle, 0, 180));

  Serial.print("X: "); Serial.print((int)xAngle);
  Serial.print("  Y: "); Serial.println((int)yAngle);
}

void updateTargets() {
  xTarget += randomFloat(-1.2, 1.2);
  xTarget = constrain(xTarget, xMin, xMax);

  yTarget += randomFloat(-1.0, 1.0);
  yTarget = constrain(yTarget, yMin, yMax);
}

void handleSerial() {
  if (Serial.available() >= 2) {
    char axis = Serial.read();
    int val = Serial.parseInt();
    if (val >= 0 && val <= 180) {
      if (axis == 'X') xTarget = val;
      else if (axis == 'Y') yTarget = val;
      Serial.print("Manual override: ");
      Serial.print(axis); Serial.print(" -> "); Serial.println(val);
    }
  }
}

float randomFloat(float minVal, float maxVal) {
  return minVal + (maxVal - minVal) * random(0, 1000) / 1000.0;
}
