#include <Arduino.h>
#include <Wire.h>
#include "PCA9635.h"
#include "settings.h"

// PCA9635 instances with their respective addresses
PCA9635 board1(0x40); // Default board (no jumpers)
// PCA9635 board2(0x41); // Board with A0 jumper set
// PCA9635 board3(0x42); // Board with A1 jumper set
// PCA9635 board4(0x44); // Board with A2 jumper set
// PCA9635 board5(0x48); // Board with A3 jumper set

// Configuration constants
#define SOLENOID_ON 255      // Logic value to turn ON the N-channel MOSFET (LOW)
#define SOLENOID_OFF 0   // Logic value to turn OFF the N-channel MOSFET (HIGH)
#define PULSE_DURATION 100 // Solenoid activation pulse in milliseconds
#define MIN_SUPPLY_VOLTAGE 10.0 // Minimum voltage required for reliable operation

// Function prototypes
void scanI2CBus();
void testAllOutputs(PCA9635 &board, String boardName);
void testSingleOutput(PCA9635 &board, String boardName, uint8_t outputPin);
void pulseOutput(PCA9635 &board, String boardName, uint8_t outputPin, unsigned long duration);
void printStatus(String message);
float readSupplyVoltage(); // If you have a voltage monitoring pin

void setup() {
  // Give USB time to initialize
  delay(2000);
  
  Serial.begin(115200);
  Serial.println("\n==========================================");
  Serial.println("PCA9635 Multi-Board Solenoid Driver");
  Serial.println("==========================================");
  
  // Initialize I2C
  Wire.begin(SDA_PIN, SCL_PIN);
  
  // Run I2C scanner to confirm device presence
  scanI2CBus();
  
  // Initialize PCA9635 boards with correct configuration for solenoid driving
  printStatus("Initializing PCA9635 boards...");
  
  // IMPORTANT: Use INVRT mode to make LOW outputs turn ON the MOSFETs
  // This inverts the logic so write1(pin, 0) turns ON the MOSFET and write1(pin, 255) turns it OFF
  bool board1Init = board1.begin(PCA9635_MODE1_NONE, PCA9635_MODE2_TOTEMPOLE | PCA9635_MODE2_INVERT);
//   bool board2Init = board2.begin(PCA9635_MODE1_NONE, PCA9635_MODE2_TOTEMPOLE | PCA9635_MODE2_INVERT);
//   bool board3Init = board3.begin(PCA9635_MODE1_NONE, PCA9635_MODE2_TOTEMPOLE | PCA9635_MODE2_INVERT);
//   bool board4Init = board4.begin(PCA9635_MODE1_NONE, PCA9635_MODE2_TOTEMPOLE | PCA9635_MODE2_INVERT);
//   bool board5Init = board5.begin(PCA9635_MODE1_NONE, PCA9635_MODE2_TOTEMPOLE | PCA9635_MODE2_INVERT);
  
  if (board1Init) {
    printStatus("Board 1 (0x40, default) initialized successfully with INVERTED outputs");
    
    // Set all channels to PWM mode and ensure they're OFF to start
    for (int channel = 0; channel < board1.channelCount(); channel++) {
      board1.setLedDriverMode(channel, PCA9635_LEDPWM);
      board1.write1(channel, SOLENOID_OFF);  // Start with all solenoids OFF
    }
  } else {
    printStatus("Failed to initialize Board 1 (0x40)!");
  }
  
//   if (board2Init) {
//     printStatus("Board 2 (0x42, A0 jumper) initialized successfully with INVERTED outputs");
    
//     // Set all channels to PWM mode and ensure they're OFF to start
//     for (int channel = 0; channel < board2.channelCount(); channel++) {
//       board2.setLedDriverMode(channel, PCA9635_LEDPWM);
//       board2.write1(channel, SOLENOID_OFF);  // Start with all solenoids OFF
//     }
//   } else {
//     printStatus("Failed to initialize Board 2 (0x42)!");
//   }

//   if (board3Init) {
//     printStatus("Board 3 (0x42, A1 jumper) initialized successfully with INVERTED outputs");
    
//     // Set all channels to PWM mode and ensure they're OFF to start
//     for (int channel = 0; channel < board3.channelCount(); channel++) {
//       board3.setLedDriverMode(channel, PCA9635_LEDPWM);
//       board3.write1(channel, SOLENOID_OFF);  // Start with all solenoids OFF
//     }
//   } else {
//     printStatus("Failed to initialize Board 3 (0x42)!");
//   }

//   if (board4Init) {
//     printStatus("Board 4 (0x43, A2 jumper) initialized successfully with INVERTED outputs");
    
//     // Set all channels to PWM mode and ensure they're OFF to start
//     for (int channel = 0; channel < board4.channelCount(); channel++) {
//       board4.setLedDriverMode(channel, PCA9635_LEDPWM);
//       board4.write1(channel, SOLENOID_OFF);  // Start with all solenoids OFF
//     }
//   } else {
//     printStatus("Failed to initialize Board 4 (0x43)!");
//   }

//   if (board5Init) {
//     printStatus("Board 5 (0x44, A3 jumper) initialized successfully with INVERTED outputs");
    
//     // Set all channels to PWM mode and ensure they're OFF to start
//     for (int channel = 0; channel < board5.channelCount(); channel++) {
//       board5.setLedDriverMode(channel, PCA9635_LEDPWM);
//       board5.write1(channel, SOLENOID_OFF);  // Start with all solenoids OFF
//     }
//   } else {
//     printStatus("Failed to initialize Board 5 (0x43)!");
//   }
  
  delay(1000);  // Wait a moment
}

