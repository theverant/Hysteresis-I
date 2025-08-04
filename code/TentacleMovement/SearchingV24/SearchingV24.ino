/*
  Searching V25
  Energy-based hysteresis search with gravitational center
  Created by Theverant with essential help from Claude Opus 4
  Date: 2025-08-03
  License: GPLv3

  Changelog:
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

// Energy system
float energy = 50.0;              // Current energy level (0-100)
const float maxEnergy = 100.0;    // Maximum energy capacity
const float restPosition = 90.0;  // Gravitational center
const float gravityStrength = 20.0; // Gravity constant

// Major circle parameters
float majorPhase = 0.0;           // Position along major circle
float majorDirection = 1.0;       // 1 or -1 for direction
float effectiveRadius = 20.0;     // Actual radius (affected by energy)
const float maxRadius = 75.0;     // Maximum possible radius at full energy

// Minor circle parameters  
float minorPhase = 0.0;           // Position in minor circle
float minorRadius = 12.0;         // Current radius of minor circles
float minorDirection = 1.0;       // Local search direction
const float minorSpeed = 0.15;    // Much faster than major

// Reversal tracking
float reversalCounter = 0.0;      // How far we've reversed
const float maxReversalSectors = 2.0; // Maximum reversal distance
bool inReversal = false;          // Currently in reversal state

// Memory systems
float localMemory = 0.0;          // Local area memory
const float maxLocalMemory = 5.0; // Local behavior change threshold

// Speed parameters
float majorSpeed = 0.001;         // Base speed for major circle

// Servo limits
const float xMin = 0.0;
const float xMax = 180.0;
const float yMin = 0.0;
const float yMax = 180.0;

// Timing
unsigned long lastUpdate = 0;
const unsigned long updateInterval = 20;

void setup() {
  servoX.attach(servoXPin);
  servoY.attach(servoYPin);
  
  Serial.begin(9600);
  Serial.println("Searching V25 - Energy/Gravity Hysteresis");
  Serial.println("Gravitational rest position with energy-based searching");
}

void loop() {
  unsigned long now = millis();
  
  if (now - lastUpdate > updateInterval) {
    lastUpdate = now;
    updateEnergySearch();
  }
}

void updateEnergySearch() {
  // Calculate distance from rest position
  float distFromRest = sqrt(pow(xAngle - restPosition, 2) + pow(yAngle - restPosition, 2));
  
  // Calculate gravitational force (inverse square law)
  float gravityForce = gravityStrength / (1.0 + distFromRest * 0.02);
  
  // Energy dynamics based on position
  if (distFromRest < 10.0) {
    // At rest position - regenerate energy
    energy += 0.3;
    if (energy > maxEnergy) energy = maxEnergy;
  } else if (distFromRest > 30.0) {
    // At edges - efficient gliding, slow energy drain
    energy -= 0.02;
  } else {
    // In between - high energy cost pushing through gravity
    energy -= 0.1;
  }
  
  // Energy affects ability to maintain radius against gravity
  effectiveRadius = (energy / maxEnergy) * maxRadius;
  
  // Apply gravity - pulls radius inward
  float gravityPull = gravityForce * 0.5;
  effectiveRadius -= gravityPull;
  if (effectiveRadius < 10.0) effectiveRadius = 10.0;
  
  // Update major circle position
  majorPhase += majorSpeed * majorDirection;
  
  // Handle reversals
  if (inReversal) {
    reversalCounter += abs(majorSpeed);
    
    // Check if reversal should end
    if (reversalCounter >= random(10, maxReversalSectors * 100) / 100.0) {
      majorDirection *= -1;  // Return to forward
      inReversal = false;
      reversalCounter = 0;
      Serial.println("Reversal complete");
    }
  } else {
    // Check for new reversal based on energy and randomness
    if (energy < 30 && random(100) < 5) {
      // Low energy makes reversals more likely
      majorDirection *= -1;
      inReversal = true;
      reversalCounter = 0;
      Serial.println("Reversal triggered");
    } else if (random(1000) < 5) {
      // Occasional reversal regardless of energy
      majorDirection *= -1;
      inReversal = true;
      reversalCounter = 0;
      Serial.println("Random reversal");
    }
  }
  
  // Update minor circle - always active
  minorPhase += minorSpeed * minorDirection;
  
  // Local memory affects minor circle behavior
  localMemory += 0.1;
  if (localMemory > maxLocalMemory) {
    minorDirection *= -1;
    minorRadius = 8.0 + random(100) / 10.0;
    localMemory = 0;
  }
  
  // Calculate position with energy-affected radius
  float majorX = cos(majorPhase) * effectiveRadius;
  float majorY = sin(majorPhase) * effectiveRadius * 0.8;
  
  // Minor circles - scale down when tired
  float energyScale = 0.5 + (energy / maxEnergy) * 0.5;  // 0.5 to 1.0
  float minorX = cos(minorPhase) * minorRadius * energyScale;
  float minorY = sin(minorPhase) * minorRadius * energyScale;
  
  // Combined target position
  float xTarget = restPosition + majorX + minorX;
  float yTarget = restPosition + majorY + minorY;
  
  // Constrain to servo limits
  xTarget = constrain(xTarget, xMin, xMax);
  yTarget = constrain(yTarget, yMin, yMax);
  
  // Smooth approach - slower when tired
  float smoothing = 0.08 + (energy / maxEnergy) * 0.08;  // 0.08 to 0.16
  xAngle += (xTarget - xAngle) * smoothing;
  yAngle += (yTarget - yAngle) * smoothing;
  
  // Write to servos
  servoX.write(constrain(xAngle, 0, 180));
  servoY.write(constrain(yAngle, 0, 180));
  
  // Debug output
  static unsigned long lastPrint = 0;
  static int updateCount = 0;
  updateCount++;
  
  if (millis() - lastPrint > 1000) {
    lastPrint = millis();
    
    float progressDegrees = majorPhase * 57.2958;
    int sector = ((int)(progressDegrees / 30)) % 12;
    if (sector < 0) sector += 12;
    
    Serial.print("Energy: "); Serial.print(energy, 1);
    Serial.print(" | Radius: "); Serial.print(effectiveRadius, 1);
    Serial.print(" | Sector: "); Serial.print(sector);
    Serial.print(" | Pos: "); Serial.print((int)xAngle);
    Serial.print(","); Serial.print((int)yAngle);
    Serial.print(" | Updates/s: "); Serial.println(updateCount);
    updateCount = 0;
  }
  
  // Keep phases bounded
  if (majorPhase > 6.28318) majorPhase -= 6.28318;
  if (majorPhase < 0) majorPhase += 6.28318;
  if (minorPhase > 6.28318) minorPhase -= 6.28318;
  
  // Ensure energy stays in bounds
  if (energy < 0) energy = 0;
  if (energy > maxEnergy) energy = maxEnergy;
}