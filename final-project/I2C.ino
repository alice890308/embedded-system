// Include the required Wire library for I2C<br>
#include <Wire.h>//A4_SDA  A5_SCL
int LED = 13;
int x = 0;
bool flag=0;
byte I2C_bus;
byte image[1024];
void setup() {
  // Define the LED pin as Output
  pinMode (LED, OUTPUT);
  Serial.begin(9600);
  // Start the I2C Bus as Slave on address 9
  Wire.begin(0x09);//LCD的位址是0x27 arduino的位址可以自己設 範圍0x00~0x27 盡量避免全0或全1 才不容易傳錯 
  // Attach a function to trigger when something is received.
  Wire.onReceive(receiveEvent);
}
void receiveEvent(int bytes) {
    while(Wire.available()){//wire.read裡面有一個佇列，會依序取出傳來的數字，available這個函數是現在這個佇列裡面有幾筆資料
      I2C_bus=Wire.read();
      Serial.println(I2C_bus);
    }
    digitalWrite(13,I2C_bus&1);//第一個位元的狀態
}
byte x=0x55
void loop() {
  Wire.beginTransmission(0x7); // transmit to device #9指定位址
  Wire.write(x);              // sends x 資料
  Wire.endTransmission();    // stop transmitting 總之就是這三行
  x=~x;//取1補數;0x55->0xAA->0x55->0xAA
  delay(100);
}
