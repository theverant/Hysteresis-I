/*
  Searching V30
  Organic perimeter search with layered movement patterns
  Created by Theverant with essential help from Claude Sonnet 4
  Date: 2025-08-04
  License: GPLv3

  Changelog:
  - V30: Updated with Gravity-Influenced Territorial Exploration
  - v29: Combined V21 organic movement with V28 perimeter positioning and grid memory
  - v28: Grid memory for systematic exploration, balanced search speeds, no randomness
  - v27: SEARCH now stays at perimeter with irregular, elongated circles
  - v26: Four-state system (Rest/Seek/Search/Return) with clear transitions
  - v25: Energy/gravity model - rest position has gravitational pull
  - v24: Two-tier search with major/minor circles, dual memory systems
  - v23: Implemented hysteresis-based dwelling for lonely search behavior
  - v22: Added epicycloid pattern generation for circles within circles
  - v21: Multi-frequency sine wave exploration with organic movement
*/

#include <Servo.h>

Servo servoX;
Servo servoY;

const int servoXPin = 3;
const int servoYPin = 6;

// MASTER TIMING CONTROLS
const float MASTER_SPEED = 1.0;  // Global speed multiplier

// REST STATE TIMING
const float REST_ENERGY_RATE = 0.5 * MASTER_SPEED;
const float REST_PHASE_SPEED = 0.1 * MASTER_SPEED;
const float REST_SMOOTHING = 0.1;

// SEEK STATE TIMING  
const float SEEK_ENERGY_RATE = 0.05 * MASTER_SPEED;
const float SEEK_PROGRESS_RATE = 0.005 * MASTER_SPEED;
const float SEEK_PHASE_SPEED = 0.08 * MASTER_SPEED;
const float SEEK_SMOOTHING_BASE = 0.05;

// SEARCH STATE TIMING (ORGANIC PATTERNS)
const float SEARCH_ENERGY_RATE = 0.025 * MASTER_SPEED;
const float SEARCH_BASE_SPEED = 0.003 * MASTER_SPEED;     // Base search progression
const float SEARCH_DWELL_FREQ = 0.073 * MASTER_SPEED;    // Dwelling pattern frequency  
const float SEARCH_MICRO_FREQ = 0.18 * MASTER_SPEED;     // Micro movement frequency
const float SEARCH_SMOOTHING_BASE = 0.08;

// RETURN STATE TIMING
const float RETURN_ENERGY_RATE = 0.1 * MASTER_SPEED;
const float RETURN_GRAVITY_BASE = 0.015 * MASTER_SPEED;
const float RETURN_SMOOTHING_BASE = 0.08;

// ORGANIC SEARCH PARAMETERS (from V21)
const float searchRadius = 18.0;        // Main search pattern radius (larger for better coverage)
const float dwellRadius = 18.0;         // Dwelling circle radius
const float microRadius = 3.0;          // Micro-movement radius
const float memoryBuildRate = 0.15;     // How fast memory accumulates
const float memoryDecayRate = 0.03;     // How fast memory fades
const float maxMemory = 2.0;            // Maximum memory accumulation

// GRID MEMORY SYSTEM
const int GRID_SIZE = 12;                    // 12x12 grid
const float CELL_SIZE = 180.0 / GRID_SIZE;  // 15° per cell
uint8_t visitGrid[GRID_SIZE][GRID_SIZE];    // Visit count per cell
int currentGridX = -1;                      // Current grid position
int currentGridY = -1;

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

// Search state variables (organic patterns)
float searchCenterX = 90.0;       // Where we're searching around (perimeter location)
float searchCenterY = 90.0;       // Local center of search
float searchPhase = 0.0;          // Primary search progression
float memoryAccumulator = 0.0;    // Hysteresis memory buildup
float dwellPhase = 0.0;           // Secondary pattern for dwelling
float microPhase = 0.0;           // Micro movement phase

// Rest state
float restPhase = 0.0;            // For shiver movement

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
void updateGridMemory();
float calculateDesirability(int gx, int gy);
void findLeastVisitedPerimeterTarget();
int getGridX(float angle);
int getGridY(float angle);
bool isPerimeterCell(int gx, int gy);

