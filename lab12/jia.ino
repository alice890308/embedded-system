#include <LiquidCrystal_I2C.h>
#include <Key.h>
#include <Keypad.h>
#include <Arduino_FreeRTOS.h>
#include <semphr.h>

#define KEY_ROWS 4 
#define KEY_COLS 4

LiquidCrystal_I2C lcd(0x3F,16,2);
byte err_dragon[8]  = {B00111, B00101, B00110, B00111, B10100, B10111, B01110, B01010};
byte cact[8] = {0x04, 0x05, 0x15, 0x15, 0x16, 0x0C, 0x04, 0x04};
byte egg[8] = {0b00000, 0b01010, 0b11111, 0b11111, 0b11111, 0b01110, 0b00100, 0b00000};
byte broken[8] = {B00100, B10101, B01110, B11111, B11111, B01110, B10101, B00100};

TaskHandle_t displayHandler = NULL;
TaskHandle_t dinoHandler = NULL;
TaskHandle_t cactusHandler = NULL;

SemaphoreHandle_t  gatekeeper; /* global handler */
SemaphoreHandle_t  binary_sem; /* global handler */

int buf[2][16];

const int buttonPin1 = 2;    // the number of the pushbutton pin
int button_int1 = 0;
int xAxis = A0,     yAxis = A1;
int prePress = 1;
int isPress = 0;

// // int ledState1 = LOW;         // the current state of the output pin
// int buttonState;             // the current reading from the input pin
// int lastButtonState = LOW;   // the previous reading from the input pin
// double duration = 0;
// unsigned long lastDebounceTime = 0;  // the last time the output pin was toggled
// unsigned long debounceDelay = 50;    // the debounce time; increase if the output flickers

int ledState1 = LOW;         // the current state of the output pin
int ledState2 = LOW;         // the current state of the output pin
int ledState3 = LOW;         // the current state of the output pin
int buttonState1;             // the current reading from the input pin
int buttonState2;             // the current reading from the input pin
int buttonState3;             // the current reading from the input pin
int lastButtonState1 = LOW;   // the previous reading from the input pin
int lastButtonState2 = LOW;   // the previous reading from the input pin
int lastButtonState3 = LOW;   // the previous reading from the input pin
// the following variables are unsigned longs because the time, measured in
// milliseconds, will quickly become a bigger number than can be stored in an int.
unsigned long lastDebounceTime1 = 0;  // the last time the output pin was toggled
unsigned long lastDebounceTime2 = 0;  // the last time the output pin was toggled
unsigned long lastDebounceTime3 = 0;  // the last time the output pin was toggled
unsigned long debounceDelay = 50;    // the debounce time; increase if the output flickers

int dinoX = 0;
int dinoY = 0;
int cactX = 0;
int cactY = 0;
int egg1X = 0, egg1Y = 0;
int egg2X = 0, egg2Y = 0;
int egg3X = 0, egg3Y = 0;
int cnt = 0;
int gameState = 0;
int move = 0;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  // pinMode(buttonPin1, INPUT); 

  xTaskCreate(displayTask, "display", 128, NULL, 1, &displayHandler);
  xTaskCreate(dinoTask, "dinosour", 128, NULL, 2, &dinoHandler);
  xTaskCreate(cactusTask, "cactus", 128, NULL, 2, &cactusHandler);
  
  gatekeeper = xSemaphoreCreateMutex();
  binary_sem = xSemaphoreCreateBinary();
  
  lcd.init();                
  lcd.backlight();
  lcd.createChar(0, err_dragon);
  lcd.createChar(1, cact);
  lcd.createChar(2, egg);
  lcd.createChar(3, broken);
  lcd.setCursor(0,0);

  vTaskStartScheduler();

  interrupts(); // enable all interrupts
  
  attachInterrupt(button_int1, handle_btn1, CHANGE);
}

