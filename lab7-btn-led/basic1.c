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
unsigned int time0[3],clk,countdown[3];
int lastButtonState[3] ={0,0,0};   // the previous reading[i] from the input pin
float temp;

bool gate[3]={1,1,1};
int time_[3]={0,0,0};
bool reading[3];
void setup() {
  for(int i=0;i<3;i++){
    pinMode(led[i],1);       //define OUTOUT=1;
    pinMode(buttonPin[i],0); //define INPUT =0;
  }
  Serial.begin(9600);
  // set initial LED state
}

void loop() {
  // read the state of the switch into a local variable:
  for(int i=0;i<3;i++){
  reading[i] = digitalRead(buttonPin[i]);

  // check to see if you just pressed the button
  // (i.e. the input went from LOW to HIGH), and you've waited long enough
  // since the last press to ignore any noise:

  // If the switch changed, due to noise or pressing:
  if (reading[i] != lastButtonState[i]) {
    // reset the debouncing timer
    lastDebounceTime[i] = millis();
  }

  if ((millis() - lastDebounceTime[i]) > debounceDelay) {
    // whatever the reading[i] is at, it's been there for longer than the debounce
    // delay, so take it as the actual current state:

    // if the button state has changed:
    if (reading[i] != buttonState[i]) {
      buttonState[i] = reading[i];

      // only toggle the LED if the new button state is HIGH
    }
  }
  if (buttonState[i]) {
    digitalWrite(led[i],1);
  }
  if(buttonState[i] && gate[i]){
    time0[i]=millis(); // newest pressing time
    gate[i]=0;
  }
  if(!buttonState[i] && gate[i]==0){ 
    time_[i]=millis()-time0[i]; // calculate duration
    countdown[i]=millis(); // release time
    if (i == 0)
      Serial.print("Blue : ");
    else if (i == 1)
      Serial.print("Red : ");
    else if (i == 2)
      Serial.print("Green : ");
    temp = time_[i] / 1000.0;
    Serial.print(temp);
    Serial.println(" sec");
    gate[i]=1;
  }
  if(millis()-countdown[i]<=time_[i]){
    //Serial.println(millis()%1000);
    digitalWrite(led[i],(millis()%1000>500));
  }
    
  // save the reading[i]. Next time through the loop, it'll be the lastButtonState[i]:
  lastButtonState[i] = reading[i];
  }
}