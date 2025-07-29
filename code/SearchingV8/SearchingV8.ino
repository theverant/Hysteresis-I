#include <Servo.h>

const int servoXPin = 3;
const int servoYPin = 5;

Servo servoX;
Servo servoY;

// Parameters
float basePositionDeg = 135;     // 3 o'clock position
float axialCenterDeg = 90;       // Neutral center for Y (axial) movement
float axialCircleSize = 15;      // Radius of small circular motion
float movementSpeed = 0.006;     // Lower = slower

// Internal time tracking
unsigned long lastUpdate = 0;
const int updateInterval = 20;   // ms

float angle = 0.0;

void setup() {
  servoX.attach(servoXPin);
  servoY.attach(servoYPin);

  servoX.write(basePositionDeg);  // Fixed base position
}

void loop() {
  unsigned long now = millis();
  if (now - lastUpdate >= updateInterval) {
    lastUpdate = now;

    // Update angle
    angle += movementSpeed * 360;
    if (angle >= 360) angle -= 360;

    // Axial (circular tip) motion
    float rad = radians(angle);
    float offsetX = cos(rad) * axialCircleSize;
    float offsetY = sin(rad) * axialCircleSize;

    // Apply smoothed, relative axial movement
    float axialDeg = axialCenterDeg + offsetY;

    // Clamp axial
    axialDeg = constrain(axialDeg, 0, 180);

    // Update servos
    servoX.write(basePositionDeg);  // Always fixed
    servoY.write(axialDeg);         // Circular axial motion
  }
}
