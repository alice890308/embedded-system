// constants won't change. They're used here to set pin numbers:
const int buttonPin[2] = {3, 2};    // the number of the pushbutton pin
const int led[2] = {6, 5};      // the number of the LED pin

// Variables will change:
int ledState = HIGH;         // the current state of the output pin
int buttonState[2];             // the current reading[i] from the input pin

// the following variables are unsigned longs because the time, measured in
// milliseconds, will quickly become a bigger number than can be stored in an int.
unsigned long lastDebounceTime[2] = {0,0};  // the last time the output pin was toggled
byte debounceDelay = 50;    // the debounce time; increase if the output flickers
unsigned int time0[2] = {0, 0};
int lastButtonState[2] ={0,0};   // the previous reading[i] from the input pin
float temp;
int state[2] = {0, 0};

bool gate[2]={1,1};
bool reading[2];
int led_on[2] = {0, 0};
void setup() {
  for(int i=0;i<2;i++){
    pinMode(led[i],1);       //define OUTOUT=1;
    pinMode(buttonPin[i],0);
  }
  attachInterrupt(1, handle_btn1_click, CHANGE);
  attachInterrupt(0, handle_btn2_click, CHANGE);
  cli();
  TCCR1A = 0;    TCCR1B = 0;    TCNT1 = 0;
  TCCR1B |= (1 << WGM12); // turn on CTC mode
  TCCR1B |= (1<<CS12) | (1<<CS10); // 1024 prescaler
  OCR1A = 1562;  // give 0.1 sec at 16 MHz/1024
  TIMSK1 |= (1<<OCIE1A); // enable timer compare int.
  sei();
  Serial.begin(9600);
  // set initial LED state
}

void loop() {
  // read the state of the switch into a local variable:
    for (int i = 0; i < 2; i++) {
        if(time0[i] > 0 && state[i] == 1){
            //Serial.println(millis()%1000);
            digitalWrite(led[i],(time0[i]%10>4));
        }
        if (state[i] == 0 && led_on[i] == 1) {
            digitalWrite(led[i], 1);
        }
    }
}

void handle_btn1_click()
{ // button debouncing, toggle LED
    static unsigned long last_int_time = 0;
    unsigned long int_time = millis(); // Read the clock
    if (int_time - last_int_time > 50)
    {
        buttonState[0] = digitalRead(buttonPin[0]);
        //Ignore when < 200 msec
        if (buttonState[0]) { // 按住
            led_on[0] = 1;
            time0[0] = 0;
            gate[0] = 0;
            state[0] = 0;
        }
        else {
            state[0] = 1; // flash led
            gate[0] = 1;
            led_on[0] = 0;
            temp = time0[0] * 0.1;
            Serial.print("Green : ");
            Serial.print(temp);
            Serial.println(" sec");
        }
        last_int_time = int_time;
    }
    
    //lastButtonState[0] = reading[0];
}

void handle_btn2_click()
{ // button debouncing, toggle LED
    static unsigned long last_int_time = 0;
    unsigned long int_time = millis(); // Read the clock
    
    if (int_time - last_int_time > 50)
    {
        buttonState[1] = digitalRead(buttonPin[1]);
        // Ignore when < 200 msec
        if (buttonState[1]) { // 開始按住
            led_on[1] = 1;
            time0[1] = 0;
            gate[1] = 0;
            state[1] = 0; // pressing button
        }
        else { // 放開的時候
            state[1] = 1; // flash led
            gate[1] = 1;
            temp = time0[1] * 0.1;
            led_on[1] = 0;
            Serial.print("Blue : ");
            Serial.print(temp);
            Serial.println(" sec");
        }
        Serial.println("in interrupt");
        last_int_time = int_time;
    }
}

ISR(TIMER1_COMPA_vect)
{ // Timer1 ISR
    for(int i = 0; i < 2; i++) {
        if (state[i] == 0) { // measure button pressed time
            time0[i]++; // pressing
        }
        else { // led flash
            if (time0[i] > 0)
                time0[i]--;
        }
    }
}