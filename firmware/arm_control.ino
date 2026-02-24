//final working code for prosthetic arm

#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>
#include <SoftwareSerial.h>

SoftwareSerial bluetooth(2, 3); // RX=2, TX=3

Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver();

// EMG Settings
#define EMG_PIN A0
const int EMG_THRESHOLD = 10;
int emgValue = 0;
bool emgMode = false; // false = Bluetooth mode, true = EMG mode

// Finger Settings
const int INDEX_CHANNEL = 0;
const int INDEX_START = 0;
const int INDEX_END = 180;
const int INDEX_SPEED = 1;

const int MIDDLE_CHANNEL = 1;
const int MIDDLE_START = 0;
const int MIDDLE_END = 120;
const int MIDDLE_SPEED = 1;

const int RING_CHANNEL = 2;
const int RING_START = 0;
const int RING_END = 180;
const int RING_SPEED = 10;

const int THUMB_CHANNEL = 3;
const int THUMB_START = 0;
const int THUMB_END = 180;
const int THUMB_SPEED = 1;

const int WRIST_CHANNEL = 4;
const int WRIST_START = 90;
const int WRIST_END = 0;
const int WRIST_SPEED = 10;

// Elbow Settings
const int ELBOW_CHANNEL = 5;
const int ELBOW_START = 0;
const int ELBOW_END = 90;
const int ELBOW_SPEED = 15;

int indexAngle = INDEX_START;
int middleAngle = MIDDLE_START;
int ringAngle = RING_START;
int thumbAngle = THUMB_START;
int wristAngle = WRIST_START;
int elbowAngle = ELBOW_START;

// EMG State
bool fistClosedEMG = false;

void setup() {
  Serial.begin(9600);
  bluetooth.begin(9600);
  pwm.begin();
  pwm.setPWMFreq(50);
  
  setAngle(INDEX_CHANNEL, INDEX_START);
  setAngle(MIDDLE_CHANNEL, MIDDLE_START);
  setAngle(RING_CHANNEL, RING_START);
  setAngle(THUMB_CHANNEL, THUMB_START);
  setAngle(WRIST_CHANNEL, WRIST_START);
  setAngle(ELBOW_CHANNEL, ELBOW_START);
  
  Serial.println("System Ready!");
  Serial.println("Default Mode: Bluetooth Control");
  Serial.println("Send '1' for EMG Mode, '0' for Bluetooth Mode");
  Serial.println("Commands: index1, index2, middle1, middle2, ring1, ring2, thumb1, thumb2, wrist1, wrist2, elbow1, elbow2, penhold1, penhold2, hold1, hold2");
  
  bluetooth.println("System Ready!");
  bluetooth.println("Send '1' for EMG Mode, '0' for Bluetooth Mode");
}

void loop() {
  // Read EMG value
  emgValue = analogRead(EMG_PIN);
  
  // Print EMG value every 500ms
  static unsigned long lastEMGPrint = 0;
  if (millis() - lastEMGPrint >= 500) {
    Serial.print("EMG Value: ");
    Serial.println(emgValue);
    lastEMGPrint = millis();
  }
  
  // EMG Control Mode
  if (emgMode) {
    if (emgValue > EMG_THRESHOLD && !fistClosedEMG) {
      // EMG detected - close fist
      closeAllFingersFast();
      fistClosedEMG = true;
      Serial.println("EMG: Muscle detected! Closing fist.");
    }
    else if (emgValue <= EMG_THRESHOLD && fistClosedEMG) {
      // EMG relaxed - open fist
      openAllFingersFast();
      fistClosedEMG = false;
      Serial.println("EMG: Muscle relaxed. Opening fist.");
    }
  }
  
  // Bluetooth Control
  if (bluetooth.available() > 0) {
    String command = bluetooth.readStringUntil('\n');
    command.trim();
    processCommand(command, true);
  }
  
  // Serial Monitor Control
  if (Serial.available() > 0) {
    String command = Serial.readStringUntil('\n');
    command.trim();
    processCommand(command, false);
  }
  
  delay(50);
}

