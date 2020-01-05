#include <MIDI.h>

// MIDI interface
MIDI_CREATE_INSTANCE(HardwareSerial, Serial, MIDI);

void handleNoteOn(byte channel, byte pitch, byte velocity) {
  MIDI.sendNoteOn(pitch, velocity, channel + 1);
}

void handleNoteOff(byte channel, byte pitch, byte velocity) {
  MIDI.sendNoteOff(pitch, velocity, channel + 1);
}

void handlePitchBend(byte channel, int bend) {
}

void handleClock() {
}

void handleStart() {
}

void handleContinue() {
}

void handleStop() {
}

void setup() {
  // Initiate MIDI communications, listen to all channels
  MIDI.begin(MIDI_CHANNEL_OMNI);

  // .begin sets the thru mode to on, so we'll have to turn it off if we don't want echo
  MIDI.turnThruOn();

  // so it is called upon reception of a NoteOn.
  MIDI.setHandleNoteOn(handleNoteOn);  // Put only the name of the function
  // Do the same for NoteOffs
  MIDI.setHandleNoteOff(handleNoteOff);
  MIDI.setHandlePitchBend(handlePitchBend);

  // Clock callback
  MIDI.setHandleClock(handleClock);
  MIDI.setHandleStart(handleStart);
  MIDI.setHandleContinue(handleContinue);
  MIDI.setHandleStop(handleStop);
}

void loop() {
  // Pump the MIDI parser as quickly as we can.
  // This will invoke the callbacks when messages are parsed.
  MIDI.read();
}
