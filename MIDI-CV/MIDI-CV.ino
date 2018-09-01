/******************************************************************************
MIDI-CV.ino
MIDI to Control Voltage converter for synthesizers.
Byron Jacquot, SparkFun Electronics
October 2, 2015
https://github.com/sparkfun/MIDI_Shield/tree/V_1.5/Firmware/MIDI-CV

This functions as a MIDI-to-control voltage interface for analog synthesizers.
It was developed for the Moog Werkstatt WS-01, but should work with any volt-per-octave
synthesizer.

Resources:
  This code is dependent on the FortySevenEffects MIDI library for Arduino.
  https://github.com/FortySevenEffects/arduino_midi_library
  This was done using version 4.2, hash fb693e724508cb8a473fa0bf1915101134206c34
  This library is now under the MIT license, as well.
  You'll need to install that library into the Arduino IDE before compiling.

  It is also dependent on the notemap class, stored in the same repository (notemap.cpp/h).

Development environment specifics:
  It was developed for the Arduino Uno compatible SparkFun RedBoard, with a  SparkFun
  MIDI Shield and a pair of Microchip MCP4725 DACs to generate the control voltages.

  Written, compiled and loaded with Arduino 1.6.5

This code is released under the [MIT License](http://opensource.org/licenses/MIT).

Please review the LICENSE.md file included with this example. If you have any questions
or concerns with licensing, please contact techsupport@sparkfun.com.

Distributed as-is; no warranty is given.
******************************************************************************/

#include <MIDI.h>
#include <Wire.h>

#include "notemap.h"

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
static const int DNBTNPIN  = 3;
static const int SHORTBTNPIN= 4;
static const int BTN_DEBOUNCE = 50;

// global variables
//
// notetracker knows which note ons & offs we have seen.
// We refer to it when it's time to generate CV and gate signals,
static notetracker themap;

// Variables for arpeggiator clock.
static int tempo_delay = 0;
static int octave = 0;
static int tick_count = 0;
static bool send_tick = false;
static bool clock_sync = true;

// The last bend records the most recently seen bend message.
// We need to keep track so we can update note CV when we get new notes,
// or new bend messages - we need the other half in order to put them together.
// bend is signed, 14-bit
static int16_t last_bend = 0;

// constants to describe the MIDI input.
// NUM_KEYS is the number of keys we're interpreting
// BASE_KEY is the offset of the lowest key number
static const int8_t NUM_KEYS = 49;
static const int8_t BASE_KEY = 36;

// The tuning constant - representing the DAC conts per semitone
//
// Arrived at using: (((2^<DAC resolution>)/5)/12) * 100
//
// 2^12 = 4096 total DAC counts.
// 4096/5 = 819.2 DAC counts per volt on a 5V supply
// 819.2/12 = dac counts per semitone = 68.26
// times 100 for some extra calculation precision = 6826
//static const uint32_t DAC_CAL = 6826;
static const uint32_t DAC_CAL = 6987;

// MIDI interface
MIDI_CREATE_INSTANCE(HardwareSerial, Serial, MIDI);

/*
 *  void updateCV(uint8_t key)
 *
 *Converts key number to DAC count value,
 *and sends the value tio the DAC
 */
void updateCV(uint8_t key) {
  uint32_t val = 400ul + ((key * DAC_CAL)/100ul);

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
  uint8_t key;

  key = themap.whichKey();

  // key is in terms of MIDI note number.
  // Constraining the key number to 4 octave range
  // Soc we can do 4 Volts +/- ~0.5V bend range.
  if (key < BASE_KEY) {
    key = 0;
  }
  else if ( key > BASE_KEY + NUM_KEYS) {
    key = NUM_KEYS;
  }
  else
  {
    key -= BASE_KEY;
  }

  updateCV(round(key + octave * 12));

  digitalWrite(GATEPIN, themap.getGate());
}

/////////////////////////////////////////////////////////////////////////
// Callbacks for the MIDI parser
/////////////////////////////////////////////////////////////////////////

/* void handleNoteOn(byte channel, byte pitch, byte velocity)
 *
 *  Called by MIDI parser when note on messages arrive.
 */
void handleNoteOn(byte channel, byte pitch, byte velocity) {
  // Do whatever you want when a note is pressed.
  // Try to keep your callbacks short (no delays ect)
  // otherwise it would slow down the loop() and have a bad impact
  // on real-time performance.
  themap.noteOn(pitch);

  updateOutputs();

}

/* void handleNoteOff(byte channel, byte pitch, byte velocity)
 *
 *  Called by MIDI parser when note off messages arrive.
 */
void handleNoteOff(byte channel, byte pitch, byte velocity) {
  themap.noteOff(pitch);

  updateOutputs();
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

  last_bend = bend >> 5;

  updateOutputs();
}

