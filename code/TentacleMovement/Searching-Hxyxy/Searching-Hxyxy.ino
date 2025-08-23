/*
  H1XYXY V1 - Arduino Uno Dual XY Implementation
  V31 territorial exploration mapped to compound curvature system
  Created by Theverant based on V31 territorial exploration
  Date: 2025-08-10
  License: GPLv3

  ARDUINO UNO CONSTRAINTS:
  - 2KB RAM requires simplified spatial memory
  - Single core processing for both servo control and behavioral algorithms
  - 16MHz processing limits computational complexity
  - Standard Arduino pin assignments

  DUAL XY SERVO SYSTEM:
  - Base XY: Primary movement control implementing V31 territorial logic
  - Tip XY: Gestural modulation providing organic expression
  - Hierarchical coordination: Base drives exploration, tip adds articulation
  - Compound curvature: Coordinated movement for serpentine patterns

  SIMPLIFIED SPATIAL MEMORY:
  - 6x6 grid instead of 12x12x12 (36 cells vs 1728)
  - 2D exploration with compound curvature expression
  - Maintains V31's core behavioral patterns within memory constraints

  Changelog:
  - v1: Arduino Uno implementation with simplified memory and single-core architecture
*/

#include <Servo.h>

// ===== HARDWARE SETUP =====
Servo servoBaseX;   // Base section X-axis control
Servo servoBaseY;   // Base section Y-axis control  
Servo servoTipX;    // Tip section X-axis control
Servo servoTipY;    // Tip section Y-axis control

const int servoBaseXPin = 3;   // PWM pin for base X servo
const int servoBaseYPin = 5;   // PWM pin for base Y servo
const int servoTipXPin = 6;    // PWM pin for tip X servo
const int servoTipYPin = 9;    // PWM pin for tip Y servo

// ===== MASTER TIMING CONTROLS =====
// Central speed multiplier for easy global timing adjustments
const float MASTER_SPEED = 1.0;  // Global speed multiplier

// REST STATE TIMING - Energy regeneration phase
const float REST_ENERGY_RATE = 0.5 * MASTER_SPEED;    // Energy gain per update cycle
const float REST_PHASE_SPEED = 0.1 * MASTER_SPEED;    // Speed of shiver movement
const float REST_SMOOTHING = 0.1;                     // Movement smoothing factor

// SEEK STATE TIMING - Movement toward target perimeter region
const float SEEK_ENERGY_RATE = 0.05 * MASTER_SPEED;   // Energy cost per update
const float SEEK_PROGRESS_RATE = 0.005 * MASTER_SPEED; // Speed of progress to target
const float SEEK_PHASE_SPEED = 0.08 * MASTER_SPEED;   // Speed of winding path variation
const float SEEK_SMOOTHING_BASE = 0.05;               // Base movement smoothing

// SEARCH STATE TIMING - Territorial exploration behavior
const float SEARCH_ENERGY_RATE = 0.025 * MASTER_SPEED;  // Energy drain during exploration
const float SEARCH_BASE_SPEED = 0.003 * MASTER_SPEED;   // Base exploration progression speed
const float SEARCH_DWELL_FREQ = 0.073 * MASTER_SPEED;   // Frequency of dwelling behavior
const float SEARCH_MICRO_FREQ = 0.18 * MASTER_SPEED;    // Frequency of micro-movements
const float SEARCH_SMOOTHING_BASE = 0.08;               // Movement smoothing factor

// RETURN STATE TIMING - Gravity-driven return to center
const float RETURN_ENERGY_RATE = 0.1 * MASTER_SPEED;    // Energy drain during struggle
const float RETURN_GRAVITY_BASE = 0.015 * MASTER_SPEED; // Base gravitational strength
const float RETURN_SMOOTHING_BASE = 0.08;               // Movement smoothing factor
const float RETURN_RESISTANCE_FREQ = 0.003 * MASTER_SPEED; // Resistance movement frequency

// COMPOUND CURVATURE TIMING - Tip articulation parameters
const float TIP_CONCORDANT_FREQ = 0.9 * MASTER_SPEED;   // Frequency for concordant movement
const float TIP_OPPOSITION_FREQ = 1.3 * MASTER_SPEED;   // Frequency for oppositional movement
const float TIP_ARTICULATION_RANGE = 15.0;              // Maximum tip deviation from base

