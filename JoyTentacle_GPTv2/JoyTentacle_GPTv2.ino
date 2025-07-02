/*
Code produced by Theverant with the help of ChatGPT
2025-07-01
no rights reserved.  GPLv3
Version 13

This version uses XY plotting to determine all 4 motors position, instead of tethering X to 1-2 and Y to 3-4.  Smoother corners.

Changelog:
- V1: Corrected axis mapping for true independent X/Y blending
*/

#include <Servo.h>

// Pins for servos | v1
const int servoPins[4] = {3, 5, 6, 9};

// Joystick input pins | v1
const int joystickXPin = A0;
const int joystickYPin = A1;

// Servo state: current positions | v1
Servo servos[4];
int servoAngles[4] = {90, 90, 90, 90};  

// Servo targets to enable interruptible movement | v8
int targetAngles[4] = {90, 90, 90, 90};  

// Joystick calibration values | v3
const int joystickMin = 0;
const int joystickMax = 1023;
const int joystickXCenter = 527;
const int joystickYCenter = 512;

// Deadzone to ignore small joystick noise | v4
const int deadzoneRange = 15;

// Movement speed configuration | v9
const int moveDelay = 30;         // Time in ms between servo movement steps | v11
const int moveStepSize = 15;      // Degrees to move per step; higher is faster but less smooth | v11
const int angleThreshold = 4;     // Ignore small angle differences to reduce chatter | v7

// Timing for non-blocking updates | v8
unsigned long lastMoveTime = 0;

void setup() {
  Serial.begin(9600);

  // Attach servos and set initial positions | v1
  for (int i = 0; i < 4; i++) {
    servos[i].attach(servoPins[i]);
    servos[i].write(servoAngles[i]);
  }

  Serial.println("Joystick-mapped tentacle control with independent X/Y mapping | v13");
}

void loop() {
  // 1. Read joystick values | v3
  int xVal = analogRead(joystickXPin);
  int yVal = analogRead(joystickYPin);

  // 2. Determine if joystick axes are in their deadzones | v4
  bool xInDeadzone = inDeadzone(xVal, joystickXCenter);
  bool yInDeadzone = inDeadzone(yVal, joystickYCenter);

  // 3. Immediate centering if both axes are in deadzone | v10
  if (xInDeadzone && yInDeadzone) {
    for (int i = 0; i < 4; i++) {
      servoAngles[i] = 90;
      targetAngles[i] = 90;
      servos[i].write(90);
    }
    printStatus(xVal, yVal);
    delay(10);
    return; // Skip further movement this loop
  }

  // 4. Calculate independent target angles for X and Y axes | v13
  int xOffset = xInDeadzone ? 0 : mapOffset(xVal, joystickXCenter);
  int yOffset = yInDeadzone ? 0 : mapOffset(yVal, joystickYCenter);

  // Y-axis pair: M1 and M2
  targetAngles[0] = constrain(90 + yOffset, 0, 180);  // M1 tightens when pushing up
  targetAngles[1] = constrain(90 - yOffset, 0, 180);  // M2 loosens when pushing up

  // X-axis pair: M3 and M4
  targetAngles[2] = constrain(90 - xOffset, 0, 180);  // M3 loosens when pushing right
  targetAngles[3] = constrain(90 + xOffset, 0, 180);  // M4 tightens when pushing right

  // 5. Incrementally move servos toward targets | v8
  unsigned long now = millis();
  if (now - lastMoveTime >= moveDelay) {
    for (int i = 0; i < 4; i++) {
      if (abs(servoAngles[i] - targetAngles[i]) > angleThreshold) {
        if (servoAngles[i] < targetAngles[i]) {
          servoAngles[i] += moveStepSize;
          if (servoAngles[i] > targetAngles[i]) servoAngles[i] = targetAngles[i];
        } else {
          servoAngles[i] -= moveStepSize;
          if (servoAngles[i] < targetAngles[i]) servoAngles[i] = targetAngles[i];
        }
        servos[i].write(servoAngles[i]);
      }
    }
    lastMoveTime = now;
  }

  // 6. Print status for debugging | v5
  printStatus(xVal, yVal);

  delay(10);  // Control loop pacing | v8
}

/*
  Check if joystick input is within deadzone
  Prevents small noise around center from triggering movement | v4
*/
bool inDeadzone(int value, int center) {
  return (value >= (center - deadzoneRange) && value <= (center + deadzoneRange));
}

/*
  Map joystick position to signed offset from center
  Returns range approx -45 to +45 after scaling
  Used to independently drive X or Y pair of motors | v13
*/
int mapOffset(int val, int center) {
  int offset = map(val, joystickMin, joystickMax, -90, 90);
  offset = offset / 1.25;  // adjust strength as needed
  return offset;
}

/*
  Print current joystick readings and servo angles to Serial Monitor
  For debugging and calibration feedback | v5
*/
void printStatus(int xVal, int yVal) {
  Serial.print("Joystick X: ");
  Serial.print(xVal);
  Serial.print(" | Y: ");
  Serial.print(yVal);
  Serial.print(" | Servos: ");
  for (int i = 0; i < 4; i++) {
    Serial.print("M");
    Serial.print(i + 1);
    Serial.print("=");
    Serial.print(servoAngles[i]);
    if (i < 3) Serial.print(", ");
  }
  Serial.println();
}
