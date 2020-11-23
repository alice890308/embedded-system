#include<Stepper.h>

// for 28BYJ-48
#define STEPS 2048

// create an instance of stepper class, specifying #
// of steps of the motor and the pins attached to
Stepper stepper(STEPS, 8, 10, 9, 11);

int button = 2, redPin = 11, bluePin = 9,greenPin = 10;
int xAxis = A0, yAxis = A1;
int redIntensity = 0, blueIntensity = 0,greenIntensity = 0;
int state = 0;
int mask = 0;
int previous = 0;
unsigned long lastX, lastY;

void setup() { 
  Serial.begin(9600);
  pinMode(button, INPUT_PULLUP); //return LOW when down
  pinMode(redPin, OUTPUT);
  pinMode(bluePin, OUTPUT);
  pinMode(greenPin, OUTPUT);
  attachInterrupt(digitalPinToInterrupt(button), button_click, FALLING);
  stepper.setSpeed(15); // set the speed to 15 RPMs
}


void loop() {
  int xVal = analogRead(xAxis);
  int yVal = analogRead(yAxis);
  
  if (mask == 1) {
    analogWrite(redPin, 0);
    analogWrite(bluePin, 0);
    analogWrite(greenPin, 0);
  }
  else {
    if(xVal < 200  && yVal < 200)
    {
        stepper.step(-256 - previous);
        previous = -256;
      if(millis() - lastY > 500 && millis() - lastX > 500)
      {
        if(blueIntensity < 5)
        {
          blueIntensity++;
          lastX = millis();
          lastY = millis();
        }
      }
    }
    else if(xVal > 800 && yVal < 200)
    {
        stepper.step(256 - previous);
        previous = 256;
      if(millis() - lastY > 500 && millis() - lastX > 500)
      {
        if(blueIntensity < 5)
        {
          blueIntensity++;
          lastX = millis();
          lastY = millis();
        }
      }
    }
    else if(xVal < 200  && yVal > 800)
    {
        stepper.step(-256 * 3 - previous);
        previous = -256 * 3;

      if(millis() - lastY > 500 && millis() - lastX > 500)
      {
        if(blueIntensity > 0)
        {
          blueIntensity--;
          lastX = millis();
          lastY = millis();
        }
      }
    }
    else if(xVal > 800 && yVal > 800){
        stepper.step(256 * 3 - previous);
        previous = 256 * 3;
        if(millis() - lastY > 500 && millis() - lastX > 500)
      {
        if(blueIntensity > 0)
        {
          blueIntensity--;
          lastX = millis();
          lastY = millis();
        }
      }
    }
    else if(xVal > 800)// && yVal > 200 && yVal < 800)
    {
        stepper.step(512 - previous);
        previous = 512;
      if(millis() - lastX > 500)
      {
        if(greenIntensity < 5)
        {
          greenIntensity++;
          lastX = millis();
        }
      }
    }
    else if(xVal < 200)//&& yVal > 200 && yVal < 800)
    {
        stepper.step(-512 - previous);
        previous = -512;
      if(millis() - lastX > 500)
      {
        if(greenIntensity > 0)
        {
          greenIntensity--;
          lastX = millis();
        }
      }
    }
    else if(yVal < 200)//&& xVal > 200 && xVal < 800)
    {
       stepper.step(0 - previous);
       previous = 0;
      if(millis() - lastY > 500)
      {
        if(redIntensity < 5)
        {
          redIntensity++;
          lastY = millis();
        }
      }
    }
    else if(yVal > 800)//&& xVal > 200 && xVal < 800)
    {
        stepper.step(1024 - previous);
        previous = 1024;
        
      if(millis() - lastY > 500)
      {
        if(redIntensity > 0)
        {
          redIntensity--;
          lastY = millis();
        }
      }
    }
    else{
        stepper.step(0 - previous);
        previous = 0;
    }
    
    analogWrite(redPin, 51 * redIntensity);
    analogWrite(bluePin, 51 * blueIntensity);
    analogWrite(greenPin, 51 * greenIntensity);
  }
 
}

void button_click() {
  state ^= 1;
  if (state == 0) mask = 0;
  cli(); // stop interrupts, atomic access to reg.
  // initialize timer1 
  TCCR1A = 0;    TCCR1B = 0;    TCNT1 = 0;
  TCCR1B |= (1 << WGM12); // turn on CTC
  TCCR1B |= (1<<CS12) | (1<<CS10); // 1024 prescaler
  OCR1A = 7812;//3905;  // target for counting
  TIMSK1 |= (1<<OCIE1A); // enable timer compare int.
  sei(); // enable all interrupts
}

ISR(TIMER1_COMPA_vect) { // Timer1 ISR
  if (state == 1)
  {
    mask ^= 1;
  }
}