#include <Arduino_FreeRTOS.h>
#include <Key.h>
#include <Keypad.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <string.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);

#define KEY_ROWS 4
#define KEY_COLS 4
#define enter 0
#define set 1

char keymap[KEY_ROWS][KEY_COLS] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};

// Column pin 1~4
byte colPins[KEY_COLS] = {9, 8, 7, 6};
// Column pin 1~4
byte rowPins[KEY_ROWS] = {13, 12, 11, 10};
Keypad myKeypad = Keypad(makeKeymap(keymap), rowPins,
                         colPins, KEY_ROWS, KEY_COLS);

char key;
char words[20] = "Enter passcode:";
char str[16];
char saved[16] = "1234";
int index = 0;
int state = 0;
volatile int lf_flag = 1;
volatile int sf_flag = 0;
volatile int dt_flag = 0;

TaskHandle_t LF = NULL;
TaskHandle_t sF = NULL;
TaskHandle_t DT = NULL;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);

  xTaskCreate(LoginFun, (const portCHAR *)"LF", 128, NULL, 1, &LF);
  xTaskCreate(setupFun, (const portCHAR *)"sF", 128, NULL, 1, &sF);
  xTaskCreate(DisplayTask, (const portCHAR *)"DT", 128, NULL, 1, &DT);
  lf_flag = 1;
  sf_flag = 0;
  dt_flag = 0;

  lcd.init();    // initialize LCD
  lcd.backlight();    // open LCD backlight
  lcd.setCursor(0, 0);  // setting cursor
  lcd.print("Enter passcode:");
  //vTaskSuspend(sF);
  vTaskStartScheduler();
}

void LoginFun(void *pvParameters) {
  (void) pvParameters;
  for ( ;; ) // A Task shall never return or exit.
  {
    //Serial.println("TEST1");
    while (!lf_flag) {}
    key = myKeypad.getKey();
    Serial.println(saved);
    if (key) {
      if (key == '#') {
        lcd.clear();
        memset(words, 0, sizeof(words));
        if (state == 0) {
          if (index == 0) {
            strcpy(words, "Set passcode:");
            state = 1;
          }
          else {
            if (!strcmp(str, saved))
              strcpy(words, "Correct!");
            else
              strcpy(words, "Wrong!");
            //vTaskResume(DT);
            //vTaskSuspend(LF);
            lf_flag = 0;
            dt_flag = 1;
            while (!lf_flag) {}
            //vTaskPrioritySet(DT, (uxTaskPriorityGet(NULL) + 1));
            delay(2000);
            lcd.clear();
            memset(words, 0, sizeof(words));
            strcpy(words, "Enter passcode:");
          }
        }
        else {
          if (index >= 4 && index <= 8)
            strcpy(saved, str);
          strcpy(words, "Enter passcode:");
          state = 0;
        }
        index = -1;
        memset(str, 0, sizeof(str));
        //vTaskResume(DT);
        //vTaskSuspend(LF);
        lf_flag = 0;
        dt_flag = 1;
        while (!lf_flag) {}
        //vTaskPrioritySet(DT, (uxTaskPriorityGet(NULL) + 1));
      }
      else {
        if (state == 0) {
          if (index <= 7)  str[index] = key;
          //vTaskResume(DT);
          //vTaskSuspend(LF);
          lf_flag = 0;
          dt_flag = 1;
          while (!lf_flag) {}
          //vTaskPrioritySet(DT, (uxTaskPriorityGet(NULL) + 1));
        }
        else
        {
          //vTaskResume(sF);
          //vTaskSuspend(LF);
          lf_flag = 0;
          sf_flag = 1;
          while (!lf_flag) {}
          //vTaskPrioritySet(sF, (uxTaskPriorityGet(NULL) + 1));
        }
      }
      index = min(8,index+1);
    }
  }
}

void setupFun(void *pvParameters) {
  (void) pvParameters;
  for ( ;; ) // A Task shall never return or exit.
  {

    while (!sf_flag) {
      Serial.println(saved);
    }
    if (index <= 7) str[index] = key;
    //vTaskResume(DT);
    //vTaskSuspend(sF);
    sf_flag = 0;
    dt_flag = 1;
    //vTaskPrioritySet(DT, (uxTaskPriorityGet(NULL) + 1));
    //vTaskPrioritySet(NULL, (uxTaskPriorityGet(NULL) - 1));
  }
}

void DisplayTask(void *pvParameters) {
  (void) pvParameters;

  for ( ;; ) // A Task shall never return or exit.
  {

    while (!dt_flag) {}
    lcd.setCursor(0, 0);  // setting cursor
    lcd.print(words);
    lcd.setCursor(0, 1);
    lcd.print(str);
    dt_flag = 0;
    lf_flag = 1;
    //vTaskResume(LF);
    //vTaskSuspend(DT);

    //vTaskPrioritySet(NULL, 1);
  }
}
void loop() {

}
