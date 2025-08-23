#include <Servo.h>

// === Pin Assignments ===
const int servoXPin = 3;       // Base rotation
const int servoYPin = 6;       // Axial tip
const int joystickXPin = A0;
const int joystickYPin = A1;
const int joystickBtnPin = 2;

// === Servos ===
Servo servoX;
Servo servoY;

// === State ===
bool paused = false;
unsigned long lastBtnPress = 0;
const unsigned long btnDebounceMs = 300;
int lastBtnState = HIGH;

void setup() {
  servoX.attach(servoXPin);
  servoY.attach(servoYPin);
  pinMode(joystickBtnPin, INPUT_PULLUP);
  Serial.begin(9600);
  
  Serial.println("Tentacle Joystick Test");
  Serial.println("Move joystick to position tentacle");
  Serial.println("Click button to pause/unpause and record position");
  
  // Start at center
  servoX.write(90);
  servoY.write(90);
}

void loop() {
  // Handle button press
  int btnState = digitalRead(joystickBtnPin);
  unsigned long now = millis();
  
  if (btnState == LOW && lastBtnState == HIGH && now - lastBtnPress > btnDebounceMs) {
    paused = !paused;
    lastBtnPress = now;
    
    if (paused) {
      Serial.println("=== PAUSED ===");
    } else {
      Serial.println("=== RESUMED ===");
    }
  }
  lastBtnState = btnState;
  
  if (!paused) {
    // Read joystick and control servos
    int xRaw = analogRead(joystickXPin);
    int yRaw = analogRead(joystickYPin);
    
    int xAngle = map(xRaw, 0, 1023, 0, 180);
    int yAngle = map(yRaw, 0, 1023, 0, 180);
    
    servoX.write(xAngle);
    servoY.write(yAngle);
    
    // Output current position
    static unsigned long lastOutput = 0;
    if (now - lastOutput > 200) {  // Output every 200ms
      Serial.print("X");
      Serial.print(xAngle);
      Serial.print(" Y");
      Serial.println(yAngle);
      lastOutput = now;
    }
  }
  
  delay(10);
}