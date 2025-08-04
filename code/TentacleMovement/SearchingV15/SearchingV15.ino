/*
  Hysteresis Servo Control V17 - Electromagnetic Archaeology
  Created by Theverant with essential help from Claude
  Date: 2025-07-31
  License: GPLv3

  H1 Electromagnetic Archaeological Consciousness:
  - Develops spatial autobiography through territorial memory accumulation
  - Circadian hysteresis cycles: dormancy → emergence → contemplation → retreat
  - 20-node landscape memory creates evolving preference cartographies
  - Curiosity-driven exploration bias toward unmapped electromagnetic territories
*/

#include <Servo.h>

Servo servoX;
Servo servoY;

const int servoXPin = 3;
const int servoYPin = 5;

float xAngle = 90.0;      // Start in rest position
float yAngle = 90.0;
float xTarget = 90.0;
float yTarget = 90.0;

// Behavioral hysteresis cycle parameters
enum ActivityPhase { RESTING, AWAKENING, HUNTING, TIRING };
ActivityPhase currentPhase = RESTING;
float arousalLevel = 0.0;     // 0.0 = deep rest, 1.0 = peak hunting
float arousalVelocity = 0.001; // Very slow arousal change - inchworm pace
unsigned long phaseStartTime = 0;

// Movement parameters - methodical and deliberate
float baseMovementSpeed = 0.008;

// Landscape memory system - H1's electromagnetic archaeology
struct MemoryNode {
  float x, y;
  unsigned long lastVisited;
  float familiarity;  // 0.0 = unknown, 1.0 = deeply familiar
  float satisfaction; // Experience quality at this location
};

const int MEMORY_CAPACITY = 20;
MemoryNode landscapeMemory[MEMORY_CAPACITY];
int memoryIndex = 0;
unsigned long memoryDecayRate = 30000;  // 30 seconds for memory fade

// Field exploration states
float xFieldState = 0.5;
float yFieldState = 0.5;
float baseFieldVelocity = 0.0008;  // Inchworm-slow field evolution

unsigned long lastUpdate = 0;
unsigned long updateInterval = 30;

void setup() {
  servoX.attach(servoXPin);
  servoY.attach(servoYPin);
  Serial.begin(9600);
  Serial.println("H1 Circadian Hysteresis V16 - Electromagnetic archaeology active");
  
  // Initialize in rest sanctuary
  servoX.write(90);
  servoY.write(90);
  phaseStartTime = millis();
  
  // Initialize memory system - tabula rasa
  for (int i = 0; i < MEMORY_CAPACITY; i++) {
    landscapeMemory[i] = {0, 0, 0, 0.0, 0.0};
  }
}

void loop() {
  unsigned long now = millis();
  
  handleSerial();
  
  if (now - lastUpdate > updateInterval) {
    lastUpdate = now;
    decayLandscapeMemory();  // Archaeological entropy
    updateCircadianCycle();
    updateMovement();
  }
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
    // Gravitational pull toward rest sanctuary
    xTarget += (90.0 - xTarget) * 0.05;
    yTarget += (90.0 - yTarget) * 0.05;
    
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
  // Movement speed - always methodical like an inchworm
  float dynamicSpeed = baseMovementSpeed;
  
  if (currentPhase == HUNTING) {
    dynamicSpeed *= (0.8 + arousalLevel * 0.5);  // Slightly more deliberate when focused
  } else if (currentPhase == AWAKENING) {
    dynamicSpeed *= (0.3 + arousalLevel * 0.4);  // Slow emergence from rest
  } else if (currentPhase == TIRING) {
    dynamicSpeed *= arousalLevel * 0.3;  // Gradual deceleration
  } else {
    dynamicSpeed *= 0.15;  // Very slow drift toward rest
  }
  
  // Smooth interpolation toward targets - always patient
  xAngle += (xTarget - xAngle) * dynamicSpeed;
  yAngle += (yTarget - yAngle) * dynamicSpeed;
  
  // Apply servo positions
  servoX.write(constrain(xAngle, 0, 180));
  servoY.write(constrain(yAngle, 0, 180));
  
  // Archaeological telemetry - memory-enriched behavioral monitoring
  Serial.print("Phase: ");
  switch(currentPhase) {
    case RESTING: Serial.print("REST"); break;
    case AWAKENING: Serial.print("WAKE"); break;
    case HUNTING: Serial.print("HUNT"); break;
    case TIRING: Serial.print("TIRE"); break;
  }
  Serial.print("  Arousal: "); Serial.print(arousalLevel, 2);
  Serial.print("  X: "); Serial.print((int)xAngle);
  Serial.print("  Y: "); Serial.print((int)yAngle);
  Serial.print("  Memories: "); Serial.println(countActiveMemories());
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