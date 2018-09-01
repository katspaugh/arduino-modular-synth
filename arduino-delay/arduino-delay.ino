/**
 * Dual trigger delay
 */
#define POT1               A0
#define POT2               A1
#define TRIGGER_IN_PIN     A2
#define TRIGGER1_OUT_PIN   A3
#define TRIGGER2_OUT_PIN   11
#define LED                13

static const int triggerLength = 2;
static const int maxTriggers = 64;
bool gotTrigger = false;
bool prevTrigger = false;
int delay1 = 0;
int delay2 = 0;
int lastTrig1 = 0;
int lastTrig2 = 0;
int now = 0;
int triggers[maxTriggers]; // keeps track of the start times of multiple triggers

void checkInputs() {
  delay1 = map(analogRead(POT1), 0, 1023, 0, 600);
  delay2 = map(analogRead(POT2), 0, 1023, 0, 600);
}

void checkTimers() {
  unsigned long diff;

  for (int i = 0; i < maxTriggers; i++) {
    int trig = triggers[i];

    if (trig == 0) continue;

    diff = (now - trig);

    if (diff >= delay1) {
      setTrigger1High();
      if (diff >= delay2) triggers[i] = 0;
    }
    if (diff >= delay2) {
      setTrigger2High();
      if (diff >= delay1) triggers[i] = 0;
    }
  }
}

void setTrigger1High() {
  lastTrig1 = now;
  digitalWrite(TRIGGER1_OUT_PIN, HIGH);
}

void setTrigger2High() {
  lastTrig2 = now;
  digitalWrite(TRIGGER2_OUT_PIN, HIGH);
}

void setTriggersLow() {
  if ((now - lastTrig1) >= triggerLength) {
    digitalWrite(TRIGGER1_OUT_PIN, LOW);
  }
  if ((now - lastTrig2) >= triggerLength) {
    digitalWrite(TRIGGER2_OUT_PIN, LOW);
  }
}

void addTimer() {
  for (int i = 0; i < maxTriggers; i++) {
    if (triggers[i] == 0) {
      triggers[i] = now;
      return;
    }
  }
}

void setup() {
  pinMode(TRIGGER_IN_PIN, INPUT_PULLUP);
  pinMode(TRIGGER1_OUT_PIN, OUTPUT);
  pinMode(TRIGGER2_OUT_PIN, OUTPUT);
  pinMode(LED, OUTPUT);

  digitalWrite(LED, LOW);
}

void loop() {
  checkInputs();

  now = millis();

  setTriggersLow();

  checkTimers();

  gotTrigger = digitalRead(TRIGGER_IN_PIN);

  if (prevTrigger != gotTrigger) {
    prevTrigger = gotTrigger;
    if (gotTrigger) addTimer();
    digitalWrite(LED, gotTrigger);
  }
}
