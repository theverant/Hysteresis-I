/*
Code produced by Theverant with essential help from ChatGPT
2025-07-25
No rights reserved.  GPLv3
Version 12

Changelog:
- V1: Basic servo centering
- V2: Joystick incremental control
- V3: Joystick direct mapping with deadzone
- V4: Deadzone returns tentacle to center
- V5: Smooth movement added with serial output and axis inversion
- V6: Simultaneous smooth movement of opposing servo pairs
- V7: Added anti-chatter threshold for cleaner motion
- V8: Non-blocking, interruptible movement for responsive control
- V9: User-adjustable speed control (step size and delay)
- V10: Immediate centering when joystick returns to deadzone
- V11: Updated speed settings and improved comments
- V12: Refactored for 2-servo control (X±, Y± directional tension model)
        Removed unused servos and logic
        Preserved smooth movement, deadzone, and anti-chatter features
        Confirmed Pro Micro compatibility with pin and serial setup
*/

#include <Servo.h>

// Servo pins (PWM-capable on Pro Micro)
const int servoXPin = 3;
const int servoYPin = 5;

// Joystick pins
const int joystickXPin = A0;
const int joystickYPin = A1;

// Servo control
Servo servoX, servoY;
int angleX = 90, angleY = 90;
int targetX = 90, targetY = 90;

// Joystick calibration
const int joyMin = 0;
const int joyMax = 1023;
const int joyXCenter = 527;
const int joyYCenter = 512;
const int deadzone = 15;

// Movement tuning
const int stepSize = 20;
const int moveDelay = 15;
const int angleThreshold = 4;

unsigned long lastMove = 0;

void setup() {
  Serial.begin(9600);
  while (!Serial) {}  // Wait for serial connection (important on Pro Micro)
  servoX.attach(servoXPin);
  servoY.attach(servoYPin);
  servoX.write(90);
  servoY.write(90);
  Serial.println("Tentacle controller | V12 | 2-servo directional model");
}

void loop() {
  int xVal = analogRead(joystickXPin);
  int yVal = analogRead(joystickYPin);

  bool xDead = inDeadzone(xVal, joyXCenter);
  bool yDead = inDeadzone(yVal, joyYCenter);

  if (xDead && yDead) {
    targetX = 90;
    targetY = 90;
    angleX = 90;
    angleY = 90;
    servoX.write(90);
    servoY.write(90);
    printStatus(xVal, yVal);
    delay(10);
    return;
  }

  targetX = xDead ? 90 : mapJoystickToServo(xVal);
  targetY = yDead ? 90 : mapJoystickToServo(yVal);

  unsigned long now = millis();
  if (now - lastMove >= moveDelay) {
    if (abs(angleX - targetX) > angleThreshold) {
      angleX += (angleX < targetX) ? stepSize : -stepSize;
      if (abs(angleX - targetX) < stepSize) angleX = targetX;
      servoX.write(constrain(angleX, 0, 180));
    }

    if (abs(angleY - targetY) > angleThreshold) {
      angleY += (angleY < targetY) ? stepSize : -stepSize;
      if (abs(angleY - targetY) < stepSize) angleY = targetY;
      servoY.write(constrain(angleY, 0, 180));
    }

    lastMove = now;
  }

  printStatus(xVal, yVal);
  delay(10);
}

bool inDeadzone(int val, int center) {
  return (val >= center - deadzone && val <= center + deadzone);
}

int mapJoystickToServo(int value) {
  value = constrain(value, joyMin, joyMax);
  return map(value, joyMin, joyMax, 0, 180);
}

void printStatus(int xVal, int yVal) {
  Serial.print("X: "); Serial.print(xVal);
  Serial.print(" | Y: "); Serial.print(yVal);
  Serial.print(" | ServoX: "); Serial.print(angleX);
  Serial.print(" | ServoY: "); Serial.println(angleY);
}
