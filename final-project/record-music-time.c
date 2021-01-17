//#include <Arduino_FreeRTOS.h>
#include "SoftwareSerial.h"
#include "DFRobotDFPlayerMini.h"
#include <Wire.h>//A4_SDA  A5_SCL

#define GET_USER 0
#define START_GAME 1
#define PLAYING 2
#define SHOW_SCORE 3

SoftwareSerial mySoftwareSerial(A1, A0); // RX, TX
DFRobotDFPlayerMini myDFPlayer;

int Red = 6;
int Green = 5;
int Blue = 3;
int Button = A2;
int IsFree = A3;
int curSong = 1;
int state = 1; // 用來表示現在位於哪個階段
char fromNano = '\0';

unsigned int initial_time = 0;
int flag = 0;
int temp;

// const int note1[37][2] = {
//     10, 0, 
// }

void StartGame();

void setup()
{
    // put your setup code here, to run once:
    mySoftwareSerial.begin(9600);
    Serial.begin(115200);
    Wire.begin(0x09); // uno的位址
    Wire.onReceive(receiveEvent);
    pinMode(Button, INPUT_PULLUP); //return LOW when down
    pinMode(Red, OUTPUT);
    pinMode(Green, OUTPUT);
    pinMode(Blue, OUTPUT);
    pinMode(IsFree, INPUT); // mp3 player busy signal

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
}

/* 如果uno接收到資料的話就會call這個function */
void receiveEvent(int bytes) {
    while(Wire.available()){//wire.read裡面有一個佇列，會依序取出傳來的數字，available這個函數是現在這個佇列裡面有幾筆資料
      fromNano = Wire.read();
      Serial.println(fromNano);
    }
}

void loop() 
{
    int temp;
    int isfree;
    
    if (state == GET_USER) {
        Serial.println("lalala");
    } 
    else if (state == START_GAME) {
        if (!digitalRead(Button) && digitalRead(IsFree)) { //如果按下按鈕，並且沒有在播音樂，那就開始播
            StartGame();
            state = PLAYING;
        }
    }
    else if (state == PLAYING) {
        //Serial.println("vivivi");
        if (!digitalRead(IsFree)) { //還在播音樂
            int temp = digitalRead(Button);
            if (!temp && (flag == 0)) {
                flag = 1;
                //Serial.println("push");
                Serial.println(millis()-initial_time);
            }
            else if (temp == 1) {
                flag = 0;
            }
        }
        else {
          state = START_GAME;
        }
        
    }

}

void StartGame()
{
    analogWrite(Red, 0);
    analogWrite(Green, 0);
    analogWrite(Blue, 255);
    delay(1000);
    analogWrite(Red, 255);
    analogWrite(Green, 0);
    analogWrite(Blue, 0);
    delay(1000);
    analogWrite(Red, 255);
    analogWrite(Green, 255);
    analogWrite(Blue, 0);
    delay(1000);
    analogWrite(Red, 0);
    analogWrite(Green, 255);
    analogWrite(Blue, 0);
    Serial.println("start!");
    myDFPlayer.play(curSong);
    initial_time = millis();
    //delay(100); // for getting correct busy signal
    delay(1000);
    analogWrite(Red, 0);
    analogWrite(Green, 0);
    analogWrite(Blue, 0);
}