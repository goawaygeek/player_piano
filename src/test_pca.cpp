#include <Arduino.h>
#include <Wire.h>
#include "PCA9635.h"
#include "settings.h"

PCA9635 board1(0x40);  // Default address

void setup() {
  // Give USB time to initialize
  delay(2000);
  
  Serial.begin(115200);
  Serial.println("PCA9635 Test Program");
  
  // Initialize I2C
  Wire.begin(SDA_PIN, SCL_PIN);
  
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
  }
}

void loop() {
  // Blink output 11 to show it's working
  board1.write1(11, 255);  // Turn on
  delay(500);
  board1.write1(11, 0);    // Turn off
  delay(500);
} 