void setup() {
  servoX.attach(servoXPin);
  servoY.attach(servoYPin);
  
  // Initialize grid memory
  for(int i = 0; i < GRID_SIZE; i++) {
    for(int j = 0; j < GRID_SIZE; j++) {
      visitGrid[i][j] = 0;
    }
  }
  
  Serial.begin(9600);
  Serial.println("Searching V29 - Organic Perimeter Search");
  Serial.println("REST -> SEEK -> SEARCH(organic at perimeter) -> RETURN");
}

void loop() {
  unsigned long now = millis();
  
  if (now - lastUpdate > updateInterval) {
    lastUpdate = now;
    
    updateGridMemory();  // Track current position
    
    switch(currentState) {
      case REST:
        updateRest();
        break;
      case SEEK:
        updateSeek();
        break;
      case SEARCH:
        updateOrganicSearch();
        break;
      case RETURN:
        updateReturn();
        break;
    }
    
    printDebug();
  }
}

void updateGridMemory() {
  int gx = getGridX(xAngle);
  int gy = getGridY(yAngle);
  
  // Update visit count if in new cell
  if(gx != currentGridX || gy != currentGridY) {
    if(gx >= 0 && gx < GRID_SIZE && gy >= 0 && gy < GRID_SIZE) {
      visitGrid[gx][gy]++;
      currentGridX = gx;
      currentGridY = gy;
    }
  }
}

float calculateDesirability(int gx, int gy) {
  // Base score - lower visits = higher desirability
  float baseScore = 1.0 / (visitGrid[gx][gy] + 1);
  
  // Proximity penalty - nearby heavily-visited cells reduce desirability
  float proximityPenalty = 0.0;
  for(int dx = -2; dx <= 2; dx++) {
    for(int dy = -2; dy <= 2; dy++) {
      int nx = gx + dx, ny = gy + dy;
      if(nx >= 0 && nx < GRID_SIZE && ny >= 0 && ny < GRID_SIZE) {
        float distance = sqrt(dx*dx + dy*dy);
        if(distance > 0) {
          // Nearby cells with high visits create penalty
          proximityPenalty += visitGrid[nx][ny] / (distance * distance);
        }
      }
    }
  }
  
  // Return final desirability score (higher = better)
  return baseScore / (1.0 + proximityPenalty * 0.05);
}

void findLeastVisitedPerimeterTarget() {
  int bestGX = 0, bestGY = 0;
  float bestScore = 0.0;
  
  // Debug: show all perimeter cells with scores
  Serial.print("Perimeter analysis: ");
  
  // Scan perimeter cells for highest desirability
  for(int gx = 0; gx < GRID_SIZE; gx++) {
    for(int gy = 0; gy < GRID_SIZE; gy++) {
      if(isPerimeterCell(gx, gy)) {
        float score = calculateDesirability(gx, gy);
        
        // Debug output
        Serial.print("("); Serial.print(gx); Serial.print(","); Serial.print(gy); 
        Serial.print(":"); Serial.print(visitGrid[gx][gy]); 
        Serial.print("/"); Serial.print(score, 2); Serial.print(") ");
        
        if(score > bestScore) {
          bestScore = score;
          bestGX = gx;
          bestGY = gy;
        }
      }
    }
  }
  Serial.println();
  
  // Convert grid coordinates to servo angles
  seekTargetX = (bestGX + 0.5) * CELL_SIZE;
  seekTargetY = (bestGY + 0.5) * CELL_SIZE;
  
  // Smart boundary checking - only constrain if search pattern would go out of bounds
  float worstCaseXMin = seekTargetX - searchRadius;
  float worstCaseXMax = seekTargetX + searchRadius;
  float worstCaseYMin = seekTargetY - searchRadius;
  float worstCaseYMax = seekTargetY + searchRadius;
  
  bool constrained = false;
  // Only apply constraints when actually needed
  if(worstCaseXMin < xMin + 5) {
    seekTargetX = xMin + searchRadius + 5;
    constrained = true;
  }
  if(worstCaseXMax > xMax - 5) {
    seekTargetX = xMax - searchRadius - 5;
    constrained = true;
  }
  if(worstCaseYMin < yMin + 5) {
    seekTargetY = yMin + searchRadius + 5;
    constrained = true;
  }
  if(worstCaseYMax > yMax - 5) {
    seekTargetY = yMax - searchRadius - 5;
    constrained = true;
  }
  
  // Debug: show target selection
  Serial.print("Target selection: bestGX="); Serial.print(bestGX);
  Serial.print(" bestGY="); Serial.print(bestGY); 
  Serial.print(" bestScore="); Serial.print(bestScore, 3);
  Serial.print(" -> ("); Serial.print((int)seekTargetX);
  Serial.print(","); Serial.print((int)seekTargetY); Serial.println(")");
}

