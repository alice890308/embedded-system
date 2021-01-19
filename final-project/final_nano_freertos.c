#include <Wire.h>//A4_SDA  A5_SCL
#include <Arduino_FreeRTOS.h>

#define MAX_SHOWING 15
#define LIGHT_ALL 14

#define IDLE 0
#define SHINE 1

/* 溝通用 */
int fromUno = 0;

/* 亮LED燈 */
byte L[3]={10,11,12};
byte H[3]={4, 3, 5};
int row = 0;
int col = 0;
int cur_pos[MAX_SHOWING][2];

/* 點擊 */
const int buttonPin =2;
int flag = true;
int flag_counter = 0;

/* state */
int state = IDLE;

void light(byte x,byte y);
void dark();
void show_note(int col_num);
void reset_all();
void shine_all(void *pvParameters);
void judge_hit(void *pvParameters);
//void my_timer(void *pvParameters);

void setup() {
    Serial.begin(9600);
    Wire.begin(0x9); // nano的位址
    Wire.onReceive(receiveEvent);

    for(int i=0;i<3;i++){
        pinMode(L[i],1);
        pinMode(H[i],1);
    }
    for(int i = 0; i < MAX_SHOWING; i++) {
        for(int j = 0; j < 2; j++)
        cur_pos[i][j] = -1;
    }

    xTaskCreate(judge_hit, "hit", 128, NULL, 2, NULL);
    xTaskCreate(shine_all, "shine", 128, NULL, 2, NULL);
    //xTaskCreate(my_timer, "timer", 128, NULL, 2, NULL);

    /* timer interrupt */
    cli();
    TCCR1A = 0;    TCCR1B = 0;    TCNT1 = 0;
    TCCR1B |= (1 << WGM12); // turn on CTC mode
    TCCR1B |= (1<<CS12) | (1<<CS10); // 1024 prescaler
    //OCR1A = 7812;  // give 500 ms at 16 MHz/1024
    OCR1A = 3905; // 250 ms
    TIMSK1 |= (1<<OCIE1A); // enable timer compare int.
    sei();
    //initial_time = millis();
}

void receiveEvent(int bytes) {
    while(Wire.available()){//wire.read裡面有一個佇列，會依序取出傳來的數字，available這個函數是現在這個佇列裡面有幾筆資料
        fromUno = Wire.read();
        //Serial.print("state = ");
        //Serial.println(state);
        if (fromUno == 15) { // 結束訊號
            state = IDLE;
            Serial.println("receive end");
            reset_all();
        }
        else if (fromUno == 10) { // 開始訊號
            state = SHINE;
            Serial.println("receive start");
        }
        else if (state == SHINE) {
            show_note(fromUno);
            Serial.println(fromUno);
        }
        
    }
}

void loop() {}

void light(byte x,byte y){
  digitalWrite(L[2],x&4);
  digitalWrite(L[1],x&2);
  digitalWrite(L[0],x&1);
  digitalWrite(H[2],y&4);
  digitalWrite(H[1],y&2);
  digitalWrite(H[0],y&1);
}

void dark(){
  digitalWrite(L[2],0);
  digitalWrite(L[1],0);
  digitalWrite(L[0],0);
  digitalWrite(H[2],1);
  digitalWrite(H[1],1);
  digitalWrite(H[0],0);
}

void reset_all()
{
    for(int i = 0; i < MAX_SHOWING; i++) {
        for(int j = 0; j < 2; j++)
        cur_pos[i][j] = -1;
    }
    dark();
}

void shine_all(void *pvParameters)
{
    (void) pvParameters;
    if (state == IDLE) {
        dark();
    }
    else if (state == SHINE) {
        /* 亮LED燈 */
        for(int i = 0; i < MAX_SHOWING-1; i++) {
            if (cur_pos[i][0] != -1 && cur_pos[i][1] != -1) {
                //Serial.println("light");
                light(cur_pos[i][0], cur_pos[i][1]);
            }
        }
    }
}

void judge_hit(void *pvParameters)
{
    (void) pvParameters;
    bool isHit;
    while(1) {
        if (state == SHINE) {
            if (digitalRead(2) && flag == true) {
                flag = false;
                flag_counter = 0;
                isHit = false;
                for(int i = 0; i < MAX_SHOWING; i++) {
                    if (cur_pos[i][0] == 0) {
                        isHit = true;
                        break;
                    }
                }
                if (isHit == true) {
                    Serial.println("hit!!");
                    Wire.beginTransmission(0x07);
                    Wire.write('h'); 
                    Wire.endTransmission();
                }
            }
        }
    }
}

void show_note(int col_num) {
    int get_pos = -1;
    for(int i = 0; i < 6; i++) {
        for(int i = 0; i < MAX_SHOWING-1; i++) {
            if (cur_pos[i][0] == -1) {
                get_pos = i;
                break;
            }
        }
        if (get_pos != -1) {
            cur_pos[get_pos][0] = 7; // set row(最內圈)
            cur_pos[get_pos][1] = i; // set col
        }
        else {
            Serial.println("cur_pos full!");
        }
    }
}

// void my_timer(void *pvParameters)
// {
//     (void) pvParameters;


// }

ISR(TIMER1_COMPA_vect)
{ // Timer1 ISR
  // 把正在顯示的音符往下掉一格
    for(int i = 0; i < MAX_SHOWING; i++) {
        if (cur_pos[i][0] != -1) {
            cur_pos[i][0]--;
            if (cur_pos[i][0] < 0) {
                cur_pos[i][0] = -1;
                cur_pos[i][1] = -1;
                dark();
            }
        }
    }
    flag_counter += 250;
}