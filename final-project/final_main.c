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
#define CLEAR 4

#define MAX_1_NOTE 24
#define MAX_2_NOTE 60

#define RST_PIN         9          
#define SS_PIN          10 //RC522卡上的SDA
#define CLK 8//pins definitions for TM1637 and can be changed to other ports
#define DIO 7

TM1637 tm1637(CLK,DIO);
SoftwareSerial mySoftwareSerial(4, 2); // RX, TX
LiquidCrystal_I2C lcd(0x27, 16, 2);
DFRobotDFPlayerMini myDFPlayer;
MFRC522 mfrc522;   // 建立MFRC522實體

/* 腳位 */
int Red = 6;
int Green = 5;
int Blue = 3;
int Button = A2;
int IsFree = A3;
byte x_posision = A0;
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
long long int initial_time = 0;
int ttemp;
int joystick_in;

/* 溝通 */
char fromNano = '\0';

/* 遊戲分數 */
int score = 0;
int highest_1 = 0; // user 1 highest score
int highest_2 = 0; // user 2 highest score
int8_t TimeDisp[] = {0x00,0x00,0x00,0x00};

/* 譜面 */
const long int note1[37][2] = {
    4506, 0,
    5467, 0,
    6387, 0,
    7271, 0,
    8172, 0,
    9092, 0,
    10031, 0,
    10993, 0,
    11927, 0,
    12816, 0,
    13664, 0,
    14605, 0,
    15489, 0,
    16399, 0,
    17338, 0,
    20141, 0,
    21986, 0,
    23810, 0,
    25639, 0,
    27501, 0,
    29374, 0,
    30528, 0,
    31278, 0
};

const long int note2[75][2] = {
    2749, 0,
    4041, 0,
    5349, 0,
    6026, 0,
    6686, 0,
    6773, 0,
    7302, 0,
    7417, 0,
    7870, 0,
    8168, 0,
    8513, 0,
    8814, 0,
    9128, 0,
    9486, 0,
    9810, 0,
    10164, 0,
    10465, 0,
    11773, 0,
    13077, 0,
    14344, 0,
    15587, 0,
    15909, 0,
    16235, 0,
    16919, 0,
    17582, 0,
    18191, 0,
    18782, 0,
    19460, 0,
    19751, 0,
    20053, 0,
    20380, 0,
    20672, 0,
    21017, 0,
    21354, 0,
    22016, 0,
    22660, 0,
    23317, 0,
    23948, 0,
    24591, 0,
    25214, 0,
    25833, 0,
    26178, 0,
    26513, 0,
    27191, 0,
    27876, 0,
    28486, 0,
    29079, 0,
    29751, 0,
    29855, 0,
    30027, 0,
    30335, 0,
    30645, 0,
    35274, 0,
    37097, 0,
    37406, 0,
    37731, 0,
    38064, 0,
    38402, 0,
    38515, 0,
    38748, 0,
    39064, 0
};

void StartGame();
void CheckIdentity(byte *buffer);
void DisplayScore();
void ChangeScore();
void ResetAll();
void ChangeSong();
void DisplayUser();

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
    
    tm1637.init();
    tm1637.set(BRIGHT_TYPICAL); // 七段顯示器
    tm1637.display(0,0);  //設定每一位燈號顯示的內容，參數1：燈號，參數2：顯示的數字
    tm1637.display(1,0); 
    tm1637.display(2,0);
    tm1637.display(3,0);

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
        if (fromNano == 'h') {
            score += 5;
            ChangeScore();
            Serial.print("score = ");
            Serial.println(score);
        }
        Serial.println(fromNano);
    }
}

void loop() 
{
    //Serial.println(state);
    //ttemp = digitalRead(IsFree);
    //Serial.println(state);
    //Serial.println(analogRead(x_posision));
    DisplayScore();
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
        joystick_in = analogRead(x_posision);
        
        if (joystick_in > 1000 || joystick_in < 10) {
            ChangeSong();
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
            
            if (curSong == 1) {
                if ((cur_pos < MAX_1_NOTE) && ((millis()-initial_time) > note1[cur_pos][0])) {
                    Wire.beginTransmission(0x9);
                    Wire.write(note1[cur_pos][1]); 
                    Wire.endTransmission();
                    Serial.println("send!");
                    //Serial.println(note1[cur_pos][1]);
                    cur_pos++;
                }
            }
            else if (curSong == 2) {
                
                if ((cur_pos < MAX_2_NOTE) && ((millis()-initial_time) > note2[cur_pos][0])) {
                    Serial.println(curSong);
                    Wire.beginTransmission(0x9);
                    Wire.write(note2[cur_pos][1]); 
                    Wire.endTransmission();
                    Serial.println("send!");
                    //Serial.println(note1[cur_pos][1]);
                    cur_pos++;
                }
            }
        }
        else if(ttemp == 1) {
            Serial.print("in else, is free = ");
            Serial.println(ttemp);

            Wire.beginTransmission(0x9);
            Wire.write(15); 
            Wire.endTransmission();
            Serial.println("send! end signal");
          //delay(5000);
            state = CLEAR;
        }
    }
    else if (state == CLEAR) {
        ResetAll();
        state = START_GAME;
    }

}

void DisplayUser()
{
    lcd.clear();
    if (curUser == 1) {
        lcd.setCursor(0, 0);
        lcd.print("Welcome User 1!");
        lcd.setCursor(0, 1);
        lcd.print("Highest: ");
        lcd.print(highest_1);
    }
    else if (curUser == 2) {
        lcd.setCursor(0, 0);
        lcd.print("Welcome User 2!");
        lcd.setCursor(0, 1);
        lcd.print("Highest: ");
        lcd.print(highest_2);
    }
    else {
        Serial.println("unknown user");
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("unknown user");
    }
}

void DisplayScore()
{
    tm1637.display(0,TimeDisp[0]);  //設定每一位燈號顯示的內容，參數1：燈號，參數2：顯示的數字
    tm1637.display(1,TimeDisp[1]); 
    tm1637.display(2,TimeDisp[2]);
    tm1637.display(3,TimeDisp[3]);
}

void ChangeScore()
{
    int temp = score;
    for(int i = 3; i >= 0; i--) {
        TimeDisp[i] = temp % 10;
        temp /= 10;
    }
}

void ChangeSong()
{
    if (joystick_in > 1000 && curSong == 1) { // 換成song 2, 現在先用
        curSong = 2;
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Change to Song 2");
        delay(1000);
        DisplayUser();
    }
    else if (joystick_in < 10 && curSong == 2) { // 換成song 1
        curSong = 1;
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Change to Song 1");
        delay(1000);
        DisplayUser();
    }
}

void ResetAll()
{
    /* 顯示最終分數 */
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Final Score!");
    lcd.setCursor(1, 1);
    lcd.print(score);
    /* 更新歷史紀錄 */
    if (curUser == 1) {
        if (score > highest_1) {
            highest_1 = score;
        }
    }
    else if (curUser == 2) {
        if (score > highest_2) {
            highest_2 = score;
        }
    }
    score = 0;
    ChangeScore(); // 把七段顯示器歸零

    delay(3000); // 顯示分數三秒
    DisplayUser();
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
        if (curUser != 1) {
            curUser = 1;
            DisplayUser();
        }
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
        if (curUser != 2) {
            curUser = 2;
            DisplayUser();
        }
        return;
    } else {
        if (curUser != 0){
            curUser = 0;
            DisplayUser();
        } 
    }
}