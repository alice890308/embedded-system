int btn = 3;
unsigned int initial_time = 0;
int flag = 0;

void setup() {
  pinMode(3, INPUT_PULLUP);
  Serial.begin(9600);
}

void loop() {
  // start play music
  initial_time = millis();
  int temp = digitalRead(btn);
  if (!temp && (flag == 0)) {
    flag = 1;
    Serial.println("push");
    //Serial.println(millis()-initial_time);
  }
  else if (temp == 1) {
    flag = 0;
  }
}