// Print help information
void printHelp() {
  Serial.println("Commands:");
  Serial.println("1:X - Test output X on Board 1 (0x40, default address)");
  Serial.println("2:X - Test output X on Board 2 (0x41, A0 jumper)");
  Serial.println("3:X - Test output X on Board 3 (0x42, A1 jumper)");
  Serial.println("4:X - Test output X on Board 4 (0x44, A2 jumper)");
  Serial.println("5:X - Test output X on Board 5 (0x48, A3 jumper)");
  Serial.println("1:all - Test all outputs on Board 1");
  Serial.println("2:all - Test all outputs on Board 2");
  Serial.println("3:all - Test all outputs on Board 3");
  Serial.println("4:all - Test all outputs on Board 4");
  Serial.println("5:all - Test all outputs on Board 5");
  Serial.println("1:pulse X - Pulse output X on Board 1");
  Serial.println("2:pulse X - Pulse output X on Board 2");
  Serial.println("3:pulse X - Pulse output X on Board 3");
  Serial.println("4:pulse X - Pulse output X on Board 4");
  Serial.println("5:pulse X - Pulse output X on Board 5");
  Serial.println("1:pulse - Pulse all outputs on Board 1");
  Serial.println("2:pulse - Pulse all outputs on Board 2");
  Serial.println("3:pulse - Pulse all outputs on Board 3");
  Serial.println("4:pulse - Pulse all outputs on Board 4");
  Serial.println("5:pulse - Pulse all outputs on Board 5");
  Serial.println("scan - Run I2C scan");
  Serial.println("X - Test output X on Board 1 (default if no board specified)");
  Serial.println("all - Test all outputs on Board 1 (default if no board specified)");
}

// Process commands for a specific board
void processCommand(PCA9635 &board, String boardName, String command) {
  Serial.print("command: ");
  Serial.println(command);
  
  // Check for non-numeric commands first
  if (command == "all") {
    Serial.print("Testing all ");
    Serial.print(boardName);
    Serial.println(" outputs sequentially");
    testAllOutputs(board, boardName);
  }
  else if (command.startsWith("pulse ")) {
    int pin = command.substring(6).toInt();
    if (pin >= 0 && pin < 16) {
      Serial.print("Sending pulse to ");
      Serial.print(boardName);
      Serial.print(" output ");
      Serial.println(pin);
      pulseOutput(board, boardName, pin, PULSE_DURATION);
    }
  }
  else if (command == "pulse") {
    Serial.print("Sending pulse to all ");
    Serial.print(boardName);
    Serial.println(" outputs");
    for (int i = 0; i < 16; i++) {
      pulseOutput(board, boardName, i, PULSE_DURATION);
      delay(100);
    }
  }
  // Check for numeric commands (pin numbers) - but only if it's actually a valid number
  else if (command.length() > 0 && (command.charAt(0) >= '0' && command.charAt(0) <= '9')) {
    int pin = command.toInt();
    if (pin >= 0 && pin < 16) {
      Serial.print("Testing ");
      Serial.print(boardName);
      Serial.print(" pin ");
      Serial.println(pin);
      testSingleOutput(board, boardName, pin);
    } else {
      Serial.println("Invalid pin number. Use 0-15.");
    }
  }
  else {
    printHelp();
  }
}

