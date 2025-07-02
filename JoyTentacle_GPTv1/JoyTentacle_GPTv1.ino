#include <Servo.h>

// Pins
const int servoPins[4] = {3, 5, 6, 9};  // M1, M2, M3, M4
const int joystickXPin = A0;
const int joystickYPin = A1;

// Servo objects
Servo servos[4];
int servoAngles[4] = {90, 90, 90, 90}; // Start centered

// Joystick config
const int joystickMin = 0;
const int joystickMax = 1023;
const int joystickXCenter = 545;
const int joystickYCenter = 558;
const int deadzoneRange = 15;  // ±15 deadzone

void setup() {
  Serial.begin(9600);

  for (int i = 0; i < 4; i++) {
    servos[i].attach(servoPins[i]);
    servos[i].write(servoAngles[i]);
  }

  Serial.println("Joystick-mapped tentacle control with return-to-center in deadzone.");
}

void loop() {
  int xVal = analogRead(joystickXPin);
  int yVal = analogRead(joystickYPin);

  bool xInDeadzone = inDeadzone(xVal, joystickXCenter);
  bool yInDeadzone = inDeadzone(yVal, joystickYCenter);

  if (xInDeadzone && yInDeadzone) {
    // Both axes in deadzone: center all servos
    centerAllServos();
  } else {
    // Y-axis controls M1 & M2 (same as before)
    if (!yInDeadzone) {
      int m1Angle = mapJoystickToServo(yVal);
      int m2Angle = 180 - m1Angle;
      updateServo(0, m1Angle);
      updateServo(1, m2Angle);
    }

    // X-axis controls M3 & M4 (opposite direction)
    if (!xInDeadzone) {
      int mappedX = mapJoystickToServo(xVal);
      int m3Angle = 180 - mappedX;  // invert for M3
      int m4Angle = mappedX;        // normal for M4
      updateServo(2, m3Angle);
      updateServo(3, m4Angle);
    }
  }

  printStatus(xVal, yVal);
  delay(50);
}

bool inDeadzone(int value, int center) {
  return (value >= (center - deadzoneRange) && value <= (center + deadzoneRange));
}

int mapJoystickToServo(int value) {
  value = constrain(value, joystickMin, joystickMax);
  return map(value, joystickMin, joystickMax, 0, 180);
}

void updateServo(int idx, int angle) {
  servoAngles[idx] = angle;
  servos[idx].write(angle);
}

void centerAllServos() {
  for (int i = 0; i < 4; i++) {
    servoAngles[i] = 90;
    servos[i].write(90);
  }
}

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
