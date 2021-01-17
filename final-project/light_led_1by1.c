//掉落時間總共2秒，每250 ms就會往下掉落一格
#include"easyTimer.h"
byte L[3]={10,11,12};
byte H[3]={4, 3, 5};
int row = 0;
int col = 0;
long int initial_time;
int next_drop = 0;
int cur_first = 0; // 當前最早開始掉的音符，位於first, last之間的音符就是每次要更新count的範圍
int cur_last = 0; // 當前最晚開始掉的音符
byte drop_counter[37] = {0};
byte note_pos[37][2]; // 0:row, 1:col
int note[37][2] = {
  10, 0,
  1598, 4,
  4742, 1,
  6347, 3,
  7886, 2,
  9514, 5,
  9871, 1,
  10245, 0
};

void play_test();
void start_next();

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  for(int i=0;i<3;i++){
    pinMode(L[i],1);
    pinMode(H[i],1);
  }
  for(int i = 0; i < 37; i++) {
    for(int j = 0; j < 2; j++) {
      note_pos[i][j] = 0;
    }
  }
  // initialize timer1 
  cli();
  TCCR1A = 0;    TCCR1B = 0;    TCNT1 = 0;
  TCCR1B |= (1 << WGM12); // turn on CTC mode
  TCCR1B |= (1<<CS12) | (1<<CS10); // 1024 prescaler
  //OCR1A = 7812;  // give 500 ms at 16 MHz/1024
  OCR1A = 781; // 50 ms
  TIMSK1 |= (1<<OCIE1A); // enable timer compare int.
  sei();
  initial_time = millis();
}
void light(byte x,byte y){
  digitalWrite(L[2],x&4);
  digitalWrite(L[1],x&2);
  digitalWrite(L[0],x&1);
  digitalWrite(H[2],y&4);
  digitalWrite(H[1],y&2);
  digitalWrite(H[0],y&1);
}
void loop() {
  Serial.print("first: ");
  Serial.print(cur_first);
  Serial.print(" last: ");
  Serial.println(cur_last); // run this code to test how many notes currently play
  for(int i = cur_first; i <= cur_last; i++) {
    light(note_pos[i][0], note_pos[i][1]);
  }
  if ((millis()-initial_time) > note[next_drop][0]) {
    //Serial.print("cur next_drop = ");
    Serial.println(next_drop);
    if (next_drop == 7) { //0~26
      Serial.println("release final! Start next round!");
      start_next();
    }
    else {
      play_test();
      next_drop++;  
    }
    
  }
}

void play_test() {
  cur_last = next_drop;
  note_pos[next_drop][0] = 7; // set row(最內圈)
  note_pos[next_drop][1] = note[next_drop][1]; // set col
}

void start_next() {
  next_drop = 0;
  cur_first = 0;
  cur_last = 0;
  for(int i = 0; i < 37; i++) {
    drop_counter[i] = 0;
  }
  initial_time = millis();
  Serial.print("initial_time = ");
  Serial.println(initial_time);
}

ISR(TIMER1_COMPA_vect)
{ // Timer1 ISR
  for(int i = cur_first; i <= cur_last; i++) {
    if (drop_counter[i] == 5) { // time up，要往下掉一格
      drop_counter[i] = 0;
      if (note_pos[i][0] == 0) { // 如果已經掉到底了
        cur_first++; //就不再顯示這個note
      }
      else {
        note_pos[i][0]--; // 否則繼續顯示，讓note往下掉一格
      }
    }
    else { // 如果時間還沒到，就把count++
      drop_counter[i]++;
    }
  }
}