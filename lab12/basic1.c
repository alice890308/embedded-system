#include <LiquidCrystal_I2C.h>
#include <Arduino_FreeRTOS.h>
#include <semphr.h>
#include <stdlib.h>

//LiquidCrystal_I2C lcd(0x3F, 16, 2);
LiquidCrystal_I2C lcd(0x27, 16, 2);
byte err_dragon[8] = {B00111, B00101, B00110, B00111,
                      B10100, B10111, B01110, B01010};
byte cact[8] = {0x04, 0x05, 0x15, 0x15, 0x16, 0x0C, 0x04, 0x04};

byte egg[8] = {0b00000, 0b01010, 0b11111, 0b11111, 0b11111, 0b01110, 0b00100, 0b00000};

byte broken[8] = {B00100, B10101, B01110, B11111, B11111, B01110, B10101, B00100};

int button = 2;
int xAxis = A0, yAxis = A1;
int isPress = 1;

SemaphoreHandle_t  gatekeeper = 0; /* global handler */
SemaphoreHandle_t binary_sem; //Global handler

TaskHandle_t display_h = NULL;
TaskHandle_t dino_h = NULL;
TaskHandle_t cactus_h = NULL;

char buf[2][16] = {'\0'};
char flag = '\0'; // represent success or fail
char no_egg = '\0';

void displayTask(void *pvParameters);
void dinoTask(void *pvParameters);
void cactusTask(void *pvParameters);

void setup()
{
    // put your setup code here, to run once:
    Serial.begin(9600);
    pinMode(button, INPUT_PULLUP); //return LOW when down

    vSemaphoreCreateBinary(binary_sem);
    gatekeeper = xSemaphoreCreateMutex();

    xTaskCreate(displayTask, "display", 128, NULL, 1, &display_h);
    xTaskCreate(dinoTask, "dino", 256, NULL, 1, &dino_h);
    xTaskCreate(cactusTask, "cactus", 128, NULL, 1, &cactus_h);

    lcd.init();
    lcd.backlight();
    lcd.createChar(0, err_dragon);
    lcd.createChar(1, cact);
    lcd.createChar(2, egg);
    lcd.createChar(3, broken);
    lcd.setCursor(0, 0);

    vTaskStartScheduler();
}

void displayTask(void *pvParameters)
{
    (void) pvParameters;
    while(1) {
        if(xSemaphoreTake(gatekeeper, 65535)){
            lcd.clear();
            for(int i = 0; i < 2; i++) {
                for(int j = 0; j < 16; j++) {
                    if (buf[i][j] == '4') {
                        Serial.print("x=");
                        Serial.println(i);
                        Serial.print("y=");
                        Serial.println(j);
                        lcd.setCursor(j, i);
                        lcd.write(0);
                    }
                    else if (buf[i][j] == '1') {
                        lcd.setCursor(j, i);
                        lcd.write(1);
                    }
                    else if (buf[i][j] == '2') {
                        lcd.setCursor(j, i);
                        lcd.write(2);
                    }
                    else if (buf[i][j] == '3') {
                        lcd.setCursor(j, i);
                        lcd.write(3);
                    }
                    
                }
            }
            if (flag == 's') {
                vTaskDelay(500 / portTICK_PERIOD_MS); 
                lcd.setCursor(0, 0);
                lcd.clear();
                lcd.print("Succeed!");
                vTaskDelay(1000 / portTICK_PERIOD_MS);
            }
            else if (flag == 'f') {
                vTaskDelay(500 / portTICK_PERIOD_MS); 
                lcd.setCursor(0, 0);
                lcd.clear();
                lcd.print("Fail!");
                vTaskDelay(1000 / portTICK_PERIOD_MS); 
            }
            if (flag == 's' || flag == 'f') {
                flag = '\0';
                isPress = 1;
                for(int i = 0; i < 2; i++) {
                    for(int j = 0; j < 16; j++) {
                        if (buf[i][j] == '2' || buf[i][j] == '3') {
                            buf[i][j] = '\0';
                        }
                    }
                }
            }
            //Serial.println("display task");
            xSemaphoreGive(gatekeeper);
        }
        vTaskDelay(100 / portTICK_PERIOD_MS); 
    }
}

void dinoTask(void *pvParameters)
{
    (void) pvParameters;

    int xVal, yVal;
    int dino_r = 0, dino_c = 0; // row and col
    int egg_num = 0;
    isPress = 1;

    while(1) {
        xVal = analogRead(xAxis);
        yVal = analogRead(yAxis);
        isPress = digitalRead(button);
        if(xSemaphoreTake(gatekeeper, 100)){
//            Serial.println(dino_c);
//            Serial.print("x=");
//            Serial.println(xVal);
//            Serial.print("y=");
//            Serial.println(yVal);
            if (buf[dino_r][dino_c] != '2') {
                buf[dino_r][dino_c] = '\0';
            }
            if (no_egg == '1') {
                egg_num = 0;
                no_egg = '\0';
            }
            if (xVal < 20 && yVal < 650 && yVal > 450 && (dino_c > 0)) { // left 
               dino_c--;
               xSemaphoreGive(binary_sem);
            }
            else if (xVal > 980 &&  yVal < 650 && yVal > 450 && (dino_c < 15)) { //right
                dino_c++;
                xSemaphoreGive(binary_sem);
            }
            if (yVal < 20 &&  xVal < 650 && xVal > 450) { // up
                dino_r = 0;
                xSemaphoreGive(binary_sem);
            }
            else if (yVal > 980 &&  xVal < 650 && xVal > 450) { // down
                dino_r = 1;
                xSemaphoreGive(binary_sem);
            }
            if (buf[dino_r][dino_c] != '2') {
                buf[dino_r][dino_c] = '4';
            }
//            Serial.println(dino_r);
//            Serial.println(dino_c);
//            Serial.println(buf[dino_r][dino_c]);
            if (isPress == 0) { //is pressing!
                if (buf[dino_r][dino_c] == '2') {
                    xSemaphoreGive(gatekeeper);
                    continue;
                }
                else {
                    buf[dino_r][dino_c] = '2';
                    egg_num++;
                    if (egg_num == 3) {
                        flag = 's';
                        egg_num = 0;
                    }
                }
                xSemaphoreGive(binary_sem);
            }
            
            //Serial.println("no task");
            xSemaphoreGive(gatekeeper);
        }
        vTaskDelay(100 / portTICK_PERIOD_MS); 
    }
}

void cactusTask(void *pvParameters)
{
    (void) pvParameters;
    int r, c_col = 0, c_row = 0;

    while(1) {
        if(xSemaphoreTake(binary_sem, 1000)){
            if (xSemaphoreTake(gatekeeper, 100)) {
                buf[c_row][c_col] = '\0';
                r = (rand() % 3);
                if (r == 0) { // left
                    if (c_col <= 0) {
                        c_col += 1;
                    } else {
                        c_col -= 1;
                    }
                }
                else if (r == 1) { // right
                    if (c_col >= 15) {
                        c_col = 14;
                    } else {
                        c_col += 1;
                    }
                }
                else if (r == 2) {
                    if (c_row == 0) {
                        c_row = 1;
                    }
                    else {
                        c_row = 0;
                    }
                }
                if (buf[c_row][c_col] == '2') { // broken
                    flag = 'f';
                    no_egg = '1';
                    buf[c_row][c_col] = '3';
                } else {
                    buf[c_row][c_col] = '1';
                }
                //Serial.println("cactus task");
                xSemaphoreGive(gatekeeper);
            }
            
        }
        vTaskDelay(100 / portTICK_PERIOD_MS); 
    }
}

void loop(){}