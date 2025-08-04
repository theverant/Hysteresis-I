/*
  Hysteresis Servo Control V15
  Created by Theverant with essential help from ChatGPT
  Date: 2025-07-31
  License: GPLv3

  Field-Driven Tentacle Positioning:
  - Electromagnetic domain simulation drives servo positioning
  - Center-avoidance mapping maintains horizontal sculptural poses
  - Hysteresis curves create organic exploration patterns
  - Manual override via serial terminal (e.g., "X120", "Y45")
*/

#include <Servo.h>

Servo servoX;
Servo servoY;

const int servoXPin = 3;  // X = base rotation
const int servoYPin = 5;  // Y = axial tilt

float xAngle = 45.0;      // Start in horizontal position
float yAngle = 45.0;
float xTarget = 45.0;
float yTarget = 45.0;

float movementSpeed = 0.08;

// Hysteresis field simulation
float xDomainState = 0.25;    // Start away from center
float yDomainState = 0.25;
float xDomainVelocity = 0.015;
float yDomainVelocity = 0.022;

unsigned long lastUpdate = 0;
unsigned long updateInterval = 20;

void setup() {
  servoX.attach(servoXPin);
  servoY.attach(servoYPin);
  Serial.begin(9600);
  Serial.println("Hysteresis Tentacle V15 - Field dynamics active");
  
  // Initialize servos to starting position
  servoX.write(xAngle);
  servoY.write(yAngle);
}

void loop() {
  unsigned long now = millis();

  handleSerial();

  if (now - lastUpdate > updateInterval) {
    lastUpdate = now;
    updateFieldDynamics();
    updateMotion();
  }
}

void updateFieldDynamics() {
  // Evolve domain states using hysteresis physics
  xDomainState += xDomainVelocity;
  yDomainState += yDomainVelocity;
  
  // Boundary reflections with organic velocity variation
  if (xDomainState > 1.0 || xDomainState < 0.0) {
    xDomainVelocity = -xDomainVelocity * randomFloat(0.85, 1.15);
    xDomainState = constrain(xDomainState, 0.0, 1.0);
  }
  
  if (yDomainState > 1.0 || yDomainState < 0.0) {
    yDomainVelocity = -yDomainVelocity * randomFloat(0.85, 1.15);
    yDomainState = constrain(yDomainState, 0.0, 1.0);
  }
  
  // Map domain states to edge-biased servo positions
  // Avoids center (90°) to maintain horizontal sculptural poses
  xTarget = xDomainState < 0.5 ? 
    map(xDomainState * 1000, 0, 500, 10, 70) :     // Low range: 10-70°
    map(xDomainState * 1000, 500, 1000, 110, 170); // High range: 110-170°
    
  yTarget = yDomainState < 0.5 ? 
    map(yDomainState * 1000, 0, 500, 10, 70) :     // Low range: 10-70°
    map(yDomainState * 1000, 500, 1000, 110, 170); // High range: 110-170°
}

void updateMotion() {
  // Smooth interpolation toward field-driven targets
  xAngle += (xTarget - xAngle) * movementSpeed;
  yAngle += (yTarget - yAngle) * movementSpeed;

  // Apply servo positions with safety constraints
  servoX.write(constrain(xAngle, 0, 180));
  servoY.write(constrain(yAngle, 0, 180));

  // Status output for field monitoring
  Serial.print("Field X: "); Serial.print((int)xAngle);
  Serial.print("  Y: "); Serial.print((int)yAngle);
  Serial.print("  Domain: "); Serial.print(xDomainState, 3);
  Serial.print(", "); Serial.println(yDomainState, 3);
}

void handleSerial() {
  if (Serial.available() >= 2) {
    char axis = Serial.read();
    int val = Serial.parseInt();
    if (val >= 0 && val <= 180) {
      if (axis == 'X') {
        xTarget = val;
        xAngle = val;  // Immediate position for manual control
      }
      else if (axis == 'Y') {
        yTarget = val;
        yAngle = val;  // Immediate position for manual control
      }