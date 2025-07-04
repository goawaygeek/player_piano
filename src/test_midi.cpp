/*
 * Proper USB String Descriptor Implementation for ESP32-S3
 * Based on TinyUSB examples and ESP32 Arduino framework
 */

#include <stdint.h>
#include "Arduino.h"

#if ARDUINO_USB_MODE
#warning This sketch must be used when USB is in OTG mode
void setup() {}
void loop() {}
#else

#include "USB.h"
#include "esp32-hal-tinyusb.h"

// String descriptor indices
enum {
    STRID_LANGID = 0,
    STRID_MANUFACTURER,
    STRID_PRODUCT,
    STRID_SERIAL,
    STRID_MIDI_INTERFACE
};

// Define your device strings
static const char* string_desc_arr[] = {
    (const char[]) { 0x09, 0x04 },  // 0: Language ID (English)
    "MyCompany",                     // 1: Manufacturer
    "Deru Piano Controller",      // 2: Product
    "123456789",                     // 3: Serial Number
    "Deru MIDI Interface"         // 4: MIDI Interface Name
};

// Custom device descriptor with proper VID/PID
static const tusb_desc_device_t device_descriptor = {
    .bLength            = sizeof(tusb_desc_device_t),
    .bDescriptorType    = TUSB_DESC_DEVICE,
    .bcdUSB             = 0x0200,     // USB 2.0
    .bDeviceClass       = 0x00,       // Defined at interface level
    .bDeviceSubClass    = 0x00,
    .bDeviceProtocol    = 0x00,
    .bMaxPacketSize0    = 64,
    
    .idVendor           = 0x1234,     // Your vendor ID (use real one for production)
    .idProduct          = 0x5678,     // Your product ID
    .bcdDevice          = 0x0100,     // Device version 1.0
    
    .iManufacturer      = STRID_MANUFACTURER,
    .iProduct           = STRID_PRODUCT,
    .iSerialNumber      = STRID_SERIAL,
    
    .bNumConfigurations = 1
};

// Custom string descriptor callback
extern "C" uint16_t const* tud_descriptor_string_cb(uint8_t index, uint16_t langid) {
    static uint16_t _desc_str[32 + 1];  // Buffer for string descriptor
    uint8_t chr_count;

    (void) langid;  // Ignore language ID for now

    switch (index) {
        case STRID_LANGID:
            memcpy(&_desc_str[1], string_desc_arr[0], 2);
            chr_count = 1;
            break;

        case STRID_MANUFACTURER:
        case STRID_PRODUCT:
        case STRID_SERIAL:
        case STRID_MIDI_INTERFACE:
            {
                const char* str = string_desc_arr[index];
                chr_count = strlen(str);
                
                // Cap at max length
                if (chr_count > 31) chr_count = 31;
                
                // Convert ASCII to UTF-16
                for (uint8_t i = 0; i < chr_count; i++) {
                    _desc_str[1 + i] = str[i];
                }
            }
            break;

        default:
            // Unsupported string index
            return NULL;
    }

    // First byte is length (including header), second byte is string type
    _desc_str[0] = (TUSB_DESC_STRING << 8) | (2 * chr_count + 2);
    
    return _desc_str;
}

// Custom device descriptor callback
extern "C" uint8_t const* tud_descriptor_device_cb(void) {
    return (uint8_t const*) &device_descriptor;
}

// MIDI descriptor with proper string reference
extern "C" uint16_t tusb_midi_load_descriptor(uint8_t *dst, uint8_t *itf) {
    Serial.println("Loading MIDI descriptor with proper strings...");
    
    // Use our custom MIDI interface string
    uint8_t str_index = STRID_MIDI_INTERFACE;
    uint8_t ep_num = tinyusb_get_free_duplex_endpoint();
    
    if (ep_num == 0) {
        Serial.println("ERROR: No free endpoints!");
        return 0;
    }
    
    Serial.printf("MIDI: Interface string=%d, Endpoint=%d\n", str_index, ep_num);
    
    uint8_t descriptor[TUD_MIDI_DESC_LEN] = {
        TUD_MIDI_DESCRIPTOR(*itf, str_index, ep_num, (uint8_t)(0x80 | ep_num), 64)
    };
    
    memcpy(dst, descriptor, TUD_MIDI_DESC_LEN);
    (*itf)++;
    
    Serial.println("MIDI descriptor loaded with custom strings");
    return TUD_MIDI_DESC_LEN;
}

// USB event callback
static void usbEventCallback(void *arg, esp_event_base_t event_base,
                           int32_t event_id, void *event_data) {
    if (event_base == ARDUINO_USB_EVENTS) {
        switch (event_id) {
        case ARDUINO_USB_STARTED_EVENT:
            Serial.println("USB CONNECTED - Device should show as 'Deru Piano Controller'");
            break;
        case ARDUINO_USB_STOPPED_EVENT:
            Serial.println("USB DISCONNECTED");
            break;
        }
    }
}

void processMidiPacket(uint8_t *packet) {
    uint8_t status = packet[1];
    uint8_t data1 = packet[2];
    uint8_t data2 = packet[3];
    
    if ((status & 0xF0) == 0x90 && data2 > 0) {
        Serial.printf("♪ Note ON:  Note=%d, Velocity=%d\n", data1, data2);
    }
    else if ((status & 0xF0) == 0x80 || ((status & 0xF0) == 0x90 && data2 == 0)) {
        Serial.printf("♫ Note OFF: Note=%d\n", data1);
    }
    else if ((status & 0xF0) == 0xB0) {
        Serial.printf("🎛 Control: CC%d = %d\n", data1, data2);
    }
}

void setup() {
    Serial.begin(115200);
    delay(1000);
    
    Serial.println("\n====================================");
    Serial.println("Deru Piano Controller");
    Serial.println("USB MIDI Test with Proper Naming");
    Serial.println("====================================");

    // Initialize USB with proper string descriptors
    USB.onEvent(usbEventCallback);
    
    // Enable MIDI interface with our custom descriptors
    tinyusb_enable_interface(USB_INTERFACE_MIDI, TUD_MIDI_DESC_LEN,
                            tusb_midi_load_descriptor);
    
    USB.begin();

    Serial.println("USB initialized - Device name should be visible now!");
    Serial.println("Check your system's MIDI devices list");
    Serial.println("====================================\n");

    // MIDI processing task
    xTaskCreate([](void *param) {
        uint8_t packet[4];
        Serial.println("MIDI task ready - play some notes!");
        
        while (true) {
            delay(1);
            while (tud_midi_available()) {
                if (tud_midi_packet_read(packet)) {
                    processMidiPacket(packet);
                }
            }
        }
    }, "midi_task", 2048, NULL, 5, NULL);
}

void loop() {
    static unsigned long lastCheck = 0;
    
    if (millis() - lastCheck > 10000) {
        Serial.println("⏰ Device running - connect MIDI controller");
        lastCheck = millis();
    }
    
    delay(100);
}

#endif /* ARDUINO_USB_MODE */