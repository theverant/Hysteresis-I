/*
  H1XYXY V1 - RP2040Zero Dual Core Implementation
  3D gravitational wells with metabolic depletion zones
  Created by Theverant based on V31 territorial exploration
  Date: 2025-08-10
  License: GPLv3

  DUAL CORE ARCHITECTURE:
  - Core 0: Real-time motor control, servo synchronization, sensor processing
  - Core 1: Behavioral algorithms, spatial memory, gravitational calculations
  
  RP2040ZERO ADVANTAGES:
  - 264KB RAM for complex 3D spatial memory
  - 133MHz dual-core processing
  - Compact form factor for tentacle integration
  - Future-ready for BLDC + encoder migration

  SYSTEM OVERVIEW:
  - REST: Gradual settling to center with compound curvature relaxation
  - SEEK: 3D path to least-visited region avoiding gravitational wells  
  - SEARCH: Volumetric exploration with depletion zone formation
  - RETURN: 3D gravitational struggle using compound curvature escape

  DUAL XY SERVO SYSTEM:
  - Base XY: Primary movement control (servoBaseX, servoBaseY)
  - Tip XY: Secondary curvature control (servoTipX, servoTipY)
  - Compound curvature: Coordinated movement for smooth S-curves
  - Prehensile capability: Opposed movement for coiling behaviors

  3D GRAVITATIONAL WELLS:
  - Metabolic depletion zones form from intensive exploration
  - Zones concentrate over time into point wells
  - Wells scatter when traversed but at accelerated energy cost
  - External stimuli create additional gravitational obstacles

  Changelog:
  - v1: RP2040Zero dual core implementation with 3D wells based on V31
*/

#include <Servo.h>

// ===== DUAL CORE SHARED DATA =====
// Mutex for thread-safe communication between cores
volatile bool core0_ready = false;
volatile bool core1_ready = false;

// Shared servo target positions (Core 1 writes, Core 0 reads)
volatile float target_baseX = 90.0;
volatile float target_baseY = 90.0;
volatile float target_tipX = 90.0;
volatile float target_tipY = 90.0;

// Shared position feedback (Core 0 writes, Core 1 reads)
volatile float current_baseX = 90.0;
volatile float current_baseY = 90.0;
volatile float current_tipX = 90.0;
volatile float current_tipY = 90.0;

// Shared sensor data (Core 0 writes, Core 1 reads)
volatile float light_level = 0.0;
volatile bool external_stimulus = false;

// Core synchronization flags
volatile bool position_updated = false;
volatile bool target_updated = false;

// ===== HARDWARE CONFIGURATION =====
// RP2040Zero GPIO pin assignments
const int servoBaseXPin = 0;   // GPIO0 for base X servo
const int servoBaseYPin = 1;   // GPIO1 for base Y servo
const int servoTipXPin = 2;    // GPIO2 for tip X servo
const int servoTipYPin = 3;    // GPIO3 for tip Y servo
const int photoDiodePin = 26;  // GPIO26 (A0) for light sensor
const int ledDataPin = 16;     // GPIO16 for WS2812 LED communication

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

// DUAL CORE TIMING - Hardware control frequencies
const unsigned long BEHAVIORAL_UPDATE_INTERVAL = 20;    // 50Hz behavioral updates (Core 1)
const unsigned long SERVO_UPDATE_INTERVAL = 5;          // 200Hz servo control (Core 0)
const unsigned long SENSOR_UPDATE_INTERVAL = 10;        // 100Hz sensor reading (Core 0)
const float SERVO_SMOOTHING = 0.15;                     // Real-time servo interpolation speed

// DEPLETION ZONE TIMING - Metabolic exhaustion parameters
const float DEPLETION_FORMATION_RATE = 0.3 * MASTER_SPEED;  // Rate of depletion accumulation
const float DEPLETION_DECAY_RATE = 0.01 * MASTER_SPEED;     // Natural decay rate
const float DEPLETION_SCATTER_RATE = 0.8;                   // Scattering efficiency (unitless)

