#define CLOCK_IN           3
#define CV_IN              (A1)
#define OUT1               12
#define OUT2               11
#define OUT3               10
#define OUT4               9
#define OUT5               8
#define OUT6               7

const int max_outs = 6;
const int max_steps = 8;

const boolean patterns[max_outs][max_steps] = {
  { 0, 1, 0, 0, 1, 0, 0, 0 },
  { 1, 0, 0, 1, 0, 0, 1, 0 },
  { 0, 1, 1, 0, 1, 0, 1, 1 },
  { 1, 0, 1, 1, 0, 1, 1, 0 },
  { 1, 1, 0, 1, 1, 0, 1, 1 },
  { 0, 1, 1, 1, 1, 1, 1, 1 }
};

const int outs[max_outs] = { OUT1, OUT2, OUT3, OUT4, OUT5, OUT6 };

bool send_tick = false;
bool on = false;
int count = -1;
int cv = 0;
int i = 0;

void onClock() {
  on = !on;
  send_tick = true;
}

void setup() {
  pinMode(CLOCK_IN, INPUT);

  for (i = 0; i < max_outs; i++) {
    pinMode(outs[i], OUTPUT);
  }

  attachInterrupt(digitalPinToInterrupt(CLOCK_IN), onClock, CHANGE);
}

void loop() {
  if (!send_tick) return;
  send_tick = false;

  if (on) {
    count += 1;
    if (count >= max_steps) count = 0;

    cv = map(analogRead(CV_IN), 0, 1024, 0, max_outs - 1);

    for (i = 0; i < max_outs; i++) {
      digitalWrite(outs[i], patterns[((i + cv) % max_outs)][count]);
    }
  } else {
    for (i = 0; i < max_outs; i++) {
      digitalWrite(outs[i], LOW);
    }
  }
}
