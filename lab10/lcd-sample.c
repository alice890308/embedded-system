#include <Wire.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2c lcd(0x27, 16, 2);

void setup() {
    lcd.init();
    lcd.backlight();
    lcd.setCursor(0, 0);
    lcd.print("Hello, world!");
    delay(1000);
    lcd.clear();
    lcd.setCursor(0, 1);
    lcd.print("Type to desplay");
}

void loop()
{}