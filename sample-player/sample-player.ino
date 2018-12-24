#define KNOB1      (A0)
#define KNOB2      (A1)
#define CV_IN      (A2)
#define TRIG_IN    (A3)
#define MAX_SAMPLE  8

int triggers[] = {
  13,
  2,
  3,
  4,
  5,
  6,
  7,
  8,
  9
};

bool send_tick = false;
bool has_trig = false;

void pciSetup(byte pin) {
  *digitalPinToPCMSK(pin) |= bit (digitalPinToPCMSKbit(pin));  // enable pin
  PCIFR  |= bit (digitalPinToPCICRbit(pin)); // clear any outstanding interrupt
  PCICR  |= bit (digitalPinToPCICRbit(pin)); // enable interrupt for the group
}

void setup() {
  for (int i = 0; i <= MAX_SAMPLE; i++) {
    pinMode(triggers[i], OUTPUT);
  }

  pinMode(KNOB1, INPUT);
  pinMode(KNOB2, INPUT);
  pinMode(CV_IN, INPUT);
  pinMode(TRIG_IN, INPUT);

  pciSetup(TRIG_IN);
}

ISR (PCINT1_vect) {
  send_tick = true;
}

void loop() {
  if (!send_tick) return;
  send_tick = false;
  has_trig = !has_trig;

  if (has_trig) {
    int cv = analogRead(KNOB1);
    int trig_index = map(cv, 0, 1023, 0, MAX_SAMPLE);
    int trig = triggers[trig_index];

    digitalWrite(trig, HIGH);
  } else {
    for (int i = 0; i <= MAX_SAMPLE; i++) {
      digitalWrite(triggers[i], LOW);
    }   
  }
}


