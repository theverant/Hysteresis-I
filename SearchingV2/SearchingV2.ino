/*
Code produced by Theverant with essential help from ChatGPT  
2025-07-25  
No rights reserved.  GPLv3  
Version 13-alt2  

Changelog:  
- V13-alt2: Smoother motion, removed jitter  
            Prioritizes base rotation with slow nested sine sweeps  
            Organic weaving, occasional reversals and pauses  
*/

#include <Servo.h>

// Servo pins
const int servoXPin = 3;
const int servoYPin = 5;

// Servo control
Servo servoX, servoY;
int angleX = 90, angleY = 90;

// Motion parameters
float basePhase = 0;
float axialPhase = 0;

float baseSpeed = 0.0012;       // rad/ms
float axialSpeed = 0.0005;

float baseRadius = 35;          // primary circular sweep
float axialRadius = 20;         // secondary sway

bool reverseSweep = false;

// Pause behavior
unsigned long lastMoveTime = 0;
unsigned long lastPauseCheck = 0;
unsigned long pauseStartTime = 0;
bool isPaused = false;
unsigned long pauseDuration = 0;

void setup() {
  Serial.begin(9600);
  randomSeed(analogRead(A3));
  servoX.attach(servoXPin);
  servoY.attach(servoYPin);
  servoX.write(90);
  servoY.write(90);
  Serial.println("Tentacle controller | V13-alt2 | Smooth base-centric motion");
}

void loop() {
  unsigned long now = millis();

  // Handle random pauses
  if (isPaused) {
    if (now - pauseStartTime > pauseDuration) {
      isPaused = false;
    } else {
      return;
    }
  } else if (now - lastPauseCheck > 8000) {
    lastPauseCheck = now;
    if (random(100) < 10) {
      isPaused = true;
      pauseDuration = random(500, 1500);
      pauseStartTime = now;
      return;
    }
  }

  // Occasionally reverse direction
  if (random(10000) < 3) {
    reverseSweep = !reverseSweep;
  }

  // Update phases
  float direction = reverseSweep ? -1 : 1;
  basePhase += baseSpeed * direction * (now - lastMoveTime);
  axialPhase += axialSpeed * (now - lastMoveTime);
  lastMoveTime = now;

  // Calculate target angles
  float xOffset = sin(basePhase) * baseRadius;
  float yOffset = cos(basePhase) * baseRadius;
  float axialWiggle = sin(axialPhase) * axialRadius;

  angleX = constrain(90 + (int)xOffset, 0, 180);
  angleY = constrain(90 + (int)(yOffset + axialWiggle), 0, 180);

  servoX.write(angleX);
  servoY.write(angleY);

  // Serial debug occasionally
  if (now % 500 < 20) {
    Serial.print("X: "); Serial.print(angleX);
    Serial.print(" | Y: "); Serial.println(angleY);
  }

  delay(15); // pacing
}
