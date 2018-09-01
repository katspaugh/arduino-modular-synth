/******************************************************************************
notemap.h

Ancillary classes to MIDI-CV intervase (MIDI-CV.ino)
Byron Jacquot, SparkFun Electronics
October 5, 2015
https://github.com/sparkfun/MIDI_Shield/tree/V_1.5/Firmware/MIDI-CV

This file contains two classes.

The first (notemap) is a simple bitmap that's used to track note on and off events.  It's an 
array of 128 bits, corresponding the the 128 possible key numbers in MIDI.  Note ons
set bits in the map, note offs remove them.  Other routines can check bits, or find the lowest bit set.
Outside code probably won't interface with this class directly.

The second class (notetracker) manages a couple instances of notemap.  The main sketch 
passes note on/off, sustain pedal, arpeggiator clock ticks, etc to it.
It keeps track of which note CV should be generated, and whether the gate should be on.

Resources:

Development environment specifics:
    It was developed for the Arduino Uno compatible SparkFun RedBoard, with a  SparkFun
    MIDI Shield and a pair of Microchip MCP4725 DACs to generate the control voltages.
    
    Written, compiled and loaded with Arduino 1.6.5

This code is released under the [MIT License](http://opensource.org/licenses/MIT).

Please review the LICENSE.md file included with this example. If you have any questions 
or concerns with licensing, please contact techsupport@sparkfun.com.

Distributed as-is; no warranty is given.
******************************************************************************/

#pragma once

#include <Arduino.h>
#include <stdint.h>


/*
 * class notemap
 *  
 * Keeps track of which keys are being held.
 * 
 * Represents the full set of 128 MIDI keys.  If you want to constrain that,
 * you can do it by limiting input to the class, or filtering it's output.
 */
class notemap
{
  public:

         notemap();

    void debug();

    void setBit(uint8_t note);
    void clearBit(uint8_t note);
    void clearAll();

    bool isBitSet(uint8_t note);

    uint8_t getNumBits();
    uint8_t getLowest();

  private:

    uint8_t keys[16];
    uint8_t numKeys;
};

/*
 * Class notetracker
 * 
 * Answers the question: what CV should we be producing.
 * 
 * Inputs are note on/off, sustain pedal events, key presses, apreggiator clocks.
 * Outputs are note CV value and gate.
 * Uses two instances of notemap to track keys (second instance is used when 
 * sustain pedal is held.), plus some logic to implement low-voice priority
 * and an arpeggiator.
 * 
 * Represents the full set of 128 MIDI keys.  If you want to constrain that,
 * you can do it by limiting input to the class, or filtering it's output.
 * 
 * Read the annotations on the indivudual functions for a better understanding
 * of their uses and interactions.  
 */
class notetracker
{
  public:
    notetracker();
  
    enum tracker_mode {NORMAL, ARP_UP, ARP_DN};

    void                  noteOn (uint8_t key);
    void                  noteOff(uint8_t key);
    
    void                  setMode    (notetracker::tracker_mode m);
    notetracker::tracker_mode getMode();

    void                  setShort(bool short_on);
    bool                  getShort();

    uint8_t               getNext(uint8_t start);

    void                  setSustain(bool);

    void                  tickArp(bool);

    uint8_t               whichKey();

    bool                  getGate();
    
  private:

    tracker_mode mode;
    
    bool     staccato;
    bool     sustaining;
    bool     clk_hi;

    notemap voice_map;
    notemap sustain_map;
    notemap * active_map_p;
    
    uint8_t last_key;
};


