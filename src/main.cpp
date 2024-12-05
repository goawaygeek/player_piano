#include <BLEMIDI_Transport.h>
#include <hardware/BLEMIDI_ESP32_NimBLE.h>
#include "piano.h"
#include "sustain.h"
#include "command.h"
#include "PCA9635.h"
#include <stdint.h>

#if ARDUINO_USB_MODE
#warning This sketch must be used when USB is in OTG mode
void setup() {}
void loop() {}
#else

#include "USB.h"
#include "esp32-hal-tinyusb.h"

Piano piano;
Sustain sustain;

PCA9635 board1(0x40);
PCA9635 board2(0x41);
PCA9635 board3(0x42);
PCA9635 board4(0x43);
PCA9635 board5(0x44);
PCA9635 board6(0x45);
PCA9635 board7(0x46);

// BLEMIDI_CREATE_INSTANCE("Amadeus", MIDI);

extern "C" uint16_t tusb_midi_load_descriptor(uint8_t *dst, uint8_t *itf) {
  uint8_t str_index = tinyusb_add_string_descriptor("Amadeus USB MIDI");
  uint8_t ep_num = tinyusb_get_free_duplex_endpoint();
  TU_VERIFY(ep_num != 0);
  uint8_t descriptor[TUD_MIDI_DESC_LEN] = {
      TUD_MIDI_DESCRIPTOR(*itf, str_index, ep_num, (uint8_t)(0x80 | ep_num), 64)
  };
  *itf += 1;
  memcpy(dst, descriptor, TUD_MIDI_DESC_LEN);
  return TUD_MIDI_DESC_LEN;
}

// Add USB event callback
static void usbEventCallback(void *arg, esp_event_base_t event_base,
                           int32_t event_id, void *event_data) {
    if (event_base == ARDUINO_USB_EVENTS) {
        switch (event_id) {
        case ARDUINO_USB_STARTED_EVENT:
            Serial.println("USB PLUGGED");
            break;
        case ARDUINO_USB_STOPPED_EVENT:
            Serial.println("USB UNPLUGGED");
            break;
        }
    }
}

bool isConnected = false;

void setup() {
  Serial.begin(115200);
  Serial.println("Setup started");

  // Initialize USB MIDI
  USB.onEvent(usbEventCallback);
  tinyusb_enable_interface(USB_INTERFACE_MIDI, TUD_MIDI_DESC_LEN,
                          tusb_midi_load_descriptor);
  USB.begin();

  //MIDI.begin(MIDI_CHANNEL_OMNI);
  //MIDI.begin();

  // BLEMIDI.setHandleConnected([]() {
  //   isConnected = true;
  //   Serial.println("Connected!");
  // });

  // BLEMIDI.setHandleDisconnected([]() {
  //   isConnected = false;
  //   Serial.println("Disconnected :( ");
  // });

  // MIDI.setHandleNoteOn([](byte channel, byte note, byte velocity) {
  //   Serial.println("Received note on!");
  // });
  // MIDI.setHandleNoteOff([](byte channel, byte note, byte velocity) {
  //   Serial.println("Received note off!");
  // });

  

//  BLEMIDI.setHandleConnected([]() { schedule.connected(); });
//  BLEMIDI.setHandleDisconnected([]() { schedule.disconnected(); });

  piano.initialize();
//  schedule.poweredOn();

  // UPDATE: this needs to use the addToSchedule function
  // MIDI.setHandleNoteOn([](uint8_t _, uint8_t noteId, uint8_t velocity) { 
  //   piano.scheduleNote(noteId, velocity); 
  //   Serial.print("Received note on: ");
  //   Serial.println(velocity); });
  // MIDI.setHandleNoteOff([](uint8_t _, uint8_t noteId, uint8_t velocity) { 
  //   piano.scheduleNote(noteId, 0); 
  //   Serial.println("Received note off!"); });
  // MIDI.setHandleControlChange([](uint8_t channel, uint8_t number, uint8_t value) { 
  //   piano.scheduleSustain(channel, number, value); 
  //   Serial.println("Received control change!");
  //   });

  // Replace MIDI handlers with USB MIDI reading task
  xTaskCreate([](void *param) {
    uint8_t packet[4];
    while (true) {
      delay(1);
      while (tud_midi_available()) {
        if (tud_midi_packet_read(packet)) {
          uint8_t status = packet[1];
          uint8_t data1 = packet[2];
          uint8_t data2 = packet[3];
          
          // Note On
          if ((status & 0xF0) == 0x90) {
            piano.scheduleNote(data1, data2);
            Serial.print("Received note on: ");
            Serial.println(data2);
          }
          // Note Off
          else if ((status & 0xF0) == 0x80) {
            piano.scheduleNote(data1, 0);
            Serial.println("Received note off!");
          }
          // Control Change
          else if ((status & 0xF0) == 0xB0) {
            piano.scheduleSustain(status & 0x0F, data1, data2);
            Serial.println("Received control change!");
          }
        }
      }
    }
  }, "midi_task", 2048, NULL, 5, NULL);
  
   Wire.begin(SDA_PIN, SCL_PIN);
  
  board1.begin(PCA9635_MODE1_NONE, PCA9635_MODE2_INVERT | PCA9635_MODE2_TOTEMPOLE);
  for (int channel = 0; channel < board1.channelCount(); channel++) {
    board1.setLedDriverMode(channel, PCA9635_LEDPWM);
    board1.write1(channel, 0);
  }
  board2.begin(PCA9635_MODE1_NONE, PCA9635_MODE2_INVERT | PCA9635_MODE2_TOTEMPOLE);
  for (int channel = 0; channel < board2.channelCount(); channel++) {
    board2.setLedDriverMode(channel, PCA9635_LEDPWM);
    board2.write1(channel, 0);
  }
  board3.begin(PCA9635_MODE1_NONE, PCA9635_MODE2_INVERT | PCA9635_MODE2_TOTEMPOLE);
  for (int channel = 0; channel < board3.channelCount(); channel++) {
    board3.setLedDriverMode(channel, PCA9635_LEDPWM);
    board3.write1(channel, 0);
  }
  board4.begin(PCA9635_MODE1_NONE, PCA9635_MODE2_INVERT | PCA9635_MODE2_TOTEMPOLE);
  for (int channel = 0; channel < board4.channelCount(); channel++) {
    board4.setLedDriverMode(channel, PCA9635_LEDPWM);
    board4.write1(channel, 0);
  }
  board5.begin(PCA9635_MODE1_NONE, PCA9635_MODE2_INVERT | PCA9635_MODE2_TOTEMPOLE);
  for (int channel = 0; channel < board5.channelCount(); channel++) {
    board5.setLedDriverMode(channel, PCA9635_LEDPWM);
    board5.write1(channel, 0);
  }
  board6.begin(PCA9635_MODE1_NONE, PCA9635_MODE2_INVERT | PCA9635_MODE2_TOTEMPOLE);
  for (int channel = 0; channel < board6.channelCount(); channel++) {
    board6.setLedDriverMode(channel, PCA9635_LEDPWM);
    board6.write1(channel, 0);
  }
  board7.begin(PCA9635_MODE1_NONE, PCA9635_MODE2_INVERT | PCA9635_MODE2_TOTEMPOLE);
  for (int channel = 0; channel < board7.channelCount(); channel++) {
    board7.setLedDriverMode(channel, PCA9635_LEDPWM);
    board7.write1(channel, 0);
  }
}

