// these constants won't change:
const int analogPin = A0;   // the pin that the potentiometer is attached to
const int ledCount = 6;    // the number of LEDs in the bar graph
int ledPins[] = {
  2, 3, 4, 5, 6, 7, 8, 9, 10, 11
};   // an array of pin numbers to which LEDs are attached



void setup() {
  // put your setup code here, to run once:
 Serial.begin (9600);
  // loop over the pin array and set them all to output:
  for (int thisLed = 0; thisLed < ledCount; thisLed++) {
    pinMode(ledPins[thisLed], OUTPUT);
  }
}

void loop() {
  // put your main code here, to run repeatedly:
  int myReading = analogRead(analogPin);
  int level = map(myReading,0,1023,0,ledCount+1);
  String stringOne = "Sensor value: ";
  String stringThree = stringOne + level;
  Serial.println(stringThree);   
  long currentTime = millis();
  stringOne = "millis() value: ";
  stringThree = stringOne + millis();
  Serial.println(stringThree); 
  //Serial.println(String ("level" + level));
  for (int ledLoop=0; ledLoop<ledCount;ledLoop++){
    if (level >ledLoop) {
        digitalWrite(ledPins[ledLoop],HIGH);
      }
    else {
         digitalWrite(ledPins[ledLoop],LOW);
    }  
  }
}
