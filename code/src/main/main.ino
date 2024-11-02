#include <Servo.h>
#include <LiquidCrystal_I2C.h>
#include <SPI.h>
#include <MFRC522.h>
#include <Adafruit_Fingerprint.h>
#include <Keypad.h>
#include <EEPROM.h>
#include <SoftwareSerial.h>

// Pin Configuration (Based on your mapping)
#define RFID_SS_PIN 53   // RFID SS pin
#define RFID_RST_PIN 5   // RFID reset pin
#define BUZZER_PIN 8     // Buzzer pin
#define TRIG_PIN 7       // Ultrasonic sensor Trig pin
#define ECHO_PIN 6       // Ultrasonic sensor Echo pin
#define MAX_DISTANCE 10  // Maximum distance in cm to trigger fingerprint sensor
#define BLUETOOTH_RX 15  // Bluetooth RX pin on Arduino Mega (RX3)
#define BLUETOOTH_TX 14  // Bluetooth TX pin on Arduino Mega (TX3)

#if (defined(__AVR__) || defined(ESP8266)) && !defined(__AVR_ATmega2560__)
#include <SoftwareSerial.h>
SoftwareSerial mySerial(18, 19);  // RX, TX pins for fingerprint sensor on Mega 2560 (18,19)
#else
#define mySerial Serial1
#endif

// Initialize components
Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);
String UID = "D9F5F15A";  // Expected RFID UID

Servo servo;
LiquidCrystal_I2C lcd(0x27, 16, 2);       // Initialize LCD
MFRC522 rfid(RFID_SS_PIN, RFID_RST_PIN);  // Initialize RFID reader

void setupBluetooth() {
  Serial3.begin(9600);  // Initialize Serial3 for Bluetooth communication
  Serial.println("Bluetooth module ready.");
}

int servoPosition = 70;           // Initial servo position for "closed" (70 for closed, 160 for open)
bool doorOpen = false;            // Door state flag
unsigned long lastReadTime = 0;   // Timestamp for last card/fingerprint read
String defaultPassword = "9999";  // Default password, loaded from EEPROM later
const int PASSWORD_ADDRESS = 0;

const byte ROWS = 4;
const byte COLS = 4;
char keys[ROWS][COLS] = {
  { '1', '2', '3', 'A' },
  { '4', '5', '6', 'B' },
  { '7', '8', '9', 'C' },
  { '*', '0', '#', 'D' }
};
byte rowPins[ROWS] = { 30, 31, 32, 33 };
byte colPins[COLS] = { 34, 35, 36, 37 };
Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

void setup() {
  Serial.begin(9600);
  lcd.init();
  lcd.backlight();
  servo.attach(9);
  servo.write(servoPosition);
  SPI.begin();
  rfid.PCD_Init();
  finger.begin(57600);

  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  setupBluetooth();

  defaultPassword = loadPasswordFromEEPROM();
  if (defaultPassword == "") {
    defaultPassword = "9999";
  }
  Serial.print("Current password: " + defaultPassword + "\n");
}

void loop() {
  lcd.setCursor(0, 0);
  lcd.print("Welcome, Group 2");
  delay(1000);
  lcd.clear();
  standbyMode(); 
}

void standbyMode() {
  lcd.setCursor(0, 0);
  lcd.print("Scan/Enter/Phone");
  
  while (true) {
    if (rfid.PICC_IsNewCardPresent() && rfid.PICC_ReadCardSerial()) {
      processRFID();
      break;
    }
    if (getDistance() < MAX_DISTANCE) {
      processFingerprint();
      break;
    }
    if (keypad.getKey()) {
      processKeypad();
      break;
    }
    String command = getCommandFromBluetooth();
    if (command != "") {
      processBluetoothCommand(command);
      break;
    }
  }
}

void processRFID() {
  lcd.clear();
  lcd.print("Scanning RFID...");
  String ID = readRFID();
  
  Serial.print("Read RFID ID: ");
  Serial.println(ID);
  Serial.print("Expected UID: ");
  Serial.println(UID);

  if (ID == UID) {
    openDoor();
  } else {
    lcd.clear();
    lcd.print("Wrong RFID");
    delay(1500);
  }
}

String readRFID() {
  String ID = "";
  for (byte i = 0; i < rfid.uid.size; i++) {
    if (rfid.uid.uidByte[i] < 0x10) {
      ID += "0";  // Add leading zero for single-digit hex values
    }
    ID += String(rfid.uid.uidByte[i], HEX);
  }
  ID.toUpperCase();  // Ensure all characters are uppercase
  rfid.PICC_HaltA();  // Stop RFID reading
  
  Serial.print("Formatted RFID ID: ");
  Serial.println(ID);  // For debugging
  return ID;
}



void processFingerprint() {
  lcd.clear();
  lcd.print("Place Finger...");
  delay(1000);
  if (finger.getImage() == FINGERPRINT_OK) {
    int fingerprintID;
    if (getFingerprintID(&fingerprintID) == FINGERPRINT_OK) {
      openDoor();
    } else {
      lcd.clear();
      lcd.print("Finger Not Found");
      delay(1500);
    }
  } else {
    lcd.clear();
    lcd.print("No Finger Detected");
    delay(1500);
  }
}

uint8_t getFingerprintID(int *result) {
  uint8_t p = finger.getImage();
  if (p != FINGERPRINT_OK) return p;

  p = finger.image2Tz();
  if (p != FINGERPRINT_OK) return p;

  p = finger.fingerSearch();
  if (p == FINGERPRINT_OK) {
    *result = finger.fingerID;
    return FINGERPRINT_OK;
  }
  return p;
}

