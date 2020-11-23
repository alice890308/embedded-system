const int trigPin = 11, echoPin = 10;
long duration, distance;
void setup()
{
    pinMode(trigPin, OUTPUT);
    pinMode(echoPin, INPUT);
    Serial.begin(9600);
}
void loop()
{
    digitalWrite(trigPin, LOW); // Clears the trigPin
    delayMicroseconds(2);
    /* Sets the trigPin on HIGH state for 10 ms */
    digitalWrite(trigPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(trigPin, LOW);
    /* Reads Echo pin, returns sound travel time in ms */
    duration = pulseIn(echoPin, HIGH);
    /* Calculates the distance */
    distance = duration * 0.034 / 2;
    Serial.println(distance);
}