#include <Servo.h>

// === Pin Assignments ===
const int servoXPin = 3;       // 12-6 o'clock line
const int servoYPin = 5;       // 3-9 o'clock line  
const int joystickXPin = A0;
const int joystickYPin = A1;
const int joystickBtnPin = 2;

// === Servos ===
Servo servoX;
Servo servoY;

// === Motion Parameters ===
float movementSpeed = 0.006;
float t = 0;
float baseAngle = 90;  // X servo
float axialAngle = 90; // Y servo

// Center point (upright - to avoid)
const float centerX = 90;
const float centerY = 90;

// === Joystick ===
bool useJoystick = false;
unsigned long lastBtnPress = 0;
const unsigned long btnDebounceMs = 300;
int lastBtnState = HIGH;

// === Time Tracking ===
unsigned long lastUpdate = 0;

// === Setup ===
void setup() {
  servoX.attach(servoXPin);
  servoY.attach(servoYPin);
  pinMode(joystickBtnPin, INPUT_PULLUP);
  Serial.begin(9600);
  
  // Start bent over
  servoX.write(90);
  servoY.write(120);
}

// === Main Loop ===
void loop() {
  unsigned long now = millis();
  handleJoystick(now);
  handleSerial();

  float dt = (now - lastUpdate) / 1000.0;
  if (dt < 0.01) return;
  lastUpdate = now;

  if (useJoystick) {
    handleJoystickControl();
  } else {
    updateClockSearch(dt);
  }

  servoX.write(constrain(baseAngle, 0, 180));
  servoY.write(constrain(axialAngle, 0, 180));
}

// === Clock Sweep Search ===
void updateClockSearch(float dt) {
  t += dt;

  // Slow clock sweep with wandering
  static float clockAngle = 0;
  static float sweepSpeed = 0.4;
  static bool clockwise = true;
  static unsigned long lastDirectionChange = 0;
  
  // Change direction occasionally
  if (millis() - lastDirectionChange > random(8000, 15000)) {
    clockwise = !clockwise;
    lastDirectionChange = millis();
  }
  
  if (clockwise) {
    clockAngle += sweepSpeed * dt;
  } else {
    clockAngle -= sweepSpeed * dt;
  }
  
  // Keep angle in bounds
  if (clockAngle > 6.28) clockAngle -= 6.28;
  if (clockAngle < 0) clockAngle += 6.28;
  
  // Use actual measured clock positions instead of mathematical circle
  // Convert clock angle (0-2π) to actual servo positions
  float normalizedAngle = fmod(clockAngle, 6.28) / 6.28;  // 0-1
  
  float targetX, targetY;
  
  if (normalizedAngle < 0.083) {  // 12-1 o'clock
    float t = normalizedAngle * 12;
    targetX = 90 + t * 40;  // 90 to 130
    targetY = 0;
  } else if (normalizedAngle < 0.25) {  // 1-3 o'clock  
    float t = (normalizedAngle - 0.083) * 6;
    targetX = 130 + t * 50;  // 130 to 180
    targetY = 0 + t * 90;    // 0 to 90
  } else if (normalizedAngle < 0.5) {  // 3-6 o'clock
    float t = (normalizedAngle - 0.25) * 4;
    targetX = 180 - t * 90;  // 180 to 90
    targetY = 90 + t * 90;   // 90 to 180
  } else if (normalizedAngle < 0.75) {  // 6-9 o'clock
    float t = (normalizedAngle - 0.5) * 4;
    targetX = 90 - t * 90;   // 90 to 0
    targetY = 180 - t * 35;  // 180 to 145 (7-8 position)
  } else if (normalizedAngle < 0.917) {  // 9-11 o'clock
    float t = (normalizedAngle - 0.75) * 6;
    targetX = 0;             // Stay at 0
    targetY = 145 - t * 55;  // 145 to 90
  } else {  // 11-12 o'clock
    float t = (normalizedAngle - 0.917) * 12;
    targetX = 0 + t * 90;    // 0 to 90
    targetY = 2 - t * 2;     // 2 to 0
  }
  
  // Add small searching circles
  static float searchPhase = 0;
  searchPhase += dt * 3.0;
  float searchSize = 3;
  
  targetX += cos(searchPhase * 1.3) * searchSize;
  targetY += sin(searchPhase) * searchSize;
  
  // Constrain
  targetX = constrain(targetX, 0, 180);
  targetY = constrain(targetY, 0, 180);
  
  // Smooth movement
  baseAngle += (targetX - baseAngle) * 0.02;
  axialAngle += (targetY - axialAngle) * 0.02;
}

// === Joystick Toggle ===
void handleJoystick(unsigned long now) {
  int btnState = digitalRead(joystickBtnPin);
  if (btnState == LOW && lastBtnState == HIGH && now - lastBtnPress > btnDebounceMs) {
    useJoystick = !useJoystick;
    lastBtnPress = now;
  }
  lastBtnState = btnState;
}

// === Manual Joystick Control ===
void handleJoystickControl() {
  int xRaw = analogRead(joystickXPin);
  int yRaw = analogRead(joystickYPin);

  baseAngle = map(xRaw, 0, 1023, 0, 180);
  axialAngle = map(yRaw, 0, 1023, 0, 180);
}

// === Serial Override ===
void handleSerial() {
  if (Serial.available()) {
    char axis = Serial.read();
    int value = Serial.parseInt();

    if (axis == 'X' || axis == 'x') {
      baseAngle = constrain(value, 0, 180);
    } else if (axis == 'Y' || axis == 'y') {
      axialAngle = constrain(value, 0, 180);
    }
  }
}