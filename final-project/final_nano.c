#include <Wire.h>//A4_SDA  A5_SCL

int fromUno = 0;

void setup() {
    Serial.begin(9600);
    Wire.begin(0x07); // nano的位址
    Wire.onReceive(receiveEvent);
}

void receiveEvent(int bytes) {
    while(Wire.available()){//wire.read裡面有一個佇列，會依序取出傳來的數字，available這個函數是現在這個佇列裡面有幾筆資料
      fromUno = Wire.read();
      Serial.println(fromUno);
    }
}

void loop()
{

}