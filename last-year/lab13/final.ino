#include "DHT.h"
#include "Servo.h"
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#define dhtPin 8      //讀取DHT11 Data
#define dhtType DHT11 //選用DHT11   

LiquidCrystal_I2C lcd(0x27, 16, 2);
DHT dht(dhtPin, dhtType); // Initialize DHT sensor
int led_green = 5 , led_blue = 6 , led_red = 7;
int x=3;//
int buzzer = 4; // 用Pin8 輸出方波至蜂鳴器
Servo myservo; // 建立Servo物件，控制伺服馬達

void setup() {
  Serial.begin(9600);//設定9600
  pinMode(x, INPUT);//輸入麥克風
  lcd.init();   // initialize LCD
  lcd.backlight();    // open LCD backlight
  lcd.setCursor(0, 0);  // setting cursor
  lcd.print("start");
  dht.begin();//啟動DHT
  myservo.attach(9); // 連接數位腳位9，伺服馬達的訊號線
  analogWrite(led_green, 10);
  analogWrite(led_red, 100);
  analogWrite(led_blue, 10);
  digitalWrite(led_green, LOW);
  digitalWrite(led_red, LOW);
  digitalWrite(led_blue, LOW);
}

void loop() {
  int x1=analogRead(x);//讀取麥克風值
  float h = dht.readHumidity();//讀取濕度
  float t = dht.readTemperature();//讀取攝氏溫度
  float f = dht.readTemperature(true);//讀取華氏溫度
  if (isnan(h) || isnan(t) || isnan(f)) {
    Serial.println("無法從DHT傳感器讀取！");
    return;
  }
  lcd.clear();
  lcd.setCursor(0, 0);
  if(h>65 && t>29){
    digitalWrite(led_green, LOW);
    digitalWrite(led_red, HIGH);
    digitalWrite(led_blue, LOW);
    lcd.print("wet & hot!!!");
  }
  else if(h>65){
    digitalWrite(led_green, LOW);
    digitalWrite(led_red, LOW);
    digitalWrite(led_blue, HIGH);
    lcd.print("wet!");
  }
  else if(t>29){
    digitalWrite(led_green, HIGH);
    digitalWrite(led_red, LOW);
    digitalWrite(led_blue, LOW);
    lcd.print("hot!");
  }
  else{
    lcd.print("comfortable!");
  }

  if(x1 < 300){
    tone(buzzer,100);
  }
  else{
    noTone(buzzer);
  }
    
  Serial.print("聲音: ");
  Serial.println(x1);
  Serial.print("濕度: ");
  Serial.print(h);
  Serial.print("%\t");
  Serial.print("攝氏溫度: ");
  Serial.print(t);
  Serial.print("*C");
  Serial.print("%\t");
  Serial.print("華氏溫度: ");
  Serial.print(f);
  Serial.print("*F\n");
  delay(5000);//延時5秒
}
