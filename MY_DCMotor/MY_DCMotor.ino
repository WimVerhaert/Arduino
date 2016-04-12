// Define which pin to be used to communicate with Base pin of TIP120 transistor
int TIP120pin = 11; //for this project, I pick Arduino's PMW pin 11
int speedMeter = A0;
void setup()
{
pinMode(TIP120pin, OUTPUT); // Set pin for output to control TIP120 Base pin
pinMode(speedMeter, INPUT);
Serial.begin(9600);

}

void loop()
{
  int sensorReading = analogRead(speedMeter);
  // map it to a range from 0 to 100:
  int motorSpeed = map(sensorReading, 0, 1023, 0, 255);
  analogWrite(TIP120pin, motorSpeed); // By changing values from 0 to 255 you can control motor speed
  String staticText = "Speed is set to:";
  String speedText = staticText + motorSpeed;
  Serial.println(speedText);
}
