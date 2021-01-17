#include "Arduino.h"
#include "SoftwareSerial.h"
#include "DFRobotDFPlayerMini.h"

SoftwareSerial mySoftwareSerial(A1, A0); // RX, TX
DFRobotDFPlayerMini myDFPlayer;

int isfree = A3;
int curSong = 1;

void setup()
{
  mySoftwareSerial.begin(9600);
  Serial.begin(115200);
  
  Serial.println(F("DFRobot DFPlayer Mini Demo"));
  Serial.println(F("Initializing DFPlayer ... (May take 3~5 seconds)"));
  pinMode(isfree, INPUT);
  
  if (!myDFPlayer.begin(mySoftwareSerial)) {  //Use softwareSerial to communicate with mp3.
    Serial.println(F("Unable to begin:"));
    Serial.println(F("1.Please recheck the connection!"));
    Serial.println(F("2.Please insert the SD card!"));
    while(true){
      delay(0); // Code to compatible with ESP8266 watch dog.
    }
  }
  Serial.println(F("DFPlayer Mini online."));
  
  myDFPlayer.volume(10);  //Set volume value. From 0 to 30
  myDFPlayer.play(curSong);  //Play the first mp3
}

void loop()
{
  int temp;
  temp = digitalRead(isfree);
  Serial.println(temp);
  if (temp == 1) {
    if (curSong == 1) {
      curSong = 2;
    } else {
      curSong = 1;
    }
    myDFPlayer.play(curSong);
    delay(100); // 因為mp3 player不會那麼快就更新BUSY的訊號，所以要加delay才可以讀到正確的訊號
  }
}