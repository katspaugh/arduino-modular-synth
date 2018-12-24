#define START_PIN          5
#define BUTTON_PIN         3
#define CLOCK_PIN          2
#define STEPS              8

bool send_tick = false;
int mode = 0; // 0 – forward, 1 – reverse, 2 – alternate
bool forward = true;
bool last_button_state = false;
int current_step = -1;

void onPress() {
  mode += 1;
  if (mode > 2) mode = 0;
}

void checkButton() {
  bool button_state = digitalRead(BUTTON_PIN) == LOW;

  if (last_button_state == button_state) return;
  last_button_state = button_state;

  if (button_state) {
    onPress();
  }
}

void updateOuts() {
  if (current_step >= 0) {
    digitalWrite(current_step + START_PIN, LOW);
  }

  if (mode == 0) {
    forward = true;
  } else if (mode == 1) {
    forward = false;
  }

  if (forward) {
    current_step += 1;
  } else {
    current_step -= 1;
  }

  if (current_step < 0) {
    if (mode == 2) {
      forward = true;
      current_step = 0;
    } else {
      current_step = STEPS - 1;
    }
  } else if (current_step >= STEPS) {
    if (mode == 2) {
      forward = false;
      current_step -= 1;
    } else {
      current_step = 0;
    }
  }

  digitalWrite(current_step + START_PIN, HIGH);
}

void onClock() {
  send_tick = true;
}

void blinkOuts() {
  for (int i = 0; i < STEPS; i++) {
    digitalWrite(i + START_PIN, HIGH);
    delay(100);
    digitalWrite(i + START_PIN, LOW);
  }
}

void setup() {
  for (int i = 0; i < STEPS; i++) {
    pinMode(i + START_PIN, OUTPUT);
  }

  pinMode(CLOCK_PIN, INPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  attachInterrupt(digitalPinToInterrupt(CLOCK_PIN), onClock, RISING);

  blinkOuts();
}

void loop() {
  checkButton();

  if (!send_tick) return;

  send_tick = false;

  updateOuts();
}
