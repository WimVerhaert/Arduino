#include <RCSwitch.h>
#include <AccelStepper.h>

#define MAX_TIME 150
#define remoteControl_PIN 2
#define heartBeat_PIN 9
#define eyeBlink_PIN 11
#define eyeRotate_PIN 13
#define communication_PIN A0
#define stepper_IN1 A1
#define stepper_IN2 A2
#define stepper_IN3 A3
#define stepper_IN4 A4
#define OFF false
#define ON true
#define CLOCKWISE true
#define COUNTERCLOCKWISE false
#define runEvery(t) for (static typeof(t) _lasttime;(typeof(t))((typeof(t))millis() - _lasttime) > (t);_lasttime += (t))

RCSwitch myReceiver = RCSwitch();

int heartRate = 1500;
int eyeRate = 2000;
int lumenHeart = 100; // light intensity in %
int lumenEye = 100; //light intensity in %
bool eyeIsRotating = OFF;
bool eyeIsBlinking = OFF;
bool heartIsBeating = OFF;
bool communicationState = OFF;
bool slowStop = OFF;
bool eyeClockWise = CLOCKWISE;
unsigned long startCommunicationState=0;
unsigned long startSlowStop=0;
unsigned long previousMicros ;
long lastDebounceTime = 0;  // the last time the output pin was toggled
long debounceDelay = 450;    // the debounce time; increase if the output flickers

int stepInterval = 1000;
byte motor1Step = 1;

void setup() { // bring the LED up nicely from being off
  //Serial.begin(9600);
  pinMode(heartBeat_PIN, OUTPUT);
  pinMode(eyeBlink_PIN, OUTPUT);
  pinMode(eyeRotate_PIN, OUTPUT);
  pinMode(communication_PIN, OUTPUT);
  pinMode(stepper_IN1, OUTPUT);
  pinMode(stepper_IN2, OUTPUT);
  pinMode(stepper_IN3, OUTPUT);
  pinMode(stepper_IN4, OUTPUT);
  
  myReceiver.enableReceive(0);  // RC Receiver on interrupt 0 => that is pin #2 
  
//  Serial.begin(115200);
  digitalWrite(heartBeat_PIN, 0);   
  digitalWrite(eyeBlink_PIN, 0);   
  digitalWrite(eyeRotate_PIN, 0); 
  digitalWrite(communication_PIN, 1);
  delay(1000);   
  digitalWrite(communication_PIN,0);
     

}

void loop() {
  //Serial.print("Start loop:");
  //Serial.println(millis());
  //Serial.print("Start checkRC:");
  //Serial.println(millis());
  unsigned long startLoop = millis();
  CHECK_RC();
  if ((millis() - startCommunicationState >= 250) && (communicationState)) {
    communicationState = OFF;
    digitalWrite(communication_PIN,communicationState);
  }   
    // put your main code here, to run repeatedly:
  if (heartIsBeating) {
    //Serial.println("Heart is still beating");
    heartBeat(heartBeat_PIN,heartRate,lumenHeart);
    //maybe make some variation in the speed of the blinking
  }
  if (eyeIsBlinking) {
     //Serial.println("Eye is still blinking");
     heartBeat(eyeBlink_PIN,eyeRate,lumenEye);  
    //maybe make some variation in the speed of the blinking
  }
   if (eyeIsRotating) {
     //Serial.println("Eye is still rotating");
     stepper();  
    //maybe make some variation in the speed of the blinking
  }
  if (slowStop){
      if (millis() - startSlowStop >= 2000)  {          
          lumenEye = lumenEye - 10;
          lumenHeart = lumenHeart - 10;
          //eyeRate = eyeRate - 100;  
          //heartRate = heartRate - 100;  
          if (lumenEye <= 0) {
            lumenEye=0;
            lumenHeart=0; 
            eyeIsRotating = OFF;
            eyeIsBlinking = OFF;
            heartIsBeating = OFF;     
            eyeIsRotating = OFF;  
            slowStop = OFF;  
            analogWrite(eyeBlink_PIN, lumenEye);
            analogWrite(heartBeat_PIN, lumenHeart);      
            lumenEye = 100;
            lumenHeart = 100;
            heartRate = 1500;
            eyeRate = 2000;
          }
          //heartBeat(heartBeat_PIN,heartRate,lumenHeart);    
          //heartBeat(eyeBlink_PIN,eyeRate,lumenEye); 
          startSlowStop = millis(); 
      }
  }
}

