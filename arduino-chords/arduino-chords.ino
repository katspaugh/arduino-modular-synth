#include <Wire.h>
#include "tables.h"

static const int CLOCK_PIN = 2;
static const int VOCT_PIN = A0;
static const int HOLD_PIN = 11;
static const int LED = 13;

// Analog pins for the potentiometers
const int pot1 = A1; // scale
const int pot2 = A2; // octave
const int pot3 = A3; // key

const byte DAC_address = 0x60;
// 2^12 = 4096 total DAC counts.
// 4096/5 = 819.2 DAC counts per volt on a 5V supply
// 819.2/12 = dac counts per semitone = 68.26
// times 100 for some extra calculation precision = 6826
//static const uint32_t DAC_CAL = 8889;
static const uint32_t DAC_CAL = 6826;

int scale = 0;          // scale
int octaveOffset = 0;   // octave
int semitoneOffset = 0; // key transposition
int total_arp_beats = 3;
int arp_beat = 0;
int chord_notes[] = { 0, 0, 0, 0, 0 };
bool hold = false;
bool last_hold = false;
bool send_tick = false;

int CV0;
int CV1;
int CV2;
int CV3;

/**
 * Quantize a V/oct input into a chord
 */
void setChord(int note) {
  int octaveSize;
  int notesToShift;

  switch (scale) {
    case 0:
      octaveSize = 12;
      notesToShift = (octaveOffset * octaveSize) + semitoneOffset;
      chord_notes[0] = mapChromatic(note, 0);
      chord_notes[1] = mapChromatic(note, 4);
      chord_notes[2] = mapChromatic(note, 7);
      for (int i = 0; i < 3; i++) {
        chord_notes[i] = shiftNotes(chord_notes[i], notesToShift, chromaTable, sizeof(chromaTable));
      }
      break;
    case 1:
      octaveSize = 7;
      notesToShift = (octaveOffset * octaveSize) + semitoneOffset;
      chord_notes[0] = mapMaj(note, 0);
      chord_notes[1] = mapMaj(note, 3);
      chord_notes[2] = mapMaj(note, 5);
      for (int i = 0; i < 3; i++) {
        chord_notes[i] = shiftNotes(chord_notes[i], notesToShift, majTable, sizeof(majTable));
      }
      break;
    case 2:
      octaveSize = 7;
      notesToShift = (octaveOffset * octaveSize) + semitoneOffset;
      chord_notes[0] = mapMin(note, 0);
      chord_notes[1] = mapMin(note, 3);
      chord_notes[2] = mapMin(note, 5);
      for (int i = 0; i < 3; i++) {
        chord_notes[i] = shiftNotes(chord_notes[i], notesToShift, minTable, sizeof(minTable));
      }
      break;
    case 3:
      octaveSize = 6;
      notesToShift = (octaveOffset * octaveSize) + semitoneOffset;
      chord_notes[0] = mapPenta(note, 0);
      chord_notes[1] = mapPenta(note, 3);
      chord_notes[2] = mapPenta(note, 5);
      for (int i = 0; i < 3; i++) {
        chord_notes[i] = shiftNotes(chord_notes[i], notesToShift, pentaTable, sizeof(pentaTable));
      }
      break;
    case 4:
      octaveSize = 7;
      notesToShift = (octaveOffset * octaveSize) + semitoneOffset;
      chord_notes[0] = mapDorian(note, 0);
      chord_notes[1] = mapDorian(note, 3);
      chord_notes[2] = mapDorian(note, 5);
      for (int i = 0; i < 3; i++) {
        chord_notes[i] = shiftNotes(chord_notes[i], notesToShift, dorianTable, sizeof(dorianTable));
      }
      break;
    case 5:
      octaveSize = 4;
      notesToShift = (octaveOffset * octaveSize) + semitoneOffset;
      chord_notes[0] = mapMaj3rd(note, 0);
      chord_notes[1] = mapMaj3rd(note, 1);
      chord_notes[2] = mapMaj3rd(note, 2);
      for (int i = 0; i < 3; i++) {
        chord_notes[i] = shiftNotes(chord_notes[i], notesToShift, maj3rdTable, sizeof(maj3rdTable));
      }
      break;
    case 6:
      octaveSize = 4;
      notesToShift = (octaveOffset * octaveSize) + semitoneOffset;
      chord_notes[0] = mapMin3rd(note, 0);
      chord_notes[1] = mapMin3rd(note, 1);
      chord_notes[2] = mapMin3rd(note, 2);
      for (int i = 0; i < 3; i++) {
        chord_notes[i] = shiftNotes(chord_notes[i], notesToShift, min3rdTable, sizeof(min3rdTable));
      }
      break;
    case 7:
      octaveSize = 4;
      notesToShift = (octaveOffset * octaveSize) + semitoneOffset;
      chord_notes[0] = mapWh(note, 0);
      chord_notes[1] = mapWh(note, 3);
      chord_notes[2] = mapWh(note, 5);
      for (int i = 0; i < 3; i++) {
        chord_notes[i] = shiftNotes(chord_notes[i], notesToShift, whTable, sizeof(whTable));
      }
      break;
  }

  if (total_arp_beats == 4) {
    chord_notes[3] = chord_notes[1];
  } else if (total_arp_beats == 5) {
    int swap = chord_notes[2];
    chord_notes[2] = chord_notes[0];
    chord_notes[3] = swap;
    chord_notes[4] = chord_notes[1];
  }
}

void playNote(int quantizedNote) {
  setDAC(quantizedNote);
}

void onClock() {
  send_tick = true;
}

void readInputs() {
  if (hold) CV0 = analogRead(VOCT_PIN);
  CV1 = analogRead(pot1);
  CV2 = analogRead(pot2);
  CV3 = analogRead(pot3);

  scale = map(CV1, 0, 1023, 0, 7);
  //octaveOffset = map(CV2, 0, 1023, 0, 3);
  semitoneOffset = map(CV2, 0, 1023, 0, 12);
  total_arp_beats = map(CV3, 0, 1023, 1, 5);
}

void setDAC(uint8_t key) {
  uint32_t DC_Value = 400ul + ((key * DAC_CAL) / 100ul);

  Wire.beginTransmission(DAC_address);
  Wire.write(byte((DC_Value & 0x0f00) >> 8));
  Wire.write(byte(DC_Value & 0xff));
  Wire.endTransmission();
}

void setup() {
  pinMode(CLOCK_PIN, INPUT);
  pinMode(HOLD_PIN, INPUT);
  pinMode(LED, OUTPUT);

  attachInterrupt(digitalPinToInterrupt(CLOCK_PIN), onClock, RISING);

  //Serial.begin(9600);
  Wire.begin();
}

void loop() {
  if (!send_tick) return;

  send_tick = false;

  hold = digitalRead(HOLD_PIN) == HIGH;

  readInputs();
  setChord(CV0);

  if (hold != last_hold) {
    last_hold = hold;

    if (hold) {
//      Serial.print(" Scale ");
//      Serial.println(scale);
//      Serial.print(chord_notes[0]);
//      Serial.print(" - ");
//      Serial.print(chord_notes[1]);
//      Serial.print(" - ");
//      Serial.print(chord_notes[2]);
//      Serial.println(" ");

      digitalWrite(LED, HIGH);
    } else {
      digitalWrite(LED, LOW);
    }
  }

  playNote(chord_notes[arp_beat]);

  arp_beat = (arp_beat + 1) % total_arp_beats;
}
