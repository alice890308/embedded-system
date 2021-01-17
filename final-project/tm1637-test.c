#include "TM1637.h"

#define CLK 8//pins definitions for TM1637 and can be changed to other ports
#define DIO 7

int8_t TimeDisp[] = {0x00,0x00,0x00,0x00};
TM1637 tm1637(CLK,DIO);

void TimeUpdate();

void setup()
{
  tm1637.set();
  tm1637.init();

}

void loop()
{
    TimeUpdate();
    tm1637.display(TimeDisp);
    delay(1000);
}

void TimeUpdate()
{
    if (TimeDisp[3] == 9) {
        TimeDisp[3] = 0;
    } else {
        TimeDisp[3] += 1;
    }
    if (TimeDisp[2] == 9) {
        TimeDisp[2] = 0;
    } else {
        TimeDisp[2] += 1;
    }
    if (TimeDisp[1] == 9) {
        TimeDisp[1] = 0;
    } else {
        TimeDisp[1] += 1;
    }
    if (TimeDisp[0] == 9) {
        TimeDisp[0] = 0;
    } else {
        TimeDisp[0] += 1;
    }
}