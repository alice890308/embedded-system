#include <LiquidCrystal_I2C.h>
#include <Key.h>
#include <Keypad.h>
#include <Arduino_FreeRTOS.h>
#include <queue.h>
#include <stdlib.h>

#define KEY_ROWS 4
#define KEY_COLS 4

//LiquidCrystal_I2C lcd(0x3F, 16, 2);
LiquidCrystal_I2C lcd(0x27, 16, 2);
byte err_dragon[8] = {B00111, B00101, B00110, B00111,
                      B10100, B10111, B01110, B01010};
byte cact[8] = {0x04, 0x05, 0x15, 0x15, 0x16, 0x0C, 0x04, 0x04};

byte egg[8] = {0b00000, 0b01010, 0b11111, 0b11111, 0b11111, 0b01110, 0b00100, 0b00000};

byte broken[8] = {B00100, B10101, B01110, B11111, B11111, B01110, B10101, B00100};

char keymap[KEY_ROWS][KEY_COLS] = {
    {'1', '2', '3', 'A'},
    {'4', '5', '6', 'B'},
    {'7', '8', '9', 'C'},
    {'*', '0', '#', 'D'}};

byte colPins[KEY_COLS] = {9, 8, 7, 6};
byte rowPins[KEY_ROWS] = {13, 12, 11, 10};
Keypad myKeypad = Keypad(makeKeymap(keymap), rowPins,
                         colPins, KEY_ROWS, KEY_COLS);

TaskHandle_t control_h = NULL;
TaskHandle_t show_h = NULL;

QueueHandle_t dragon_pos = 0; //Global Handler
QueueHandle_t drop_egg = 0;
QueueHandle_t cacti_pos = 0;

void control(void *pvParameters);
void show(void *pvParameters);

void setup()
{
    // put your setup code here, to run once:
    Serial.begin(9600);

    xTaskCreate(control, "control", 128, NULL, 1, &control_h);
    xTaskCreate(show, "show", 128, NULL, 1, &show_h);

    dragon_pos = xQueueCreate(10, sizeof(int));
    drop_egg = xQueueCreate(10, sizeof(int));
    cacti_pos = xQueueCreate(10, sizeof(int));

    lcd.init();
    lcd.backlight();
    lcd.createChar(0, err_dragon);
    lcd.createChar(1, cact);
    lcd.createChar(2, egg);
    lcd.createChar(3, broken);
    lcd.setCursor(0, 0);

    vTaskStartScheduler();
}

void control(void *pvParameters)
{
    (void) pvParameters;
    int x = 0, y = 0;
    int c_x = 5, c_y = 0;
    int egg = 0;
    int r = 0;

    while (1)
    {
        char key = myKeypad.getKey();
        if (key)
        {
            if (key == '2')
            {
                x = max(0, x - 1);
            }
            else if (key == '8')
            {
                x = min(1, x + 1);
            }
            else if (key == '4')
            {
                y = max(0, y - 1);
            }
            else if (key == '6')
            {
                y = min(15, y + 1);
            }
            if (key == '5')
            {
                egg = 1;
            }
            else {
                egg = 0;
            }

            r = (rand() % 3);
            if (r == 0) { // left
                if (c_x <= 0) {
                    c_x += 1;
                } else {
                    c_x -= 1;
                }
            }
            else if (r == 1) { // right
                if (c_x >= 15) {
                    c_x = 14;
                } else {
                    c_x += 1;
                }
            }
            else if (r == 2) {
                if (c_y == 0) {
                    c_y = 1;
                }
                else {
                    c_y = 0;
                }
            }
            xQueueSend(dragon_pos, &x, 1000);
            xQueueSend(dragon_pos, &y, 1000);
            xQueueSend(drop_egg, &egg, 1000);
            xQueueSend(cacti_pos, &c_x, 1000);
            xQueueSend(cacti_pos, &c_y, 1000);
            
        }
        
       
    }
}

void show(void *pvParameters)
{
    (void) pvParameters;
    int rx_x = 0, rx_y = 0;
    int ca_x = 0, ca_y = 0;
    int d_egg = 0;
    int egg_num = 0;
    int egg_place[3][2];
    int lose = 0;
    int reset = 0;
    int draw_lcd = 0;

    while (1)
    {
        if (xQueueReceive(dragon_pos, &rx_x, 1000) && xQueueReceive(dragon_pos, &rx_y, 1000)){
            draw_lcd = 1;
            //Serial.print("receive rx x:");
            //Serial.println(rx_x);  
            //Serial.println("receive ry y:");
            //Serial.println(rx_y);  
        }
        if (xQueueReceive(cacti_pos, &ca_y, 1000) && xQueueReceive(cacti_pos, &ca_x, 1000)) {
            draw_lcd = 1;
            //Serial.println("receive ca x:");
            //Serial.println(ca_x); 
            //Serial.println("receive ca y:");
            //Serial.println(ca_y);   
        }
        if (xQueueReceive(drop_egg, &d_egg, 1000)) {
            if (d_egg == 1) {
                d_egg = 0;
                egg_place[egg_num][0] = rx_x;
                egg_place[egg_num][1] = rx_y;
                egg_num++;
                //Serial.println("lay egg");
            }
        }
        //Serial.println(draw_lcd);
        if (draw_lcd) {
            draw_lcd = 0;
            lcd.clear();
            lcd.setCursor(rx_y, rx_x);
            if (d_egg) {
                lcd.write(2);
            } else {
                lcd.write(0);
            }
            lcd.setCursor(ca_y, ca_x);
            lcd.write(1);
            for (int i = 0; i < egg_num; i++) {
                lcd.setCursor(egg_place[i][1], egg_place[i][0]);
                if (egg_place[i][0] == ca_x && egg_place[i][1] == ca_y) {
                    lcd.write(3);
                    vTaskDelay(500/portTICK_PERIOD_MS);
                    lose = 1;
                } else {
                    lcd.write(2);
                }
            }
        }
        //Serial.println("egg num:");
        //Serial.println(egg_num); 
        if (lose == 1) {
            lcd.clear();
            lcd.setCursor(0, 0);
            lcd.print("Fail");
            lose = 0;
            reset = 1;
        }
        else if (egg_num == 3) {
            vTaskDelay(500/portTICK_PERIOD_MS);
            lcd.clear();
            lcd.setCursor(0, 0);
            lcd.print("Success");
            reset = 1;
        }
        
        if (reset == 1) {
            //Serial.println("in reset");
            reset = 0;
            egg_num = 0;
            for (int i = 0; i < 3; i++) {
                egg_place[i][0] = 0;
                egg_place[i][1] = 0;
            }
            vTaskDelay(2000/portTICK_PERIOD_MS);
            lcd.clear();
            lcd.setCursor(rx_y, rx_x);
            lcd.write(0);
            lcd.setCursor(ca_y, ca_x);
            lcd.write(1);
        }

        vTaskDelay(10 / portTICK_PERIOD_MS); //delay 3 sec
    }
}

void loop(){}