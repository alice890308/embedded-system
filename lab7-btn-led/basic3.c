/*
  Debounce

  Each time the input pin goes from LOW to HIGH (e.g. because of a push-button
  press), the output pin is toggled from LOW to HIGH or HIGH to LOW. There's a
  minimum delay between toggles to debounce the circuit (i.e. to ignore noise).

  The circuit:
  - LED attached from pin 13 to ground
  - pushbutton attached from pin 2 to +5V
  - 10 kilohm resistor attached from pin 2 to ground

  - Note: On most Arduino boards, there is already an LED on the board connected
    to pin 13, so you don't need any extra components for this example.

  created 21 Nov 2006
  by David A. Mellis
  modified 30 Aug 2011
  by Limor Fried
  modified 28 Dec 2012
  by Mike Walters
  modified 30 Aug 2016
  by Arturo Guadalupi

  This example code is in the public domain.

  http://www.arduino.cc/en/Tutorial/Debounce
*/

// constants won't change. They're used here to set pin numbers:
const int buttonPin[3] = {9, 8, 7};    // the number of the pushbutton pin
const int led[3] = {6, 5, 4};      // the number of the LED pin

// Variables will change:
int ledState = HIGH;         // the current state of the output pin
int buttonState[3];             // the current reading[i] from the input pin

// the following variables are unsigned longs because the time, measured in
// milliseconds, will quickly become a bigger number than can be stored in an int.
unsigned long lastDebounceTime[3] = {0,0,0};  // the last time the output pin was toggled
byte debounceDelay = 50;    // the debounce time; increase if the output flickers
//uint64_t time0[3],clk,countdown[3];
unsigned int time0[3] = {0, 0, 0},clk,countdown[3];
int lastButtonState[3] ={0,0,0};   // the previous reading[i] from the input pin
float temp;
int state[3] = {0, 0, 0};

bool gate[3]={1,1,1};
int time_[3]={0,0,0};
bool reading[3];
void setup() {
  for(int i=0;i<3;i++){
    pinMode(led[i],1);       //define OUTOUT=1;
    pinMode(buttonPin[i],0); //define INPUT =0;
  }
  noInterrupts(); // atomic access to timer reg.
  TCCR1A = 0;    TCCR1B = 0;    TCNT1 = 0;
  TCCR1B |= (1 << WGM12); // turn on CTC mode
  TCCR1B |= (1<<CS12) | (1<<CS10); // 1024 prescaler
  OCR1A = 1562;  // give 0.1 sec at 16 MHz/1024
  interrupts(); // enable all interrupts
  Serial.begin(9600);
  // set initial LED state
}

void loop() {
  // read the state of the switch into a local variable:
  for(int i=0;i<3;i++){
  reading[i] = digitalRead(buttonPin[i]);

// If the switch changed, due to noise or pressing:
  if (reading[i] != lastButtonState[i]) {
    // reset the debouncing timer
    lastDebounceTime[i] = millis();
  }

  if ((millis() - lastDebounceTime[i]) > debounceDelay) {
    // if the button state has changed:
    if (reading[i] != buttonState[i]) {
      buttonState[i] = reading[i];
    }
  }
  if (buttonState[i]) { // light up led when pressing button
    digitalWrite(led[i],1);
    if (gate[i]) {
        time0[i] = 0;
        gate[i] = 0;
        state[i] = 0;
    }
  }
  if(!buttonState[i] && gate[i]==0){  // detecet release
    if (i == 0)
      Serial.print("Blue : ");
    else if (i == 1)
      Serial.print("Red : ");
    else if (i == 2)
      Serial.print("Green : ");
    temp = time0[i] * 0.1;
    Serial.print(temp);
    Serial.println(" sec");
    gate[i]=1;
    state[i] = 1; // flash led
  }
  if(time0[i] > 0 && state[i] == 1){
    //Serial.println(millis()%1000);
    digitalWrite(led[i],(time0[i]%10>4));
  }
    
  // save the reading[i]. Next time through the loop, it'll be the lastButtonState[i]:
  lastButtonState[i] = reading[i];
  }
  if (TIFR1 & (1 << OCF1A)){
    for(int i = 0; i < 3; i++) {
        if (state[i] == 0) { // measure button pressed time
            time0[i]++; // pressing
        }
        else { // led flash
            if (time0[i] > 0)
                time0[i]--;
        }
    }
    TIFR1 = (1<<OCF1A); 
  }
}