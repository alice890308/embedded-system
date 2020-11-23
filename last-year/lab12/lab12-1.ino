#include <Wire.h>
#include <LiquidCrystal_I2C.h>
//#include <Keypad.h>
#include "AnalogMatrixKeypad.h"
#include <string.h>
//#include <cstring>
#include <Arduino_FreeRTOS.h>
//#define KEY_ROWS 4
//#define KEY_COLS 4
#define Enter_passcode 0
#define Set_passcode 1
#define MAX 8
#define MIN 4
using namespace std;  
LiquidCrystal_I2C lcd(0x27,16,2);
AnalogMatrixKeypad keypad(A0);
int buzzer = 9;
int mode =0;
char passcode[8]={"8080"};
char input[8]={0};
int index=0;
int correct =1;
int r=0, c=0;
char *string;
char _string[20];
bool Display_enabled = true;
int redPin = 6, greenPin = 5, bluePin = 3;

void LoginTask(void *pvParameters);
void DisplayTask(void *pvParameters);
void setup(){
  Serial.begin(9600);
  lcd.init();   // initialize LCD
  lcd.backlight();    // open LCD backlight
  lcd.clear(); // clear all 
  xTaskCreate(LoginTask, "LoginTask", 128, NULL, 1, NULL);
  xTaskCreate(DisplayTask, "DisplayTask", 128, NULL, 1, NULL);
  Set_Display(0, 0, "Input:");
  Display_enabled = true;
  pinMode(buzzer, OUTPUT);
  pinMode(redPin, OUTPUT);
  pinMode(greenPin, OUTPUT);
  pinMode(bluePin, OUTPUT);
}
void go_to_Enter(void){
  index =0 ;
  for(int i=0; i < MAX;i++){input[i] = {0};}
  lcd.clear();
  Set_Display(0, 0, "Input:");
  Display_enabled = true;
  mode = 0; 
}
void Set_Display(int col, int row, char* print){
  r = row;
  c = col;
  string = print;
  Display_enabled = true;
}
//void Set_string(
void LoginTask(void *pvParameters)  {
  (void) pvParameters;
  for (;;) {
    if(mode == Enter_passcode){
      analogWrite(redPin, 0);
      analogWrite(greenPin,0);
      analogWrite(bluePin, 0);
      char key = keypad.readKey();
      Serial.print("Input:");   
      if(key != KEY_NOT_PRESSED){
        if(index >= 9){
          go_to_Enter();
        }
        if(key !='#'){
          input[index] = key;
          lcd.setCursor(index+6, 0);
          lcd.print(key);
          index++;
        }
        else if(key == '#'){
          if(index ==0){
            go_to_Enter();
          }
          else{
            lcd.clear();
            if(strcmp (input,passcode) == 0){
              Set_Display(0, 0, "Correct");
              Display_enabled = true;
              analogWrite(redPin, 0);
              analogWrite(greenPin,250);
              analogWrite(bluePin, 0);
              tone(buzzer,500);
            }
            else {
//              Set_Display(0, 0, "Wrong");
//              Display_enabled = true;
//              Set_Display(0, 5, input);
              lcd.clear();
              lcd.setCursor(0, 0);
              lcd.print("Wrong!");
//              Display_enabled = true;
              lcd.setCursor(6, 0);
              lcd.print(input);
//              Display_enabled = true;
              analogWrite(redPin, 250);
              analogWrite(greenPin, 0);
              analogWrite(bluePin, 0);
              tone(buzzer,1000);
            }

            delay(1000);
            noTone(buzzer);
            go_to_Enter();
          }
        }
      }
    }
    vTaskDelay(10); 
  }
}

void DisplayTask(void *pvParameters)  {
  (void) pvParameters;
  for (;;) { 
    Serial.println(Display_enabled);
    if(Display_enabled){
      lcd.setCursor(0, 0);
      Serial.println(string);
      lcd.print(string);
      Display_enabled = false;
      vTaskDelay(10); 
    }
  }
}
void loop(){}