// ===== SIMPLIFIED SPATIAL MEMORY SYSTEM =====
// Reduced grid size for Arduino Uno RAM constraints
const int GRID_SIZE = 6;                          // 6x6 grid (36 cells total)
const float CELL_SIZE = 180.0 / GRID_SIZE;       // Each cell is 30° x 30°
uint8_t visitGrid[GRID_SIZE][GRID_SIZE];         // Visit count for each grid cell
int currentGridX = -1;                           // Current X grid position
int currentGridY = -1;                           // Current Y grid position

// ===== COMPOUND POSITION TRACKING =====
float baseX = 90.0;    // Base section X angle (0-180°)
float baseY = 90.0;    // Base section Y angle (0-180°)
float tipX = 90.0;     // Tip section X angle (0-180°)
float tipY = 90.0;     // Tip section Y angle (0-180°)

// Calculated territorial position (follows base for spatial memory)
float territorialX = 90.0;   // Position for grid memory tracking
float territorialY = 90.0;   // Position for grid memory tracking

// ===== STATE MACHINE SYSTEM =====
enum State {
  REST,    // Energy regeneration at center
  SEEK,    // Movement toward target region
  SEARCH,  // Territorial exploration with compound curvature
  RETURN   // Gravitational return to center
};
State currentState = REST;  // Start in rest state

// ===== ENERGY SYSTEM =====
float energy = 0.0;                    // Current energy level (0-100)
const float maxEnergy = 100.0;         // Maximum energy capacity
const float restPosition = 90.0;       // Center position (gravitational attractor)

// ===== SEEK STATE VARIABLES =====
float seekTargetBaseX = 90.0, seekTargetBaseY = 90.0;  // Base target coordinates
float seekTargetTipX = 90.0, seekTargetTipY = 90.0;    // Tip target coordinates
float seekPhase = 0.0;                 // Phase for winding path generation
float seekProgress = 0.0;              // Progress toward target (0-1)

// ===== SEARCH STATE VARIABLES =====
float searchCenterX = 90.0, searchCenterY = 90.0;      // Center of current search region
float searchPhase = 0.0;               // Primary exploration progression phase
float memoryAccumulator = 0.0;         // Hysteresis memory level (affects behavior)
float dwellPhase = 0.0;                // Phase for dwelling circle patterns
float microPhase = 0.0;                // Phase for micro-movement texture

// ===== COMPOUND CURVATURE VARIABLES =====
enum CurvatureMode {
  CONCORDANT,    // Base and tip move in harmony (serpentine)
  OPPOSITIONAL,  // Base and tip create tension (coiling)
  BASE_DOMINANT, // Base drives, tip follows with minimal variation
  TIP_EXPRESSIVE // Base stable, tip creates detailed articulation
};
CurvatureMode curvatureMode = CONCORDANT;

// ===== SERVO PHYSICAL LIMITS =====
const float servoMin = 5.0;    // Minimum safe servo angle
const float servoMax = 175.0;  // Maximum safe servo angle

// ===== TIMING CONTROL =====
unsigned long lastUpdate = 0;           // Timestamp of last update
const unsigned long updateInterval = 20; // Update every 20ms (50Hz)

// ===== FUNCTION DECLARATIONS =====
void printDebug();                                    // Debug output to serial
void updateGridMemory();                             // Update spatial memory system
float calculateDesirability(int gx, int gy);         // Calculate target desirability score
void findLeastVisitedPerimeterTarget();              // Select next exploration target
void setCompoundCurvature(float targetBaseX, float targetBaseY, float targetTipX, float targetTipY);
void updateCurvatureMode();                          // Select appropriate curvature behavior
void calculateTipArticulation(float& tipTargetX, float& tipTargetY, float baseTargetX, float baseTargetY);
int getGridX(float angle);                           // Convert servo angle to grid X
int getGridY(float angle);                           // Convert servo angle to grid Y
bool isPerimeterCell(int gx, int gy);               // Check if grid cell is on perimeter

