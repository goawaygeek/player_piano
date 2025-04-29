#include <Arduino.h>
#include <Wire.h>
#include "PCA9635.h"
#include "settings.h"

// PCA9635 instance with confirmed address 0x40
PCA9635 board1(0x40);

// Function prototypes
void scanI2CBus();
void testAllOutputs();
void testSingleOutput(uint8_t outputPin);
void printStatus(String message);

void setup() {
  // Give USB time to initialize
  delay(2000);
  
  Serial.begin(115200);
  Serial.println("\n==========================================");
  Serial.println("PCA9635 Solenoid Driver Troubleshooting");
  Serial.println("==========================================");
  
  // Initialize I2C
  Wire.begin(SDA_PIN, SCL_PIN);
  
  // Run I2C scanner to confirm device presence
  scanI2CBus();
  
  // Initialize the PCA9635 with correct configuration for solenoid driving
  printStatus("Initializing PCA9635...");
  
  // Important: MODE2_INVRT bit should be OFF for driving N-channel MOSFETs (HIGH = ON)
  // MODE2_TOTEMPOLE should be ON for better gate drive
  if (board1.begin(PCA9635_MODE1_NONE, PCA9635_MODE2_TOTEMPOLE)) {
    printStatus("PCA9635 initialized successfully!");
    
    // Set all channels to PWM mode
    printStatus("Configuring all outputs as PWM mode...");
    for (int channel = 0; channel < board1.channelCount(); channel++) {
      board1.setLedDriverMode(channel, PCA9635_LEDPWM);
      board1.write1(channel, 0);  // Start with all outputs OFF
    }
    
    // Wait a moment
    delay(1000);
    
    // Start the testing sequence
    printStatus("Beginning solenoid test sequence");
    
    // First test a specific output (e.g., PWM 11 that wasn't working)
    printStatus("Testing PWM output 11 specifically");
    testSingleOutput(11);
    
    // Then test all outputs sequentially
    printStatus("Now testing all outputs sequentially");
    testAllOutputs();
    
    printStatus("Initial test sequence complete");
  } else {
    printStatus("Failed to initialize PCA9635!");
    printStatus("Running I2C scan again to verify connection...");
    scanI2CBus();
  }
}

void loop() {
  // Continuous testing of specific output for debugging
  static bool outputState = false;
  static unsigned long lastToggle = 0;
  
  // Toggle the state of output 11 every 2 seconds
  if (millis() - lastToggle >= 2000) {
    outputState = !outputState;
    Serial.print("Setting output 11 to ");
    
    if (outputState) {
      // Important: Use full 255 value for maximum gate drive
      board1.write1(11, 255);
      Serial.println("ON (255)");
    } else {
      board1.write1(11, 0);
      Serial.println("OFF (0)");
    }
    
    lastToggle = millis();
  }
  
  // Check for serial commands
  if (Serial.available() > 0) {
    String input = Serial.readStringUntil('\n');
    input.trim();
    
    // Check if input is a number
    if (input.toInt() >= 0 && input.toInt() < 16) {
      int pin = input.toInt();
      Serial.print("Testing pin ");
      Serial.println(pin);
      testSingleOutput(pin);
    } 
    else if (input == "all") {
      Serial.println("Testing all outputs sequentially");
      testAllOutputs();
    }
    else if (input == "scan") {
      Serial.println("Running I2C scan");
      scanI2CBus();
    }
    else {
      Serial.println("Commands:");
      Serial.println("0-15: Test specific output");
      Serial.println("all: Test all outputs");
      Serial.println("scan: Run I2C scan");
    }
  }
}

// Function to test a single output
void testSingleOutput(uint8_t outputPin) {
  if (outputPin >= 16) return;
  
  Serial.print("Testing output pin ");
  Serial.print(outputPin);
  Serial.println(":");
  
  // Turn off all outputs first
  for (int i = 0; i < 16; i++) {
    board1.write1(i, 0);
  }
  
  // Turn on the selected output at full power
  Serial.print("  Turning ON output ");
  Serial.println(outputPin);
  board1.write1(outputPin, 255);  // Full ON for maximum gate drive
  
  delay(1000);  // Keep on for 1 second
  
  Serial.print("  Turning OFF output ");
  Serial.println(outputPin);
  board1.write1(outputPin, 0);  // Turn off
  
  delay(500);  // Wait a moment before next test
}

// Function to test all outputs one by one
void testAllOutputs() {
  Serial.println("Sequential test of all outputs...");
  
  for (int i = 0; i < 16; i++) {
    Serial.print("Output ");
    Serial.print(i);
    Serial.println(": ON");
    
    board1.write1(i, 255);  // Turn on at full power
    delay(500);             // Keep on for 0.5 seconds
    board1.write1(i, 0);    // Turn off
    
    delay(200);  // Brief pause between outputs
  }
  
  Serial.println("All outputs tested.");
}

// Print status message with timestamp
void printStatus(String message) {
  Serial.print("[");
  Serial.print(millis() / 1000);
  Serial.print("s] ");
  Serial.println(message);
}

// I2C Scanner function
void scanI2CBus() {
  byte error, address;
  int deviceCount = 0;
  
  Serial.println("\n----- I2C Bus Scan -----");
  
  // Scan addresses from 1 to 127 (0x01 to 0x7F)
  for(address = 1; address < 128; address++) {
    Wire.beginTransmission(address);
    error = Wire.endTransmission();
    
    if(error == 0) {
      Serial.print("Device found at address 0x");
      if(address < 16) {
        Serial.print("0");
      }
      Serial.print(address, HEX);
      
      // Check if this might be a PCA9635
      if(address == 0x40) {
        Serial.print(" (PCA9635 - MATCHES expected address!)");
      } else if(address >= 0x40 && address <= 0x7F) {
        Serial.print(" (Possible PCA9635)");
      }
      Serial.println();
      deviceCount++;
    }
  }
  
  if(deviceCount == 0) {
    Serial.println("No I2C devices found! Check connections.");
  }
  else {
    Serial.print("Found ");
    Serial.print(deviceCount);
    Serial.println(" device(s)");
  }
  
  Serial.println("-------------------------\n");
}