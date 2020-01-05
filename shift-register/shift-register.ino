static const int CLOCK_PIN = 3;
static const int CV_PIN = A1;
static const int OUT1 = 12;
static const int OUT2 = 11;
static const int OUT3 = 10;
static const int OUT4 = 9;
static const int OUT5 = 8;
static const int OUT6 = 7;

static const int max_outs = 6;
static bool clock_tick = false;
static bool clock_high = false;
int i = 0;

static const int outs[max_outs] = { OUT1, OUT2, OUT3, OUT4, OUT5, OUT6 };
static bool states[max_outs] = { 0, 0, 0, 0, 0, 0 };

void onClock() {
  clock_tick = true;
  clock_high = !clock_high;
}

void setup() {
  pinMode(CLOCK_PIN, INPUT);
  pinMode(LED_BUILTIN, OUTPUT);

  for (i = 0; i < max_outs; i++) {
    pinMode(outs[i], OUTPUT);
  }

  attachInterrupt(digitalPinToInterrupt(CLOCK_PIN), onClock, CHANGE);
}

void loop() {
  digitalWrite(LED_BUILTIN, clock_high);

  if (!clock_tick) { return; }
  clock_tick = false;

  if (clock_high) {
    for (i = max_outs - 1; i > 0; i--) {
      states[i] = states[i - 1];
      if (states[i]) {
        digitalWrite(outs[i], HIGH);
      }
    }

    states[0] = analogRead(CV_PIN) > 500;
    if (states[0]) {
      digitalWrite(outs[0], HIGH);
    }
  } else {
    digitalWrite(LED_BUILTIN, LOW);

    for (i = 0; i < max_outs; i++) {
      digitalWrite(outs[i], LOW);
    }
  }
}
