
/*
  Android Breath v0.2
  Simulates led breathing found on Android devices.
  Tested with Arduino Nano ATmega328.
  
  Cesar Schneider <cesschneider@gmail.com>
  https://gist.github.com/cesschneider/7689698
 */
 
// Pin 13 has an LED connected on most Arduino boards.
// give it a name:
int led = 9;

// the setup routine runs once when you press reset:
void setup() {                
  pinMode(led, OUTPUT);     
}

int i;
int on;
int off;
int pulses;
int cycles = 5;

void loop() {
  
  pulses = 160;
  for (on = 1; on <= cycles; on++) {
    for (i = 0; i < pulses; i++) {
      digitalWrite(led, HIGH);   
      delay(on);               
      digitalWrite(led, LOW);    
      delay(cycles - on);               
    }
    //  on  off  pu  cy
    // (1 + 4) * 8 * 5 = 200ms
  }
  
  pulses = 160;
  for (off = cycles; off > 1; off--) {
    for (i = 0; i < pulses; i++) {
      digitalWrite(led, HIGH);   
      delay(off);               
      digitalWrite(led, LOW);    
      delay(cycles - off);               
    }
    //  on  off  pu  cy
    // (5 + 1) * 8 * 5 = 200ms
  }

  digitalWrite(led, LOW);    
  delay(100);               
}

