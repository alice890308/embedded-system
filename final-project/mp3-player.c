//SCL要接arduino A5
//SDA要接arduino A4
#include <Wire.h>
 
const int SD178_addr = 0x20;  // SLAVE ADDRESS of SD178B
byte play2[5]={0x88,0x00,0x00,0x00,0x01};
byte vol[2]={0x86,0x00};
byte stopf[2]={0x40,0x80};


// the setup function runs once when you press reset or power the board
void setup() {
  Wire.begin(); // join i2c bus (address optional for master)
}

 byte x[13] = {
  0x8A,0x00,                  // set SD178BM output pins MO[2:0] = 0b000
  0x8A,0x04,                  // set SD178BM output pins MO[2:0] = 0b100
  0x86,0xC0,
  0x88,0x27,0x0C,0x00,0x02,   // 播放 microSD 9998.WAV 2遍
  0x8A,0x00                   // set SD178BM output pins MO[2:0] = 0b000
 };

// the loop function runs over and over again forever
void loop() {
  // 用soundf來控制聲音
  soundf(0x0a);
  soundf(0x10);
  delay(10000);
  soundf(0x11);// song 2
  delay(5000);
  soundf(0x09);
  delay(3000);
  soundf(0xff);
}

/*
ins是一個byte，前四個byte是指令，後四個byte是資料
0x00 ~ 0x10 表示音量大小
0x10 ~ 0x20 用來播歌，第一首歌就是0x10, 最後一首是0x12 (因為我們只有三首歌)
*/

void soundf(byte ins){
  Serial.print("ins=");
  Serial.println(ins,HEX);
  Wire.beginTransmission(SD178_addr);
  Wire.write(stopf,2);// sends x
  Wire.endTransmission();
  delay(20);
  if(ins<0x10){
    vol[1]=ins<<4;
    Wire.beginTransmission(SD178_addr);
    Wire.write(vol,2);// sends x
    Wire.endTransmission();
  }
  else if(ins<0x20){
    play2[2]=ins&0xF;
    Wire.beginTransmission(SD178_addr); 
    Wire.write(play2,5);// sends x
    Wire.endTransmission();
  }
}

// #include <Wire.h>
 
// const int SLAVE_ADDRESS = 0x20;  // SLAVE ADDRESS of SD178B

// // the setup function runs once when you press reset or power the board
// void setup() {
//   Wire.begin(); // join i2c bus (address optional for master)
// }

//  byte x[13] = {
//   0x8A,0x00,                  // set SD178BM output pins MO[2:0] = 0b000
//   0x8A,0x04,                  // set SD178BM output pins MO[2:0] = 0b100
//   0x86,0xC0,
//   0x88,0x00,0x00,0x00,0x02,   // 播放 microSD 9998.WAV 2遍
//   0x8A,0x00                   // set SD178BM output pins MO[2:0] = 0b000
//  };

// // the loop function runs over and over again forever
// void loop() {
//   delay(10); // waits 10ms
//   Wire.beginTransmission(SLAVE_ADDRESS); // transmit to slave device
//   Wire.write(x,13);
//   Wire.endTransmission();     // stop transmitting
// }