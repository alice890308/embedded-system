#include <SoftwareSerial.h> 
SoftwareSerial mySerial(10,11);

char receive;
void setup() { 
    Serial.begin(9600);   
    mySerial.begin(9600);   
}

void loop() {
  if (mySerial.available()){
    char temp = mySerial.read();
    Serial.print(temp);
  }
  else if (Serial.available()) {
    receive = Serial.read();
    while(mySerial.available());
    mySerial.print(receive);
  } 
}
