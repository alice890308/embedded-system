#include <Arduino_FreeRTOS.h>

int pins[8] = {2, 3, 4, 5, 6, 7, 13, 12};//pins to 7-seg.
boolean data[10][7] = { 
// define the pins to light the 3 number: 0, 1, 2
  {true, true, true, true, true,true, false}, // 0
  {false, true, true, false, false, false, false}, //1
  {true, true, false, true, true, false,true}, //2
  {true, true, true, true, false, false, true}, //3
  {false, true, true, false, false, true, true}, //4
  {true, false, true, true, false, true, true}, //5
  {true, false, true, true, true, true, true}, //6
  {true, true, true, false, false, false, false}, //7
  {true, true, true, true, true, true, true}, //8
  {true, true, true, true, false, true, true} // 9
};

const int photo1 = A0, photo2 = A1;
int photo1_val ,photo2_val; 
int speed = 0;

void handle_photo1(void *pvParameters);
void handle_photo2(void *pvParameters);
void cal_speed(void *pvParameters);

void setup(){
    Serial.begin(9600);
    while (!Serial) {
        ; // wait for serial port to connect.
    }
    xTaskCreate (handle_photo1, "photo1", 128, NULL, 1, NULL);
    xTaskCreate(handle_photo2, "photo2", 128, NULL, 1, NULL);
    xTaskCreate(cal_speed, "Number", 128,NULL, 1, NULL);
}

void loop(){}

void handle_photo1(void *pvParameters)  {
  (void) pvParameters;
  pinMode(photo1, INPUT);

  for (;;) { // A Task shall never return or exit.
    photo1_val = analogRead(photo1);
  }
}

void handle_photo2(void *pvParameters)  {
  (void) pvParameters;
  pinMode(photo2, INPUT);

  for (;;) { // A Task shall never return or exit.
    photo2_val = analogRead(photo2);
  }
}

void cal_speed(void *pvParameters)  {
  (void) pvParameters;

  for (;;) { // A Task shall never return or exit.
    // to display the number 0, 1, or 2
    if (photo1_val < 600 && photo2_val > 600) { // only photo1 dark, speed down
        if (speed > 0) {
            speed--;
        }
    }
    if (photo1_val > 600 && photo2_val < 600) { //onlyt photo2 dark, speed up
        if (speed < 9) {
            speed++;
        }
    }
    for(int i = 0;i < 8;i++){
        digitalWrite(pins[i],data[speed][i] == true? HIGH : LOW);
    }
    Serial.println(speed);
    vTaskDelay(100); // wait for 1000 ticks
  }
}