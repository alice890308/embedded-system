#include <Stepper.h>
// for 28BYJ-48
#define STEPS 2048
// create an instance of stepper class, specifying #
// of steps of the motor and the pins attached to
Stepper stepper(STEPS, 4, 6, 5, 7);
// the previous reading from the analog input
int previous = 0;
void setup() {
stepper.setSpeed(30); // set the speed to 30 RPMs
}
void loop() {
// get the sensor value from pin A0
int val = analogRead(A0);
// move a number of steps equal to the change in the
// sensor reading
stepper.step(val - previous);
// remember the previous value of the sensor
previous = val;
}