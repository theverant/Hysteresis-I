// H1_Tuning_Mode.ino
// Simplified version for tuning with OpenFrameworks interface
// Switch between TUNING mode (manual control) and AUTO mode (V31 algorithm)

#include <Servo.h>

// ===== HARDWARE SETUP =====
Servo servoBaseX;
Servo servoBaseY;  
Servo servoTipX;
Servo servoTipY;

const int servoBaseXPin = 3;
const int servoBaseYPin = 5;
const int servoTipXPin = 6;
const int servoTipYPin = 9;

// ===== OPERATION MODES =====
enum Mode {
  TUNING,  // Manual control via serial
  AUTO     // V31 algorithm (add your existing code here)
};
Mode currentMode = TUNING;  // Start in tuning mode

// ===== SERVO POSITIONS =====
float baseX = 90.0;
float baseY = 90.0;
float tipX = 90.0;
float tipY = 90.0;

// ===== SERIAL PARSING =====
String inputString = "";
boolean stringComplete = false;

// ===== TIMING =====
unsigned long lastUpdate = 0;
const unsigned long updateInterval = 20;  // 50Hz update rate
unsigned long lastHeartbeat = 0;

void setup() {
  Serial.begin(115200);
  Serial.println("H1 TENTACLE TUNING MODE");
  Serial.println("Commands:");
  Serial.println("  bX:val,bY:val,tX:val,tY:val - Set servo positions");
  Serial.println("  MODE:TUNING - Switch to manual control");
  Serial.println("  MODE:AUTO - Switch to V31 algorithm");
  Serial.println("  STATUS - Get current positions");
  
  // Attach servos
  servoBaseX.attach(servoBaseXPin);
  servoBaseY.attach(servoBaseYPin);
  servoTipX.attach(servoTipXPin);
  servoTipY.attach(servoTipYPin);
  
  // Center all servos
  setServos(90, 90, 90, 90);
  
  inputString.reserve(200);
}

void loop() {
  // Check for serial commands
  if (stringComplete) {
    processCommand(inputString);
    inputString = "";
    stringComplete = false;
  }
  
  // Update based on current mode
  if (millis() - lastUpdate > updateInterval) {
    lastUpdate = millis();
    
    switch(currentMode) {
      case TUNING:
        // In tuning mode, servos are set directly by serial commands
        // Just update the physical servos
        updateServos();
        break;
        
      case AUTO:
        // Run your V31 algorithm here
        // For now, just a placeholder oscillation
        runAutoMode();
        break;
    }
  }
  
  // Send heartbeat/status every 1 second
  if (millis() - lastHeartbeat > 1000) {
    lastHeartbeat = millis();
    sendStatus();
  }
}

void processCommand(String cmd) {
  cmd.trim();
  
  // Parse position command: "bX:120,bY:95,tX:115,tY:102"
  if (cmd.indexOf("bX:") >= 0) {
    int bx_start = cmd.indexOf("bX:") + 3;
    int bx_end = cmd.indexOf(",bY");
    int by_start = cmd.indexOf("bY:") + 3;
    int by_end = cmd.indexOf(",tX");
    int tx_start = cmd.indexOf("tX:") + 3;
    int tx_end = cmd.indexOf(",tY");
    int ty_start = cmd.indexOf("tY:") + 3;
    
    if (bx_end > bx_start && by_end > by_start && tx_end > tx_start) {
      float newBaseX = cmd.substring(bx_start, bx_end).toFloat();
      float newBaseY = cmd.substring(by_start, by_end).toFloat();
      float newTipX = cmd.substring(tx_start, tx_end).toFloat();
      float newTipY = cmd.substring(ty_start).toFloat();
      
      // Validate ranges
      if (isValidAngle(newBaseX) && isValidAngle(newBaseY) && 
          isValidAngle(newTipX) && isValidAngle(newTipY)) {
        baseX = newBaseX;
        baseY = newBaseY;
        tipX = newTipX;
        tipY = newTipY;
        
        if (currentMode == TUNING) {
          Serial.print("OK: Set to ");
          Serial.print("bX:"); Serial.print(baseX);
          Serial.print(",bY:"); Serial.print(baseY);
          Serial.print(",tX:"); Serial.print(tipX);
          Serial.print(",tY:"); Serial.println(tipY);
        }
      } else {
        Serial.println("ERROR: Angles must be 5-175");
      }
    }
  }
  // Mode switching
  else if (cmd.indexOf("MODE:") >= 0) {
    if (cmd.indexOf("TUNING") >= 0) {
      currentMode = TUNING;
      Serial.println("MODE: TUNING - Manual control active");
    } else if (cmd.indexOf("AUTO") >= 0) {
      currentMode = AUTO;
      Serial.println("MODE: AUTO - V31 algorithm active");
    }
  }
  // Status request
  else if (cmd.indexOf("STATUS") >= 0) {
    sendStatus();
  }
  // Center command
  else if (cmd.indexOf("CENTER") >= 0) {
    setServos(90, 90, 90, 90);
    Serial.println("OK: Centered all servos");
  }
}

void setServos(float bx, float by, float tx, float ty) {
  baseX = constrain(bx, 5, 175);
  baseY = constrain(by, 5, 175);
  tipX = constrain(tx, 5, 175);
  tipY = constrain(ty, 5, 175);
  updateServos();
}

void updateServos() {
  servoBaseX.write((int)baseX);
  servoBaseY.write((int)baseY);
  servoTipX.write((int)tipX);
  servoTipY.write((int)tipY);
}

boolean isValidAngle(float angle) {
  return (angle >= 5.0 && angle <= 175.0);
}

void sendStatus() {
  Serial.print("STATUS|");
  Serial.print("Mode:"); 
  Serial.print(currentMode == TUNING ? "TUNING" : "AUTO");
  Serial.print("|bX:"); Serial.print(baseX);
  Serial.print("|bY:"); Serial.print(baseY);
  Serial.print("|tX:"); Serial.print(tipX);
  Serial.print("|tY:"); Serial.print(tipY);
  Serial.println("|");
}

void runAutoMode() {
  // Placeholder for V31 algorithm
  // Replace this with your actual Searching_Hxyxy_1_1.ino logic
  
  // Example: Simple oscillation for testing
  float time = millis() * 0.001;
  baseX = 90 + sin(time * 0.5) * 30;
  baseY = 90 + cos(time * 0.3) * 25;
  tipX = 90 + sin(time * 0.7) * 20;
  tipY = 90 + cos(time * 0.9) * 15;
  
  updateServos();
}

// Serial event handler
void serialEvent() {
  while (Serial.available()) {
    char inChar = (char)Serial.read();
    if (inChar == '\n' || inChar == '\r') {
      stringComplete = true;
    } else {
      inputString += inChar;
    }
  }
}