void loop() {
  // MIDI.read();  
  // loop through the notes and and see if their schedule needs to be adjusted
  // Serial.println("Looping through notes");
  for (auto it = piano.notes.begin(); it != piano.notes.end(); it++) {
    it->processSchedule();
    it->checkForErrors();
  }

  sustain.processSchedule();
  sustain.checkForErrors();

  // loop through the commands and find which ones need to run
  for (auto it = piano.commands.begin(); it != piano.commands.end(); it++) {
    int midiId = it->getMidiId();
    int pwm = it->getPwm();
    //pwm = 90;
    if (midiId >= BOARD_1_MIN_ID && midiId <= BOARD_1_MAX_ID) {
      board1.write1(midiId - BOARD_1_MIN_ID, pwm);
    } else if (midiId >= BOARD_2_MIN_ID && midiId <= BOARD_2_MAX_ID) {
      board2.write1(midiId - BOARD_2_MIN_ID, pwm);
    } else if (midiId >= BOARD_3_MIN_ID && midiId <= BOARD_3_MAX_ID) {
      board3.write1(midiId - BOARD_3_MIN_ID, pwm);
    } else if (midiId >= BOARD_4_MIN_ID && midiId <= BOARD_4_MAX_ID) {
      board4.write1(midiId - BOARD_4_MIN_ID, pwm);
    } else if (midiId >= BOARD_5_MIN_ID && midiId <= BOARD_5_MAX_ID) {
      board5.write1(midiId - BOARD_5_MIN_ID, pwm);
    } else if (midiId >= BOARD_6_MIN_ID && midiId <= BOARD_6_MAX_ID) {
      board6.write1(midiId - BOARD_6_MIN_ID, pwm);
    } else if (midiId >= BOARD_7_MIN_ID && midiId <= BOARD_7_MAX_ID) {
      board7.write1(midiId - BOARD_7_MIN_ID, pwm);
    } else if (midiId == 109) { // sustain
      board7.write1(SUSTAIN_1_INDEX, it->getPwm());
      board7.write1(SUSTAIN_2_INDEX, it->getPwm());
    }
   Serial.print("RUNNING COMMAND: ");
   Serial.print("Midi Id: ");
   Serial.print(it->getMidiId());
   Serial.print(", PWM: ");
   Serial.println(it->getPwm());
   piano.commands.erase(it--);
  }
}

#endif /* ARDUINO_USB_MODE */