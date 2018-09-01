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

static const int MAX_NO_CLOCK = 3000;
static const int PULSE_TIME = 15;

int random_notes[] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
bool random_gates[] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
int random_count = 0;
int last_update_index = -1;
int random_notes_length = 16;
int clock_div = 24;
int update_rate = 4;
bool freeze = true;
bool antifreeze = false;
bool last_gate = false;
int last_clock_time = millis();
int clock_count = 0;
int internal_clock_delay = 0;
bool internal_clock = false;
bool seq_mode = false;
bool send_tick = false;

int CV1;
int CV2;
int CV3;
int CV4;

void updateCV(word DC_Value) {
  Wire.beginTransmission(DAC_address);
  Wire.write(byte((DC_Value & 0x0f00) >> 8));
  Wire.write(byte(DC_Value & 0xff));
  Wire.endTransmission();
}

void checkPots() {
  CV1 = analogRead(pot1);
  CV2 = analogRead(pot2);
  CV3 = analogRead(pot3);
  CV4 = analogRead(pot0);

  // Toggle the sequencer mode when all the pots are turned all the way down
  if (CV1 == 0 && CV2 == 0 && CV3 == 0 && CV4 == 0) {
    seq_mode = true;
  } else if (CV1 == 1023 && CV2 == 1023 && CV3 == 1023 && CV4 == 1023) {
    seq_mode = false;
  }

  // 4-step sequencer mode
  if (seq_mode) {
    freeze = false;
    antifreeze = true;
    update_rate = 1;
    random_notes_length = 8;
    clock_div = 24;

    random_notes[0] = CV4;
    random_notes[1] = CV4;
    random_notes[2] = CV1;
    random_notes[3] = CV1;
    random_notes[4] = CV2;
    random_notes[5] = CV2;
    random_notes[6] = CV3;
    random_notes[7] = CV3;

    random_gates[0] = true;
    random_gates[1] = false;
    random_gates[2] = true;
    random_gates[3] = false;
    random_gates[4] = true;
    random_gates[5] = false;
    random_gates[6] = true;
    random_gates[7] = false;
  }
  // Random mode
  else {
    random_notes_length = map(CV1, 0, 1023, 2, 32);
    freeze = CV3 <= 0;
    update_rate = map(CV3, 0, 1023, 5, 1);
    antifreeze = update_rate == 5;
    clock_div = round(map(CV4, 0, 1023, 24, 1) / 2) * 2;
    internal_clock_delay = round(20 * clock_div);
  }
}

int getRandomNote() {
  return random(0, 1023);
}

bool getRandomGate() {
  return random(0, 255) >= 127;
}

int getCurrentIndex() {
  return random_count % random_notes_length;
}

void addRandom() {
  int notes_in_cycle = update_rate * random_notes_length;

  if (!freeze) {
    // Add a single new random note on each update cycle
    if ((random_count % notes_in_cycle) == 0) {
      int update_index = (last_update_index + 1) % random_notes_length;
      last_update_index = update_index;

      bool change_note = random(0, 255) >= 127;

      if (antifreeze || change_note) {
        random_notes[update_index] = getRandomNote();
      }

      if (antifreeze || !change_note) {
        random_gates[update_index] = getRandomGate();
      }
    }
  }

  random_count = (random_count + 1) % notes_in_cycle;
}

void playRandomNote(int index) {
  int cv = random_notes[index];
  updateCV(cv);
}

void playRandom(int index) {
  bool gate = random_gates[index];
  digitalWrite(gate_pin, gate ? HIGH : LOW);

  if (gate && (gate != last_gate)) {
    playRandomNote(index);
  }

  last_gate = gate;
}

void onClock() {
  if (clock_count >= clock_div) {
    send_tick = true;
    clock_count = 0;
  }
  clock_count += 1;
  internal_clock = false;
}

void onTimer() {
  MsTimer2::stop();

  if (internal_clock) {
    MsTimer2::set(internal_clock_delay, onTimer);
    MsTimer2::start();

    send_tick = true;
  }
}

void setup() {
  pinMode(gate_pin, OUTPUT);
  pinMode(clock_out_pin, OUTPUT);
  pinMode(clock_pin, INPUT);

  // Power up the DAC
  pinMode(DAC_vin, OUTPUT);
  pinMode(DAC_ground, OUTPUT);
  digitalWrite(DAC_vin, HIGH);
  digitalWrite(DAC_ground, LOW);

  attachInterrupt(digitalPinToInterrupt(clock_pin), onClock, RISING);

  randomSeed(analogRead(7));
  for (uint8_t i = 0; i < sizeof(random_notes); i++) {
    random_notes[i] = getRandomNote();
    random_gates[i] = getRandomGate();
  }

  Wire.begin();
}

void loop() {
  checkPots();

  if (!send_tick) {
    int elapsed = millis() - last_clock_time;

    if (elapsed >= MAX_NO_CLOCK) {
      internal_clock = true;
      onTimer();
    }

    if (elapsed >= PULSE_TIME) {
      digitalWrite(clock_out_pin, LOW);
    }

    return;
  }

  last_clock_time = millis();
  send_tick = false;

  int index = getCurrentIndex();

  addRandom();

  playRandom(index);

  digitalWrite(clock_out_pin, HIGH);
}
