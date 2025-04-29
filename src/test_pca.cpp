#include <Arduino.h>
#include <Wire.h>
#include "PCA9635.h"
#include "settings.h"

PCA9635 board1(0x40);  // Default address

// Function prototype for I2C scanner
void scanI2CBus();
void analyzePCA9635Address(byte foundAddres);

void setup() {
  // Give USB more time to initialize
  delay(2000);  
  
  Serial.begin(115200);
  while (!Serial) {
    ; // wait for serial port to connect
  }
  
  Serial.println("\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n");  // Clear any old output
  Serial.println("==========================================");
  Serial.println("PCA9635 Test Program - STARTING");
  Serial.println("==========================================");
  
  // Initialize I2C
  Wire.begin(SDA_PIN, SCL_PIN);


  // Run I2C scanner first to see what's on the bus
  scanI2CBus();

  Serial.println("I2C initialized");
  
  // Try to initialize the PCA9635
  if (board1.begin(PCA9635_MODE1_NONE, PCA9635_MODE2_INVERT | PCA9635_MODE2_TOTEMPOLE)) {
    Serial.println("PCA9635 initialized successfully!");
    
    // Set all channels to PWM mode
    for (int channel = 0; channel < board1.channelCount(); channel++) {
      board1.setLedDriverMode(channel, PCA9635_LEDPWM);
      board1.write1(channel, 0);  // Turn all channels off initially
    }
    
    // Test output 11 (D13/Q13)
    Serial.println("Testing output 11 (D13/Q13)");
    board1.write1(11, 255);  // Turn on at full power
    delay(1000);
    board1.write1(11, 0);    // Turn off
    delay(1000);
    board1.write1(11, 128);  // Turn on at half power
    delay(1000);
    board1.write1(11, 0);    // Turn off
  } else {
    Serial.println("Failed to initialize PCA9635!");
    Serial.println("Please check:");
    Serial.println("1. I2C connections (SDA and SCL)");
    Serial.println("2. Power supply to PCA9635");
    Serial.println("3. Address jumpers (should be 0x40)");

    // Run scanner again if initialization failed
    Serial.println("\nRunning I2C scan again to verify...");
    scanI2CBus();
  }
}

void loop() {
  // Blink output 11 to show it's working
  board1.write1(11, 255);  // Turn on
  Serial.println("Output 11 ON");
  delay(500);
  board1.write1(11, 0);    // Turn off
  Serial.println("Output 11 OFF");
  delay(500);
} 

// I2C Scanner function
void scanI2CBus() {
  byte error, address;
  int deviceCount = 0;
  boolean foundPCA9635 = false;
  byte foundPCA9635Address = 0;
  
  Serial.println("\n========== I2C SCANNER ==========");
  Serial.println("Scanning addresses 0x01-0x7F...");
  
  // Scan addresses from 1 to 127 (0x01 to 0x7F)
  for(address = 1; address < 128; address++) {
    // The i2c_scanner uses the return value of
    // the Write.endTransmission to see if a device acknowledged the address
    Wire.beginTransmission(address);
    error = Wire.endTransmission();
    
    if(error == 0) {
      Serial.print("Device found at address 0x");
      if(address < 16) {
        Serial.print("0");
      }
      Serial.print(address, HEX);
      
      // Check if this might be a PCA9635
      if(address >= 0x40 && address <= 0x7F) {
        Serial.print(" (Possible PCA9635)");
        foundPCA9635 = true;
        foundPCA9635Address = address;
      }
      Serial.println();
      deviceCount++;
    }
    else if(error == 4) {
      Serial.print("Unknown error at address 0x");
      if(address < 16) {
        Serial.print("0");
      }
      Serial.println(address, HEX);
    }
  }
  
  if(deviceCount == 0) {
    Serial.println("No I2C devices found! Check your connections.");
    Serial.println("Issues to check:");
    Serial.println("1. Are SDA and SCL connected correctly?");
    Serial.println("2. Do you have proper pull-up resistors (typically 4.7kΩ)?");
    Serial.println("3. Is the PCA9635 powered properly?");
    Serial.println("4. Are there any shorts or broken traces on the PCB?");
  }
  else {
    Serial.print("Found ");
    Serial.print(deviceCount);
    Serial.println(" I2C device(s)");
    
    if(foundPCA9635) {
      // Analyze what address was found and what it means for jumper settings
      analyzePCA9635Address(foundPCA9635Address);
    } else {
      Serial.println("WARNING: No device detected in the PCA9635 address range (0x40-0x7F).");
      Serial.println("Expected default address is 0x40 (with all jumpers OPEN).");
    }
  }
  
  Serial.println("===================================\n");
}

// Analyze PCA9635 address to determine jumper settings
void analyzePCA9635Address(byte foundAddress) {
  byte baseAddress = 0x40;  // PCA9635 base address
  byte addressOffset = foundAddress - baseAddress;
  
  Serial.println("\nPCA9635 ADDRESS ANALYSIS:");
  Serial.print("Found address: 0x");
  if(foundAddress < 16) Serial.print("0");
  Serial.println(foundAddress, HEX);
  
  Serial.println("Expected jumper settings (based on found address):");
  
  if(addressOffset == 0) {
    Serial.println("All address jumpers (A0-A5) should be OPEN for address 0x40");
    Serial.println("This is the DEFAULT configuration.");
  } else {
    Serial.print("Base address: 0x40 + Offset: 0x");
    Serial.print(addressOffset, HEX);
    Serial.print(" = 0x");
    Serial.println(foundAddress, HEX);
    
    Serial.println("The following jumpers should be CLOSED:");
    
    // Check each bit and report which jumpers should be closed
    if(addressOffset & 0x01) Serial.println("- A0 jumper");
    if(addressOffset & 0x02) Serial.println("- A1 jumper");
    if(addressOffset & 0x04) Serial.println("- A2 jumper");
    if(addressOffset & 0x08) Serial.println("- A3 jumper");
    if(addressOffset & 0x10) Serial.println("- A4 jumper");
    if(addressOffset & 0x20) Serial.println("- A5 jumper");
    
    Serial.println("\nAll other jumpers should be OPEN.");
  }
  
  // If the address is not 0x40 (default), give additional information
  if(addressOffset != 0) {
    Serial.println("\nTo use this device at its current address:");
    Serial.println("1. Update your code to use: PCA9635 board1(0x" + String(foundAddress, HEX) + ");");
    Serial.println("2. OR remove all jumpers to return to default address 0x40");
  }
}