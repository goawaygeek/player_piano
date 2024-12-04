#include <BLEMIDI_Transport.h>
#include <hardware/BLEMIDI_ESP32_NimBLE.h>
#include "piano.h"
#include "sustain.h"
#include "command.h"
#include "PCA9635.h"
#include <stdint.h>

Piano piano;
Sustain sustain;

PCA9635 board1(0x40);
PCA9635 board2(0x41);
PCA9635 board3(0x42);
PCA9635 board4(0x43);
PCA9635 board5(0x44);
PCA9635 board6(0x45);
PCA9635 board7(0x46);

BLEMIDI_CREATE_INSTANCE("Amadeus", MIDI);

bool isConnected = false;

void setup() {
  Serial.begin(115200);
  Serial.println("Setup started");

  //MIDI.begin(MIDI_CHANNEL_OMNI);
  MIDI.begin();

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
  MIDI.setHandleNoteOn([](uint8_t _, uint8_t noteId, uint8_t velocity) { 
    piano.scheduleNote(noteId, velocity); 
    Serial.print("Received note on: ");
    Serial.println(velocity); });
  MIDI.setHandleNoteOff([](uint8_t _, uint8_t noteId, uint8_t velocity) { 
    piano.scheduleNote(noteId, 0); 
    Serial.println("Received note off!"); });
  MIDI.setHandleControlChange([](uint8_t channel, uint8_t number, uint8_t value) { 
    piano.scheduleSustain(channel, number, value); 
    Serial.println("Received control change!");
    });
  
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
  MIDI.read();  
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
