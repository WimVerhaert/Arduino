int controlPin = 10;
void setup()
{
  Serial.begin(9600);
  TCCR1A = 0x23;
  TCCR1B = 0x02;  // select clock
  ICR1 = 639;  // aiming for 25kHz
  pinMode(controlPin, OUTPUT);  // enable the PWM output (you now have a PWM signal on digital pin 3)
  OCR1B = 320;  // set the PWM duty cycle
  test();
}

void loop()
{

}

void test(){
  OCR1B = 639;
}

