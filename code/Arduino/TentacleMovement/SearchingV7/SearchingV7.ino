/*
  Searching V7
  Created by Theverant with essential help from ChatGPT
  Date: 2025-07-25
  License: GPLv3

  Changelog:
  - V1: Basic circular motion with randomness
  - V2: Axial + rotational motion with organic variation
  - V3: Added joystick toggle for manual/auto mode
  - V4: Center/rest mode with joystick button
  - V5: Reduced chatter, smoother transitions
  - V6: Axial movement stays more bent-over
  - V7: Smooth continuous motion, deeper reach, no abrupt movement
*/

#include <Servo.h>

// Servo pins
const int servoXPin = 3;
const int servoYPin = 5;

// Joystick pins
const int joyXPin = A0;
const int joyYPin = A1;
const int joyButtonPin = 2;

// Servo objects
Servo servoX;
Servo servoY;

// Adjustable parameters
float baseSpeed = 0.0001;       // Movement speed multiplier
float randomness = 0.5;        // 0 = no randomness, 1 = max randomness
float axialRadius = 40.0;      // Base size of axial circle (smaller means more bent over)
float axialVariation = 20.0;   // How far the axial radius can vary (+/-)

// State variables
bool joystickMode = false;
bool lastButtonState = HIGH;

float t = 0.0;
unsigned long lastUpdate = 0;

void setup() {
  servoX.attach(servoXPin);
  servoY.attach(servoYPin);
  pinMode(joyButtonPin, INPUT_PULLUP);

  Serial.begin(9600);
  Serial.println("Tentacle | Searching V7");
}

void loop() {
  handleJoystickToggle();

  if (joystickMode) {
    handleJoystickControl();
  } else {
    handleSearchingMotion();
  }

  delay(10);
}

void handleJoystickToggle() {
  bool currentButtonState = digitalRead(joyButtonPin);
  if (lastButtonState == HIGH && currentButtonState == LOW) {
    joystickMode = !joystickMode;
    Serial.print("Joystick mode: ");
    Serial.println(joystickMode ? "ON" : "OFF");
    if (!joystickMode) {
      centerServos();
    }
  }
  lastButtonState = currentButtonState;
}

void handleJoystickControl() {
  int joyX = analogRead(joyXPin);
  int joyY = analogRead(joyYPin);

  int xAngle = map(joyX, 0, 1023, 0, 180);
  int yAngle = map(joyY, 0, 1023, 0, 180);

  servoX.write(xAngle);
  servoY.write(yAngle);
}

void handleSearchingMotion() {
  unsigned long now = millis();
  float deltaTime = (now - lastUpdate) / 1000.0; // Seconds
  lastUpdate = now;

  // Advance time with variable speed
  float timeIncrement = baseSpeed + randomFloat(-randomness, randomness) * baseSpeed;
  t += timeIncrement * deltaTime * 100;

  // Smooth circular patterns using sine waves
  float axialR = axialRadius + randomFloat(-axialVariation, axialVariation);

  float xAngle = 90 + axialR * sin(t * 0.7 + randomFloat(-0.5, 0.5));
  float yAngle = 90 + (axialR * 0.8) * sin(t + randomFloat(-0.5, 0.5)); // Less vertical stretch

  // Bound angles and write to servos
  xAngle = constrain(xAngle, 0, 180);
  yAngle = constrain(yAngle, 0, 180);

  servoX.write(xAngle);
  servoY.write(yAngle);
}

float randomFloat(float minVal, float maxVal) {
  return minVal + (random(1000) / 1000.0) * (maxVal - minVal);
}

void centerServos() {
  servoX.write(90);
  servoY.write(90);
}