void setup() {
  // Initialize servo motors
  servoBaseX.attach(servoBaseXPin);
  servoBaseY.attach(servoBaseYPin);
  servoTipX.attach(servoTipXPin);
  servoTipY.attach(servoTipYPin);
  
  // Initialize spatial memory grid (all cells start with 0 visits)
  for(int i = 0; i < GRID_SIZE; i++) {
    for(int j = 0; j < GRID_SIZE; j++) {
      visitGrid[i][j] = 0;
    }
  }
  
  // Initialize serial communication for debugging
  Serial.begin(9600);
  Serial.println("H1XYXY V1 - Arduino Uno Dual XY Implementation");
  Serial.println("V31 territorial exploration with compound curvature");
}

void loop() {
  unsigned long now = millis();
  
  // Update at fixed intervals for consistent behavior
  if (now - lastUpdate > updateInterval) {
    lastUpdate = now;
    
    // Update spatial memory system
    updateGridMemory();
    
    // Update curvature mode based on current state and energy
    updateCurvatureMode();
    
    // Execute current state behavior
    switch(currentState) {
      case REST:
        updateRest();           // Energy regeneration with compound shiver
        break;
      case SEEK:
        updateSeek();           // Movement toward target with curvature
        break;
      case SEARCH:
        updateSearch();         // Territorial exploration with articulation
        break;
      case RETURN:
        updateReturn();         // Gravitational return with resistance
        break;
    }
    
    // Output debug information
    printDebug();
  }
}

// ===== SPATIAL MEMORY SYSTEM =====
void updateGridMemory() {
  // Convert current base position to grid coordinates (base drives territorial navigation)
  int gx = getGridX(baseX);
  int gy = getGridY(baseY);
  
  // Update visit count when entering a new grid cell
  if(gx != currentGridX || gy != currentGridY) {
    if(gx >= 0 && gx < GRID_SIZE && gy >= 0 && gy < GRID_SIZE) {
      visitGrid[gx][gy] = min(255, visitGrid[gx][gy] + 1);  // Increment visit count
      currentGridX = gx;            // Update current position tracking
      currentGridY = gy;
    }
  }
  
  // Update territorial position for spatial logic
  territorialX = baseX;
  territorialY = baseY;
}

// ===== SPATIAL WEIGHTING ALGORITHM =====
float calculateDesirability(int gx, int gy) {
  // Base score: lower visit count = higher desirability
  float baseScore = 1.0 / (visitGrid[gx][gy] + 1);
  
  // Proximity penalty: nearby heavily-visited cells reduce desirability
  float proximityPenalty = 0.0;
  
  // Check 3x3 neighborhood around target cell
  for(int dx = -1; dx <= 1; dx++) {
    for(int dy = -1; dy <= 1; dy++) {
      int nx = gx + dx, ny = gy + dy;
      
      // Only consider cells within grid bounds
      if(nx >= 0 && nx < GRID_SIZE && ny >= 0 && ny < GRID_SIZE) {
        float distance = sqrt(dx*dx + dy*dy);
        if(distance > 0) {
          // Nearby cells with high visits create penalty
          proximityPenalty += visitGrid[nx][ny] / (distance * distance);
        }
      }
    }
  }
  
  // Return final desirability score (higher = better target)
  return baseScore / (1.0 + proximityPenalty * 0.1);
}

// ===== TARGET SELECTION SYSTEM =====
void findLeastVisitedPerimeterTarget() {
  int bestGX = 0, bestGY = 0;     // Grid coordinates of best target
  float bestScore = 0.0;          // Highest desirability score found
  
  // Scan all perimeter cells to find highest desirability score
  for(int gx = 0; gx < GRID_SIZE; gx++) {
    for(int gy = 0; gy < GRID_SIZE; gy++) {
      if(isPerimeterCell(gx, gy)) {
        float score = calculateDesirability(gx, gy);
        
        // Track best target found so far
        if(score > bestScore) {
          bestScore = score;
          bestGX = gx;
          bestGY = gy;
        }
      }
    }
  }
  
  // Convert grid coordinates to servo angles (cell center)
  seekTargetBaseX = (bestGX + 0.5) * CELL_SIZE;
  seekTargetBaseY = (bestGY + 0.5) * CELL_SIZE;
  
  // Calculate complementary tip targets based on curvature mode
  calculateTipArticulation(seekTargetTipX, seekTargetTipY, seekTargetBaseX, seekTargetBaseY);
  
  // Constrain to servo limits
  seekTargetBaseX = constrain(seekTargetBaseX, servoMin, servoMax);
  seekTargetBaseY = constrain(seekTargetBaseY, servoMin, servoMax);
  seekTargetTipX = constrain(seekTargetTipX, servoMin, servoMax);
  seekTargetTipY = constrain(seekTargetTipY, servoMin, servoMax);
}

