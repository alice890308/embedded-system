#include <Arduino_FreeRTOS.h>

int pins[8] = {2, 3, 4, 5, 6, 7, 13, 12};//pins to 7-seg.
boolean data[3][8] = { 
// define the pins to light the 3 number: 0, 1, 2
  {true, true, true, true, true,true, false}, // 0
  {false, true, true, false, false, false, false}, //1
  {true, true, false, true, true, false,true}
};

const int ohm1Pin = A0, ohm2Pin = A1;
int ohm1,ohm2; 

void saveOhm1(void *pvParameters);
void saveOhm2(void *pvParameters);
void printNumber(void *pvParameters);

void setup(){
    Serial.begin(9600);
    while (!Serial) {
        ; // wait for serial port to connect.
    }
    xTaskCreate(saveOhm1, (const portCHAR *)"Ohm1", 128,
       NULL, 1, NULL);
    xTaskCreate(saveOhm2, (const portCHAR *)"Ohm2", 128,
       NULL, 1, NULL);
    xTaskCreate(printNumber, (const portCHAR *)"Number", 128,NULL, 1, NULL);
}

void loop(){


     

}
void saveOhm1(void *pvParameters)  {
  (void) pvParameters;
  pinMode(ohm1Pin, INPUT);

  for (;;) { // A Task shall never return or exit.
    ohm1 = analogRead(ohm1Pin);
  }
}

void saveOhm2(void *pvParameters)  {
  (void) pvParameters;
  pinMode(ohm2Pin, INPUT);

  for (;;) { // A Task shall never return or exit.
    ohm2 = analogRead(ohm2Pin);
  }
}

void printNumber(void *pvParameters)  {
  (void) pvParameters;

  for (;;) { // A Task shall never return or exit.
    // to display the number 0, 1, or 2
    for(int i = 0;i < 8;i++){
        int number;
        if(ohm1 < 500 && ohm2 < 500) number = 0;
        else if(ohm1 < 500 || ohm2 < 500) number = 1;
        else number = 2;
        digitalWrite(pins[i],data[number][i] == true? HIGH : LOW);
    }
  }
}
