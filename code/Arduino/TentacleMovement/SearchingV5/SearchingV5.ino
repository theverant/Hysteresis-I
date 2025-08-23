/*
  Searching V5 - Tentacle Sketch
  2025-07-25
  Code by Theverant with essential help from ChatGPT
  License: GPLv3

  Changelog:
  - V1: Basic circular sweep using two servos.
  - V2: Added axial motion.
  - V3: Adjustable speed, randomness, axial circle size, and variation range.
  - V4: Joystick control with toggle, recenters on mode switch.
  - V5: Reduced chatter with movement threshold, smoother transitions, more bent-over posture.
*/

#include <Servo.h>

// Servo pins
const int servoXPin = 3;
const int servoYPin = 5;

// Joystick pins
const int joystickXPin = A0;
const int joystickYPin = A1;
const int joystickButtonPin = 2;

// Servo objects
Servo servoX;
Servo servoY;

// State
int currentX = 90;
int currentY = 90;
bool manualMode = false;
bool lastButtonState = HIGH;

// Adjustable parameters
float baseSpeed = 0.003;          // Overall motion speed (lower = slower)
float randomness = 0.2;           // Amount of noise/randomness
float axialCircleSize = 30;       // How far it bends over
float axialDeviation = 10;        // +/- variation from axialCircleSize
int moveThreshold = 2;            // Degrees before motor updates

// Time tracking
unsigned long lastUpdate = 0;

// Phase for smooth circular motion
float basePhase = 0;
float axialPhase = 0;

// Joystick center and deadzone
const int joyCenter = 512;
const int deadzone = 20;

void setup() {
  servoX.attach(servoXPin);
  servoY.attach(servoYPin);
  pinMode(joystickButtonPin, INPUT_PULLUP);

  centerTentacle();
  Serial.begin(9600);
  Serial.println("Searching V5 Ready");
}

void loop() {
  handleJoystickToggle();

  if (manualMode) {
    manualControl();
  } else {
    automaticSearch();
  }

  delay(10);
}

void handleJoystickToggle() {
  bool buttonState = digitalRead(joystickButtonPin);
  if (buttonState == LOW && lastButtonState == HIGH) {
    manualMode = !manualMode;
    centerTentacle();
    delay(300); // debounce
  }
  lastButtonState = buttonState;
}

void centerTentacle() {
  currentX = 90;
  currentY = 90;
  servoX.write(currentX);
  servoY.write(currentY);
}

void manualControl() {
  int xVal = analogRead(joystickXPin);
  int yVal = analogRead(joystickYPin);

  int xDelta = abs(xVal - joyCenter) > deadzone ? map(xVal, 0, 1023, 0, 180) : 90;
  int yDelta = abs(yVal - joyCenter) > deadzone ? map(yVal, 0, 1023, 0, 180) : 90;

  if (abs(currentX - xDelta) > moveThreshold) {
    currentX = xDelta;
    servoX.write(currentX);
  }

  if (abs(currentY - yDelta) > moveThreshold) {
    currentY = yDelta;
    servoY.write(currentY);
  }
}

void automaticSearch() {
  unsigned long now = millis();
  if (now - lastUpdate < 15) return;
  lastUpdate = now;

  // Increment phases slowly
  basePhase += baseSpeed + random(-randomness * 100, randomness * 100) / 10000.0;
  axialPhase += (baseSpeed * 0.9) + random(-randomness * 100, randomness * 100) / 12000.0;

  // Bent-over pattern: axial motion stays offset
  float radiusX = 40 + random(-5, 5); // stays mostly flat
  float radiusY = axialCircleSize + random(-axialDeviation, axialDeviation);

  int targetX = 90 + radiusX * cos(basePhase);
  int targetY = 90 + radiusY * sin(axialPhase);

  if (abs(currentX - targetX) > moveThreshold) {
    currentX += (targetX - currentX) * 0.2;
    servoX.write((int)currentX);
  }

  if (abs(currentY - targetY) > moveThreshold) {
    currentY += (targetY - currentY) * 0.2;
    servoY.write((int)currentY);
  }
}