// ===== CURVATURE MODE SELECTION =====
void updateCurvatureMode() {
  // Select curvature behavior based on state and energy
  switch(currentState) {
    case REST:
      curvatureMode = CONCORDANT;      // Gentle compound shiver
      break;
    case SEEK:
      curvatureMode = BASE_DOMINANT;   // Base drives navigation, tip follows
      break;
    case SEARCH:
      // Energy level determines search articulation style
      if(energy > 70.0) {
        curvatureMode = TIP_EXPRESSIVE; // High energy = detailed tip exploration
      } else if(energy > 40.0) {
        curvatureMode = CONCORDANT;     // Medium energy = serpentine patterns
      } else {
        curvatureMode = BASE_DOMINANT;  // Low energy = minimal tip movement
      }
      break;
    case RETURN:
      curvatureMode = OPPOSITIONAL;    // Tip resists while base pulled to center
      break;
  }
}

// ===== TIP ARTICULATION CALCULATION =====
void calculateTipArticulation(float& tipTargetX, float& tipTargetY, float baseTargetX, float baseTargetY) {
  float tipOffsetX = 0.0, tipOffsetY = 0.0;
  
  switch(curvatureMode) {
    case CONCORDANT:
      // Tip echoes base movement with slight phase delay and scaling
      tipOffsetX = sin(millis() * TIP_CONCORDANT_FREQ * 0.001) * TIP_ARTICULATION_RANGE * 0.3;
      tipOffsetY = cos(millis() * TIP_CONCORDANT_FREQ * 0.001 * 1.1) * TIP_ARTICULATION_RANGE * 0.3;
      break;
      
    case OPPOSITIONAL:
      // Tip moves in opposition to base direction
      tipOffsetX = -sin(millis() * TIP_OPPOSITION_FREQ * 0.001) * TIP_ARTICULATION_RANGE * 0.6;
      tipOffsetY = -cos(millis() * TIP_OPPOSITION_FREQ * 0.001 * 0.8) * TIP_ARTICULATION_RANGE * 0.6;
      break;
      
    case BASE_DOMINANT:
      // Minimal tip variation - mostly follows base
      tipOffsetX = sin(millis() * 0.002) * TIP_ARTICULATION_RANGE * 0.1;
      tipOffsetY = cos(millis() * 0.002 * 1.2) * TIP_ARTICULATION_RANGE * 0.1;
      break;
      
    case TIP_EXPRESSIVE:
      // Dynamic tip articulation independent of base
      tipOffsetX = sin(millis() * 0.005) * TIP_ARTICULATION_RANGE * 0.8;
      tipOffsetY = cos(millis() * 0.005 * 1.4) * TIP_ARTICULATION_RANGE * 0.8;
      break;
  }
  
  tipTargetX = baseTargetX + tipOffsetX;
  tipTargetY = baseTargetY + tipOffsetY;
}

// ===== COMPOUND CURVATURE APPLICATION =====
void setCompoundCurvature(float targetBaseX, float targetBaseY, float targetTipX, float targetTipY) {
  float smoothing = SEARCH_SMOOTHING_BASE;
  
  // Smooth interpolation to targets
  baseX += (targetBaseX - baseX) * smoothing;
  baseY += (targetBaseY - baseY) * smoothing;
  tipX += (targetTipX - tipX) * smoothing;
  tipY += (targetTipY - tipY) * smoothing;
  
  // Constrain all servos to safe limits
  baseX = constrain(baseX, servoMin, servoMax);
  baseY = constrain(baseY, servoMin, servoMax);
  tipX = constrain(tipX, servoMin, servoMax);
  tipY = constrain(tipY, servoMin, servoMax);
  
  // Update physical servos
  servoBaseX.write((int)baseX);
  servoBaseY.write((int)baseY);
  servoTipX.write((int)tipX);
  servoTipY.write((int)tipY);
}

