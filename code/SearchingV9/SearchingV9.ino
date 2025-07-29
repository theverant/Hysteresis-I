/*
  Searching V10
  Created by Theverant with essential help from ChatGPT
  Date: 2025-07-27
  License: GPLv3

  Improvements:
  - Full base servo sweep from 30° to 150°
  - Smooth easing of base and axial positions
  - Axial tip height varies smoothly between low (30°) and high (135°)
  - Sweeping direction and speed random but transitions smoothly
  - Motor angles sometimes reach near physical limits for natural range
*/

#include <Servo.h>

// Pins
const int servoXPin = 3;  // Base servo
const int servoYPin = 5;  // Axial servo

Servo servoX;
Servo servoY;

// Limits and parameters
const int baseMin = 30;
const int baseMax = 150;
const int axialMin = 30;
const int axialMax = 150;

float baseAngle = baseMin;
float baseTarget = baseMax;
float axialAngle = axialMax;
float axialTarget = axialMin;

float baseSpeed = 0.02;   // Movement increment per update, adjustable
float axialSpeed = 0.015;

unsigned long lastUpdate = 0;
const int updateInterval = 25; // ms

// Sweep control
bool baseIncreasing = true;
unsigned long sweepChangeTime = 0;
unsigned long sweepDuration = 0;

bool axialIncreasing = false;
unsigned long axialChangeTime = 0;
unsigned long axialDuration = 0;

void setup() {
  servoX.attach(servoXPin);
  servoY.attach(servoYPin);
  randomSeed(analogRead(A3));

  baseAngle = random(baseMin, baseMax);
  baseTarget = (baseIncreasing) ? baseMax : baseMin;
  axialAngle = random(axialMin, axialMax);
  axialTarget = (axialIncreasing) ? axialMax : axialMin;

  scheduleNextSweep();
  scheduleNextAxial();
}

void loop() {
  unsigned long now = millis();
  if (now - lastUpdate >= updateInterval) {
    lastUpdate = now;

    // Smooth base motion towards target
    baseAngle = moveTowards(baseAngle, baseTarget, baseSpeed);
    servoX.write(baseAngle);

    // Smooth axial motion towards target
    axialAngle = moveTowards(axialAngle, axialTarget, axialSpeed);
    servoY.write(axialAngle);

    // Check if time to change base sweep direction/speed
    if (now > sweepChangeTime) {
      baseIncreasing = !baseIncreasing;
      baseTarget = baseIncreasing ? baseMax : baseMin;
      baseSpeed = randomFloat(0.007, 0.02);
      scheduleNextSweep();
    }

    // Check if time to change axial rise/drop
    if (now > axialChangeTime) {
      axialIncreasing = !axialIncreasing;
      axialTarget = axialIncreasing ? axialMax : axialMin;
      axialSpeed = randomFloat(0.005, 0.015);
      scheduleNextAxial();
    }
  }
}

float moveTowards(float current, float target, float step) {
  if (abs(target - current) <= step) return target;
  return (current < target) ? current + step : current - step;
}

void scheduleNextSweep() {
  sweepDuration = random(4000, 12000);
  sweepChangeTime = millis() + sweepDuration;
}

void scheduleNextAxial() {
  axialDuration = random(3000, 9000);
  axialChangeTime = millis() + axialDuration;
}

float randomFloat(float minVal, float maxVal) {
  return minVal + (random(1000) / 1000.0f) * (maxVal - minVal);
}
