#include <EEPROM.h>

#define BUTTON_PIN         11
#define CLOCK_PIN          12
#define OUT_PIN            13
#define INVERTED_OUT_PIN   10
#define RESET_PIN          A7
#define LONG_PRESS         300
#define CLOCK_PULSES       1
#define MAX_STEPS          8

int leds[] = {
  3,
  4,
  5,
  6,
  8,
  9,
  7,
  2
};

int active_steps = 1;
int offset = 0;
int clock_counter = 0;
int counter = 0;
int steps = 8;
bool positions[8] = {};

bool send_tick = false;
bool clock_state = false;
bool last_button_state = false;
bool last_reset_state = false;
int button_pressed_time = 0;

bool getPosition(uint8_t index) {
  return positions[(index + offset) % steps];
}

void checkButton() {
  bool button_state = digitalRead(BUTTON_PIN) == LOW;
  if (last_button_state == button_state) return;

  // On button up
  if (last_button_state) {
    int press_time = millis() - button_pressed_time;

    // Change the total steps on long press
    if (press_time >= LONG_PRESS) {
      steps -= 1;
      if (steps <= 0) steps = MAX_STEPS;
      active_steps = min(active_steps, steps);
    // or change the number of active steps
    } else {
      active_steps = (active_steps + 1);
      if (active_steps > steps) active_steps = 1;
      EEPROM.write(0, active_steps);
    }

    setPositions();
    setActiveLeds();
  // On button down
  } else {
    button_pressed_time = millis();
  }

  last_button_state = button_state;
}

void checkReset() {
  bool reset = analogRead(RESET_PIN) > 500;
  if (reset == last_reset_state) return;
  last_reset_state = reset;
  if (reset) counter = 0;
}

void setPositions() {
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
    setLed(i, getPosition(i));
  }
}

void onClockOn() {
  bool is_active = getPosition(counter);

  digitalWrite(OUT_PIN, is_active ? HIGH : LOW);
  digitalWrite(INVERTED_OUT_PIN, is_active ? LOW : HIGH);
  setLed(counter, !is_active);

  counter = (counter + 1) % steps;
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
  clock_state = !clock_state;
  if (clock_counter == 0) send_tick = true;
  clock_counter += 1;
  if (clock_counter >= CLOCK_PULSES) clock_counter = 0;
}

void setup() {
  for (int i = 0; i < steps; i++) {
    int pin = leds[i];
    pinMode(pin, OUTPUT);
  }

  pinMode(CLOCK_PIN, INPUT_PULLUP);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(OUT_PIN, OUTPUT);
  pinMode(INVERTED_OUT_PIN, OUTPUT);

  pciSetup(CLOCK_PIN);

  active_steps = EEPROM.read(0);

  setPositions();
  setActiveLeds();
}

void loop() {
  checkButton();
  checkReset();

  if (!send_tick) return;

  send_tick = false;

  clock_state ? onClockOn() : onClockOff();
}
