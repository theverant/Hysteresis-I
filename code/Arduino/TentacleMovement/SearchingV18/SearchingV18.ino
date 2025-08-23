/*
  Hysteresis Servo Control V19 - Pure Search Behavior
  Created by Theverant with essential help from Claude
  Date: 2025-07-31
  License: GPLv3

  H1 Pure Searching Behavior:
  - Continuous wandering across full 0-180 coordinate space
  - Edge-biased exploration with gentle center avoidance
  - Simple velocity-based field evolution with boundary bouncing
  - No cycles, phases, or memory - just perpetual searching motion
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
float baseMovementSpeed = 0.015;

// Field exploration states
float xFieldState = 0.5;  // Start at center of field
float yFieldState = 0.5;

unsigned long lastUpdate = 0;
unsigned long updateInterval = 30;

void setup() {
  servoX.attach(servoXPin);
  servoY.attach(servoYPin);
  Serial.begin(9600);
  Serial.println("H1 Search V19 - Pure searching behavior");
  
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
  // Simple wandering across full coordinate space
  static float xVel = 0.03;
  static float yVel = 0.025;
  
  // Update field positions
  xFieldState += xVel;
  yFieldState += yVel;
  
  // Boundary bouncing with slight velocity changes
  if (xFieldState > 1.0 || xFieldState < 0.0) {
    xVel = -xVel * randomFloat(0.9, 1.1);
    xFieldState = constrain(xFieldState, 0.0, 1.0);
  }
  if (yFieldState > 1.0 || yFieldState < 0.0) {
    yVel = -yVel * randomFloat(0.9, 1.1);
    yFieldState = constrain(yFieldState, 0.0, 1.0);
  }
  
  // Map directly to servo coordinates
  xTarget = xFieldState * 180.0;
  yTarget = yFieldState * 180.0;
  
  // Gentle center avoidance - not blocking, just biasing
  float centerDist = sqrt(pow(xTarget - 90.0, 2) + pow(yTarget - 90.0, 2));
  if (centerDist < 25.0) {
    float pushX = (xTarget - 90.0) / centerDist;
    float pushY = (yTarget - 90.0) / centerDist;
    float pushForce = (25.0 - centerDist) * 0.8;
    
    xTarget += pushX * pushForce;
    yTarget += pushY * pushForce;
  }
  
  // Constrain to valid range
  xTarget = constrain(xTarget, 0, 180);
  yTarget = constrain(yTarget, 0, 180);
}

void updateCircadianCycle() {
  unsigned long now = millis();
  unsigned long phaseElapsed = now - phaseStartTime;
  
  // Update arousal level with slow, methodical evolution
  arousalLevel += arousalVelocity;
  arousalLevel = constrain(arousalLevel, 0.0, 1.0);
  
  // Phase transitions following biological hysteresis - extended durations
  switch (currentPhase) {
    case RESTING:
      if (phaseElapsed > random(45000, 80000)) {  // 45-80 second rest periods
        currentPhase = AWAKENING;
        arousalVelocity = 0.001;  // Very gradual awakening
        phaseStartTime = now;
      }
      arousalLevel = max(0.0, arousalLevel - 0.002);  // Slow decay toward deep rest
      break;
      
    case AWAKENING:
      if (arousalLevel > 0.6) {  // Transition to hunting at moderate arousal
        currentPhase = HUNTING;
        arousalVelocity = 0.0005;  // Barely perceptible increase
        phaseStartTime = now;
      }
      break;
      
    case HUNTING:
      if (phaseElapsed > random(90000, 180000)) {  // 1.5-3 minute hunting periods
        currentPhase = TIRING;
        arousalVelocity = -0.0008;  // Begin slow fatigue
        phaseStartTime = now;
      }
      arousalLevel = min(1.0, arousalLevel + 0.0003);  // Gentle maintenance of focus
      break;
      
    case TIRING:
      if (arousalLevel < 0.1) {  // Return to rest when exhausted
        currentPhase = RESTING;
        arousalVelocity = -0.001;  // Continue toward deep rest
        phaseStartTime = now;
      }
      break;
  }
  
  // Generate movement targets based on arousal state
  generateMovementTargets();
}

void generateMovementTargets() {
  if (currentPhase == RESTING) {
    // Electromagnetic breathing - slow drift around rest sanctuary
    unsigned long now = millis();
    float restDrift = 8.0;  // Small oscillation radius around center
    float driftSpeed = 0.0005;
    
    float xDrift = sin(now * driftSpeed) * restDrift;
    float yDrift = cos(now * driftSpeed * 1.3) * restDrift;  // Different frequencies
    
    xTarget = 90.0 + xDrift;
    yTarget = 90.0 + yDrift;
    
  } else {
    // Active field exploration with memory-guided curiosity
    float fieldVelocity = baseFieldVelocity * (1.0 + arousalLevel * 2.0);
    
    // Memory-influenced field evolution - bias toward unexplored regions
    float explorationBias = calculateExplorationBias(xFieldState, yFieldState);
    
    xFieldState += fieldVelocity * randomFloat(0.8, 1.2) * (1.0 + explorationBias);
    yFieldState += fieldVelocity * randomFloat(0.8, 1.2) * (1.0 + explorationBias);
    
    // Boundary reflections
    if (xFieldState > 1.0 || xFieldState < 0.0) {
      xFieldState = constrain(xFieldState, 0.0, 1.0);
    }
    if (yFieldState > 1.0 || yFieldState < 0.0) {
      yFieldState = constrain(yFieldState, 0.0, 1.0);
    }
    
    // Map field states to movement range based on arousal
    float explorationRange = arousalLevel * 70.0;  // 0-70° range from center
    
    // Awakening: gentle emergence from center
    if (currentPhase == AWAKENING) {
      xTarget = 90.0 + (xFieldState - 0.5) * explorationRange * 0.5;
      yTarget = 90.0 + (yFieldState - 0.5) * explorationRange * 0.5;
    }
    // Hunting: full peripheral exploration guided by memory
    else if (currentPhase == HUNTING) {
      xTarget = (xFieldState < 0.5) ? 
        map(xFieldState * 1000, 0, 500, 20, 70) :
        map(xFieldState * 1000, 500, 1000, 110, 160);
      yTarget = (yFieldState < 0.5) ?
        map(yFieldState * 1000, 0, 500, 30, 75) :
        map(yFieldState * 1000, 500, 1000, 105, 150);
    }
    // Tiring: contracting exploration radius toward familiar zones
    else if (currentPhase == TIRING) {
      float contractionFactor = arousalLevel;
      float familiaritySeek = findNearestFamiliarZone(xAngle, yAngle);
      xTarget = 90.0 + (xTarget - 90.0) * contractionFactor * (1.0 + familiaritySeek);
      yTarget = 90.0 + (yTarget - 90.0) * contractionFactor * (1.0 + familiaritySeek);
    }
  }
  
  // Record current position in electromagnetic archaeology
  recordLandscapeMemory(xTarget, yTarget);
  
  // Safety constraints
  xTarget = constrain(xTarget, 10, 170);
  yTarget = constrain(yTarget, 20, 160);
}

void recordLandscapeMemory(float x, float y) {
  unsigned long now = millis();
  
  // Check if we're near an existing memory
  for (int i = 0; i < MEMORY_CAPACITY; i++) {
    float distance = sqrt(pow(landscapeMemory[i].x - x, 2) + pow(landscapeMemory[i].y - y, 2));
    if (distance < 15.0 && landscapeMemory[i].lastVisited > 0) {  // Within 15° radius
      // Update existing memory
      landscapeMemory[i].lastVisited = now;
      landscapeMemory[i].familiarity = min(1.0, landscapeMemory[i].familiarity + 0.05);
      landscapeMemory[i].satisfaction = calculateSatisfaction(x, y, currentPhase);
      return;
    }
  }
  
  // Create new memory node
  landscapeMemory[memoryIndex] = {x, y, now, 0.1, calculateSatisfaction(x, y, currentPhase)};
  memoryIndex = (memoryIndex + 1) % MEMORY_CAPACITY;
}

float calculateSatisfaction(float x, float y, ActivityPhase phase) {
  // Satisfaction based on position optimality for current phase
  if (phase == RESTING) {
    // Satisfaction increases toward center sanctuary
    float centerDistance = sqrt(pow(x - 90.0, 2) + pow(y - 90.0, 2));
    return max(0.0, 1.0 - centerDistance / 80.0);
  } else {
    // Satisfaction increases toward edges during active phases
    float edgeDistance = min(min(abs(x - 20), abs(x - 160)), min(abs(y - 30), abs(y - 150)));
    return min(1.0, edgeDistance / 50.0);
  }
}

float calculateExplorationBias(float fieldX, float fieldY) {
  // Convert field coordinates to servo space for memory lookup
  float testX = 90.0 + (fieldX - 0.5) * 140.0;
  float testY = 90.0 + (fieldY - 0.5) * 120.0;
  
  // Find nearest memory - boost bias for unexplored regions
  float nearestMemoryDistance = 180.0;  // Max possible distance
  for (int i = 0; i < MEMORY_CAPACITY; i++) {
    if (landscapeMemory[i].lastVisited > 0) {
      float distance = sqrt(pow(landscapeMemory[i].x - testX, 2) + pow(landscapeMemory[i].y - testY, 2));
      nearestMemoryDistance = min(nearestMemoryDistance, distance);
    }
  }
  
  // Return exploration bias - higher for distant/unexplored areas
  return constrain(nearestMemoryDistance / 60.0, 0.0, 0.8);
}

float findNearestFamiliarZone(float x, float y) {
  float maxFamiliarity = 0.0;
  float bestDirection = 0.0;
  
  for (int i = 0; i < MEMORY_CAPACITY; i++) {
    if (landscapeMemory[i].familiarity > 0.5) {  // Familiar zones
      float distance = sqrt(pow(landscapeMemory[i].x - x, 2) + pow(landscapeMemory[i].y - y, 2));
      if (distance < 50.0 && landscapeMemory[i].familiarity > maxFamiliarity) {
        maxFamiliarity = landscapeMemory[i].familiarity;
        bestDirection = atan2(landscapeMemory[i].y - y, landscapeMemory[i].x - x);
      }
    }
  }
  
  return maxFamiliarity * 0.3;  // Gentle bias toward familiar territory when tiring
}

void updateMovement() {
  // Smooth interpolation toward search targets
  float searchSpeed = baseMovementSpeed * 2.0;  // Consistent search speed
  
  xAngle += (xTarget - xAngle) * searchSpeed;
  yAngle += (yTarget - yAngle) * searchSpeed;
  
  // Apply servo positions
  servoX.write(constrain(xAngle, 0, 180));
  servoY.write(constrain(yAngle, 0, 180));
  
  // Simple telemetry
  Serial.print("SEARCH  X: "); Serial.print((int)xAngle);
  Serial.print("  Y: "); Serial.print((int)yAngle);
  Serial.print("  Target X: "); Serial.print((int)xTarget);
  Serial.print("  Y: "); Serial.println((int)yTarget);
}

void decayLandscapeMemory() {
  unsigned long now = millis();
  
  // Archaeological entropy - memories fade without reinforcement
  for (int i = 0; i < MEMORY_CAPACITY; i++) {
    if (landscapeMemory[i].lastVisited > 0) {
      unsigned long timeSinceVisit = now - landscapeMemory[i].lastVisited;
      
      if (timeSinceVisit > memoryDecayRate) {
        // Gradual forgetting - familiarity and satisfaction decay
        float decayFactor = 1.0 - ((timeSinceVisit - memoryDecayRate) / (float)(memoryDecayRate * 3));
        decayFactor = constrain(decayFactor, 0.0, 1.0);
        
        landscapeMemory[i].familiarity *= decayFactor;
        landscapeMemory[i].satisfaction *= decayFactor;
        
        // Complete forgetting threshold
        if (landscapeMemory[i].familiarity < 0.05) {
          landscapeMemory[i] = {0, 0, 0, 0.0, 0.0};  // Tabula rasa
        }
      }
    }
  }
}

int countActiveMemories() {
  int count = 0;
  for (int i = 0; i < MEMORY_CAPACITY; i++) {
    if (landscapeMemory[i].lastVisited > 0) count++;
  }
  return count;
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