void processCommand(String command, bool fromBluetooth) {
  // Mode switching
  if (command == "1") {
    emgMode = true;
    sendResponse("Switched to EMG Muscle Control Mode", fromBluetooth);
  }
  else if (command == "0") {
    emgMode = false;
    sendResponse("Switched to Bluetooth Control Mode", fromBluetooth);
  }
  
  // Only process commands if not in EMG mode
  if (!emgMode || command == "1" || command == "0") {
    if (command == "index1") {
      closeFinger("index", INDEX_CHANNEL, INDEX_START, INDEX_END, INDEX_SPEED, indexAngle, fromBluetooth);
    }
    else if (command == "index2") {
      openFinger("index", INDEX_CHANNEL, INDEX_START, INDEX_SPEED, indexAngle, fromBluetooth);
    }
    else if (command == "middle1") {
      closeFinger("middle", MIDDLE_CHANNEL, MIDDLE_START, MIDDLE_END, MIDDLE_SPEED, middleAngle, fromBluetooth);
    }
    else if (command == "middle2") {
      openFinger("middle", MIDDLE_CHANNEL, MIDDLE_START, MIDDLE_SPEED, middleAngle, fromBluetooth);
    }
    else if (command == "ring1") {
      closeFinger("ring", RING_CHANNEL, RING_START, RING_END, RING_SPEED, ringAngle, fromBluetooth);
    }
    else if (command == "ring2") {
      openFinger("ring", RING_CHANNEL, RING_START, RING_SPEED, ringAngle, fromBluetooth);
    }
    else if (command == "thumb1") {
      closeFinger("thumb", THUMB_CHANNEL, THUMB_START, THUMB_END, THUMB_SPEED, thumbAngle, fromBluetooth);
    }
    else if (command == "thumb2") {
      openFinger("thumb", THUMB_CHANNEL, THUMB_START, THUMB_SPEED, thumbAngle, fromBluetooth);
    }
    else if (command == "wrist1") {
      moveWrist(90, 0, wristAngle, fromBluetooth);
    }
    else if (command == "wrist2") {
      moveWrist(0, 90, wristAngle, fromBluetooth);
    }
    else if (command == "elbow1") {
      moveElbow(0, 90, elbowAngle, fromBluetooth);
    }
    else if (command == "elbow2") {
      moveElbow(90, 0, elbowAngle, fromBluetooth);
    }
    else if (command == "penhold1") {
      penholdGrip(true, fromBluetooth);
    }
    else if (command == "penhold2") {
      penholdGrip(false, fromBluetooth);
    }
    else if (command == "hold1") {
      holdAllFingers(true, fromBluetooth);
    }
    else if (command == "hold2") {
      holdAllFingers(false, fromBluetooth);
    }
    else if (command != "1" && command != "0") {
      sendResponse("Unknown command: " + command, fromBluetooth);
    }
  }
}

// EMG Fast Functions
void closeAllFingersFast() {
  int maxAngle = max(max(max(INDEX_END, MIDDLE_END), RING_END), THUMB_END);
  for (int angle = 0; angle <= maxAngle; angle++) {
    if (angle <= INDEX_END) setAngle(INDEX_CHANNEL, angle);
    if (angle <= MIDDLE_END) setAngle(MIDDLE_CHANNEL, angle);
    if (angle <= RING_END) setAngle(RING_CHANNEL, angle);
    if (angle <= THUMB_END) setAngle(THUMB_CHANNEL, angle);
    delay(2);
  }
  indexAngle = INDEX_END;
  middleAngle = MIDDLE_END;
  ringAngle = RING_END;
  thumbAngle = THUMB_END;
}

void openAllFingersFast() {
  int currentMax = max(max(max(indexAngle, middleAngle), ringAngle), thumbAngle);
  for (int angle = currentMax; angle >= 0; angle--) {
    if (angle <= indexAngle) setAngle(INDEX_CHANNEL, angle);
    if (angle <= middleAngle) setAngle(MIDDLE_CHANNEL, angle);
    if (angle <= ringAngle) setAngle(RING_CHANNEL, angle);
    if (angle <= thumbAngle) setAngle(THUMB_CHANNEL, angle);
    delay(2);
  }
  indexAngle = INDEX_START;
  middleAngle = MIDDLE_START;
  ringAngle = RING_START;
  thumbAngle = THUMB_START;
}

void holdAllFingers(bool closeGrip, bool fromBluetooth) {
  if (closeGrip) {
    sendResponse("Holding ALL fingers...", fromBluetooth);
    closeAllFingersFast();
    sendResponse("All fingers closed (Fist)", fromBluetooth);
  } else {
    sendResponse("Releasing ALL fingers...", fromBluetooth);
    openAllFingersFast();
    sendResponse("All fingers opened", fromBluetooth);
  }
}

