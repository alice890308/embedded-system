/*
  DigitalReadSerial

  Reads a digital input on pin 2, prints the result to the Serial Monitor

  This example code is in the public domain.

  http://www.arduino.cc/en/Tutorial/DigitalReadSerial
*/

unsigned long lastDebounceTime = 0;  // the last time the output pin was toggled
unsigned long debounceDelay = 50;    // the debounce time; increase if the output flickers

// digital pin 2 has a btn7 attached to it. Give it a name:
int btn7 = 7;
float count7= 0;
int flag7 = 0;
int led4State = HIGH;         // the current state of the output pin
int btn7State;             // the current reading from the input pin
int lastBtn7State = LOW;   // the previous reading from the input pin


int btn6 = 6;
float count6= 0;
int flag6 = 0;
int led3State = HIGH;         // the current state of the output pin
int btn6State;             // the current reading from the input pin
int lastBtn6State = LOW;   // the previous reading from the input pin

int btn5 = 5;
float count5= 0;
int flag5 = 0;
int led2State = HIGH;         // the current state of the output pin
int btn5State;             // the current reading from the input pin
int lastBtn5State = LOW;   // the previous reading from the input pin

// the setup routine runs once when you press reset:
void setup() {
  // initialize serial communication at 9600 bits per second:
  Serial.begin(9600);
  // make the btn7's pin an input:
  pinMode(btn7, INPUT);
  pinMode(btn6, INPUT);
  pinMode(btn5, INPUT);
}

// the loop routine runs over and over again forever:
void loop() {
  // read the input pin:
  int btn7_state = digitalRead(btn7);
  int btn6_state = digitalRead(btn6);
  int btn5_state = digitalRead(btn5);

  if (btn7_state != lastBtn7State) {
    // reset the debouncing timer
    lastDebounceTime = millis();
    
  }
  if ((millis() - lastDebounceTime) > debounceDelay) {
    if (btn7_state != lastBtn7State) {
      lastBtn7State = btn7_state;
      Serial.println("button7!");
      if (lastBtn7State == HIGH) {
        flag7 = 1;
      }
    }
  }

  if (btn6_state != lastBtn6State) {
    // reset the debouncing timer
    lastDebounceTime = millis();
    
  }
  if ((millis() - lastDebounceTime) > debounceDelay) {
    if (btn6_state != lastBtn6State) {
      lastBtn6State = btn6_state;
      Serial.println("button6!");
      if (lastBtn6State == HIGH) {
        flag6 = 1;
      }
    }
  }

  if (btn5_state != lastBtn5State) {
    // reset the debouncing timer
    lastDebounceTime = millis();
    
  }
  if ((millis() - lastDebounceTime) > debounceDelay) {
    if (btn5_state != lastBtn5State) {
      lastBtn5State = btn5_state;
      Serial.println("button5!");
      if (lastBtn5State == HIGH) {
        flag5 = 1;
      }
    }
  }

  if (btn7_state == 1 && flag7 == 1) {
    count7++;
  }
  else {
    if (flag7 == 1) {
      flag7 = 0;
      Serial.println("button 7: ");
      Serial.println(count7 / 100.0);
      Serial.println(" sec");
      count7 = 0;
    }
  }

  if (btn6_state == 1 && flag6 == 1) {
    count6++;
  }
  else {
    if (flag6 == 1) {
      flag6 = 0;
      Serial.println("button 6: ");
      Serial.println(count6 / 100.0);
      Serial.println(" sec");
      count6 = 0;
    }
  }

  if (btn5_state == 1 && flag5 == 1) {
    count5++;
  }
  else {
    if (flag5 == 1) {
      flag5 = 0;
      Serial.println("button 5: ");
      Serial.println(count5 / 100.0);
      Serial.println(" sec");
      count5 = 0;
    }
  }
  delay(10);        // delay in between reads for stability
}