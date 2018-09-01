#include <MsTimer2.h>

#define CLOCK_SWITCH  2
#define CLOCK_IN      3
#define CLOCK_OUT_1   7
#define CLOCK_OUT_2   8
#define CLOCK_OUT_3   9
#define CLOCK_OUT_4   11
#define LED           13

#define TEMPO_DELAY   20

bool externalClock = false;
bool timer = false;
bool sendTick = false;
int clockCount = -1;

void onClockSwitch() {
  // The switch is grounded when a jack is inserted
  externalClock = digitalRead(CLOCK_SWITCH) == LOW;
}

void onClock() {
  sendTick = true;
}

void onTimer() {
  if (!timer) return;
  sendTick = true;
}

void setup() {
  pinMode(CLOCK_IN, INPUT_PULLUP);
  pinMode(CLOCK_SWITCH, INPUT);
  pinMode(CLOCK_OUT_1, OUTPUT);
  pinMode(CLOCK_OUT_2, OUTPUT);
  pinMode(CLOCK_OUT_3, OUTPUT);
  pinMode(CLOCK_OUT_4, OUTPUT);

  pinMode(LED, OUTPUT);
  digitalWrite(LED, LOW);

  attachInterrupt(digitalPinToInterrupt(CLOCK_IN), onClock, CHANGE);
  attachInterrupt(digitalPinToInterrupt(CLOCK_SWITCH), onClockSwitch, CHANGE);

  randomSeed(analogRead(7));

  onClockSwitch();
}

void loop() {
  if (timer && externalClock) {
    timer = false;
    MsTimer2::stop();
  } else if (!timer && !externalClock) {
    timer = true;
    MsTimer2::set(TEMPO_DELAY, onTimer);
    MsTimer2::start();
  }

  if (!sendTick) return;

  sendTick = false;

  clockCount += 1;
  if (clockCount > 24) clockCount = 0;

  bool n2 = clockCount % 2 == 0;
  bool n3 = !n2 && clockCount % 3 == 0;
  bool n4 = n2 && clockCount % 4 == 0;

  digitalWrite(CLOCK_OUT_1, n2);
  digitalWrite(CLOCK_OUT_2, n3);
  digitalWrite(CLOCK_OUT_3, n4);
  digitalWrite(CLOCK_OUT_4, random(0, 1023) < 250);
}

