/*
  Searching V26
  Four-state behavioral system: Rest, Seek, Search, Return
  Created by Theverant with essential help from Claude Opus 4
  Date: 2025-08-03
  License: GPLv3

  Changelog:
  - v26: Four-state system (Rest/Seek/Search/Return) with clear transitions
  - v25: Energy/gravity model - rest position has gravitational pull
  - v24: Two-tier search with major/minor circles, dual memory systems
  - v23: Implemented hysteresis-based dwelling for lonely search behavior
  - v22: Added epicycloid pattern generation for circles within circles
  - v21: Multi-frequency sine wave exploration with organic movement
  - v20: Added Perlin-like noise generation for natural variations
  - v19: Improved smoothing algorithms for servo response
  - v18: Enhanced boundary constraints and limit handling
  - v17: Optimized timing intervals for consistent updates
  - v16: Refactored movement calculation pipeline
  - v15: Added debug output for position tracking
  - v14: Fixed servo pin assignments (X=3, Y=5)
  - v13: Renamed base/axial to X/Y for clarity
  - v12: Implemented smooth searching motion from startup
  - v11: Added manual override via serial terminal
  - v10: Basic random walk implementation
*/

#include <Servo.h>

Servo servoX;
Servo servoY;

const int servoXPin = 3;
const int servoYPin = 5;

// Current positions
float xAngle = 90.0;
float yAngle = 90.0;

// State machine
enum State {
  REST,
  SEEK,
  SEARCH,
  RETURN
};
State currentState = REST;

// Energy system
float energy = 0.0;               // Current energy level (0-100)
const float maxEnergy = 100.0;    // Maximum energy capacity
const float restPosition = 90.0;  // Gravitational center

// Seek state variables
float seekTargetX = 90.0;         // Where we're seeking to
float seekTargetY = 90.0;
float seekPhase = 0.0;            // For winding path
float seekProgress = 0.0;         // 0 to 1 progress

// Major circle parameters
float majorPhase = 0.0;           // Position along major circle
float majorDirection = 1.0;       // 1 or -1 for direction
float currentRadius = 60.0;       // Current search radius

// Minor circle parameters  
float minorPhase = 0.0;           // Position in minor circle
float minorRadius = 12.0;         // Current radius of minor circles
float minorDirection = 1.0;       // Local search direction
const float minorSpeed = 0.15;    // Much faster than major

// Rest state
float restPhase = 0.0;            // For shiver movement

// Reversal tracking
float reversalCounter = 0.0;      // How far we've reversed
const float maxReversalSectors = 2.0; // Maximum reversal distance
bool inReversal = false;          // Currently in reversal state

// Memory systems
float localMemory = 0.0;          // Local area memory
const float maxLocalMemory = 5.0; // Local behavior change threshold

// Speed parameters
float majorSpeed = 0.0015;        // Base speed for major circle

// Servo limits
const float xMin = 5.0;
const float xMax = 175.0;
const float yMin = 5.0;
const float yMax = 175.0;

// Timing
unsigned long lastUpdate = 0;
const unsigned long updateInterval = 20;

// Function declarations
void printDebug();

void setup() {
  servoX.attach(servoXPin);
  servoY.attach(servoYPin);
  
  Serial.begin(9600);
  Serial.println("Searching V26 - Four State System");
  Serial.println("REST -> SEEK -> SEARCH -> RETURN");
}

void loop() {
  unsigned long now = millis();
  
  if (now - lastUpdate > updateInterval) {
    lastUpdate = now;
    
    switch(currentState) {
      case REST:
        updateRest();
        break;
      case SEEK:
        updateSeek();
        break;
      case SEARCH:
        updateSearch();
        break;
      case RETURN:
        updateReturn();
        break;
    }
    
    printDebug();
  }
}

void updateRest() {
  // Small shiver movement
  restPhase += 0.1;
  float shiverX = restPosition + cos(restPhase) * 3.0;
  float shiverY = restPosition + sin(restPhase * 1.3) * 2.5;
  
  // Regenerate energy
  energy += 0.5;
  
  // Transition to SEEK when energized
  if (energy >= 80.0) {
    // Choose a perimeter target
    float targetAngle = random(0, 628) / 100.0;  // Random angle in radians
    seekTargetX = 90.0 + cos(targetAngle) * 70.0;
    seekTargetY = 90.0 + sin(targetAngle) * 60.0;
    seekTargetX = constrain(seekTargetX, xMin + 10, xMax - 10);
    seekTargetY = constrain(seekTargetY, yMin + 10, yMax - 10);
    
    seekProgress = 0.0;
    seekPhase = 0.0;
    currentState = SEEK;
    Serial.println("REST -> SEEK");
  }
  
  // Apply movement
  xAngle += (shiverX - xAngle) * 0.1;
  yAngle += (shiverY - yAngle) * 0.1;
  
  servoX.write(constrain(xAngle, 0, 180));
  servoY.write(constrain(yAngle, 0, 180));
}