void processKeypad() {
  lcd.clear();
  lcd.print("Enter Password:");
  if (checkPassword()) {
    openDoor();
  } else {
    lcd.clear();
    lcd.print("Incorrect Password");
    delay(1500);
  }
}

void processBluetoothCommand(String command) {
  Serial.println("Bluetooth command received: " + command);

  int firstSeparatorIndex = command.indexOf('-');
  int secondSeparatorIndex = command.indexOf('-', firstSeparatorIndex + 1);

  if (firstSeparatorIndex != -1 && secondSeparatorIndex == -1) {
    String receivedPassword = command.substring(0, firstSeparatorIndex);
    String action = command.substring(firstSeparatorIndex + 1);
    
    if (action == "OPEN" && receivedPassword == defaultPassword) {
      openDoor();
    } else {
      lcd.clear();
      lcd.print("Wrong Password");
      delay(1500);
    }
  } else if (firstSeparatorIndex != -1 && secondSeparatorIndex != -1) {
    String oldPassword = command.substring(0, firstSeparatorIndex);
    String action = command.substring(firstSeparatorIndex + 1, secondSeparatorIndex);
    String newPassword = command.substring(secondSeparatorIndex + 1);

    if (action == "CHANGE" && oldPassword == defaultPassword) {
      if (newPassword.length() == 4) {
        defaultPassword = newPassword;
        savePasswordToEEPROM(defaultPassword);
        lcd.clear();
        lcd.print("Password Updated");
        delay(1500);
      } else {
        lcd.clear();
        lcd.print("Invalid New Pass");
        delay(1500);
      }
    } else {
      lcd.clear();
      lcd.print("Wrong Old Password");
      delay(1500);
    }
  } else {
    lcd.clear();
    lcd.print("Invalid Command");
    delay(1500);
  }
}

float getDistance() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  long duration = pulseIn(ECHO_PIN, HIGH);
  float distance = duration * 0.034 / 2;
  return distance;
}

void openDoor() {
  if (!doorOpen) {        
    servoPosition = 160;  
    servo.write(servoPosition);
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Door is open");
    Serial.println("Door is open");
    doorOpen = true;  

    tone(BUZZER_PIN, 1000, 500);  

    for (int countdown = 5; countdown > 0; countdown--) {
      lcd.setCursor(0, 1);
      lcd.print("Closing in: ");
      lcd.print(countdown);
      lcd.print(" s ");
      delay(1000);
    }

    closeDoor();  
  }
}

void closeDoor() {
  if (doorOpen) {        
    servoPosition = 70;  
    servo.write(servoPosition);
    lcd.clear();
    lcd.setCursor(0, 0);
    tone(BUZZER_PIN, 600, 700);  
    lcd.print("Door is locked");
    Serial.println("Door is locked");
    delay(1500);
    lcd.clear();
    doorOpen = false;  
  }
}

String getInputFromUser(String message, unsigned long timeout, int maxKey) {
  String enteredInput = "";  
  char key;
  unsigned long lastKeyPressTime = millis();  

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(message);  

  while (true) {
    key = keypad.getKey();  
    if (key) {
      lastKeyPressTime = millis();  

      if (key == '#') {  
        break;
      } else if (key == '*') {  
        enteredInput = "";
        lcd.setCursor(0, 1);
        lcd.print("                ");              
      } else if (enteredInput.length() < maxKey) {  
        enteredInput += key;
        lcd.setCursor(enteredInput.length() - 1, 1);  
        lcd.print('*');                               
      }
    }

    if (enteredInput.length() >= maxKey) {
      break;
    }

    if (millis() - lastKeyPressTime > timeout) {
      Serial.println("Input timed out.");
      break;  
    }
  }

  return enteredInput;  
}

bool checkPassword() {
  String enteredPassword = getInputFromUser("Enter Password:", 5000, 4);  
  defaultPassword = loadPasswordFromEEPROM();
  if (enteredPassword == defaultPassword) {
    Serial.println("Password correct");
    return true;
  } else {
    Serial.println("Password incorrect");
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Incorrect Password");
    delay(1500);  
    return false;
  }
}

void savePasswordToEEPROM(String password) {
  for (int i = 0; i < 4; i++) {
    EEPROM.write(PASSWORD_ADDRESS + i, password[i]);
  }
  EEPROM.write(PASSWORD_ADDRESS + 4, '\0');  
}

void changePasswordByKeypad() {
  String newPassword = "";
  char key;
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("New Password:");

  while (newPassword.length() < 4) {
    key = keypad.getKey();
    if (key) {
      if (key == '*') {  
        newPassword = "";
        lcd.setCursor(0, 1);
        lcd.print("                ");        
      } else if (key >= '0' && key <= '9') {  
        newPassword += key;
        lcd.setCursor(newPassword.length() - 1, 1);
        lcd.print('*');  
      }
    }
  }

  savePasswordToEEPROM(newPassword);
  Serial.println("New password saved: " + newPassword);

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Password Changed!");
  delay(1500);  
}

String loadPasswordFromEEPROM() {
  String loadedPassword = "";
  char ch;
  for (int i = 0; i < 4; i++) {
    ch = EEPROM.read(PASSWORD_ADDRESS + i);
    loadedPassword += ch;
  }
  return loadedPassword;
}

String getCommandFromBluetooth() {
  String command = "";

  while (Serial3.available() > 0) {
    char receivedChar = Serial3.read();
    if (receivedChar == '\n') {
      break;
    } else {
      command += receivedChar;
    }
    delay(5);  
  }

  return command;
}