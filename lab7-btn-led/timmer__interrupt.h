void timmer_init() {
// initialize timer1
  noInterrupts(); // atomic access to timer reg.
  TCCR1A = 0; TCCR1B = 0; TCNT1 = 0;
  TCCR1B |= (1 << WGM12); // turn on CTC mode
  TCCR1B |= (1<<CS12) | (1<<CS10); // 1024 prescaler
  OCR1A = 1562; // give 0.1 sec at 16 MHz/1024
  interrupts(); // enable all interrupts
}
uint64_t x=0;
uint32_t millis_(){
  return x;
}
void timmer_enable() {
  x+=((TIFR1 & 1 << OCF1A)!=0);
  TIFR1 = (TIFR1 & 1 << OCF1A)&(1<<OCF1A);
}
