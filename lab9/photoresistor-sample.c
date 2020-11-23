void setup()
{
    Serial.begin(9600);
    pinMode(A0, INPUT);
    pinMode(13, OUTPUT);
    digitalWrite(13, LOW);
}
int pr_min = 400;
void loop()
{
    int pr = analogRead(A0);
    /* Read value of A0 input (must in range of 0~1023) */
    Serial.println(pr);
    digitalWrite(13, pr > pr_min ? LOW : HIGH);
    /* If input value > pr_min, then turn on LED */
    delay(1000);
}