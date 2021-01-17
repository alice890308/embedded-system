#include"easyTimer.h"
byte L[3]={10,11,12};
byte H[3]={2,3,4};

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
    for(int j=0;j<8;j++)
      light(i,j);
    delay(100);
  }
}