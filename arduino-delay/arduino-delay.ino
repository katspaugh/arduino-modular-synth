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
static const int maxTriggers = 200;
bool gotTrigger = false;
bool prevTrigger = false;
int delay1 = 0;
int delay2 = 0;
int lastTrig1 = 0;
int lastTrig2 = 0;
int triggers1[maxTriggers];
int triggers2[maxTriggers];
int now = 0;

void checkInputs() {
  delay1 = map(analogRead(POT1), 0, 1023, 0, 600);
  delay2 = map(analogRead(POT2), 0, 1023, 0, 600);
}

void checkTimers() {
  unsigned long diff1, diff2;
  int i, trig1, trig2;

  for (i = 0; i < maxTriggers; i++) {
    trig1 = triggers1[i];
    trig2 = triggers2[i];

    if (trig1 > 0) {
      diff1 = (now - trig1);
  
      if (diff1 >= delay1) {
        setTrigger1High();
        triggers1[i] = 0;
      }
    }
    if (trig2 > 0) {
      diff2 = (now - trig2);
  
      if (diff2 >= delay2) {
        setTrigger2High();
        triggers2[i] = 0;
      }
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
  unsigned long diff1 = now - lastTrig1;
  unsigned long diff2 = now - lastTrig2;

  if (diff1 >= triggerLength) {
    digitalWrite(TRIGGER1_OUT_PIN, LOW);
  }
  if (diff2 >= triggerLength) {
    digitalWrite(TRIGGER2_OUT_PIN, LOW);
  }
}

void addTimer() {
  int i;
  for (i = 0; i < maxTriggers; i++) {
    if (triggers1[i] == 0) {
      triggers1[i] = now;
      break;
    }
  }
  for (i = 0; i < maxTriggers; i++) {
    if (triggers2[i] == 0) {
      triggers2[i] = now;
      break;
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
