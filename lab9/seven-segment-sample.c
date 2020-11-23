int pins[8] = {2, 3, 4, 5, 6, 7, 13, 12}; //pins to 7-seg.
boolean data[3][8] = {
    // define the pins to light the 3 number: 0, 1, 2
    {true, true, true, true, true, true, false},     // 0
    {false, true, true, false, false, false, false}, //1
    {true, true, false, true, true, false, true}};

void setup()
{
    pinMode(2, OUTPUT);
    pinMode(3, OUTPUT);
    pinMode(4, OUTPUT);
    pinMode(5, OUTPUT);
    pinMode(6, OUTPUT);
    pinMode(7, OUTPUT);
    pinMode(12, OUTPUT);
    pinMode(13, OUTPUT);
    digitalWrite(9, 0); // 關閉小數點
}

void loop()
{
    int number = 1;
    for (int i = 0; i < 8; i++)
    {
        digitalWrite(pins[i], data[number][i] == true ? HIGH : LOW);
    }
}