/*
Code produced by Theverant with essential help from ChatGPT  
2025-07-25  
No rights reserved.  GPLv3  
Version 13-alt  

Changelog:  
- V13-alt: Removed photoresistor input  
           Tentacle performs autonomous organic sweep using circular and axial rotation  
           Random pauses, reversals, and weaving motion included  
*/

#include <Servo.h>

// Servo pins
const int servoXPin = 3;
const int servoYPin = 5;

// Servo control
Servo servoX, servoY;
int angleX = 90, angleY = 90;

// Sweep motion config
float sweepRadius = 30;
float sweepSpeed = 0.001;     // radians/ms
float offsetPhase = 1.57;     // 90 degrees out of phase
bool reverseSweep = false;

unsigned long lastUpdate = 0;
unsigned long lastPauseTime = 0;
unsigned long pauseDuration = 0;
bool isPaused = false;

// Random pause config
const unsigned long minPauseInterval = 3000;
const unsigned long maxPauseInterval = 7000;
const unsigned long minPauseDuration = 300;
const unsigned long maxPauseDuration = 1200;

void setup() {
  Serial.begin(9600);
  while (!Serial) {}
  servoX.attach(servoXPin);
  servoY.attach(servoYPin);
  servoX.write(90);
  servoY.write(90);
  randomSeed(analogRead(A3));  // entropy for randomness
  Serial.println("Tentacle controller | V13-alt | Autonomous sweep mode");
}

void loop() {
  unsigned long now = millis();

  // Pause logic
  if (isPaused) {
    if (now - lastPauseTime >= pauseDuration) {
      isPaused = false;
      lastPauseTime = now;
    } else {
      return;
    }
  } else {
    if (now - lastPauseTime >= random(minPauseInterval, maxPauseInterval)) {
      isPaused = true;
      pauseDuration = random(minPauseDuration, maxPauseDuration);
      lastPauseTime = now;
      return;
    }
  }

  // Occasionally reverse direction
  if (random(0, 10000) < 5) {
    reverseSweep = !reverseSweep;
  }

  // Generate circular phase motion
  float phase = (reverseSweep ? -1 : 1) * (now * sweepSpeed);
  float axialWiggle = sin(now * 0.0004 + offsetPhase * 2.0) * 10;

  float xWave = sin(phase) * sweepRadius + random(-2, 3);
  float yWave = cos(phase + offsetPhase) * sweepRadius + axialWiggle;

  angleX = constrain(90 + (int)xWave, 0, 180);
  angleY = constrain(90 + (int)yWave, 0, 180);

  // Move servos
  if (now - lastUpdate > 15) {
    servoX.write(angleX);
    servoY.write(angleY);
    lastUpdate = now;
  }

  // Optional serial debug
  if (now % 500 < 20) {
    Serial.print("X: ");
    Serial.print(angleX);
    Serial.print(" | Y: ");
    Serial.println(angleY);
  }
}
