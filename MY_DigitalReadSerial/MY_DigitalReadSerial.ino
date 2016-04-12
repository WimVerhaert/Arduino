/*
  DigitalReadSerial
 Reads a digital input on pin 2, prints the result to the serial monitor

 This example code is in the public domain.
 */

// digital pin 2 has a pushbutton attached to it. Give it a name:
int pushButton = 2;
int led = 13;
int previousState = 0;
int trigger = 0;

// the setup routine runs once when you press reset:
void setup() {
  // initialize serial communication at 9600 bits per second:
  Serial.begin(9600);
  // make the pushbutton's pin an input:
  pinMode(pushButton, INPUT);
  pinMode(led, OUTPUT);
}

// the loop routine runs over and over again forever:
void loop() {
  // read the input pin:
  
  int buttonState = digitalRead(pushButton);
  Serial.println("pushbutton=" );
  Serial.println(buttonState);
  Serial.println("previousState=");
  Serial.println(previousState);
  if  (buttonState) {
    if (!trigger){
    switch (previousState) {
        case 0: 
          previousState = 1 ;
          break;
        case 1:
          previousState = 0 ;
          break;
      }
    }
    trigger = 1;
   // Serial.println("ledstate=");
    //Serial.println(previousState);    
  }
  if  (!buttonState) {
    trigger = 0;
  }
  digitalWrite(led, previousState);
  delay(200);        // delay in between reads for stability
  
}



