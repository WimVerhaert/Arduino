/* CommaDelimitedOutput sketch */
#define ANALOG_IN_Ch1 0
#define ANALOG_IN_Ch2 1

void setup()
{
  Serial.begin(9600);
}

void loop()
{
  int val1 = analogRead(ANALOG_IN_Ch1); /* Reads values from analog Pin0 = Channel1 */
  int val2 = analogRead (ANALOG_IN_Ch2); /* Reads values from analog Pin1 = Channel2 */
 
  Serial.print('H'); /* Unique header to identify start of message */
  Serial.print(",");
  Serial.print(val1,DEC);
  Serial.print(",");
  Serial.print(val2,DEC);
  Serial.print(",");  /* Note that a comma is sent after the last field */
  Serial.println();  /* send a cr/lf */
  delay(50); //This may be able to be faster than 50ms
}