void updateSeek() {
  // Progress toward target with winding path
  seekProgress += 0.005;  // Takes ~4 seconds to reach target
  seekPhase += 0.08;
  
  // Accelerate as we get further from center
  float distFromCenter = sqrt(pow(xAngle - 90, 2) + pow(yAngle - 90, 2));
  float speedBoost = 1.0 + (distFromCenter / 90.0) * 2.0;  // 1x to 3x speed
  
  // Winding path to target
  float windX = sin(seekPhase * 2.1) * 20.0 * (1.0 - seekProgress);
  float windY = cos(seekPhase * 1.7) * 15.0 * (1.0 - seekProgress);
  
  // Interpolate to target with winding
  float targetX = restPosition + (seekTargetX - restPosition) * seekProgress + windX;
  float targetY = restPosition + (seekTargetY - restPosition) * seekProgress + windY;
  
  // Small energy cost for seeking
  energy -= 0.05;
  
  // Transition to SEARCH when arrived
  if (seekProgress >= 1.0) {
    currentState = SEARCH;
    majorPhase = atan2(seekTargetY - 90, seekTargetX - 90);  // Start search at arrival angle
    Serial.println("SEEK -> SEARCH");
  }
  
  // Apply movement with acceleration
  float smoothing = 0.05 * speedBoost;
  xAngle += (targetX - xAngle) * smoothing;
  yAngle += (targetY - yAngle) * smoothing;
  
  servoX.write(constrain(xAngle, 0, 180));
  servoY.write(constrain(yAngle, 0, 180));
}

void updateSearch() {
  // Full search behavior at perimeter
  // Update major circle position
  majorPhase += majorSpeed * majorDirection;
  
  // Handle reversals
  if (inReversal) {
    reversalCounter += abs(majorSpeed);
    
    if (reversalCounter >= random(10, maxReversalSectors * 100) / 100.0) {
      majorDirection *= -1;
      inReversal = false;
      reversalCounter = 0;
    }
  } else {
    if (random(1000) < 8) {
      majorDirection *= -1;
      inReversal = true;
      reversalCounter = 0;
    }
  }
  
  // Update minor circle
  minorPhase += minorSpeed * minorDirection;
  
  // Local memory changes
  localMemory += 0.1;
  if (localMemory > maxLocalMemory) {
    minorDirection *= -1;
    minorRadius = 8.0 + random(100) / 10.0;
    localMemory = 0;
  }
  
  // Calculate position
  float majorX = cos(majorPhase) * currentRadius;
  float majorY = sin(majorPhase) * currentRadius * 0.8;
  
  float minorX = cos(minorPhase) * minorRadius;
  float minorY = sin(minorPhase) * minorRadius;
  
  float xTarget = 90.0 + majorX + minorX;
  float yTarget = 90.0 + majorY + minorY;
  
  xTarget = constrain(xTarget, xMin, xMax);
  yTarget = constrain(yTarget, yMin, yMax);
  
  // Slow energy drain while searching
  energy -= 0.03;
  
  // Transition to RETURN when exhausted
  if (energy <= 20.0) {
    currentState = RETURN;
    Serial.println("SEARCH -> RETURN");
  }
  
  // Apply movement
  xAngle += (xTarget - xAngle) * 0.12;
  yAngle += (yTarget - yAngle) * 0.12;
  
  servoX.write(constrain(xAngle, 0, 180));
  servoY.write(constrain(yAngle, 0, 180));
}

void updateReturn() {
  // Being pulled back to center while resisting
  // Gravity increases as energy depletes
  float gravityPull = (1.0 - energy / 100.0) * 0.02;  // Stronger pull when tired
  
  // Shrink major radius
  currentRadius -= 0.3;
  if (currentRadius < 10.0) currentRadius = 10.0;
  
  // Continue minor circles but weakening
  minorPhase += minorSpeed * minorDirection * 0.5;  // Half speed
  float resistX = cos(minorPhase) * minorRadius * (energy / 100.0);
  float resistY = sin(minorPhase) * minorRadius * (energy / 100.0);
  
  // Major circle still tries to move but radius shrinking
  majorPhase += majorSpeed * majorDirection * 0.3;
  float majorX = cos(majorPhase) * currentRadius;
  float majorY = sin(majorPhase) * currentRadius * 0.8;
  
  // Combined position with gravity pull
  float xTarget = 90.0 + majorX + resistX;
  float yTarget = 90.0 + majorY + resistY;
  
  // Apply gravity
  xTarget = xTarget * (1.0 - gravityPull) + restPosition * gravityPull;
  yTarget = yTarget * (1.0 - gravityPull) + restPosition * gravityPull;
  
  // Continue energy drain
  energy -= 0.1;
  
  // Check if pulled back to rest
  float distFromRest = sqrt(pow(xAngle - restPosition, 2) + pow(yAngle - restPosition, 2));
  if (distFromRest < 15.0 && energy <= 5.0) {
    currentState = REST;
    energy = 0.0;
    currentRadius = 60.0;  // Reset for next cycle
    Serial.println("RETURN -> REST");
  }
  
  // Apply movement - slower as tired
  float smoothing = 0.08 * (energy / 100.0 + 0.3);
  xAngle += (xTarget - xAngle) * smoothing;
  yAngle += (yTarget - yAngle) * smoothing;
  
  servoX.write(constrain(xAngle, 0, 180));
  servoY.write(constrain(yAngle, 0, 180));
}

// Debug output
void printDebug() {
  static unsigned long lastPrint = 0;
  
  if (millis() - lastPrint > 1000) {
    lastPrint = millis();
    
    Serial.print("State: ");
    switch(currentState) {
      case REST: Serial.print("REST"); break;
      case SEEK: Serial.print("SEEK"); break;
      case SEARCH: Serial.print("SEARCH"); break;
      case RETURN: Serial.print("RETURN"); break;
    }
    
    Serial.print(" | Energy: "); Serial.print(energy, 1);
    Serial.print(" | Pos: "); Serial.print((int)xAngle);
    Serial.print(","); Serial.println((int)yAngle);
  }
}