#include"easyTimer.h"
byte L[3]={10,11,12};
byte H[3]={4,3,5};
const int buttonPin =2;
byte channel,db;
void light(byte x){
  digitalWrite(L[2],x&4);
  digitalWrite(L[1],x&2);
  digitalWrite(L[0],x&1);
  digitalWrite(H[2],(channel)&4);
  digitalWrite(H[1],(channel)&2);
  digitalWrite(H[0],(channel)&1);
}
void handle_btn1_click(){
  Serial.println((digitalRead(H[2])<<2)+(digitalRead(H[1])<<1)+(digitalRead(H[0])));
}
void setup() {
  // put your setup code here, to run once:
  for(int i=0;i<3;i++){
    pinMode(L[i],1);
    pinMode(H[i],1);
  }
  attachInterrupt(0, handle_btn1_click, FALLING);
  Serial.begin(9600);
  cli();
  TCCR1A = 0;    TCCR1B = 0;    TCNT1 = 0;
  TCCR1B |= (1 << WGM12); // turn on CTC mode
  TCCR1B |= (1<<CS12) | (1<<CS10); // 1024 prescaler
  OCR1A = 1;  // give 0.1 sec at 16 MHz/1024
  TIMSK1 |= (1<<OCIE1A); // enable timer compare int.
  sei();
}

void loop() {
  // put your main code here, to run repeatedly:
}
ISR(TIMER1_COMPA_vect)
{
  channel++;
  if(channel>6)channel=0;
  digitalWrite(H[2],(channel)&4);
  digitalWrite(H[1],(channel)&2);
  digitalWrite(H[0],(channel)&1);
}
/*
 *  #include"easyTimer.h"
byte L[3]={10,11,12};
byte H[3]={5,3,4};

void setup() {
  // put your setup code here, to run once:
  for(int i=0;i<3;i++){
    pinMode(L[i],1);
    pinMode(H[i],1);
  }
}
void light(byte x,byte y){
  digitalWrite(L[2],x&4);
  digitalWrite(L[1],x&2);
  digitalWrite(L[0],x&1);
  digitalWrite(H[2],y&4);
  digitalWrite(H[1],y&2);
  digitalWrite(H[0],y&1);
}
void loop() {
  // put your main code here, to run repeatedly:
  for(int i=0;i<8;i++){
    mdelay(100)
    for(int j=0;j<8;j++)
      light(i,j);
  }
}
 */
