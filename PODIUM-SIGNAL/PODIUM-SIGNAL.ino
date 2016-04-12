
// Detect the falling edge

// Include the Bounce2 library found here :
// https://github.com/thomasfredericks/Bounce-Arduino-Wiring
#include <Bounce2.h>


#define BUTTON_PODIUM_GREEN_PIN 7
#define BUTTON_PODIUM_ORANGE_PIN 8
#define BUTTON_PODIUM_RED_PIN 9

#define BUTTON_TECHNIC_GREEN_PIN 12
#define BUTTON_TECHNIC_ORANGE_PIN 11
#define BUTTON_TECHNIC_RED_PIN 10
#define BUTTON_TECHNIC_SHOWPODIUM_PIN 13 // button show my status locally, will light up my 3 leds with what I told the PODIUM

#define LED_PODIUM_1_RED_PIN 4
#define LED_PODIUM_1_ORANGE_PIN 5
#define LED_PODIUM_1_GREEN_PIN 6

#define LED_PODIUM_2_RED_PIN A2
#define LED_PODIUM_2_ORANGE_PIN A1
#define LED_PODIUM_2_GREEN_PIN A0

#define LED_TECHNIC_RED_PIN A3
#define LED_TECHNIC_ORANGE_PIN A4
#define LED_TECHNIC_GREEN_PIN A5

int PODIUM_STATE_GREEN = HIGH;
int PODIUM_STATE_ORANGE = LOW;
int PODIUM_STATE_RED = LOW;
int TECHNIC_STATE_GREEN = HIGH;
int TECHNIC_STATE_ORANGE = LOW;
int TECHNIC_STATE_RED = LOW;
int blinkState_TECHNIC = LOW;
int blinkState_PODIUM = LOW;
bool showPodiumState = false;

unsigned long blinkTimeStamp = 0;
unsigned long showStartTimeStamp = 0;
long showTime = 15000; // show for x seconds when requesting state of PODIUM

// Instantiate a Bounce objects :
Bounce db_BUTTON_PODIUM_GREEN_PIN = Bounce(); 
Bounce db_BUTTON_PODIUM_ORANGE_PIN = Bounce(); 
Bounce db_BUTTON_PODIUM_RED_PIN = Bounce(); 

Bounce db_BUTTON_TECHNIC_GREEN_PIN = Bounce(); 
Bounce db_BUTTON_TECHNIC_ORANGE_PIN = Bounce(); 
Bounce db_BUTTON_TECHNIC_RED_PIN = Bounce(); 
Bounce db_BUTTON_TECHNIC_SHOWPODIUM_PIN = Bounce(); 


