#include "piano.h"

void Piano::addCommand(Command command) { commands.push_back(command); }

vector<Note>::iterator Piano::find(int id) {
  if (id < MIN_NOTE_ID || id > MAX_NOTE_ID)
    return notes.end();

  return notes.begin() + (id - MIN_NOTE_ID); // notes are allocated in ascending id order
}

void Piano::initialize() {
  for (int id = MIN_NOTE_ID; id <= MAX_NOTE_ID; id++) {
    notes.emplace_back(id);
    notes.back().resetSchedule();
  }
}

void Piano::scheduleNote(uint8_t midiId, uint8_t velocity) {
  auto note_it = find(midiId);
  if (note_it != notes.end())
    note_it->addToSchedule(velocity);
}

void Piano::scheduleSustain(uint8_t channel, uint8_t number, uint8_t value) {
  if(number == SUSTAIN_CONTROL_NUMBER && channel == 1) {
    sustain.addToSchedule(value);
  }
}