void CHECK_RC() {
  
  
  if (myReceiver.available()) {   
    
    if ((millis() - lastDebounceTime) > debounceDelay) {
      //Serial.println(millis()- lastDebounceTime);
      int value = myReceiver.getReceivedValue();
      if (value == 0) {
        //Serial.print("Unknown encoding");
      } else {
//        Serial.print("Received ");
//        Serial.print( myReceiver.getReceivedValue() );
//        Serial.print(" / ");
//        Serial.print( myReceiver.getReceivedBitlength() );
//        Serial.print("bit ");
//        Serial.print("Protocol: ");
//        Serial.println( myReceiver.getReceivedProtocol() );
      }
      myReceiver.resetAvailable();
      lastDebounceTime = millis();
      if (value == int(69909)) { // button A:ON     
        // Serial.println("Received button A:ON  = Start Eye rotating and slow down");
        
        eyeIsRotating = ON;   
//        Serial.print("Eye Speed value ");
//        Serial.println( stepInterval ); 
//        Serial.print("eyeClockWise value ");
//        Serial.println( eyeClockWise );
      }
      if (value == int(69908)) {// button A:OFF
        //Serial.println("Received button A:OFF  = Stop Eye rotating and switch direction");
        eyeIsRotating = OFF;
        stepInterval = 1000;
        eyeClockWise=!eyeClockWise;
      }
      if (value == int(70677)) {// button B:ON
        //Serial.println("Received button B:ON  = Start Eye Blinking and speed up");
        eyeIsBlinking = ON;
        eyeRate = eyeRate - 200;
        if (eyeRate <=00) {
          eyeRate = 2000;
        }
      }
      if (value == int(70676)) {// button B:OFF
            //Serial.println("Received button B:OFF  = Slower Eye Blinking");      
        eyeRate = eyeRate + 200;  
 
      }
      if (value == int(70933)) {// button C:ON
        //Serial.println("Received button C:ON  = Start Heart Beating and speed up");     
        heartIsBeating = ON;
        heartRate = heartRate - 200;
       
        if (heartRate <=00) {
          heartRate = 2000;
        }
//        Serial.print("HeartRate value ");
//        Serial.println( heartRate ); 
      }
      if (value == int(70932)) {// button C:OFF
        //Serial.println("Received button C:OFF  = Slower Heart Beating");  
        heartRate = heartRate + 200;    
      }
      if (value == int(69653)) {// button D:ON
        //Serial.println("Received button D:ON  = fade eye and heart "); 
        lumenEye = lumenEye - 5;
        lumenHeart = lumenHeart - 5;
        if (lumenEye <= 0) {
          lumenEye=0;
          lumenHeart=0; 
        }
      }
      if (value == int(69652)) {// button D:OFF
        //Serial.println("Received button D:OFF  = ALL OFF"); 
        slowStop=ON;
        startSlowStop = millis();
       
      }
      myReceiver.resetAvailable();
      
      communicationState = ON;
      startCommunicationState = millis();
      digitalWrite(communication_PIN,communicationState);
    }
  
    else{
      myReceiver.resetAvailable();
    }
  }
}

void heartBeat(int ledPin, int freq, int lumen)
{
    float val = (exp(sin(millis()/float(freq)*PI)) - 0.36787944)*108.0;
    //let LED never completely go out
    int lumenPercentage = map(lumen,0,100,1,255);
    val = map(val,0,255, 1,lumenPercentage);
    analogWrite(ledPin, val);
   //  Serial.print("lumen value ");
   //   Serial.println( lumen );
   //    Serial.print("val value ");
   //   Serial.println( val );
}

void stepper()
{
  // Clockwise stepper code
  if (micros() >= previousMicros){
    previousMicros = previousMicros+stepInterval;
    if (eyeClockWise) {
       motor1Step = motor1Step +1;
       if (motor1Step >= 9){
          motor1Step = 1;
       }  
    }
    if (!eyeClockWise) {
       motor1Step = motor1Step -1;
       if (motor1Step <= 0){
          motor1Step = 8;
       }  
    }
    
  }
  switch (motor1Step){
  case 1:
    digitalWrite(stepper_IN4, HIGH);
    digitalWrite(stepper_IN3, LOW);
    digitalWrite(stepper_IN2, LOW);
    digitalWrite(stepper_IN1, LOW);
    break;
  case 2:
    digitalWrite(stepper_IN4, HIGH);
    digitalWrite(stepper_IN3, HIGH);
    digitalWrite(stepper_IN2, LOW);
    digitalWrite(stepper_IN1, LOW);
    break;
  case 3:
    digitalWrite(stepper_IN4, LOW);
    digitalWrite(stepper_IN3, HIGH);
    digitalWrite(stepper_IN2, LOW);
    digitalWrite(stepper_IN1, LOW);
    break;
  case 4:
    digitalWrite(stepper_IN4, LOW);
    digitalWrite(stepper_IN3, HIGH);
    digitalWrite(stepper_IN2, HIGH);
    digitalWrite(stepper_IN1, LOW);
    break;
  case 5:
    digitalWrite(stepper_IN4, LOW);
    digitalWrite(stepper_IN3, LOW);
    digitalWrite(stepper_IN2, HIGH);
    digitalWrite(stepper_IN1, LOW);
    break;
  case 6:
    digitalWrite(stepper_IN4, LOW);
    digitalWrite(stepper_IN3, LOW);
    digitalWrite(stepper_IN2, HIGH);
    digitalWrite(stepper_IN1, HIGH);
    break;
  case 7:
    digitalWrite(stepper_IN4, LOW);
    digitalWrite(stepper_IN3, LOW);
    digitalWrite(stepper_IN2, LOW);
    digitalWrite(stepper_IN1, HIGH);
    break;
  case 8:
    digitalWrite(stepper_IN4, HIGH);
    digitalWrite(stepper_IN3, LOW);
    digitalWrite(stepper_IN2, LOW);
    digitalWrite(stepper_IN1, HIGH);
    break;
  } // end switch
}
