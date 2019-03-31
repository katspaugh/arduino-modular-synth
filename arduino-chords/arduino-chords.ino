#include <Wire.h>

static const int CLOCK_PIN = 2;
static const int VOCT_PIN = A0;
static const int HOLD_PIN = 11;
static const int LED = 13;

// Analog pins for the potentiometers
const int pot1 = A1; // scale
const int pot2 = A2; // octave
const int pot3 = A3; // arp pattern

const byte DAC_address = 0x60;
// 2^12 = 4096 total DAC counts.
// 4096/5 = 819.2 DAC counts per volt on a 5V supply
// 819.2/12 = dac counts per semitone = 68.26
// times 100 for some extra calculation precision = 6826
static const uint32_t DAC_CAL = 8889;
//static const uint32_t DAC_CAL = 6826;

static const int SCALES[8][24] = {
  //scale00 Chromatic
  {
    0,  1,  2,  3,  4,  5,  6,  7,  8,  9,  10,  11,
    12, 13, 13, 15, 16, 17, 18, 19, 20, 21, 22,  23
  },
  
  //scale01 Major
  {
    0,  0,  2,  2,  4,  4,  5,  7,  7,  9,  9,   11,
    12, 12, 14, 14, 16, 16, 17, 19, 19, 21, 21,  22
  },
  
  //scale02 Minor
  {
    0,  0,  2,  3,  3,  5,  5,  7,  8,  8,  10,  10,
    12, 12, 14, 15, 15, 17, 17, 19, 20, 20, 22,  22
  },
  
  //scale03 Pentatonic
  {
    0,  0,  2,  3,  3,  5,  5,  7,  7,  7,  10,  10,
    12, 12, 14, 15, 15, 17, 17, 19, 19, 19, 22,  22
  },
  
  //scale04 Dorian
  {
    0,  0,  2,  3,  3,  5,  5,  7,  7,  9,  10,  10,
    12, 12, 14, 15, 15, 17, 17, 19, 19, 21, 22,  22
  },
  
  //scale05 Maj7(9)
  {
    0,  0,  0,  0,  4,  4,  4,  7,  7,  7,  7,   11,
    12, 12, 12, 12, 16, 16, 16, 19, 19, 19, 19,  23
  },
  
  //scale06 Minor7(9,11)
  {
    0,  0,  0,  3,  3,  3,  3,  7,  7,  7,  10,  10,
    12, 12, 12, 15, 15, 15, 15, 19, 19, 19, 22,  22
  },
  
  //scale07 (WholeTone)
  {
    0,  0,  2,  2,  4,  4,  6,  6,  8,  8,  10,  10,
    12, 12, 14, 14, 16, 16, 18, 18, 20, 20, 22,  22
  }
};

int scale = 0;
int range = 12; // 1 octave range
int octave_offset = 0;
int arp_mode = 0;
int arp_beat = 0;
int total_arp_beats = 1;
int chord_notes[] = { 0, 0, 0, 0, 0, 0, 0, 0, 0 };
bool send_tick = false;
bool hold = false;

int CV0;
int CV1;
int CV2;
int CV3;

/**
 * Quantize a V/oct input into a chord
 */
void setChord() {
  int semitone = map(CV0, 0, 1023, 0, range);
  int octave = ceil(semitone / 12);
  int interval = semitone % 12;
  int offset = (octave + octave_offset) * 12;

  int note1 = SCALES[scale][interval] + offset;
  int note2 = SCALES[scale][interval + 3] + offset;
  int note3 = SCALES[scale][interval + 5] + offset;
  int note4 = SCALES[scale][interval + 12] + offset;
  int note5 = SCALES[scale][interval + 15] + offset;
  int note6 = SCALES[scale][interval + 17] + offset;

  total_arp_beats = arp_mode + 1;
  chord_notes[0] = note1;
  chord_notes[1] = note2;
  chord_notes[2] = note3;
  chord_notes[3] = note4;
  chord_notes[4] = note5;
  chord_notes[5] = note6;

  switch (arp_mode) {
    case 3: {
      chord_notes[3] = chord_notes[1];
      break;
    }
    case 4: {
      chord_notes[2] = note1;
      chord_notes[3] = note3;
      chord_notes[4] = note2;
      break;
    }
    case 6: {
      total_arp_beats = 6;
      chord_notes[0] = note6;
      chord_notes[1] = note5;
      chord_notes[2] = note4;
      chord_notes[3] = note3;
      chord_notes[4] = note2;
      chord_notes[5] = note1;
      break;
    }
    case 7: {
      total_arp_beats = 9;
      chord_notes[6] = note5;
      chord_notes[7] = note4;
      chord_notes[8] = note3;
      break;
    }
  }
}

void playNote(int quantizedNote) {
  uint32_t DC_Value = 400ul + ((quantizedNote * DAC_CAL) / 100ul);
  setDAC(DC_Value);
}

void onClock() {
  send_tick = true;
}

void readInputs() {
  CV0 = analogRead(VOCT_PIN);
  CV1 = analogRead(pot1);
  CV2 = analogRead(pot2);
  CV3 = analogRead(pot3);

  scale = map(CV1, 0, 1023, 0, 8);
  octave_offset = map(CV2, 0, 1023, -2, 2);
  arp_mode = map(CV3, 0, 1023, 0, 7);
}

void setDAC(uint32_t DC_Value) {
  Wire.beginTransmission(DAC_address);
  Wire.write(byte((DC_Value & 0x0f00) >> 8));
  Wire.write(byte(DC_Value & 0xff));
  Wire.endTransmission();
}

void setup() {
  pinMode(CLOCK_PIN, INPUT);
  pinMode(HOLD_PIN, INPUT); // normalized to the clock pin
  pinMode(LED, OUTPUT);
  digitalWrite(LED, LOW);

  attachInterrupt(digitalPinToInterrupt(CLOCK_PIN), onClock, RISING);

  Wire.begin();
}

void loop() {
  if (digitalRead(HOLD_PIN)) {
    hold = true;
  }
  if (!send_tick) return;
  send_tick = false;

  readInputs();

  if (arp_mode == 0 || hold) {
    setChord();
    hold = false;
    arp_beat = 0;
  }

  playNote(chord_notes[arp_beat]);

  arp_beat = (arp_beat + 1) % total_arp_beats;
}