void handle_btn1(){
  int curBtnState1 = digitalRead(buttonPin1);
  Serial.println("btn1");

  if ((millis() - lastDebounceTime1) > debounceDelay) {
    if (curBtnState1 != buttonState1) {
      buttonState1 = curBtnState1;
      lastDebounceTime1 = millis();
      if (buttonState1 == HIGH) {
        // ledState1 = HIGH;
        isPress = 1;
        Serial.println("button one down");
      } else { // the button has been just released
        // ledState1 = LOW;
        isPress = 0;
        Serial.println("button one up");
        // Serial.println(duration1); 
      }
      // updateState1(); // button state changed. It runs only once.
    }
    else {
      if(buttonState1 == HIGH){
        // digitalWrite(ledPin1, HIGH);
        isPress = 1;
      }
    }
  }
  lastButtonState1 = curBtnState1;
}

void cactusTask(){
  int dir = 0;
  cactX = 1; // up down
  cactY = 14; // left right
  for(;;){
    if(xSemaphoreTake(binary_sem, 100)){
      // Serial.println("cactusTask");
      buf[cactX][cactY] = 0;
      if(move == 1){
        dir = random(1, 4);
        if(dir == 1){
          if(cactY == 15) cactY = 0;
          else cactY += 1;
        }
        else if(dir == 2){
          if(cactY == 0) cactY = 15;
          else cactY -= 1;
        }
        else if(dir == 3 || dir == 4){
          cactX = 1 - cactX;
        }
        move = 0;
      }
      
      if(buf[cactX][cactY] == 0) buf[cactX][cactY] = 3;
      if(cnt > 0){
        if(cactX == egg1X && cactY == egg1Y ){
          buf[cactX][cactY] = 4; // broken;
          gameState = 1; // lose
        }
        else if(cactX == egg2X && cactY == egg2Y){
          buf[cactX][cactY] = 4; // broken
          gameState = 1; // lose
        }
        else if(cactX == egg3X && cactY == egg3Y){
          buf[cactX][cactY] = 4; // broken
          gameState = 1; // lose
        }
        else if(cnt == 3){
          gameState = 2; // win
        }
        else gameState = 0; // in game
      }
      vTaskPrioritySet(displayHandler, 3);
    }
    vTaskDelay(100/portTICK_PERIOD_MS); //delay 0.5 sec 
  }
}

void dinoTask(){
  int xVal = 0; // joystick x
  int yVal = 0; // joystick y
  dinoX = 0; // left right
  dinoY = 0; // up down
  // isPress = 0;
  for(;;){
    // isPress = digitalRead(buttonPin1);
    if(xSemaphoreTake(gatekeeper, 100)){
      xVal = analogRead(xAxis);
      yVal = analogRead(yAxis);
      Serial.println(isPress);
      if(isPress == 1){
        cnt++;
        Serial.println("lay egg");
        if(cnt == 1){
          egg1X = dinoX;
          egg1Y = dinoY;
          buf[egg1X][egg1Y] = 2; 
        }
        else if(cnt == 2){
          egg2X = dinoX;
          egg2Y = dinoY;
          buf[egg2X][egg2Y] = 2;
        }
        else if(cnt == 3){
          egg3X = dinoX;
          egg3Y = dinoY;
          buf[egg3X][egg3Y] = 2;
        }
        isPress = 0;
      }
      move = 1; // check if dino move
      buf[dinoX][dinoY] = 0;
      if(xVal > 700 && yVal > 300 && yVal < 700){
        if(dinoY == 15) dinoY = 15;
        else dinoY = dinoY + 1;
        buf[dinoX][dinoY] = 1; // dino
      }
      else if(xVal < 300 && yVal > 300 && yVal < 700){
        // dinoX = max(0, dinoX - 1);
        if(dinoY == 0) dinoY = 0;
        else dinoY = dinoY - 1;
        buf[dinoX][dinoY] = 1; // dino
      }
      else if(yVal < 300 && xVal > 300 && xVal < 700){
        // dinoY = min(1, dinoY + 1);
        if(dinoX == 0) dinoX = 0;
        else dinoX = dinoX - 1;
        buf[dinoX][dinoY] = 1; // dino
      }
      else if(yVal > 700 && xVal > 300 && xVal < 700){
        // dinoY = max(0, dinoY - 1);
        if(dinoX == 1) dinoX = 1;
        else dinoX = dinoX + 1;
        buf[dinoX][dinoY] = 1; // dino
      }
      else {
        move = 0;
        buf[dinoX][dinoY] = 1; // dino
      }
      if(cnt == 1){
        buf[egg1X][egg1Y] = 2; 
      }
      else if(cnt == 2){
        buf[egg1X][egg1Y] = 2; 
        buf[egg2X][egg2Y] = 2;
      }
      else if(cnt == 3){
        buf[egg1X][egg1Y] = 2; 
        buf[egg2X][egg2Y] = 2;
        buf[egg3X][egg3Y] = 2;
      }
      xSemaphoreGive(gatekeeper); 
      xSemaphoreGive(binary_sem); 
    }
    vTaskDelay(300/portTICK_PERIOD_MS); //delay 0.5 sec 
  }
}

