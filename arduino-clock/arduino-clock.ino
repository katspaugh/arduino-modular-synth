#define RESET_IN           2
#define CLOCK_IN           3
#define OUT1               9
#define OUT2               10
#define OUT3               11
#define OUT4               12
#define RESET_POT          (A7)

const int max_outs = 4;
const int max_count = 128;
const int outs[max_outs] = { OUT1, OUT2, OUT3, OUT4 };
const int pots[max_outs] = { A6, A5, A4, A3 };
const int divisions[max_outs] = { 2, 4, 8, 16 };

bool send_tick = false;
bool on = false;
int count = -1;
int reset_count = -1;
int autoreset = 16;
int i = 0;

void onClock() {
  on = !on;
  send_tick = true;
}

void onReset() {
  count = -1;
}

void setup() {
  pinMode(CLOCK_IN, INPUT);

  for (i = 0; i < max_outs; i++) {
    pinMode(outs[i], OUTPUT);
  }

  attachInterrupt(digitalPinToInterrupt(CLOCK_IN), onClock, CHANGE);
  attachInterrupt(digitalPinToInterrupt(RESET_IN), onReset, RISING);
}

void loop() {
  if (send_tick) {
    send_tick = false;

    if (on) {
      count += 1;
      if (count >= max_count) count = 0;

      autoreset = map(analogRead(RESET_POT), 0, 1024, 2, 17);
      reset_count = count % autoreset;
  
      for (i = 0; i < max_outs; i++) {
        int div_i = divisions[i];
  
        if ((reset_count % div_i) == 0) {
          int skip = map(analogRead(pots[i]), 0, 1024, 1, 10);
          if (skip == 9 || ((count / div_i) % skip) != 0) {
            digitalWrite(outs[i], HIGH);
          }
        }
      }
    } else {
      for (i = 0; i < max_outs; i++) {
        digitalWrite(outs[i], LOW);
      }
    }
  }
}
