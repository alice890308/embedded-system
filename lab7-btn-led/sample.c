void setup() {
  pinMode(13, OUTPUT);
  // initialize timer1 
  noInterrupts(); // atomic access to timer reg.
  TCCR1A = 0;    TCCR1B = 0;    TCNT1 = 0;
  TCCR1B |= (1 << WGM12); // turn on CTC mode
  TCCR1B |= (1<<CS12) | (1<<CS10); // 1024 prescaler
  OCR1A = 7812;  // give 0.5 sec at 16 MHz/1024
  interrupts(); // enable all interrupts
}
void loop() {
  if (TIFR1 & (1 << OCF1A)) { // wait for time up
    digitalWrite(13, digitalRead(13) ^ 1);
    TIFR1 = (1<<OCF1A); 
  } // clear overflow flag
}
