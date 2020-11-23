#include"timmer__interrupt.h"
void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  pinMode(13,1);
  timmer_init();
}

void loop() {
  // put your main code here, to run repeatedly:
  digitalWrite(13,millis_()%2);
  Serial.println(millis_());
  timmer_enable();
}
