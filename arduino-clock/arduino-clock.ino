#define CLOCK_PIN          2
#define OUT1               8
#define OUT2               9
#define OUT3               10
#define OUT4               11
#define OUT5               12
#define OUT6               13

bool send_tick = false;
bool on = false;
int count = -1;

void onClock() {
  on = !on;
  send_tick = true;
}

void setup() {
  pinMode(CLOCK_PIN, INPUT);
  pinMode(OUT1, OUTPUT);
  pinMode(OUT2, OUTPUT);
  pinMode(OUT3, OUTPUT);
  pinMode(OUT4, OUTPUT);
  pinMode(OUT5, OUTPUT);
  pinMode(OUT6, OUTPUT);

  attachInterrupt(digitalPinToInterrupt(CLOCK_PIN), onClock, CHANGE);
}

void loop() {
  if (!send_tick) return;
  send_tick = false;

  if (on) {
    count += 1;

    if (count % 2 == 0) {
      bool fourth = count % 4 == 0;
      digitalWrite(OUT1, HIGH);
      digitalWrite(OUT3, fourth);
      digitalWrite(OUT4, !fourth && count % 6 == 0);
      digitalWrite(OUT5, fourth && count % 8 == 0);
      digitalWrite(OUT6, fourth && count % 16 == 0);
    } else {
      digitalWrite(OUT2, count % 3 == 0);
    }

    if (count >= 96) count = 0;
  } else {
    digitalWrite(OUT1, LOW);
    digitalWrite(OUT2, LOW);
    digitalWrite(OUT3, LOW);
    digitalWrite(OUT4, LOW);
    digitalWrite(OUT5, LOW);
    digitalWrite(OUT6, LOW);
  }
}
