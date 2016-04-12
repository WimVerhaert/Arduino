#include <RCSwitch.h>
#include <AFMotor.h>
#include <AccelStepper.h>

#define MAX_TIME 150
#define remoteControl_PIN 2
#define heartBeat_PIN 9
#define eyeBlink_PIN 10
#define eyeRotate_PIN 13
#define communication_PIN A0
#define stepper_IN1 A1
#define stepper_IN2 A2
#define stepper_IN3 A3
#define stepper_IN4 A4
#define OFF false
#define ON true
#define runEvery(t) for (static typeof(t) _lasttime;(typeof(t))((typeof(t))millis() - _lasttime) > (t);_lasttime += (t))

RCSwitch myReceiver = RCSwitch();
AF_DCMotor motor1(1);
AF_DCMotor motor2(2);
AF_DCMotor motor3(3);
AF_DCMotor motor4(4);

int heartRate = 2200;
bool motor1State = OFF;
bool motor2State = OFF;
bool motor3State = OFF;
bool motor4State = OFF;
bool eyeIsRotating = OFF;
bool eyeIsBlinking = OFF;
bool heartIsBeating = OFF;
bool communicationState = OFF;
unsigned long starttimeMotor1=0;
unsigned long starttimeMotor2=0;
unsigned long starttimeMotor3=0;
unsigned long starttimeMotor4=0;
unsigned long startCommunicationState=0;
unsigned long previousMicros ;

const long motorRunTime = 5000;
int stepInterval = 1000;
byte motor1Step = 0;

void setup() { // bring the LED up nicely from being off
  Serial.begin(9600);
  pinMode(heartBeat_PIN, OUTPUT);
  pinMode(eyeBlink_PIN, OUTPUT);
  pinMode(eyeRotate_PIN, OUTPUT);
  pinMode(communication_PIN, OUTPUT);
  pinMode(stepper_IN1, OUTPUT);
  pinMode(stepper_IN2, OUTPUT);
  pinMode(stepper_IN3, OUTPUT);
  pinMode(stepper_IN4, OUTPUT);
  
  myReceiver.enableReceive(0);  // RC Receiver on interrupt 0 => that is pin #2 

  motor1.setSpeed(120);
  motor2.setSpeed(120);
  motor3.setSpeed(120);
  motor4.setSpeed(120);

  //set direction of motors and set speed
  motor1.run(FORWARD); 
  motor1.setSpeed(200);
  motor2.run(FORWARD);
  motor2.setSpeed(200);
  motor3.run(FORWARD);
  motor3.setSpeed(200);
  motor4.run(FORWARD);
  motor4.setSpeed(200);
  
  motor1.run(RELEASE);
  motor2.run(RELEASE);
  motor3.run(RELEASE);
  motor4.run(RELEASE);
  
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
    heartBeat(heartBeat_PIN,2000);
    //maybe make some variation in the speed of the blinking
  }
  if (eyeIsBlinking) {
     //Serial.println("Eye is still blinking");
     heartBeat(eyeBlink_PIN,heartRate);  
    //maybe make some variation in the speed of the blinking
  }
   if (eyeIsRotating) {
     //Serial.println("Eye is still rotating");
     stepper();  
    //maybe make some variation in the speed of the blinking
  }
  if (motor1State){
    //Serial.println("Motor 1 still running");
    if (millis() - starttimeMotor1 >= motorRunTime) {
      motor1State = OFF;
      motor1.run(RELEASE);      
    }    
  }  
  if (motor2State){
    //Serial.println("Motor 2 still running");
    if (millis() - starttimeMotor2 >= motorRunTime) {
      motor2State = OFF;
      motor2.run(RELEASE);
    } 
  }
  if (motor3State){
    //Serial.println("Motor 3 still running");
    if (millis() - starttimeMotor3 >= motorRunTime) {
      motor3State = OFF;
      motor3.run(RELEASE);
    }     
  }
  if (motor4State){
    //Serial.println("Motor 4 still running");
    if (millis() - starttimeMotor4 >= motorRunTime) {
      motor4State = OFF;
      motor4.run(RELEASE);
    }     
  }