// GRAVITATIONAL WELL TIMING - Well formation and behavior
const float WELL_FORMATION_THRESHOLD = 8.0;             // Depletion level for well formation
const float WELL_TRAVERSE_COST = 2.0;                   // Energy multiplier for well traversal
const float WELL_DECAY_RATE = 0.005 * MASTER_SPEED;     // Natural well weakening over time

// ===== 3D SPATIAL MEMORY SYSTEM =====
const int GRID_SIZE = 12;                         // 12x12x12 grid (1728 cells)
const float CELL_SIZE = 180.0 / GRID_SIZE;       // Each cell is 15° x 15° x 15°
uint8_t visitGrid[GRID_SIZE][GRID_SIZE][GRID_SIZE]; // 3D visit count
uint8_t depletionGrid[GRID_SIZE][GRID_SIZE][GRID_SIZE]; // Metabolic depletion levels

// ===== COMPOUND POSITION TRACKING =====
// 3D position calculated from compound servo angles
float pos3DX = 90.0;   // Combined X position
float pos3DY = 90.0;   // Combined Y position  
float pos3DZ = 90.0;   // Combined Z position

// ===== STATE MACHINE SYSTEM =====
enum State {
  REST,
  SEEK,
  SEARCH,
  RETURN
};
State currentState = REST;

// ===== ENERGY SYSTEM =====
float energy = 0.0;
const float maxEnergy = 100.0;
const float restPosition = 90.0;

// ===== SEEK STATE VARIABLES =====
float seekTargetBaseX = 90.0, seekTargetBaseY = 90.0;
float seekTargetTipX = 90.0, seekTargetTipY = 90.0;
float seekPhase = 0.0;
float seekProgress = 0.0;

// ===== SEARCH STATE VARIABLES =====
float searchCenterX = 90.0, searchCenterY = 90.0, searchCenterZ = 90.0;
float searchPhase = 0.0;
float memoryAccumulator = 0.0;
float dwellPhase = 0.0;
float microPhase = 0.0;

// ===== 3D GRAVITATIONAL WELLS =====
struct GravityWell {
  float x, y, z;           // Well position
  float strength;          // Gravitational strength
  float coronaRadius;      // Wide influence zone
  float tailRadius;        // Narrow capture zone
  unsigned long formed;    // Formation timestamp
  bool active;             // Well state
};

const int MAX_WELLS = 32;    // Increased capacity with RP2040 RAM
GravityWell wells[MAX_WELLS];
int activeWells = 0;

// ===== DEPLETION ZONE PARAMETERS =====
// Well formation and interaction parameters (timing controls above)
const float WELL_FORMATION_THRESHOLD = 8.0;     // Depletion level for well formation
const float WELL_TRAVERSE_COST = 2.0;          // Energy multiplier for well traversal

// ===== SERVO LIMITS =====
const float servoMin = 5.0;
const float servoMax = 175.0;

// ===== TIMING CONTROL =====
unsigned long lastUpdate = 0;

// ===== FUNCTION DECLARATIONS =====
void setup1();                                 // Core 1 setup
void loop1();                                  // Core 1 loop
void updateCompoundPosition();
void calculateGravitationalForces(float& forceX, float& forceY, float& forceZ);
void updateDepletionZones();
void checkWellFormation();
void scatterWellsNearPosition(float x, float y, float z);
void findLeastDepletedTarget();
void setServoTargets(float targetBaseX, float targetBaseY, float targetTipX, float targetTipY);
int getGrid3DX(float angle);
int getGrid3DY(float angle);
int getGrid3DZ(float angle);

// ===== CORE 0: REAL-TIME MOTOR CONTROL =====
void setup() {
  Serial.begin(115200);
  
  // Initialize hardware on Core 0
  pinMode(photoDiodePin, INPUT);
  pinMode(ledDataPin, OUTPUT);
  
  Serial.println("Core 0: Motor control and sensor processing");
  core0_ready = true;
  
  // Wait for Core 1 to initialize
  while(!core1_ready) {
    delay(1);
  }
  
  Serial.println("H1XYXY V1 - RP2040Zero Dual Core Implementation");
  Serial.println("3D gravitational wells with metabolic depletion zones");
}

