#define CLOCK_PIN          2
#define OUT1               7
#define OUT2               8
#define OUT3               9
#define OUT4               10
#define OUT5               11
#define OUT6               12

bool send_tick = false;
bool on = false;
int count = -1;

void onClock() {
  on = !on;
  send_tick = true;
}

void setup() {
  pinMode(CLOCK_PIN, INPUT);
  pinMode(12, OUTPUT);
  pinMode(11, OUTPUT);
  pinMode(10, OUTPUT);
  pinMode(9, OUTPUT);
  pinMode(8, OUTPUT);
  pinMode(7, OUTPUT);

  attachInterrupt(digitalPinToInterrupt(CLOCK_PIN), onClock, CHANGE);
}

void loop() {
  if (!send_tick) return;
  send_tick = false;

  if (on) {
    count += 1;

    if (count % 2 == 0) {
      digitalWrite(OUT1, HIGH);
      digitalWrite(OUT3, count % 4 == 0);
      digitalWrite(OUT4, count % 6 == 0);
      digitalWrite(OUT5, count % 8 == 0);
      digitalWrite(OUT6, count % 16 == 0);
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