// ===== STATE IMPLEMENTATIONS =====
void updateRest() {
  // Regenerate energy slowly
  energy += REST_ENERGY_RATE;
  
  // Compound shiver movement - both base and tip contribute
  float restPhase = millis() * REST_PHASE_SPEED * 0.001;
  float shiverBaseX = restPosition + cos(restPhase) * 3.0;
  float shiverBaseY = restPosition + sin(restPhase * 1.3) * 2.5;
  
  // Calculate tip articulation for current targets
  float shiverTipX, shiverTipY;
  calculateTipArticulation(shiverTipX, shiverTipY, shiverBaseX, shiverBaseY);
  
  setCompoundCurvature(shiverBaseX, shiverBaseY, shiverTipX, shiverTipY);
  
  // Transition to SEEK when fully energized
  if(energy >= maxEnergy) {
    findLeastVisitedPerimeterTarget();  // Select next exploration target
    seekProgress = 0.0;
    seekPhase = 0.0;
    currentState = SEEK;
    Serial.println("REST -> SEEK (compound exploration)");
  }
}

void updateSeek() {
  // Update progress and winding path phase
  seekProgress += SEEK_PROGRESS_RATE;
  seekPhase += SEEK_PHASE_SPEED;
  energy -= SEEK_ENERGY_RATE;
  
  // Winding path to target (diminishes as we approach)
  float windX = sin(seekPhase * 2.1) * 15.0 * (1.0 - seekProgress);
  float windY = cos(seekPhase * 1.7) * 12.0 * (1.0 - seekProgress);
  
  // Calculate base target with winding
  float targetBaseX = restPosition + (seekTargetBaseX - restPosition) * seekProgress + windX;
  float targetBaseY = restPosition + (seekTargetBaseY - restPosition) * seekProgress + windY;
  
  // Calculate tip articulation for current base target
  float targetTipX, targetTipY;
  calculateTipArticulation(targetTipX, targetTipY, targetBaseX, targetBaseY);
  
  setCompoundCurvature(targetBaseX, targetBaseY, targetTipX, targetTipY);
  
  // Transition to SEARCH when target reached
  if(seekProgress >= 1.0) {
    currentState = SEARCH;
    searchCenterX = baseX;    // Search around current base position
    searchCenterY = baseY;
    searchPhase = 0.0;
    memoryAccumulator = 0.0;
    dwellPhase = 0.0;
    microPhase = 0.0;
    Serial.println("SEEK -> SEARCH (compound territorial exploration)");
  }
}

void updateSearch() {
  // Update exploration phases
  dwellPhase += SEARCH_DWELL_FREQ;
  microPhase += SEARCH_MICRO_FREQ;
  energy -= SEARCH_ENERGY_RATE;
  
  // Hysteresis memory system
  float dwellIntensity = (sin(dwellPhase) + 1.0) * 0.5;
  if (dwellIntensity > 0.6) {
    memoryAccumulator += 0.15 * dwellIntensity;
    if (memoryAccumulator > 2.0) memoryAccumulator = 2.0;
  } else {
    memoryAccumulator -= 0.03;
    if (memoryAccumulator < 0) memoryAccumulator = 0;
  }
  
  // Memory affects exploration speed
  float effectiveSpeed = SEARCH_BASE_SPEED * (1.0 - (memoryAccumulator / 2.0) * 0.7);
  searchPhase += effectiveSpeed;
  
  // Base exploration pattern
  float explorationRange = 12.0 + sin(searchPhase * 0.7) * 8.0 + memoryAccumulator * 2.0;
  float baseOffsetX = cos(searchPhase * 1.3) * explorationRange;
  float baseOffsetY = sin(searchPhase * 0.9) * explorationRange * 0.8;  // Elliptical
  
  // Micro-movements for organic texture
  float microX = cos(microPhase) * 2.5;
  float microY = sin(microPhase * 1.3) * 2.0;
  
  float targetBaseX = searchCenterX + baseOffsetX + microX;
  float targetBaseY = searchCenterY + baseOffsetY + microY;
  
  // Calculate tip articulation based on current curvature mode
  float targetTipX, targetTipY;
  calculateTipArticulation(targetTipX, targetTipY, targetBaseX, targetBaseY);
  
  setCompoundCurvature(targetBaseX, targetBaseY, targetTipX, targetTipY);
  
  // Transition to RETURN when energy depleted
  if(energy <= 20.0) {
    currentState = RETURN;
    Serial.println("SEARCH -> RETURN (compound gravitational struggle)");
  }
}

