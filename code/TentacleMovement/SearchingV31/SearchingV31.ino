 /* Searching V31
  Pull-pullback gravity dynamics with territorial exploration
  Created by Theverant with essential help from Claude Opus 4
  Date: 2025-08-04
  License: GPLv3

  SYSTEM OVERVIEW:
  - REST: Gradual settling to centre of darkness, then energy regeneration with shiver
  - SEEK: Winding path to least-visited perimeter region (escaping despair) 
  - SEARCH: Territorial exploration with progressive gravity transition (energy 60->35)
  - RETURN: Prolonged dramatic struggle against psychological darkness (energy 35->0)

  KEY INNOVATIONS:
  - Spatial weighting prevents clustering in previously explored areas
  - Multi-vector exploration creates natural, organic movement patterns
  - Progressive gravity system with realistic resistance dynamics
  - Grid memory system ensures systematic coverage of all perimeter regions

  Changelog:
  - v31: Prolonged dramatic struggle against darkness - slower energy drain, stronger resistance, tighter centre threshold
  - v30: Gravity-influenced territorial exploration, multi-vector movement system
  - v29: Spatial weighting for systematic coverage, organic perimeter search
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

// ===== HARDWARE SETUP =====
Servo servoX;  // Controls horizontal (X-axis) movement
Servo servoY;  // Controls vertical (Y-axis) movement

const int servoXPin = 3;  // PWM pin for X-axis servo
const int servoYPin = 6;  // PWM pin for Y-axis servo

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

// ===== TERRITORIAL EXPLORATION PARAMETERS =====
// Define the scale and behavior of exploration patterns
const float searchRadius = 18.0;        // Maximum exploration radius from search center
const float dwellRadius = 18.0;         // Radius of dwelling circles
const float microRadius = 3.0;          // Radius of micro-movements for texture
const float memoryBuildRate = 0.15;     // Rate of hysteresis memory accumulation
const float memoryDecayRate = 0.03;     // Rate of memory fade when not dwelling
const float maxMemory = 2.0;            // Maximum memory accumulation level

// ===== GRID MEMORY SYSTEM =====
// Spatial memory system for systematic coverage of exploration space
const int GRID_SIZE = 12;                         // 12x12 grid divides space into cells
const float CELL_SIZE = 180.0 / GRID_SIZE;       // Each cell is 15° x 15°
uint8_t visitGrid[GRID_SIZE][GRID_SIZE];         // Visit count for each grid cell
int currentGridX = -1;                           // Current X grid position
int currentGridY = -1;                           // Current Y grid position

// ===== SERVO POSITION TRACKING =====
// Current physical position of the tentacle
float xAngle = 90.0;  // Current X servo angle (0-180°)
float yAngle = 90.0;  // Current Y servo angle (0-180°)

// ===== STATE MACHINE SYSTEM =====
// Four-state behavioral system
enum State {
  REST,    // Energy regeneration at center
  SEEK,    // Movement toward target region
  SEARCH,  // Territorial exploration
  RETURN   // Gravitational return to center
};
State currentState = REST;  // Start in rest state

// ===== ENERGY SYSTEM =====
// Energy drives the exploration/rest cycle
float energy = 0.0;                    // Current energy level (0-100)
const float maxEnergy = 100.0;         // Maximum energy capacity
const float restPosition = 90.0;       // Center position (gravitational attractor)

// ===== SEEK STATE VARIABLES =====
// Controls movement from center toward target perimeter region
float seekTargetX = 90.0;              // Target X coordinate for seeking
float seekTargetY = 90.0;              // Target Y coordinate for seeking
float seekPhase = 0.0;                 // Phase for winding path generation
float seekProgress = 0.0;              // Progress toward target (0-1)

// ===== SEARCH STATE VARIABLES =====
// Controls territorial exploration behavior at perimeter locations
float searchCenterX = 90.0;            // X center of current search region
float searchCenterY = 90.0;            // Y center of current search region
float searchPhase = 0.0;               // Primary exploration progression phase
float memoryAccumulator = 0.0;         // Hysteresis memory level (affects behavior)
float dwellPhase = 0.0;                // Phase for dwelling circle patterns
float microPhase = 0.0;                // Phase for micro-movement texture

// ===== REST STATE VARIABLES =====
// Controls gradual settling and shiver movement during energy regeneration
float restPhase = 0.0;                 // Phase for shiver movement at centre
float restSettleProgress = 0.0;        // Progress toward final centre position (0-1)
float restStartX = 90.0;               // X position where REST began
float restStartY = 90.0;               // Y position where REST began

// ===== SERVO PHYSICAL LIMITS =====
// Define safe operating range for servo motors
const float xMin = 5.0;    // Minimum safe X angle
const float xMax = 175.0;  // Maximum safe X angle
const float yMin = 5.0;    // Minimum safe Y angle
const float yMax = 175.0;  // Maximum safe Y angle

// ===== TIMING CONTROL =====
// Controls update frequency for smooth movement
unsigned long lastUpdate = 0;           // Timestamp of last update
const unsigned long updateInterval = 20; // Update every 20ms (50Hz)

// ===== FUNCTION DECLARATIONS =====
void printDebug();                                    // Debug output to serial
void updateGridMemory();                             // Update spatial memory system
float calculateDesirability(int gx, int gy);         // Calculate target desirability score
void findLeastVisitedPerimeterTarget();              // Select next exploration target
int getGridX(float angle);                           // Convert servo angle to grid X
int getGridY(float angle);                           // Convert servo angle to grid Y
bool isPerimeterCell(int gx, int gy);               // Check if grid cell is on perimeter

void setup() {
  // Initialize servo motors
  servoX.attach(servoXPin);
  servoY.attach(servoYPin);
  
  // Initialize spatial memory grid (all cells start with 0 visits)
  for(int i = 0; i < GRID_SIZE; i++) {
    for(int j = 0; j < GRID_SIZE; j++) {
      visitGrid[i][j] = 0;
    }
  }
  
  // Initialize serial communication for debugging
  Serial.begin(9600);
  Serial.println("Searching V31 - Pull-Pullback Gravity Dynamics");
  Serial.println("REST -> SEEK -> SEARCH(territorial) -> RETURN(struggle)");
}

void loop() {
  unsigned long now = millis();
  
  // Update at fixed intervals for consistent behavior
  if (now - lastUpdate > updateInterval) {
    lastUpdate = now;
    
    // Update spatial memory system
    updateGridMemory();
    
    // Execute current state behavior
    switch(currentState) {
      case REST:
        updateRest();           // Energy regeneration with small movements
        break;
      case SEEK:
        updateSeek();           // Movement toward target region
        break;
      case SEARCH:
        updateOrganicSearch();  // Territorial exploration
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
// Tracks which grid cells have been visited to enable systematic coverage
void updateGridMemory() {
  // Convert current servo position to grid coordinates
  int gx = getGridX(xAngle);
  int gy = getGridY(yAngle);
  
  // Update visit count when entering a new grid cell
  if(gx != currentGridX || gy != currentGridY) {
    if(gx >= 0 && gx < GRID_SIZE && gy >= 0 && gy < GRID_SIZE) {
      visitGrid[gx][gy]++;          // Increment visit count for this cell
      currentGridX = gx;            // Update current position tracking
      currentGridY = gy;
    }
  }
}

// ===== SPATIAL WEIGHTING ALGORITHM =====
// Calculates desirability score for each grid cell based on visit history and proximity
float calculateDesirability(int gx, int gy) {
  // Base score: lower visit count = higher desirability
  float baseScore = 1.0 / (visitGrid[gx][gy] + 1);
  
  // Proximity penalty: nearby heavily-visited cells reduce desirability
  float proximityPenalty = 0.0;
  
  // Check 5x5 neighborhood around target cell
  for(int dx = -2; dx <= 2; dx++) {
    for(int dy = -2; dy <= 2; dy++) {
      int nx = gx + dx, ny = gy + dy;
      
      // Only consider cells within grid bounds
      if(nx >= 0 && nx < GRID_SIZE && ny >= 0 && ny < GRID_SIZE) {
        float distance = sqrt(dx*dx + dy*dy);
        if(distance > 0) {
          // Nearby cells with high visits create penalty (inverse square law)
          proximityPenalty += visitGrid[nx][ny] / (distance * distance);
        }
      }
    }
  }
  
  // Return final desirability score (higher = better target)
  // Penalty coefficient 0.05 controls how much clustering is avoided
  return baseScore / (1.0 + proximityPenalty * 0.05);
}

// ===== TARGET SELECTION SYSTEM =====
// Selects the most desirable perimeter location for next exploration
void findLeastVisitedPerimeterTarget() {
  int bestGX = 0, bestGY = 0;     // Grid coordinates of best target
  float bestScore = 0.0;          // Highest desirability score found
  
  // Debug output header
  Serial.print("Perimeter analysis: ");
  
  // Scan all perimeter cells to find highest desirability score
  for(int gx = 0; gx < GRID_SIZE; gx++) {
    for(int gy = 0; gy < GRID_SIZE; gy++) {
      if(isPerimeterCell(gx, gy)) {
        float score = calculateDesirability(gx, gy);
        
        // Debug output: show visits and desirability score for each cell
        Serial.print("("); Serial.print(gx); Serial.print(","); Serial.print(gy); 
        Serial.print(":"); Serial.print(visitGrid[gx][gy]); 
        Serial.print("/"); Serial.print(score, 2); Serial.print(") ");
        
        // Track best target found so far
        if(score > bestScore) {
          bestScore = score;
          bestGX = gx;
          bestGY = gy;
        }
      }
    }
  }
  Serial.println();
  
  // Convert grid coordinates to servo angles (cell center)
  seekTargetX = (bestGX + 0.5) * CELL_SIZE;
  seekTargetY = (bestGY + 0.5) * CELL_SIZE;
  
  // ===== SMART BOUNDARY CHECKING =====
  // Only constrain target if search pattern would exceed servo limits
  float worstCaseXMin = seekTargetX - searchRadius;  // Leftmost exploration point
  float worstCaseXMax = seekTargetX + searchRadius;  // Rightmost exploration point
  float worstCaseYMin = seekTargetY - searchRadius;  // Bottom exploration point
  float worstCaseYMax = seekTargetY + searchRadius;  // Top exploration point
  
  bool constrained = false;
  
  // Apply constraints only when necessary to prevent servo limit violations
  if(worstCaseXMin < xMin + 5) {
    seekTargetX = xMin + searchRadius + 5;  // Move target right
    constrained = true;
  }
  if(worstCaseXMax > xMax - 5) {
    seekTargetX = xMax - searchRadius - 5;  // Move target left
    constrained = true;
  }
  if(worstCaseYMin < yMin + 5) {
    seekTargetY = yMin + searchRadius + 5;  // Move target up
    constrained = true;
  }
  if(worstCaseYMax > yMax - 5) {
    seekTargetY = yMax - searchRadius - 5;  // Move target down
    constrained = true;
  }
  
  // Debug output: show final target selection
  Serial.print("Target selection: bestGX="); Serial.print(bestGX);
  Serial.print(" bestGY="); Serial.print(bestGY); 
  Serial.print(" bestScore="); Serial.print(bestScore, 3);
  Serial.print(" -> ("); Serial.print((int)seekTargetX);
  Serial.print(","); Serial.print((int)seekTargetY); Serial.print(")");
  if(constrained) Serial.print(" [CONSTRAINED]");  // Indicate if target was adjusted
  Serial.println();
}

// ===== PERIMETER DETECTION =====
// Determines if a grid cell is on the perimeter (outer 2 rows/columns)
bool isPerimeterCell(int gx, int gy) {
  return (gx <= 1 || gx >= GRID_SIZE-2 || gy <= 1 || gy >= GRID_SIZE-2);
}

// ===== COORDINATE CONVERSION UTILITIES =====
// Convert servo angle to grid X coordinate
int getGridX(float angle) {
  return constrain((int)(angle / CELL_SIZE), 0, GRID_SIZE-1);
}

// Convert servo angle to grid Y coordinate
int getGridY(float angle) {
  return constrain((int)(angle / CELL_SIZE), 0, GRID_SIZE-1);
}

// ===== REST STATE BEHAVIOR =====
// Gradual settling to centre of darkness followed by shiver movements during energy regeneration
void updateRest() {
  // ===== GRADUAL SETTLING PHASE =====
  // Smoothly settle from struggle end position to centre of darkness
  if (restSettleProgress < 1.0) {
    restSettleProgress += 0.006;  // ~3.3 seconds to settle completely (slower)
    if (restSettleProgress > 1.0) restSettleProgress = 1.0;
    
    // Eased interpolation from start position to centre (smooth deceleration)
    float easedProgress = restSettleProgress * restSettleProgress * (3.0 - 2.0 * restSettleProgress); // Smoothstep
    float settleX = restStartX + (restPosition - restStartX) * easedProgress;
    float settleY = restStartY + (restPosition - restStartY) * easedProgress;
    
    // Apply very gentle movement during settling - tentacle is exhausted
    xAngle += (settleX - xAngle) * 0.04;  // Very slow settling into darkness
    yAngle += (settleY - yAngle) * 0.04;
    
  } else {
    // ===== SHIVER PHASE =====
    // Small movements representing isolation, loneliness, self-loathing
    restPhase += REST_PHASE_SPEED;
    float shiverX = restPosition + cos(restPhase) * 2.5;           // Slightly smaller shiver
    float shiverY = restPosition + sin(restPhase * 1.3) * 2.0;     // More constrained movement
    
    // Apply shiver movement - tentacle is consumed by darkness
    xAngle += (shiverX - xAngle) * REST_SMOOTHING;
    yAngle += (shiverY - yAngle) * REST_SMOOTHING;
  }
  
  // Regenerate energy slowly - building courage to leave darkness again
  energy += REST_ENERGY_RATE;
  
  // Transition to SEEK when fully energized - gathering strength to escape despair
  if (energy >= 100.0) {
    findLeastVisitedPerimeterTarget();  // Select next exploration target using spatial memory
    
    // Reset seeking parameters
    seekProgress = 0.0;
    seekPhase = 0.0;
    currentState = SEEK;
    
    // Debug output
    Serial.print("REST -> SEEK targeting (");
    Serial.print((int)seekTargetX); Serial.print(",");
    Serial.print((int)seekTargetY); Serial.println(") - escaping darkness");
  }
  
  // Enforce servo limits
  xAngle = constrain(xAngle, 0, 180);
  yAngle = constrain(yAngle, 0, 180);
  
  // Update servo positions
  servoX.write((int)xAngle);
  servoY.write((int)yAngle);
}

// ===== SEEK STATE BEHAVIOR =====
// Movement from center toward selected perimeter target with winding path
void updateSeek() {
  // Update progress and winding path phase
  seekProgress += SEEK_PROGRESS_RATE;      // Linear progress toward target
  seekPhase += SEEK_PHASE_SPEED;           // Phase for path variation
  
  // Acceleration system: move faster when further from center
  float distFromCenter = sqrt(pow(xAngle - 90, 2) + pow(yAngle - 90, 2));
  float speedBoost = 1.0 + (distFromCenter / 90.0) * 2.0;  // 1x to 3x speed boost
  
  // Generate winding path components (diminish as we approach target)
  float windX = sin(seekPhase * 2.1) * 20.0 * (1.0 - seekProgress);
  float windY = cos(seekPhase * 1.7) * 15.0 * (1.0 - seekProgress);
  
  // Calculate target position: interpolate from center to target with winding
  float targetX = restPosition + (seekTargetX - restPosition) * seekProgress + windX;
  float targetY = restPosition + (seekTargetY - restPosition) * seekProgress + windY;
  
  // Energy cost for seeking (small drain)
  energy -= SEEK_ENERGY_RATE;
  
  // Transition to SEARCH when target reached
  if (seekProgress >= 1.0) {
    currentState = SEARCH;
    
    // Set search center to current arrival position
    searchCenterX = xAngle;
    searchCenterY = yAngle;
    
    // Reset all search-related phase variables
    searchPhase = 0.0;
    memoryAccumulator = 0.0;
    dwellPhase = 0.0;
    microPhase = 0.0;
    
    Serial.println("SEEK -> SEARCH (territorial exploration)");
  }
  
  // Apply movement with speed boost
  float smoothing = SEEK_SMOOTHING_BASE * speedBoost;
  xAngle += (targetX - xAngle) * smoothing;
  yAngle += (targetY - yAngle) * smoothing;
  
  // Enforce servo limits
  xAngle = constrain(xAngle, 0, 180);
  yAngle = constrain(yAngle, 0, 180);
  
  // Update servo positions
  servoX.write((int)xAngle);
  servoY.write((int)yAngle);
}

// ===== SEARCH STATE BEHAVIOR =====
// Territorial exploration with gravity-influenced multi-vector movement system
void updateOrganicSearch() {
  // Update organic behavior phases
  dwellPhase += SEARCH_DWELL_FREQ;    // Controls dwelling intensity
  microPhase += SEARCH_MICRO_FREQ;    // Controls micro-movements
  
  // ===== HYSTERESIS MEMORY SYSTEM =====
  // Calculate dwelling intensity (0-1) based on dwelling phase
  float dwellIntensity = (sin(dwellPhase) + 1.0) * 0.5;
  
  // Build memory when dwelling strongly, decay when moving
  if (dwellIntensity > 0.6) {
    // Strong dwelling: accumulate memory
    memoryAccumulator += memoryBuildRate * dwellIntensity;
    if (memoryAccumulator > maxMemory) memoryAccumulator = maxMemory;
  } else {
    // Weak dwelling: memory decays
    memoryAccumulator -= memoryDecayRate;
    if (memoryAccumulator < 0) memoryAccumulator = 0;
  }
  
  // Hysteresis effect: high memory slows exploration speed
  float effectiveSpeed = SEARCH_BASE_SPEED * (1.0 - (memoryAccumulator / maxMemory) * 0.7);
  searchPhase += effectiveSpeed;
  
  // ===== GRAVITY-INFLUENCED EXPLORATION SYSTEM =====
  
  // Calculate gravitational vector toward center (90,90)
  float gravityX = (90.0 - searchCenterX);    // X component of gravity
  float gravityY = (90.0 - searchCenterY);    // Y component of gravity
  float gravityDistance = sqrt(gravityX * gravityX + gravityY * gravityY);
  
  // Normalize gravity vector and apply base strength
  if(gravityDistance > 0) {
    gravityX = (gravityX / gravityDistance) * 0.3;  // 0.3 = base gravity strength
    gravityY = (gravityY / gravityDistance) * 0.3;
  }
  
  // ===== VARIABLE EXPLORATION RADIUS =====
  // Dynamic exploration range: 5-23° based on memory and phase variation
  float explorationRange = 8.0 +                           // Base radius
                           sin(searchPhase * 0.7) * 12.0 +  // Phase variation
                           memoryAccumulator * 3.0;         // Memory expansion
  
  // ===== PRIMARY EXPLORATION VECTOR =====
  // Main sweeping pattern with elliptical shape for variety
  float exploreX = cos(searchPhase * 1.3) * explorationRange;
  float exploreY = sin(searchPhase * 0.9) * explorationRange * 0.8;  // 0.8 = elliptical factor
  
  // ===== SECONDARY EXPLORATION VECTOR =====
  // Perpendicular sweeps for broader coverage
  float secondaryPhase = searchPhase * 2.1 + dwellPhase * 0.5;      // Complex phase relationship
  float secondaryRange = 6.0 + sin(microPhase * 0.3) * 4.0;        // 2-10° variable range
  float secondaryX = cos(secondaryPhase + 1.57) * secondaryRange;   // +90° offset (perpendicular)
  float secondaryY = sin(secondaryPhase + 1.57) * secondaryRange;
  
  // ===== GRAVITY RESISTANCE SYSTEM =====
  // Resistance varies with memory level and exploration intent
  float gravityResistance = 0.6 + memoryAccumulator * 0.3;  // 0.6-1.2 resistance range
  float resistedGravityX = gravityX / gravityResistance;
  float resistedGravityY = gravityY / gravityResistance;
  
  // ===== CENTER-SEEKING EXCURSIONS =====
  // Occasional deliberate movement toward center when memory is low
  float centerSeekingFactor = (1.0 - memoryAccumulator / maxMemory) * 0.4;
  if(sin(searchPhase * 0.2) > 0.7 && memoryAccumulator < 1.0) {
    // Amplify gravity during center-seeking episodes
    resistedGravityX *= (1.0 + centerSeekingFactor * 3.0);
    resistedGravityY *= (1.0 + centerSeekingFactor * 3.0);
  }
  
  // ===== ORGANIC MICRO-MOVEMENTS =====
  // Small movements for organic texture
  float microX = cos(microPhase) * 2.5;
  float microY = sin(microPhase * 1.3) * 2.0;
  
  // ===== VECTOR COMBINATION =====
  // Combine all movement vectors relative to search region center
  float xTarget = searchCenterX +    // Search region center
                  exploreX +         // Primary exploration
                  secondaryX +       // Secondary exploration
                  resistedGravityX + // Gravity influence
                  microX;            // Micro texture
                  
  float yTarget = searchCenterY +    // Search region center
                  exploreY +         // Primary exploration
                  secondaryY +       // Secondary exploration
                  resistedGravityY + // Gravity influence
                  microY;            // Micro texture
  
  // ===== ENHANCED BOUNDARY SAFETY =====
  // Gradient approach to boundaries prevents harsh clipping
  float bufferZone = 8.0;  // Distance from limits where pushback begins
  
  // Apply soft pushback when approaching boundaries
  if(xTarget < xMin + bufferZone) {
    float pushback = (xMin + bufferZone - xTarget) * 0.3;
    xTarget += pushback;
  }
  if(xTarget > xMax - bufferZone) {
    float pushback = (xTarget - (xMax - bufferZone)) * 0.3;
    xTarget -= pushback;
  }
  if(yTarget < yMin + bufferZone) {
    float pushback = (yMin + bufferZone - yTarget) * 0.3;
    yTarget += pushback;
  }
  if(yTarget > yMax - bufferZone) {
    float pushback = (yTarget - (yMax - bufferZone)) * 0.3;
    yTarget -= pushback;
  }
  
  // Final hard boundary enforcement as safety net
  xTarget = constrain(xTarget, xMin, xMax);
  yTarget = constrain(yTarget, yMin, yMax);
  
  // ===== ENERGY DRAIN AND PROGRESSIVE GRAVITY =====
  energy -= SEARCH_ENERGY_RATE;  // Standard energy drain during exploration
  
  // Progressive gravity influence starts much earlier for smoother transition
  float gravityTransitionFactor = 0.0;
  if(energy < 60.0) {
    gravityTransitionFactor = (60.0 - energy) / 60.0;  // 0 to 1 as energy drops 60->0
    
    // Gradually increasing gravity influence toward center
    float progressiveGravityStrength = 0.01 + gravityTransitionFactor * 0.08;  // 0.01 to 0.09
    float progressiveGravityX = (90.0 - xAngle) * progressiveGravityStrength;
    float progressiveGravityY = (90.0 - yAngle) * progressiveGravityStrength;
    
    xTarget += progressiveGravityX;
    yTarget += progressiveGravityY;
    
    // Gradually reduce exploration range as gravity takes hold
    float explorationDamping = 1.0 - (gravityTransitionFactor * 0.4);  // Reduce to 60% of normal
    xTarget = searchCenterX + (xTarget - searchCenterX) * explorationDamping;
    yTarget = searchCenterY + (yTarget - searchCenterY) * explorationDamping;
  }
  
  // Transition to RETURN when gravity becomes dominant
  if (energy <= 35.0) {  // Higher threshold, but by now gravity is already strong
    currentState = RETURN;
    Serial.println("SEARCH -> RETURN (gravity dominance)");
  }
  
  // Apply smooth movement
  xAngle += (xTarget - xAngle) * SEARCH_SMOOTHING_BASE;
  yAngle += (yTarget - yAngle) * SEARCH_SMOOTHING_BASE;
  
  // Enforce servo limits
  xAngle = constrain(xAngle, 0, 180);
  yAngle = constrain(yAngle, 0, 180);
  
  // Update servo positions
  servoX.write((int)xAngle);
  servoY.write((int)yAngle);
  
  // Keep phase variables bounded to prevent overflow
  if (searchPhase > 6.28318) searchPhase -= 6.28318;   // 2π
  if (dwellPhase > 6.28318) dwellPhase -= 6.28318;     // 2π
  if (microPhase > 6.28318) microPhase -= 6.28318;     // 2π
}

// ===== RETURN STATE BEHAVIOR =====
// Prolonged struggle against psychological darkness - tentacle fights being pulled to centre
void updateReturn() {
  // ===== GRAVITY VECTOR CALCULATION =====
  // Calculate normalized vector toward centre (the place of darkness and despair)
  float gravityX = (90.0 - xAngle);    // X distance to centre
  float gravityY = (90.0 - yAngle);    // Y distance to centre
  float distanceFromCentre = sqrt(gravityX * gravityX + gravityY * gravityY);
  
  // Normalize gravity vector for consistent direction
  if(distanceFromCentre > 0.1) {
    gravityX = gravityX / distanceFromCentre;
    gravityY = gravityY / distanceFromCentre;
  }
  
  // ===== PROGRESSIVE GRAVITY STRENGTH =====
  // Psychological gravity increases as the tentacle weakens, but starts gentler
  float energyFactor = (35.0 - energy) / 35.0;         // 0 to 1 as energy drops 35->0
  float baseGravityStrength = 0.09 + energyFactor * 0.8; // 0.09 to 0.89 (reduced max strength)
  
  // ===== DRAMATIC RESISTANCE SYSTEM =====
  // Tentacle fights desperately against being pulled into darkness
  float resistanceStrength = energy / 35.0;  // 1.0 to 0.0 as energy depletes
  
  // Resistance onset factor: fighting behavior builds quickly then sustains
  float resistanceOnset = 1.0 - (energy / 35.0);  // 0 to 1 as energy depletes
  resistanceOnset = sqrt(resistanceOnset);  // Square root for faster initial buildup
  
  // ===== COMPLEX STRUGGLE DYNAMICS =====
  // Multi-layered resistance movement showing desperate fight against despair
  microPhase += SEARCH_MICRO_FREQ * 0.8;  // Struggle movement speed
  
  // Primary struggle: large, desperate movements
  float primaryStruggle = sin(microPhase * 0.7) * 2.0 + cos(microPhase * 1.1) * 1.5;
  
  // Secondary struggle: rapid, panicked movements
  float secondaryStruggle = sin(microPhase * 3.2) * 0.8 + sin(microPhase * 4.7) * 0.6;
  
  // Tertiary struggle: erratic, final resistance
  float tertiaryStruggle = sin(microPhase * 7.1) * 0.4 * (1.0 - energyFactor);
  
  // Combined struggle phase
  float totalStruggle = primaryStruggle + secondaryStruggle + tertiaryStruggle;
  
  // ===== RESISTANCE FORCE VECTORS (MUCH STRONGER) =====
  // Forces represent desperate fight against psychological gravity
  float resistanceIntensity = resistanceStrength * resistanceOnset;
  
  // Perpendicular resistance (tentacle tries desperately to move away from centre)
  float resistanceX = cos(microPhase * 0.9) * resistanceIntensity * 15.0;  // Much stronger
  float resistanceY = sin(microPhase * 1.2) * resistanceIntensity * 12.0;  // Much stronger
  
  // Direct pullback against gravity (direct opposition to despair)
  float pullbackIntensity = resistanceIntensity * resistanceOnset;
  float pullbackX = -gravityX * totalStruggle * pullbackIntensity * 8.0;  // Increased
  float pullbackY = -gravityY * totalStruggle * pullbackIntensity * 8.0;  // Increased
  
  // Chaotic struggle movements (desperation creates erratic behavior)
  float chaosX = sin(microPhase * 5.3) * cos(microPhase * 2.1) * resistanceIntensity * 6.0;
  float chaosY = cos(microPhase * 4.7) * sin(microPhase * 3.4) * resistanceIntensity * 5.0;
  
  // ===== DISTANCE-BASED GRAVITY AMPLIFICATION =====
  // Psychological gravity intensifies as the tentacle nears the centre of despair
  float proximityFactor = 1.0 - (distanceFromCentre / 90.0);  // 0 to 1 (stronger near centre)
  float effectiveGravityStrength = baseGravityStrength * (1.0 + proximityFactor * 0.8);  // Less amplification
  
  // ===== FINAL GRAVITATIONAL FORCE =====
  // Apply psychological gravity pulling toward despair
  float gravitationalPullX = gravityX * effectiveGravityStrength;
  float gravitationalPullY = gravityY * effectiveGravityStrength;
  
  // ===== FORCE COMBINATION =====
  // Combine all forces: gravity toward darkness + desperate resistance
  float xTarget = xAngle +              // Current position
                  gravitationalPullX +  // Inexorable pull toward darkness
                  resistanceX +         // Perpendicular resistance
                  pullbackX +           // Direct opposition
                  chaosX;               // Chaotic desperation
                  
  float yTarget = yAngle +              // Current position
                  gravitationalPullY +  // Inexorable pull toward darkness
                  resistanceY +         // Perpendicular resistance
                  pullbackY +           // Direct opposition
                  chaosY;               // Chaotic desperation
  
  // ===== STRUGGLE DYNAMICS =====
  // Movement becomes more erratic as the fight intensifies
  float struggleDamping = 0.4 + resistanceStrength * 0.6;  // 0.4 to 1.0
  
  // Slower energy drain = longer struggle against despair
  float energyDrainRate = RETURN_ENERGY_RATE * (0.6 + resistanceStrength * 0.4);
  energy -= energyDrainRate;
  if (energy < 0) energy = 0;  // Prevent negative energy
  
  // ===== SURRENDER TO DARKNESS DETECTION =====
  // Only surrender when very close to centre or completely exhausted
  float restThreshold = 3.0 + resistanceStrength * 2.0;  // 3-5° (much tighter - must reach centre)
  float distFromRest = sqrt(pow(xAngle - 90, 2) + pow(yAngle - 90, 2));
  
  // Transition to REST only when truly defeated or at centre of darkness
  if (distFromRest < restThreshold || energy <= 0.0) {
    currentState = REST;
    energy = 0.0;
    
    // Initialize gradual settling to centre
    restStartX = xAngle;        // Remember where REST began
    restStartY = yAngle;        
    restSettleProgress = 0.0;   // Start settling process
    restPhase = 0.0;            // Reset shiver phase
    
    Serial.println("RETURN -> REST (consumed by darkness)");
  }
  
  // ===== MOVEMENT APPLICATION =====
  // Variable smoothing creates more dramatic, less predictable movement
  float smoothing = RETURN_SMOOTHING_BASE * struggleDamping * (0.5 + energyFactor * 0.5);
  
  xAngle += (xTarget - xAngle) * smoothing;
  yAngle += (yTarget - yAngle) * smoothing;
  
  // Enforce servo limits
  xAngle = constrain(xAngle, 0, 180);
  yAngle = constrain(yAngle, 0, 180);
  
  // Update servo positions
  servoX.write((int)xAngle);
  servoY.write((int)yAngle);
}

// ===== DEBUG OUTPUT SYSTEM =====
// Comprehensive status information for monitoring and debugging
void printDebug() {
  static unsigned long lastPrint = 0;
  
  // Print debug info every second
  if (millis() - lastPrint > 1000) {
    lastPrint = millis();
    
    // ===== BASIC STATUS =====
    Serial.print("State: ");
    switch(currentState) {
      case REST: Serial.print("REST"); break;
      case SEEK: Serial.print("SEEK"); break;
      case SEARCH: Serial.print("SEARCH"); break;
      case RETURN: Serial.print("RETURN"); break;
    }
    
    // Energy and position (always shown)
    Serial.print(" | Energy: "); Serial.print(energy, 1);
    Serial.print(" | Pos: "); Serial.print((int)xAngle);
    Serial.print(","); Serial.print((int)yAngle);
    
    // ===== SEARCH-SPECIFIC DEBUG INFO =====
    if (currentState == SEARCH) {
      Serial.print(" | Memory: "); Serial.print(memoryAccumulator, 2);
      Serial.print(" | Center: ("); Serial.print((int)searchCenterX);
      Serial.print(","); Serial.print((int)searchCenterY); Serial.print(")");
      Serial.print(" | Radius: "); Serial.print((int)searchRadius);
    }
    
    // ===== REST-SPECIFIC DEBUG INFO =====
    if (currentState == REST) {
      if (restSettleProgress < 1.0) {
        Serial.print(" | Settling: "); Serial.print(restSettleProgress, 2);
      } else {
        Serial.print(" | Shivering in darkness");
      }
    }
    
    // ===== RETURN-SPECIFIC DEBUG INFO =====
    if (currentState == RETURN) {
      // Distance from centre (shows progress toward darkness)
      float distanceFromCentre = sqrt(pow(xAngle - 90, 2) + pow(yAngle - 90, 2));
      Serial.print(" | Distance: "); Serial.print((int)distanceFromCentre);
      
      // Resistance strength (shows struggle intensity against despair)
      float resistanceStrength = energy / 35.0;  // Updated to match new threshold
      Serial.print(" | Resistance: "); Serial.print(resistanceStrength, 2);
      
      // Resistance onset (shows how much struggle behavior is active)
      float resistanceOnset = 1.0 - (energy / 35.0);
      resistanceOnset = sqrt(resistanceOnset);  // Match the calculation in updateReturn
      Serial.print(" | Fight: "); Serial.print(resistanceOnset, 2);
    }
    
    // Show gravity transition during SEARCH
    if (currentState == SEARCH && energy < 60.0) {
      float gravityTransitionFactor = (60.0 - energy) / 60.0;
      Serial.print(" | GravTrans: "); Serial.print(gravityTransitionFactor, 2);
    }
    
    // ===== SPATIAL MEMORY STATUS =====
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