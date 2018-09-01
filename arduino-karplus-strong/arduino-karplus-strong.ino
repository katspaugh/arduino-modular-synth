#include <Wire.h>

// calibration
#define VOCT_SCALE         1.0
#define KNOB_SCALE         1.0
#define BASE_OCTAVE        8

// Functional assignments to Arduino pin numbers.
// Digital outputs
static const int LEDPIN = 13;

// setup
//#define PWM_PIN            9
#define GATE               2
#define VOCT               A0
#define TUNE_KNOB          A1
#define LOWER_KNOB         A2
#define KNOB_RANGE         1024
#define OCT_RANGE          10


// stuff used for smoothing
#define NB_SMOOTHING       5
float readings[NB_SMOOTHING];
int readIndex = 0;
float total = 0;
float average = 0;

float freq;
float volt;
bool is_sustaining = false;
int sustain_length = 4000;
int sustain_count = 0;
bool trigger_value = false;
bool last_trigger_value = false;
int last_led_value = 0;

const byte DAC_address = 0x60; // The I2C address can vary

// The tuning constant - representing the DAC conts per semitone
//
// Arrived at using: (((2^<DAC resolution>)/5)/12) * 100
//
// 2^12 = 4096 total DAC counts.
// 4096/5 = 819.2 DAC counts per volt on a 5V supply
// 819.2/12 = dac counts per semitone = 68.26
// times 100 for some extra calculation precision = 6826
static const uint32_t DAC_CAL = 6826;

/**
 * https://gist.github.com/joefutrelle/e127f9dcd8e66a21808d
 *
 * Karplus-Strong plucked string algorithm
 * http://en.wikipedia.org/wiki/Karplus%E2%80%93Strong_string_synthesis
 */
// using signed sample type
#define SAMPLE_T int16_t
// zero is 0xFFF >> 1 in unsigned representation
#define ZERO 2047

// sample buffer
#define BUF_SIZE 64
SAMPLE_T wav[BUF_SIZE];
// minimum wavelength
#define MIN_WL 5
// noise burst length
#define NOISE_L 24

size_t ks_wptr; // position in wavetable
SAMPLE_T ks_prev; // previous sample

size_t wl;

/* 
 *  void updateCV(uint8_t key)
 *  
 *Converts key number to DAC count value,
 *and sends the value tio the DAC
 */ 
void updateCV(uint32_t DC_Value) {  
  Wire.beginTransmission(DAC_address);
  Wire.write(byte((DC_Value & 0x0f00) >> 8));
  Wire.write(byte(DC_Value & 0xff));
  Wire.endTransmission();
}

void ks_init(size_t wl) {
  // initialize wavetable to short noise burst
  size_t i;
  for(i = 0; i < wl; i++) {
    if(i < NOISE_L) { // short burst of noise
      wav[i] = (SAMPLE_T)(rand() & 0x0FFF) - ZERO;
    } else {
      wav[i] = 0;
    }
  }
  // initialize wavetable pointer
  ks_wptr = 0;
  // initialize filter
  SAMPLE_T ks_prev = wav[wl-1];
}

SAMPLE_T ks_iter(size_t wl) {
  // retrieve sample value from buffer
  SAMPLE_T v = wav[ks_wptr];
  // now filter the sample
  wav[ks_wptr] = (ks_prev / 2) + (v / 2);
  ks_prev = v;
  // recur
  ks_wptr = (ks_wptr + 1) % wl;
  // return output sample
  return v;
}

float smooth(float value_to_smooth){
  total = total - readings[readIndex];
  readings[readIndex] = value_to_smooth;
  total = total + readings[readIndex];
  readIndex = readIndex + 1;
  if (readIndex >= NB_SMOOTHING) {
    readIndex = 0;
  }
  average = total / NB_SMOOTHING;
  return average;
}

float volt2freq(float volt){
  return 440 / pow(2, 4.75) * pow(2, min(volt, OCT_RANGE) + BASE_OCTAVE); 
}

float quantize(float volt){
  return round(volt * 12) / 12.0;
}

void setup() {
  pinMode(LEDPIN, OUTPUT);
  pinMode(GATE, INPUT);

  Wire.begin();
}

void loop() {
  trigger_value = digitalRead(GATE) == HIGH;
  sustain_length = (4095.0 * analogRead(LOWER_KNOB)/KNOB_RANGE*KNOB_SCALE);

  if (is_sustaining && sustain_count < sustain_length) {
    updateCV(ks_iter(wl) + ZERO);
    sustain_count = (sustain_count + 1) % 100000;
  } else {
    is_sustaining = false;
  }

  if (last_trigger_value == trigger_value) {
    if (trigger_value) {
      last_led_value -= 1;
      if (last_led_value <= 0) {
        digitalWrite(LEDPIN, LOW);
      }
    }
    return;
  }

  last_trigger_value = trigger_value;

  if (!trigger_value) {
    last_led_value = 0;
    digitalWrite(LEDPIN, LOW);
    return;
  }

  volt = (1.0 * analogRead(TUNE_KNOB)/KNOB_RANGE*KNOB_SCALE) + (analogRead(VOCT)/204.0*VOCT_SCALE);
  volt = smooth(volt);
  volt = quantize(volt);

  // silence
  updateCV(ZERO);
  // choose wavelength
  wl = (round(volt2freq(volt)) % (BUF_SIZE - MIN_WL)) + MIN_WL;
  // initialize Karplus-Strong algorithm with this wavelength
  ks_init(wl);

  is_sustaining = true;
  sustain_count = 0;

  last_led_value = 30;
  digitalWrite(LEDPIN, HIGH);
}
