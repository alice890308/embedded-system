#include <Key.h>
#include <Keypad.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Arduino_FreeRTOS.h>

LiquidCrystal_I2C lcd(0x3F, 16, 2);
byte cactus[8] = {0x04, 0x05, 0x15, 0x15, 0x16, 0x0c, 0x04, 0x04};
byte dinosaur[8] = {0x07, 0x05, 0x06, 0x07, 0x14, 0x17, 0x0E, 0x0A};
const int photo1 = A0, photo2 = A1;
int photo1_val = 900 ,photo2_val = 900;
int cacti1_pos[2] = {12, 0};
int cacti2_pos[2] = {15, 1}; 
int dinosaur_pos[2] = {0, 0};
int speed = 100;
int score = 0;
int i;
int gameover = 0;

char keymap[4][4] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};
// Column pin 1~4=
byte colPins[4] = {9, 8, 7, 6};
// Column pin 1~4
byte rowPins[4] = {13, 12, 11, 10};
Keypad myKeypad = Keypad(makeKeymap(keymap), rowPins, colPins, 4, 4);

TaskHandle_t cacti_handle;
TaskHandle_t lcd_handle;

void cacti(void *pvParameters);
void LCD(void *pvParameters);
void keypad(void *pvParameters);
void left(void *pvParameters);
void right(void *pvParameters);

void setup() {
  pinMode(photo1, INPUT);
  pinMode(photo2, INPUT);
  lcd.init();   // initialize LCD
  lcd.backlight();    // open LCD backlight
  lcd.createChar(0, cactus);
  lcd.createChar(1, dinosaur);
  xTaskCreate (cacti, "cactiTask", 64, NULL, 1, &cacti_handle);
  xTaskCreate (LCD, "LCDTask", 128, NULL, 1, &lcd_handle);
  xTaskCreate (keypad, "keypadTask", 64, NULL, 1, NULL);
  xTaskCreate (left, "leftTask", 64, NULL, 1, NULL);
  xTaskCreate (right, "rightTask", 64, NULL, 1, NULL);
  Serial.begin(9600);
}

void loop(){}

void keypad(void *pvParameters) {
    (void) pvParameters;

    for(;;) {
      char key = myKeypad.getKey();
      if (key == '2') {
        dinosaur_pos[1] = 0;
      }
      else if (key == '8') {
        dinosaur_pos[1] = 1;
      }
      //vTaskDelay(1);
    }
}

void left(void *pvParameters)  {
  (void) pvParameters;
//  pinMode(photo1, INPUT);

  for (;;) { // A Task shall never return or exit.
    photo1_val = analogRead(photo1);
  }
}

void right(void *pvParameters)  {
  (void) pvParameters;
//  pinMode(photo2, INPUT);

  for (;;) { // A Task shall never return or exit.
    photo2_val = analogRead(photo2);
  }
}

void LCD1(void *pvParameters)  {
  (void) pvParameters;

  for (;;) { // A Task shall never return or exit.
    if (photo1_val > 800 || photo2_val > 800) {
      vTaskResume(cacti_handle);
    }
    Serial.println(cacti1_pos[1]);
    lcd.clear();
    lcd.setCursor(cacti1_pos[0], cacti1_pos[1]);
    lcd.write(0);
    lcd.setCursor(cacti2_pos[0], cacti2_pos[1]);
    lcd.write(0);
    vTaskDelay(50);
  }
}

void cacti(void *pvParameters)  {
  (void) pvParameters;

  for (;;) { // A Task shall never return or exit.
//    Serial.print("photo1 : ");
//    Serial.println(photo1_val);
//    Serial.print("photo2 : ");
//    Serial.println(photo2_val);
    if (photo1_val < 800 && photo2_val > 800) { // only photo1 dark, speed down
        if (speed < 200) {
            speed += 20;
        }
    }
    if (photo1_val > 800 && photo2_val < 800) { //onlyt photo2 dark, speed up
        if (speed > 40) {
            speed -= 20;
        }
    }
    if (photo1_val < 800 && photo2_val < 800) {
      vTaskSuspend(cacti_handle);
    }
    
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
    Serial.print("speed : ");
    Serial.println(speed);
    vTaskDelay(speed);
  }
}

void LCD(void *pvParameters)  {
  (void) pvParameters;

  for (;;) { // A Task shall never return or exit.
    if (photo1_val > 800 || photo2_val > 800) {
      vTaskResume(cacti_handle);
    }
//    Serial.println(cacti1_pos[1]);
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
        vTaskDelay(2000/portTICK_PERIOD_MS);
    }
    vTaskDelay(50);
  }
} 