#include <math.h>
int led = 13;

void setup()
{
  pinMode(led, OUTPUT);
}

void loop()
{
  
  float val = (exp(sin(millis()/float(2000)*PI)) - 0.36787944)*108.0;
  digitalWrite(led, HIGH);
  delayMicroseconds(val);           
  digitalWrite(led, LOW); 
}