void setup() {
  
  // Setup the button with an internal pull-up :
    pinMode(BUTTON_PODIUM_GREEN_PIN,INPUT_PULLUP);
  pinMode(BUTTON_PODIUM_ORANGE_PIN,INPUT_PULLUP);
  pinMode(BUTTON_PODIUM_RED_PIN,INPUT_PULLUP);
  
  pinMode(BUTTON_TECHNIC_GREEN_PIN,INPUT_PULLUP);
  pinMode(BUTTON_TECHNIC_ORANGE_PIN,INPUT_PULLUP);
  pinMode(BUTTON_TECHNIC_RED_PIN,INPUT_PULLUP);
  pinMode(BUTTON_TECHNIC_SHOWPODIUM_PIN,INPUT_PULLUP);

  // After setting up the button, setup the Bounce instance :
  db_BUTTON_PODIUM_GREEN_PIN.attach(BUTTON_PODIUM_GREEN_PIN);
  db_BUTTON_PODIUM_ORANGE_PIN.attach(BUTTON_PODIUM_ORANGE_PIN); 
  db_BUTTON_PODIUM_RED_PIN.attach(BUTTON_PODIUM_RED_PIN); 
  
  db_BUTTON_TECHNIC_GREEN_PIN.attach(BUTTON_TECHNIC_GREEN_PIN); 
  db_BUTTON_TECHNIC_ORANGE_PIN.attach(BUTTON_TECHNIC_ORANGE_PIN ); 
  db_BUTTON_TECHNIC_RED_PIN.attach(BUTTON_TECHNIC_RED_PIN); 
  db_BUTTON_TECHNIC_SHOWPODIUM_PIN.attach(BUTTON_TECHNIC_SHOWPODIUM_PIN);

    db_BUTTON_PODIUM_GREEN_PIN.interval(500);
  db_BUTTON_PODIUM_ORANGE_PIN.interval(500); 
  db_BUTTON_PODIUM_RED_PIN.interval(500);
  
  db_BUTTON_TECHNIC_GREEN_PIN.interval(500); 
  db_BUTTON_TECHNIC_ORANGE_PIN.interval(500);
  db_BUTTON_TECHNIC_RED_PIN.interval(500); 
  db_BUTTON_TECHNIC_SHOWPODIUM_PIN.interval(500);

  // Setup the LED :
    pinMode(LED_PODIUM_1_RED_PIN,OUTPUT);
  pinMode(LED_PODIUM_1_ORANGE_PIN,OUTPUT);
  pinMode(LED_PODIUM_1_GREEN_PIN,OUTPUT);
  
  pinMode(LED_PODIUM_2_RED_PIN,OUTPUT);
  pinMode(LED_PODIUM_2_ORANGE_PIN,OUTPUT);
  pinMode(LED_PODIUM_2_GREEN_PIN,OUTPUT);
  
  pinMode(LED_TECHNIC_RED_PIN,OUTPUT);
  pinMode(LED_TECHNIC_ORANGE_PIN,OUTPUT);
  pinMode(LED_TECHNIC_GREEN_PIN,OUTPUT);

//set the initial default start state
    digitalWrite(LED_PODIUM_1_RED_PIN,LOW);
  digitalWrite(LED_PODIUM_1_ORANGE_PIN,LOW);
  digitalWrite(LED_PODIUM_1_GREEN_PIN,HIGH);
  
  digitalWrite(LED_PODIUM_2_RED_PIN,LOW);
  digitalWrite(LED_PODIUM_2_ORANGE_PIN,LOW);
  digitalWrite(LED_PODIUM_2_GREEN_PIN,HIGH);
  
  digitalWrite(LED_TECHNIC_RED_PIN,LOW);
  digitalWrite(LED_TECHNIC_ORANGE_PIN,LOW);
  digitalWrite(LED_TECHNIC_GREEN_PIN,HIGH);
    
}

