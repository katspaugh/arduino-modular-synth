#include <MIDI.h>
#include <Wire.h>

// Functional assignments to Arduino pin numbers.
// Digital outputs
static const int GATEPIN = 6; // the same as the green led pin (but inversed)
static const int CLOCKPIN = 13;
static const int REDLEDPIN = 7;

// Analog input
static const int PIN_OCTAVE_POT = 0;
static const int PIN_TEMPO_POT = 1;

// Digital inputs
static const int UPBTNPIN = 2;
static const int DNBTNPIN = 3;
static const int SHORTBTNPIN = 4;

// MIDI constants
static const int CV_CHAN = 10;
static const int MIN_KEY = 36;
static const int MAX_KEY = 84;

// DAC steps
static const uint32_t DAC_CAL = 6987;

// Variables for the clock
static int tempo_delay = 0;
static int tick_count = 0;
static bool send_tick = false;
static bool clock_sync = false;

// The last bend records the most recently seen bend message.
// We need to keep track so we can update note CV when we get new notes,
// or new bend messages - we need the other half in order to put them together.
// bend is signed, 14-bit
static int octave = 0;
static int16_t last_bend = 0;
static uint8_t last_key = 0;
static uint8_t last_gates = 0;

// MIDI interface
MIDI_CREATE_INSTANCE(HardwareSerial, Serial, MIDI);

/*
 *  void updateCV(uint8_t key)
 *
 *Converts key number to DAC count value,
 *and sends the value tio the DAC
 */
void updateCV(uint8_t key) {
  uint32_t val = round((key * DAC_CAL) / 100ul);

  val += last_bend;

  Wire.beginTransmission(0x60);
  Wire.write(byte((val & 0x0f00) >> 8));
  Wire.write(byte(val & 0xff));
  Wire.endTransmission();
}

/*
 * void updateOutputs()
 *
 *  Update otputs sets the outputs to the current conditions.
 *  Called from note on, note off, arp tick.
 */
void updateOutputs() {
  uint8_t key = last_key + octave * 12;
  if (key < MIN_KEY) {
    key = MIN_KEY;
  } else if (key > MAX_KEY) {
    key = MAX_KEY;
  }
  key -= MIN_KEY;

  updateCV(key);
  digitalWrite(GATEPIN, last_gates > 0);
}

/////////////////////////////////////////////////////////////////////////
// Callbacks for the MIDI parser
/////////////////////////////////////////////////////////////////////////

/* void handleNoteOn(byte channel, byte pitch, byte velocity)
 *
 *  Called by MIDI parser when note on messages arrive.
 */
void handleNoteOn(byte channel, byte pitch, byte velocity) {
  if (channel == CV_CHAN) {
    last_key = pitch;
    last_gates += 1;
    updateOutputs();
  }
}

/* void handleNoteOff(byte channel, byte pitch, byte velocity)
 *
 *  Called by MIDI parser when note off messages arrive.
 */
void handleNoteOff(byte channel, byte pitch, byte velocity) {
  if (channel == CV_CHAN) {
    last_gates -= 1;
    updateOutputs();
  }
}

/*void handlePitchBend(byte channel, int bend)
 *
 * Called by parser when bend messages arrive.
 */
void handlePitchBend(byte channel, int bend) {
  // Bend data from the parser is 14 bits, signed, centered
  // on 0.
  // unsigned conversion & dual-7-bit thwacking
  // already handled by midi parser

  if (channel == CV_CHAN) {
    last_bend = bend >> 5;
    updateOutputs();
  }
}

/////////////////////////////////////////////////////////////////////////
// On MIDI clock signal
////////////////////////////////////////////////////////////////////////
void timer_callback() {
  if (!clock_sync) return;

  // Tell the mainline loop that time has elapsed
  if (tick_count >= tempo_delay) {
    send_tick = true;
    tick_count = 0;
  }
  tick_count += 1;
}

void start_callback() {
  tick_count = 0;
  clock_sync = true;
}

void continue_callback() {
  clock_sync = true;
}

void stop_callback() {
  clock_sync = false;
}

/* void tick_func()
 *
 * Called by mainline loop when send_tick is true.
 * Keeps track of rising/falling edges, and notifies notetracker
 * of clock status.
 */
void tick_func() {
  if (send_tick) {
    send_tick = false;

    digitalWrite(REDLEDPIN, HIGH);
    digitalWrite(CLOCKPIN, HIGH);
  } else {
    digitalWrite(REDLEDPIN, LOW);
    digitalWrite(CLOCKPIN, LOW);
  }
}

/////////////////////////////////////////////////////////////////////////
// Panel interface control routines
/////////////////////////////////////////////////////////////////////////

/*void check_pots()
 *
 * Read the analog input (tempo control)
 */
void check_pots() {
  uint32_t pot1_val = analogRead(PIN_TEMPO_POT);
  uint32_t pot2_val = analogRead(PIN_OCTAVE_POT);

  octave = map(pot2_val, 0, 1023, 2, -2);
  tempo_delay = round(map(pot1_val, 0, 1023, 16, 0) / 2) * 2;
}

/////////////////////////////////////////////////////////////////////////
// Arduino boilerplate - setup() & loop()
/////////////////////////////////////////////////////////////////////////

void setup() {
  // Output pins
  pinMode(GATEPIN, OUTPUT);
  pinMode(CLOCKPIN, OUTPUT);
  pinMode(REDLEDPIN, OUTPUT);

  digitalWrite(REDLEDPIN, HIGH);

  // Button pins
  pinMode(UPBTNPIN, INPUT_PULLUP);
  pinMode(DNBTNPIN, INPUT_PULLUP);
  pinMode(SHORTBTNPIN, INPUT_PULLUP);

  Wire.begin();

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
  MIDI.setHandleClock(timer_callback);
  MIDI.setHandleStart(start_callback);
  MIDI.setHandleContinue(continue_callback);
  MIDI.setHandleStop (stop_callback);
}


void loop() {
  // Pump the MIDI parser as quickly as we can.
  // This will invoke the callbacks when messages are parsed.
  MIDI.read();

  // check the tempo pot.
  check_pots();
  tick_func();
}