/* void handleCC(byte channel, byte number, byte value)
 *
 *  Called by parser when continuous controller message arrive
 */
void handleCC(byte channel, byte number, byte value) {
  switch (number) {
    case 1: { // Mod wheel

      Wire.beginTransmission(0x61);
      // Turn 7 bits into 12
      Wire.write(byte((value & 0x70) >> 3));
      Wire.write(byte((value & 0x0f) << 4));
      Wire.endTransmission();
    };
    break;
    case 64: { // sustain pedal
      themap.setSustain( (value != 0) );
    }

    // Other CC's would line up here...

    default:
      break;
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
  //clock_sync = false;
}

/* void tick_func()
 *
 * Called by mainline loop when send_tick is true.
 * Keeps track of rising/falling edges, and notifies notetracker
 * of clock status.
 */
void tick_func() {
  static uint8_t counter = 0;  
  counter++;

  if (counter & 0x01) {
    digitalWrite(REDLEDPIN, HIGH);

    themap.tickArp(false);
    updateOutputs();
  } else {
    digitalWrite(REDLEDPIN, LOW);

    themap.tickArp(true);
    updateOutputs();
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

  octave = map(pot2_val, 0, 1023, 4, 0);
  tempo_delay = round(map(pot1_val, 0, 1023, 16, 0) / 2) * 2;
}

/* void up_btn_func()
 *
 *  Called when button reader detects arpeggio up button has been pressed.
 *
 *  If not in up mode, turn on up mode.
 *  If in up mode, stops arpeggiator.
 */
void up_btn_func() {
  if(themap.getMode() == notetracker::ARP_UP) {
    themap.setMode(notetracker::NORMAL);
  }
  else {
    themap.setMode(notetracker::ARP_UP);
  }
}

/* void dn_btn_func()
 *
 *  Called when button reader detects arpeggio up button has been pressed.
 *
 *  If not in dn mode, turn on up mode.
 *  If in dn mode, stops arpeggiator.
 */
void dn_btn_func() {
  if (themap.getMode() == notetracker::ARP_DN) {
    themap.setMode(notetracker::NORMAL);
  }
  else {
    themap.setMode(notetracker::ARP_DN);
  }
}

/* void short_btn_func()
 *
 *  Toggles staccato articulations
 *  Works independently of arpeggiator enablement.
 */
void short_btn_func() {
  if (themap.getShort()) {
    themap.setShort(false);
  }
  else {
    themap.setShort(true);
  }
}

/* Button behavior function array for ease of referencing using index
 *
 */
void(*func_array[3])(void) =
{
  up_btn_func,
  dn_btn_func,
  short_btn_func
};

/* void check_buttons()
 *
 *  Implements debounced button polling for the 3 buttons on then MIDI shield.
 *
 *  Buttons are active low, pulled up.
 *  On a poll cycle, reads each button.
 *  If button is pressed (LOW), it counts up.
 *  If button is held for enough cycles, it calls the indirect routine for the button.
 *  When button is released, debounce counter clears.
 */
void check_buttons() {
  static uint8_t deb_array[3];
  uint8_t val;

  for (uint8_t i = 0; i < 3; i++) {
    // active low, pulled high
    val = digitalRead(i + UPBTNPIN);

    if (val == LOW) {
      if (deb_array[i] < BTN_DEBOUNCE+1) {
        deb_array[i]++;

        if(deb_array[i] == BTN_DEBOUNCE)
        {
          (*func_array[i])();
        }
      }
    }
    else {
      deb_array[i] = 0;
    }
  }
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
  //MIDI.begin(MIDI_CHANNEL_OMNI);

  MIDI.begin(1);

  // .begin sets the thru mode to on, so we'll have to turn it off if we don't want echo
  //MIDI.turnThruOff();

  // so it is called upon reception of a NoteOn.
  MIDI.setHandleNoteOn(handleNoteOn);  // Put only the name of the function
  // Do the same for NoteOffs
  MIDI.setHandleNoteOff(handleNoteOff);
  //MIDI.setHandleControlChange(handleCC);
  MIDI.setHandlePitchBend(handlePitchBend);

  // Clock callback
  MIDI.setHandleClock(timer_callback);
  MIDI.setHandleStart(start_callback);
  MIDI.setHandleContinue(continue_callback);
  MIDI.setHandleStop (stop_callback);

  themap.setMode(notetracker::ARP_UP);
  themap.setShort(true);
}


void loop() {
  // Pump the MIDI parser as quickly as we can.
  // This will invoke the callbacks when messages are parsed.
  MIDI.read();

  // check the tempo pot.
  check_pots();
  // check panel buttons
  check_buttons();

  if (send_tick) {
    digitalWrite(CLOCKPIN, HIGH);

    tick_func();
    send_tick = false;
  } else {
    digitalWrite(CLOCKPIN, LOW);
  }
}
