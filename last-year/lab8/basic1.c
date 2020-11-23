#define TIMER1_COMPA_vect _VECTOR(11) /* Timer/Counter1 Compare Match A */
int toggle = false;
int redPin = 6, greenPin = 5, bluePin = 3;
int red=10, green=10, blue=10;
int button = 2;
int flag = 1;
int xAxis = A0,     yAxis = A1;
void setup() {
  Serial.begin(9600);
  cli(); // stop interrupts, atomic access to reg.
  // initialize timer1 
  TCCR1A = 0;    TCCR1B = 0;    TCNT1 = 0;
  TCCR1B |= (1 << WGM12); // turn on CTC
  TCCR1B |= (1<<CS12) | (1<<CS10); // 1024 prescaler
  OCR1A = 7812/2;  // target for counting
  TIMSK1 |= (1<<OCIE1A); // enable timer compare int.
  sei(); // enable all interrupts
  attachInterrupt(0, handle_click, RISING);

  
  pinMode(redPin, OUTPUT);
  pinMode(greenPin, OUTPUT);
  pinMode(bluePin, OUTPUT);
  pinMode(button, INPUT_PULLUP); //return LOW when down
}
void handle_click() { // button debouncing, toggle LED
  static unsigned long last_int_time = 0;
  unsigned long int_time = millis(); // Read the clock

  if (int_time - last_int_time > 200 ) {  
    // Ignore when < 200 msec
    toggle = !toggle;  // switch LED
  }

  last_int_time = int_time;
}

void loop() {
  int xVal = analogRead(xAxis);
  int yVal = analogRead(yAxis);
  int isPress = digitalRead(button);
  Serial.print("X="); Serial.print(xVal);
  Serial.print("Y="); Serial.print(yVal);
  if(isPress == 0) Serial.println("Button is pressed.");
  else Serial.println("Button is not pressed.");
  delay(500);

  
  if(xVal <= 10 && yVal <= 516 && yVal >= 496){
    analogWrite(greenPin, green);
    green-=10;
    if(green<0){
      green = 0;
    }
  }
  else if(xVal >= 1013 && yVal <= 516 && yVal >= 496){
    analogWrite(greenPin, green);
    green+=10;
  }
  else if(xVal>=490 && xVal<=510 && yVal <= 10){
    analogWrite(redPin, red);
    red+=10;
  }
  else if(xVal>=490 && xVal<=510 && yVal >= 1013){
    analogWrite(redPin, red);
    red-=10;
    if(red<0){
      red = 0;
    }
  }
  else if(yVal>506){
    analogWrite(bluePin, blue);
    blue+=10;
  }
  else if(yVal<506){
    analogWrite(bluePin, blue);
    blue-=10;
    if(blue<0){
      blue = 0;
    }
  }
}

ISR(TIMER1_COMPA_vect) { // Timer1 ISR
  if (toggle) {
    
    if(flag){
      analogWrite(redPin, red);
      analogWrite(greenPin, green);
      analogWrite(bluePin, blue);
      flag = 0;
    }
    else{
      analogWrite(redPin, 0);
      analogWrite(greenPin, 0);
      analogWrite(bluePin, 0);
      flag = 1;
    }
  } else {
    analogWrite(redPin, red);
    analogWrite(greenPin, green);
    analogWrite(bluePin, blue);
    
  }
}