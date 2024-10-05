#include <SPI.h>
#include <MFRC522.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include "SoftwareSerial.h"         // Include the SoftwareSerial library for serial communication
#include "DFRobotDFPlayerMini.h"    // Include the DFRobotDFPlayerMini library for the DFPlayer Mini module

#define RST_PIN         9           // Configurable, see typical pin layout above
#define SS_PIN          10          // Configurable, see typical pin layout above
#define Solenoid        6           // Solenoid pin

MFRC522 mfrc522(SS_PIN, RST_PIN);   // Create MFRC522 instance
LiquidCrystal_I2C lcd(0x27, 16, 2); // Change the address to match your setup

SoftwareSerial mySoftwareSerial(2, 3); // RX, TX // Create a software serial connection on pins 2 (RX) and 3 (TX)
DFRobotDFPlayerMini myDFPlayer;        // Create a DFPlayerMini object

// The specific card UID you want to check for
byte targetUID[] = {0x0B, 0x23, 0x9B, 0x15};
unsigned long displayMessageStartTime = 0;
unsigned long solenoidOpenTime = 0;
bool messageDisplayed = false;
bool solenoidOpened = false;

void setup() {
  pinMode(Solenoid, OUTPUT);
  digitalWrite(Solenoid, HIGH); // Close solenoid initially
  mySoftwareSerial.begin(9600);     // Start software serial communication at 9600 baud rate
  Serial.begin(115200);             // Initialize serial communications with the PC
  SPI.begin();                      // Init SPI bus
  mfrc522.PCD_Init();               // Init MFRC522 card
  lcd.init();                       // Initialize the LCD
  lcd.backlight();                  // Turn on the backlight
  lcd.setCursor(0, 0);
  lcd.print("Your Card please");

  if (!myDFPlayer.begin(mySoftwareSerial)) { // Initialize the DFPlayer Mini module
    Serial.println(F("Not initialized:"));
    Serial.println(F("1. Check the DFPlayer Mini connections"));
    Serial.println(F("2. Insert an SD card"));
    while (true); // If initialization fails, print error messages and halt the program
  }
  
  Serial.println();
  Serial.println(F("DFPlayer Mini module initialized!")); // Print initialization success message
  myDFPlayer.setTimeOut(500);       // Set the timeout value for serial communication
  myDFPlayer.volume(30);            // Set the volume level (0 to 30)
  myDFPlayer.EQ(0);                 // Set the equalizer setting (0: Normal, 1: Pop, 2: Rock, 3: Jazz, 4: Classic, 5: Bass)
}

void loop() {
  // Reset the loop if no new card present on the sensor/reader. This saves the entire process when idle.
  if (!mfrc522.PICC_IsNewCardPresent()) {
    checkMessageTimeout();
    checkSolenoidTimeout();
    return;
  }

  // Select one of the cards
  if (!mfrc522.PICC_ReadCardSerial()) {
    checkMessageTimeout();
    checkSolenoidTimeout();
    return;
  }

  Serial.println(F("**Card Detected:**"));

  // Check if the detected card UID matches the target UID
  if (mfrc522.uid.size == sizeof(targetUID) && memcmp(mfrc522.uid.uidByte, targetUID, sizeof(targetUID)) == 0) {
    myDFPlayer.play(1);
    digitalWrite(Solenoid, LOW);  // Open solenoid
    solenoidOpenTime = millis();  // Record the time solenoid is opened
    solenoidOpened = true;

    Serial.println("Good");
    lcd.setCursor(0, 1);
    lcd.print("Welcome Home      "); // Clear the line by adding spaces
  } else {
    myDFPlayer.play(2);
    Serial.println("Wrong");
    lcd.setCursor(0, 1);
    lcd.print("Wrong identification"); // Clear the line by adding spaces
  }

  // Record the time when the message is displayed
  displayMessageStartTime = millis();
  messageDisplayed = true;

  mfrc522.PICC_HaltA();
  mfrc522.PCD_StopCrypto1();
}

void checkMessageTimeout() {
  if (messageDisplayed && millis() - displayMessageStartTime >= 2000) {
    lcd.setCursor(0, 1);
    lcd.print("                "); // Clear the second line
    messageDisplayed = false;
  }
}

void checkSolenoidTimeout() {
  if (solenoidOpened && millis() - solenoidOpenTime >= 3000) {
    digitalWrite(Solenoid, HIGH); // Close solenoid
    solenoidOpened = false;
  }
}
