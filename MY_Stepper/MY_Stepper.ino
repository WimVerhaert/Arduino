#define stepper_IN1 A1
#define stepper_IN2 A2
#define stepper_IN3 A3
#define stepper_IN4 A4
int Steps = 0;
boolean Direction = true;// gre
unsigned long last_time;
unsigned long currentMillis ;
int steps_left=4095;
long time;
void setup()
{
Serial.begin(9600);
pinMode(stepper_IN1, OUTPUT); 
pinMode(stepper_IN2, OUTPUT); 
pinMode(stepper_IN3, OUTPUT); 
pinMode(stepper_IN4, OUTPUT); 
// delay(1000);

}

void loop()
{
  
  int motorSpeed = 100;
  // set the motor speed:
 
  while(steps_left>0){
  currentMillis = micros();
  if(currentMillis-last_time>=1000){
  stepper(1); 
  time=time+micros()-last_time;
  last_time=micros();
  steps_left--;
  }
  }
   Serial.println(time);
  Serial.println("Wait...!");
  delay(2000);
  Direction=!Direction;
  steps_left=2*4095;
}

void stepper(int xw){
  for (int x=0;x<xw;x++){
switch(Steps){
   case 0:
     digitalWrite(stepper_IN1, LOW); 
     digitalWrite(stepper_IN2, LOW);
     digitalWrite(stepper_IN3, LOW);
     digitalWrite(stepper_IN4, HIGH);
   break; 
   case 1:
     digitalWrite(stepper_IN1, LOW); 
     digitalWrite(stepper_IN2, LOW);
     digitalWrite(stepper_IN3, HIGH);
     digitalWrite(stepper_IN4, HIGH);
   break; 
   case 2:
     digitalWrite(stepper_IN1, LOW); 
     digitalWrite(stepper_IN2, LOW);
     digitalWrite(stepper_IN3, HIGH);
     digitalWrite(stepper_IN4, LOW);
   break; 
   case 3:
     digitalWrite(stepper_IN1, LOW); 
     digitalWrite(stepper_IN2, HIGH);
     digitalWrite(stepper_IN3, HIGH);
     digitalWrite(stepper_IN4, LOW);
   break; 
   case 4:
     digitalWrite(stepper_IN1, LOW); 
     digitalWrite(stepper_IN2, HIGH);
     digitalWrite(stepper_IN3, LOW);
     digitalWrite(stepper_IN4, LOW);
   break; 
   case 5:
     digitalWrite(stepper_IN1, HIGH); 
     digitalWrite(stepper_IN2, HIGH);
     digitalWrite(stepper_IN3, LOW);
     digitalWrite(stepper_IN4, LOW);
   break; 
     case 6:
     digitalWrite(stepper_IN1, HIGH); 
     digitalWrite(stepper_IN2, LOW);
     digitalWrite(stepper_IN3, LOW);
     digitalWrite(stepper_IN4, LOW);
   break; 
   case 7:
     digitalWrite(stepper_IN1, HIGH); 
     digitalWrite(stepper_IN2, LOW);
     digitalWrite(stepper_IN3, LOW);
     digitalWrite(stepper_IN4, HIGH);
   break; 
   default:
     digitalWrite(stepper_IN1, LOW); 
     digitalWrite(stepper_IN2, LOW);
     digitalWrite(stepper_IN3, LOW);
     digitalWrite(stepper_IN4, LOW);
   break; 
}
SetDirection();
}
} 
void SetDirection(){
if(Direction==1){ Steps++;}
if(Direction==0){ Steps--; }
if(Steps>7){Steps=0;}
if(Steps<0){Steps=7; }
}