bool isPerimeterCell(int gx, int gy) {
  // Define perimeter as outer 2 rows/columns
  return (gx <= 1 || gx >= GRID_SIZE-2 || gy <= 1 || gy >= GRID_SIZE-2);
}

int getGridX(float angle) {
  return constrain((int)(angle / CELL_SIZE), 0, GRID_SIZE-1);
}

int getGridY(float angle) {
  return constrain((int)(angle / CELL_SIZE), 0, GRID_SIZE-1);
}

void updateRest() {
  // Small shiver movement
  restPhase += REST_PHASE_SPEED;
  float shiverX = restPosition + cos(restPhase) * 3.0;
  float shiverY = restPosition + sin(restPhase * 1.3) * 2.5;
  
  // Regenerate energy
  energy += REST_ENERGY_RATE;
  
  // Transition to SEEK when fully energized
  if (energy >= 100.0) {
    findLeastVisitedPerimeterTarget();  // Memory-based targeting
    
    seekProgress = 0.0;
    seekPhase = 0.0;
    currentState = SEEK;
    Serial.print("REST -> SEEK targeting (");
    Serial.print((int)seekTargetX); Serial.print(",");
    Serial.print((int)seekTargetY); Serial.println(")");
  }
  
  // Apply movement
  xAngle += (shiverX - xAngle) * REST_SMOOTHING;
  yAngle += (shiverY - yAngle) * REST_SMOOTHING;
  
  xAngle = constrain(xAngle, 0, 180);
  yAngle = constrain(yAngle, 0, 180);
  
  servoX.write((int)xAngle);
  servoY.write((int)yAngle);
}

void updateSeek() {
  // Progress toward target with winding path
  seekProgress += SEEK_PROGRESS_RATE;
  seekPhase += SEEK_PHASE_SPEED;
  
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
  energy -= SEEK_ENERGY_RATE;
  
  // Transition to SEARCH when arrived
  if (seekProgress >= 1.0) {
    currentState = SEARCH;
    searchCenterX = xAngle;  // Search around where we arrived (perimeter location)
    searchCenterY = yAngle;
    
    // Reset organic search parameters
    searchPhase = 0.0;
    memoryAccumulator = 0.0;
    dwellPhase = 0.0;
    microPhase = 0.0;
    
    Serial.println("SEEK -> SEARCH (organic patterns)");
  }
  
  // Apply movement with acceleration
  float smoothing = SEEK_SMOOTHING_BASE * speedBoost;
  xAngle += (targetX - xAngle) * smoothing;
  yAngle += (targetY - yAngle) * smoothing;
  
  xAngle = constrain(xAngle, 0, 180);
  yAngle = constrain(yAngle, 0, 180);
  
  servoX.write((int)xAngle);
  servoY.write((int)yAngle);
}