void updateReturn() {
  energy -= RETURN_ENERGY_RATE;
  
  // Gravitational pull toward center
  float gravityX = (90.0 - baseX) * RETURN_GRAVITY_BASE;
  float gravityY = (90.0 - baseY) * RETURN_GRAVITY_BASE;
  
  // Energy depletion increases gravity strength
  float energyFactor = (20.0 - energy) / 20.0;  // 0 to 1 as energy drops
  gravityX *= (1.0 + energyFactor * 2.0);
  gravityY *= (1.0 + energyFactor * 2.0);
  
  // Base movement - pulled toward center
  float targetBaseX = baseX + gravityX;
  float targetBaseY = baseY + gravityY;
  
  // Tip resistance - oppositional movement
  float resistancePhase = millis() * RETURN_RESISTANCE_FREQ;
  float tipResistanceX = cos(resistancePhase) * TIP_ARTICULATION_RANGE * (energy / 20.0);
  float tipResistanceY = sin(resistancePhase * 1.2) * TIP_ARTICULATION_RANGE * (energy / 20.0);
  
  float targetTipX = tipX + tipResistanceX;
  float targetTipY = tipY + tipResistanceY;
  
  setCompoundCurvature(targetBaseX, targetBaseY, targetTipX, targetTipY);
  
  // Check for return to center
  float distFromCenter = sqrt(pow(baseX - 90, 2) + pow(baseY - 90, 2));
  if(distFromCenter < 5.0 || energy <= 0.0) {
    currentState = REST;
    energy = 0.0;
    Serial.println("RETURN -> REST (consumed by darkness)");
  }
}

// ===== PERIMETER DETECTION =====
bool isPerimeterCell(int gx, int gy) {
  return (gx == 0 || gx == GRID_SIZE-1 || gy == 0 || gy == GRID_SIZE-1);
}

// ===== COORDINATE CONVERSION UTILITIES =====
int getGridX(float angle) {
  return constrain((int)(angle / CELL_SIZE), 0, GRID_SIZE-1);
}

int getGridY(float angle) {
  return constrain((int)(angle / CELL_SIZE), 0, GRID_SIZE-1);
}

// ===== DEBUG OUTPUT SYSTEM =====
void printDebug() {
  static unsigned long lastPrint = 0;
  
  // Print debug info every second
  if (millis() - lastPrint > 1000) {
    lastPrint = millis();
    
    // Basic status
    Serial.print("State: ");
    switch(currentState) {
      case REST: Serial.print("REST"); break;
      case SEEK: Serial.print("SEEK"); break;
      case SEARCH: Serial.print("SEARCH"); break;
      case RETURN: Serial.print("RETURN"); break;
    }
    
    // Curvature mode
    Serial.print(" | Mode: ");
    switch(curvatureMode) {
      case CONCORDANT: Serial.print("CONCORDANT"); break;
      case OPPOSITIONAL: Serial.print("OPPOSITIONAL"); break;
      case BASE_DOMINANT: Serial.print("BASE_DOM"); break;
      case TIP_EXPRESSIVE: Serial.print("TIP_EXPR"); break;
    }
    
    // Energy and positions
    Serial.print(" | Energy: "); Serial.print(energy, 1);
    Serial.print(" | Base:("); Serial.print((int)baseX); Serial.print(","); Serial.print((int)baseY); Serial.print(")");
    Serial.print(" | Tip:("); Serial.print((int)tipX); Serial.print(","); Serial.print((int)tipY); Serial.print(")");
    
    // Search-specific info
    if(currentState == SEARCH) {
      Serial.print(" | Memory: "); Serial.print(memoryAccumulator, 2);
    }
    
    // Grid position
    Serial.print(" | Grid:("); Serial.print(currentGridX); Serial.print(","); Serial.print(currentGridY); Serial.print(")");
    
    Serial.println();
  }
}