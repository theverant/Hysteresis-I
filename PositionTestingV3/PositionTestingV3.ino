/*
  Searching V9
  Created by Theverant with essential help from ChatGPT
  Date: 2025-07-26
  License: GPLv3

  Features:
  - Base servo sweeps smoothly back and forth with randomized direction, speed, and duration
  - Axial servo (tip) moves in elliptical pattern, mostly low but occasionally rises
  - Smooth, organic motion with no abrupt jumps
  - Parameters for speed, randomness, sweep limits, and tip height control
*/

#include <Servo.h>

// Pins
const int servoXPin = 3;  // Base servo
const int servoYPin = 5;  // Axial servo

Servo servoX;
Servo servoY;

// Parameters
float baseCenter = 135;        // Center base angle (~3 o'clock)
float baseSweepRadius = 20;    // Max deviation from center for base sweep
float baseSweepSpeedMin = 0.0015; // Min speed for base sweep
float baseSweepSpeedMax = 0.004;  // Max speed for base sweep
int baseSweepDurationMin = 4000;  // Min time in ms before changing sweep direction
int baseSweepDurationMax = 9000;  // Max time in ms before changing sweep direction

float axialCenterLow = 110;      // Low axial median position
float axialCenterHigh = 135;     // High axial median position
float axialRadiusX = 15;         // Axial ellipse radius sideways
float axialRadiusY = 8;          // Axial ellipse radius vertical (bend over)
float axialRiseChance = 0.005;   // Chance per update to rise in axial median height
float axialRiseDurationMin = 2000; // Min duration of high axial position (ms)
float axialRiseDurationMax = 5000; // Max duration of high axial position (ms)

float movementSpeed = 0.006;     // Controls speed of elliptical motion

// State variables
float baseAngle = baseCenter;
float baseSweepSpeed = 0;
int baseSweepDirection = 1;  // 1 or -1
unsigned long baseSweepEndTime = 0;

float axialMedian = axialCenterLow;
unsigned long axialRiseEndTime = 0;

float t = 0;  // time for elliptical motion

unsigned long lastUpdate = 0;
const int updateInterval = 20;

void setup() {
  servoX.attach(servoXPin);
  servoY.attach(servoYPin);

  randomSeed(analogRead(A3));  // seed random from unconnected analog pin

  resetBaseSweep();
  axialRiseEndTime = 0; // not rising at start

  servoX.write(baseAngle);
  servoY.write(axialMedian);
}

void loop() {
  unsigned long now = millis();
  if (now - lastUpdate >= updateInterval) {
    lastUpdate = now;

    updateBaseSweep(now);
    updateAxialRise(now);
    updateEllipseMotion();

    servoX.write(constrain(baseAngle, 0, 180));
    servoY.write(constrain(computeAxialAngle(), 0, 180));
  }
}

// --- Functions ---

void resetBaseSweep() {
  baseSweepDirection = random(0, 2) * 2 - 1; // random 1 or -1
  baseSweepSpeed = randomFloat(baseSweepSpeedMin, baseSweepSpeedMax);
  baseSweepEndTime = millis() + random(baseSweepDurationMin, baseSweepDurationMax);
}

void updateBaseSweep(unsigned long now) {
  if (now > baseSweepEndTime) {
    resetBaseSweep();
  }
  baseAngle += baseSweepSpeed * baseSweepDirection * updateInterval;
  float leftLimit = baseCenter - baseSweepRadius;
  float rightLimit = baseCenter + baseSweepRadius;
  if (baseAngle < leftLimit) {
    baseAngle = leftLimit;
    baseSweepDirection = 1;
    resetBaseSweep();
  } else if (baseAngle > rightLimit) {
    baseAngle = rightLimit;
    baseSweepDirection = -1;
    resetBaseSweep();
  }
}

void updateAxialRise(unsigned long now) {
  if (axialRiseEndTime == 0 && randomFloat(0,1) < axialRiseChance) {
    axialMedian = axialCenterHigh;
    axialRiseEndTime = now + random(axialRiseDurationMin, axialRiseDurationMax);
  }
  if (axialRiseEndTime != 0 && now > axialRiseEndTime) {
    axialMedian = axial
