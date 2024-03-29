#include "AnalogMatrixKeypad.h"
#include <Arduino_FreeRTOS.h>
#include <LiquidCrystal_I2C.h>
#include <SPI.h>
#include <MFRC522.h>
#include <Wire.h>
#include <task.h>
#include <semphr.h>
#define LEDR 6
#define LEDG 5
#define Buzzer 9
#define RST_PIN         A1           // Configurable, see typical pin layout above
#define SS_PIN          10           // Configurable, see typical pin layout above
#define IRQ_PIN         2           // Configurable, depends on hardware
#define keypadMode      0
#define RFIDMode        1

MFRC522 mfrc522(SS_PIN, RST_PIN);   // Create MFRC522 instance.

MFRC522::MIFARE_Key key;

SemaphoreHandle_t  gatekeeper = 0; /* global handler */


byte regVal = 0x7F;
void activateRec(MFRC522 mfrc522);
void clearInt(MFRC522 mfrc522);
LiquidCrystal_I2C lcd(0x27,16,2);
// include the file in your lab12 folder
// ???� lab12
//      ???� lab12.ino
//      ???� AnalogMatrixKeypad.h
//      ???� AnalogMatrixKeypad.cpp
AnalogMatrixKeypad keypad(A0);
// init keypad and keypad input is A0
char passcode[9] = {'8','0','8','0','\0','\0','\0','\0','\0'};
char entercode[9] = {'\0'};
int keyLength = 0;
bool bNewInt = false;
byte *id;
int sensorMode = keypadMode;

TaskHandle_t Task_taskKeypad;
TaskHandle_t Task_taskLEDTone;
TaskHandle_t Task_taskRFID;

void taskKeypadFun(void *pvParameters);
void taskLEDToneFun(void *pvParameters);
void taskRFIDFun(void *pvParameters);

// clear all things
void clearAll(){
    memset(entercode, '\0', sizeof(entercode));
    turnoffLED();
    keyLength = 0;
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Input:");
    noTone(Buzzer);
    sensorMode = keypadMode;
}
// ??LED
void turnoffLED(){
  digitalWrite(LEDR, LOW);
  digitalWrite(LEDG, LOW);
}
void setup(){
    pinMode(LEDR, OUTPUT);
    pinMode(LEDG, OUTPUT);
    turnoffLED();
    lcd.init();
    lcd.backlight();
    Serial.begin(9600);
    gatekeeper = xSemaphoreCreateMutex();
    while (!Serial);      // Do nothing if no serial port is opened (added for Arduinos based on ATMEGA32U4)
    SPI.begin();          // Init SPI bus
  
    mfrc522.PCD_Init(); // Init MFRC522 card

    /* read and printout the MFRC522 version (valid values 0x91 & 0x92)*/
    Serial.print(F("Ver: 0x"));
    byte readReg = mfrc522.PCD_ReadRegister(mfrc522.VersionReg);
    Serial.println(readReg, HEX);
  
    /* setup the IRQ pin*/
    pinMode(IRQ_PIN, INPUT_PULLUP);
    /*
     * Allow the ... irq to be propagated to the IRQ pin
     * For test purposes propagate the IdleIrq and loAlert
     */
    regVal = 0xA0; //rx irq
    mfrc522.PCD_WriteRegister(mfrc522.ComIEnReg, regVal);
    attachInterrupt(digitalPinToInterrupt(IRQ_PIN), readCard, FALLING);
    
    xTaskCreate(taskKeypadFun, "taskKeypad", 128, NULL, 2, &Task_taskKeypad);
    xTaskCreate(taskRFIDFun, "taskRFID", 128, NULL, 2, &Task_taskRFID);
    xTaskCreate(taskLEDToneFun, "taskLEDTone", 128, NULL, 1, &Task_taskLEDTone);

    Serial.println(F("End setup"));
}

