int button = 2;
int xAxis = A0, yAxis = A1;
void setup()
{
    Serial.begin(9600);
    pinMode(button, INPUT_PULLUP); //return LOW when down
}
void loop()
{
    int xVal = analogRead(xAxis);
    int yVal = analogRead(yAxis);
    int isPress = digitalRead(button);
    Serial.print("X=");
    Serial.println(xVal);
    Serial.print("Y=");
    Serial.println(yVal);
    if (isPress == 0)
        Serial.println("Button is pressed.");
    else
        Serial.println("Button is not pressed.");
    delay(200);
}
// 問題：燒板子後發現如果按住button的話只會暫停，不會印出button is pressed
/*
joystick left: x = 0, y ~= 500
joystick right: x = 1000, y ~= 500
joystick up: x~= 500, y = 0
joystick down: x~= 500, y = 1000

右上：x = 500~1000, y = 0~50
*/