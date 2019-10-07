/**
 * Based on Dial-Up by Møffenzeef Mødular
 * @see https://github.com/moffenzeefmodular/dialup
 */

unsigned int Acc[] {0, 0, 0, 0};
unsigned int Note = 0;
unsigned int Note1 = 0;
unsigned int Note2 = 0;
unsigned int Note3 = 0;

void setup() {
  TCCR2A = _BV(COM2A1) | _BV(COM2B1) | _BV(WGM21) | _BV(WGM20);
  TCCR0A = (1<<WGM01);
  TCCR0B |= (1<<CS01);
  TIMSK0 |= (1<<OCIE0A);

  pinMode(11, OUTPUT);
  analogReference(DEFAULT); 
}

void loop() {
  Note =  map(analogRead(A0), 0, 850, 1, 32);
  Note1 = map(analogRead(A1), 0, 850, 1, 32); 
  Note2 = map(analogRead(A2), 0, 850, 1, 32);  
  OCR0A = map(analogRead(A3), 0, 850, 80, 20);
}

ISR(TIMER0_COMPA_vect) {
  Acc[0] = Acc[0] + Note;
  Acc[1] = Acc[1] + Note1;
  Acc[2] = Acc[2] + Note2;
  OCR2A = (Acc[0] * Acc [1]) ^ Acc[2] >> 8 & 0x80;
}
