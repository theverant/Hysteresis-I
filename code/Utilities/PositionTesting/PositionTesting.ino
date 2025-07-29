#include <Servo.h>

const int servoXPin = 3;
const int servoYPin = 5;

Servo servoX;
Servo servoY;

// === PARAMETERS ===
const float basePositionDeg = 135;     // Locked base position (e.g., 3 o'clock)
const float axialCenterX = 90;         // Center point for X servo
const float axialCenterY = 90;         // Center point for Y servo
const float axialCircleSize = 15;      // Radius of the axial circle
const float movementSpeed = 0.006;     // Speed of motion

// === STATE ===
float t = 0.0;
unsigned long lastUpdate = 0;
const int updateInterval = 20;  // ms

void setup() {
  servoX.attach(servoXPin);
  servoY.attach(servoYPin);

  // Position base at 3 o'clock
  servoX.write(basePositionDeg);
}

void loop() {
  unsigned long now = millis();
  if (now - lastUpdate >= updateInterval) {
    lastUpdate = now;

    // Advance time
    t += movementSpeed * 360;
    if (t >= 360.0) t -= 360.0;

    float rad = radians(t);

    // Small circular motion around axial center
    float offsetX = cos(rad) * axialCircleSize;
    float offsetY = sin(rad) * axialCircleSize;

    int xOut = constrain(axialCenterX + offsetX, 0, 180);
    int yOut = constrain(axialCenterY + offsetY, 0, 180);

    servoX.write(xOut);
    servoY.write(yOut);
  }
}