void penholdGrip(bool closeGrip, bool fromBluetooth) {
  if (closeGrip) {
    sendResponse("Activating penhold grip...", fromBluetooth);
    
    int maxAngle = max(max(THUMB_END, INDEX_END), MIDDLE_END);
    for (int angle = 0; angle <= maxAngle; angle++) {
      if (angle <= THUMB_END) setAngle(THUMB_CHANNEL, angle);
      if (angle <= INDEX_END) setAngle(INDEX_CHANNEL, angle);
      if (angle <= MIDDLE_END) setAngle(MIDDLE_CHANNEL, angle);
      delay(1);
    }
    thumbAngle = THUMB_END;
    indexAngle = INDEX_END;
    middleAngle = MIDDLE_END;
    
    sendResponse("Penhold grip activated", fromBluetooth);
  } else {
    sendResponse("Releasing penhold grip...", fromBluetooth);
    
    int currentMax = max(max(thumbAngle, indexAngle), middleAngle);
    for (int angle = currentMax; angle >= 0; angle--) {
      if (angle <= thumbAngle) setAngle(THUMB_CHANNEL, angle);
      if (angle <= indexAngle) setAngle(INDEX_CHANNEL, angle);
      if (angle <= middleAngle) setAngle(MIDDLE_CHANNEL, angle);
      delay(1);
    }
    thumbAngle = THUMB_START;
    indexAngle = INDEX_START;
    middleAngle = MIDDLE_START;
    
    sendResponse("Penhold grip released", fromBluetooth);
  }
}

void moveWrist(int fromAngle, int toAngle, int &currentAngle, bool fromBluetooth) {
  sendResponse("Moving wrist " + String(fromAngle) + " to " + String(toAngle) + "...", fromBluetooth);
  
  if (fromAngle > toAngle) {
    for (int angle = fromAngle; angle >= toAngle; angle--) {
      setAngle(WRIST_CHANNEL, angle);
      delay(WRIST_SPEED);
    }
  } else {
    for (int angle = fromAngle; angle <= toAngle; angle++) {
      setAngle(WRIST_CHANNEL, angle);
      delay(WRIST_SPEED);
    }
  }
  currentAngle = toAngle;
  
  sendResponse("Wrist at " + String(toAngle) + " degree", fromBluetooth);
}

void moveElbow(int fromAngle, int toAngle, int &currentAngle, bool fromBluetooth) {
  sendResponse("Moving elbow " + String(fromAngle) + " to " + String(toAngle) + "...", fromBluetooth);
  
  if (fromAngle > toAngle) {
    for (int angle = fromAngle; angle >= toAngle; angle--) {
      setAngle(ELBOW_CHANNEL, angle);
      delay(ELBOW_SPEED);
    }
  } else {
    for (int angle = fromAngle; angle <= toAngle; angle++) {
      setAngle(ELBOW_CHANNEL, angle);
      delay(ELBOW_SPEED);
    }
  }
  currentAngle = toAngle;
  
  sendResponse("Elbow at " + String(toAngle) + " degree", fromBluetooth);
}

void closeFinger(String name, int channel, int start, int end, int speedDelay, int &currentAngle, bool fromBluetooth) {
  sendResponse("Closing " + name + " finger...", fromBluetooth);
  
  for (int angle = start; angle <= end; angle++) {
    setAngle(channel, angle);
    delay(speedDelay);
  }
  currentAngle = end;
  
  sendResponse(name + " finger closed", fromBluetooth);
}

void openFinger(String name, int channel, int start, int speedDelay, int &currentAngle, bool fromBluetooth) {
  sendResponse("Opening " + name + " finger...", fromBluetooth);
  
  for (int angle = currentAngle; angle >= start; angle--) {
    setAngle(channel, angle);
    delay(speedDelay);
  }
  currentAngle = start;
  
  sendResponse(name + " finger opened", fromBluetooth);
}

void sendResponse(String message, bool toBluetooth) {
  Serial.println(message);
  if (toBluetooth) {
    bluetooth.println(message);
  }
}

void setAngle(int channel, int angle) {
  int pulse = map(angle, 0, 180, 150, 600);
  pwm.setPWM(channel, 0, pulse);
}
