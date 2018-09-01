/******************************************************************************
notemap.cpp

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

#include "notemap.h"

/* notemap::notemap()
 *  
 * Construct an empty notemap. 
 */
notemap::notemap()
{
    for(uint8_t i = 0; i < 16; i++)
    {
      keys[i] = 0;
    }
    numKeys = 0;
}  

/* void notemap::debug()
 *  
 *  Display the contents of a notemap
 */
void notemap::debug()
{
  Serial.print("Notemap: numKeys: ");  
  Serial.print(numKeys);
  Serial.print(" Array: ");
  for (uint8_t i = 0; i < 16; i++)
  {
    Serial.print(keys[i], HEX);
    Serial.print(' ');
  }
  Serial.println();
}

/* void notemap::setBit(uint8_t note)
 *  
 *  Set a bit in a notemap.
 *  Usually corresponds to note-on commands
 */
void notemap::setBit(uint8_t note)
{
  uint8_t idx = note / 8;
  uint8_t pos = (note % 8) ;

  keys[idx] |= (0x01 << pos);

  numKeys ++;
}

/* void notemap::clearBit(uint8_t note)
 *  
 *  Clear a bit in a notemap.
 *  Usually corresponds to note-off commands
 */
void notemap::clearBit(uint8_t note)
{
  uint8_t idx = note / 8;
  uint8_t pos = (note % 8) ;

  keys[idx] &= ~(0x01 << pos);

  numKeys --;
}

/* void notemap::clearAll(uint8_t note)
 *  
 *  Empty out a notemap.
 *  Mainly used when sustain pedal is released.
 */
void notemap::clearAll()
{
    for(uint8_t i = 0; i < 16; i++)
    {
      keys[i] = 0;
    }
    numKeys = 0;
}
  
/* void notemap::isBitSet(uint8_t note)
 *  
 *  Check a bit in a notemap, return it's status.
 */
bool notemap::isBitSet(uint8_t note)
{  
  uint8_t idx = note / 8;
  uint8_t pos = (note % 8) ;

  return( keys[idx] & (0x01 << pos));
}

/* void notemap::getLowest(uint8_t note)
 *  
 *  Find the lowest bit set in the bitmap and return it's index.
 *  Used when we want to determine which not to play.
 *  
 *  Implements low-note priority.  
 *  TBD: Could reverse the search for high-note priority.
 */
uint8_t notemap::getLowest()
{
 //Serial.print("Checkmap:");
  //mapDebug();

  uint8_t keynum;

  // starting at bottom gives us low note priority.
  // could alternately start from the top...
  for (uint8_t i = 0; i < 16; i++)
  {
    if (keys[i] != 0x0)
    {
      // find the lowest bit set
      uint8_t j, k;
      for (j = 0x1, k = 0; k < 8; j <<= 1, k++)
      {
#if 0
        Serial.print("j: ");
        Serial.print(j);
        Serial.print("k: ");
        Serial.println(k);
#endif
        if (keys[i] & j)
        {
          keynum = (i * 8) + k ;

          return keynum;
        }
      }
    }
  }

  return 0;
}

/* void notemap::getNumBits()
 *  
 *  Tell te caller how many bits have been set in the map.
 */
uint8_t notemap::getNumBits()
{
  return numKeys;  
}


/////////////////////////////////////////////////////////////////////////////////////////////

/* notetracker::notetracker()
 *  
 *  Notetracker class constructor.
 *  Build an empty notrtracker.
 */
notetracker::notetracker()
{
    mode = NORMAL;
    
    staccato   = false;
    sustaining = false;
    clk_hi     = false;

    active_map_p = &voice_map;
    
    last_key = 0;
}

/* void notetracker::noteOn(uint8_t key)
 *  
 *  Accept a note on event.
 *  Always set bits in the main bitmap.
 *  If we're sustaining, add the bits to the sustain bitmap, too.
 */
void notetracker::noteOn(uint8_t key)
{
  voice_map.setBit(key);

  if(sustaining)
  {
    sustain_map.setBit(key);
  }
}

/* void notetracker::noteOff(uint8_t key)
 *  
 *  Accept a note off event.
 *  Remove bits from the main bitmap.
 *  Don't touch the suystain bitmap.
 *  If we're sustaining, note ons get held until pedal is released,
 *  even if the keys are released.
 */
void notetracker::noteOff(uint8_t key)
{
  voice_map.clearBit(key);
}

/*void notetracker::setMode(notetracker::tracker_mode m)
 * 
 * Set an arpeggiator mode - normal, up or down
 */
void notetracker::setMode(notetracker::tracker_mode m)
{
  mode = m;
}

/* notetracker::tracker_mode notetracker::getMode()
 * 
 * Return present arpeggiator mode - normal, up or down
 */
notetracker::tracker_mode notetracker::getMode()
{
  return mode;
}

