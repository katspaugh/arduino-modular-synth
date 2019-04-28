#include <MsTimer2.h>

static const int CLOCK_PIN = 3;
static const int CV_PIN = A0;
static const int OUT1 = 12;
static const int OUT2 = 11;
static const int OUT3 = 10;
static const int OUT4 = 9;
static const int OUT5 = 8;
static const int OUT6 = 7;

static const int FACTOR = 12;

static bool clock_tick = false;
static bool timer_tick = false;
int now = millis();
int last_time = now - 1;
int rate = 1;
int prev_rate = 1;
int timer_count = 0;

void onClock() {
  clock_tick = true;
  now = millis();
  rate = ceil((now - last_time) / FACTOR);
  if (rate < 1) rate = 1;
  if (abs(prev_rate - rate) <= 1) {
    rate = prev_rate;
  } else {
    prev_rate = rate;
    resetTimer();
    onTimer();
  }
  last_time = now;
}

void onTimer() {
  timer_tick = true;
  timer_count += 1;
  if (timer_count >= FACTOR) {
    timer_count = 0;
  }
}

void resetTimer() {
  timer_count = 0;
  MsTimer2::stop();
  MsTimer2::set(rate, onTimer);
  MsTimer2::start();
}

void setup() {
  pinMode(CLOCK_PIN, INPUT);
  pinMode(OUT1, OUTPUT);
  pinMode(OUT2, OUTPUT);
  pinMode(OUT3, OUTPUT);
  pinMode(OUT4, OUTPUT);
  pinMode(OUT5, OUTPUT);
  pinMode(OUT6, OUTPUT);

  attachInterrupt(digitalPinToInterrupt(CLOCK_PIN), onClock, RISING);

  randomSeed(analogRead(7));
}

void loop() {
  if (clock_tick) {
    clock_tick = false;
  }

  if (timer_tick) {
    digitalWrite(OUT1, timer_count == 0);
    digitalWrite(OUT2, timer_count % 6 == 0);
    digitalWrite(OUT3, timer_count % 4 == 0);
    digitalWrite(OUT4, timer_count % 3 == 0);
    digitalWrite(OUT5, timer_count % 2 == 0);
    digitalWrite(OUT6, random(0, 2));
  }
}
