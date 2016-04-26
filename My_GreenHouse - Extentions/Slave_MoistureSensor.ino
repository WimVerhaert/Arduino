
// Remote soil moisture 4-sensor node based on Vinduino-H V3
// Added galvanic voltage bias compensation
// Date April 21, 2015

#include <Wire.h>       // I2C communication to RTC
#include <math.h>       // Conversion equation from resistance to %

// Setting up format for reading 3 soil sensors
#define NUM_READS 11    // Number of sensor reads for filtering

// >> put this into a header file you include at the beginning for better clarity
enum { 
  I2C_CMD_GET_ANALOGS = 1
};

enum { 
  I2C_MSG_ARGS_MAX = 32,
  I2C_RESP_LEN_MAX = 32
};

#define I2C_ADDR                 0        

extern const byte supportedI2Ccmd[] = { 
  1
};
// << put this into a header file you include at the beginning for better clarity


int supplyVoltage;                // Measured supply voltage
int sensorVoltage;                // Measured sensor voltage

int argsCnt = 0;                        // how many arguments were passed with given command
int requestedCmd = 0;                   // which command was requested (if any)

byte i2cArgs[I2C_MSG_ARGS_MAX];         // array to store args received from master
int i2cArgsLen = 0;                     // how many args passed by master to given command

uint8_t i2cResponse[I2C_RESP_LEN_MAX];  // array to store response
int i2cResponseLen = 0;                 // response length

typedef struct {        // Structure to be used in percentage and resistance values matrix to be filtered (have to be in pairs)
  int moisture;
  long resistance;
} values;

const long knownResistor = 4700;  // Constant value of known resistor in Ohms

values valueOf[NUM_READS];        // Calculated  resistances to be averaged
long buffer[NUM_READS];
int index;

int i;                            // Simple index variable


void setup() {
  // initialize serial communications at 9600 bps:
  Serial.begin(9600); 
  
    // initialize I2C communications
  Wire.begin();
  
  // initialize the digital pins as an output.
  // Pin 6,7 is for sensor 1
  pinMode(6, OUTPUT);    
  pinMode(7, OUTPUT); 
  // Pin 8,9 is for sensor 2
  pinMode(8, OUTPUT);    
  pinMode(9, OUTPUT);  
  // Pin 10,11 is for sensor 3
  pinMode(10, OUTPUT);    
  pinMode(11, OUTPUT);
  
  // Pin 13 is radio enable
  pinMode(13, OUTPUT); 
  Wire.begin(I2C_ADDR);                        // join i2c bus 
  Wire.onRequest(requestEvent);                // register event
  Wire.onReceive(receiveEvent);    
  // << starting i2c 
  
}

void loop() {
  
 // read sensor 1-3, filter, and calculate resistance value
 // Noise filter: median filter
 
measure(1,6,7,1);
long read1 = average();
measure(1,7,6,0);
long read2= average();
long sensor1 = (read1 + read2)/2;

delay (100);


measure(2,8,9,2);
long read3 = average();
measure(2,9,8,6);
long read4= average();
long sensor2 = (read3 + read4)/2;

delay (100);

measure(3,10,11,3);
long read5 = average();
measure(3,11,10,7);
long read6= average();
long sensor3 = (read5 + read6)/2;


 
   // convert data to string
  char buf[16];

  char sn1[16];
  String strsensor1 = dtostrf(sensor1, 4, 1, buf);
  
  char sn2[16];
  String strsensor2 = dtostrf(sensor2, 4, 1, buf);
  
  char sn3[16];
  String strsensor3 = dtostrf(sensor3, 4, 1, buf);
  
  char Vbatt[16];
  
//  updateTemp(strsensor1, strsensor2, strsensor3, strVcc );
  
 if(requestedCmd == I2C_CMD_GET_ANALOGS){
    // read inputs and save to response array; example (not tested) below
    i2cResponseLen = 0;
    // analog readings should be averaged and not read one-by-one to reduce noise which is not done in this example
    i2cResponseLen++;
    i2cResponse[i2cResponseLen -1] = analogRead(A0);
    i2cResponseLen++;
    i2cResponse[i2cResponseLen -1] = analogRead(A1);
    i2cResponseLen++;
    i2cResponse[i2cResponseLen -1] = analogRead(A2);
    i2cResponseLen++;
    i2cResponse[i2cResponseLen -1] = analogRead(A3);
    // now slave is ready to send back four bytes each holding analog reading from a specific analog input; you can improve robustness of the protocol by including e.g. crc16 at the end or instead of returning just 4 bytes return 8 where odd bytes indicate analog input indexes and even bytes their values; change master implementation accordingly
    requestedCmd = 0;   // set requestd cmd to 0 disabling processing in next loop

  }
  else if (requestedCmd != 0){
    // log the requested function is unsupported (e.g. by writing to serial port or soft serial

    requestedCmd = 0;   // set requestd cmd to 0 disabling processing in next loop
  }

}

