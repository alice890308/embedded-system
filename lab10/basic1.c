//#include <Key.h>
//#include <Keypad.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Arduino_FreeRTOS.h>

LiquidCrystal_I2C lcd(0x3F, 16, 2);
byte cactus[8] = {0x04, 0x05, 0x15, 0x15, 0x16, 0x0c, 0x04, 0x04};
const int photo1 = A0, photo2 = A1;
int photo1_val = 900 ,photo2_val = 900;
int cacti1_pos[2] = {14, 0};
int cacti2_pos[2] = {15, 1}; 
int speed = 100;

TaskHandle_t cacti_handle;

void cacti(void *pvParameters);
void LCD(void *pvParameters);
void left(void *pvParameters);
void right(void *pvParameters);

void setup() {
  lcd.init();   // initialize LCD
  lcd.backlight();    // open LCD backlight
  lcd.createChar(0, cactus);
  xTaskCreate (cacti, "cactiTask", 128, NULL, 1, &cacti_handle);
  xTaskCreate (LCD, "LCDTask", 128, NULL, 1, NULL);
  xTaskCreate (left, "leftTask", 128, NULL, 1, NULL);
  xTaskCreate (right, "rightTask", 128, NULL, 1, NULL);
  Serial.begin(9600);
}

void loop(){}

void left(void *pvParameters)  {
  (void) pvParameters;
  pinMode(photo1, INPUT);

  for (;;) { // A Task shall never return or exit.
    photo1_val = analogRead(photo1);
  }
}

void right(void *pvParameters)  {
  (void) pvParameters;
  pinMode(photo2, INPUT);

  for (;;) { // A Task shall never return or exit.
    photo2_val = analogRead(photo2);
  }
}

void cacti(void *pvParameters)  {
  (void) pvParameters;

  for (;;) { // A Task shall never return or exit.
    Serial.print("photo1 : ");
    Serial.println(photo1_val);
    Serial.print("photo2 : ");
    Serial.println(photo2_val);
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
      Serial.println("in cacti");
    }
    if (cacti1_pos[0] == 0) {
      cacti2_pos[0] = 15;
    }
    else {
      cacti2_pos[0] -= 1;
      Serial.println("in cacti2");
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
    Serial.println(cacti1_pos[1]);
    lcd.clear();
    lcd.setCursor(cacti1_pos[0], cacti1_pos[1]);
    lcd.write(0);
    lcd.setCursor(cacti2_pos[0], cacti2_pos[1]);
    lcd.write(0);
    vTaskDelay(50);
  }
}