#include <Servo.h>

// Servo pins
const int servoXPin = 3;
const int servoYPin = 5;

Servo servoX, servoY;

void setup() {
  servoX.attach(servoXPin);
  servoY.attach(servoYPin);

  // Move both servos to center (90°)
  servoX.write(90);
  servoY.write(90);
}

void loop() {
  // Nothing — keep holding position
}
