// example for 2 arduino and lcd communication
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);
int x = 0;
int flag = 0;
void setup() {
  // Start the I2C Bus as Master
  Wire.begin();
  Serial.begin(9600);
  lcd.init();
  lcd.backlight();
}
void loop() {
  Wire.beginTransmission(9); // transmit to device #9
  Wire.write(x);              // sends x 
  Wire.endTransmission();    // stop transmitting
  Serial.println("send!");
  lcd.clear();
  lcd.setCursor(0, 0);
  if (flag == 1) {
    lcd.print("one");
    flag = 0;
  }
  else {
    lcd.print("zero");
    flag = 1;
  }
  x++; // Increment x
  if (x > 5) x = 0; // `reset x once it gets 6
  delay(1000);
}