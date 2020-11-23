int redPin = 11, greenPin = 10, bluePin = 9;
void setup()
{
    pinMode(redPin, OUTPUT);
    pinMode(greenPin, OUTPUT);
    pinMode(bluePin, OUTPUT);
}
void loop()
{
    analogWrite(redPin, 50);
    analogWrite(greenPin, 100);
    analogWrite(bluePin, 150);
    delay(1000);
    analogWrite(redPin, 0);
    analogWrite(greenPin, 0);
    analogWrite(bluePin, 0);
    delay(1000);
}