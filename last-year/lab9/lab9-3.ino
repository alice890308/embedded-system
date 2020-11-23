#include <Arduino_FreeRTOS.h>
#include<Stepper.h>

#define STEPS 2048

Stepper stepper(STEPS,8,10,9,11);

int pins[8] = {2, 3, 4, 5, 6, 7, 13, 12};//pins to 7-seg.
boolean data[3][8] = { 
// define the pins to light the 3 number: 0, 1, 2
  {true, true, true, true, true,true, false}, // 0
  {false, true, true, false, false, false, false}, //1
  {true, true, false, true, true, false,true}
};

const int ohm1Pin = A0, ohm2Pin = A1;
const int trigPin = 17,echoPin = 16;
int ohm1,ohm2,angle = 0,lastAngle; 
long duration,carDistance;

void saveOhm1(void *pvParameters);
void saveOhm2(void *pvParameters);
void printNumber(void *pvParameters);
void motor(void *pvParameters);
void distance(void *pvParameters);

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
    xTaskCreate(motor, (const portCHAR *)"Motor", 128,NULL, 1, NULL);
    xTaskCreate(distance, (const portCHAR *)"Distance", 128,NULL, 1, NULL);

}

void loop(){


     

}
void saveOhm1(void *pvParameters)  {
  (void) pvParameters;
  pinMode(ohm1Pin, INPUT);

  for (;;) { // A Task shall never return or exit.
    ohm1 = analogRead(ohm1Pin);
    delayMicroseconds(20);
  }
}

void saveOhm2(void *pvParameters)  {
  (void) pvParameters;
  pinMode(ohm2Pin, INPUT);

  for (;;) { // A Task shall never return or exit.
    ohm2 = analogRead(ohm2Pin);
    delayMicroseconds(20);
  }
}

void printNumber(void *pvParameters)  {
  (void) pvParameters;

  for (;;) { // A Task shall never return or exit.
    // to display the number 0, 1, or 2
    for(int i = 0;i < 8;i++){
        int number;
        if(ohm1 < 400 && ohm2 < 400) number = 0;
        else if(ohm1 < 400 || ohm2 < 400) number = 1;
        else number = 2;
        digitalWrite(pins[i],data[number][i] == true? HIGH : LOW);
        delayMicroseconds(20);
    }
  }
}

void motor(void *pvParameters){
    (void) pvParameters;
    stepper.setSpeed(15);
    for(;;){
      if(carDistance > 15){
      Serial.println(carDistance);
        angle = 0;
        stepper.step(angle - lastAngle);
        lastAngle = angle;
    }
    else {
      if(carDistance < 15 && ohm1 < 500 && ohm2 < 500){
        Serial.println(carDistance);
          angle = 1024;
          stepper.step(angle - lastAngle);
          lastAngle = angle;
      }
      else if(carDistance < 15 && ohm1 < 500){
        Serial.println(carDistance);
          angle = -512;
          stepper.step(angle - lastAngle);
          lastAngle = angle;
      }
      else if(carDistance < 15 && ohm2 < 500){
        Serial.println(carDistance);
          angle = 512;
          stepper.step(angle - lastAngle);
          lastAngle = angle;
      }
      else if(carDistance < 15){
        Serial.println(carDistance);
          angle = -512;
          stepper.step(angle - lastAngle);
          lastAngle = angle;
      }
    }
    }
    
}

void distance(void *pvParameters){
    (void) pvParameters;
    pinMode(trigPin, OUTPUT);
    pinMode(echoPin, INPUT);

    for(;;){
        digitalWrite(trigPin, LOW); // Clears the trigPin
        delayMicroseconds(2);
        /* Sets the trigPin on HIGH state for 10 ms */
        digitalWrite(trigPin, HIGH);    
        delayMicroseconds(10);
        digitalWrite(trigPin, LOW);
        /* Reads Echo pin, returns sound travel time in ms */
        duration = pulseIn(echoPin, HIGH);
        /* Calculating the distance */
        carDistance = duration*0.034/2;
    }
} 