void measure (int sensor, int phase_b, int phase_a, int analog_input)
{
 
  // read sensor, filter, and calculate resistance value
  // Noise filter: median filter

  for (i=0; i<NUM_READS; i++) {

    // Read 1 pair of voltage values
    digitalWrite(phase_a, HIGH);                 // set the voltage supply on
    delayMicroseconds(25);
    delayMicroseconds(25);
    digitalWrite(phase_a, LOW);                  // set the voltage supply off 
    delay(1);
     
    digitalWrite(phase_b, HIGH);                 // set the voltage supply on
    delayMicroseconds(25);
    sensorVoltage = analogRead(analog_input);   // read the sensor voltage
    delayMicroseconds(25);
    digitalWrite(phase_b, LOW);                  // set the voltage supply off 

    // Calculate resistance
    // the 0.5 add-term is used to round to the nearest integer
    // Tip: no need to transform 0-1023 voltage value to 0-5 range, due to following fraction
    long resistance = (knownResistor * (supplyVoltage - sensorVoltage ) / sensorVoltage) ;
    
    delay(1); 
    addReading(resistance);
    
   }
  }


// Averaging algorithm
void addReading(long resistance){
  buffer[index] = resistance;
  index++;
  if (index >= NUM_READS) index = 0;
}

long average(){
  long sum = 0;
  for (int i = 0; i < NUM_READS; i++){
    sum += buffer[i];
  }
  return (long)(sum / NUM_READS);
}



// function that executes whenever data is requested by master
// this function is registered as an event, see setup()
void requestEvent(){

  Wire.write(i2cResponse, i2cResponseLen);

}


// function that executes when master sends data (begin-end transmission)
// this function is registered as an event, see setup()
void receiveEvent(int howMany)
{
  //digitalWrite(13,HIGH);
  int cmdRcvd = -1;
  int argIndex = -1; 
  argsCnt = 0;

  if (Wire.available()){
    cmdRcvd = Wire.read();                 // receive first byte - command assumed
    while(Wire.available()){               // receive rest of tramsmission from master assuming arguments to the command
      if (argIndex < I2C_MSG_ARGS_MAX){
        argIndex++;
        i2cArgs[argIndex] = Wire.read();
      }
      else{
        ; // implement logging error: "too many arguments"
      }
      argsCnt = argIndex+1;  
    }
  }
  else{
    // implement logging error: "empty request"
    return;
  }
  // validating command is supported by slave
  int fcnt = -1;
  for (int i = 0; i < sizeof(supportedI2Ccmd); i++) {
    if (supportedI2Ccmd[i] == cmdRcvd) {
      fcnt = i;
    }
  }

  if (fcnt<0){
    // implement logging error: "command not supported"
    return;
  }
  requestedCmd = cmdRcvd;
  // now main loop code should pick up a command to execute and prepare required response when master waits before requesting response
}
