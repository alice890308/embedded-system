//#include"easyTimer.h"
#define MAX_SHOWING 15
#define LIGHT_ALL 14
#define MAX_NOTE 4

byte L[3]={10,11,12};
byte H[3]={4, 3, 5};
int row = 0;
int col = 0;
long int initial_time;
int cur_pos[MAX_SHOWING][2];
bool wait_finish = false;
int showing_num = 0; // 紀錄現在有幾個音符是亮的
int note_pos = 0; // 紀錄現在顯示到第幾個note
int note[37][2] = { // 
  10, 0,
  3000, 1,
  6000, 2,
  10000, 3,
  14000, 4
};

void light(byte x,byte y);
void dark();
void show_note();
void start_next();

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  for(int i=0;i<3;i++){
    pinMode(L[i],1);
    pinMode(H[i],1);
  }
  for(int i = 0; i < MAX_SHOWING; i++) {
    for(int j = 0; j < 2; j++) {
      cur_pos[i][j] = -1;
    }
  }

  // initialize timer1 
  cli();
  TCCR1A = 0;    TCCR1B = 0;    TCNT1 = 0;
  TCCR1B |= (1 << WGM12); // turn on CTC mode
  TCCR1B |= (1<<CS12) | (1<<CS10); // 1024 prescaler
  //OCR1A = 7812;  // give 500 ms at 16 MHz/1024
  OCR1A = 3905; // 250 ms
  TIMSK1 |= (1<<OCIE1A); // enable timer compare int.
  sei();
  initial_time = millis();
}

void loop() {
//  counter++;
//  if (counter == 100) {
//    counter = 0;
//    Serial.println("------------");
//    for(int i = 0; i < MAX_SHOWING; i++) {
//      Serial.print(cur_pos[i][0]);
//      Serial.print(", ");
//      Serial.println(cur_pos[i][1]);
//    }
//  }
  delay(100); // 不知道為什麼就是要加的delay，否則會亮多餘的燈
  // 亮燈
  for(int i = 0; i < MAX_SHOWING-1; i++) {
    if (cur_pos[i][0] != -1 && cur_pos[i][1] != -1) {
      light(cur_pos[i][0], cur_pos[i][1]);
    }
  }

  // 增加新的lightint note
  if (!wait_finish) {
    if ((millis()-initial_time) > note[note_pos][0]) {
      show_note();
      note_pos++;
      if (note_pos > MAX_NOTE) {
        wait_finish = true;
        Serial.println("wait_finish = true");
      }
    }
  }
  else {
    if (showing_num == 0) {
      start_next();
      wait_finish = false;
    }
  }
  
}

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

void show_note() {
  Serial.println(millis()-initial_time);
  Serial.println(note_pos);
  int get_pos = -1;
  for(int i = 0; i < MAX_SHOWING-1; i++) {
    if (cur_pos[i][0] == -1) {
      get_pos = i;
      break;
    }
  }
  if (get_pos != -1) {
    cur_pos[get_pos][0] = 7; // set row(最內圈)
    cur_pos[get_pos][1] = note[note_pos][1]; // set col
    showing_num++;
  }
  else {
    Serial.println("cur_pos full!");
  }
}

void start_next() {
  initial_time = millis();
  Serial.print("initial_time = ");
  Serial.println(initial_time);
  for(int i = 0; i < MAX_SHOWING; i++) {
    cur_pos[i][0] = -1;
    cur_pos[i][1] = -1;
  }
  note_pos = 0;
  Serial.println("start next round!");
}

ISR(TIMER1_COMPA_vect)
{ // Timer1 ISR
  // 把正在顯示的音符往下掉一格
  for(int i = 0; i < MAX_SHOWING; i++) {
    if (cur_pos[i][0] != -1) {
      cur_pos[i][0]--;
      if (cur_pos[i][0] < 0) {
        cur_pos[i][0] = -1;
        cur_pos[i][1] = -1;
        showing_num--;
        dark();
      }
    }
  }
}