// handle RFID
void taskRFIDFun(void *pvParameters){
    (void) pvParameters;
    while(1){
        if (bNewInt) { //new read interrupt
          Serial.print(F("Interrupt. "));
          mfrc522.PICC_ReadCardSerial(); //read the tag data
          // Show some details of the PICC (that is: the tag/card)
          Serial.print(F("Card UID:"));
          id = mfrc522.uid.uidByte;
          /*
          if(xSemaphoreTake(gatekeeper, 100)){
              dump_byte_array(mfrc522.uid.uidByte, mfrc522.uid.size);
              xSemaphoreGive(gatekeeper);
          }*/
          clearInt(mfrc522);
          mfrc522.PICC_HaltA();
          bNewInt = false;
          vTaskPrioritySet(Task_taskKeypad, 1);
          vTaskPrioritySet(Task_taskRFID, 1);
          vTaskPrioritySet(Task_taskLEDTone, 2);
        }
        activateRec(mfrc522);
        delay(100);
    }
}
// handle keypad
void taskKeypadFun(void *pvParameters){
    (void) pvParameters;
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Input:");
    while(1){
        Serial.println("keypad");
        char key = keypad.readKey();
        if (key != KEY_NOT_PRESSED) {
            // if keypad is pressed, do some analy
            if(key != '#'){
                if(xSemaphoreTake(gatekeeper, 100)){
                    if(keyLength < 9){
                        entercode[keyLength] = key;
                        keyLength++;
                        lcd.print(key);
                    }
                    xSemaphoreGive(gatekeeper); 
                }
            }
            // goto comepare and LED mode
            else{
                vTaskPrioritySet(Task_taskRFID, 1);
                vTaskPrioritySet(Task_taskKeypad, 1);
                vTaskPrioritySet(Task_taskLEDTone, 2);
            }
        }
        delay(100);
    }
}

// handle tone
void taskLEDToneFun(void *pvParameters){
    (void) pvParameters;
      while(1){
          if(sensorMode == keypadMode){
              if(!strcmp(passcode , entercode)){
                  lcd.clear();
                  lcd.setCursor(0,0);
                  lcd.print("Corret");
                  digitalWrite(LEDG, HIGH);
                  tone(Buzzer , 500);
              }
              else{
                  lcd.clear();
                  lcd.setCursor(0,0);
                  lcd.print("Wrong!");
                  lcd.print(entercode);
                  digitalWrite(LEDR, HIGH);
                  tone(Buzzer , 1000);
              }
          }
          else if(sensorMode == RFIDMode){
              int i = 0;
              bool access = 0;
              if(id[0] == 144 && id[1] == 26 && id[2] == 139 && id[3] == 195)
                  access = 1;
              else
                  access = 0;

              if(access){
                  if(xSemaphoreTake(gatekeeper, 100)){
                      //dump_byte_array(mfrc522.uid.uidByte, mfrc522.uid.size);
                      lcd.clear();
                      lcd.setCursor(0,0);
                      lcd.print("Corret");
                      digitalWrite(LEDG, HIGH);
                      tone(Buzzer , 500);
                      xSemaphoreGive(gatekeeper);
                  }  
              }
              else{
                  if(xSemaphoreTake(gatekeeper, 100)){
                      digitalWrite(LEDR, HIGH);
                      tone(Buzzer , 1000);
                      dump_byte_array(mfrc522.uid.uidByte, mfrc522.uid.size);
                      xSemaphoreGive(gatekeeper);
                  }
              }
          }
          // clear all and go back to input mode
          delay(1000);
          clearAll();
          vTaskPrioritySet(Task_taskRFID, 2);
          vTaskPrioritySet(Task_taskKeypad, 2);
          vTaskPrioritySet(Task_taskLEDTone, 1);
      }
}

void dump_byte_array(byte *buffer, byte bufferSize) {
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Wrong!");
  for (byte i = 0; i < bufferSize; i++) {
    Serial.print(buffer[i] < 0x10 ? " 0" : " ");
    Serial.print(buffer[i], HEX);
    lcd.print(buffer[i] < 0x10 ? "0" : "");
    lcd.print(buffer[i], HEX);
  }
  delay(300);
}
/**
 * MFRC522 interrupt serving routine
 */
void readCard() {
  Serial.println("readCard");
  bNewInt = true;
  sensorMode = RFIDMode;
  vTaskPrioritySet(Task_taskKeypad, 2);
  vTaskPrioritySet(Task_taskRFID, 3);
  vTaskPrioritySet(Task_taskLEDTone, 1);
  //delay(1000);
}

/*
 * The function sending to the MFRC522 the needed commands to activate the reception
 */
void activateRec(MFRC522 mfrc522) {
  mfrc522.PCD_WriteRegister(mfrc522.FIFODataReg, mfrc522.PICC_CMD_REQA);
  mfrc522.PCD_WriteRegister(mfrc522.CommandReg, mfrc522.PCD_Transceive);
  mfrc522.PCD_WriteRegister(mfrc522.BitFramingReg, 0x87);
}

/*
 * The function to clear the pending interrupt bits after interrupt serving routine
 */
void clearInt(MFRC522 mfrc522) {
  mfrc522.PCD_WriteRegister(mfrc522.ComIrqReg, 0x7F);
}


void loop(){
}
