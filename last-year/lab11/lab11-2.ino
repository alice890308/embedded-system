#include <LiquidCrystal_I2C.h>
#include <Key.h>
#include <Keypad.h>
#include <Arduino_FreeRTOS.h>
#include <queue.h>

#define KEY_ROWS 4
#define KEY_COLS 4

LiquidCrystal_I2C lcd(0x3F,16,2);
byte err_dragon[8] = {B00111, B00101, B00110,
        B00111, B10100, B10111, B01110, B01010};
byte minesweeper[8] = {B01110, B01110, B10100,
        B11111, B00101, B01110, B01010, B11011};
byte unchecked[8] = {B11111, B11111, B11111,
        B11111, B11111, B11111, B11111, B11111};
byte exploded[8] = {B00100, B10101, B01110,
        B11111, B11111, B01110, B10101, B00100};
byte succeeded[8] = {B00100, B00110, B00111,
        B00110, B00100, B00100, B01110, B11111};

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
TaskHandle_t map_h = NULL;

QueueHandle_t game_state, gen_map; //Global Handler

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  randomSeed(analogRead(A0));

  xTaskCreate(control, "control", 128, NULL, 1, &control_h);
  xTaskCreate(show, "show", 128, NULL, 1, &show_h);
  xTaskCreate(createmap, "createmap", 128, NULL, 2, &map_h);

  game_state = xQueueCreate(12, sizeof(int));
  gen_map = xQueueCreate(5, sizeof(int)*2);

  lcd.init();
  lcd.backlight();
  lcd.createChar(0, minesweeper);
  lcd.createChar(1, succeeded);
  lcd.createChar(2, exploded);
  lcd.createChar(3, unchecked);

  int start = 0;
  xQueueSend(game_state, &start, 100);
  xQueueSend(game_state, &start, 100);
  xQueueSend(game_state, &start, 100);

  vTaskStartScheduler();
}

void control(void *pvParameters){
  (void) pvParameters;
  int mine_pos[2]={0}, miner[2]={0};
  int condition = 0, remover = 10;

  while(1){
    char key = myKeypad.getKey();

    if(key){
      xQueuePeek(gen_map, &mine_pos, 100);

      //Serial.println(condition);
      condition = 0;

      if(key == '2'){
        if( mine_pos[0]==miner[0] && mine_pos[1]==miner[1] && miner[0]!=0)
            condition = 5; //loss
        else
          miner[0] = max(0, miner[0]-1);
      }
      else if(key == '8'){
        if( mine_pos[0]==miner[0] && mine_pos[1]==miner[1] && miner[0]!=1)
            condition = 5; //loss
        else
          miner[0] = min(1, miner[0]+1);
      }
      else if(key == '4'){
        if( mine_pos[0]==miner[0] && mine_pos[1]==miner[1] && miner[1]!=0)
            condition = 5; //loss
        else
          miner[1] = max(0, miner[1]-1);
      }
      else if(key == '6'){
        if( mine_pos[0]==miner[0] && mine_pos[1]==miner[1] && miner[1]!=15)
            condition = 5; //loss
        else
          miner[1] = min(15, miner[1]+1);
      }
      else if(key == '5'){
        if(remover -- > 0){
          if( mine_pos[0]==miner[0] && mine_pos[1]==miner[1])
            condition = 4; // win
          else if(remover == 0)
            condition = 3; // no remover
          else if(abs(mine_pos[0]-miner[0]) + abs(mine_pos[1]-miner[1]) <= 3 )
            condition = 1; // "1"
          else
            condition = 2; // "0"
        }
        else
          condition = 3; // no remover
      }
      lcd.clear();
      xQueueSend(game_state, &miner[0], 100);
      xQueueSend(game_state, &miner[1], 100);
      xQueueSend(game_state, &condition, 100);

      if(condition > 2){
        miner[0] = miner[1] = 0;
        remover = 10;
      }
      vTaskResume(show_h);
      vTaskSuspend(NULL);
    }
  }
}

void show(void *pvParameters){
  (void) pvParameters;
  int rx_x = 0, rx_y = 0, condition = 0;
  char mine_map[2][16]={'0'};
  int mine_pos[2]={0};
  while(1){
    xQueueReceive(game_state,&rx_x,100);
    xQueueReceive(game_state,&rx_y,100);
    xQueueReceive(game_state,&condition,100);
    Serial.println(condition);
    if(condition > 2){
      lcd.setCursor(rx_y,rx_x);
      if(condition == 4) lcd.write(1);
      else if(condition == 5) lcd.write(2);
      xQueueReceive(gen_map,&mine_pos,100);
      rx_x=0; rx_y=0;
      memset(mine_map, '0', sizeof(mine_map));
      if(condition > 3) delay(1000);
      condition = 0;
      vTaskResume(map_h);
    }
    for(int k = 0; k<2; k++){
      for(int i = 0; i<2; i++){
        for(int j = 0; j<16; j++){
          lcd.setCursor(j,i);

          if(i == rx_x && j == rx_y){
            if(condition == 1 && !k){
              mine_map[i][j] = '1';
              lcd.write('1');
            }
            else if(condition == 2 && !k){
              mine_map[i][j] = '2';
              lcd.write('0');
            }
            else
              lcd.write(0);
          }
          else{
            if(mine_map[i][j] == '1' && condition < 4)
              lcd.write('1');
            else if(mine_map[i][j] == '2' && condition < 4)
              lcd.write('0');
            else if(condition < 4)
              lcd.write(3);
          }
        }
      }

      if(!k && condition)
        delay(1000);
    }

    condition = 0;
    vTaskResume(control_h);
    vTaskSuspend(NULL);
  }
}

void createmap(void *pvParameters){
  (void) pvParameters;
  int mine_pos[2] = {0};

  while(1){
    mine_pos[0] = random(0, 2);
    mine_pos[1] = random(0, 16);
    // mine_pos[0] = 0;
    // mine_pos[1] = 3;

    xQueueSend(gen_map, &mine_pos, 100);
    vTaskSuspend(NULL);
  }
}

void loop() {
  // put your main code here, to run repeatedly:
}