void loop() {

  // Update the Bounce instances 
    db_BUTTON_PODIUM_GREEN_PIN.update();
  db_BUTTON_PODIUM_ORANGE_PIN.update();
  db_BUTTON_PODIUM_RED_PIN.update();
  
  db_BUTTON_TECHNIC_GREEN_PIN.update();
  db_BUTTON_TECHNIC_ORANGE_PIN.update();
  db_BUTTON_TECHNIC_RED_PIN.update();
  db_BUTTON_TECHNIC_SHOWPODIUM_PIN.update();
   
   
   // Call code if Bounce fell (transition from HIGH to LOW) :
   // Button control from PODIUM => signal local and Remote
   if ( db_BUTTON_PODIUM_GREEN_PIN.fell() ) {
        digitalWrite(LED_PODIUM_1_RED_PIN,LOW);
    digitalWrite(LED_PODIUM_1_ORANGE_PIN,LOW);
    digitalWrite(LED_PODIUM_1_GREEN_PIN,HIGH);
    
    digitalWrite(LED_PODIUM_2_RED_PIN,LOW);
    digitalWrite(LED_PODIUM_2_ORANGE_PIN,LOW);
    digitalWrite(LED_PODIUM_2_GREEN_PIN,HIGH);
    PODIUM_STATE_GREEN = HIGH;
    PODIUM_STATE_ORANGE = LOW;
    PODIUM_STATE_RED = LOW;

   }
   if ( db_BUTTON_PODIUM_ORANGE_PIN.fell() ) {
        digitalWrite(LED_PODIUM_1_RED_PIN,LOW);
    digitalWrite(LED_PODIUM_1_ORANGE_PIN,HIGH);
    digitalWrite(LED_PODIUM_1_GREEN_PIN,LOW);
    
    digitalWrite(LED_PODIUM_2_RED_PIN,LOW);
    digitalWrite(LED_PODIUM_2_ORANGE_PIN,HIGH);
    digitalWrite(LED_PODIUM_2_GREEN_PIN,LOW);
    
    PODIUM_STATE_GREEN = LOW;
    PODIUM_STATE_ORANGE = HIGH;
    PODIUM_STATE_RED = LOW;
   }
   if ( db_BUTTON_PODIUM_RED_PIN.fell() ) {
        digitalWrite(LED_PODIUM_1_RED_PIN,HIGH);
    digitalWrite(LED_PODIUM_1_ORANGE_PIN,LOW);
    digitalWrite(LED_PODIUM_1_GREEN_PIN,LOW);
    
    digitalWrite(LED_PODIUM_2_RED_PIN,HIGH);
    digitalWrite(LED_PODIUM_2_ORANGE_PIN,LOW);
    digitalWrite(LED_PODIUM_2_GREEN_PIN,LOW);
    
    PODIUM_STATE_GREEN = LOW;
    PODIUM_STATE_ORANGE = LOW;
    PODIUM_STATE_RED = HIGH;
   }
   
   //Button control from TECHNIC => signal Remote
   if ( db_BUTTON_PODIUM_GREEN_PIN.fell() ) {
    digitalWrite(LED_TECHNIC_RED_PIN,LOW);
    digitalWrite(LED_TECHNIC_ORANGE_PIN,LOW);
    digitalWrite(LED_TECHNIC_GREEN_PIN,HIGH);
    
    TECHNIC_STATE_GREEN = HIGH;
    TECHNIC_STATE_ORANGE = LOW;
    TECHNIC_STATE_RED = LOW;
   }
 
   if ( db_BUTTON_PODIUM_ORANGE_PIN.fell() ) {
        digitalWrite(LED_TECHNIC_RED_PIN,LOW);
    digitalWrite(LED_TECHNIC_ORANGE_PIN,HIGH);
    digitalWrite(LED_TECHNIC_GREEN_PIN,LOW);

    TECHNIC_STATE_GREEN = LOW;
    TECHNIC_STATE_ORANGE = HIGH;
    TECHNIC_STATE_RED = LOW;
   }
   if ( db_BUTTON_PODIUM_RED_PIN.fell() ) {
        digitalWrite(LED_TECHNIC_RED_PIN,HIGH);
    digitalWrite(LED_TECHNIC_ORANGE_PIN,LOW);
    digitalWrite(LED_TECHNIC_GREEN_PIN,LOW);

    TECHNIC_STATE_GREEN = LOW;
    TECHNIC_STATE_ORANGE = LOW;
    TECHNIC_STATE_RED = HIGH;
   }
   //Button control from TECHNIC, when this button is pushed we show the status for 'showTime' we send to PODIUM
   if ( db_BUTTON_TECHNIC_SHOWPODIUM_PIN.fell() ) {
      showStartTimeStamp = millis();
      showPodiumState = true;
        digitalWrite(LED_TECHNIC_RED_PIN,PODIUM_STATE_RED);
    digitalWrite(LED_TECHNIC_ORANGE_PIN,PODIUM_STATE_ORANGE);
    digitalWrite(LED_TECHNIC_GREEN_PIN,PODIUM_STATE_GREEN);
   }
   
   //when BUTTON_TECHNIC_SHOWPODIUM_PIN is pressed we show the PODIUM state on the TECHNIC's site for 'showTime' seconds
   unsigned long now = millis();
   bool turnShowPodium_OFF = now - showStartTimeStamp >showTime;
   if (turnShowPodium_OFF && showPodiumState) {
      digitalWrite(LED_TECHNIC_RED_PIN,TECHNIC_STATE_RED);
    digitalWrite(LED_TECHNIC_ORANGE_PIN,TECHNIC_STATE_ORANGE);
    digitalWrite(LED_TECHNIC_GREEN_PIN,TECHNIC_STATE_GREEN);  
    showPodiumState = false;
   }
   
   //blink the red led if it is lid to make sure it is noticed
   bool toggleBlink = now - blinkTimeStamp > 1000;
   if (TECHNIC_STATE_RED && toggleBlink){
      blinkState_TECHNIC = !blinkState_TECHNIC;
      digitalWrite(LED_TECHNIC_RED_PIN,blinkState_TECHNIC);
      blinkTimeStamp = now;
   }
   if (PODIUM_STATE_RED && toggleBlink){
      blinkState_PODIUM = !blinkState_PODIUM;
      digitalWrite(LED_PODIUM_1_RED_PIN,blinkState_PODIUM);
      digitalWrite(LED_PODIUM_2_RED_PIN,blinkState_PODIUM);
      blinkTimeStamp = now;
   }
   
}


