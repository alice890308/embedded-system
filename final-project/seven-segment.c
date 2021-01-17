#include "easyTimer.h"
byte number[4] = {10, 13, 12, 11};
byte digit[2] = {9, 8};

void show(int num);

void setup()
{
  Serial.begin(9600);
  for (int i = 0; i < 4; i++)
    pinMode(number[i], 1);
  for (int i = 0; i < 2; i++)
    pinMode(digit[i], 1);
}
void loop()
{
  for(int i = 0; i < 10; i++) {
    Serial.print("show i = ");
    Serial.println(i);
    mdelay(1000) {
      show(i);
    }
  }
  int temp = 1;
  for(int i = 0; i < 4; i++) {
    Serial.print("show temp = ");
    Serial.println(temp);
    mdelay(1000) {
      show(temp);
    }
    temp *= 10;
  }
}

void show(int num) {
  int cur;
  for(int i = 0; i < 4; i++) { // 最多4 digit
    //Serial.println(i);
    cur = num % 10;
    //Serial.println(cur);
    digitalWrite(number[3],cur&8);
    digitalWrite(number[2], cur&4);
    digitalWrite(number[1],cur&2);
    digitalWrite(number[0],cur&1);
    digitalWrite(digit[1],(3-i)&2);
    digitalWrite(digit[0],(3-i)&1);
    num /= 10;
    delay(3);
  }
}