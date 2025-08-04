/*
  Hysteresis Servo Control V20 - Contemplative Search
  Created by Theverant with essential help from Claude
  Date: 2025-07-31
  License: GPLv3

  H1 Contemplative Searching:
  - Slow, methodical movement (4x slower than V19)
  - Discrete steps optimized for MG90S servo resolution
  - 80ms update intervals for deliberate motion
  - Adaptive interpolation speed based on movement distance
*/

#include <Servo.h>

Servo servoX;
Servo servoY;

const int servoXPin = 3;
const int servoYPin = 5;

float xAngle = 90.0;      // Start at center
float yAngle = 90.0;
float xTarget = 90.0;
float yTarget = 90.0;

// Movement parameters
float baseMovementSpeed = 0.006;  // 4x slower interpolation speed

// Field exploration states
float xFieldState = 0.5;  // Start at center of field
float yFieldState = 0.5;

unsigned long lastUpdate = 0;
unsigned long updateInterval = 80;  // Even slower updates for contemplative movement

void setup() {
  servoX.attach(servoXPin);
  servoY.attach(servoYPin);
  Serial.begin(9600);
  Serial.println("H1 Search V20 - Contemplative movement");
  
  // Initialize at center
  servoX.write(90);
  servoY.write(90);
}

void loop() {
  unsigned long now = millis();
  
  handleSerial();
  
  if (now - lastUpdate > updateInterval) {
    lastUpdate = now;
    updateSearching();
    updateMovement();
  }
}

void updateSearching() {
  // Slower discrete movement steps
  static float xVel = 0.004;   // Slower but still resolvable
  static float yVel = 0.003;  
  static unsigned long lastDirectionChange = 0;
  
  // Prevent velocity death - maintain minimum energy
  if (abs(xVel) < 0.0015) xVel = randomFloat(-0.004, 0.004);
  if (abs(yVel) < 0.0015) yVel = randomFloat(-0.003, 0.003);
  
  // Frequent hesitation and direction changes
  if (millis() - lastDirectionChange > random(3000, 8000)) {  // Longer intervals
    xVel *= randomFloat(-1.2, 1.2);
    yVel *= randomFloat(-1.2, 1.2);
    lastDirectionChange = millis();
  }
  
  // Update field positions with discrete steps
  xFieldState += xVel;
  yFieldState += yVel;
  
  // Boundary bouncing with less momentum loss
  if (xFieldState > 1.0 || xFieldState < 0.0) {
    xVel = -xVel * randomFloat(0.85, 1.0);
    xFieldState = constrain(xFieldState, 0.0, 1.0);
  }
  if (yFieldState > 1.0 || yFieldState < 0.0) {
    yVel = -yVel * randomFloat(0.85, 1.0);
    yFieldState = constrain(yFieldState, 0.0, 1.0);
  }
  
  // Map to servo coordinates
  xTarget = xFieldState * 180.0;
  yTarget = yFieldState * 180.0;
  
  // Strong center repulsion
  float centerDist = sqrt(pow(xTarget - 90.0, 2) + pow(yTarget - 90.0, 2));
  if (centerDist < 45.0) {
    float pushX = (xTarget - 90.0) / centerDist;
    float pushY = (yTarget - 90.0) / centerDist;
    float pushForce = (45.0 - centerDist) * 2.0;
    
    xTarget += pushX * pushForce;
    yTarget += pushY * pushForce;
  }
  
  // Constrain to valid range
  xTarget = constrain(xTarget, 0, 180);
  yTarget = constrain(yTarget, 0, 180);
}

void updateMovement() {
  // Smooth interpolation with easing
  float distance = sqrt(pow(xTarget - xAngle, 2) + pow(yTarget - yAngle, 2));
  
  // Adaptive speed based on distance - slower for fine positioning
  float dynamicSpeed = baseMovementSpeed;
  if (distance > 5.0) {
    dynamicSpeed *= 2.0;  // Faster for large movements
  } else if (distance < 2.0) {
    dynamicSpeed *= 0.7;  // Slower for fine positioning
  }
  
  // Smooth interpolation with easing
  xAngle += (xTarget - xAngle) * dynamicSpeed;
  yAngle += (yTarget - yAngle) * dynamicSpeed;
  
  // Apply servo positions
  servoX.write(constrain(xAngle, 0, 180));
  servoY.write(constrain(yAngle, 0, 180));
  
  // Simple telemetry
  Serial.print("SEARCH  X: "); Serial.print((int)xAngle);
  Serial.print("  Y: "); Serial.print((int)yAngle);
  Serial.print("  Target X: "); Serial.print((int)xTarget);
  Serial.print("  Y: "); Serial.println((int)yTarget);
}

void handleSerial() {
  if (Serial.available() >= 2) {
    char axis = Serial.read();
    int val = Serial.parseInt();
    if (val >= 0 && val <= 180) {
      if (axis == 'X') {
        xTarget = val;
        xAngle = val;
      } else if (axis == 'Y') {
        yTarget = val;
        yAngle = val;
      }
      Serial.print("Manual override: ");
      Serial.print(axis); Serial.print(" -> "); Serial.println(val);
    }
  }
}

float randomFloat(float minVal, float maxVal) {
  return minVal + (maxVal - minVal) * random(0, 1000) / 1000.0;
}