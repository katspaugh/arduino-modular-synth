#include <Wire.h>
#include <MsTimer2.h>

#define pot0          A6
#define pot1          A0
#define pot2          A7
#define pot3          A1
#define gate_pin      13
#define clock_pin     2
#define clock_out_pin 6

// Pins A4 (SDA), A5 (SCL) are reserved for the DAC
#define DAC_vin       A3
#define DAC_ground    A2
#define DAC_address   0x60

#define CLOCK_PPQ     24
#define MAX_NO_CLOCK  3000

int random_notes[] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
int random_count = 0;
int last_update_index = -1;
int random_notes_length = 16;
int tempo = 24;
int offset = 0;
int update_rate = 4;
bool freeze = true;
int clock_count = -1;
bool send_tick = false;

int CV0;
int CV1;
int CV2;
int CV3;

void updateCV(word DC_Value) {
  Wire.beginTransmission(DAC_address);
  Wire.write(byte((DC_Value & 0x0f00) >> 8));
  Wire.write(byte(DC_Value & 0xff));
  Wire.endTransmission();
}

void checkPots() {
  CV0 = analogRead(pot0);
  CV1 = analogRead(pot1);
  CV2 = analogRead(pot2);
  CV3 = analogRead(pot3);

  tempo = map(CV0, 0, 1023, 100, 1);
  random_notes_length = map(CV1, 0, 1023, 2, 16);
  offset = map(CV2, 0, 1023, -3000, 0);
  update_rate = map(CV3, 0, 1023, 5, 1);
  freeze = CV3 <= 5;
}

int getRandomNote() {
  return random(3000, 4095);
}

void addRandom() {
  int notes_in_cycle = update_rate * random_notes_length;

  if (!freeze) {
    // Add a single new random note on each update cycle
    if ((random_count % notes_in_cycle) == 0) {
      int update_index = (last_update_index + 1) % random_notes_length;
      last_update_index = update_index;
      random_notes[update_index] = getRandomNote();
    }
  }

  random_count = (random_count + 1) % notes_in_cycle;
}

void playRandomNote() {
  int index = random_count % random_notes_length;
  int cv = random_notes[index];
  updateCV(cv + offset);
}

void onTimer() {
  MsTimer2::stop();
  MsTimer2::set(tempo, onTimer);
  MsTimer2::start();

  send_tick = true;
}

void setup() {
  pinMode(gate_pin, OUTPUT);
  pinMode(clock_out_pin, OUTPUT);
  pinMode(clock_pin, INPUT);

  randomSeed(analogRead(DAC_vin));

  // Power up the DAC
  pinMode(DAC_vin, OUTPUT);
  pinMode(DAC_ground, OUTPUT);
  digitalWrite(DAC_vin, HIGH);
  digitalWrite(DAC_ground, LOW);

  for (uint8_t i = 0; i < sizeof(random_notes); i++) {
    random_notes[i] = getRandomNote();
  }

  Wire.begin();
  
  checkPots();
  onTimer();
}

void loop() {
  if (!send_tick) return;
  send_tick = false;

  clock_count += 1;

  digitalWrite(clock_out_pin, (clock_count % 2) == 0);

  if (clock_count >= CLOCK_PPQ || digitalRead(clock_pin)) {
    clock_count = 0;

    checkPots();
    addRandom();
    playRandomNote();
    digitalWrite(gate_pin, HIGH);
  } else {
    digitalWrite(gate_pin, LOW);
  }
}
