#include <Key.h>
#include <Keypad.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#define KEY_ROWS 4
#define KEY_COLS 4 ?
// Set pins on I2C chip for LCD connections:

LiquidCrystal_I2C lcd(0x3F, 16, 2);

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
char pwd[8] = {'1', '2', '3', '4', 'x', 'x', 'x', 'x'};
int pwdLength = 4;
char enterPwd[8] =  {'x', 'x', 'x', 'x', 'x', 'x', 'x', 'x'};
int curLength = 0;
char key;

bool isEdit = false;

void setup() {
  lcd.init();   // initialize LCD
  lcd.backlight();    // open LCD backlight
  lcd.setCursor(0, 0);  // setting cursor
  lcd.print("Enter passcode:");
  Serial.begin(9600);
}

void loop() {
  LoginFun();
  delay(100);
}

void LoginFun() {

  key = myKeypad.getKey();
  if (key) {
    Serial.println(key);
    if (curLength == 0) {
      if (key == '#') {
        isEdit = true;
      }
    }
    if (isEdit) {
      SetupFun();
    } else {
      
      if (key == '#') {
        if (curLength >= 4 && curLength <= 8) {
          comparePwd();
        }
      } else {
        if (curLength <= 7) {
          enterPwd[curLength] = key;
          curLength += 1;
        }
      }
      DisplayTask();
    }
  }
}

void SetupFun() {
  lcd.clear();
  lcd.setCursor(0, 0);  // setting cursor
  lcd.print("Set passcode:");
  lcd.setCursor(0, 1);
  if (key == '#') {
    if (curLength >= 4 && curLength <= 8) {
      setPwd();
    }
  } else {
    if (curLength <= 7) {
      enterPwd[curLength] = key;
      curLength += 1;
    }
  }

  DisplayTask();
}

void DisplayTask() {
  lcd.setCursor(0, 1);  // setting cursor
  for (int i = 0; i < 8; i++) {
    if (enterPwd[i] != 'x') {
      lcd.print(enterPwd[i]);
    }
  }
}

void setPwd() {
  for (int i = 0; i < 8; i++) {
    Serial.print(enterPwd[i]);
    pwd[i] = enterPwd[i];
    enterPwd[i] = 'x';
  }
  Serial.println("Set pwd");
  for (int i = 0; i < 8; i++) {
    Serial.print(pwd[i]);
  }
  Serial.println("Set pwd finish");
  if (curLength >= 8) {
    pwdLength = 8;
  } else {
    pwdLength = curLength;
  }
  curLength = 0;
  isEdit = false;
  lcd.clear();
  lcd.setCursor(0, 0);  // setting cursor
  lcd.print("Enter passcode:");
}

void comparePwd() {

  Serial.println("Compare pwd");
  bool isSame = true;
  for (int i = 0; i < pwdLength; i++) {
    if (enterPwd[i] != pwd[i]) {
      isSame = false;
    }
  }
  if (curLength != pwdLength) {
    isSame = false;
  }

  lcd.clear();
  lcd.setCursor(0, 0);
  if (isSame) {
    lcd.print("Correct!");
  } else {
    lcd.print("Wrong!");
  }
  lcd.setCursor(0, 1);
  for (int i = 0; i < curLength; i++) {
    if (enterPwd[i] != 'x') {
      lcd.print(enterPwd[i]);
    }
  }
  for (int i = 0; i < 8; i++) {
    enterPwd[i] = 'x';
  }
  curLength = 0;
  delay(2000);
  lcd.clear();
  lcd.setCursor(0, 0);  // setting cursor
  lcd.print("Enter passcode:");
}