/* void notetracker::setShort(bool short_on)
 *  
 * Change present staccato setting. 
 */
void notetracker::setShort(bool short_on)
{
  staccato = short_on;
}

/* bool notetracker::getShort()
 *  
 *  return present staccato setting
 */
bool notetracker::getShort()
{
  return staccato;
}

/* void notetracker::setSustain(bool on)
 *  
 *  Turn sustain on/off.
 *  
 *  Sustain is the trickiest piece of this.
 *  When sustain pedal is pressed, we make a copy of the notemap, and use it
 *  instead of the regular one.
 *  The sustaining bitmap accumulates all new note ons, but is only pueged when the pedal
 *  is let up.
 *  
 *  The regular bitmap continues to update with both note on and offs.
 *  
 *  When the pedal is released, the sustain bitmap is purged, and the
 *  regular bitmap takes over, knowing which
 *  keys are presently being held. 
 */
void notetracker::setSustain(bool on)
{
#if 0
  Serial.print("Sustain: ");
  Serial.println(sustaining);
#endif

  if(on)
  {
    sustaining = true;

    //This gives us a member-by-member copy of bitmap object
    sustain_map = voice_map;

    active_map_p = &sustain_map;
    
  }
  else
  {
    sustaining = false;

    sustain_map.clearAll();

    active_map_p = &voice_map;
  }

}

/* uint8_t notetracker::getNext(uint8_t start)
 *  
 *  Called when the arpeggiator is active and the clock advances.
 *  It hunts the active bitmap in the approptiate direction (up/dn),
 *  and finds the next note.
 *  
 *  While it's similar to getLowest, it makes more sense as part of the 
 *  notetracker (rather than notemap), because the arpeggiator info is all in the 
 *  notetracker - notemap has no knowledge of arpeggiation.
 */
uint8_t notetracker::getNext(uint8_t start)
{
  int8_t step;

  if(mode == ARP_UP)
  {
    step = 1;
  }
  else if(mode == ARP_DN)
  {
    step = -1;
  }

  for(uint8_t key = start+step;  key != start; key = (key+step)%128)
  {
    if(active_map_p->isBitSet(key))
    {
      return key;
    }
  }
  return start;  
}

/* void notetracker::tickArp(bool rising)
 *  
 *  Called when the apreggiator clock sees a rising or falling event.
 *  
 *  The arp clock runs at twice the desired frequency, so we can have 
 *  50% duty cycle for LED blinking, and staccato mode.  The clock notifies
 *  us of both rising and falling edges.
 *  
 *  Rising edges cause us to hunt for the next note in the active map.
 *  Falling edges simply set the clk_hi to false, which interrupts the gate output
 *  in staccato mode.
 */
void notetracker::tickArp(bool rising)
{
  if(rising)
  {  
    clk_hi = true;
    
    if(active_map_p->getNumBits() > 1)
    {
      last_key = getNext(last_key);
    }
  }
  else // falling
  {
    clk_hi = false;
  }
}

/* uint8_t notetracker::whichKey()
 *  
 *  Which key should be used to generate the current CV?
 *  
 *  If no keys are held, return the last value we sent.
 *  If apreggiator is off, just return the lowest bit in the active map.
 *  
 *  If arpeggiator is on:
 *  If only one key is held, return that key.
 *  If multiple keys are held, return last_key.
 *  Last_key will be updated by arpeggiator clock
 *  
 */
uint8_t notetracker::whichKey()
{
  uint8_t num_keys = active_map_p->getNumBits();
  
  if(num_keys == 0)
  {
    return last_key;
  }
  
  switch(mode)
  {
    case ARP_UP:
    {
      // When there's only one key held, we need to respond to it 
      // immediately.
      // If multiple keys are held, advanceArp() sets last_key
      // based on timer. 
      if(num_keys == 1)
      {
        last_key = active_map_p->getLowest();
      }
    }        
    break;
    case ARP_DN:
    {
      if(num_keys == 1)
      {
        last_key = active_map_p->getLowest();
      }
    }        
    break;
    case NORMAL:
    default:
    {
       last_key = active_map_p->getLowest();
    }        
    break;
  }

  return last_key;
  
}

/* bool notetracker::getGate()
 *  
 *  Return whether we should be driving the gatr signal or not.
 *  
 *  In legato mode, we drive the gate is any keys are presently held/sustained.
 *  
 *  In staccato mode, the gate is the combination of keys held and the 
 *  arpeggiator clock.  It seemed more fun to have staccato mode
 *  independent of the arpeggio mode...if you don't like that,
 *  then change it here.
 */
bool notetracker::getGate()
{
  if(staccato)
  {
    return clk_hi & ((active_map_p->getNumBits())> 0);
  }
  else
  {
    return ((active_map_p->getNumBits()) > 0);
  }
}
    

