byte L[3]={10,11,12};
byte H[3]={4, 3, 5};
void light(byte x,byte y){
  digitalWrite(L[2],x&4);
  digitalWrite(L[1],x&2);
  digitalWrite(L[0],x&1);
  digitalWrite(H[2],y&4);
  digitalWrite(H[1],y&2);
  digitalWrite(H[0],y&1);
}
void setup() {
    for(int i=0;i<3;i++){
        pinMode(L[i],1);
        pinMode(H[i],1);
    }
}
void loop(){
    light(1,1);
}