void loop() {
  static Servo servoBaseX, servoBaseY, servoTipX, servoTipY;
  static bool servos_attached = false;
  static unsigned long lastServoUpdate = 0;
  static float smooth_baseX = 90.0, smooth_baseY = 90.0;
  static float smooth_tipX = 90.0, smooth_tipY = 90.0;
  
  unsigned long now = micros();
  
  // Initialize servos on first run
  if(!servos_attached) {
    servoBaseX.attach(servoBaseXPin);
    servoBaseY.attach(servoBaseYPin);
    servoTipX.attach(servoTipXPin);
    servoTipY.attach(servoTipYPin);
    servos_attached = true;
  }
  
  // High-frequency servo control loop (200Hz)
  if(now - lastServoUpdate >= SERVO_UPDATE_INTERVAL * 1000) {
    lastServoUpdate = now;
    
    // Smooth servo interpolation for compound curvature coordination
    if(target_updated) {
      smooth_baseX += (target_baseX - smooth_baseX) * SERVO_SMOOTHING;
      smooth_baseY += (target_baseY - smooth_baseY) * SERVO_SMOOTHING;
      smooth_tipX += (target_tipX - smooth_tipX) * SERVO_SMOOTHING;
      smooth_tipY += (target_tipY - smooth_tipY) * SERVO_SMOOTHING;
      
      // Constrain and update servos atomically
      smooth_baseX = constrain(smooth_baseX, servoMin, servoMax);
      smooth_baseY = constrain(smooth_baseY, servoMin, servoMax);
      smooth_tipX = constrain(smooth_tipX, servoMin, servoMax);
      smooth_tipY = constrain(smooth_tipY, servoMin, servoMax);
      
      // Synchronized servo writes for compound curvature
      servoBaseX.write((int)smooth_baseX);
      servoBaseY.write((int)smooth_baseY);
      servoTipX.write((int)smooth_tipX);
      servoTipY.write((int)smooth_tipY);
      
      // Update shared position feedback
      current_baseX = smooth_baseX;
      current_baseY = smooth_baseY;
      current_tipX = smooth_tipX;
      current_tipY = smooth_tipY;
      
      position_updated = true;
      target_updated = false;
    }
  }
  
  // Sensor processing (lower frequency)
  static unsigned long lastSensorRead = 0;
  if(now - lastSensorRead >= SENSOR_UPDATE_INTERVAL * 1000) {  // 100Hz sensor reading
    lastSensorRead = now;
    
    // Read photodiode for light level and external stimulus detection
    int raw_light = analogRead(photoDiodePin);
    light_level = raw_light * (3.3 / 1023.0);  // Convert to voltage
    
    // Detect external stimulus (shadow, disruption)
    static float baseline_light = light_level;
    static int stable_readings = 0;
    
    if(abs(light_level - baseline_light) < 0.1) {
      stable_readings++;
      if(stable_readings > 100) {  // 1 second of stability
        baseline_light = light_level;
        external_stimulus = false;
      }
    } else {
      stable_readings = 0;
      if(abs(light_level - baseline_light) > 0.3) {  // Significant change
        external_stimulus = true;
      }
    }
  }
  
  // LED communication processing will be added here
  // TODO: Integrate steganographic brightness modulation
  
  // Core 0 runs continuously for real-time control
}

// ===== CORE 1: BEHAVIORAL ALGORITHMS =====
void setup1() {
  // Initialize 3D memory grids
  for(int i = 0; i < GRID_SIZE; i++) {
    for(int j = 0; j < GRID_SIZE; j++) {
      for(int k = 0; k < GRID_SIZE; k++) {
        visitGrid[i][j][k] = 0;
        depletionGrid[i][j][k] = 0;
      }
    }
  }
  
  // Initialize gravitational wells
  for(int i = 0; i < MAX_WELLS; i++) {
    wells[i].active = false;
  }
  
  Serial.println("Core 1: Behavioral algorithms and spatial memory");
  core1_ready = true;
  
  // Wait for Core 0 to initialize
  while(!core0_ready) {
    delay(1);
  }
}

