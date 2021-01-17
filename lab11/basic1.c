#include <Key.h>
#include <Keypad.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Arduino_FreeRTOS.h>
#include <queue.h>

LiquidCrystal_I2C lcd(0x3F, 16, 2);
byte dinosaur[8] = {B00111, B00101, B00110, B00111, B10100, B10111, B01110, B01010};

char keymap[4][4] = {
    {'1', '2', '3', 'A'},
    {'4', '5', '6', 'B'},
    {'7', '8', '9', 'C'},
    {'*', '0', '#', 'D'}};
// Column pin 1~4=
byte colPins[4] = {9, 8, 7, 6};
// Column pin 1~4
byte rowPins[4] = {13, 12, 11, 10};
Keypad myKeypad = Keypad(makeKeymap(keymap), rowPins, colPins, 4, 4);

TaskHandle_t cacti_handle;
TaskHandle_t lcd_handle;
QueueHandle_t queue = 0;

void display(void *pvParameters);
void control(void *pvParameters);

void setup()
{
    lcd.init();      // initialize LCD
    lcd.backlight(); // open LCD backlight
    lcd.createChar(1, dinosaur);
    xTaskCreate(display, "displayTask", 128, NULL, 1, &lcd_handle);
    xTaskCreate(control, "controlTask", 64, NULL, 1, NULL);
    queue = xQueueCreate(10, sizeof(int));
    Serial.begin(9600);
}

void loop() {}

void control(void *pvParameters)
{
    (void)pvParameters;
    int y = 0;
    int x = 0;

    for (;;)
    {
        char key = myKeypad.getKey();
        if (key == '2')
        {
            y = 0;
        }
        else if (key == '8')
        {
            y = 1;
        }
        else if (key == '4')
        {
            if (x > 0)
            {
                x--;
            }
        }
        else if (key == '6')
        {
            if (x < 15)
            {
                x++;
            }
        }
        if (!xQueueSend(queue, &x, 1000))
            Serial.println("Failed to send to queue");
        if (!xQueueSend(queue, &y, 1000))
            Serial.println("Failed to send to queue");
    }
}

void display(void *pvParameters)
{
    (void)pvParameters;
    int x = 0;
    int y = 0;

    for (;;)
    { // A Task shall never return or exit.
        //    Serial.println(cacti1_pos[1]);
        if (xQueueReceive(queue, &x, 1000))
        {
            Serial.println("receive value:");
            Serial.println(x);
        }
        else
            Serial.println("Failed to receive from queue");

        if (xQueueReceive(queue, &y, 1000))
        {
            Serial.println("receive value:");
            Serial.println(y);
        }
        else
            Serial.println("Failed to receive from queue");
        lcd.clear();
        lcd.setCursor(x, y);
        lcd.write(1);
        vTaskDelay(50);
    }
}