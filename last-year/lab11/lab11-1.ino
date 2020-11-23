#include <LiquidCrystal_I2C.h>
#include <Key.h>
#include <Keypad.h>
#include <Arduino_FreeRTOS.h>
#include <queue.h>

#define KEY_ROWS 4 
#define KEY_COLS 4

LiquidCrystal_I2C lcd(0x3F,16,2);
byte err_dragon[8] = {B00111, B00101, B00110, B00111,
        B10100, B10111, B01110, B01010};

char keymap[KEY_ROWS][KEY_COLS] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};

byte colPins[KEY_COLS] = {9, 8, 7, 6};
byte rowPins[KEY_ROWS] = {13, 12, 11, 10}; 
Keypad myKeypad = Keypad(makeKeymap(keymap), rowPins,
                  colPins, KEY_ROWS, KEY_COLS);

TaskHandle_t control_h = NULL;
TaskHandle_t show_h = NULL;

QueueHandle_t Global_Queue_Handle = 0; //Global Handler

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);

  xTaskCreate(control, "control", 128, NULL, 1, &control_h);
  xTaskCreate(show, "show", 128, NULL, 1, &show_h);

  Global_Queue_Handle = xQueueCreate(10, sizeof(int));
  
  lcd.init();                
  lcd.backlight();
  lcd.createChar(0, err_dragon);
  lcd.setCursor(0,0);

  vTaskStartScheduler();
}

void control(){
  int x = 0, y = 0;
  
  while(1){
    char key = myKeypad.getKey();
    if(key){
      if(key == '2'){
        x = max(0, x-1);
      }
      else if(key == '8'){
        x = min(1, x+1);
      }
      else if(key == '4'){
        y = max(0, y-1);
      }
      else if(key == '6'){
        y = min(15, y+1);
      }
    }
    
    if(!xQueueSend(Global_Queue_Handle, &x, 1000))
      Serial.println("Failed to send to queue");    
    if(!xQueueSend(Global_Queue_Handle, &y, 1000))
      Serial.println("Failed to send to queue");  
  }
}

void show(){
  int rx_x = 0, rx_y = 0;
  while(1){
    if(xQueueReceive(Global_Queue_Handle,&rx_x,1000)){
      Serial.println("receive value:");
      Serial.println(rx_x);   
    }
    else 
     Serial.println("Failed to receive from queue");  

    if(xQueueReceive(Global_Queue_Handle,&rx_y,1000)){
      Serial.println("receive value:");
      Serial.println(rx_y);   }
    else 
     Serial.println("Failed to receive from queue");

    lcd.clear();
    lcd.setCursor(rx_y, rx_x);
    lcd.write(0);

    vTaskDelay(10/portTICK_PERIOD_MS); //delay 3 sec 
  }
}

void loop() {
  // put your main code here, to run repeatedly:
}
