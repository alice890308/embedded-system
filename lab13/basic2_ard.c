#include <Key.h>
#include <Keypad.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Arduino_FreeRTOS.h>
#include <SoftwareSerial.h> 
SoftwareSerial mySerial(10,11);

LiquidCrystal_I2C lcd(0x3F, 16, 2);
byte cactus[8] = {0x04, 0x05, 0x15, 0x15, 0x16, 0x0c, 0x04, 0x04};
byte dinosaur[8] = {0x07, 0x05, 0x06, 0x07, 0x14, 0x17, 0x0E, 0x0A};
int cacti1_pos[2] = {12, 0};
int cacti2_pos[2] = {15, 1}; 
int dinosaur_pos[2] = {0, 0};
int speed = 100;
int score = 0;
int i;
int gameover = 0;
char read_msp = '\0';

TaskHandle_t cacti_handle;
TaskHandle_t lcd_handle;

void cacti(void *pvParameters);
void LCD(void *pvParameters);
void msp(void *pvParameters);

void setup() {
  lcd.init();   // initialize LCD
  lcd.backlight();    // open LCD backlight
  lcd.createChar(0, cactus);
  lcd.createChar(1, dinosaur);
  xTaskCreate (cacti, "cactiTask", 64, NULL, 1, &cacti_handle);
  xTaskCreate (LCD, "LCDTask", 128, NULL, 1, &lcd_handle);
  xTaskCreate (msp, "mspTask", 64, NULL, 1, NULL);
  Serial.begin(9600);
  mySerial.begin(9600); 
}

void loop() {}

void msp(void *pvParameters) {
  (void) pvParameters;

  for(;;) {
    if (mySerial.available()) {
      read_msp = (char)mySerial.read();
      if (read_msp == '1') {
        if (dinosaur_pos[1] == 0) {
          dinosaur_pos[1] = 1;
        }
        else {
          dinosaur_pos[1] = 0;
        }
        read_msp = '\0';
      }
    }
  }  
}

void cacti(void *pvParameters)  {
  (void) pvParameters;

  for (;;) { // A Task shall never return or exit.
//    Serial.print("photo1 : ");
//    Serial.println(photo1_val);
//    Serial.print("photo2 : ");
//    Serial.println(photo2_val);
    
    // if (cacti1_pos[0] == 0) {
    //   cacti1_pos[0] = 15;
    // }
    // else {
    //   cacti1_pos[0] -= 1;
    //   //Serial.println("in cacti");
    // }
    // if (cacti2_pos[0] == 0) {
    //   cacti2_pos[0] = 15;
    // }
    // else {
    //   cacti2_pos[0] -= 1;
    //   //Serial.println("in cacti2");
    // }
    vTaskDelay(30);
  }
}

void LCD(void *pvParameters)  {
  (void) pvParameters;

  for (;;) { // A Task shall never return or exit.
//    Serial.println(cacti1_pos[1]);
    if (cacti1_pos[0] == 0) {
      cacti1_pos[0] = 15;
    }
    else {
      cacti1_pos[0] -= 1;
      //Serial.println("in cacti");
    }
    if (cacti2_pos[0] == 0) {
      cacti2_pos[0] = 15;
    }
    else {
      cacti2_pos[0] -= 1;
      //Serial.println("in cacti2");
    }
    lcd.clear();
    lcd.setCursor(dinosaur_pos[0], dinosaur_pos[1]);
    lcd.write(1);
    lcd.setCursor(cacti1_pos[0], cacti1_pos[1]);
    lcd.write(0);
    lcd.setCursor(cacti2_pos[0], cacti2_pos[1]);
    lcd.write(0);

    if ((dinosaur_pos[0] == cacti1_pos[0]) && (dinosaur_pos[1] == cacti1_pos[1])) {
        gameover = 1;
    }
    else if ((dinosaur_pos[0] == cacti2_pos[0]) && (dinosaur_pos[1] == cacti2_pos[1])) {
        gameover = 1;
    }
    else if ((dinosaur_pos[0] == cacti1_pos[0]) && (dinosaur_pos[1] != cacti1_pos[1])){
        score++;
    }
    else if ((dinosaur_pos[0] == cacti2_pos[0]) && (dinosaur_pos[1] != cacti2_pos[1])){
        score++;
    }
    if (gameover == 1) {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Game over");
        lcd.setCursor(2, 1);
        lcd.print("Score = ");
        lcd.print(score);
        cacti1_pos[0] = 12;
        cacti1_pos[1] = 0;
        cacti2_pos[0] = 15;
        cacti2_pos[1] = 1; 
        dinosaur_pos[0] = 0;
        dinosaur_pos[1] = 0;
        gameover = 0;
        score = 0;
        while(mySerial.available());
        mySerial.print('7');
        vTaskDelay(2000/portTICK_PERIOD_MS);
    }
    vTaskDelay(50);
  }
} 