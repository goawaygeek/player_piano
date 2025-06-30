/*
 * Fixed ESP32-S3 USB MIDI Test
 * Corrected descriptor loading and interface management
 */

#include <stdint.h>
#include "Arduino.h"

#if !ARDUINO_USB_MODE
#error "This sketch requires USB OTG mode. Please set ARDUINO_USB_MODE=1 in build flags"
void setup() {}
void loop() {}
#else

#include "USB.h"
#include "esp32-hal-tinyusb.h"

// USB connection state tracking
volatile bool usbConnected = false;

// USB event callback
static void usbEventCallback(void *arg, esp_event_base_t event_base,
                           int32_t event_id, void *event_data) {
    if (event_base == ARDUINO_USB_EVENTS) {
        switch (event_id) {
        case ARDUINO_USB_STARTED_EVENT:
            Serial.println("=== USB CABLE PLUGGED IN ===");
            usbConnected = true;
            break;
        case ARDUINO_USB_STOPPED_EVENT:
            Serial.println("=== USB CABLE UNPLUGGED ===");
            usbConnected = false;
            break;
        case ARDUINO_USB_SUSPEND_EVENT:
            Serial.println("USB SUSPENDED");
            break;
        case ARDUINO_USB_RESUME_EVENT:
            Serial.println("USB RESUMED");
            break;
        }
    }
}

// Fixed MIDI descriptor function
extern "C" uint16_t tusb_midi_load_descriptor(uint8_t *dst, uint8_t *itf) {
    Serial.printf("Loading MIDI descriptor... Interface: %d\n", *itf);
    
    // Add string descriptor for MIDI interface
    uint8_t str_index = tinyusb_add_string_descriptor("ESP32-S3 MIDI");
    if (str_index == 0) {
        Serial.println("ERROR: Failed to add string descriptor!");
        return 0;
    }
    
    // Get free endpoint pair
    uint8_t ep_num = tinyusb_get_free_duplex_endpoint();
    if (ep_num == 0) {
        Serial.println("ERROR: No free endpoints available!");
        return 0;
    }
    
    Serial.printf("MIDI: String index: %d, Endpoint: %d\n", str_index, ep_num);
    
    // Create descriptor with proper interface number
    uint8_t descriptor[TUD_MIDI_DESC_LEN] = {
        TUD_MIDI_DESCRIPTOR(*itf, str_index, ep_num, (uint8_t)(0x80 | ep_num), 64)
    };
    
    // Copy descriptor
    memcpy(dst, descriptor, TUD_MIDI_DESC_LEN);
    
    // Increment interface counter
    (*itf)++;
    
    Serial.printf("MIDI descriptor loaded successfully (length: %d)\n", TUD_MIDI_DESC_LEN);
    return TUD_MIDI_DESC_LEN;
}

// Simple MIDI packet processor
void processMidiPacket(uint8_t *packet) {
    uint8_t cable = (packet[0] >> 4) & 0x0F;
    uint8_t cin = packet[0] & 0x0F;
    uint8_t status = packet[1];
    uint8_t data1 = packet[2];
    uint8_t data2 = packet[3];
    
    Serial.printf("MIDI: Cable=%d, CIN=0x%X, Status=0x%X, Data1=%d, Data2=%d\n", 
                  cable, cin, status, data1, data2);
    
    // Echo the packet back (for testing)
    tud_midi_packet_write(packet);
}

void setup() {
    Serial.begin(115200);
    delay(2000);  // Give time for serial monitor to connect
    
    Serial.println("====================================");
    Serial.println("ESP32-S3 USB MIDI Test Starting...");
    Serial.println("====================================");
    
    // Register USB event callback FIRST
    USB.onEvent(usbEventCallback);
    Serial.println("USB event callback registered");
    
    // Enable MIDI interface ONLY (no CDC to avoid conflicts)
    Serial.println("Enabling MIDI interface...");
    if (!tinyusb_enable_interface(USB_INTERFACE_MIDI, TUD_MIDI_DESC_LEN, tusb_midi_load_descriptor)) {
        Serial.println("ERROR: Failed to enable MIDI interface!");
        return;
    }
    
    Serial.println("Starting USB...");
    USB.begin();
    
    // Small delay to let USB initialize
    delay(100);
    
    Serial.println("USB initialization complete");
    Serial.println("Please connect USB cable and check if device appears in MIDI setup");
    Serial.println("====================================");
    
    // Create MIDI monitoring task
    xTaskCreate([](void *param) {
        uint8_t packet[4];
        Serial.println("MIDI monitoring task started");
        
        while (true) {
            // Check TinyUSB connection status
            static bool wasMounted = false;
            bool isMounted = tud_mounted();
            
            if (isMounted && !wasMounted) {
                Serial.println("*** TinyUSB DEVICE MOUNTED (Host recognized device) ***");
                wasMounted = true;
            } else if (!isMounted && wasMounted) {
                Serial.println("*** TinyUSB DEVICE UNMOUNTED ***");
                wasMounted = false;
            }
            
            // Process incoming MIDI
            while (tud_midi_available()) {
                if (tud_midi_packet_read(packet)) {
                    processMidiPacket(packet);
                }
            }
            
            vTaskDelay(pdMS_TO_TICKS(10)); // 10ms delay
        }
    }, "midi_monitor", 4096, NULL, 5, NULL);
    
    // Send a test MIDI note every 5 seconds
    xTaskCreate([](void *param) {
        uint8_t note = 60; // Middle C
        bool noteOn = true;
        
        while (true) {
            vTaskDelay(pdMS_TO_TICKS(5000)); // Wait 5 seconds
            
            if (tud_mounted()) {
                uint8_t packet[4];
                packet[0] = 0x09; // Cable 0, Note On
                packet[1] = noteOn ? 0x90 : 0x80; // Note On/Off, Channel 1
                packet[2] = note;
                packet[3] = noteOn ? 100 : 0; // Velocity
                
                if (tud_midi_packet_write(packet)) {
                    Serial.printf("Sent test %s: Note %d\n", noteOn ? "Note On" : "Note Off", note);
                    noteOn = !noteOn;
                } else {
                    Serial.println("Failed to send test MIDI");
                }
            } else {
                Serial.println("Device not mounted - skipping test MIDI");
            }
        }
    }, "midi_test", 2048, NULL, 3, NULL);
}

void loop() {
    // Print status every 10 seconds
    static unsigned long lastStatus = 0;
    unsigned long now = millis();
    
    if (now - lastStatus > 10000) {
        Serial.println("--- Status Report ---");
        Serial.printf("USB Connected: %s\n", usbConnected ? "YES" : "NO");
        Serial.printf("TinyUSB Mounted: %s\n", tud_mounted() ? "YES" : "NO");
        Serial.printf("Free heap: %d bytes\n", ESP.getFreeHeap());
        Serial.printf("Uptime: %lu seconds\n", now / 1000);
        Serial.println("--------------------");
        lastStatus = now;
    }
    
    delay(100);
}

#endif /* ARDUINO_USB_MODE */