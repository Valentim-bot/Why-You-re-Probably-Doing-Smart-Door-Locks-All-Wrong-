/*
Wiring:
---------------------------------
RFID Reader (SPI):
SDA to Arduino D10 (or any other digital pin for SS)
SCK to Arduino D13
MOSI to Arduino D11
MISO to Arduino D12
RST to Arduino D9
GND to Arduino GND
3.3V to Arduino 3.3V
--------------------------------
*/

#include <SPI.h>                  // Include SPI library for RFID communication
#include <MFRC522.h>              // Include MFRC522 library for RFID
#include <Wire.h>                 // Include Wire library for I2C communication
#include <LiquidCrystal_I2C.h>    // Include LiquidCrystal_I2C library for LCD

// RFID pins
#define SS_PIN 10                 // Define Slave Select pin for RFID
#define RST_PIN 9                 // Define Reset pin for RFID

MFRC522 rfid(SS_PIN, RST_PIN);    // Create MFRC522 instance with defined SS and RST pins

// I2C LCD address (typically 0x27 or 0x3F)
LiquidCrystal_I2C lcd(0x27, 16, 2); // Create LCD instance with I2C address 0x27, 16 columns, and 2 rows

void setup() {
  Serial.begin(9600);             // Start serial communication at 9600 baud rate

  // Initialize RFID
  SPI.begin();                    // Initiate SPI bus
  rfid.PCD_Init();                // Initiate MFRC522 RFID module

  // Initialize LCD
  lcd.init();                     // Initialize the LCD
  lcd.backlight();                // Turn on the LCD backlight
  lcd.clear();                    // Clear the LCD screen
  lcd.setCursor(0, 0);            // Set cursor to the first row and column
  lcd.print("Getting UID Tag");   // Display initial message on LCD

  delay(2000);                    // Wait for 2 seconds
  lcd.clear();                    // Clear the LCD screen
}

void loop() {
  // Look for new cards
  if (!rfid.PICC_IsNewCardPresent() || !rfid.PICC_ReadCardSerial()) {
    return;                       // If no new card is present, exit the loop
  }

  // Display UID on Serial Monitor
  Serial.print("UID tag: ");
  String content = "";            // Create a string to hold the UID content
  for (byte i = 0; i < rfid.uid.size; i++) {
    Serial.print(rfid.uid.uidByte[i] < 0x10 ? " 0" : " ");
    Serial.print(rfid.uid.uidByte[i], HEX); // Print each byte of the UID in HEX format
    content.concat(String(rfid.uid.uidByte[i] < 0x10 ? " 0" : " "));
    content.concat(String(rfid.uid.uidByte[i], HEX)); // Append each byte to the content string
  }
  Serial.println();
  Serial.println();

  // Display UID on LCD
  lcd.clear();                    // Clear the LCD screen
  lcd.setCursor(0, 0);            // Set cursor to the first row and column
  lcd.print("RFID UID:");         // Display message on LCD
  lcd.setCursor(0, 1);            // Set cursor to the second row
  lcd.print(content);             // Display the UID content on LCD

  delay(2000);                    // Hold the display for 2 seconds

  // Halt PICC
  rfid.PICC_HaltA();              // Halt the PICC (card)
}
