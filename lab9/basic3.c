#include <Arduino_FreeRTOS.h>

int pins[8] = {2, 3, 4, 5, 6, 7, 13, 12}; //pins to 7-seg.
boolean data[10][7] = {
    // define the pins to light the 3 number: 0, 1, 2
    {true, true, true, true, true, true, false},     // 0
    {false, true, true, false, false, false, false}, //1
    {true, true, false, true, true, false, true},    //2
    {true, true, true, true, false, false, true},    //3
    {false, true, true, false, false, true, true},   //4
    {true, false, true, true, false, true, true},    //5
    {true, false, true, true, true, true, true},     //6
    {true, true, true, false, false, false, false},  //7
    {true, true, true, true, true, true, true},      //8
    {true, true, true, true, false, true, true}      // 9
};

const int photo1 = A0, photo2 = A1;
const int trigPin = 19, echoPin = 18;
long duration, distance;
int photo1_val, photo2_val;
int speed = 0;
int redPin = 9, greenPin = 10, bluePin = 11;

void handle_photo1(void *pvParameters);
void handle_photo2(void *pvParameters);
void cal_speed(void *pvParameters);
void sensor_distance(void *pvParameters);
void light_rgb(void *pvParameters);

void setup()
{
    Serial.begin(9600);
    while (!Serial)
    {
        ; // wait for serial port to connect.
    }
    xTaskCreate(handle_photo1, "photo1", 128, NULL, 1, NULL);
    xTaskCreate(handle_photo2, "photo2", 128, NULL, 1, NULL);
    xTaskCreate(cal_speed, "Number", 128, NULL, 1, NULL);
    xTaskCreate(sensor_distance, "ultra", 128, NULL, 1, NULL);
    xTaskCreate(light_rgb, "rgb", 128, NULL, 1, NULL);
}

void loop() {}

void sensor_distance(void *pvParameters)
{
    (void)pvParameters;
    pinMode(trigPin, OUTPUT);
    pinMode(echoPin, INPUT);

    for (;;)
    { // A Task shall never return or exit.
        digitalWrite(trigPin, LOW); // Clears the trigPin
        delayMicroseconds(2);
        /* Sets the trigPin on HIGH state for 10 ms */
        digitalWrite(trigPin, HIGH);
        delayMicroseconds(10);
        digitalWrite(trigPin, LOW);
        /* Reads Echo pin, returns sound travel time in ms */
        duration = pulseIn(echoPin, HIGH);
        /* Calculates the distance */
        distance = duration * 0.034 / 2;
        if (distance > 30) {
            speed = 0;
        }
        Serial.print("distance: ");
        Serial.println(distance);
        vTaskDelay(100); // wait for 1000 ticks
    }
}

void light_rgb(void *pvParameters)
{
    (void)pvParameters;
    pinMode(redPin, OUTPUT);
    pinMode(greenPin, OUTPUT);
    pinMode(bluePin, OUTPUT);

    for (;;)
    { // A Task shall never return or exit.
        if (speed < 4) { // green
            analogWrite(redPin, 0);
            analogWrite(greenPin, 150);
            analogWrite(bluePin, 0);
        } else if (speed > 7) { // red
            analogWrite(redPin, 150);
            analogWrite(greenPin, 0);
            analogWrite(bluePin, 0);
        } else { // yellow
            analogWrite(redPin, 150);
            analogWrite(greenPin, 150);
            analogWrite(bluePin, 0);
        }
    }
}

void handle_photo1(void *pvParameters)
{
    (void)pvParameters;
    pinMode(photo1, INPUT);

    for (;;)
    { // A Task shall never return or exit.
        photo1_val = analogRead(photo1);
    }
}

void handle_photo2(void *pvParameters)
{
    (void)pvParameters;
    pinMode(photo2, INPUT);

    for (;;)
    { // A Task shall never return or exit.
        photo2_val = analogRead(photo2);
    }
}

void cal_speed(void *pvParameters)
{
    (void)pvParameters;

    for (;;)
    { // A Task shall never return or exit.
        // to display the number 0, 1, or 2
        if (photo1_val < 800 && photo2_val > 800)
        { // only photo1 dark, speed down
            if (speed > 0)
            {
                speed--;
            }
        }
        if (photo1_val > 800 && photo2_val < 800)
        { //onlyt photo2 dark, speed up
            if (speed < 9)
            {
                speed++;
            }
        }
        for (int i = 0; i < 8; i++)
        {
            digitalWrite(pins[i], data[speed][i] == true ? HIGH : LOW);
        }
        Serial.println(speed);
        Serial.print("photo1");
        Serial.println(photo1_val);
        Serial.print("photo2");
        Serial.println(photo2_val);
        vTaskDelay(100); // wait for 1000 ticks
    }
}