void displayTask(){
  for(;;){
    if(xSemaphoreTake(gatekeeper, 100)){
      // Serial.println("displayTask");
      // Serial.println(gameState);
      lcd.clear();
      if(gameState == 2) { // win
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Succeed!");
        vTaskDelay(1000/portTICK_PERIOD_MS); //delay 1 sec 
        // reset everything
        cnt = 0;
        gameState = 0; // in game
        egg1X = -1; egg1Y = -1;
        egg2X = -1; egg2Y = -1;
        egg3X = -1; egg3Y = -1;
        dinoY = random(0, 15);
        dinoX = 0;
        cactY = random(0, 15);
        cactX = 1;
        prePress = 1;
        isPress = 0;
        for(int i=0; i<2; i++){
          for(int j=0; j<16; j++){
            if(i == dinoX && j == dinoY) buf[i][j] = 1;
            else if(i == cactX && j == cactY) buf[i][j] = 3;
            else buf[i][j] = 0;
          }
        }
      }
      else if(gameState == 1){ // lose
        lcd.clear();
        lcd.print("Broken!");
        vTaskDelay(1000/portTICK_PERIOD_MS); //delay 1 sec 
        lcd.clear();
        lcd.print("Game Over!");
        vTaskDelay(1000/portTICK_PERIOD_MS); //delay 1 sec 
        cnt = 0;
        gameState = 0;
        egg1X = -1; egg1Y = -1;
        egg2X = -1; egg2Y = -1;
        egg3X = -1; egg3Y = -1;
        dinoY = random(0, 15);
        dinoX = 0;
        cactY = random(0, 15);
        cactX = 1;
        prePress = 1;
        isPress = 0;
        for(int i=0; i<2; i++){
          for(int j=0; j<16; j++){
            if(i == dinoX && j == dinoY) buf[i][j] = 1;
            else if(i == cactX && j == cactY) buf[i][j] = 3;
            else buf[i][j] = 0;
          }
        }
      }
      lcd.clear();
      // else {
        for(int i=0; i<2; i++){
          for(int j=0; j<16; j++){
            if(buf[i][j] == 1){ // dino
              lcd.setCursor(j, i);
              lcd.write(0);
            }
            else if(buf[i][j] == 2){ // egg
              lcd.setCursor(j, i);
              lcd.write(2);
            }
            else if(buf[i][j] == 3){ // cactus
              // Serial.println("cactus");
              lcd.setCursor(j, i);
              lcd.write(1);
            }
            else if(buf[i][j] == 4) {// broken
              lcd.setCursor(j, i);
              lcd.write(3);
            }
          }
        // }
        vTaskPrioritySet(displayHandler, 2);
        vTaskPrioritySet(dinoHandler, 3);
      }
      vTaskPrioritySet(displayHandler, 1);
      xSemaphoreGive(gatekeeper); 
    }
    vTaskDelay(300/portTICK_PERIOD_MS); //delay 0.5 sec 
  }
}

void loop() {}
