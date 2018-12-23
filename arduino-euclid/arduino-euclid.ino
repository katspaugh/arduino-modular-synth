#include <EEPROM.h>

#define BUTTON_PIN         11
#define CLOCK_PIN          12
#define OUT_PIN            13
#define INVERTED_OUT_PIN   10
#define RESET_PIN          A7
#define LONG_PRESS         300
#define SHORT_PRESS        10
#define MAX_STEPS          8

int leds[] = { 2, 7, 9, 8, 6, 5, 4, 3 };

bool alt_positions = false;
int active_steps1 = 1;
int active_steps2 = 1;
int offset = 0;
int counter = -1;
int steps = MAX_STEPS;
bool positions1[MAX_STEPS] = {};
bool positions2[MAX_STEPS] = {};

bool send_tick = false;
bool clock_state = false;
bool last_clock_state = false;
bool last_button_state = false;
bool last_reset_state = false;
int button_pressed_time = 0;

bool getPosition(uint8_t index, bool positions[MAX_STEPS]) {
  int step_idx = (index + offset) % steps;
  return positions[step_idx];
}

void checkButton() {
  bool button_state = digitalRead(BUTTON_PIN) == LOW;
  if (last_button_state == button_state) return;
  last_button_state = button_state;

  int now = millis();

  if (button_state) {
    button_pressed_time = now;
  } else {
    int press_duration = now - button_pressed_time;

    if (press_duration < SHORT_PRESS) return;

    if (press_duration >= LONG_PRESS) {
      alt_positions = !alt_positions;
    } else {
      if (alt_positions) {
        active_steps2 += 1;
        if (active_steps2 > steps) active_steps2 = 1;
        setPositions(active_steps2, positions2);
        EEPROM.write(1, active_steps2);
      } else {
        active_steps1 += 1;
        if (active_steps1 > steps) active_steps1 = 1;
        setPositions(active_steps1, positions1);
        EEPROM.write(0, active_steps1);
      }
    }

    setActiveLeds();
  }
}

void checkReset() {
  bool reset = analogRead(RESET_PIN) > 500;
  if (reset == last_reset_state) return;
  last_reset_state = reset;
  if (reset) counter = 0;
}

void setPositions(int active_steps, bool positions[MAX_STEPS]) {
  for (int i = 0; i < steps; i++) {
    positions[i] = false;
  }

  if (active_steps == 1 || steps - active_steps <= 1) {
    for (int i = 0; i < active_steps; i++) {
      positions[i] = true;
    }
    return;
  }

  int remainder = active_steps;
  int quotient = 0;
  int skip = 0;

  while (remainder > 0) {
    quotient = floor(steps / remainder);
    int rem = steps % remainder;
    if (rem) quotient += 1;

    for (int i = 0; i < steps; i++) {
      if ((i + 1) % quotient == 0) {
        positions[(i + skip) % steps] = true;
      }
    }

    skip += 1;
    remainder -= floor(steps / quotient);
  }
}

void setLed(int index, bool active) {
  int pin = leds[index];
  digitalWrite(pin, active ? HIGH : LOW);
}

void setActiveLeds() {
  for (int i = 0; i < steps; i++) {
    setLed(i, getPosition(i, alt_positions ? positions2 : positions1));
  }
}

void onClockOn() {
  counter += 1;
  if (counter >= steps) counter = 0;

  bool is_active1 = getPosition(counter, positions1);
  digitalWrite(OUT_PIN, is_active1);

  bool is_active2 = getPosition(counter, positions2);
  digitalWrite(INVERTED_OUT_PIN, is_active2);

  setLed(counter, alt_positions ? !is_active2 : !is_active1);
}

void onClockOff() {
  digitalWrite(OUT_PIN, LOW);
  digitalWrite(INVERTED_OUT_PIN, LOW);
  setActiveLeds();
}

void pciSetup(byte pin) {
  *digitalPinToPCMSK(pin) |= bit (digitalPinToPCMSKbit(pin));  // enable pin
  PCIFR  |= bit (digitalPinToPCICRbit(pin)); // clear any outstanding interrupt
  PCICR  |= bit (digitalPinToPCICRbit(pin)); // enable interrupt for the group
}

ISR(PCINT0_vect) {
  send_tick = true;
}

void setup() {
  for (int i = 0; i < steps; i++) {
    int pin = leds[i];
    pinMode(pin, OUTPUT);
  }

  pinMode(CLOCK_PIN, INPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(OUT_PIN, OUTPUT);
  pinMode(INVERTED_OUT_PIN, OUTPUT);

  pciSetup(CLOCK_PIN);

  active_steps1 = EEPROM.read(0);
  active_steps2 = EEPROM.read(1);

  setPositions(active_steps1, positions1);
  setPositions(active_steps2, positions2);
  setActiveLeds();
}

void loop() {
  checkButton();
  checkReset();

  if (!send_tick) return;
  send_tick = false;

  clock_state = digitalRead(CLOCK_PIN);
  if (clock_state == last_clock_state) return;
  last_clock_state = clock_state;

  clock_state ? onClockOn() : onClockOff();
}
