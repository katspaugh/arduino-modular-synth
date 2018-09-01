// ADSRduino
//
// a simple ADSR for the Arduino
// m0xpd
// Feb 2017
//
// Modified by katspaugh in 2018 to use the Wire library
//
// see http://m0xpd.blogspot.co.uk/2017/02/signal-processing-on-arduino.html
//
// Uses a DAC microchip to produce CV
// receives gate inputs on digital pin 2 (remember to protect e.g. with a 200R resistor and a 5V1 Zener diode)
// and a loop mode input on digital pin 3 (pulling to 0V selects loop mode)
//
// Voltages between 0 and 5V (e.g. from potentiometers) on analog pins A0:A3 control Attack, Decay, Sustain & Release

#include <Wire.h>

// Pin definitions...
const int gatePin = 2;

// Analog pins for the potentiometers
const int pot0 = 6;
const int pot1 = 0;
const int pot2 = 7;
const int pot3 = 1;

// Pins A4 (SDA), A5 (SCL) are reserved for the DAC
const int DAC_vin = A3;    // A2
const int DAC_ground = A2; // A3
const byte DAC_address = 0x60; // The I2C address can vary

float alpha = 0.7;  // this is the pole location ('time constant') used for the first-order difference equation
double alpha1 = 0.9; // initial value for attack
double alpha2 = 0.9; // initial value for decay
double alpha3 = 0.95; // initial value for release

float envelope = 0.0;  // initialise the envelope
float CV0 = 0.0;       // result of reads from potentiometers (yes - it will only be an int, but helps with the casting!)
float CV1 = 0.0;
int CV2 = 0;
float CV3 = 0.0;

int drive = 0;
int sustain_Level = 0;
int scan = 0;
boolean note_active = false;
boolean trigger = false;
boolean decay = false;
boolean release_done = true;


void writeToDAC(int DC_Value) {
  Wire.beginTransmission(DAC_address);
  Wire.write(byte((DC_Value & 0x0f00) >> 8));
  Wire.write(byte(DC_Value & 0xff));
  Wire.endTransmission();
}

void setup() {
  pinMode(gatePin, INPUT);

  // Power up the DAC
  pinMode(DAC_vin, OUTPUT);
  pinMode(DAC_ground, OUTPUT);
  digitalWrite(DAC_vin, HIGH);
  digitalWrite(DAC_ground, LOW);

  Wire.begin();

  Serial.begin(9600);
}

void loop() {
  boolean gate = digitalRead(gatePin);      // read the gate input every time through the loop
  Serial.println(gate);
  update_params(scan);                      // scan only one of the other inputs each pass

  boolean trigger = gate;                   // trigger an ADSR even if there's a gate OR if we're in loop mode
  while (trigger) {
    if (note_active == false) {                 // if a note isn't active and we're triggered, then start one!
      decay = false;
      drive = 4096;                             // drive toward full value
      alpha = alpha1;                           // set 'time constant' alpha1 for attack phase
      note_active = true;                       // set the note_active flag
    }
    if ((decay == false) && (envelope > 4000) && (drive == 4096)) { // if we've reached envelope >4000 with drive= 4096,
      // we must be at the end of attack phase
      // so switch to decay...
      decay = true;                                         // set decay flag
      drive = sustain_Level;                                // drive toward sustain level
      alpha = alpha2;                                       // and set 'time constant' alpha2 for decay phase
    }

    envelope = ((1.0 - alpha) * drive + alpha * envelope); // implement difference equation: y(k) = (1 - alpha) * x(k) + alpha * y(k-1)
    writeToDAC(round(envelope));                   // and output the envelope to the DAC

    gate = digitalRead(gatePin);                    // read the gate pin (remember we're in the while loop)
    trigger = gate;                                 // and re-evaluate the trigger function
  }

  if (note_active == true) {              // this is the start of the release phase
    drive = 0;                            // drive towards zero
    alpha = alpha3;                       // set 'time comnstant' alpha3 for release phase
    note_active = false;                  // turn off note_active flag
    release_done = false;                 // and set release_flag done false
  }

  envelope = ((1.0 - alpha3) * drive + alpha3 * envelope); // implement the difference equation again (outside the while loop)
  writeToDAC(round(envelope));                     // and output envelope
  gate = digitalRead(gatePin);                     // watch out for a new note
  scan += 1;                                       // prepare to look at a new parameter input
  if (envelope < 4) {                              // is the release phase ended?
    release_done = true;                           // yes - so flag it
  }
  if (scan == 4) {                                 // increment the scan pointer
    scan = 0;
  }
}

// read the input parameters
void update_params(int scan) {
  switch (scan) {
    case 0:
      CV0 = analogRead(pot0);                   // get the attack pole location
      alpha1 = 0.999 * cos((1023 - CV0) / 795);
      alpha1 = sqrt(alpha1);
      break;
    case 1:
      CV1 = analogRead(pot1);                   // get the release pole location
      alpha2 = 0.999 * cos((1023 - CV1) / 795);
      alpha2 = sqrt(alpha2);
      break;
    case 2:
      CV2 = analogRead(pot2);                   // get the (integer) sustain level
      sustain_Level = CV2 << 2;
      break;
    case 3:
      CV3 = analogRead(pot3);                   // get the release pole location (potentially closer to 1.0)
      alpha3 = 0.99999 * cos((1023 - CV3) / 795);
      alpha3 = sqrt(alpha3);
      break;
  }
}
