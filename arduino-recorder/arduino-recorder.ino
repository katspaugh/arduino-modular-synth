#include <Wire.h>
#include <MsTimer2.h>

#define cv0           A0
#define pot1          A1
#define pot2          A2
#define pot3          A3
#define gate_pin      11
#define led_pin       13
#define trig_pin      2

// Pins A4 (SDA), A5 (SCL) are reserved for the DAC
#define DAC_address   0x60
#define MAX_SIZE      900

int sample_rate = 5;     // 5 ms
word cvs[MAX_SIZE] = {}; // the CV buffer
int max_len = MAX_SIZE;

int CV0;
int CV1;
int CV2;
int CV3;

int i = 0;
int len = 120;
word current_cv = 0;
bool start_rec = false;
bool send_tick = false;
bool internal_timer = false;

void updateCV(word DC_Value) {
  digitalWrite(gate_pin, DC_Value >= 2048);

  Wire.beginTransmission(DAC_address);
  Wire.write(byte((DC_Value & 0x0f00) >> 8));
  Wire.write(byte(DC_Value & 0xff));
  Wire.endTransmission();
}

void checkPots() {
  CV0 = analogRead(cv0);
  CV1 = analogRead(pot1);
  CV2 = analogRead(pot2);
  CV3 = analogRead(pot3);
}

void onTimer() {
  if (internal_timer) {
    send_tick = true;
  }
}

void onTrigger() {
  send_tick = true;
}

void resetTimer() {
  MsTimer2::stop();
  MsTimer2::set(sample_rate, onTimer);
  MsTimer2::start();
}

void stopTimer() {
  MsTimer2::stop();
}

void setup() {
  pinMode(led_pin, OUTPUT);
  pinMode(gate_pin, OUTPUT);
  pinMode(trig_pin, INPUT);

  attachInterrupt(digitalPinToInterrupt(trig_pin), onTrigger, RISING);

  Wire.begin();

  digitalWrite(led_pin, LOW);
}

void loop() {
  checkPots();

  bool new_internal_timer = CV2 < 1023;
  if (internal_timer != new_internal_timer) {
    internal_timer = new_internal_timer;
    internal_timer ? resetTimer() : stopTimer();
  }

  if (internal_timer) {
    int new_sample_rate = map(CV2, 0, 1023, 1, 100);
    if (sample_rate != new_sample_rate) {
      sample_rate = new_sample_rate;
      resetTimer();
    }
  }

  if (!send_tick) return;
  send_tick = false;

  // Recording CV
  if (CV1 >= 10) {
    if (!start_rec) {
      start_rec = true;
      i = 0;
    }
    current_cv = map(CV0, 0, 1023, 0, 4095);
    cvs[i] = current_cv;
    i = (i + 1) % MAX_SIZE;
    len = i;

    updateCV(current_cv);

    // Blink when the loop restarts
    digitalWrite(led_pin, i > 25);

    return;
  }

  // Playing back
  if (start_rec) {
    start_rec = false;
    digitalWrite(led_pin, LOW);
  }

  max_len = map(CV3, 0, 1023, 1, len);

  current_cv = cvs[i];
  i += 1;
  if (i >= len || i >= max_len) i = 0;

  updateCV(current_cv);
}