void loop1() {
  unsigned long now = millis();
  
  if (now - lastUpdate > BEHAVIORAL_UPDATE_INTERVAL) {
    lastUpdate = now;
    
    // Update 3D position from compound servo feedback
    if(position_updated) {
      updateCompoundPosition();
      position_updated = false;
    }
    
    // Handle external stimulus creating gravitational wells
    if(external_stimulus) {
      createExternalWell(pos3DX, pos3DY, pos3DZ);
    }
    
    // Update depletion zones and gravitational wells
    updateDepletionZones();
    checkWellFormation();
    
    // Execute current state behavior
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
  
  // Core 1 can handle additional processing during idle time
  // TODO: LED communication protocol processing
  // TODO: Multi-tentacle coordination
}

// ===== 3D POSITION CALCULATION =====
void updateCompoundPosition() {
  // Calculate combined 3D position from dual XY servo feedback
  float baseFactor = 0.7;  // Base section influence
  float tipFactor = 0.3;   // Tip section influence
  
  pos3DX = (current_baseX * baseFactor) + (current_tipX * tipFactor);
  pos3DY = (current_baseY * baseFactor) + (current_tipY * tipFactor);
  
  // Z position calculated from compound curvature difference
  float curvatureDiff = abs((current_tipX - current_baseX)) + abs((current_tipY - current_baseY));
  pos3DZ = 90.0 + (curvatureDiff * 0.5); // 0-180° Z range
  pos3DZ = constrain(pos3DZ, 0, 180);
  
  // Update spatial memory
  int gx = getGrid3DX(pos3DX);
  int gy = getGrid3DY(pos3DY);
  int gz = getGrid3DZ(pos3DZ);
  
  if(gx >= 0 && gx < GRID_SIZE && gy >= 0 && gy < GRID_SIZE && gz >= 0 && gz < GRID_SIZE) {
    visitGrid[gx][gy][gz] = min(255, visitGrid[gx][gy][gz] + 1);
  }
}

// ===== EXTERNAL STIMULUS WELL CREATION =====
void createExternalWell(float x, float y, float z) {
  if(activeWells < MAX_WELLS) {
    wells[activeWells].x = x;
    wells[activeWells].y = y;
    wells[activeWells].z = z;
    wells[activeWells].strength = 0.5;      // Stronger than metabolic wells
    wells[activeWells].coronaRadius = 40.0; // Wider influence
    wells[activeWells].tailRadius = 15.0;   // Larger capture zone
    wells[activeWells].formed = millis();
    wells[activeWells].active = true;
    activeWells++;
    
    Serial.println("External stimulus well created");
  }
}

// ===== GRAVITATIONAL FORCE CALCULATION =====
void calculateGravitationalForces(float& forceX, float& forceY, float& forceZ) {
  forceX = forceY = forceZ = 0.0;
  
  // Central gravity well (psychological darkness)
  float centerDist = sqrt(pow(pos3DX - 90, 2) + pow(pos3DY - 90, 2) + pow(pos3DZ - 90, 2));
  if(centerDist > 0.1) {
    float centerForce = 0.5 / (centerDist * centerDist);
    forceX += (90.0 - pos3DX) * centerForce;
    forceY += (90.0 - pos3DY) * centerForce;
    forceZ += (90.0 - pos3DZ) * centerForce;
  }
  
  // Metabolic depletion wells
  for(int i = 0; i < MAX_WELLS; i++) {
    if(!wells[i].active) continue;
    
    float wellDist = sqrt(pow(pos3DX - wells[i].x, 2) + 
                         pow(pos3DY - wells[i].y, 2) + 
                         pow(pos3DZ - wells[i].z, 2));
    
    if(wellDist < wells[i].coronaRadius) {
      float wellForce = wells[i].strength;
      if(wellDist < wells[i].tailRadius) {
        wellForce *= 3.0; // Stronger pull in tail zone
      }
      
      if(wellDist > 0.1) {
        forceX += (wells[i].x - pos3DX) * wellForce / wellDist;
        forceY += (wells[i].y - pos3DY) * wellForce / wellDist;
        forceZ += (wells[i].z - pos3DZ) * wellForce / wellDist;
      }
    }
  }
}

// ===== DEPLETION ZONE MANAGEMENT =====
void updateDepletionZones() {
  // Natural decay of all depletion zones
  for(int i = 0; i < GRID_SIZE; i++) {
    for(int j = 0; j < GRID_SIZE; j++) {
      for(int k = 0; k < GRID_SIZE; k++) {
        if(depletionGrid[i][j][k] > 0) {
          depletionGrid[i][j][k] = max(0, depletionGrid[i][j][k] - DEPLETION_DECAY_RATE);
        }
      }
    }
  }
  
  // Add depletion during intensive exploration
  if(currentState == SEARCH && memoryAccumulator > 1.0) {
    int gx = getGrid3DX(pos3DX);
    int gy = getGrid3DY(pos3DY);
    int gz = getGrid3DZ(pos3DZ);
    
    if(gx >= 0 && gx < GRID_SIZE && gy >= 0 && gy < GRID_SIZE && gz >= 0 && gz < GRID_SIZE) {
      depletionGrid[gx][gy][gz] = min(255, depletionGrid[gx][gy][gz] + DEPLETION_FORMATION_RATE);
    }
  }
}

// ===== WELL FORMATION FROM DEPLETION =====
void checkWellFormation() {
  for(int i = 0; i < GRID_SIZE; i++) {
    for(int j = 0; j < GRID_SIZE; j++) {
      for(int k = 0; k < GRID_SIZE; k++) {
        if(depletionGrid[i][j][k] > WELL_FORMATION_THRESHOLD) {
          // Create new gravitational well
          if(activeWells < MAX_WELLS) {
            wells[activeWells].x = (i + 0.5) * CELL_SIZE;
            wells[activeWells].y = (j + 0.5) * CELL_SIZE;
            wells[activeWells].z = (k + 0.5) * CELL_SIZE;
            wells[activeWells].strength = 0.3;
            wells[activeWells].coronaRadius = 30.0;
            wells[activeWells].tailRadius = 10.0;
            wells[activeWells].formed = millis();
            wells[activeWells].active = true;
            activeWells++;
            
            // Clear depletion zone
            depletionGrid[i][j][k] = 0;
          }
        }
      }
    }
  }
}

// ===== WELL SCATTERING MECHANICS =====
void scatterWellsNearPosition(float x, float y, float z) {
  for(int i = 0; i < MAX_WELLS; i++) {
    if(!wells[i].active) continue;
    
    float dist = sqrt(pow(x - wells[i].x, 2) + 
                     pow(y - wells[i].y, 2) + 
                     pow(z - wells[i].z, 2));
    
    if(dist < wells[i].tailRadius) {
      // Scatter well - reduce strength
      wells[i].strength *= DEPLETION_SCATTER_RATE;
      if(wells[i].strength < 0.05) {
        wells[i].active = false;
      }
      
      // Apply energy cost for traversing well
      energy -= WELL_TRAVERSE_COST;
    }
  }
}

// ===== TARGET SELECTION FOR SEEKING =====
void findLeastDepletedTarget() {
  float bestScore = 1000.0;
  int bestGX = 6, bestGY = 6, bestGZ = 6; // Default to center

  // Find least depleted perimeter region
  for(int i = 0; i < GRID_SIZE; i++) {
    for(int j = 0; j < GRID_SIZE; j++) {
      for(int k = 0; k < GRID_SIZE; k++) {
        // Focus on perimeter regions
        if(i > 2 && i < GRID_SIZE-3 && j > 2 && j < GRID_SIZE-3 && k > 2 && k < GRID_SIZE-3) continue;
        
        float score = visitGrid[i][j][k] + (depletionGrid[i][j][k] * 0.1);
        if(score < bestScore) {
          bestScore = score;
          bestGX = i; bestGY = j; bestGZ = k;
        }
      }
    }
  }
  
  // Convert to servo targets
  seekTargetBaseX = (bestGX + 0.5) * CELL_SIZE;
  seekTargetBaseY = (bestGY + 0.5) * CELL_SIZE;
  seekTargetTipX = seekTargetBaseX + random(-15, 16); // Tip variation
  seekTargetTipY = seekTargetBaseY + random(-15, 16);
  
  // Constrain to servo limits
  seekTargetBaseX = constrain(seekTargetBaseX, servoMin, servoMax);
  seekTargetBaseY = constrain(seekTargetBaseY, servoMin, servoMax);
  seekTargetTipX = constrain(seekTargetTipX, servoMin, servoMax);
  seekTargetTipY = constrain(seekTargetTipY, servoMin, servoMax);
}

// ===== SERVO TARGET SETTING =====
void setServoTargets(float targetBaseX, float targetBaseY, float targetTipX, float targetTipY) {
  target_baseX = constrain(targetBaseX, servoMin, servoMax);
  target_baseY = constrain(targetBaseY, servoMin, servoMax);
  target_tipX = constrain(targetTipX, servoMin, servoMax);
  target_tipY = constrain(targetTipY, servoMin, servoMax);
  
  target_updated = true;
}

// ===== STATE IMPLEMENTATIONS =====
void updateRest() {
  energy += REST_ENERGY_RATE;
  
  float restPhase = millis() * REST_PHASE_SPEED * 0.001;
  float shiverBaseX = restPosition + cos(restPhase) * 3.0;
  float shiverBaseY = restPosition + sin(restPhase * 1.3) * 3.0;
  float shiverTipX = restPosition + cos(restPhase * 0.7) * 2.0;
  float shiverTipY = restPosition + sin(restPhase * 0.9) * 2.0;
  
  setServoTargets(shiverBaseX, shiverBaseY, shiverTipX, shiverTipY);
  
  if(energy >= maxEnergy) {
    findLeastDepletedTarget();
    seekProgress = 0.0;
    currentState = SEEK;
    Serial.println("REST -> SEEK (3D exploration)");
  }
}

void updateSeek() {
  seekProgress += SEEK_PROGRESS_RATE;
  energy -= SEEK_ENERGY_RATE;
  
  // 3D path to target with gravitational avoidance
  float gravX, gravY, gravZ;
  calculateGravitationalForces(gravX, gravY, gravZ);
  
  float targetBaseX = restPosition + (seekTargetBaseX - restPosition) * seekProgress - gravX * 5.0;
  float targetBaseY = restPosition + (seekTargetBaseY - restPosition) * seekProgress - gravY * 5.0;
  float targetTipX = restPosition + (seekTargetTipX - restPosition) * seekProgress - gravX * 3.0;
  float targetTipY = restPosition + (seekTargetTipY - restPosition) * seekProgress - gravY * 3.0;
  
  setServoTargets(targetBaseX, targetBaseY, targetTipX, targetTipY);
  
  if(seekProgress >= 1.0) {
    searchCenterX = pos3DX;
    searchCenterY = pos3DY;
    searchCenterZ = pos3DZ;
    searchPhase = 0.0;
    memoryAccumulator = 0.0;
    currentState = SEARCH;
    Serial.println("SEEK -> SEARCH (3D territorial exploration)");
  }
}

void updateSearch() {
  energy -= SEARCH_ENERGY_RATE;
  searchPhase += SEARCH_BASE_SPEED;
  memoryAccumulator += 0.1;
  
  // 3D exploration patterns
  float exploreRadius = 15.0 + sin(searchPhase) * 8.0;
  float baseOffsetX = cos(searchPhase) * exploreRadius;
  float baseOffsetY = sin(searchPhase * 0.8) * exploreRadius;
  float tipOffsetX = cos(searchPhase * 1.3) * exploreRadius * 0.6;
  float tipOffsetY = sin(searchPhase * 1.1) * exploreRadius * 0.6;
  
  // Apply gravitational influences
  float gravX, gravY, gravZ;
  calculateGravitationalForces(gravX, gravY, gravZ);
  
  float targetBaseX = searchCenterX + baseOffsetX - gravX * 2.0;
  float targetBaseY = searchCenterY + baseOffsetY - gravY * 2.0;
  float targetTipX = searchCenterX + tipOffsetX - gravX * 1.0;
  float targetTipY = searchCenterY + tipOffsetY - gravY * 1.0;
  
  setServoTargets(targetBaseX, targetBaseY, targetTipX, targetTipY);
  
  // Check for well scattering
  scatterWellsNearPosition(pos3DX, pos3DY, pos3DZ);
  
  if(energy <= 35.0) {
    currentState = RETURN;
    Serial.println("SEARCH -> RETURN (3D gravitational struggle)");
  }
}

void updateReturn() {
  energy -= RETURN_ENERGY_RATE;
  
  // Strong gravitational forces pulling toward center
  float gravX, gravY, gravZ;
  calculateGravitationalForces(gravX, gravY, gravZ);
  
  // Compound curvature resistance - tip tries to escape while base pulled toward center
  float resistancePhase = millis() * RETURN_RESISTANCE_FREQ;
  float tipResistX = cos(resistancePhase) * 15.0 * (energy / 35.0);
  float tipResistY = sin(resistancePhase * 1.2) * 15.0 * (energy / 35.0);
  
  float targetBaseX = current_baseX + gravX * 2.0;
  float targetBaseY = current_baseY + gravY * 2.0;
  float targetTipX = current_tipX + tipResistX - gravX * 0.5;
  float targetTipY = current_tipY + tipResistY - gravY * 0.5;
  
  setServoTargets(targetBaseX, targetBaseY, targetTipX, targetTipY);
  
  float distFromCenter = sqrt(pow(pos3DX - 90, 2) + pow(pos3DY - 90, 2) + pow(pos3DZ - 90, 2));
  if(distFromCenter < 8.0 || energy <= 0.0) {
    currentState = REST;
    energy = 0.0;
    Serial.println("RETURN -> REST (consumed by 3D darkness)");
  }
}

// ===== COORDINATE CONVERSION =====
int getGrid3DX(float angle) { return constrain((int)(angle / CELL_SIZE), 0, GRID_SIZE-1); }
int getGrid3DY(float angle) { return constrain((int)(angle / CELL_SIZE), 0, GRID_SIZE-1); }
int getGrid3DZ(float angle) { return constrain((int)(angle / CELL_SIZE), 0, GRID_SIZE-1); }

// ===== DEBUG OUTPUT =====
void printDebug() {
  static unsigned long lastPrint = 0;
  if(millis() - lastPrint > 1000) {
    lastPrint = millis();
    
    Serial.print("State: ");
    switch(currentState) {
      case REST: Serial.print("REST"); break;
      case SEEK: Serial.print("SEEK"); break;
      case SEARCH: Serial.print("SEARCH"); break;
      case RETURN: Serial.print("RETURN"); break;
    }
    
    Serial.print(" | Energy: "); Serial.print(energy, 1);
    Serial.print(" | Base:("); Serial.print((int)current_baseX); Serial.print(","); Serial.print((int)current_baseY); Serial.print(")");
    Serial.print(" | Tip:("); Serial.print((int)current_tipX); Serial.print(","); Serial.print((int)current_tipY); Serial.print(")");
    Serial.print(" | 3D:("); Serial.print((int)pos3DX); Serial.print(","); Serial.print((int)pos3DY); Serial.print(","); Serial.print((int)pos3DZ); Serial.print(")");
    Serial.print(" | Wells: "); Serial.print(activeWells);
    Serial.print(" | Light: "); Serial.print(light_level, 2);
    
    if(external_stimulus) {
      Serial.print(" | STIMULUS");
    }
    
    if(currentState == SEARCH) {
      Serial.print(" | Memory: "); Serial.print(memoryAccumulator, 1);
    }
    
    Serial.println();
  }
}