//  Serial.print("Loop Time:");
//  Serial.println(millis()-startLoop);
//  Serial.print("StatusBits:");
//  Serial.print(motor1State);
//  Serial.print(motor2State);
//  Serial.print(motor3State);
//  Serial.print(motor4State);
//  Serial.print(eyeIsRotating);
//  Serial.print(eyeIsBlinking);
//  Serial.print(heartIsBeating);
//  Serial.print(communicationState);
//  Serial.println("");
  
  
}

void CHECK_RC() {
  if (myReceiver.available()) {   
    int value = myReceiver.getReceivedValue();
    if (value == 0) {
      Serial.print("Unknown encoding");
    } else {
      Serial.print("Received ");
      Serial.print( myReceiver.getReceivedValue() );
      Serial.print(" / ");
      Serial.print( myReceiver.getReceivedBitlength() );
      Serial.print("bit ");
      Serial.print("Protocol: ");
      Serial.println( myReceiver.getReceivedProtocol() );
    }
    myReceiver.resetAvailable();
    
    if (value == int(69909)) { // button A:ON     
       // Serial.println("Received button A:ON  = Start Motor 1");
        motor1State = ON;
        starttimeMotor1 = millis();
        motor1.run(FORWARD); 
        motor1.setSpeed(100);
    }
    if (value == int(69908)) {// button A:OFF
      //Serial.println("Received button A:OFF  = Start Motor 2");
      motor2State = ON;
      starttimeMotor2 = millis();
      motor2.run(FORWARD); 
      motor2.setSpeed(200);
    }
    if (value == int(70677)) {// button B:ON
      //Serial.println("Received button B:ON  = Start Motor 3");
      motor3State = ON; 
      starttimeMotor3 = millis();  
      motor3.run(FORWARD);    
      motor3.setSpeed(200);
    }
    if (value == int(70676)) {// button B:OFF
      //Serial.println("Received button B:OFF  = Start Motor 4");      
      motor4State = ON;
      starttimeMotor4 = millis();
      motor4.run(FORWARD); 
      motor4.setSpeed(200);
    }
    if (value == int(70933)) {// button C:ON
      //Serial.println("Received button C:ON  = Start Heart Beating");      
      heartIsBeating = ON;
    }
    if (value == int(70932)) {// button C:OFF
      //Serial.println("Received button C:OFF  = Start Eye Blinking");      
      eyeIsBlinking = ON;
    }
    if (value == int(69653)) {// button D:ON
      //Serial.println("Received button D:ON  = "); 
      eyeIsRotating = ON;    
    }
    if (value == int(69652)) {// button D:OFF
      //Serial.println("Received button D:OFF  = ALL OFF"); 
      motor1State = OFF;
      motor2State = OFF;
      motor3State = OFF;
      motor4State = OFF;
      eyeIsRotating = OFF;
      eyeIsBlinking = OFF;
      heartIsBeating = OFF;     
      eyeIsRotating = OFF;
      //setup();
    }
    myReceiver.resetAvailable();
    
    communicationState = ON;
    startCommunicationState = millis();
    digitalWrite(communication_PIN,communicationState);
  }
  
}

void heartBeat(int ledPin, int freq)
{
    float val = (exp(sin(millis()/float(freq)*PI)) - 0.36787944)*108.0;
    //let LED never completely go out
    val = map(val,0,255, 1,255);
    analogWrite(ledPin, val);
}

void stepper()
{
  // Clockwise stepper code
  if (micros() >= previousMicros){
    previousMicros = previousMicros+stepInterval;
    motor1Step = motor1Step +1;
    if (motor1Step == 9){
      motor1Step = 0;
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
