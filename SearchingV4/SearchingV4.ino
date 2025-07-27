/*
  Searching V5
  July 25, 2025
  Code by Theverant with essential help from ChatGPT
  License: GPLv3

  Changelog:
  - V1: Base circular motion with 2 servos
  - V2: Random axial variation and speed control
  - V3: Added randomness smoothing, axial circle weaving
  - V4: Joystick control added with click toggle and centering
  - V5: Reduced jitter with angle update thresholds, smoother pattern transitions
*/

#include <Servo.h>

// === CONFIGURATION ===
const int servoXPin = 3;
const int servoYPin = 5;
const int joystickXPin = A0;
const int joystickYPin = A1;
const int joystickBtnPin = 2;

// Adjustable Parameters
int movementSpeed = 30;          // Delay between updates (ms)
float randomnessIntensity = 0.5; // 0.0 to 1.0
float axialBaseSize = 20.0;      // Average radius of axial circle
float axialSizeDeviation = 10.0; // +/- deviation from axialBaseSize
float updateThreshold = 1.5;     // Minimum angle change to trigger servo write

Servo servoX, servoY;

// Motion State
float angleAroundBase = 0;
float axialAngle = 0;
float currentAxialSize = 20.0;
float targetAxialSize = 20.0;
unsigned long lastUpdate = 0;

int lastServoX = 90;
int lastServoY = 90;

// Mode Toggle
bool manualMode = false;
bool lastButtonState = HIGH;

// === SETUP ===
void setup() {
  servoX.attach(servoXPin);
  servoY.attach(servoYPin);
  pinMode(joystickBtnPin, INPUT_PULLUP);

  centerTentacle();

  Serial.begin(9600);
  Serial.println("Searching V5 started");
}

// === MAIN LOOP ===
void loop() {
  handleJoystickToggle();

  if (manualMode) {
    manualControl();
  } else {
    autoSearchMotion();
  }
}

// === MANUAL JOYSTICK MODE ===
void manualControl() {
  int xRaw = analogRead(joystickXPin);
  int yRaw = analogRead(joystickYPin);

  int xMapped = map(xRaw, 0, 1023, 0, 180);
  int yMapped = map(yRaw, 0, 1023, 0, 180);

  setServoIfNeeded(servoX, lastServoX, xMapped);
  setServoIfNeeded(servoY, lastServoY, yMapped);
}

// === AUTOMATIC SEARCH MODE ===
void autoSearchMotion() {
  unsigned long now = millis();
  if (now - lastUpdate < movementSpeed) return;
  lastUpdate = now;

  // Slowly adjust axial size to smooth transitions
  if (abs(currentAxialSize - targetAxialSize) < 0.5) {
    // Pick a new target if close to current
    targetAxialSize = axialBaseSize + random(-axialSizeDeviation * 10, axialSizeDeviation * 10) / 10.0;
  } else {
    // Gradually interpolate
    currentAxialSize += (targetAxialSize - currentAxialSize) * 0.05;
  }

  // Increment angles
  angleAroundBase += 0.015 + randomnessIntensity * random(-10, 10) / 1000.0;
  axialAngle += 0.03 + randomnessIntensity * random(-10, 10) / 1000.0;

  // Calculate positions
  float baseRadius = 30.0;
  float x = baseRadius * cos(angleAroundBase);
  float y = currentAxialSize * sin(axialAngle);

  // Map to servo angles
  int xAngle = constrain(90 + x, 0, 180);
  int yAngle = constrain(90 + y, 0, 180);

  setServoIfNeeded(servoX, lastServoX, xAngle);
  setServoIfNeeded(servoY, lastServoY, yAngle);
}

// === HELPERS ===

void centerTentacle() {
  lastServoX = 90;
  lastServoY = 90;
  servoX.write(90);
  servoY.write(90);
}

void handleJoystickToggle() {
  bool currentButtonState = digitalRead(joystickBtnPin);

  if (lastButtonState == HIGH && currentButtonState == LOW) {
    manualMode = !manualMode;
    centerTentacle();
    delay(250);  // Debounce delay
  }

  lastButtonState = currentButtonState;
}

void setServoIfNeeded(Servo &servo, int &lastPos, int newPos) {
  if (abs(newPos - lastPos) >= updateThreshold) {
    servo.write(newPos);
    lastPos = newPos;
  }
}

