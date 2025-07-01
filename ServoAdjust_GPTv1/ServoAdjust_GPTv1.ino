#include <IRremote.h>
#include <Servo.h>

// Pins
const int RECV_PIN = 11;
const int servoPins[4] = {3, 5, 6, 9};

// Servo objects
Servo servos[4];
int servoAngles[4] = {90, 90, 90, 90};

// Control state
int selectedServo = -1; // -1 means none selected

// IR
IRrecv irrecv(RECV_PIN);
decode_results results;

// Replace with *your* remote codes
#define BTN_1 0xFF30CF
#define BTN_2 0xFF18E7
#define BTN_3 0xFF7A85
#define BTN_4 0xFF10EF
#define BTN_LEFT 0xFF22DD
#define BTN_RIGHT 0xFFC23D
#define BTN_OK 0xFF02FD
#define BTN_0 0xFF6897

void centerAllServos() {
  for (int i = 0; i < 4; i++) {
    servoAngles[i] = 90;
    servos[i].write(servoAngles[i]);
  }
}

void setup() {
  Serial.begin(9600);
  irrecv.enableIRIn();

  for (int i = 0; i < 4; i++) {
    servos[i].attach(servoPins[i]);
    servos[i].write(servoAngles[i]);
  }

  Serial.println("Servo calibration controller ready.");
}

void loop() {
  if (irrecv.decode(&results)) {
    unsigned long code = results.value;
    Serial.print("Code: 0x"); Serial.println(code, HEX);

    if (code == BTN_1) {
      selectedServo = 0;
      Serial.println("Servo 1 selected");
    }
    else if (code == BTN_2) {
      selectedServo = 1;
      Serial.println("Servo 2 selected");
    }
    else if (code == BTN_3) {
      selectedServo = 2;
      Serial.println("Servo 3 selected");
    }
    else if (code == BTN_4) {
      selectedServo = 3;
      Serial.println("Servo 4 selected");
    }
    else if (code == BTN_LEFT && selectedServo != -1) {
      servoAngles[selectedServo] = max(0, servoAngles[selectedServo] - 5);
      servos[selectedServo].write(servoAngles[selectedServo]);
      Serial.print("Servo "); Serial.print(selectedServo + 1);
      Serial.print(" angle: "); Serial.println(servoAngles[selectedServo]);
    }
    else if (code == BTN_RIGHT && selectedServo != -1) {
      servoAngles[selectedServo] = min(180, servoAngles[selectedServo] + 5);
      servos[selectedServo].write(servoAngles[selectedServo]);
      Serial.print("Servo "); Serial.print(selectedServo + 1);
      Serial.print(" angle: "); Serial.println(servoAngles[selectedServo]);
    }
    else if (code == BTN_OK) {
      selectedServo = -1;
      Serial.println("Control deselected");
    }
    else if (code == BTN_0) {
      centerAllServos();
      Serial.println("All servos centered to 90°");
    }

    irrecv.resume(); // ready for next
  }
}