void loop() {
  // Process serial commands
  if (Serial.available() > 0) {
    String input = Serial.readStringUntil('\n');
    input.trim();
    
    // Process commands for all boards
    if (input.startsWith("1:")) {
      // Commands for Board 1 (default 0x40)
      String boardCmd = input.substring(2);
      processCommand(board1, "Board 1", boardCmd);
    }
    // else if (input.startsWith("2:")) {
    //   // Commands for Board 2 (A1 jumper, 0x42)
    //   String boardCmd = input.substring(2);
    //   processCommand(board2, "Board 2", boardCmd);
    // } 
    // else if (input.startsWith("4:")) {
    //   // Commands for Board 2 (A1 jumper, 0x42)
    //   String boardCmd = input.substring(2);
    //   processCommand(board4, "Board 4", boardCmd);
    // }
    // else if (input.startsWith("5:")) {
    //   // Commands for Board 2 (A1 jumper, 0x42)
    //   String boardCmd = input.substring(2);
    //   processCommand(board5, "Board 5", boardCmd);
    // }
    else if (input == "scan") {
      Serial.println("Running I2C scan");
      scanI2CBus();
    }
    else if (input == "help") {
      printHelp();
    }
    else {
      // Default to Board 1 if no board specified (changed from Board 2)
      processCommand(board1, "Board 1", input);
    }
  }
}

// Function to test a single output with proper ON/OFF values
void testSingleOutput(PCA9635 &board, String boardName, uint8_t outputPin) {
  if (outputPin >= 16) return;
  
  Serial.println("Single solenoid test. ");
  Serial.print("Testing ");
  Serial.print(boardName);
  Serial.print(" output pin ");
  Serial.print(outputPin);
  Serial.println(":");
  
  // Turn off all outputs first
  for (int i = 0; i < 16; i++) {
    board.write1(i, SOLENOID_OFF);
  }
  
  // Turn on the selected output
  Serial.print("  Turning ON output ");
  Serial.println(outputPin);
  board.write1(outputPin, SOLENOID_ON);  // Use the correct ON value
  
  delay(500);  // Keep on for 0.5 second
  
  Serial.print("  Turning OFF output ");
  Serial.println(outputPin);
  board.write1(outputPin, SOLENOID_OFF);  // Use the correct OFF value
  
  delay(500);  // Wait a moment before next test
}

// Function to test all outputs one by one with proper ON/OFF values
void testAllOutputs(PCA9635 &board, String boardName) {
  Serial.print("Sequential test of all ");
  Serial.print(boardName);
  Serial.println(" outputs...");
  
  for (int i = 0; i < 16; i++) {
    Serial.print(boardName);
    Serial.print(" Output ");
    Serial.print(i);
    Serial.println(": ON");
    
    board.write1(i, SOLENOID_ON);   // Turn ON using the correct value
    delay(500);                     // Keep on for 0.5 seconds
    board.write1(i, SOLENOID_OFF);  // Turn OFF using the correct value
    
    delay(200);  // Brief pause between outputs
  }
  
  Serial.print("All ");
  Serial.print(boardName);
  Serial.println(" outputs tested.");
}

// Function to send a short pulse to a solenoid
void pulseOutput(PCA9635 &board, String boardName, uint8_t outputPin, unsigned long duration) {
  if (outputPin >= 16) return;
  
  Serial.print("Pulsing ");
  Serial.print(boardName);
  Serial.print(" output ");
  Serial.print(outputPin);
  Serial.print(" for ");
  Serial.print(duration);
  Serial.println("ms");
  
  // Quick OFF-ON-OFF sequence
  board.write1(outputPin, SOLENOID_OFF);  // Ensure OFF
  delay(50);
  board.write1(outputPin, SOLENOID_ON);   // Turn ON
  delay(duration);                        // Hold for specified duration
  board.write1(outputPin, SOLENOID_OFF);  // Turn OFF
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
        Serial.print(" (PCA9635 Board 1 - default address)");
      // } else if(address == 0x41) {
      //   Serial.print(" (PCA9635 Board 2 - A0 jumper set)");
      // } else if(address == 0x42) {
      //   Serial.print(" (PCA9635 Board 3 - A1 jumper set)");
      // } else if(address == 0x44) {
      //   Serial.print(" (PCA9635 Board 4 - A2 jumper set)");
      // } else if(address == 0x48) {
      //   Serial.print(" (PCA9635 Board 5 - A3 jumper set)");
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