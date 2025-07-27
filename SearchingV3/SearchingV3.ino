/*
Code produced by Theverant with essential help from ChatGPT
2025-07-25
No rights reserved. GPLv3
Version 14

Changelog:
- V2: Experimental sweeping pattern with layered circular motion
- V3: Tunable movement speed, randomness, axial circle size and variation
*/

#include <Servo.h>
#include <math.h>

// === Servo Setup ===
const int servoXPin = 3;
const int servoYPin = 5;

Servo servoX, servoY;

// === Adjustable Parameters ===
float sweepSpeed = 0.005;       // Speed of sweeping motion (lower = slower)
int randomnessLevel = 5;        // Max random noise added to X/Y angles
int axialSize = 30;             // Size of inner circle wobble (degrees)
int axialJitter = 10;           // +/- deviation allowed in axial circle size

// === Internal State ===
unsigned long lastUpdate = 0;
const int updateInterval = 20;  // ms between servo updates

float baseAngle = 0.0;          // Outer sweep phase
float axialAngle = 0.0;         // Inner rotation phase
int currentAxialSize = axialSize;
unsigned long lastAxialChange = 0;
const unsigned long axialChangeInterval = 3000;

void setup() {
  servoX.attach(servoXPin);
  servoY.attach(servoYPin);

  // Center both servos
  servoX.write(90);
  servoY.write(90);

  randomSeed(analogRead(A0)); // Init RNG
}

void loop() {
  unsigned long now = millis();
  if (now - lastUpdate >= updateInterval) {
    lastUpdate = now;

    // Occasionally update the axial circle size with randomness
    if (now - lastAxialChange > axialChangeInterval) {
      currentAxialSize = axialSize + random(-axialJitter, axialJitter);
      lastAxialChange = now;
    }

    // Advance base and axial phases
    baseAngle += sweepSpeed;
    axialAngle -= sweepSpeed * 2.3;  // Opposing direction for organic feel

    // Circular motion around base
    float outerX = sin(baseAngle) * 40;
    float outerY = cos(baseAngle) * 40;

    // Axial circle wobble
    float axialX = sin(axialAngle) * currentAxialSize;
    float axialY = cos(axialAngle) * currentAxialSize;

    // Combine and add random noise
    int x = 90 + outerX + axialX + random(-randomnessLevel, randomnessLevel);
    int y = 90 + outerY + axialY + random(-randomnessLevel, randomnessLevel);

    x = constrain(x, 0, 180);
    y = constrain(y, 0, 180);

    servoX.write(x);
    servoY.write(y);
  }
}
