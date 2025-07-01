#include <Servo.h>

// Pins
const int servoPins[4] = {3, 5, 6, 9}; // M1, M2, M3, M4
const int joystickXPin = A0;
const int joystickYPin = A1;

// Servo objects
Servo servos[4];
int servoAngles[4] = {90, 90, 90, 90}; // Start centered

// Joystick config
const int deadzone = 75;
const int joystickCenter = 512;
const int joystickStep = 2;

void setup() {
  Serial.begin(9600);

  // Attach and center servos
  for (int i = 0; i < 4; i++) {
    servos[i].attach(servoPins[i]);
    servos[i].write(servoAngles[i]);
  }

  Serial.println("Joystick-only tentacle control initialized.");
}

void loop() {
  int xVal = analogRead(joystickXPin);
  int yVal = analogRead(joystickYPin);

  // Y-axis controls M1 (north) and M2 (south)
  if (abs(yVal - joystickCenter) > deadzone) {
    int direction = (yVal > joystickCenter) ? 1 : -1;
    adjustOpposingPair(0, 1, direction); // M1 tighter, M2 looser
  }

  // X-axis controls M3 (east) and M4 (west)
  if (abs(xVal - joystickCenter) > deadzone) {
    int direction = (xVal > joystickCenter) ? 1 : -1;
    adjustOpposingPair(2, 3, direction); // M3 tighter, M4 looser
  }

  delay(30); // adjust responsiveness
}

void adjustOpposingPair(int tightenIdx, int loosenIdx, int direction) {
  int delta = direction * joystickStep;

  servoAngles[tightenIdx] = constrain(servoAngles[tightenIdx] + delta, 0, 180);
  servoAngles[loosenIdx]  = constrain(servoAngles[loosenIdx] - delta, 0, 180);

  servos[tightenIdx].write(servoAngles[tightenIdx]);
  servos[loosenIdx].write(servoAngles[loosenIdx]);

  Serial.print("M"); Serial.print(tightenIdx + 1);
  Serial.print(" = "); Serial.print(servoAngles[tightenIdx]);
  Serial.print(" | M"); Serial.print(loosenIdx + 1);
  Serial.print(" = "); Serial.println(servoAngles[loosenIdx]);
}
