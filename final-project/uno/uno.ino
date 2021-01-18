//#include <Arduino_FreeRTOS.h>
#include "SoftwareSerial.h"
#include "DFRobotDFPlayerMini.h"
#include <LiquidCrystal_I2C.h>
#include <Wire.h>//A4_SDA  A5_SCL
#include <MFRC522.h>
#include <SPI.h>
#include "TM1637.h" // 七段顯示器

#define GET_USER 0
#define START_GAME 1
#define PLAYING 2
#define SHOW_SCORE 3
#define MAX_1_NOTE 10

#define RST_PIN         9          
#define SS_PIN          10 //RC522卡上的SDA
#define CLK 8//pins definitions for TM1637 and can be changed to other ports
#define DIO 7

int8_t TimeDisp[] = {0x00,0x00,0x00,0x00};
TM1637 tm1637(CLK,DIO);
SoftwareSerial mySoftwareSerial(A1, A0); // RX, TX
LiquidCrystal_I2C lcd(0x27, 16, 2);
DFRobotDFPlayerMini myDFPlayer;
MFRC522 mfrc522;   // 建立MFRC522實體

/* 腳位 */
int Red = 6;
int Green = 5;
int Blue = 3;
int Button = A2;
int IsFree = A3;

/* 流程控制 */
int state = 0; // 用來表示現在位於哪個階段

/* 讀取使用者 */
byte user1[]={0xA1, 0xA7, 0xE4, 0x00};  //學生證的UID
byte user2[] = {0x10, 0x11, 0x39, 0x52}; // 藍色鑰匙環
bool detectUser = false;
int curUser = 0;

/* 播音樂 */
int curSong = 1;
int cur_pos = 0;
unsigned int initial_time = 0;
int hit_pos = 0;
int ttemp;

/* 溝通 */
char fromNano = '\0';

/* 遊戲 */
int score = 0;
int highest_1 = 0; // user 1 highest score
int highest_2 = 0; // user 2 highest score

/* 譜面 */
const int note1[37][2] = {
    4086, 0,
    4944, 1,
    5882, 2,
    6748, 3,
    7722, 4,
    8646,5,
    9580,0,
    10504,1,
    11429,2,
    12290,3
};

/* 模仿nano回傳值 */
const int hit[37] = {
    6086,
    6944, 
    7882, 
    8748, 
    9722, 
    10646, 
    11580, 
    12504, 
    13429, 
    14290
};

void StartGame();
void CheckIdentity(byte *buffer);
void DisplayScore();

void setup()
{
    mySoftwareSerial.begin(9600); // tfplayer

    Serial.begin(115200);

    Wire.begin(0x07); // i2c, uno的位址
    Wire.onReceive(receiveEvent);

    SPI.begin();
    mfrc522.PCD_Init(SS_PIN, RST_PIN); // 初始化MFRC522卡
    //用來確認讀卡機ok
    Serial.print(F("Reader "));
    Serial.print(F(": "));
    mfrc522.PCD_DumpVersionToSerial(); // 顯示讀卡設備的版本

    lcd.init();   // initialize LCD
    lcd.backlight();    // open LCD backlight
    
    tm1637.set(); // 七段顯示器
    tm1637.init();

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
    myDFPlayer.volume(7);  //Set volume value. From 0 to 30
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
    //Serial.println(state);
    //ttemp = digitalRead(IsFree);
    //Serial.println(state);
    tm1637.display(TimeDisp);
    if (state == GET_USER) {
        if(detectUser) { // 已經偵測並且讀取到user
            detectUser = false;
            CheckIdentity(mfrc522.uid.uidByte);
            if (curUser != 0) { // 確認已經讀到user，就進入start_game
                state = START_GAME;
            }
        }
        if (mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial()) { //偵測到刷卡
            detectUser = true;
        }
    } 
    else if (state == START_GAME) {
        if (!digitalRead(Button) && digitalRead(IsFree)) { //如果按下按鈕，並且沒有在播音樂，那就開始播
            StartGame();
            state = PLAYING;
            //Serial.println("state = Playing");
            //delay(5000);
        }
        if (mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial()) { //偵測到刷卡
            detectUser = true;
            state = GET_USER;
        }
    }
    else if (state == PLAYING) {
        //Serial.println(ttemp);
        ttemp = digitalRead(IsFree);
        if (ttemp == 0) { //還在播音樂
            /*  傳送音符給nano，讓他亮出來 */
            if ((cur_pos < MAX_1_NOTE) && ((millis()-initial_time) > note1[cur_pos][0])) {
                Wire.beginTransmission(0x9);
                Wire.write(note1[cur_pos][1]); 
                Wire.endTransmission();
                Serial.println("send!");
                //Serial.println(note1[cur_pos][1]);
                cur_pos++;
            }
            // if ((hit_pos < MAX_1_NOTE) && ((millis()-initial_time) > hit[hit_pos])) {
            //     score += 5;
            //     DisplayScore();
            // }
        }
        else if(ttemp == 1) {
            Serial.print("in else, is free = ");
            Serial.println(ttemp);

            Wire.beginTransmission(0x9);
            Wire.write(15); 
            Wire.endTransmission();
            Serial.println("send! end signal");
          //delay(5000);
          state = START_GAME;
        }
    }

}

void DisplayScore()
{
    int temp = score;
    for(int i = 3; i >= 0; i++) {
        TimeDisp[i] = temp % 10;
        temp /= 10;
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

    Wire.beginTransmission(0x9);
    Wire.write(10); 
    Wire.endTransmission();
    Serial.println("send! start signal");

    Serial.println("start!");
    myDFPlayer.play(curSong);
    initial_time = millis();
    cur_pos = 0;
    //delay(100); // for getting correct busy signal
    delay(1000);
    analogWrite(Red, 0);
    analogWrite(Green, 0);
    analogWrite(Blue, 0);
}

void CheckIdentity(byte *buffer)
{
    bool find = true;
    //比對user 1
    for ( int i = 0; i < 4; i++ ) { // 卡片UID為4段，分別做比對
        if ( user1[i] != mfrc522.uid.uidByte[i] ) { 
            find = false; // 如果任何一個比對不正確，they_match就為false，然後就結束比對
            break; 
        }
    }
    if (find == true) {
        curUser = 1;
        Serial.println("is user1!");
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Welcome User 1!");
        lcd.setCursor(2, 1);
        lcd.print("Highest Score: ");
        lcd.print(highest_1);
        return;
    }
    //比對user 2
    find = true;
    for ( int i = 0; i < 4; i++ ) { // 卡片UID為4段，分別做比對
        if ( user2[i] != mfrc522.uid.uidByte[i] ) { 
            find = false; // 如果任何一個比對不正確，they_match就為false，然後就結束比對
            break; 
        }
    }
    
    if (find == true) {
        curUser = 2;
        Serial.println("is user2!");
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Welcome User 2!");
        lcd.setCursor(2, 1);
        lcd.print("Highest Score: ");
        lcd.print(highest_2);
        return;
    } else {
        curUser = 0;
        Serial.println("unknown user");
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("unknown user");
    }
}