void updateOrganicSearch() {
  // Update organic phases
  dwellPhase += SEARCH_DWELL_FREQ;
  microPhase += SEARCH_MICRO_FREQ;
  
  // Calculate hysteresis effect (dwelling intensity)
  float dwellIntensity = (sin(dwellPhase) + 1.0) * 0.5;  // 0 to 1
  
  // Build memory when dwelling is strong, decay when weak
  if (dwellIntensity > 0.6) {
    memoryAccumulator += memoryBuildRate * dwellIntensity;
    if (memoryAccumulator > maxMemory) memoryAccumulator = maxMemory;
  } else {
    memoryAccumulator -= memoryDecayRate;
    if (memoryAccumulator < 0) memoryAccumulator = 0;
  }
  
  // Hysteresis affects search speed - high memory = slower movement
  float effectiveSpeed = SEARCH_BASE_SPEED * (1.0 - (memoryAccumulator / maxMemory) * 0.7);
  searchPhase += effectiveSpeed;
  
  // Main search pattern (elliptical, positioned at perimeter)
  float searchX = cos(searchPhase) * searchRadius;
  float searchY = sin(searchPhase) * searchRadius * 0.6;  // Elliptical for variety
  
  // Dwelling circles - intensity based on memory
  float dwellX = cos(dwellPhase * 3.7) * dwellRadius * memoryAccumulator / maxMemory;
  float dwellY = sin(dwellPhase * 4.1) * dwellRadius * memoryAccumulator / maxMemory;
  
  // Micro movements for organic feel
  float microX = cos(microPhase) * microRadius;
  float microY = sin(microPhase * 1.3) * microRadius;
  
  // Combine all movements relative to perimeter search center
  float xTarget = searchCenterX + searchX + dwellX + microX;
  float yTarget = searchCenterY + searchY + dwellY + microY;
  
  // Enhanced boundary checking - prevent clipping at servo limits
  xTarget = constrain(xTarget, xMin + 5, xMax - 5);
  yTarget = constrain(yTarget, yMin + 5, yMax - 5);
  
  // Energy drain while searching
  energy -= SEARCH_ENERGY_RATE;
  
  // Transition to RETURN when exhausted
  if (energy <= 20.0) {
    currentState = RETURN;
    Serial.println("SEARCH -> RETURN");
  }
  
  // Apply smooth movement
  xAngle += (xTarget - xAngle) * SEARCH_SMOOTHING_BASE;
  yAngle += (yTarget - yAngle) * SEARCH_SMOOTHING_BASE;
  
  xAngle = constrain(xAngle, 0, 180);
  yAngle = constrain(yAngle, 0, 180);
  
  servoX.write((int)xAngle);
  servoY.write((int)yAngle);
  
  // Keep phases bounded
  if (searchPhase > 6.28318) searchPhase -= 6.28318;
  if (dwellPhase > 6.28318) dwellPhase -= 6.28318;
  if (microPhase > 6.28318) microPhase -= 6.28318;
}

void updateReturn() {
  // Being pulled back to center while resisting
  // Gravity increases as energy depletes
  float gravityPull = (1.0 - energy / 100.0) * RETURN_GRAVITY_BASE;
  
  // Continue micro movements but weakening
  microPhase += SEARCH_MICRO_FREQ * 0.5;  // Half speed
  float resistX = cos(microPhase) * microRadius * (energy / 100.0);
  float resistY = sin(microPhase * 1.3) * microRadius * (energy / 100.0);
  
  // Current position with resistance
  float xTarget = xAngle + resistX;
  float yTarget = yAngle + resistY;
  
  // Apply gravity pull toward center
  xTarget = xTarget * (1.0 - gravityPull) + restPosition * gravityPull;
  yTarget = yTarget * (1.0 - gravityPull) + restPosition * gravityPull;
  
  // Continue energy drain
  energy -= RETURN_ENERGY_RATE;
  if (energy < 0) energy = 0;  // Prevent negative energy
  
  // Check if pulled back to rest
  float distFromRest = sqrt(pow(xAngle - restPosition, 2) + pow(yAngle - restPosition, 2));
  if (distFromRest < 15.0 || energy <= 0.0) {
    currentState = REST;
    energy = 0.0;
    Serial.println("RETURN -> REST");
  }
  
  // Apply movement - slower as tired
  float smoothing = RETURN_SMOOTHING_BASE * (energy / 100.0 + 0.3);
  xAngle += (xTarget - xAngle) * smoothing;
  yAngle += (yTarget - yAngle) * smoothing;
  
  // Force bounds check
  xAngle = constrain(xAngle, 0, 180);
  yAngle = constrain(yAngle, 0, 180);
  
  servoX.write((int)xAngle);
  servoY.write((int)yAngle);
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
    Serial.print(","); Serial.print((int)yAngle);
    
    if (currentState == SEARCH) {
      Serial.print(" | Memory: "); Serial.print(memoryAccumulator, 2);
      Serial.print(" | Center: ("); Serial.print((int)searchCenterX);
      Serial.print(","); Serial.print((int)searchCenterY); Serial.print(")");
      Serial.print(" | Radius: "); Serial.print((int)searchRadius);
    }
    
    Serial.print(" | Grid: "); Serial.print(currentGridX);
    Serial.print(","); Serial.print(currentGridY);
    Serial.print(" | Visits: ");
    if(currentGridX >= 0 && currentGridY >= 0) {
      Serial.println(visitGrid[currentGridX][currentGridY]);
    } else {
      Serial.println("--");
    }
  }
}