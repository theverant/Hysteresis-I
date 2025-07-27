#include <Servo.h>

const int servoXPin = 3; // Base servo
const int servoYPin = 5; // Axial servo

Servo servoX;
Servo servoY;

// Fixed base at 3 o’clock (135 degrees)
const int basePosition = 135;

// Axial motion parameters
const float axialCenterX = 90;   // Center for axial X servo movement
const float axialCenterY = 90;   // Center for axial Y servo movement
const float axialRadiusX = 15;   // Radius in X axis (sideways)
const float axialRadiusY = 8;    // Smaller radius in Y axis (forward/backwards tilt)
const float movementSpeed = 0.006;

float t = 0;
unsigned long lastUpdate = 0;
const int updateInterval = 20;

void setup() {
  servoX.attach(servoXPin);
  servoY.attach(servoYPin);

  // Fix base servo at 3 o'clock position
  servoX.write(basePosition);
}

void loop() {
  unsigned long now = millis();
  if (now - lastUpdate >= updateInterval) {
    lastUpdate = now;

    t += movementSpeed * 360;
    if (t >= 360) t -= 360;

    float rad = radians(t);

    // Elliptical axial motion to simulate tilt
    float offsetX = cos(rad) * axialRadiusX;
    float offsetY = sin(rad) * axialRadiusY;

    int axialX = constrain(axialCenterX + offsetX, 0, 180);
    int axialY = constrain(axialCenterY + offsetY, 0, 180);

    // Base is fixed; axial servo moves in elliptical (tilted) circle
    servoX.write(basePosition);
    servoY.write(axialY);
  }
}
